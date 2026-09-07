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
 * 로컬에서 어둠을 그려내는 렌더러(build-order 5c, 앞당겨 구현 — docs/systems/light.md).
 * 순전히 표현 담당이다: 게임 스테이트의 밤 단계를 구독해서 밤이 되면 레벨의 태양/하늘을
 * 어둡게 만들고(자동 노출이 그 어둠을 되돌리지 못하도록 수동 노출 고정도 함께 건다).
 * 레벨마다 인스턴스 하나만 존재하며, 복제된 단계 값을 기준으로 모든 머신에서 똑같이
 * 동작한다 — 여기 있는 어떤 것도 권위 있는 판정이 아니다(몬스터의 시야 판정은 화면
 * 밝기가 아니라 값으로 이루어진다 — docs/systems/monster.md).
 *
 * 기준값은 BeginPlay 시점에 맵에 배치된 라이트의 원래 값에서 뽑아내고, 아래 단계별
 * 항목들은 그 기준값에 곱해지는 배율이라서 리빌드 없이 에디터에서 튜닝할 수 있다
 * (build-order 원칙 3).
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
	// -- 로비(밤과 밤 사이의 휴식 지점 — 완전한 낮이 아니라 어스름한 저녁) --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CL9 Night|Lobby", meta=(ClampMin="0.0"))
	float LobbySunScale = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CL9 Night|Lobby", meta=(ClampMin="0.0"))
	float LobbySkyScale = 0.5f;

	// -- 밤(습격 시간대 — 거의 암흑이라 손전등이 중요해진다 — docs/systems/light.md) --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CL9 Night|Night", meta=(ClampMin="0.0"))
	float NightSunScale = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CL9 Night|Night", meta=(ClampMin="0.0"))
	float NightSkyScale = 0.1f;

	/** 밤이 되면 태양 라이트가 옮겨가는 달빛 색조 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CL9 Night|Night")
	FLinearColor NightSunColor = FLinearColor(0.35f, 0.45f, 0.75f, 1.f);

	// -- 블랙아웃(오늘 밤의 블랙아웃 시점을 지난 뒤 — 손전등에 대한 카운터) --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CL9 Night|Blackout", meta=(ClampMin="0.0"))
	float BlackoutSunScale = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CL9 Night|Blackout", meta=(ClampMin="0.0"))
	float BlackoutSkyScale = 0.03f;

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

	/** 레벨의 태양/하늘을 찾아 원래 밝기 값을 낮 기준값으로 기록한다 */
	void ResolveLights();

	/** 단계 델리게이트를 바인딩하고 현재 단계를 즉시 적용한다(중간 참가에도 안전) */
	bool TryBindGameState();

	/** 단계(그리고 밤 중 블랙아웃 여부)로부터 목표 상태값을 채운다 */
	void SelectTarget(ECL9NightPhase Phase, bool bBlackout, float& OutSun, float& OutSky, FLinearColor& OutColor) const;

	/** 현재 보간된 값을 라이트 컴포넌트에 반영한다 */
	void ApplyToLights() const;

	TWeakObjectPtr<ADirectionalLight> Sun;
	TWeakObjectPtr<ASkyLight> Sky;
	TWeakObjectPtr<ACL9GameState> BoundGameState;

	/** BeginPlay 시점에 기록한 원래 밝기 값 — 배율들은 이 값에 곱해진다 */
	float BaselineSunIntensity = 0.f;
	float BaselineSkyIntensity = 0.f;
	FLinearColor BaselineSunColor = FLinearColor::White;

	/** 보간되고 있는 현재 상태값 */
	float CurrentSunScale = 1.f;
	float CurrentSkyScale = 1.f;
	FLinearColor CurrentSunColor = FLinearColor::White;

	bool bLightsReady = false;
};
