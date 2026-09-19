// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9GameInstance.h"
#include "CL9.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ShakeTable.h"
#include "Data/CL9SoundTable.h"
#include "Data/CL9WaveTable.h"
#include "Misc/ConfigCacheIni.h"
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

	// 없으면 여기서 한 번만 알린다 — 재생 헬퍼(CL9Sound::PlayAt)는 매 호출 조용히 지나간다
	SoundTable = SoundTableAsset.LoadSynchronous();
	if (SoundTable == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("'%s' has no sound table assigned — sound effects stay silent."), *GetNameSafe(this));
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
	}
	WaveTable = LoadedWaveTable;

	SettingsTable = SettingsTableAsset.LoadSynchronous();
	if (SettingsTable == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("'%s' has no settings table assigned — volume sliders reach nothing."), *GetNameSafe(this));
	}
}
