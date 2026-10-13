// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "CL9RulesDef.generated.h"

class UWorld;
class UStaticMesh;
class USkeletalMesh;
class UAnimationAsset;
class UMaterialInterface;

class UMaterialInterface;
class UNiagaraSystem;
class ACL9Counter;

/**
 * 유일한 전역 규칙 애셋 — 어떤 캐릭터도 다르게 가질 수 없는 값들이다: 소음,
 * 죽음, 밤 시계, 소지 마찰 (docs/plan/data/rules.md). 기본값은 문서화된 표를
 * 그대로 반영하며, 미정 컬럼은 결정되기 전까지 0으로 둔다.
 */
UCLASS()
class UCL9RulesDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	// -- 소리 (몬스터가 이 반경 안에서 듣는다 — build-order ③에서 사용) --

	/** 달릴 때의 소음 반경, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float RunNoiseRadius = 390.f;

	/** 걷는 중에 줍는 소음 반경, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float PickupQuietRadius = 150.f;

	/** 달린 직후에 줍는 소음 반경, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float PickupLoudRadius = 390.f;

	/** 밀치기, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float PushNoiseRadius = 255.f;

	/** 미끼, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float DecoyRadius = 330.f;

	/** 이 시간 안에 달린 적이 있으면 줍기가 시끄러운 것으로 간주된다, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float RecentRunTime = 1.f;

	// -- 죽음과 회복 --

	/** 죽은 뒤 팀원 관전으로 전환되기까지의 시간, 초 단위 — 초안 3, 플레이하며 조정 (docs/plan/data/rules.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float SpectateDelay = 3.f;

	/** 트럭 패드에 서서 HP가 다시 차기까지의 시간, 초 단위 — 초안 5, 플레이하며 조정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float TruckPadHealTime = 5.f;

	/** 이탈자의 폰이 남아있는 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float LeaverGraceTime = 3.f;

	/** 이 반경 안에서는 위협이 절대 깨어나거나 스폰되지 않는다, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float ThreatActMinRadius = 420.f;

	/**
	 * 처형 진입 때 피해자 시점이 몬스터의 처형 카메라로 넘어가는 블렌드 시간, 초 단위 — 0 이면 즉시. 관전과 같은
	 * `SetViewTarget` 경로에 `SetViewTargetWithBlend` 로 얹는다 (docs/systems/character.md 처형). 값은 balance
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float ExecuteCameraBlend = 0.f;

	/**
	 * 데스캠 자리 — 죽은 순간부터 관전 전까지(혼자면 밤이 끝날 때까지) 자기 시체를 3인칭으로 보는 카메라의 **액터 공간** 오프셋,
	 * cm 단위(X = 캐릭터 전방, Y = 오른쪽, Z = 위 — 처형 카메라와 같은 규약). **(0,0,0) 이면 데스캠 없음** = 옛 동작(넘어짐 1인칭
	 * 카메라가 바닥으로, 처형이면 몬스터 카메라에 머묾). (−320, 140, 260) = 뒤·옆·위 — PIE 에서 누운 몸 전체·옆의 돼지가 잡히는 값(2026-09-21), 아트 튜닝 (docs/systems/character.md 죽음)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	FVector DeathCameraOffset = FVector(-320.f, 140.f, 260.f);

	/** 데스캠이 바라보는 점 — 캡슐 중심(액터 위치) 기준 높이, cm 단위(음수 = 중심 아래). −60 = 누운 몸 높이(캡슐 반높이 −90 근처)라 시체가 화면 중앙에 온다 — 아트 튜닝 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float DeathCameraLookAtZ = -60.f;

	/**
	 * 죽는 순간 몬스터의 처형 카메라에서 데스캠으로 넘어가는 블렌드 시간, 초 단위 — 0 이면 즉시(`SetViewTargetWithBlend(폰)`,
	 * `HostBeginExecuted` 의 짝). 보통 사망은 뷰 타깃이 이미 자기 폰이라 엔진이 블렌드하지 않는다 — 넘어짐 카메라에서 데스캠으로 즉시 전환
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float DeathCameraBlend = 0.5f;

	// -- 밤 시계 --

	/** 밤 길이, 초 단위 — 시간이 다 되면 새벽 실패가 된다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float NightLength = 245.f;

	/** 정전까지 걸리는 기본 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float DarkBase = 185.f;

	/** 항아리 반납 한 번이 정전 시각을 앞당기는 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float DarkAccel = 4.f;

	/** 가능한 가장 이른 정전 시각, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float DarkMin = 125.f;

	/** 새벽 화면 길이, 초 단위 — 미정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float DawnLength = 0.f;

	/**
	 * 밤 시작에 큐로 모은 몬스터를 한 프레임에 몇 마리씩 스폰하는가 (마리/프레임, 1 이상). 한꺼번에 올리지 않는다
	 * (docs/goal/giant-stage.md Gate answers). 기본값은 자리표시 — balance 가 정한다
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	int32 MonsterSpawnPerFrame = 4;

	// -- 판 중간 스폰 (docs/goal/loop-7e-mid-round-spawn.md, 2026-10-12 사용자 — 값은 초안, 밸런스는 나중) --

	/** 판 중간 스폰 간격, 초 — 판이 열린 뒤 이 간격마다 한 마리(첫 번째는 +간격). 0 이면 판 중간 스폰이 없다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night", meta=(ClampMin="0", UIMin="0"))
	float MidSpawnInterval = 60.f;

	/** 판 중간 스폰 자리가 살아 있는 모든 플레이어에게서 떨어져야 하는 최소 거리, cm (3D) — 눈앞에서 튀어나오지 않게 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night", meta=(ClampMin="0", UIMin="0"))
	float MidSpawnMinDistance = 1500.f;

	/** 한 판의 판 중간 스폰 총량 = 판 시작에 큐에 든 몬스터 수 × 이 비율(반올림). 0 이면 없다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night", meta=(ClampMin="0", UIMin="0"))
	float MidSpawnCapRatio = 0.5f;


	/**
	 * 전멸 패널티 — 전멸로 판이 끝나면(`bWipeEndsRun` false) 판의 누적 이더(`SharedMoney`)에서 이 비율만큼 깎는다(0~1, 2026-10-12 사용자 30%).
	 * 정수 이더라 내림, 잔액은 0 아래로 내려가지 않는다. 기간 수입(정산 판정 대상)은 건드리지 않는다. 범위 밖·NaN 은 로드 오류
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Run", meta=(ClampMin="0", ClampMax="1", UIMin="0", UIMax="1"))
	float WipePenaltyRate = 0.3f;

	/** 누적 이더 `Money` 에서 전멸 패널티로 깎을 액수 — floor(Money × WipePenaltyRate), 음수 잔액·범위 밖 비율은 0~잔액으로 자른다 */
	int32 WipePenaltyFor(int32 Money) const;

	/**
	 * 바깥 재화(화면 이름 "이더") 귀가 보상 비율 — 판 끝에 살아 있는 사람이 그 판 수입(코인)의 이 비율을 받는다(0~1, 2026-10-12 사용자
	 * "판 수입 비율에 따라 다르게", 초안 0.01 = 100 코인당 1). 임포스터 성공·처치 보상(7g)은 이보다 커야 한다. 범위 밖·NaN 은 로드 오류
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Run", meta=(ClampMin="0", ClampMax="1", UIMin="0", UIMax="1"))
	float WalletIncomeRate = 0.01f;

	/** 판 수입 `Income` 에 대한 귀가 보상 — floor(Income × WalletIncomeRate), 만분율 정수 계산(WipePenaltyFor 와 같다) */
	int32 WalletIncomeFor(int32 Income) const;

	/**
	 * 임포스터 배정 확률 — 판을 시작할 때 살아 있는 사람 수로 고른다(번호 = 사람 수 − 1, 배열 끝을 넘으면 마지막 값).
	 * 2026-10-12 사용자 "3인 이상일 때 10%, 4인일 때 25%" — 1·2인은 0(docs/goal/loop-7g-imposter.md). 각 값은 0~1, 범위 밖·NaN 은 로드 오류
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Imposter")
	TArray<float> ImposterChanceByPlayers = { 0.f, 0.f, 0.1f, 0.25f };

	/** 사람 수 `Players` 일 때의 임포스터 배정 확률(0~1로 자른다) — 표가 비었거나 0명 이하면 0 */
	float ImposterChanceFor(int32 Players) const;

	/**
	 * 임포스터 성공 보상(이더) — 이긴 임포스터는 귀가 보상 **대신** 이것을 받는다. 귀가 보상(판당 보통 3~10)보다 커야 한다(7f 사용자 답).
	 * 초안 30 — "밸런스는 표에서 조절 가능하게 하고 넘기자"(2026-10-12). 음수는 로드 오류
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Imposter", meta=(ClampMin="0"))
	int32 ImposterWinReward = 30;

	/** 임포스터 처치 보상(이더) — 임포스터를 죽인 사람이 귀가 보상에 **더해** 받는다(살았든 죽었든). 초안 20, 음수는 로드 오류 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Imposter", meta=(ClampMin="0"))
	int32 ImposterKillReward = 20;

	/**
	 * 처치 기여 인정 시간, 초(docs/goal/loop-7h-kill-contribution.md) — 몬스터를 깨운 뒤 이 안에 그 몬스터가 다른 플레이어를 죽이면 깨운 사람의 기여다.
	 * 2026-10-12 사용자 "몬스터를 깨운 사람으로 한정, 인정 시간은 셀"(초안 30). 0 = 기여 끔, 음수·NaN 은 로드 오류
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Imposter", meta=(ClampMin="0"))
	float ContributionWindow = 30.f;

	/** 임포스터 유도 보상(이더) — 판의 임포스터가 깨운 몬스터에게 죽은 사람 하나마다(이겼든 졌든) 더 받는다. 초안 10, 음수는 로드 오류 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Imposter", meta=(ClampMin="0"))
	int32 ImposterAssistReward = 10;

	/**
	 * 회사 레벨(docs/goal/loop-7i-company.md) — 정산일 로비에서 출발 레버를 당기면 판을 싣고 이 레벨로 간다(정산·장비 구매·다음 목적지).
	 * 비어 있으면 회사가 없고 정산은 옛대로 로비에서 한다. 레벨 경로는 C++ 에 없다(conventions § 5)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Company")
	TSoftObjectPtr<UWorld> CompanyLevel;

	/** 회사 레벨이 정해져 있고 `World` 가 그 레벨이면 true — PIE 접두사를 떼고 패키지 이름으로 비교한다(사이트 판별과 같다) */
	bool IsCompanyWorld(const UWorld* World) const;

	/**
	 * 정산 미달 패널티 — 목표에 못 미치면 판이 끝나는 대신 판의 코인에서 이 비율만큼 깎고 다음 기간으로 이어 간다
	 * (2026-10-12 사용자 "게임 오버 대신 패널티 후 계속", 초안 0.3). 0~1, 범위 밖·NaN 은 로드 오류
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Company", meta=(ClampMin="0", ClampMax="1", UIMin="0", UIMax="1"))
	float QuotaMissPenaltyRate = 0.3f;

	/** 코인 `Money` 에서 정산 미달로 깎을 액수 — floor(Money × QuotaMissPenaltyRate), 전멸 패널티와 같은 만분율 정수 계산 */
	int32 QuotaMissPenaltyFor(int32 Money) const;

	// -- 스테이지 별 (docs/goal/loop-7l-stage-stars.md) --

	/**
	 * 정산 기간 하나(목적지 하나 + 3판 + 회사 정산)를 스테이지로 보고 별을 매긴다 — 별로 다음 목적지가 열리고, 마지막 목적지를
	 * 별을 받고 마치면 판이 클리어로 끝난다(2026-10-13 사용자). 끄면 7i 그대로(별 없음·목적지 전부 열림·클리어 없음)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage")
	bool bStageStars = true;

	/**
	 * 별 기준 — 별 n 개는 기간 수입이 목표 × StageStarRatios[n-1] 이상. 오름차순 양수, 개수가 별의 최대 수다(초안 1.0·1.5·2.0).
	 * 별 0 은 정산 미달과 같은 뜻이 되도록 첫 값을 1.0 으로 둔다(미달 판정·패널티는 따로 그대로 돈다)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage")
	TArray<float> StageStarRatios = { 1.f, 1.5f, 2.f };

	/** 다음 목적지가 열리고, 마지막 목적지면 클리어가 되는 별 수(초안 1). 1 이상 StageStarRatios 개수 이하 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage", meta=(ClampMin="1"))
	int32 StageUnlockStars = 1;

	/** 기간 수입 `Earned` 와 목표 `Target` 으로 별 수(0..StageStarRatios 개수). 목표가 0 이하면 모든 기준을 넘은 것으로 본다 */
	int32 StageStarsFor(int32 Earned, int32 Target) const;

	// -- 판 시작 연출 (docs/goal/loop-7j-round-start.md) --

	/** 트럭 도착 길이(초) — 레벨이 서고 여는 첫 판에 트럭이 후진해 서는 동안 카메라가 보고 조작이 잠긴다. 0 = 도착 없음. 초안 5 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Arrival", meta=(ClampMin="0"))
	float ArrivalTime = 5.f;

	/** 도착 시작 때 트럭이 제자리에서 앞쪽(트럭 +X)으로 떨어져 있는 거리(cm) — 메시만 옮긴다(판정용 트럭은 제자리). 초안 2500 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Arrival", meta=(ClampMin="0"))
	float ArrivalDistance = 2500.f;

	/** 도착 카메라 자리 — 트럭 패드 공간(cm). 트럭 몸체를 바라본다. 초안 (700, −900, 260) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Arrival")
	FVector ArrivalCameraOffset = FVector(700.f, -900.f, 260.f);

	/** 전원을 켜기 전(첫 어둠) 안개 밀도 배율 — 사이트 행의 밤 안개에 곱한다. 마당을 건너는 길을 무겁게(2026-10-12 사용자 "낮게 깔린 안개"). 초안 2, 1 = 그대로 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Yard", meta=(ClampMin="0"))
	float PrePowerFogScale = 2.f;

	/** 마당 → 건물 문 바깥 위에 거는 작업등 메시 — 비면 빛만 건다. 경로는 C++ 에 없다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Yard")
	TSoftObjectPtr<UStaticMesh> YardLampMesh;

	/** 작업등 높이(cm, 문 바닥에서) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Yard", meta=(ClampMin="0"))
	float YardLampHeight = 330.f;

	/** 작업등 밝기(칸델라) — 0 이면 등을 걸지 않는다. 끊기듯 깜박인다. 초안 10 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Yard", meta=(ClampMin="0"))
	float YardLampIntensity = 10.f;

	/** 작업등 감쇠 반경(cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Yard", meta=(ClampMin="0"))
	float YardLampRadius = 900.f;

	/** 마당에 있는 동안(전원 전) 건물 안에서 나는 울음의 간격(초) 최솟값·최댓값 — 최댓값 0 = 끔. 초안 8~16 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Yard", meta=(ClampMin="0"))
	float YardCryIntervalMin = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Yard", meta=(ClampMin="0"))
	float YardCryIntervalMax = 16.f;

	/** 문간을 가로지르는 실루엣의 몸(보이기만, 판정 없음) — 비면 실루엣 없음 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Yard")
	TSoftObjectPtr<USkeletalMesh> SilhouetteMesh;

	/** 실루엣이 걷는 애님(반복) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Yard")
	TSoftObjectPtr<UAnimationAsset> SilhouetteAnim;

	/** 실루엣의 모든 재질 칸을 덮는 어두운 재질 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Yard")
	TSoftObjectPtr<UMaterialInterface> SilhouetteMaterial;

	/** 실루엣 배율·걷는 속도(cm/s)·보이는 거리(cm, 0 = 끔)·뒤에서 비추는 빛(칸델라) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Yard", meta=(ClampMin="0"))
	float SilhouetteScale = 1.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Yard", meta=(ClampMin="0"))
	float SilhouetteSpeed = 260.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Yard", meta=(ClampMin="0"))
	float SilhouetteRange = 2500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Yard", meta=(ClampMin="0"))
	float SilhouetteBackLight = 20.f;

	/**
	 * 트럭 창구 — 납품 창구(`ACL9Counter`) 블루프린트(docs/goal/truck-counter.md). 트럭이 서는 판 레벨마다 트럭 옆면 바깥에 하나 세운다
	 * (2026-10-13 사용자 "트럭 창구에서 납품", 옆면 바깥). 레벨에 창구가 이미 놓여 있으면(시험 레벨 Lvl_Flat) 세우지 않는다.
	 * 비어 있으면 트럭 창구가 없다 — 경로는 C++ 에 없다(conventions § 5)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Truck")
	TSoftClassPtr<ACL9Counter> TruckCounterClass;

	/**
	 * 트럭 창구 자리 — 트럭 공간 상대 변환(cm, 도). 헤더 기본 (350, −267, 0) = 짐칸 가운데 옆, B 트럭 몸체 반폭 212 바깥에
	 * 창구 반폭 50 과 틈 5 를 둔 지면. 트럭 몸체에 창을 뚫으면 그 앵커로 옮긴다 — 자리표시
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Truck")
	FTransform TruckCounterOffset = FTransform(FRotator::ZeroRotator, FVector(350.f, -267.f, 0.f));

	// -- 던지기와 소지 --

	/** 퀵슬롯 칸 수 — 한 번에 들고 다닐 수 있는 아이템 수. 손에 실물로 들리는 것은 활성 칸 하나뿐이다 (docs/systems/inventory.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	int32 QuickSlots = 4;

	/** 달리며 소지하는 상태가 지속되어 흔들림 게이지가 가득 차기까지의 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ShakeFullTime = 3.f;

	/** 흔들림 게이지 회복 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ShakeDrainTime = 1.5f;

	/** 충전 0일 때의 투척 속도, cm/s 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ThrowMinSpeed = 600.f;

	/** 완전 충전 시의 투척 속도, cm/s 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ThrowMaxSpeed = 1680.f;

	/** 완전 충전까지 걸리는 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ThrowChargeTime = 0.85f;

	/** 칸에서 꺼내 스폰하는 높이, cm 단위 — 놓기에는 안 쓴다(놓기는 손 자리, 2026-09-14) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float CarryHeight = 120.f;

	/** 줍기 사거리, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float PickRadius = 120.f;

	/**
	 * 우클 길게 누르기 문턱, 초 단위 — 이 시간 안에 떼면 탭(스캔), 넘기면 활성 칸의 요리를 먹는다
	 * (2026-09-13 사용자: "우클 길게 = 먹기"). 소유 클라가 로컬 타이머로 잰다 (docs/systems/inventory.md 조작).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float UseHoldTime = 0.4f;

	// -- 휘두르기 히트 구간 (docs/plan/data/item/common.md 사용) --

	/**
	 * 휘두르기 히트 구간의 시작, 초 단위 — 좌클을 뗀 뒤(`GetSwingElapsed()`) 이 시각부터 손에 든 것의 히트박스가 몬스터를
	 * 맞힌다. 휘두르기 클립은 ABP가 `SwingElapsed + 0.12`로 평가해 `[0, 0.347)` 동안만 돌아가므로 구간은 그 안이어야 한다
	 * (`Validate`). 접촉 프레임에서 정한다 (docs/asset/character/farmer_a.md 026/031). 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Swing")
	float SwingHitStart = 0.05f;

	/** 휘두르기 히트 구간의 끝, 초 단위 — 이 시각부터는 맞지 않는다(구간은 반열림 `[Start, End)`). 초안값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Swing")
	float SwingHitEnd = 0.25f;

	/**
	 * 휘두르기 판정 선분의 끝, cm 단위 — 손 높이 원점에서 컨트롤 회전 방향으로 이 거리까지 구를 끌고 간다
	 * (`ACL9Character::HostTickSwingHit`, docs/systems/character.md 휘두르기). 최대 닿는 거리 = Reach + Radius + 몬스터 반경.
	 * **헤더 0 = 미설정 = 로드 오류**(`Validate`) — 되돌아갈 옛 경로가 없다. 값은 balance(docs/plan/data/rules.md)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Swing")
	float SwingHitReach = 0.f;

	/**
	 * 휘두르기 판정 구의 반경, cm 단위 — 선분은 원점에서 이 거리만큼 앞에서 시작해(`Origin + Dir·Radius`) 구가 등 뒤로
	 * 나가지 않는다. 옆 관용치 = √((Radius + 몬스터 반경)² − 높이차²). **헤더 0 = 미설정 = 로드 오류**(`Validate`).
	 * `SwingHitReach >= SwingHitRadius` 여야 선분이 앞을 향한다. 값은 balance
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Swing")
	float SwingHitRadius = 0.f;

	/**
	 * 휘두른 뒤 다음 휘두르기가 받아들여지기까지, 초 단위 — `HostSwing` 이 `SwingReadyTime = 지금 + 이 값` 을 찍고 그 전의
	 * 입력은 거부(또는 `SwingBufferTime` 예약)한다. 첫 명중 때 `HitStopTime` 만큼 늘어난다(팔 히트스톱과 같은 여유).
	 * `Validate`: `>= SwingClipWindow(0.347)` ∧ `> SwingHitEnd`. **헤더 0.6 = 옛 상수 동작**. 값은 balance
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Swing")
	float SwingRecovery = 0.6f;

	/**
	 * 입력 예약 창, 초 단위 — 회복 중에 좌클을 떼었는데 회복 끝까지 이 시간 이하로 남았으면 그 휘두르기를 하나 예약해
	 * 회복이 끝나는 호스트 틱에 낸다(`bSwingBuffered`, 덮어쓰기·중첩 없음). **헤더 0 = 끔 = 옛 동작**. 값은 balance
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Swing")
	float SwingBufferTime = 0.f;

	/**
	 * 휘두르기 클립이 ABP에서 도는 구간의 길이, 초 단위 — `ABP_Unarmed`·`ABP_FP_View`가 `SwingElapsed ∈ [0, 0.347)`
	 * 동안만 시퀀스 평가기를 태운다(30fps 0.467초 클립을 0.12초 지점부터, docs/systems/animation.md). 애님 에셋의 사실이지
	 * 밸런스 값이 아니라 상수다 — 히트 구간·회복이 이 안팎에 있는지 검사하고(`Validate`), 치트 로그의 `arm_return` 이 읽는다.
	 */
	static constexpr float SwingClipWindow = 0.347f;

	// -- 타격감·피격감 (docs/systems/monster.md 플래시, docs/systems/character.md 피격) --

	/**
	 * 맞은 몬스터가 빨간 오버레이를 쓰고 있는 시간, 초 단위 — 가장 짧은 경직(`HitStunMs` 300ms)의 절반이라 색이
	 * 먼저 꺼진다: 빨강은 맞는 순간, 젖혀진 자세는 경직을 말한다. 모든 머신이 복제된 상태 시각(`GetStateElapsed`)에서
	 * 파생한다. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeedback")
	float HitFlashTime = 0.15f;

	/**
	 * 잡힌 사람의 몸(과 소유자의 1인칭 팔)이 빨간 시간, 초 단위 — `HurtStun` 0.35초 비틀거림을 덮고 일어서기가
	 * 시작되는 0.9초 전에 꺼진다. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeedback")
	float HurtFlashTime = 0.45f;

	/**
	 * 잡힌 사람 화면의 빨간 비네트가 사라지기까지, 초 단위 — 넘어짐 경과에 따라 1 → 0으로 선형으로 옅어진다.
	 * `KnockdownTime` 1.5초보다 0.3초 길어서 일어설 때쯤 화면도 갠다. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeedback")
	float HurtVignetteTime = 1.8f;

	/**
	 * 위 두 플래시가 메시에 덧씌우는 오버레이 머티리얼 — 애셋 경로는 C++에 없다(conventions § 5).
	 * 비어 있으면 플래시는 조용히 꺼지고 나머지 연출(카메라 킥·비네트)만 남는다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeedback")
	TSoftObjectPtr<UMaterialInterface> HitFlashMaterial;

	// -- 피격 카메라 (docs/systems/character.md 피격 — 잡는 순간 카메라가 범인을 본다) --

	/**
	 * 잡힌 사람의 카메라가 `HurtLookPitch` 까지 내려가는 데 걸리는 시간, 초 단위 — 넘어진 순간부터 EaseOut 으로 도달을
	 * 보장한다(`UCL9HurtLookModifier`, POV 오프셋만 — 컨트롤 회전은 그대로). **0 이면 이 효과 전체가 꺼진다.**
	 * 로드 검사: `HurtLookTime + 일어나기 0.6초 <= CharacterDef.KnockdownTime` 이어야 진입·복귀 보간이 겹치지 않는다(어기면 Warning). 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeedback")
	float HurtLookTime = 0.45f;

	/** 넘어진 동안 POV 에 더하는 피치 오프셋, 도 단위 — 음수가 아래. 발밑의 닭이 화면 중앙 아래에 오는 값. 초안값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeedback")
	float HurtLookPitch = -25.f;

	/**
	 * 공격자 방향으로 POV 요를 돌리는 상한, 도 단위 — 공격자가 화면 가장자리에 걸칠 때까지만 돌고 이 값을 넘지 않는다.
	 * 가로 FOV 90° + 60° = 시선에서 105° 안의 공격자만 화면에 걸친다(그 밖은 방향 호가 답한다). 0 이면 요는 돌지 않는다. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeedback")
	float HurtLookYawMax = 60.f;

	/** HUD 가장자리의 피격 방향 호를 그리나 — 장르 톤이 맞지 않으면 빌드 없이 끈다 (docs/systems/ui.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeedback")
	bool bHurtDirectionIndicator = true;

	// -- 타격의 표현 (2026-09-18, docs/goal/game-feel.md §2·§3, docs/systems/monster.md 타격의 표현) --

	/**
	 * 히트스톱, 초 단위 — 맞은 몬스터의 메시가 Stun·Dead 진입 뒤 이 시간 동안 애니메이션을 멈추고(모든 머신,
	 * `bPauseAnims`), 때린 사람의 1인칭 팔은 `SwingHitServerTime` 뒤 같은 시간 동안 그 프레임에 머문다
	 * (`GetSwingAnimElapsed`, 소유자 화면). 이동·AI·히트박스는 멈추지 않는다 — 전역 시간 지연은 쓰지 않는다.
	 * 0.08 = `HitFlashTime` 0.15 안에 끝나고(정지 ⊂ 빨강), 히트박스 창 폭 0.20 의 절반 아래. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeel")
	float HitStopTime = 0.08f;

	/**
	 * 타격 넉백이 걸리는 시간, 초 단위 — 맞은 몬스터가 Stun 진입 뒤 `HitStopTime` 을 기다렸다가(멈춘 프레임은 밀리지 않는다)
	 * 이 시간 동안 등속(아이템 행 `HitKnockback` / 이 값)으로 밀린 뒤 멈춘다. 경직(`HitStunMs`)과는 별개의 시계라 경직
	 * 튜닝에 끌려가지 않는다 (docs/systems/monster.md 넉백). 0·음수·NaN 은 로드 오류. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeel")
	float HitKnockbackTime = 0.12f;

	/**
	 * 죽는 타격에 몸이 날아가는 수평 속도, cm/s 단위 — 방향은 타격 원점(`Die(Source)`) → 몬스터. 호스트가 넉백과 같은
	 * 방식으로 매 틱 적분하고(수직은 월드 중력 980) 트랜스폼 복제로 퍼진다. 0이면 제자리에서 죽는다.
	 * 300 = 체공 2·260/980 = 0.53초 동안 수평 159cm — 복도 한 칸(500cm)에서 가로로 맞아도 벽 앞에 눕는다. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeel")
	float DeathLaunchSpeed = 300.f;

	/**
	 * 죽음 발사의 위쪽 초기 속도, cm/s 단위 — 월드 중력(`GetGravityZ`)이 끌어내려 포물선을 그린다.
	 * 260 = 정점 34.5cm ≈ 닭 몸통(`Radius` 36), 체공 0.53초라 죽음 클립이 나는 동안 보인다. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeel")
	float DeathLaunchUp = 260.f;

	/**
	 * 죽을 때 흩어지는 드랍 아이템의 발사 속도, cm/s 단위 — 몬스터 중심 높이·옆 30~90cm 자리에서 앙각 45~70°·요 무작위로
	 * `OnThrown`. 이후는 아이템 물리·멈춤 규칙 그대로다. 0이면 던지지 않고 놓인다.
	 * 300 = 계란 표 중력 1950·앙각 70°·낙차 15cm 에서 착지 371 < `CrackSpeed` 450 — 평지에서 금이 가지 않는다. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeel")
	float DeathLootSpeed = 300.f;

	/**
	 * 폰이 달릴 때 발소리 한 번의 보폭, cm 단위 — 수평 이동 거리를 누적해 이 값마다 `Footstep` 행을 발밑에서 한 번 튼다.
	 * 폰은 수평 속도가 행의 `WalkSpeed`를 넘으면 달리기 보폭, 몬스터는 Chase 상태에서 달리기 보폭.
	 * 196 = `RunSpeed` 654 × 0.3초(`AN_Farmer_FPRun` 0.6초 루프 = 두 걸음) — 팔 펌프마다 한 발. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeel")
	float FootstepStrideRun = 196.f;

	/**
	 * 걸을 때 발소리 한 번의 보폭, cm 단위 — 0이면 발소리가 나지 않는다.
	 * 150 = `WalkSpeed` 396 에서 2.64 걸음/초(0.38초 간격) — 달리기 0.30초와 케이던스만으로 갈린다. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeel")
	float FootstepStrideWalk = 150.f;

	/**
	 * 타격 퍼프 나이아가라 — Stun·Dead 진입 에지(타격 플래시·`Hit` 소리와 같은 자리)에 몬스터 메시 중심(`spine` 소켓,
	 * 없으면 경계 중심)에서 스폰한다. `User.Scale`(float)을 Dead 는 3.0, 아니면 1.0 으로 준다. 비어 있으면 건너뛴다 —
	 * 애셋 경로는 C++에 없다(conventions § 5).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeel")
	TSoftObjectPtr<UNiagaraSystem> HitPuffSystem;
};
