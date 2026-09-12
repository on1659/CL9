// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/CL9NightLighting.h"
#include "CL9.h"
#include "Data/CL9DefRegistry.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameState.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"

ACL9NightLighting::ACL9NightLighting()
{
	PrimaryActorTick.bCanEverTick = true;

	ExposureControl = CreateDefaultSubobject<UPostProcessComponent>(TEXT("ExposureControl"));
	SetRootComponent(ExposureControl);
	ExposureControl->bUnbound = true;
	ExposureControl->Priority = 100.f;
}

void ACL9NightLighting::BeginPlay()
{
	Super::BeginPlay();

	// 자동 노출을 고정한다(min=max) — 어두운 씬이 스스로 밝게 적응해버리지 못하게 막는다.
	// 생성자가 아니라 런타임에 설정해서, 맵에 직렬화된 값이 이를 덮어쓰지 못하게 한다
	// (docs/systems/light.md 렌더 세팅).
	if (ExposureControl != nullptr)
	{
		ExposureControl->Settings.bOverride_AutoExposureMinBrightness = true;
		ExposureControl->Settings.AutoExposureMinBrightness = ExposureLock;
		ExposureControl->Settings.bOverride_AutoExposureMaxBrightness = true;
		ExposureControl->Settings.AutoExposureMaxBrightness = ExposureLock;
	}

	ResolveLights();

	// 게임 스테이트가 아직 복제되지 않았을 수 있다 — 될 때까지 재시도한다.
	if (!TryBindGameState())
	{
		FTimerHandle RetryHandle;
		GetWorldTimerManager().SetTimer(RetryHandle, [this]()
		{
			TryBindGameState();
		}, 0.25f, true);
	}
}

void ACL9NightLighting::ResolveLights()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		Sun = *It;
		break;
	}
	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		Sky = *It;
		break;
	}
	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
	{
		Fog = *It;
		break;
	}

	if (Sun.IsValid() && Sun->GetLightComponent() != nullptr)
	{
		BaselineSunIntensity = Sun->GetLightComponent()->Intensity;
		BaselineSunColor = Sun->GetLightComponent()->GetLightColor();
	}
	else
	{
		UE_LOG(LogCL9, Warning, TEXT("NightLighting: no DirectionalLight in the level — sun stays as-is."));
	}

	if (Sky.IsValid() && Sky->GetLightComponent() != nullptr)
	{
		BaselineSkyIntensity = Sky->GetLightComponent()->Intensity;
		BaselineSkyColor = Sky->GetLightComponent()->GetLightColor();
	}
	else
	{
		UE_LOG(LogCL9, Warning, TEXT("NightLighting: no SkyLight in the level — sky stays as-is."));
	}

	// 안개는 없어도 된다 — 표의 안개 값이 갈 곳이 없을 뿐이다. 볼류메트릭 여부 같은
	// 나머지 세팅은 레벨에 놓인 액터의 것을 그대로 쓴다.
	if (!Fog.IsValid())
	{
		UE_LOG(LogCL9, Log, TEXT("NightLighting: no ExponentialHeightFog in the level — the site's fog settings have nowhere to go."));
	}

	Current = FCL9LightingPhase();
	bLightsReady = true;
}

bool ACL9NightLighting::TryBindGameState()
{
	if (BoundGameState.IsValid())
	{
		return true;
	}

	ACL9GameState* CL9GameState = GetWorld() ? GetWorld()->GetGameState<ACL9GameState>() : nullptr;
	if (CL9GameState == nullptr)
	{
		return false;
	}

	BoundGameState = CL9GameState;
	CL9GameState->OnNightPhaseChanged.AddDynamic(this, &ACL9NightLighting::OnNightPhaseChanged);

	// 현재 단계로 즉시 맞춰서 중간 참가자에게도 곧바로 올바르게 보이도록 한다.
	Current = SelectTarget(CL9GameState->GetPhase(), CL9GameState->IsBlackout());
	ApplyToLights();
	return true;
}

void ACL9NightLighting::OnNightPhaseChanged(ECL9NightPhase /*NewPhase*/)
{
	// Tick이 단계를 직접 읽어가므로, 이 함수는 그저 즉각적인 반응을 보장할 뿐이다.
}

FCL9LightingPhase ACL9NightLighting::SelectTarget(ECL9NightPhase Phase, bool bBlackout) const
{
	// 사이트 행은 각 머신의 레지스트리에 있고, 어느 행인지는 게임 스테이트가 복제한다
	// (conventions § 정적 표는 로컬, 런타임 상태만 복제).
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9SiteDef* Site = (Registry != nullptr && BoundGameState.IsValid()) ? Registry->FindSite(BoundGameState->GetCurrentSiteAlias()) : nullptr;
	if (Site == nullptr)
	{
		// 별칭이 아직 비어 있는 것은 사이트가 정해지기 전(로비 진입 직후)이라 정상이다 —
		// 이름이 있는데 행이 없을 때만 경고한다.
		const FName Alias = BoundGameState.IsValid() ? BoundGameState->GetCurrentSiteAlias() : NAME_None;
		if (!Alias.IsNone() && !bWarnedNoSite)
		{
			bWarnedNoSite = true;
			UE_LOG(LogCL9, Warning, TEXT("NightLighting: no site row for '%s' — lights stay at their authored values."), *Alias.ToString());
		}
		return FCL9LightingPhase();
	}

	switch (Phase)
	{
	case ECL9NightPhase::Night:
		return bBlackout ? Site->BlackoutLighting : Site->NightLighting;

	case ECL9NightPhase::Dawn:
	case ECL9NightPhase::Lobby:
	default:
		return Site->LobbyLighting;
	}
}

void ACL9NightLighting::ApplyToLights() const
{
	if (Sun.IsValid() && Sun->GetLightComponent() != nullptr)
	{
		Sun->GetLightComponent()->SetIntensity(BaselineSunIntensity * Current.SunScale);
		Sun->GetLightComponent()->SetLightColor(BaselineSunColor * Current.SunColor);
	}
	if (Sky.IsValid() && Sky->GetLightComponent() != nullptr)
	{
		Sky->GetLightComponent()->SetIntensity(BaselineSkyIntensity * Current.SkyScale);
		Sky->GetLightComponent()->SetLightColor(BaselineSkyColor * Current.SkyColor);
	}
	if (Fog.IsValid() && Fog->GetComponent() != nullptr)
	{
		Fog->GetComponent()->SetFogDensity(Current.FogDensity);
		Fog->GetComponent()->SetFogInscatteringColor(Current.FogColor);
	}
	// 루멘이 닫힌 방을 완전한 검정으로 만드는 것을 표의 비율만큼 풀어 준다 — 노출 고정과
	// 같은 포스트프로세스 컴포넌트에 얹는다.
	if (ExposureControl != nullptr)
	{
		ExposureControl->Settings.bOverride_LumenSkylightLeaking = true;
		ExposureControl->Settings.LumenSkylightLeaking = Current.SkyLeak;
	}
}

void ACL9NightLighting::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bLightsReady || !BoundGameState.IsValid())
	{
		return;
	}

	const FCL9LightingPhase Target = SelectTarget(BoundGameState->GetPhase(), BoundGameState->IsBlackout());

	if (TransitionSpeed <= 0.f)
	{
		Current = Target;
	}
	else
	{
		Current.SunScale = FMath::FInterpTo(Current.SunScale, Target.SunScale, DeltaSeconds, TransitionSpeed);
		Current.SunColor = FMath::CInterpTo(Current.SunColor, Target.SunColor, DeltaSeconds, TransitionSpeed);
		Current.SkyScale = FMath::FInterpTo(Current.SkyScale, Target.SkyScale, DeltaSeconds, TransitionSpeed);
		Current.SkyColor = FMath::CInterpTo(Current.SkyColor, Target.SkyColor, DeltaSeconds, TransitionSpeed);
		Current.SkyLeak = FMath::FInterpTo(Current.SkyLeak, Target.SkyLeak, DeltaSeconds, TransitionSpeed);
		Current.FogDensity = FMath::FInterpTo(Current.FogDensity, Target.FogDensity, DeltaSeconds, TransitionSpeed);
		Current.FogColor = FMath::CInterpTo(Current.FogColor, Target.FogColor, DeltaSeconds, TransitionSpeed);
	}

	ApplyToLights();
}
