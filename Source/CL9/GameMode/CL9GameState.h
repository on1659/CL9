// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Data/CL9Types.h"
#include "CL9GameState.generated.h"

class UCL9RulesDef;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCL9NightPhaseChanged, ECL9NightPhase, NewPhase);

/** 제안되거나 수락된 주문이 바뀌면 모든 머신에서 발동한다 — 로비 화면과 HUD가 구독한다(docs/systems/order.md UI) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCL9OrdersChanged);

/** 판의 자금이 움직이면 모든 머신에서 발동한다 — HUD의 이더 표시가 구독한다(docs/systems/economy.md) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCL9MoneyChanged);

/**
 * 정산이 끝났다 — **통과든 미달이든** 모든 머신에서 발동한다 (docs/systems/balance.md).
 * 새벽 화면이 읽는 유일한 신호다. **상태를 되돌리기 전에** 보낸다: 통과하면 기간 수입이
 * 0으로 리셋되고 미달하면 1일차로 돌아가므로, 발동 뒤에는 이 숫자들을 어디서도 구할 수 없다.
 * `bPassed`가 false면 그 판은 그 자리에서 끝났다는 뜻이다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FCL9Settled, int32, Day, int32, Earned, int32, Target, bool, bPassed);

/**
 * 모든 머신이 어느 사이트 맵 위에 서 있는지를 나타낸다(docs/systems/map.md 생성 흐름). 전달되는
 * 것은 시드뿐이며, Build는 설정할 때마다 증가시켜 같은 시드가 두 번 깔릴 때도
 * 알림이 발동하게 한다(처음 합류할 때는 시드 0이 기본값과 같다).
 */
USTRUCT()
struct FCL9SiteMapKey
{
	GENERATED_BODY()

	/** 0 = 고정된 껍데기만(로비), 그 외에는 오늘 밤의 미로 */
	UPROPERTY()
	uint32 Seed = 0;

	UPROPERTY()
	int32 Build = 0;
};

/**
 * 공유되는 런타임 상태다(conventions § 4·5) — 정적 테이블은 절대 리플리케이트되지
 * 않고, 밤 동안 변하는 것만 그렇다. 전환 판정은 게임 모드에 있고,
 * 이 클래스는 그 결과만 나른다.
 */
UCLASS()
class ACL9GameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 호스트 전용: 항아리 반납 하나를 기록한다 — 밤 동안이면 정전 시각도 앞당긴다(docs/plan/data/rules.md 밤) */
	void RecordReturn(FName ItemAlias, FName ItemSubAlias);

	/** 호스트 전용: 페이즈를 바꾸고 로컬 변경 훅을 발동한다(클라이언트는 OnRep을 통해 발동한다) */
	void SetPhase(ECL9NightPhase NewPhase);

	/**
	 * 호스트 전용: 밤 시계를 가동한다 — 시작/종료/정전을 서버 시각으로 찍는다.
	 * NightLength는 날짜로 보정된 길이이며, bNoBlackout이면 정전을 걸어두지 않는다
	 * (docs/plan/data/progression.md 특정 일자 덮어쓰기).
	 */
	void StartNightClock(const UCL9RulesDef& Rules, float NightLength, bool bNoBlackout);

	/** 호스트 전용: 시계를 0으로 되돌린다(로비로 돌아왔을 때) */
	void ClearNightClock();

	/** 호스트 전용: 다음 날로 — 밤 사이의 로비가 새 날짜 번호를 넘겨받는다 */
	void AdvanceDay();

	/** 호스트 전용: 특정 날짜로 건너뛴다(cl9.day — 밤을 끝까지 플레이하지 않고 곡선을 확인할 때) */
	void SetDay(int32 NewDay);

	/** 호스트 전용: 이번 런이 습격할 목적지와 오늘 밤 위험도 배율을 설정한다(site × curve — docs/systems/balance.md) */
	void SetSiteAndDanger(FName SiteAlias, float NewDangerScale);

	/** 오늘 밤의 추격 예산, 탐지 속도, 시야에 걸리는 배율이다 — 밤이 아니면 1(docs/systems/monster.md 일자 스케일링) */
	UFUNCTION(BlueprintPure, Category="Night")
	float GetDangerScale() const { return DangerScale; }

	/**
	 * 호스트 전용: 시드를 이용해 모든 머신에 현재 사이트의 맵을 깐다 —
	 * 0이면 고정된 껍데기만 깐다. 사이트 행은 CurrentSiteAlias에서 오므로
	 * 그것부터 먼저 설정해야 한다(docs/systems/map.md 생성 흐름).
	 */
	void SetSiteMap(uint32 Seed);

	/** 오늘 밤의 맵 시드 — 밤이 아니면 0 */
	uint32 GetMapSeed() const { return SiteMapKey.Seed; }

	/** 호스트 전용: 로비의 추첨 결과 — N개의 후보와 그중 몇 개를 선택해야 하는지(docs/systems/order.md 주문 흐름) */
	void SetOfferedOrders(const TArray<FName>& Aliases, int32 Required);

	/** 호스트 전용: 확정된 선택이 offer를 대체한다; 수락된 주문은 납품되거나 만료될 때까지(4e) 여러 날에 걸쳐 쌓인다 */
	void SetAcceptedOrders(const TArray<FCL9AcceptedOrder>& Orders);

	/** 로컬 표현용 훅 — 주문 화면과 HUD가 구독한다(모든 머신에서 발동) */
	UPROPERTY(BlueprintAssignable, Category="Orders")
	FCL9OrdersChanged OnOrdersChanged;

	/** 오늘 밤의 후보들을 alias로 나열한다 — 선택이 끝나거나 로비가 아니면 비어 있다 */
	UPROPERTY(ReplicatedUsing=OnRep_Orders, BlueprintReadOnly, Category="Orders")
	TArray<FName> OfferedOrders;

	/** offer 중 몇 개를 선택해야 하는지(M) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Orders")
	int32 OrdersRequired = 0;

	/** 이번 런이 진행 중인 주문들과 그 마감일 — 런 진행 상황이며 저장된다(docs/systems/save.md) */
	UPROPERTY(ReplicatedUsing=OnRep_Orders, BlueprintReadOnly, Category="Orders")
	TArray<FCL9AcceptedOrder> AcceptedOrders;

	/**
	 * 호스트 전용: 벌어들인 이더 — 납품 보상과 판매가 여기로 들어온다. 누적 자금과
	 * 이번 정산 기간의 수입이 함께 오른다(docs/systems/economy.md 이더의 흐름).
	 */
	void AddIncome(int32 Amount);

	/**
	 * 호스트 전용: 나가는 이더 — 위약금과 장비 구매가 여기로 나간다. 채무는 없다,
	 * 잔액은 0 아래로 내려가지 않는다. 실제로 빠져나간 액수를 반환한다.
	 */
	int32 Deduct(int32 Amount);

	/** 호스트 전용: 정산이 끝나 다음 기간이 시작된다 — 누적 자금은 그대로 두고 기간 수입만 0으로 */
	void ResetPeriodIncome();

	/**
	 * 호스트 전용: 지난 정산 결과를 지운다 — **밤이 시작될 때** 부른다.
	 * 그래야 `LastSettlement.Day != 0`이 "이번 새벽에 정산이 있었다"와 정확히 같은 뜻이 된다.
	 * 날짜로 대조하면 미달 직후 새 판이 1일차로 되돌아가 결과를 잃는다.
	 */
	void ClearLastSettlement();

	/** 호스트 전용: 정산 결과를 전원에게 알린다 — 상태를 바꾸지는 않고 신호만 보낸다 */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Settled(int32 Day, int32 Earned, int32 Target, bool bPassed);

	/** 로컬 표현용 훅 — 새벽 화면이 구독한다(모든 머신에서 발동) */
	UPROPERTY(BlueprintAssignable, Category="Economy")
	FCL9Settled OnSettled;

	/**
	 * 마지막 정산 결과 — 신호와 같은 값을 복제로도 들고 있다. **새벽 화면이 신호를 놓쳐도
	 * 여기서 읽으면 된다**(클라이언트에서 화면 열기 RPC와 정산 멀티캐스트의 도착 순서는 보장되지 않는다).
	 * **`Day != 0`은 "이번 새벽에 정산이 있었다"는 뜻이다** — 밤이 시작될 때 지워지므로 날짜를
	 * 대조할 필요가 없다(미달 직후에는 판이 1일차로 되돌아가 날짜 대조가 어긋난다).
	 */
	UFUNCTION(BlueprintPure, Category="Economy")
	const FCL9SettlementResult& GetLastSettlement() const
	{
		return LastSettlement;
	}

	/** 판이 지금까지 모은 이더 — 플레이어별 지갑이 아니라 판 하나의 자금이다 */
	UFUNCTION(BlueprintPure, Category="Economy")
	int32 GetSharedMoney() const { return SharedMoney; }

	/** 이번 정산 기간에 번 이더 — 정산 판정(⑥)이 읽을 값이다 */
	UFUNCTION(BlueprintPure, Category="Economy")
	int32 GetPeriodIncome() const { return PeriodIncome; }

	/** 로컬 표현용 훅 — HUD의 이더 표시가 구독한다(모든 머신에서 발동) */
	UPROPERTY(BlueprintAssignable, Category="Economy")
	FCL9MoneyChanged OnMoneyChanged;

	/**
	 * 지금 이 장비를 살 수 있는지, 못 산다면 왜인지 (docs/systems/shop.md).
	 * **판정이 여기 있는 이유는 게임 모드가 클라이언트에 없기 때문이다** — 구매 화면은
	 * 모든 머신에서 뜨므로, 호스트의 판정과 화면의 판정이 같은 함수 하나를 읽어야
	 * 갈라지지 않는다. 호스트의 실제 구매도 이 함수를 다시 돌린 뒤에 값을 치른다.
	 */
	UFUNCTION(BlueprintPure, Category="Shop")
	ECL9BuyResult CanBuy(FName EquipAlias) const;

	/** 월드에 나와 있는 이 장비의 실물 수 — 클라이언트에서도 복제된 실물을 센다 */
	UFUNCTION(BlueprintPure, Category="Shop")
	int32 GetOwnedCount(FName EquipAlias) const;

	/**
	 * 호스트 전용: 저장본에서 판을 되돌린다 — 날짜·자금·진행 중 주문을 한 번에 얹는다
	 * (docs/systems/save.md). 저장 단위가 "날"이라 로비에서만 부른다.
	 */
	void RestoreRun(int32 Day, int32 Money, int32 Income, const TArray<FCL9AcceptedOrder>& Orders);

	/** 습격 중인 목적지(docs/plan/data/site.md) — 4g가 선택지를 추가하기 전까지는 게임 모드의 기본값이다 */
	UFUNCTION(BlueprintPure, Category="Night")
	FName GetCurrentSiteAlias() const { return CurrentSiteAlias; }

	/** 호스트 전용: 정전을 지금 시각으로 당긴다(cl9.blackout — 시계를 다 기다리지 않고 검증할 때) */
	void ForceBlackout();

	/** 밤이 끝날 때까지 남은 초 — 밤이 아니면 0 */
	UFUNCTION(BlueprintPure, Category="Night")
	float GetNightRemaining() const;

	/** 오늘 밤 정전 시각이 지나면 true */
	UFUNCTION(BlueprintPure, Category="Night")
	bool IsBlackout() const;

	UFUNCTION(BlueprintPure, Category="Night")
	ECL9NightPhase GetPhase() const { return Phase; }

	UFUNCTION(BlueprintPure, Category="Night")
	int32 GetCurrentDay() const { return CurrentDay; }

	/** 로컬 표현용 훅 — 조명/UI가 여기를 구독한다(모든 머신에서 발동) */
	UPROPERTY(BlueprintAssignable, Category="Night")
	FCL9NightPhaseChanged OnNightPhaseChanged;

	/** 오늘 밤 항아리에 반납된 모든 것, 반납 순서대로 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Night")
	TArray<FCL9CarriedItem> ReturnedItems;

protected:
	UFUNCTION()
	void OnRep_Phase();

	UFUNCTION()
	void OnRep_Orders();

	UFUNCTION()
	void OnRep_SiteMap();

	UFUNCTION()
	void OnRep_Money();

	/** 로컬: 현재 사이트 행과 시드로 사이트 맵을 다시 만든다 — 시드가 0이면 껍데기만 */
	void RebuildSiteMap();

	UPROPERTY(ReplicatedUsing=OnRep_Phase, BlueprintReadOnly, Category="Night")
	ECL9NightPhase Phase = ECL9NightPhase::Lobby;

	/** 모든 머신이 로컬에서 만드는 맵 — 벽 그 자체는 절대 아니다 */
	UPROPERTY(ReplicatedUsing=OnRep_SiteMap)
	FCL9SiteMapKey SiteMapKey;

	/** 판이 모은 누적 자금(에테르) — 세이브에 들어간다(docs/systems/save.md) */
	UPROPERTY(ReplicatedUsing=OnRep_Money, BlueprintReadOnly, Category="Economy")
	int32 SharedMoney = 0;

	/** 이번 정산 기간의 수입 — 정산 판정 대상이며, 누적 자금과 따로 센다 */
	UPROPERTY(ReplicatedUsing=OnRep_Money, BlueprintReadOnly, Category="Economy")
	int32 PeriodIncome = 0;

	/** 마지막 정산 결과 — 새벽 화면이 신호를 놓쳤을 때의 근거 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Economy")
	FCL9SettlementResult LastSettlement;

	/** 호스트 런의 날짜 — 1부터 시작한다(docs/systems/session.md) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Night")
	int32 CurrentDay = 1;

	/** 밤이 시작된 서버 시각(밤이 아니면 0) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Night")
	float NightStartServerTime = 0.f;

	/** 밤이 끝나는 서버 시각(밤이 아니면 0) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Night")
	float NightEndServerTime = 0.f;

	/** 정전이 발생하는 서버 시각 — 반납이 있을 때마다 앞당겨지되 DarkMin 아래로는 내려가지 않는다(밤이 아니면 0) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Night")
	float BlackoutServerTime = 0.f;

	/** 이번 런의 목적지 행 alias — 파생된 런타임 상태이며, 행 자체는 각 머신의 레지스트리에 남아 있다 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Night")
	FName CurrentSiteAlias;

	/** 오늘 밤의 위험도 배율(site BaseDanger × DangerCurve) — 밤이 아니면 1 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Night")
	float DangerScale = 1.f;

	const UCL9RulesDef* GetRules() const;
};
