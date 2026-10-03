// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/CL9SiteDef.h"
#include "Data/CL9Types.h"
#include "CL9NightLighting.generated.h"

class ADirectionalLight;
class AExponentialHeightFog;
class ASkyAtmosphere;
class ASkyLight;
class AStaticMeshActor;
class AVolumetricCloud;
class UMaterialInterface;
class UPostProcessComponent;
class ACL9GameState;

/**
 * 로컬에서 어둠을 그려내는 렌더러(build-order 5c, 앞당겨 구현 — docs/systems/light.md).
 * 순전히 표현 담당이다: 게임 스테이트의 밤 단계를 구독해서 밤이 되면 레벨의 태양/하늘/
 * 안개를 사이트 행의 조명 세팅으로 옮긴다(자동 노출이 그 어둠을 되돌리지 못하도록 수동
 * 노출 고정도 함께 건다). 레벨마다 인스턴스 하나만 존재하며, 복제된 단계 값을 기준으로
 * 모든 머신에서 똑같이 동작한다 — 여기 있는 어떤 것도 권위 있는 판정이 아니다(몬스터의
 * 시야 판정은 화면 밝기가 아니라 값으로 이루어진다 — docs/systems/monster.md).
 *
 * 페이즈별 값은 이 액터가 아니라 **사이트 표**(UCL9SiteDef::LobbyLighting/NightLighting/
 * BlackoutLighting)에 있다 — 어둠은 사이트의 성격이라서다 (2026-09-12). 기준값은 BeginPlay
 * 시점에 맵에 배치된 라이트의 원래 값에서 뽑아내고, 표의 배율이 그 위에 곱해지므로 표를
 * 고치는 데 리빌드가 필요 없다 (build-order 원칙 3).
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
	/**
	 * 씬을 고정시키는 노출값(자동 노출의 min=max로 설정) — 이렇게 해야 라이트를
	 * 어둡게 낮췄을 때 눈이 다시 적응해 밝아지는 대신 화면이 실제로 어두워진다
	 * (docs/systems/light.md 렌더 세팅). 1.0이 중립값이고, 낮출수록 전체가 어두워진다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CL9 Night", meta=(ClampMin="0.0"))
	float ExposureLock = 1.f;

	/** 목표값을 향해 얼마나 빠르게 보간해가는지(초당 값 — 0이면 즉시 전환) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CL9 Night", meta=(ClampMin="0.0"))
	float TransitionSpeed = 1.25f;

	/** 전역 노출 고정 — bUnbound라서 맵에 볼륨을 따로 둘 필요가 없다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CL9 Night")
	TObjectPtr<UPostProcessComponent> ExposureControl;

private:
	/** 델리게이트 대상 — 다이내믹 멀티캐스트 바인딩을 위해 UFUNCTION이어야 한다 */
	UFUNCTION()
	void OnNightPhaseChanged(ECL9NightPhase NewPhase);

	/** 레벨의 태양/하늘/안개를 찾아 원래 값을 낮 기준값으로 기록한다 */
	void ResolveLights();

	/** 단계 델리게이트를 바인딩하고 현재 단계를 즉시 적용한다(중간 참가에도 안전) */
	bool TryBindGameState();

	/** 현재 사이트 행의 페이즈 세팅을 고른다 — 사이트 행이 없으면 기준값 그대로(배율 1·안개 0) */
	FCL9LightingPhase SelectTarget(ECL9NightPhase Phase, bool bBlackout) const;

	/** 현재 보간된 값을 라이트·안개 컴포넌트에 반영한다 */
	void ApplyToLights() const;

	/** 보간하지 않는 것 — 하늘 재질·구름 표시 — 을 목표 페이즈로 바로 바꾼다(바뀔 때만) */
	void ApplyDiscrete(const FCL9LightingPhase& Target);

	TWeakObjectPtr<ADirectionalLight> Sun;
	TWeakObjectPtr<ASkyLight> Sky;
	TWeakObjectPtr<AExponentialHeightFog> Fog;
	/** 보이는 하늘 — 태양과 별개로 배율(SkyLuminance)이 곱해진다 (2026-09-16) */
	TWeakObjectPtr<ASkyAtmosphere> Atmosphere;
	/** 하늘 구 — 레벨의 `SM_SkySphere` 스태틱 메시. 페이즈별 SkyMaterial 이 여기 씌워진다 (2026-09-18) */
	TWeakObjectPtr<AStaticMeshActor> SkySphere;
	TWeakObjectPtr<AVolumetricCloud> Clouds;
	/** 레벨에 놓여 있던 하늘 재질 — SkyMaterial 이 빈 페이즈는 이것으로 돌아간다 */
	TWeakObjectPtr<UMaterialInterface> BaselineSkyMaterial;
	TWeakObjectPtr<UMaterialInterface> AppliedSkyMaterial;
	bool bAppliedClouds = true;
	TWeakObjectPtr<ACL9GameState> BoundGameState;

	/** BeginPlay 시점에 기록한 원래 값 — 표의 배율들은 이 값에 곱해진다 */
	float BaselineSunIntensity = 0.f;
	FLinearColor BaselineSunColor = FLinearColor::White;
	float BaselineSkyIntensity = 0.f;
	FLinearColor BaselineSkyColor = FLinearColor::White;
	FLinearColor BaselineSkyLuminance = FLinearColor::White;

	/** 보간되고 있는 현재 상태값 (사이트 행과 같은 단위 — 배율·색조·안개 절대값) */
	FCL9LightingPhase Current;

	bool bLightsReady = false;

	/** 사이트 행을 못 찾았다는 경고는 한 번만 */
	mutable bool bWarnedNoSite = false;
};
