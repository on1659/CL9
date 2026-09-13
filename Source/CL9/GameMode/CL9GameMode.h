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
class UCL9EquipmentDef;
class UCL9ProgressionDef;
class UCL9RulesDef;
class UCL9SiteDef;

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

	/** 호스트 전용: 로비 → 밤 — 시계를 가동하고 만료를 예약한다(docs/plan/data/rules.md 밤) */
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
	 * 주문 제시를 다시 뽑는다. 로비 [이어하기] UI가 생기기 전까지는 `cl9.load` 전용이다.
	 */
	bool LoadRun(int32 Slot = 1);

	/** 다음 밤의 맵 시드를 고정한다(cl9.seed) — 0이면 다시 무작위로 굴린다 */
	void SetForcedMapSeed(uint32 Seed) { ForcedMapSeed = Seed; }

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
	 * 아직 맵 생성기가 없어(⑤) 평지 레벨이 트럭 주변에 흩뿌려 대신한다.
	 */
	void SpawnNightPopulation(const UCL9SiteDef& Site, const UCL9ProgressionDef* Progression, int32 Day);

	/** 호스트 전용, 로비: 사이트는 밤과 함께 사라진다 — 스포너가 배치한 것이 전부 없어진다(docs/plan/data/equipment.md 원정과 분실) */
	void ClearNightPopulation();

	/** 트럭에서 무작위 방위와 거리만큼 떨어진, 모든 플레이어의 ThreatActMinRadius 바깥에 있는 지면 위치 */
	bool FindSpawnPoint(float MinDistance, float MaxDistance, FVector& OutLocation) const;

	/** 어떤 플레이어의 ThreatActMinRadius 안쪽이면 true — 그 안에서는 아무것도 깨어나거나 스폰되지 않는다(docs/plan/data/rules.md) */
	bool IsInsideCalmRadius(const FVector& Location) const;

	/** 최장 생존 일차를 기록 파일에 남긴다 — 판과 수명이 다르므로 슬롯과 무관한 별도 파일이다 */
	void RecordBestDay(int32 Day);

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

	const UCL9RulesDef* GetRules() const;
	const UCL9ProgressionDef* GetProgression() const;

	FTimerHandle NightExpiryHandle;
	FTimerHandle DawnHandle;

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
