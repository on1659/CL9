// Copyright Epic Games, Inc. All Rights Reserved.

#include "FX/CL9WaveRenderer.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "CL9.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9MonsterDef.h"
#include "Data/CL9RulesDef.h"
#include "Data/CL9WaveTable.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameState.h"
#include "GameMode/CL9PlayerState.h"
#include "Monsters/CL9Monster.h"

namespace
{
	/** 차폐 트레이스가 쓰는 전용 채널 — 조각 표의 bBlocksSight가 시야·파동·카메라를 한꺼번에 정한다 (Config/DefaultEngine.ini CL9_Occlusion) */
	constexpr ECollisionChannel OcclusionChannel = ECC_GameTraceChannel2;

	/** 디버그 원의 분할 수 — 판정 반경과 링이 같은지 눈으로 대조하는 용도라 이 정도면 원으로 보인다 */
	constexpr int32 DebugCircleSegments = 48;
}

bool UCL9WaveRenderer::bShowNoise = false;

UCL9WaveRenderer* UCL9WaveRenderer::Get(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine != nullptr ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World != nullptr ? World->GetSubsystem<UCL9WaveRenderer>() : nullptr;
}

bool UCL9WaveRenderer::ToggleShowNoise()
{
	bShowNoise = !bShowNoise;
	return bShowNoise;
}

const UCL9WaveTable* UCL9WaveRenderer::GetTable() const
{
	const UCL9GameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance<UCL9GameInstance>() : nullptr;
	return GameInstance != nullptr ? GameInstance->GetWaveTable() : nullptr;
}

bool UCL9WaveRenderer::IsLocalInstigator(const AActor* Instigator) const
{
	if (Instigator == nullptr || GetWorld() == nullptr)
	{
		return false;
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* Controller = It->Get();
		if (Controller == nullptr || !Controller->IsLocalController())
		{
			continue;
		}
		const AActor* LocalPlayerState = Controller->PlayerState;
		if (LocalPlayerState == Instigator || Controller->GetPawn() == Instigator)
		{
			return true;
		}
	}
	return false;
}

bool UCL9WaveRenderer::IsVisibleNow(const FCL9WaveInfo& Row) const
{
	const ACL9GameState* GameState = GetWorld() ? GetWorld()->GetGameState<ACL9GameState>() : nullptr;
	if (GameState == nullptr)
	{
		return Row.Visibility == ECL9WaveVisibility::Always;
	}

	switch (Row.Visibility)
	{
	case ECL9WaveVisibility::NightOnly:
		return GameState->GetPhase() == ECL9NightPhase::Night;
	case ECL9WaveVisibility::BlackoutOnly:
		// 정전은 복제 bool이 아니라 서버 시각에서 파생된다 — 클라도 그대로 읽는다 (docs/systems/light.md)
		return GameState->IsBlackout();
	default:
		return true;
	}
}

float UCL9WaveRenderer::ResolveRadius(const FCL9WaveInfo& Row, const AActor* Instigator) const
{
	if (Row.RadiusFrom == ECL9NoiseRadiusSource::PackAlarm)
	{
		// 울음 링은 무리 경보의 시각 표현이다 — 링이 닿은 동료가 실제로 깨어난다
		const ACL9Monster* Monster = Cast<ACL9Monster>(Instigator);
		const UCL9MonsterDef* MonsterDef = Monster != nullptr ? Monster->GetMonsterDef() : nullptr;
		return MonsterDef != nullptr ? MonsterDef->PackAlarmRadius * Row.RadiusScale : 0.f;
	}

	if (Row.RadiusFrom == ECL9NoiseRadiusSource::None)
	{
		return Row.FallbackRadius * Row.RadiusScale;
	}

	const UCL9GameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance<UCL9GameInstance>() : nullptr;
	const UCL9DefRegistry* Registry = GameInstance != nullptr ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9RulesDef* Rules = Registry != nullptr ? Registry->GetRules() : nullptr;
	if (Rules == nullptr)
	{
		return 0.f;
	}

	float Ruling = 0.f;
	switch (Row.RadiusFrom)
	{
	case ECL9NoiseRadiusSource::RunNoise:
		Ruling = Rules->RunNoiseRadius;
		break;
	case ECL9NoiseRadiusSource::PickupQuiet:
		Ruling = Rules->PickupQuietRadius;
		break;
	case ECL9NoiseRadiusSource::PickupLoud:
		Ruling = Rules->PickupLoudRadius;
		break;
	case ECL9NoiseRadiusSource::PushNoise:
		Ruling = Rules->PushNoiseRadius;
		break;
	case ECL9NoiseRadiusSource::Decoy:
		Ruling = Rules->DecoyRadius;
		break;
	default:
		break;
	}
	return Ruling * Row.RadiusScale;
}

void UCL9WaveRenderer::TraceOcclusion(const FVector& Location, const FCL9WaveInfo& Row, float Radius, const AActor* Instigator, TArray<float>& OutDistances) const
{
	OutDistances.Reset();
	if (Row.OcclusionRays <= 0)
	{
		return;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CL9WaveOcclusion), false);
	if (Instigator != nullptr)
	{
		QueryParams.AddIgnoredActor(Instigator);
		// 소리를 낸 사람의 몸이 자기 소리를 막지 않는다 — 발신자가 PlayerState면 그 폰까지 뺀다
		if (const APlayerState* PlayerState = Cast<APlayerState>(Instigator))
		{
			QueryParams.AddIgnoredActor(PlayerState->GetPawn());
		}
	}

	const FVector Origin = Location + FVector(0.f, 0.f, Row.TraceHeight);
	OutDistances.Reserve(Row.OcclusionRays);
	for (int32 Ray = 0; Ray < Row.OcclusionRays; ++Ray)
	{
		// 0번이 +X, 반시계로 균등 — 나이아가라가 파티클 각도로 이 배열을 조회한다
		const float Angle = (2.f * UE_PI * Ray) / Row.OcclusionRays;
		const FVector End = Origin + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Radius;

		FHitResult Hit;
		const bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, Origin, End, OcclusionChannel, QueryParams);
		OutDistances.Add(bBlocked ? Hit.Distance : Radius);
	}
}

void UCL9WaveRenderer::EnforceConcurrency(int32 MaxConcurrent)
{
	// 이미 사라진 것부터 치운다 — 컴포넌트는 bAutoDestroy로 스스로 없어진다
	LiveWaves.RemoveAll([](const TWeakObjectPtr<UNiagaraComponent>& Wave)
	{
		return !Wave.IsValid();
	});

	if (MaxConcurrent <= 0)
	{
		return;
	}

	while (LiveWaves.Num() >= MaxConcurrent)
	{
		UNiagaraComponent* Oldest = LiveWaves[0].Get();
		LiveWaves.RemoveAt(0);
		if (Oldest != nullptr)
		{
			Oldest->DestroyComponent();
		}
	}
}

void UCL9WaveRenderer::PlayWave(ECL9NoiseSource Source, const FVector& Location, AActor* Instigator, bool bPredicted)
{
	UWorld* World = GetWorld();
	const UCL9WaveTable* Table = GetTable();
	if (World == nullptr || Table == nullptr)
	{
		return;
	}

	// 전용 서버에는 그릴 화면이 없다 — 차폐 트레이스까지 통째로 건너뛴다
	if (World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	// 1. 행 — 없거나, 로드 검사에서 떨어졌거나, 꺼져 있으면 조용히 끝낸다
	const FCL9WaveInfo* Found = Table->FindRow(Source);
	if (Found == nullptr || !Found->bEnabled)
	{
		return;
	}
	const FCL9WaveInfo& Row = *Found;

	// 2. 표시 시점
	if (!IsVisibleNow(Row))
	{
		return;
	}

	// 3. 폭주 안전판 — 같은 발신자·같은 행이 MinInterval 안에 두 번 나지 않는다
	const double Now = World->GetTimeSeconds();
	const TPair<FObjectKey, uint8> IntervalKey(FObjectKey(Instigator), static_cast<uint8>(Source));
	if (Row.MinInterval > 0.f)
	{
		if (const double* Last = LastPlayTimes.Find(IntervalKey))
		{
			if (Now - *Last < Row.MinInterval)
			{
				return;
			}
		}
	}

	// 4. 거리 컷 — 로컬 카메라에서 먼 발원지는 트레이스도 스폰도 하지 않는다
	const APlayerController* LocalController = World->GetFirstPlayerController();
	if (LocalController != nullptr && Table->CullDistance > 0.f)
	{
		FVector ViewLocation = FVector::ZeroVector;
		FRotator ViewRotation = FRotator::ZeroRotator;
		LocalController->GetPlayerViewPoint(ViewLocation, ViewRotation);
		if (FVector::DistSquared(ViewLocation, Location) > FMath::Square(Table->CullDistance))
		{
			return;
		}
	}

	// 5. 반경 — 판정이 정하고 링은 그것을 비춘다
	const float Radius = ResolveRadius(Row, Instigator);
	if (Radius <= 0.f)
	{
		return;
	}

	LastPlayTimes.Add(IntervalKey, Now);

	// 6. 색과 밝기 — 내 소리와 남의 소리는 색이 아니라 OwnerPowerScale이 가른다
	const bool bMine = bPredicted || IsLocalInstigator(Instigator);
	const ACL9PlayerState* PlayerState = Cast<ACL9PlayerState>(Instigator);
	const FLinearColor Color = (Row.bUseInstigatorColor && PlayerState != nullptr)
		? PlayerState->GetSlotColor(Row.Color)
		: Row.Color;
	const float Power = Row.Power * (bMine ? Row.OwnerPowerScale : 1.f);

	// 7. 차폐 — 생성 때 한 번만 쏜다. 이후 링이 커지는 것은 나이아가라 안의 일이다
	TArray<float> Distances;
	TraceOcclusion(Location, Row, Radius, Instigator, Distances);

	if (bShowNoise)
	{
		UE_LOG(LogCL9, Log, TEXT("wave '%s'%s at %s — radius %.0f cm, %d rays, power %.2f."),
			*CL9Wave::ToAlias(Source), bMine ? TEXT(" (mine)") : TEXT(""), *Location.ToCompactString(), Radius, Distances.Num(), Power);
#if ENABLE_DRAW_DEBUG
		// 판정 반경 그 자체 — 링이 멈추는 자리와 이 원이 같은지 눈으로 대조한다
		DrawDebugCircle(World, Location, Radius, DebugCircleSegments, Color.ToFColor(true), false,
			Row.Speed > 0.f ? Radius / Row.Speed : 1.f, 0, 2.f, FVector(1.f, 0.f, 0.f), FVector(0.f, 1.f, 0.f), false);
#endif
	}

	// 8. 스폰 — 행의 나이아가라가 비어 있으면 여기까지만 한다(에셋 없이도 위의 로그로 사건을 검증할 수 있다)
	UNiagaraSystem* System = Row.Niagara.LoadSynchronous();
	if (System == nullptr)
	{
		return;
	}

	EnforceConcurrency(Table->MaxConcurrent);

	UNiagaraComponent* Wave = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, System, Location, FRotator::ZeroRotator, FVector::OneVector, true, true);
	if (Wave == nullptr)
	{
		return;
	}

	// 나이아가라가 받는 것은 파라미터 이름과 파생된 값뿐이다 — 수치도 에셋 경로도 C++에 없다
	Wave->SetVariableFloat(TEXT("User.Radius"), Radius);
	Wave->SetVariableFloat(TEXT("User.Speed"), Row.Speed);
	Wave->SetVariableFloat(TEXT("User.Thickness"), Row.Thickness);
	Wave->SetVariableLinearColor(TEXT("User.Color"), Color);
	Wave->SetVariableFloat(TEXT("User.Power"), Power);
	Wave->SetVariableInt(TEXT("User.RayCount"), Distances.Num());
	Wave->SetVariableFloat(TEXT("User.StampHeight"), Row.bWallStamp ? Row.StampHeight : 0.f);
	Wave->SetVariableFloat(TEXT("User.StampDensity"), Row.bWallStamp ? Row.StampDensity : 0.f);
	// `User.Distances`(Float Array)는 지금 에셋에 없다 — 링은 나이아가라 CPU Collision(CL9_Occlusion)으로 스스로 벽에 멈춘다
	// (2026-09-16 리드). 위의 C++ 트레이스는 진단(cl9.shownoise 광선 수)용이고, 배열 DI가 생기면 여기서 다시 잇는다.

	LiveWaves.Add(Wave);
}
