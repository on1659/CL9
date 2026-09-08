// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "CL9CharacterDef.generated.h"

class UAnimInstance;
class USkeletalMesh;
class USkeleton;

/**
 * 플레이 가능한 캐릭터 한 행 (docs/plan/data/character.md). 신체 수치와
 * 스태미나는 모든 머신이 자신의 로컬 레지스트리에서 읽는 판정 값이고,
 * 메시와 색상은 연출용이다. 아래 기본값은 문서화된 테이블 기본값을
 * 그대로 반영한 것이며, 실제로 게임이 읽는 것은 저장된 애셋이다.
 */
UCLASS()
class UCL9CharacterDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	// -- 외형 (연출 전용) --

	/** 다른 플레이어에게 보이는 전신 메시. 베이스의 Mesh 컬럼은 아이템용 StaticMesh라서 별도 필드로 둔다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;

	/** 전신 메시와 애니메이션이 공유하는 스켈레톤 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TSoftObjectPtr<USkeleton> Skeleton;

	/** 전신 애니메이션 블루프린트 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TSoftClassPtr<UAnimInstance> AnimBP;

	/** 1인칭 시점 메시 — 소유자에게만 보인다 (docs/systems/animation.md 1인칭 팔) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TSoftObjectPtr<USkeletalMesh> ArmsMesh;

	/** 1인칭 시점 애니메이션 블루프린트 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	TSoftClassPtr<UAnimInstance> ArmsAnimBP;

	/** 슬롯 색상 — 어둠 속에서 플레이어를 구분할 유일한 수단이므로 채도를 높게 유지한다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Appearance")
	FLinearColor TintColor = FLinearColor::White;

	// -- 신체 (판정 값) --

	/** 걷기 속도 (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float WalkSpeed = 396.f;

	/** 달리기 속도 (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float RunSpeed = 654.f;

	/** 소지 무게에 곱하는 배율 — 1보다 작으면 더 가볍게 든다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float CarryMult = 1.f;

	/** 카트를 끄는 동안의 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float CartMult = 0.75f;

	/** 캡슐 반지름 (cm) — 복도 폭 255cm가 이 값을 기준으로 정해졌다. 메시가 캡슐에 맞춰야지 그 반대는 안 된다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float CapsuleRadius = 39.f;

	/** 캡슐 절반 높이 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float CapsuleHalfHeight = 90.f;

	/** 바닥 기준 1인칭 카메라 높이 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float EyeHeight = 156.f;

	/** 점프 초기 속도 (cm/s) — 아래 기본 중력값 기준으로 정점 높이는 156cm */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float JumpVelocity = 780.f;

	/** 공중에 떠 있는 동안의 중력 (cm/s²) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Body")
	float JumpGravity = 1950.f;

	// -- 체력 --

	/** 최소 체력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	int32 HpMin = 3;

	/** 최대 체력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	int32 HpMax = 3;

	/** 완충된 피격 후 경직 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	float HurtStun = 0.35f;

	/** 붙잡혔을 때 쓰러진 채 움직이지 못하는 시간 (초) (초안 — docs/plan/data/character.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health")
	float KnockdownTime = 1.5f;

	// -- 스태미나 (0..1로 정규화됨. docs/systems/character.md의 세 조항 모두 이 값들에 달려 있다) --

	/** 달리는 동안 가득 찬 상태에서 0이 될 때까지 걸리는 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stamina")
	float StaminaDrainTime = 6.f;

	/** 달리지 않는 동안 0에서 가득 찰 때까지 걸리는 시간 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stamina")
	float StaminaRegenTime = 8.f;

	/**
	 * 탈진 상태는 스태미나가 이 값을 넘어 회복해야만 풀린다 — 경계값에서 깜빡이지 않게 하는
	 * 히스테리시스이자, 1.0이면 **만충까지 잠그는** 문턱이다. 0~1 정규화 값이다.
	 *
	 * 기본값이 1.0인 것은 2026-09-08 결정이다(decisions.md — "0이 되면 lock이 걸려서 풀로
	 * 올라갈 때까지 못 쓰게 막힘"). 그때는 표 값만 올리고 이 기본값을 0.25로 두었는데, 그러면
	 * 새로 만든 캐릭터 행이 조용히 옛 동작으로 시작한다 — 그래서 여기까지 맞췄다 (2026-09-09).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stamina")
	float StaminaMinResume = 1.f;
};
