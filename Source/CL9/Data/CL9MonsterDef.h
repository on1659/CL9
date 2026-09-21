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

	// -- 잡기 예고 Pounce (docs/systems/monster.md 잡기 예고). CatchRadius 0 또는 PounceWindup 0 이면 예고 없이 옛 즉시 잡기다 --

	/** 예고가 시작되는 거리, cm 단위 — CatchRadius 이상이어야 한다(로드 검사). 자리표시 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float PounceRadius = 0.f;

	/** 예고 길이, 초 단위 — 이동 0 으로 타깃을 향해 돌며 기다린다. 0 이면 Pounce 없음. 자리표시 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float PounceWindup = 0.f;

	/** 예고 끝에 타깃 방향으로 한 번에 덮치는 거리, cm 단위 — 벽이면 거기까지. 자리표시 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float PounceLunge = 0.f;

	/** 헛친 뒤 다시 예고할 수 있기까지, 초 단위 — KillCooldown 과 무관하다. 자리표시 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float PounceMissCooldown = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	int32 SearchHops = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Chase")
	float SearchDwellTime = 0.f;

	// -- 스토킹·처형 Execute (docs/plan/data/monster/common.md 스토킹과 처형). 두 갈래: ExecuteRadius > 0 = 스토커(너구리 — 몰래 뒤에서),
	// bExecuteOnCatch = 잡히면 처형(돼지 — 추격·예고·잡기 그대로, 잡는 순간이 처형). 둘 다 아니면 처형 없는 종 — 옛 추격·예고·잡기 그대로 --

	/** 스토킹 처형 진입 거리, cm 단위 — **0 이면 스토킹 없는 종**(Stalk 도 없다; bExecuteOnCatch 도 아니면 처형 없음). 자리표시 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Execute")
	float ExecuteRadius = 0.f;

	/** 타깃 등 뒤 원뿔의 반각, 도 단위 — 타깃 전방 · (타깃→몬스터) <= −cos(이 값) 이면 등 뒤다. 자리표시 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Execute")
	float ExecuteBehindAngle = 0.f;

	/** 아무도 보지 않은 채 이만큼 이어져야 처형에 들어간다, 초 단위. 자리표시 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Execute")
	float ExecuteUnseenTime = 0.f;

	/** 처형 진입부터 HostKill 까지, 초 단위 — 클립이 이 값을 따른다(규칙 2). 자리표시 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Execute")
	float ExecuteDuration = 0.f;

	/** 앵커 = 타깃 위치 − 타깃 전방(요) × 이 값, cm 단위 — 로드 검사 `>= Radius + 39`(캡슐 반경). 자리표시 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Execute")
	float ExecuteAnchorDistance = 0.f;

	/** 처형 카메라를 붙일 메시 소켓(본 이름 허용) — 없으면 액터 루트(구 콜리전)에 붙는다(2026-09-20 밤 — 메시는 `MeshYawOffset`·`Radius` 만큼 돌고 내려가 있어 메시 루트 기준은 종마다 축이 어긋난다) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Execute")
	FName ExecuteCameraSocket;

	/** 처형 카메라의 상대 위치, cm 단위 — 소켓이 있으면 소켓 공간, 없으면 액터 공간(X = 전방·타깃 쪽, Z = 위) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Execute")
	FVector ExecuteCameraOffset = FVector::ZeroVector;

	/** 처형 카메라의 상대 회전, 도 단위 — 위치와 같은 공간 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Execute")
	FRotator ExecuteCameraRotation = FRotator::ZeroRotator;

	/** 스토킹 접근 속도, cm/s 단위 — 안 보일 때 타깃 등 뒤 지점으로. 자리표시 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Execute")
	float StalkSpeed = 0.f;

	/** "본다" 판정의 시선 반각, 도 단위 — 플레이어 조준 요와 몬스터 방향 요의 차가 이 안이면 본다. 자리표시 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Execute")
	float StalkGazeHalfAngle = 0.f;

	/** "본다" 판정이 유효한 거리, cm 단위. 자리표시 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Execute")
	float StalkGazeRadius = 0.f;

	/** 이만큼 응시당하면 격분해 Chase 로, 초 단위 — 안 보이면 같은 속도로 0 까지 감쇠. 자리표시 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Execute")
	float StalkStareAnger = 0.f;

	/** 보이는 동안 타깃과 이 거리 안이면 `RetreatSpeed` 로 물러난다, cm 단위 — 0 이면 후퇴 없이 정지만. 자리표시 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Execute")
	float StalkRetreatRadius = 0.f;

	/**
	 * 잡는 순간(즉시 잡기·덮침 성공)이 처형이다 — 스토킹 없이. 피해·Catch 휴지기 대신 잡은 자리에서(텔레포트 없음) Execute 로 들어가
	 * ExecuteDuration 뒤 즉사(로드 검사: ExecuteDuration > 0). 자리표시 false
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Execute")
	bool bExecuteOnCatch = false;

	/** 처형이 있는 종인가 — 스토커(ExecuteRadius > 0) 또는 잡히면 처형(bExecuteOnCatch). 치트 `cl9.execute` 의 후보·게이트가 쓴다 */
	bool IsExecutioner() const { return ExecuteRadius > 0.f || bExecuteOnCatch; }

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
