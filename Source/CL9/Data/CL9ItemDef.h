// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "Data/CL9Types.h"
#include "CL9ItemDef.generated.h"

/**
 * 주울 수 있는 아이템 한 행 (docs/plan/data/item/common.md). 공용 ACL9Item
 * 액터는 메시, 물리, 소지 규칙 등 모든 걸 여기서 읽으므로, 새 아이템은
 * 새 행일 뿐 새 C++ 코드가 아니다.
 */
UCLASS()
class UCL9ItemDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	// -- 소지 --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	ECL9CarrySlot CarrySlot = ECL9CarrySlot::Pocketable;

	/** 무게→속도 공식이 정해지면 이 유효 무게가 감속에 반영된다 (미결) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float Weight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	bool bStackable = false;

	// -- 물리 --

	/**
	 * 놓인 아이템은 실제 호스트 물리를 탄다 — 구르고, 미끄러지고, 자연스럽게
	 * 멈춘다 (2026-09-03 결정). 꺼두면 단순 바운스 시뮬레이션으로 대체된다
	 * (카트는 이 옵션을 꺼둔다 — 카트의 구형 바디가 마차 전체를 굴려버리기 때문이다).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	bool bPhysicsEnabled = true;

	/** 충돌 반지름 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	float Radius = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	bool bFragile = false;

	/** 멀쩡한 아이템에 금이 가게 하는 충돌 속도 (cm/s) — BreakSpeed보다 낮은 CrackedBreakSpeed보다도 낮아야 한다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	float CrackSpeed = 0.f;

	/** 멀쩡한 아이템을 깨뜨리는 충돌 속도 (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	float BreakSpeed = 0.f;

	/** 이미 금 간 아이템을 깨뜨리는 충돌 속도 (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	float CrackedBreakSpeed = 0.f;

	/** 낙하 가속도 (cm/s²) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	float Gravity = 1950.f;

	/** 바닥 반발 계수 0..1 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	float Restitution = 0.3f;

	/** 바닥에 닿을 때의 수평 감쇠 0..1 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	float Friction = 0.5f;

	// -- 소리와 빛 --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light")
	bool bEmitsLight = false;

	/** 은은한 발광이냐 켜고 끄는 손전등이냐 — bEmitsLight일 때만 읽는다 (docs/systems/light.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light", meta=(EditCondition="bEmitsLight"))
	ECL9ItemLight LightKind = ECL9ItemLight::Glow;

	/** 소지 중 발광 반경 / 손전등 도달 거리 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light")
	float LightRadius = 0.f;

	/** 밝기 — 포인트 라이트는 칸델라, 손전등은 빔 강도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light", meta=(EditCondition="bEmitsLight"))
	float LightIntensity = 5000.f;

	/** 손전등 외곽 원뿔의 반각 (도) — Glow에서는 무시된다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light", meta=(EditCondition="bEmitsLight"))
	float LightConeAngle = 32.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light")
	bool bLightLostInPocket = true;

	/** 정전 중 발광 중인 소지자가 보이는 거리 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light")
	float CarrierSeenRadius = 0.f;

	// -- 표시 --

	/** 전역 규칙이 아니라 종류별 값이다 (docs/systems/ui.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Display")
	bool bShowOnMinimap = false;

	// -- 가치 (에테르, 정수 — docs/plan/economy.md) --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Value")
	int32 ValueMin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Value")
	int32 ValueMax = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Value")
	bool bWorthlessIfBroken = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Value")
	bool bWorthlessIfBurnt = false;

	// -- 요리 (사슬로 이어지는 행들이 실제로 존재해야 한다 — 로드 검사가 확인한다) --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cooking")
	FCL9ItemRef CookResult;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cooking")
	FCL9ItemRef BurnResult;

	/** 익을 때까지 열을 받는 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cooking")
	float CookTime = 12.f;

	/** 탈 때까지 열을 받는 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cooking")
	float BurnTime = 24.f;

	/** 연기 기둥 경고가 뜨는 시점 — 연출용일 뿐 판정에는 절대 쓰이지 않는다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cooking")
	float BurnWarnTime = 6.f;

	// -- 종류와 섭취 --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Type")
	ECL9ItemType ItemType = ECL9ItemType::Ingredient;

	/** 먹을 때 적용된다 — Dish 종류만 먹을 수 있다 (종류가 이를 제한한다) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Type")
	TArray<FCL9StatEffect> EatEffects;
};
