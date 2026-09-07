// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CL9Types.generated.h"

/** 아이템을 어떻게 들고 다닐 수 있는지 (docs/plan/data/item/common.md 운반) */
UENUM(BlueprintType)
enum class ECL9CarrySlot : uint8
{
	/** 손으로만 — 주머니에는 절대 들어가지 않는다 */
	Hand,
	/** 주머니에 들어간다 (거기서는 빛을 잃지만 여전히 탐지된다) */
	Pocketable,
	/** 카트로만 — 손으로는 전혀 들 수 없다 */
	CartOnly,
	/** 머리에 착용 — 주우면 바로 착용되고, 손은 계속 자유롭다 (헤드램프, 2026-09-05) */
	Head
};

/** 발광 아이템이 빛나는 방식 (docs/systems/light.md 광원 목록) */
UENUM(BlueprintType)
enum class ECL9ItemLight : uint8
{
	/** 들고 있는 동안 전방향으로 발광 — 달걀. 정전 중에도 계속 켜져 있다 */
	Glow,
	/** 전방 스팟 조명, IA_Flashlight로 켜고 끈다. 정전이 오면 강제로 꺼진다 */
	Flashlight
};

/** 아이템 종류 — Dish 아이템만 먹을 수 있고, 별도 플래그는 없다 (docs/plan/data/item/common.md 타입과 먹기) */
UENUM(BlueprintType)
enum class ECL9ItemType : uint8
{
	Ingredient,
	Dish,
	/** 재료도 요리도 아닌, 사용 가능한 것들 (카트, 손전등) */
	Tool
};

/**
 * 먹기 효과가 변화시킬 수 있는 스탯 목록 (docs/plan/data/character.md 스탯 enum).
 * 캡슐/눈 높이는 제외한다 (기하학적 전제 조건이므로), HP도 제외한다
 * (푸드트럭 패드만이 유일한 회복 수단이다).
 */
UENUM(BlueprintType)
enum class ECL9Stat : uint8
{
	WalkSpeed,
	RunSpeed,
	CarryMult,
	CartMult,
	JumpVelocity,
	JumpGravity,
	StaminaDrainTime,
	StaminaRegenTime,
	StaminaMinResume
};

/**
 * 밤의 수명주기 단계 (docs/systems/session.md · docs/plan/loop.md). 모든 전환은
 * 호스트 게임 모드가 결정하며, 클라이언트는 리플리케이트된 값으로부터 표현만
 * 파생시킨다. 로딩 단계는 나중에 seamless travel과 함께 합류한다.
 */
UENUM(BlueprintType)
enum class ECL9NightPhase : uint8
{
	/** 밤과 밤 사이 트럭에서 대기 — 로비가 되는 정거장 */
	Lobby,
	/** 습격 — 밤 시계가 돌아가고, 시간이 다 되면 새벽 실패가 된다 */
	Night,
	/** 밤이 끝나고 다음 로비 사이의 새벽 화면 */
	Dawn
};

/** 사이트가 사용하는 맵 생성기 — 목적지 하나에 생성기 하나 (docs/plan/data/site.md ★) */
UENUM(BlueprintType)
enum class ECL9Generator : uint8
{
	/** 생성된 맵 없음 — 아무것도 없는 평지 레벨 */
	None,
	/** 우리 행이 있는 통로 × 열 격자, 지하실과 일방향 플랩 (docs/systems/map.md 양계장 구조) */
	Henhouse
};

/**
 * 월드 액터의 종류 — 각 액터 클래스의 생성자가 정하고(ACL9ActorBase::Kind), "무엇을 보여줄지"를
 * 표로 고르는 곳(스캔의 ScanKinds 등)이 읽는다 (2026-09-06 사용자).
 */
UENUM(BlueprintType)
enum class ECL9ActorKind : uint8
{
	None,
	Item,
	Monster
};

/**
 * 마지막 정산의 결과 (docs/systems/balance.md). 신호(`OnSettled`)와 **같은 값을 복제로도** 들고 있는다 —
 * 새벽 화면이 신호보다 늦게 열려도(클라이언트에서는 화면 열기 RPC와 정산 멀티캐스트의 도착 순서가
 * 보장되지 않는다) 보여줄 숫자를 잃지 않게 하려는 것이다. Day가 0이면 아직 정산한 적이 없다.
 */
USTRUCT(BlueprintType)
struct FCL9SettlementResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 Day = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Earned = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Target = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bPassed = false;
};

/**
 * 장비를 살 수 있는지, 못 산다면 왜인지 (docs/systems/shop.md 구매 흐름).
 * 화면이 버튼을 끄고 이유를 적을 때 읽는다 — 문구는 표현 계층에서 만든다.
 */
UENUM(BlueprintType)
enum class ECL9BuyResult : uint8
{
	Ok,
	/** 표에 없거나 깨진 행이다 */
	NoRow,
	/** 트럭 패널은 정차(로비) 중에만 연다 */
	NotInLobby,
	/** 아직 그 일차가 아니다 (UnlockDay) */
	Locked,
	/** 이미 MaxOwned만큼 갖고 있다 */
	MaxOwned,
	/** 잔액이 모자란다 — 채무는 없다 */
	NotEnoughMoney
};

/**
 * 아이템이 열원 위에서 어느 쪽으로 가는 중인지 (docs/systems/cooking.md). 열원은 조리대이며,
 * 표현(진행도 게이지)이 읽을 수 있게 아이템에 실려 복제된다 — 판정 자체는 호스트의 조리대가 한다.
 */
UENUM(BlueprintType)
enum class ECL9CookPhase : uint8
{
	/** 열원 위가 아니다 */
	None,
	/** 다음 전이가 "익음"이다 */
	Cooking,
	/** 이미 익었고 다음 전이는 "탐"이다 — 표현은 여기서 경고로 바뀐다 */
	Burning
};

/** 아이템 행과 상태를 가리킨다 — 요리 체인과 소지품 미러가 이를 사용한다 (docs/data/schema.md) */
USTRUCT(BlueprintType)
struct FCL9ItemRef
{
	GENERATED_BODY()

	/** 아이템 테이블에 반드시 존재해야 한다 — 로드 체크가 이를 검증한다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemAlias;

	/** 상태 변형 (예: 조리됨) — None이 기본 상태다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemSubAlias;

	bool IsSet() const
	{
		return !ItemAlias.IsNone();
	}
};

/** 손이나 주머니 미러에 있는 소지품 하나 (docs/systems/inventory.md 상태 모델) */
USTRUCT(BlueprintType)
struct FCL9CarriedItem : public FCL9ItemRef
{
	GENERATED_BODY()
};

/**
 * 납품 창구에 올려둔 물건 하나 — 개체별로 굴린 값을 그대로 들고 있어야
 * 창구에 놓아두는 사이에 판매가가 다시 굴려지지 않는다 (docs/systems/economy.md 가격식).
 */
USTRUCT(BlueprintType)
struct FCL9CounterItem : public FCL9ItemRef
{
	GENERATED_BODY()

	/** 올릴 때의 굴림값(에테르) — 배율은 판매 시점에 곱한다 */
	UPROPERTY(BlueprintReadOnly)
	int32 Value = 0;
};

/** 아이템 참조에 개수를 더한 것 — 주문 재료와 드롭이 이를 공유한다 (docs/data/schema.md) */
USTRUCT(BlueprintType)
struct FCL9ItemCount : public FCL9ItemRef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Count = 1;
};

/** 몬스터 드롭 항목 하나 — 수량은 Min..Max 사이에서 굴리고, 기본 Count는 여기서 쓰이지 않는다 */
USTRUCT(BlueprintType)
struct FDropItem : public FCL9ItemCount
{
	GENERATED_BODY()

	/** 0..1 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DropChance = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MinQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxQuantity = 1;
};

/**
 * 지금 캐릭터에 걸려 있는 먹기 효과 하나 (docs/plan/data/item/common.md 타입과 먹기).
 * **끝나는 시각을 서버 시각으로 들고 복제한다** — 넘어짐과 같은 방식이다. 그래야 서버와
 * 자율 프록시가 같은 보정값을 도출해 이동 예측이 어긋나지 않는다(docs/systems/character.md).
 */
USTRUCT()
struct FCL9ActiveEffect
{
	GENERATED_BODY()

	UPROPERTY()
	ECL9Stat Stat = ECL9Stat::WalkSpeed;

	/** 스탯에 **더하는** 값 — 배율이 아니다. 크기는 표가 정한다 */
	UPROPERTY()
	float Amount = 0.f;

	/** 이 시각(서버 시각)이 지나면 사라진다 */
	UPROPERTY()
	float EndServerTime = 0.f;
};

/** 먹기 효과 행 하나 — 대상, 양, 지속시간이 모두 테이블에서 온다 */
USTRUCT(BlueprintType)
struct FCL9StatEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ECL9Stat Stat = ECL9Stat::WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Amount = 0.f;

	/** 0이면 즉시 한 번 적용되고, 그보다 크면 그 초만큼 지속된다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DurationSec = 0.f;
};

/** 스폰 기준값 행 하나 — 사이트의 동물이나 재료이며, 일자 커브가 굴림값을 조정한다 (docs/data/schema.md) */
USTRUCT(BlueprintType)
struct FCL9SpawnEntry
{
	GENERATED_BODY()

	/** 몬스터 또는 아이템 별칭 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Alias;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MinCount = 0;

	/** 일자 커브가 Min과 Max 사이 굴림값에 곱해진다 (docs/plan/data/progression.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxCount = 0;
};

/** 특정 하루에 대해 손으로 지정하는 예외값 — 배율 0은 "커브를 사용하라"는 뜻이다 (docs/plan/data/progression.md 특정 일자 덮어쓰기) */
USTRUCT(BlueprintType)
struct FCL9DayOverride
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Day = 1;

	/** 0보다 크면 이 날의 MonsterCountCurve를 대체한다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MonsterCountMult = 0.f;

	/** 0보다 크면 이 날의 DangerCurve를 대체한다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DangerMult = 0.f;

	/** 이 밤에는 정전이 없다 (튜토리얼 날) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bNoBlackout = false;
};

/** 진행 중인 런에서 수락된 주문 하나 — 진행 상황이 리플리케이트되고 런과 함께 저장된다 (docs/systems/order.md 주문 흐름) */
USTRUCT(BlueprintType)
struct FCL9AcceptedOrder
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName OrderAlias;

	UPROPERTY(BlueprintReadOnly)
	int32 AcceptedDay = 0;

	/** 아직 납품 가능한 마지막 날 — AcceptedDay + DeadlineDays − 1 */
	UPROPERTY(BlueprintReadOnly)
	int32 DeadlineDay = 0;

	/**
	 * 수락할 때 [RewardMin, RewardMax]에서 굴린 이 주문의 확정 보상(에테르).
	 * 카드가 범위 대신 실제로 받을 값을 보여주고, 마감을 놓쳤을 때의 위약금도
	 * 이 값에서 나온다 (docs/systems/order.md 주문 흐름).
	 */
	UPROPERTY(BlueprintReadOnly)
	int32 Reward = 0;
};

namespace CL9
{
	/** 충격 상태 체인 — 성함 → 금감 → 깨짐 순으로 SubAlias를 바꾼다 (docs/systems/item.md 상태 = SubAlias) */
	inline const FName SubAliasCracked = FName("cracked");
	inline const FName SubAliasBroken = FName("broken");
}
