// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/CL9NightLighting.h"
#include "CL9.h"
#include "GameMode/CL9GameState.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/DirectionalLight.h"
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

	// Pin auto-exposure (min=max) so the dark scene can't adapt itself bright.
	// Set at runtime, not in the constructor, so a serialized map value can't
	// override it (docs/systems/light.md 렌더 세팅).
	if (ExposureControl != nullptr)
	{
		ExposureControl->Settings.bOverride_AutoExposureMinBrightness = true;
		ExposureControl->Settings.AutoExposureMinBrightness = ExposureLock;
		ExposureControl->Settings.bOverride_AutoExposureMaxBrightness = true;
		ExposureControl->Settings.AutoExposureMaxBrightness = ExposureLock;
	}

	ResolveLights();

	// The game state may not be replicated in yet — retry until it is.
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
	}
	else
	{
		UE_LOG(LogCL9, Warning, TEXT("NightLighting: no SkyLight in the level — sky stays as-is."));
	}

	CurrentSunScale = 1.f;
	CurrentSkyScale = 1.f;
	CurrentSunColor = BaselineSunColor;
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

	// Snap to the current phase so a mid-join looks right immediately.
	float Sun_ = 0.f, SkyV = 0.f;
	FLinearColor Color = BaselineSunColor;
	SelectTarget(CL9GameState->GetPhase(), CL9GameState->IsBlackout(), Sun_, SkyV, Color);
	CurrentSunScale = Sun_;
	CurrentSkyScale = SkyV;
	CurrentSunColor = Color;
	ApplyToLights();
	return true;
}

void ACL9NightLighting::OnNightPhaseChanged(ECL9NightPhase /*NewPhase*/)
{
	// Tick reads the phase directly; this only guarantees an immediate reaction.
}

void ACL9NightLighting::SelectTarget(ECL9NightPhase Phase, bool bBlackout, float& OutSun, float& OutSky, FLinearColor& OutColor) const
{
	switch (Phase)
	{
	case ECL9NightPhase::Night:
		OutSun = bBlackout ? BlackoutSunScale : NightSunScale;
		OutSky = bBlackout ? BlackoutSkyScale : NightSkyScale;
		OutColor = NightSunColor;
		break;

	case ECL9NightPhase::Dawn:
	case ECL9NightPhase::Lobby:
	default:
		OutSun = LobbySunScale;
		OutSky = LobbySkyScale;
		OutColor = BaselineSunColor;
		break;
	}
}

void ACL9NightLighting::ApplyToLights() const
{
	if (Sun.IsValid() && Sun->GetLightComponent() != nullptr)
	{
		Sun->GetLightComponent()->SetIntensity(BaselineSunIntensity * CurrentSunScale);
		Sun->GetLightComponent()->SetLightColor(CurrentSunColor);
	}
	if (Sky.IsValid() && Sky->GetLightComponent() != nullptr)
	{
		Sky->GetLightComponent()->SetIntensity(BaselineSkyIntensity * CurrentSkyScale);
	}
}

void ACL9NightLighting::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bLightsReady || !BoundGameState.IsValid())
	{
		return;
	}

	float TargetSun = 0.f, TargetSky = 0.f;
	FLinearColor TargetColor = BaselineSunColor;
	SelectTarget(BoundGameState->GetPhase(), BoundGameState->IsBlackout(), TargetSun, TargetSky, TargetColor);

	if (TransitionSpeed <= 0.f)
	{
		CurrentSunScale = TargetSun;
		CurrentSkyScale = TargetSky;
		CurrentSunColor = TargetColor;
	}
	else
	{
		CurrentSunScale = FMath::FInterpTo(CurrentSunScale, TargetSun, DeltaSeconds, TransitionSpeed);
		CurrentSkyScale = FMath::FInterpTo(CurrentSkyScale, TargetSky, DeltaSeconds, TransitionSpeed);
		CurrentSunColor = FMath::CInterpTo(CurrentSunColor, TargetColor, DeltaSeconds, TransitionSpeed);
	}

	ApplyToLights();
}
