// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraModifier.h"
#include "CL9HurtLookModifier.generated.h"

class ACL9Character;

namespace CL9HurtLook
{
	/**
	 * 일어나기 구간의 길이, 초 단위 — 넘어짐이 끝나기 전 이 시간 동안 피격 카메라가 0 으로 돌아온다. `AN_Farmer_GetUp` 0.6초
	 * (docs/systems/animation.md 넘어짐 가지)의 사실이지 밸런스 값이 아니라 상수로 둔다. 로드 검사 `HurtLookTime + 이 값 <=
	 * KnockdownTime` 이 진입·복귀 보간이 겹치지 않음을 확인한다 (ACL9Character::ApplyCharacterDef)
	 */
	constexpr float GetUpSeconds = 0.6f;
}

/**
 * 잡히는 순간 카메라가 범인을 본다 (docs/systems/character.md 피격 — 카메라 전용 효과). 소유 클라의 카메라 매니저에 붙어
 * `ModifyCamera` 가 넘어진 동안(`GetKnockdownElapsed() ∈ [0, KnockdownTime)`) **POV 에만** 두 오프셋을 더한다:
 *
 * - 피치: 목표 `CL9Rules.HurtLookPitch` 까지 `HurtLookTime` 동안 EaseOut(도달 보장 — `FInterpTo` 아님).
 * - 요: 피격 사건(`FCL9HurtEvent.From`)의 방향으로, 공격자가 화면 가장자리에 걸칠 때까지만, `HurtLookYawMax` 안에서.
 *
 * 일어나기 구간(넘어짐 끝 `CL9HurtLook::GetUpSeconds`)에 둘 다 0 으로 되돌린다. **컨트롤 회전·`LookInput`·조준은 건드리지
 * 않는다** — 플레이어의 시선은 그대로고 이 효과는 카메라가 잠깐 곁눈질하는 것이다(스태미나 셰이크와 같은 "POV 만" 원칙).
 * 재피격(`Seq` 변화)은 요 목표만 갱신하고 피치 진입은 다시 시작하지 않는다(덜컹 방지). 죽음(넘어짐 끝 = 무한)은 진입 뒤
 * 유지, 관전(뷰 타깃이 남의 폰)에서는 아무것도 하지 않는다. `HurtLookTime` 0 = 끔.
 */
UCLASS()
class UCL9HurtLookModifier final : public UCameraModifier
{
	GENERATED_BODY()

public:
	/** 이 모디파이어가 읽을 폰 — 등록 직후 캐릭터가 한 번 부른다 */
	void SetCharacter(ACL9Character* InCharacter);

	/** 등록된 카메라 매니저 — 캐릭터가 컨트롤러 교체 때 같은 매니저인지 대조한다 */
	APlayerCameraManager* GetCameraOwner() const
	{
		return CameraOwner;
	}

	virtual bool ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV) override;

private:
	/** 오프셋을 전부 0 으로 — 넘어짐이 끝났거나 이 폰을 보고 있지 않을 때 */
	void ResetOffsets();

	TWeakObjectPtr<ACL9Character> Character;

	/** 지난 틱에 반영한 사건 번호 — 달라지면 요 목표를 다시 잰다 */
	uint16 LastSeq = 0;

	/** 요 목표를 잰 넘어짐의 시작 시각 — 새 넘어짐이면 LastSeq 와 무관하게 다시 잰다 */
	float YawKnockdownStart = -1000.f;

	/** 요 목표(도) — 사건 순간의 카메라 요 기준으로 공격자가 가장자리에 걸치는 오프셋, ±HurtLookYawMax */
	float YawTarget = 0.f;

	/** 요 목표를 바꾼 순간의 오프셋과 월드 시각 — 재피격 때 지금 값에서 새 목표로 EaseOut 한다 */
	float YawFrom = 0.f;
	float YawRetargetTime = 0.f;

	/** 지금 POV 에 더하고 있는 오프셋(도) */
	float PitchOffset = 0.f;
	float YawOffset = 0.f;
};
