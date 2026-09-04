// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/CL9Types.h"
#include "CL9NightLighting.generated.h"

class ADirectionalLight;
class ASkyLight;
class UPostProcessComponent;
class ACL9GameState;

/**
 * Local darkness renderer (build-order 5c, pulled forward — docs/systems/light.md).
 * Presentation only: subscribes to the game state's night phase and drives the
 * level's sun/sky down (and a manual exposure lock so auto-exposure can't undo
 * the dark) as the night falls. One instance lives in the level, so it runs on
 * every machine from the replicated phase — nothing here is authoritative
 * (monster sight is judged by value, not screen brightness — docs/systems/monster.md).
 *
 * Baselines are captured from the map's authored lights on BeginPlay; the
 * per-phase columns below are multipliers on those, so tuning stays in the
 * editor with no rebuild (build-order 원칙 3).
 */
UCLASS()
class ACL9NightLighting : public AActor
{
	GENERATED_BODY()

public:
	ACL9NightLighting();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	// -- Lobby (the stop between nights — a dim evening, not full day) --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CL9 Night|Lobby", meta=(ClampMin="0.0"))
	float LobbySunScale = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CL9 Night|Lobby", meta=(ClampMin="0.0"))
	float LobbySkyScale = 0.5f;

	// -- Night (the raid — near dark, so a flashlight matters — docs/systems/light.md) --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CL9 Night|Night", meta=(ClampMin="0.0"))
	float NightSunScale = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CL9 Night|Night", meta=(ClampMin="0.0"))
	float NightSkyScale = 0.1f;

	/** Moonlight tint the sun shifts to at night */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CL9 Night|Night")
	FLinearColor NightSunColor = FLinearColor(0.35f, 0.45f, 0.75f, 1.f);

	// -- Blackout (past the blackout moment this night — the flashlight's counter) --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CL9 Night|Blackout", meta=(ClampMin="0.0"))
	float BlackoutSunScale = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CL9 Night|Blackout", meta=(ClampMin="0.0"))
	float BlackoutSkyScale = 0.03f;

	/**
	 * Fixed exposure the scene is pinned at (auto-exposure min=max), so dimming
	 * the lights actually darkens the image instead of the eye adapting back up
	 * (docs/systems/light.md 렌더 세팅). 1.0 = neutral; lower darkens everything.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CL9 Night", meta=(ClampMin="0.0"))
	float ExposureLock = 1.f;

	/** How fast the look eases toward its target (per second — 0 snaps) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CL9 Night", meta=(ClampMin="0.0"))
	float TransitionSpeed = 1.25f;

	/** Global exposure lock — bUnbound so it needs no volume in the map */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CL9 Night")
	TObjectPtr<UPostProcessComponent> ExposureControl;

private:
	/** Delegate target — must be UFUNCTION for the dynamic multicast bind */
	UFUNCTION()
	void OnNightPhaseChanged(ECL9NightPhase NewPhase);

	/** Grabs the level's sun/sky and records their authored intensities as the day baseline */
	void ResolveLights();

	/** Binds the phase delegate and applies the current phase now (mid-join safe) */
	bool TryBindGameState();

	/** Fills the target look from the phase (and blackout during the night) */
	void SelectTarget(ECL9NightPhase Phase, bool bBlackout, float& OutSun, float& OutSky, FLinearColor& OutColor) const;

	/** Pushes the current interpolated values onto the light components */
	void ApplyToLights() const;

	TWeakObjectPtr<ADirectionalLight> Sun;
	TWeakObjectPtr<ASkyLight> Sky;
	TWeakObjectPtr<ACL9GameState> BoundGameState;

	/** Authored intensities captured on BeginPlay — the multipliers scale these */
	float BaselineSunIntensity = 0.f;
	float BaselineSkyIntensity = 0.f;
	FLinearColor BaselineSunColor = FLinearColor::White;

	/** Interpolated current look */
	float CurrentSunScale = 1.f;
	float CurrentSkyScale = 1.f;
	FLinearColor CurrentSunColor = FLinearColor::White;

	bool bLightsReady = false;
};
