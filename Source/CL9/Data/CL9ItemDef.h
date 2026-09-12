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
/**
 * 손에 들었을 때 팔이 취하는 자세의 범주 (docs/systems/inventory.md 손에 든 아이템). 아이템마다
 * 클립을 따로 만들지 않고 범주로 묶는다 — 전신 ABP가 이 값으로 상체 Hold 레이어를 고르고,
 * 1인칭 팔은 전신 포즈를 복사해 같은 자세가 된다.
 */
UENUM(BlueprintType)
enum class ECL9HoldPose : uint8
{
	/** 자세를 바꾸지 않는다 — 손이 내려간 채 소켓에만 붙는다 */
	None,
	/** 한 손에 쥐는 작은 것 — 계란·고기·프라이팬. 손을 가슴 앞으로 올린다 */
	Small,
	/** 손전등처럼 앞을 향해 쥐는 것 — 렌즈(+X)가 시선 방향 */
	Flashlight,
	/** 두 손으로 앞에 드는 큰 것 — 냄비·작업등 */
	Large
};

/**
 * 손에 든 채 좌클(IA_Use)하면 무엇을 하는가 (docs/systems/input.md 사용). 먹기는 여기 없다 —
 * 먹을 수 있는 건 `ItemType == Dish`뿐이고 그 행은 좌클이 곧 먹기다 (plan/data/item/common.md).
 */
UENUM(BlueprintType)
enum class ECL9UseAction : uint8
{
	/** 아무것도 안 한다 — 달걀·냄비·작업등 */
	None,
	/** 들고 있는 손전등을 켜고 끈다 (헤드램프는 F) */
	ToggleLight,
	/** 정면으로 휘둘러 몬스터를 친다 — HitDamage·HitStun이 결과를 정한다 */
	Swing
};

/** 휘둘러 맞았을 때 몬스터가 받는 경직의 종류 — 시간·거리 값은 몬스터 행이 갖는다 */
UENUM(BlueprintType)
enum class ECL9HitStun : uint8
{
	None,
	/** 죽지 않는 타격의 경직 — 몬스터 행 `HitStunMs` */
	Hit,
	/** 밀쳐냄 — 몬스터 행 `PushStun` 경직 + `PushKnockback` 넉백 + 범인 추격 (프라이팬) */
	Push
};

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

	/**
	 * 손에 들었을 때 손 소켓(`HandGrip_R`, 없으면 `hand_r` 뼈)에서의 상대 위치 (cm). 손잡이가 메시
	 * 원점에 있지 않은 물건(손전등 몸통, 냄비 고리)을 손바닥에 맞추는 값이다 — 2026-09-12부터
	 * 든 아이템은 캡슐이 아니라 소지자 전신 메시의 손 소켓에 붙는다 (docs/systems/inventory.md).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	FVector GripOffset = FVector::ZeroVector;

	/** 손 소켓에서의 상대 회전 — 손전등처럼 렌즈(+X)가 앞을 봐야 하는 물건이 쓴다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	FRotator GripRotation = FRotator::ZeroRotator;

	/** 들었을 때 팔 자세 범주 — 전신 ABP의 상체 Hold 레이어가 읽는다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	ECL9HoldPose HoldPose = ECL9HoldPose::Small;

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

	// -- 빛 --
	// 빛을 내느냐는 행이 아니라 클래스가 정한다: ActorClass가 ACL9LightItem 계열이면 빛난다
	// (item - lightitem - flashitem, 2026-09-11). 아래 값은 그 클래스가 읽는 광원 수치다.

	/** 소지 중 발광 반경 / 손전등 도달 거리 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light")
	float LightRadius = 0.f;

	/** 밝기 — 포인트 라이트는 칸델라, 손전등은 빔 강도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light")
	float LightIntensity = 5000.f;

	/** 손전등 외곽 원뿔의 반각 (도) — 발광(점광원)에서는 무시된다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light")
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

	/**
	 * 먹을 때 걸리는 버프들의 별칭 — Dish 종류만 먹을 수 있다 (종류가 이를 제한한다).
	 * 효과 값은 여기 적지 않고 버프 표가 갖는다 (docs/plan/data/buff.md). 여기 적힌
	 * 별칭이 버프 표에 없으면 로드 검사가 깨진 행으로 잡는다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Type")
	TArray<FName> EatBuffs;

	// -- 사용 (좌클) --

	/** 든 채 좌클하면 하는 일. Dish 행은 이 값과 무관하게 먹는다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Use")
	ECL9UseAction UseAction = ECL9UseAction::None;

	/** Swing으로 맞힌 몬스터의 HP 피해 — 0이면 피해 없음 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Use")
	int32 HitDamage = 0;

	/** Swing으로 맞힌 몬스터가 받는 경직 종류 — 피해로 죽으면 경직은 없다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Use")
	ECL9HitStun HitStun = ECL9HitStun::None;
};
