// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9GameInstance.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraShakeBase.h"
#include "CL9.h"
#include "Data/CL9CharacterDef.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ItemDef.h"
#include "Data/CL9MonsterDef.h"
#include "Data/CL9RulesDef.h"
#include "Data/CL9ShakeTable.h"
#include "Data/CL9SoundTable.h"
#include "Data/CL9WaveTable.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"
#include "Misc/ConfigCacheIni.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "UI/CL9SettingsTable.h"
#include "UI/CL9UITable.h"

UCL9DefRegistry* UCL9GameInstance::CreateStandaloneRegistry(UObject* Outer)
{
	// 엔진 설정에서 프로젝트의 게임 인스턴스 클래스를 가져온다 — 에디터 도구 하나
	// 때문에 이 모듈이 EngineSettings에 의존하지 않도록 직접 읽는다
	FString ClassPath;
	GConfig->GetString(TEXT("/Script/EngineSettings.GameMapsSettings"), TEXT("GameInstanceClass"), ClassPath, GEngineIni);
	const UClass* Class = FSoftClassPath(ClassPath).TryLoadClass<UGameInstance>();
	const UCL9GameInstance* Defaults = Class ? Cast<UCL9GameInstance>(Class->GetDefaultObject()) : nullptr;
	if (Defaults == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("The project's game instance class is not a CL9 game instance — no def list to read."));
		return nullptr;
	}

	UCL9DefRegistry* Registry = NewObject<UCL9DefRegistry>(Outer);
	Registry->Init(Defaults->DefListAsset.LoadSynchronous());
	return Registry;
}

void UCL9GameInstance::Init()
{
	Super::Init();

	DefRegistry = NewObject<UCL9DefRegistry>(this);

	const UCL9DefList* DefList = DefListAsset.LoadSynchronous();
	if (DefList == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("'%s' has no def list asset assigned — set it in the game instance BP."), *GetNameSafe(this));
	}
	DefRegistry->Init(DefList);

	// 행의 소프트 참조를 지금 다 올려 붙든다 — 첫 사용 자리(종별 첫 스폰·첫 타격)에서 동기 로드로 프레임이 서지 않게
	// (docs/systems/performance.md 첫 사용 동기 로드 금지). 사용처의 LoadSynchronous 는 안전망으로 남는다 — 올라와 있으면 즉시 돌아온다
	if (const UCL9RulesDef* Rules = DefRegistry->GetRules())
	{
		Warm(Rules->HitFlashMaterial.LoadSynchronous());
		Warm(Rules->HitPuffSystem.LoadSynchronous());
	}
	for (const UCL9MonsterDef* Monster : DefRegistry->GetMonsters())
	{
		Warm(Monster->ActorClass.LoadSynchronous());
		Warm(Monster->SkeletalMesh.LoadSynchronous());
		Warm(Monster->AnimBP.LoadSynchronous());
	}
	// 아이템은 밤 시작 외에도 드롭·치트로 판 도중 처음 스폰된다 — 종별 첫 드롭에서 메시·클래스를 로드하지 않게
	for (const UCL9ItemDef* Item : DefRegistry->GetItems())
	{
		Warm(Item->ActorClass.LoadSynchronous());
		Warm(Item->Mesh.LoadSynchronous());
		Warm(Item->Icon.LoadSynchronous());
	}
	for (const UCL9CharacterDef* Character : DefRegistry->GetCharacters())
	{
		Warm(Character->SkeletalMesh.LoadSynchronous());
		Warm(Character->AnimBP.LoadSynchronous());
		Warm(Character->ArmsMesh.LoadSynchronous());
		Warm(Character->ArmsAnimBP.LoadSynchronous());
	}

	UITable = UITableAsset.LoadSynchronous();
	if (UITable == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("'%s' has no UI table assigned — the game runs without UI."), *GetNameSafe(this));
	}

	ShakeTable = ShakeTableAsset.LoadSynchronous();
	if (ShakeTable == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("'%s' has no shake table assigned — camera shakes stay silent."), *GetNameSafe(this));
	}
	else
	{
		// 셰이크는 블루프린트 클래스라 첫 재생 자리(잡히는 순간의 캐릭터 틱)에서 로드하면 컴파일까지 붙어 0.6초를 세운다(2026-09-21 실측)
		for (const TPair<FName, FCL9ShakeInfo>& Row : ShakeTable->Rows)
		{
			Warm(Row.Value.Shake.LoadSynchronous());
		}
	}

	// 없으면 여기서 한 번만 알린다 — 재생 헬퍼(CL9Sound::PlayAt)는 매 호출 조용히 지나간다
	SoundTable = SoundTableAsset.LoadSynchronous();
	if (SoundTable == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("'%s' has no sound table assigned — sound effects stay silent."), *GetNameSafe(this));
	}
	else
	{
		// 행의 사운드 애셋을 지금 다 올려 붙든다 — 첫 재생(첫 잡힘)에서 동기 로드로 프레임이 끊기지 않게. 빈 행은 조용히 건너뛴다
		for (const TPair<FName, TSoftObjectPtr<USoundBase>>& Row : SoundTable->Rows)
		{
			Warm(Row.Value.LoadSynchronous());
		}
	}

	// 검사는 표를 받은 이 자리에서 한 번만 돈다 — 깨진 행은 여기서 비활성이 된다 (docs/systems/wave.md 로드 검사)
	UCL9WaveTable* LoadedWaveTable = WaveTableAsset.LoadSynchronous();
	if (LoadedWaveTable == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("'%s' has no wave table assigned — no noise draws a ring."), *GetNameSafe(this));
	}
	else
	{
		LoadedWaveTable->ValidateRows();
		// 첫 링에서 나이아가라 시스템을 동기 로드하면 0.5초를 세운다(2026-09-21 실측) — 행의 시스템을 지금 올려 붙든다
		for (const TPair<ECL9NoiseSource, FCL9WaveInfo>& Row : LoadedWaveTable->Rows)
		{
			Warm(Row.Value.Niagara.LoadSynchronous());
		}
	}
	WaveTable = LoadedWaveTable;

	SettingsTable = SettingsTableAsset.LoadSynchronous();
	if (SettingsTable == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("'%s' has no settings table assigned — volume sliders reach nothing."), *GetNameSafe(this));
	}
}

void UCL9GameInstance::Warm(UObject* Asset)
{
	if (Asset != nullptr)
	{
		WarmAssets.AddUnique(Asset);
	}
}
