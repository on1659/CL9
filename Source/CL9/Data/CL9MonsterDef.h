// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "Data/CL9Types.h"
#include "CL9MonsterDef.generated.h"

class UAnimInstance;
class USkeletalMesh;
class USkeleton;

/**
 * 몬스터 행 하나 (docs/plan/data/monster/common.md). 공유되는 ACL9Monster 액터와
 * 그 하나뿐인 상태 머신은 모든 것을 여기서 읽어온다 — 새 몬스터는 새 행이지,
 * 새 AI 코드가 아니다 (docs/systems/monster.md). 거위 전용 컬럼은 모든 행에
 * 존재하며(쓰지 않는 종은 0/false로 둔다), 두 번째 종이 그 컬럼을 승격시킬
 * 때까지 그대로 유지된다.
 */
UCLASS()
class UCL9MonsterDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	// -- 외형 (표현 전용; 기본 Mesh 컬럼은 아이템 StaticMesh다) --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	/** 종마다 스켈레톤 하나 — 변종끼리는 이를 공유한다 (docs/systems/animation.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TSoftObjectPtr<USkeleton> Skeleton;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TSoftClassPtr<UAnimInstance> AnimBP;

	/**
	 * 비주얼 메시가 액터의 전방(+X)을 바라보도록 더하는 요(yaw) 값.
	 * 동물 팩마다 내보내는 정면 축이 달라서, 코드가 아니라 데이터로
	 * 이를 보정한다 (docs/systems/animation.md 원칙 1).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	float MeshYawOffset = 0.f;

	// -- 체력 --

	/** 개체마다 HpMin..HpMax 사이에서 HP를 굴린다 — 즉사 방지는 규칙이 아니라 값 선택이다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	int32 HpMin = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	int32 HpMax = 1;

	/** 치명적이지 않은 타격의 경직 시간, 밀리초 단위 (1000 = 1초) — 전용 애니메이션이 필요하다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	int32 HitStunMs = 300;

	// -- 이동 --

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float WanderSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float PatrolSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float ChaseSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float RetreatSpeed = 0.f;

	/** 초당 회전 각도 상한 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float TurnRate = 180.f;

	/** 회전이 달리기를 얼마나 느리게 만드는가 — 미로를 무기로 만드는 요소가 바로 이것이다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float TurnDrag = 0.f;

	/** 마당에서의 추격 속도 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float OutdoorChaseMult = 1.f;

	/** 몸통 반경, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float Radius = 30.f;

	/** 개체끼리 이만큼 간격을 유지한다, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	float SeparationDist = 0.f;

	// -- 감각 --

	/** 발견 거리 — SightOut보다 커야 한다 (이력현상 쌍) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Senses")
	float SightIn = 0.f;

	/** 놓치는 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Senses")
	float SightOut = 0.f;

	/** 정전 중의 시야 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Senses")
	float SightDark = 0.f;

	/** 청각 값은 이 하나뿐이다 — 소음 반경은 액션 쪽(CL9Rules)에서 온다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Senses")
	float HearRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Senses")
	float DetectNearRadius = 0.f;

	/** 가까이 있을 때 초당 탐지 게이지 상승량 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Senses")
	float DetectRateNear = 0.f;

	/** 멀리서 눈에 띄기만 했을 때 초당 게이지 상승량 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Senses")
	float DetectRateFar = 0.f;

	/** 탐지되지 않는 동안 초당 게이지 하락량 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Senses")
	float DetectDecay = 0.f;

	/** 시야에서 벗어난 뒤 유예 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Senses")
	float DetectGrace = 0.f;

	// -- 추격 --

	/** 추격이 지속될 수 있는 최대 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float ChaseBudget = 0.f;

	/** 시야를 놓친 뒤 포기하기까지의 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float ChaseLoseTime = 0.f;

	/** 포기한 뒤 주변을 둘러보는 정지 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float ChaseBreakTime = 0.f;

	/** 휴식 후 이 시간만큼은 새 경계 발동이 없다 — 정전/복수/기절 각성은 절대 막지 않는다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float ChaseCooldown = 0.f;

	/** 서식지로부터의 최대 거리, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float HomeRange = 0.f;

	/** 들고 있던 모든 것을 떨어뜨리면 이 거리에서 추격이 풀린다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float DeaggroRadius = 0.f;

	/** 붙잡기 사거리, cm 단위 — 0이면 절대 붙잡지 않는 종이다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float CatchRadius = 0.f;

	/** 붙잡힐 때마다 깎이는 플레이어 HP */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	int32 CatchDamage = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	int32 SearchHops = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float SearchDwellTime = 0.f;

	// -- 각성과 경보 --

	/** 각성 전 날갯짓+울음 경고, 초 단위 — 모든 플레이어에게 보인다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float WakeTelegraph = 0.f;

	/** 이 반경 안에 시선이 머무는 동안에는 각성이 유지된다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float WakeSightRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float WakeRetryTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float PackAlarmRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float KillAlarmRadius = 0.f;

	/** 경보를 울리는 개체 플래그 — 지금은 거위만 사용한다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	bool bAlertsOthers = false;

	// -- 거위 전용 컬럼 (docs/plan/data/monster/goose.md — 다른 종에서는 0/false) --

	/** 이 안에서 걷기만 해도 깨어난다 (걷기는 조용하다는 원칙의 예외) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float WakeRadius = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float HearRunRadius = 0.f;

	/** 경보가 발동하기 전 울음소리 준비 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float AlertWindup = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float SleepTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Alarm")
	float AlertCooldown = 0.f;

	// -- 보복과 회복 (문서화된 전역 기본값) --

	/** 죽음을 듣고 난 뒤의 사냥 모드 지속 시간, 초 단위 — KillCooldown과 같은 값으로 고정된다 (로드 체크) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Retaliation")
	float RageTime = 9.f;

	/** 다시 죽일 수 있게 되기까지의 재장전 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Retaliation")
	float KillCooldown = 9.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Retaliation")
	float RevengeTime = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Retaliation")
	float RevengeRadius = 276.f;

	/** 밀쳤을 때의 경직 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Retaliation")
	float PushStun = 0.8f;

	/** 넉백 거리, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Retaliation")
	float PushKnockback = 1560.f;

	// -- 표시 --

	/** 종마다 다른 값이지, 전역 규칙이 아니다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Display")
	bool bShowOnMinimap = false;

	// -- 가치와 드롭 --

	/** 사체 판매 같은 기능이 생기기 전까지는 쓰이지 않는다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Value")
	int32 ValueMin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Value")
	int32 ValueMax = 0;

	/** 사냥 보상 — 아이템만 지급하며 직접 에테르는 절대 주지 않는다 (아이템 테이블만 참조) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Value")
	TArray<FDropItem> DropItems;
};
