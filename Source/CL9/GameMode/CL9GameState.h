// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Data/CL9Types.h"
#include "Engine/NetSerialization.h"
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
 * 전멸 패널티 결과가 바뀌었다 — 모든 머신에서 발동한다(클라이언트는 OnRep, 호스트는 setter 가 직접).
 * 새벽 화면이 구독한다 (docs/goal/loop-7a-wipe-penalty.md).
 */
DECLARE_MULTICAST_DELEGATE(FCL9OnWipePenaltyChanged);
DECLARE_MULTICAST_DELEGATE(FCL9OnImposterResultChanged);
DECLARE_MULTICAST_DELEGATE(FCL9OnContributionsChanged);
DECLARE_MULTICAST_DELEGATE(FCL9OnQuotaPenaltyChanged);

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

	/**
	 * 호스트 전용: 소음 사건 하나를 알린다 — **소리가 나는 모든 곳이 지나는 단일 입구**다
	 * (docs/systems/wave.md 소음 이벤트). 자기 렌더러에 바로 그리고, 나머지 머신에는 방송한다.
	 *
	 * `NoiseInstigator`는 소리를 낸 액터다 — 플레이어가 냈으면 그 `ACL9PlayerState`, 몬스터가 냈으면 그
	 * `ACL9Monster`(이름이 `Instigator`가 아닌 것은 `AActor`의 같은 이름 멤버를 가리면 빌드가 실패하기
	 * 때문이다). `Radius`는 그 행동의 판정 반경(없으면 0)이며 **파동은 이 값을 쓰지 않는다**
	 * (각 머신이 표에서 다시 파생한다) — 몬스터 청각을 이 이벤트로 통일할 때 쓰려고 자리를 잡아 둔 인자다.
	 */
	void HostReportNoise(ECL9NoiseSource Source, const FVector& Location, float Radius, AActor* NoiseInstigator);

	/**
	 * 소음 사건 방송 — **Unreliable**이다: 발걸음 링 하나가 빠져도 다음 걸음이 오고, 신뢰 큐를
	 * 발걸음으로 채우면 줍기·문 열기 같은 판정 RPC가 늦어진다 (docs/systems/wave.md 복제).
	 * 반경은 싣지 않는다 — 각 머신이 같은 표에서 파생한다.
	 */
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_Noise(ECL9NoiseSource Source, FVector_NetQuantize Location, AActor* NoiseInstigator);

	/** 호스트 전용: 페이즈를 바꾸고 로컬 변경 훅을 발동한다(클라이언트는 OnRep을 통해 발동한다) */
	void SetPhase(ECL9NightPhase NewPhase);

	/**
	 * 호스트 전용: 밤 시계를 가동한다 — 시작/종료/정전을 서버 시각으로 찍는다.
	 * NightLength는 날짜로 보정된 길이이며, bNoBlackout이면 정전을 걸어두지 않는다
	 * (docs/plan/data/progression.md 특정 일자 덮어쓰기). DarkBase·DarkMin은 사이트 덮어쓰기가 반영된 값이다
	 * (docs/plan/data/site.md 밤 시계, 2026-10-03).
	 */
	void StartNightClock(float NightLength, float DarkBase, float DarkMin, bool bNoBlackout);

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


	/** 호스트 전용(7l): 판이 클리어로 끝났다 — 마지막 목적지를 별을 받고 마쳤다. 화면은 게임 오버와 같은 자리에서 파생한다 */
	void SetRunCleared(int32 Day, int32 BestDay);

	// -- 스테이지 별 (docs/goal/loop-7l-stage-stars.md) — 이번 판의 것, 판 저장본으로 레벨을 건넌다 --

	/** 목적지별 이번 판 최고 별 */
	const TArray<FCL9SiteStars>& GetStageStars() const { return StageStars; }

	/** 목적지 `Site` 의 이번 판 최고 별 — 정산한 적이 없으면 -1 */
	int32 GetSiteStars(FName Site) const;

	/** 호스트 전용: 목적지 `Site` 를 별 `Stars` 로 정산했다 — 더 높을 때만 바꾼다. 처음이면 항목을 만든다(별 0 이어도 열린 목적지) */
	void RecordSiteStars(FName Site, int32 Stars);

	/** 호스트 전용: 판 저장본에서 되돌리거나(빈 배열이면) 새 판으로 지운다 */
	void SetStageStars(const TArray<FCL9SiteStars>& Stars);

	/**
	 * 목적지 `Site` 가 열려 있나 — `Order`(사이트 표 순서) 의 첫 목적지이거나, 이번 판에 정산한 적이 있거나,
	 * 바로 앞 목적지가 `UnlockStars` 이상을 받았으면. `Order` 에 없으면 false
	 */
	bool IsStageOpen(FName Site, const TArray<FName>& Order, int32 UnlockStars) const;

	/** 호스트 전용: 이번 정산의 스테이지 결과를 지난 정산 복제본에 싣는다 — `Multicast_Settled` 보다 먼저 부른다 */
	void SetSettlementStage(int32 Stars, FName Site, FName OpenedSite, bool bCleared);


	/** 호스트 전용(7j): 이번 판이 트럭 도착으로 열린다 — 도착 시작 = 판 시작 시각. `StartNightClock` 뒤에 부른다 */
	void SetArrival();

	/** 트럭 도착 진행(0~1, 1 이상 = 끝) — 이번 판에 도착이 없거나 판이 아니면 −1 (docs/goal/loop-7j-round-start.md) */
	float GetArrivalAlpha() const;

	/**
	 * 도착 순번 — 도착을 열 때마다 하나 오른다(0 = 아직 한 번도 없음). 로컬 카메라가 같은 도착을 두 번 시작하지 않게 견준다.
	 * 레벨이 서자마자 여는 첫 판은 서버 시각이 0 이라 시각으로는 "없음"과 가를 수 없다
	 */
	int32 GetArrivalSeq() const { return ArrivalSeq; }



	/** 호스트 전용: 전멸 패널티 결과를 싣는다(지울 때는 빈 값) — 호스트에서는 `OnWipePenaltyChanged` 를 직접 발동한다 */
	void SetWipePenalty(const FCL9WipePenalty& Penalty);

	/** 지난 판이 전멸로 끝났을 때 깎인 이더 — 새벽 화면이 읽는다. 다음 판이 시작될 때 비워진다 */
	const FCL9WipePenalty& GetWipePenalty() const { return WipePenalty; }

	/** 로컬 표현용 훅 — 전멸 패널티 결과가 바뀌면(모든 머신) */
	FCL9OnWipePenaltyChanged OnWipePenaltyChanged;

	/** 호스트 전용: 판 끝 임포스터 판정 결과를 싣는다(판 시작에 빈 값으로 지운다) — 호스트에서는 `OnImposterResultChanged` 를 직접 발동한다 */
	void SetImposterResult(const FCL9ImposterResult& Result);

	/** 지난 판의 임포스터 결과 — 새벽 화면이 모두에게 공개한다(loop-7g). 판 동안에는 비어 있다 */
	const FCL9ImposterResult& GetImposterResult() const { return ImposterResult; }

	/** 로컬 표현용 훅 — 임포스터 결과가 바뀌면(모든 머신) */
	FCL9OnImposterResultChanged OnImposterResultChanged;

	/** 호스트 전용: 지난 판의 처치 기여 목록을 싣는다(판 시작에 빈 목록으로 지운다) — 호스트에서는 `OnContributionsChanged` 를 직접 발동한다 */
	void SetContributions(const TArray<FCL9Contribution>& InContributions);

	/** 지난 판의 처치 기여 — 새벽 화면이 한 줄씩 보인다(7h) */
	const TArray<FCL9Contribution>& GetContributions() const { return Contributions; }

	/** 로컬 표현용 훅 — 처치 기여 목록이 바뀌면(모든 머신) */
	FCL9OnContributionsChanged OnContributionsChanged;

	/** 호스트 전용: 정산 미달 패널티 결과를 싣는다(판 시작에 빈 값으로 지운다, 7i) — 호스트에서는 `OnQuotaPenaltyChanged` 를 직접 발동한다 */
	void SetQuotaPenalty(const FCL9WipePenalty& Penalty);

	/** 지난 정산이 미달로 깎은 코인 — 새벽(정산) 화면이 읽는다. 모양은 전멸 패널티와 같다(있었나 + 깎은 양) */
	const FCL9WipePenalty& GetQuotaPenalty() const { return QuotaPenalty; }

	/** 로컬 표현용 훅 — 정산 미달 패널티가 바뀌면(모든 머신) */
	FCL9OnQuotaPenaltyChanged OnQuotaPenaltyChanged;

protected:
	/** 지난 판의 전멸 패널티 — `SetWipePenalty` 로만 쓴다 */
	UPROPERTY(ReplicatedUsing=OnRep_WipePenalty)
	FCL9WipePenalty WipePenalty;

	UFUNCTION()
	void OnRep_WipePenalty();

	/** 지난 판의 임포스터 결과 — `SetImposterResult` 로만 쓴다 */
	UPROPERTY(ReplicatedUsing=OnRep_ImposterResult)
	FCL9ImposterResult ImposterResult;

	UFUNCTION()
	void OnRep_ImposterResult();

	/** 지난 판의 처치 기여 — `SetContributions` 로만 쓴다 */
	UPROPERTY(ReplicatedUsing=OnRep_Contributions)
	TArray<FCL9Contribution> Contributions;

	UFUNCTION()
	void OnRep_Contributions();

	/** 지난 정산의 미달 패널티 — `SetQuotaPenalty` 로만 쓴다 */
	UPROPERTY(ReplicatedUsing=OnRep_QuotaPenalty)
	FCL9WipePenalty QuotaPenalty;

	UFUNCTION()
	void OnRep_QuotaPenalty();

	/** 목적지별 이번 판 최고 별(7l) — 목적지 판의 프롬프트가 모든 머신에서 읽는다 */
	UPROPERTY(Replicated)
	TArray<FCL9SiteStars> StageStars;

	UFUNCTION()
	void OnRep_Phase();

	UFUNCTION()
	void OnRep_Orders();

	UFUNCTION()
	void OnRep_SiteMap();

	UFUNCTION()
	void OnRep_Money();

	/** 로컬: 현재 사이트 행과 시드로 사이트 맵을 다시 만든다(구역이 있으면 구역 전부) — 시드가 0이면 껍데기만 */
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

	/** 트럭 도착 시작 서버 시각(7j) — `SetArrival` 로만 쓴다. 레벨의 첫 판이면 0 일 수 있다(그래서 아래 표시로 가른다) */
	UPROPERTY(Replicated)
	float ArrivalServerTime = 0.f;

	/** 이번 판이 도착으로 열렸나 — 판 시계를 열 때·걷을 때 지운다 */
	UPROPERTY(Replicated)
	bool bArrivalThisRound = false;

	/** `GetArrivalSeq` */
	UPROPERTY(Replicated)
	int32 ArrivalSeq = 0;

	/** 밤이 끝나는 서버 시각(밤이 아니면 0) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Night")
	float NightEndServerTime = 0.f;

	/** 정전이 발생하는 서버 시각 — 반납이 있을 때마다 앞당겨지되 DarkMin 아래로는 내려가지 않는다(밤이 아니면 0) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Night")
	float BlackoutServerTime = 0.f;

	/** 이번 밤의 DarkMin(사이트 덮어쓰기 반영) — 호스트 판정 전용이라 복제하지 않는다 */
	float NightDarkMin = 0.f;

	/** 이번 런의 목적지 행 alias — 파생된 런타임 상태이며, 행 자체는 각 머신의 레지스트리에 남아 있다 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Night")
	FName CurrentSiteAlias;

	/** 오늘 밤의 위험도 배율(site BaseDanger × DangerCurve) — 밤이 아니면 1 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Night")
	float DangerScale = 1.f;

	const UCL9RulesDef* GetRules() const;
};
