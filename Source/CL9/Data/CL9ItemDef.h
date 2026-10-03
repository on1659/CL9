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
 * 1인칭 팔 ABP(카메라 자식 뷰모델, 몸 포즈 복사 없음 — 2026-09-13)는 같은 값으로 카메라 로컬
 * Hold 클립을 고른다. 두 벌은 서로 다른 클립이고 범주만 공유한다.
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
 * 먹을 수 있는 건 `ItemType == Dish`뿐이고 그건 우클 길게(`UseHoldTime`)다 (plan/data/item/common.md, 2026-09-13).
 */
UENUM(BlueprintType)
enum class ECL9UseAction : uint8
{
	/** 아무것도 안 한다 — 달걀·냄비·작업등 */
	None,
	/** 들고 있는 손전등을 누르는 순간 켜고 끈다 (헤드램프는 F) */
	ToggleLight,
	/** 누르면 공격 대기, 떼면 정면으로 휘둘러 몬스터를 친다 — HitDamage·HitStun이 결과를 정한다(대기 시간 무관) */
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

	/**
	 * 처음 주웠을 때 조작 힌트("[좌클] 살짝 누르기 — …")를 띄울 종류인가 — 좌클에 반응하거나(`UseAction`) 먹을 수
	 * 있는(`Dish`) 것만. 달걀처럼 들고 놓는 게 전부인 것에는 설명할 게 없다. 본 적 있음은 `UCL9GameUserSettings`가
	 * `Alias`별로 기억한다 (2026-09-13 사용자: "설명이 필요해 보이는 것만, 종류별 한 번").
	 */
	bool HasUseHint() const;

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

	/** 들었을 때 팔 자세 범주 — 전신 ABP의 상체 Hold 레이어와 1인칭 팔 ABP가 각자의 클립을 고르는 데 읽는다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	ECL9HoldPose HoldPose = ECL9HoldPose::Small;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	bool bStackable = false;

	// -- 물리 --

	/**
	 * 놓인 아이템은 실제 호스트 물리를 탄다 — 구르고, 미끄러지고, 자연스럽게
	 * 멈춘다 (2026-09-03 결정). 꺼두면 단순 바운스 시뮬레이션으로 대체된다.
	 * 수레도 켠다 — bMeshIsBody로 메시가 몸체가 되므로 구가 마차째 굴러가던 문제가 없다 (2026-09-13).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	bool bPhysicsEnabled = true;

	/** 충돌 반지름 (cm) — bMeshIsBody 행은 판정에 쓰지 않는다(메시 콜리전이 몸체) */
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

	/**
	 * 물리 행 전용 각감쇠 (1/s) — 몸체의 각속도가 매초 이 비율로 줄어든다. 구 몸체는 마찰만으로는 구르기가 서지 않아
	 * 이 값이 구르기를 멈추게 한다 (2026-09-14 "무한히 회전"). 초안값, balance가 정한다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	float AngularDamping = 3.f;

	/** 물리 행 전용 선형 감쇠 (1/s) — 공기 저항처럼 속도를 매초 이 비율로 줄인다. 초안값, balance가 정한다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Physics")
	float LinearDamping = 0.3f;

	// -- 수레(물리 몸체) — bMeshIsBody가 켜진 행만 읽는다 (docs/plan/data/item/cart.md). 자리표시 기본값이고 값은 balance가 정한다 --

	/**
	 * 메시 콜리전(임포트한 UCX 심플 콜리전)이 곧 물리 몸체다 — 반경 구도, 구에 맞추는 자동 스케일도 쓰지 않고
	 * 로드 때 메시가 루트가 된다. 이런 물건은 손에 들리지 않는다(수레). 아래 열들은 이 값이 true일 때만 뜻이 있다.
	 * 메시에는 소켓 GripSocket(손잡이 중심)·CargoSocket(적재 판정 상자 중심)이 있어야 한다 — 로드 검사가 확인한다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cart")
	bool bMeshIsBody = false;

	/** 질량 (kg) — 0이면 엔진이 콜리전 부피로 계산한다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cart")
	float Mass = 0.f;

	/** 손잡이 스프링 (1/s²) — 손잡이 가속 = PushSpring × (목표점 − 손잡이)[cm]. 질량과 무관하다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cart")
	float PushSpring = 0.f;

	/** 손잡이 감쇠 (1/s) — 손잡이와 미는 사람의 **상대** 속도에 곱한다(같은 속도로 걸으면 처지지 않는다) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cart")
	float PushDamping = 0.f;

	/** yaw 스프링 (1/s²) — 밀리는 방향(손잡이→짐칸)이 미는 사람의 yaw를 향하도록, 오차(rad)에 비례한 각가속 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cart")
	float PushYawSpring = 0.f;

	/** yaw 감쇠 (1/s) — 수레의 yaw 각속도에 곱한다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cart")
	float PushYawDamping = 0.f;

	/** 미는 사람 캡슐 표면에서 손잡이 목표점까지 (cm) — 걷기 시작의 처짐(≈ 캐릭터 가속도 ÷ PushSpring)보다 커야 캡슐이 손잡이에 닿지 않는다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cart")
	float GripDistance = 0.f;

	/** 손잡이가 목표점에서 이보다 벌어지면(수평, 또는 미는 사람 캡슐 중심과의 높이 차) 손을 놓친다 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cart")
	float GripBreakDistance = 0.f;

	/** 소켓 CargoSocket을 중심으로 하는 적재 판정 상자의 반폭 (cm) — 이 상자와 겹치는 자유 상태 아이템이 곧 실린 것 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cart")
	FVector CargoExtent = FVector::ZeroVector;

	/** bMeshIsBody 메시가 갖춰야 하는 소켓 이름 — 손잡이 중심(`Grip`) / 적재 판정 상자 중심(`Cargo`) */
	static const FName GripSocket;
	static const FName CargoSocket;

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

	/** 볼류메트릭 안개 속 빔 후광 세기 (VolumetricScatteringIntensity, 1 = 엔진 기본) — 리썰식 안개가 조명 기본 (2026-09-17, light.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light", meta=(ClampMin="0.0"))
	float LightScattering = 1.f;

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

	/** 든 채 좌클을 누르면(그리고 떼면) 하는 일. 먹기는 이 값과 무관하게 Dish 행을 우클 길게 누르는 것이다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Use")
	ECL9UseAction UseAction = ECL9UseAction::None;

	/** Swing으로 맞힌 몬스터의 HP 피해 — 0이면 피해 없음 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Use")
	int32 HitDamage = 0;

	/** Swing으로 맞힌 몬스터가 받는 경직 종류 — 피해로 죽으면 경직은 없다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Use")
	ECL9HitStun HitStun = ECL9HitStun::None;

	/**
	 * Swing 으로 맞힌 몬스터가 밀리는 거리, cm 단위 — Stun 진입 뒤 `CL9Rules.HitStopTime` 을 기다렸다가 `HitKnockbackTime` 동안
	 * 등속으로 밀린다(`ACL9Monster::OnDamage`, docs/systems/monster.md 넉백). 죽이는 타격에는 없다(죽음 발사가 대신).
	 * **헤더 0 = 넉백 없음 = 옛 동작**. `UseAction != Swing` 인데 0 이 아니면 로드 오류. 값은 balance(docs/plan/data/item/common.md)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Use")
	float HitKnockback = 0.f;
};
