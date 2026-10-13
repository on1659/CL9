// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Data/CL9Types.h"
#include "CL9GameMode.generated.h"

class ACL9Character;
class ACL9Counter;
class ACL9Item;
class ACL9Jar;
class ACL9Monster;
class ACL9PlayerController;
class ACL9PlayerState;
class UCL9EquipmentDef;
class UCL9ProgressionDef;
class UCL9RulesDef;
class UCL9SiteDef;

/** 밤 시작에 자리까지 정해 둔, 아직 스폰하지 않은 몬스터 한 마리 — 프레임마다 몇 마리씩 꺼내 스폰한다 */
struct FCL9PendingMonsterSpawn
{
	FName Alias;
	FTransform Transform;
};

/**
 * 베이스 게임 모드. 리슨 호스트에만 존재한다 — 권한을 가진 게임 흐름
 * 로직은 여기에 두고, 클라이언트가 필요로 하는 상태는 대신 게임
 * 스테이트로 보낸다(conventions § 2). DefaultPawnClass는 BP 자식에서
 * 설정하므로 C++는 애셋 참조를 전혀 갖지 않는다.
 */
UCLASS(abstract)
class ACL9GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACL9GameMode();

	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;
	virtual bool AllowCheats(APlayerController* P) override;

	/**
	 * 호스트 전용: 아이템 하나를 스폰하고 표준 데이터 경로
	 * (SetAlias → replication → LoadData)를 거치게 한다. 행의 ActorClass 컬럼을
	 * 존중한다 — 설정돼 있으면 전용 BP를, 아니면 공용 아이템 BP를 쓴다(docs/plan/data/README.md 규칙 2).
	 * PresetValue가 0 이상이면 행에서 굴린 값 대신 그 값을 쓴다 — 퀵슬롯 칸에서 꺼내 다시 스폰할 때
	 * 개체의 가격을 보존하는 경로다(docs/systems/inventory.md). 음수면 새로 굴린다.
	 */
	ACL9Item* SpawnItem(FName ItemAlias, FName ItemSubAlias, const FTransform& Transform, int32 PresetValue = -1);

	/** 호스트 전용: 몬스터 하나를 표준 데이터 경로로 스폰한다 — 아이템과 동일한 ActorClass 규칙을 따른다 */
	ACL9Monster* SpawnMonster(FName MonsterAlias, FName MonsterSubAlias, const FTransform& Transform);

	/**
	 * 호스트 전용: 로비 → 밤 — 시계를 가동하고 만료를 예약한다(docs/plan/data/rules.md 밤).
	 * 구역 짓기(`ACL9SiteMap::IsBuildPending`)나 내비 빌드가 아직 진행 중이면 다음 프레임으로 미루고 다시 본다 — 기다린
	 * 시간은 로그로 남는다(docs/goal/giant-stage.md Nav). 기다리는 동안은 여전히 로비다.
	 */
	void StartNight();

	/** 호스트 전용: 지금 밤 → 새벽. 만료와 전멸 모두 여기로 온다 */
	void EndNight();

	/**
	 * 호스트 전용: 캐릭터의 HP가 0이 됐다(docs/systems/character.md 죽음). 아무도
	 * 서 있지 않으면 전멸이다 — 트럭이 떠나고 밤이 끝난다. 그렇지 않으면
	 * 죽은 플레이어는 SpectateDelay가 지난 뒤 살아있는 팀원의 시점을 탄다.
	 * 부활은 없다 — 다음 로비가 부활시킨다(ReviveDead).
	 */
	void HandlePlayerDeath(ACL9Character* Dead);

	/** 호스트 전용: 관전자의 시점을 다음으로 살아있는 팀원에게 옮긴다(docs/systems/input.md 관전 대상 전환) */
	void SpectateNext(ACL9PlayerController* Spectator);

	/**
	 * 호스트 전용, 로비: 오늘의 주문 후보를 뽑는다 — MinDay/MaxDay 필터,
	 * 중복 없는 가중치 추첨, N과 M은 progression 행에서 온다
	 * (docs/systems/order.md 주문 흐름). 시작 시점과 매 로비, cl9.day에서 실행된다.
	 */
	void OfferOrders();

	/**
	 * 호스트 전용, 로비: 선택을 확정한다 — offer 안에서 정확히 M개의
	 * 서로 다른 인덱스여야 한다. 수락된 주문은 마감일을 가지며 납품되거나
	 * 만료될 때까지 유지된다(4e). 선택이 거부되면 false를 반환한다.
	 */
	bool AcceptOrders(const TArray<int32>& OfferedIndices);

	/**
	 * 호스트 전용: 창구에 올라온 것으로 채워지는 수락 주문을 전부 완납 처리한다
	 * (docs/systems/order.md 납품 판정). 마감이 이른 주문부터 채우고, 재료는
	 * 채워지는 그 자리에서 창구에서 빠지므로 같은 완성물이 두 주문에 이중으로
	 * 세어지지 않는다. 지급한 보상 합계를 반환한다.
	 */
	int32 SettleOrdersAt(ACL9Counter& Counter);

	/**
	 * 호스트 전용: 창구에 남은 것을 전부 판다 — 최종 가격은 개체의 굴림값 ×
	 * 목적지 ValueRate × 일자 ValueCurve다(docs/plan/economy.md 가격식).
	 * 지급한 이더 합계를 반환한다.
	 */
	int32 SellStockAt(ACL9Counter& Counter);

	/**
	 * 호스트 전용: 마감일이 지난 수락 주문을 지우고 위약금을 물린다 —
	 * 보상 × FailPenaltyRate만큼 누적 이더에서 빠진다(docs/plan/data/order.md 마감).
	 * 새 날이 시작될 때(로비) 실행된다.
	 */
	void ExpireOrders();

	/**
	 * 호스트 전용: 장비 하나를 산다 — 판정을 통과하면 이더를 빼고 트럭 옆에 **실물을 스폰한다**.
	 * "인벤토리에 추가"가 아니다(docs/systems/shop.md 구매 흐름). 거절 사유를 그대로 반환한다.
	 */
	ECL9BuyResult BuyEquipment(FName EquipAlias);

	/**
	 * 호스트 전용, 새벽: 트럭에서 멀리 두고 온 장비를 잃는다 —
	 * "원정에 들고 갈 수 있고, 두고 오면 잃는다. 다시 사는 값이 그 대가다"
	 * (docs/plan/data/equipment.md 원정과 분실). 잃은 개수를 반환한다.
	 */
	int32 LoseEquipmentLeftBehind();

	/** 지금 월드에 남아 있는 장비의 alias 목록 — 세이브가 이걸 적는다(같은 것이 둘이면 둘 다) */
	void ListOwnedEquipment(TArray<FName>& OutAliases) const;

	/**
	 * 호스트 전용, 새벽: 정산일이면 이번 기간 수입을 목표(`QuotaCurve`)와 견준다
	 * (docs/systems/balance.md 실패 판정). **판정 대상은 보유 잔액이 아니라 기간 수입이다** —
	 * 장비를 사도 정산이 불리해지지 않는다. 미달이면 그날로 판이 끝나고 새 판이 열린다(유예 없음).
	 * 정산이 실제로 일어났으면 true.
	 */
	bool Settle();

	/** 호스트 전용: 판이 끝났다 — 기록을 남기고 1일차부터 다시 시작한다 (docs/systems/session.md) */
	void StartNewRun();

	/** 개체의 굴림값에 목적지·일자 배율을 곱한 최종 가격(에테르) */
	int32 ComputeSellPrice(int32 RolledValue) const;

	/**
	 * 호스트 전용: 판을 슬롯에 저장한다 — 새벽마다 한 번 자동으로 실행되고, `cl9.save`로도
	 * 부를 수 있다. 저장 단위는 "날"이다: 손에 든 것도 맵도 저장하지 않는다(docs/systems/save.md).
	 */
	bool SaveRun(int32 Slot = 1);

	/**
	 * 호스트 전용, 로비: 슬롯의 판을 불러온다 — 날짜·자금·진행 중 주문을 되돌리고 그 일차의
	 * 주문 제시를 다시 뽑는다(`bOfferOrders`). 레벨을 건너 이어 온 판(7i)은 떠나기 전 로비에서 이미 그날 주문을 받았으므로 다시 뽑지 않는다.
	 * 로비 [이어하기] UI가 생기기 전까지는 `cl9.load` 와 레벨 이동 전용이다.
	 */
	bool LoadRun(int32 Slot = 1, bool bOfferOrders = true);

	/** 회사 레벨(`CL9Rules.CompanyLevel`)이 정해져 있나 — 없으면 정산은 옛대로 로비에서 한다 (docs/goal/loop-7i-company.md) */
	bool HasCompany() const;

	/** 지금 레벨이 회사인가 */
	bool IsCompanyLevel() const;

	/** 회사에 갈 때인가 — 회사가 있고, 여기가 회사가 아니고, 로비이고, 오늘이 정산일이다. 출발 레버가 판 대신 회사로 간다 */
	bool IsCompanyDue() const;

	/** 다음 밤의 맵 시드를 고정한다(cl9.seed) — 0이면 다시 무작위로 굴린다 */
	void SetForcedMapSeed(uint32 Seed) { ForcedMapSeed = Seed; }

	/**
	 * `cl9.imposter` (docs/goal/loop-7g-imposter.md): 플레이어 목록의 `Index` 번째를 임포스터로 — 판 중이면 지금 바꾸고, 로비면 다음 판에 쓴다.
	 * -1 이면 판 중엔 임포스터를 거두고, 로비에선 다음 판을 "임포스터 없음"으로 정한다. 지금 상태를 로그로 남긴다
	 */
	void CheatSetImposter(int32 Index);

	/** `cl9.imposter` 인자 없이: 지금 임포스터와 다음 판 지정을 로그로 남긴다 */
	void LogImposter() const;

protected:
	virtual void StartPlay() override;
	/** 새벽 → 로비, 날짜를 진행시킨다 — 다음 밤을 준비하는 정차 지점이다(docs/plan/loop.md 정차) */
	void EnterLobby();

	/** 호스트 전용: SpectateDelay가 다 지났다 — 시점이 첫 번째로 살아있는 팀원을 탄다 */
	void BeginSpectating(ACL9PlayerController* Spectator);

	/** 합류 순서상 `After` 다음에 오는 살아있는 플레이어 폰(순환), `Except`는 절대 아니다; 아무도 살아있지 않으면 null */
	ACL9Character* FindLivingPawn(const AActor* Except, const AActor* After) const;

	/** 호스트 전용: 죽은 플레이어 전원이 트럭 패드 위에서 풀 HP로 일어나고 관전을 멈춘다 */
	void ReviveDead();

	/**
	 * 트럭 패드 앞의 깨어나는 자리 (docs/plan/loop.md 귀환). 패드 원점이 아니라 **앞으로
	 * 비켜선 자리**다 — 패드는 트럭 몸체까지 포함한 덩어리라 원점에 세우면 캡슐이 그
	 * 콜리전에 갇힌다 (2026-09-10).
	 */
	static FVector WakeSpot(const class ACL9TruckPad& Pad, const class ACL9Character& Character);

	/**
	 * 호스트 전용, 밤 시작: 사이트의 기준값 × 날짜 곡선을 굴려
	 * 오늘 밤의 동물과 재료를 스폰한다(docs/systems/raiding.md 스폰).
	 * 지어진 맵(구역 사이트면 구역마다 하나)마다 그 맵의 지점·구역 행·항아리를 따로 다룬다. 맵이 없으면 평지 레벨이
	 * 트럭 주변에 흩뿌려 대신한다. 재료·항아리는 지금 스폰하고, 몬스터는 자리까지 정해 큐에 넣은 뒤 다음 프레임부터
	 * `MonsterSpawnPerFrame` 마리씩 스폰한다(SpawnPendingMonsters).
	 */
	void SpawnNightPopulation(const UCL9SiteDef& Site, const UCL9ProgressionDef* Progression, int32 Day);

	/** 호스트 전용: 밤 몬스터 큐에서 이번 프레임 몫을 스폰하고, 남았으면 다음 프레임에 다시 부른다 */
	void SpawnPendingMonsters();

	/**
	 * 호스트 전용, 판 시작 스폰 직후 (docs/goal/loop-7e-mid-round-spawn.md): 판 중간 스폰 타이머를 건다 — 생성된 맵이 있고
	 * `CL9Rules.MidSpawnInterval` > 0 이고 총량(판 시작 큐 수 × `MidSpawnCapRatio`, 반올림) > 0 일 때만. 종 풀은 사이트 행 + 지어진 맵의 구역 행
	 */
	void ArmMidSpawns(const UCL9SiteDef& Site, int32 Day);

	/** 호스트 전용, 타이머 한 번: 종 하나를 비율로 뽑아 모든 산 사람에게 안 보이고 멀리 떨어진 `Monster` 지점 하나에 세운다. 총량이면 타이머를 멈춘다 */
	void MidSpawnTick();

	/**
	 * 호스트 전용: 구역 짓기나 내비 빌드가 진행 중이면 다음 프레임에 `StartNight` 를 다시 부르도록 걸고 true.
	 * 다 끝났으면 false — 기다렸던 경우 그 시간을 로그로 남긴다.
	 */
	bool DeferNightStartWhileBuilding();

	/** 미룬 밤 시작의 재시도 — 핸들을 비우고 StartNight 를 다시 부른다 */
	void RetryStartNight();

	/** 호스트 전용, 로비: 사이트는 밤과 함께 사라진다 — 스포너가 배치한 것이 전부 없어진다(docs/plan/data/equipment.md 원정과 분실) */
	void ClearNightPopulation();

	/** 트럭에서 무작위 방위와 거리만큼 떨어진, 모든 플레이어의 ThreatActMinRadius 바깥에 있는 지면 위치 */
	bool FindSpawnPoint(float MinDistance, float MaxDistance, FVector& OutLocation) const;

	/** 어떤 플레이어의 ThreatActMinRadius 안쪽이면 true — 그 안에서는 아무것도 깨어나거나 스폰되지 않는다(docs/plan/data/rules.md) */
	bool IsInsideCalmRadius(const FVector& Location) const;

	/** 최장 생존 일차를 기록 파일에 남긴다 — 판과 수명이 다르므로 슬롯과 무관한 별도 파일이다 */
	void RecordBestDay(int32 Day);

	/**
	 * 호스트 전용, `EndNight` 의 남겨짐 판정 뒤 (docs/goal/loop-7a-wipe-penalty.md): 산 사람이 없으면 전멸 패널티 —
	 * 판의 누적 이더에서 `CL9Rules.WipePenaltyFor` 만큼 깎고(`Deduct`, 기간 수입은 그대로) 결과를 `SetWipePenalty` 로 싣는다.
	 * 판 중 전멸(`HandlePlayerDeath` → `EndNight`)과 판 끝에 전원이 남겨진 경우가 모두 여기를 지난다. `bWipeEndsRun` 이 켜져 있으면
	 * 판이 끝나 이 자리까지 오지 않는다
	 */
	void ApplyWipePenalty();

	/**
	 * 호스트 전용, `EndNight` 의 전멸 패널티 뒤 (docs/goal/loop-7f-out-game-currency.md): 이번 판 수입(`PeriodIncome` − 판 시작 값)에 대한
	 * 귀가 보상(`CL9Rules.WalletIncomeFor`)을 살아 있는 사람에게, 0 을 나머지에게 — 모두에게 `Client_WalletGrant` 한 번씩
	 */
	void GrantRoundWallet();

	/** 판이 열린 순간의 `PeriodIncome` — 판 수입 = 판 끝 값 − 이것(7f) */
	int32 NightIncomeStart = 0;

	/**
	 * 호스트 전용 (docs/goal/loop-7i-company.md): 판을 저장하고(`SaveRun(1)`) 게임 인스턴스에 "이어 간다"를 표시한 뒤 모두를 `Level` 로 데려간다 —
	 * 비심리스 `ServerTravel`(상대 이동, Shipping 이 아니면 `?listen`), 클라이언트는 따라와 다시 접속한다. `FromSite` 는 떠나는 사이트(회사의 목적지 판 처음 값)
	 */
	bool TravelForRun(const TSoftObjectPtr<UWorld>& Level, FName FromSite, const TCHAR* Why);

	/** 회사에서 다음에 갈 사이트 — 목적지 판의 값, 판이 없으면 떠나온 사이트, 그것도 없으면 표의 첫 사이트 */
	FName GetCompanyDestination() const;

	/** `TravelForRun` 이 한 번 떠나면 켠다 — 레벨이 바뀌기 전 두 번째 출발을 막는다 */
	bool bTravelling = false;

	/** 이 레벨의 첫 판은 트럭 도착으로 연다(7j) — StartPlay 가 켜고 실제로 판이 열릴 때(`StartNight` 의 시계 뒤) 한 번 쓴다 */
	bool bArrivalPending = false;

	/**
	 * 호스트 전용, `StartNight` 의 스폰 뒤 (docs/goal/loop-7g-imposter.md): 산 사람 수로 `CL9Rules.ImposterChanceFor` 를 굴려 0~1명을
	 * 임포스터로 정한다(`ACL9PlayerState::HostSetImposter` — 본인 머신에만 복제). `cl9.imposter` 지정이 있으면 그것을 쓰고 지운다
	 */
	void AssignImposter();

	/** 지금 임포스터인 플레이어 — 없으면 null. 역할은 PlayerState 의 플래그가 정본이다 */
	ACL9PlayerState* FindImposter() const;

	/**
	 * 호스트 전용, `EndNight` 의 남겨짐 판정 **앞** (loop-7g): 이긴다 = 임포스터가 살아서 트럭 안, 다른 사람이 한 명 이상이고 모두 이미 죽었고,
	 * 그 누구도 임포스터가 직접 죽이지 않았다. 남겨짐은 아직 일어나지 않았으니 트럭 밖에 산 사람이 있으면 진다(2026-10-12 사용자 "남겨짐은 승리로 치지 않는다").
	 * 결과를 게임 스테이트(새벽 공개)와 `ImposterWinner`·`ImposterKiller`(보상)에 싣고 역할 플래그를 거둔다
	 */
	void JudgeImposter();

	/** 이번 판 판정에서 이긴 임포스터 — `GrantRoundWallet` 이 귀가 보상 대신 성공 보상을 준다. 판 시작에 비운다 */
	TWeakObjectPtr<ACL9PlayerState> ImposterWinner;

	/** 이번 판 판정에서 임포스터를 죽인 사람 — `GrantRoundWallet` 이 처치 보상을 더한다. 판 시작에 비운다 */
	TWeakObjectPtr<ACL9PlayerState> ImposterKiller;

	/** 이번 판의 임포스터(판정이 역할 플래그를 거둔 뒤에도) — `GrantRoundWallet` 이 유도 보상(7h)을 센다. 판 시작에 비운다 */
	TWeakObjectPtr<ACL9PlayerState> RoundImposter;

	/**
	 * 호스트 전용, `EndNight` 의 지갑 지급 앞 (docs/goal/loop-7h-kill-contribution.md): 판 동안 몬스터에게 죽은 사람 중 `InducedByPlayer` 가 있는 사람마다
	 * 기여 한 건을 모아 게임 스테이트에 싣는다(새벽 화면). 남겨진 사람은 몬스터에게 죽지 않았으니 들어가지 않는다
	 */
	void PublishContributions();

	/** `cl9.imposter` 의 다음 판 지정 — 플레이어 목록 번호, -1 = 임포스터 없음, `NoForcedImposter` = 지정 없음(굴린다) */
	static constexpr int32 NoForcedImposter = -2;
	int32 ForcedImposterIndex = NoForcedImposter;

	/**
	 * 호스트 전용, StartPlay(판 레벨): 트럭 옆면 바깥에 납품 창구를 하나 세운다(`CL9Rules.TruckCounterClass` × `TruckCounterOffset` × 트럭,
	 * docs/goal/truck-counter.md). 트럭이 없거나, 클래스 셀이 비었거나, 레벨에 창구가 이미 있으면(Lvl_Flat) 세우지 않는다
	 */
	void SpawnTruckCounter();


	/** 호스트 전용: 장비 하나의 실물을 트럭 옆에 내려놓는다 — 구매와 세이브 복원이 함께 쓴다 */
	ACL9Item* SpawnEquipmentAtTruck(const UCL9EquipmentDef& Equip);

	/**
	 * 새벽 결과 화면의 UI 표 행 이름 — C++ 상수로 박지 않는다(트럭 패널과 같은 이유).
	 * 비워 두면 새벽 화면을 열지 않는다.
	 */
	UPROPERTY(EditDefaultsOnly, Category="UI")
	FName DawnUIRow = FName("Dawn");

	/** 호스트 전용: 새벽 화면을 전원의 화면에 연다 — 판정은 호스트가, 화면은 각자의 것이다 */
	void OpenDawnScreen();

	/**
	 * 새벽에 이 거리 밖에 있는 장비는 잃는다 (cm, 트럭 패드 기준). 밤을 무사히 마치고
	 * 트럭 곁에 세워 둔 것만 다음 날로 넘어간다.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Shop")
	float EquipmentKeepRadius = 1200.f;

	/** 다음 밤을 위해 cl9.seed로 고정된 시드; 0 = 새로 굴린다 */
	uint32 ForcedMapSeed = 0;

	/**
	 * 이 레벨이 치르는 사이트 — 사이트 표에서 Level 열이 지금 레벨인 첫 행, 없으면 DefaultSiteAlias
	 * (docs/plan/data/site.md 스테이지, 2026-10-03). StartPlay에서 한 번 정한다.
	 */
	FName ResolveSiteAlias() const;
	FName ActiveSiteAlias;

	const UCL9RulesDef* GetRules() const;
	const UCL9ProgressionDef* GetProgression() const;

	FTimerHandle NightExpiryHandle;
	FTimerHandle DawnHandle;

	/** 미룬 밤 시작의 다음 프레임 재시도 — 유효하면 이미 기다리는 중이다 */
	FTimerHandle NightStartRetryHandle;

	/** 밤 시작을 미루기 시작한 실시간(초); 기다리지 않는 중이면 음수 */
	double NightStartWaitBegan = -1.0;

	/** 밤 몬스터 큐 — 밤 시작에 자리까지 정해 두고, NextPendingMonster 부터 프레임마다 꺼낸다 */
	TArray<FCL9PendingMonsterSpawn> PendingMonsterSpawns;
	int32 NextPendingMonster = 0;
	int32 PendingMonstersSpawned = 0;

	/** 판 중간 스폰 (7e) — 타이머, 이번 판의 총량·지금까지 수, 종 풀(별칭·무게), 판 일차(로그) */
	FTimerHandle MidSpawnHandle;
	int32 MidSpawnCap = 0;
	int32 MidSpawnCount = 0;
	TArray<TPair<FName, float>> MidSpawnPool;
	int32 MidSpawnDay = 0;
	double PendingMonstersBegan = 0.0;
	FTimerHandle MonsterSpawnHandle;

	/** 밤 스포너가 배치한 것들 — 다음 로비에서 제거된다; 치트 스폰과 레벨 소품은 남는다 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> NightSpawned;

	/**
	 * BP 자식에서 설정되는, 모든 플레이어가 스폰되는 캐릭터 행이다.
	 * 로비가 PlayerState에 플레이어별 선택을 담기 전까지의 임시 조치다
	 * (docs/systems/character.md "캐릭터 Def 적용 경로").
	 */
	UPROPERTY(EditDefaultsOnly, Category="Character")
	FName DefaultCharacterAlias;

	/** BP 자식에서 설정되는, 매일 밤 습격하는 사이트 행이다 — 4g가 목적지 선택을 추가하기 전까지의 임시 조치다 */
	UPROPERTY(EditDefaultsOnly, Category="Night")
	FName DefaultSiteAlias;

	/** 공용 아이템 BP — 행이 ActorClass를 비워둘 때마다 쓰인다 */
	UPROPERTY(EditDefaultsOnly, Category="Classes")
	TSubclassOf<ACL9Item> ItemClass;

	/** 공용 몬스터 BP — 행이 ActorClass를 비워둘 때마다 쓰인다 */
	UPROPERTY(EditDefaultsOnly, Category="Classes")
	TSubclassOf<ACL9Monster> MonsterClass;

	/** 밤 스포너가 생성기의 항아리 셀 위에 배치하는 반환용 항아리 BP(docs/systems/map.md 항아리) */
	UPROPERTY(EditDefaultsOnly, Category="Classes")
	TSubclassOf<ACL9Jar> JarClass;
};
