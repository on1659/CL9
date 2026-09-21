// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9MonsterDef.h"

void UCL9MonsterDef::Validate(TArray<FString>& OutErrors) const
{
	Super::Validate(OutErrors);

	if (Radius <= 0.f)
	{
		OutErrors.Add(TEXT("Radius must be positive."));
	}
	if (HpMin < 0 || HpMax < HpMin)
	{
		OutErrors.Add(TEXT("HP range needs 0 <= HpMin <= HpMax."));
	}
	if (SightIn > 0.f && SightIn <= SightOut)
	{
		OutErrors.Add(TEXT("SightIn must exceed SightOut — the hysteresis pair (docs/plan/data/monster/common.md)."));
	}
	if (!FMath::IsNearlyEqual(KillCooldown, RageTime))
	{
		OutErrors.Add(TEXT("KillCooldown must equal RageTime — one knob, not two (docs/plan/economy.md 부등식)."));
	}

	// 잡기 예고 네 열은 전부 유한·비음수 — 음수 대기는 즉시 덮침, 음수 거리는 뒤로 덮침이 되어 표가 뜻하지 않은 규칙을 만든다
	const bool bPounceFinite = FMath::IsFinite(PounceRadius) && FMath::IsFinite(PounceWindup)
		&& FMath::IsFinite(PounceLunge) && FMath::IsFinite(PounceMissCooldown);
	if (!bPounceFinite || PounceRadius < 0.f || PounceWindup < 0.f || PounceLunge < 0.f || PounceMissCooldown < 0.f)
	{
		OutErrors.Add(TEXT("PounceRadius, PounceWindup, PounceLunge and PounceMissCooldown must all be finite and non-negative."));
	}
	// 예고가 켜진 종(CatchRadius > 0 ∧ PounceWindup > 0)은 예고가 잡기보다 먼저 시작돼야 한다 — 아니면 예고 없이 잡히는 거리가 생긴다
	if (CatchRadius > 0.f && PounceWindup > 0.f && PounceRadius < CatchRadius)
	{
		OutErrors.Add(TEXT("PounceRadius must be >= CatchRadius when PounceWindup > 0 — the telegraph has to start before the catch distance (docs/plan/data/monster/common.md 추격)."));
	}

	// 처형 열은 전부 유한·비음수 — 음수 시간은 즉시 처형, 음수 거리는 앞에서 처형이 되어 표가 뜻하지 않은 규칙을 만든다
	const float ExecuteScalars[] = { ExecuteRadius, ExecuteBehindAngle, ExecuteUnseenTime, ExecuteDuration, ExecuteAnchorDistance,
		StalkSpeed, StalkGazeHalfAngle, StalkGazeRadius, StalkStareAnger, StalkRetreatRadius };
	bool bExecuteFinite = FMath::IsFinite(ExecuteCameraOffset.X) && FMath::IsFinite(ExecuteCameraOffset.Y) && FMath::IsFinite(ExecuteCameraOffset.Z)
		&& FMath::IsFinite(ExecuteCameraRotation.Pitch) && FMath::IsFinite(ExecuteCameraRotation.Yaw) && FMath::IsFinite(ExecuteCameraRotation.Roll);
	for (const float Value : ExecuteScalars)
	{
		bExecuteFinite = bExecuteFinite && FMath::IsFinite(Value) && Value >= 0.f;
	}
	if (!bExecuteFinite)
	{
		OutErrors.Add(TEXT("Execute columns (ExecuteRadius, ExecuteBehindAngle, ExecuteUnseenTime, ExecuteDuration, ExecuteAnchorDistance, ExecuteCameraOffset, ExecuteCameraRotation, StalkSpeed, StalkGazeHalfAngle, StalkGazeRadius, StalkStareAnger, StalkRetreatRadius) must all be finite and non-negative."));
	}
	// 처형 종(ExecuteRadius > 0)은 처형 길이·접근 속도·시선 반경이 있어야 하고, 앵커가 타깃 캡슐(반경 39)과 겹치지 않아야 한다 —
	// 0 이면 진입 즉시 죽거나(길이), 영영 못 다가가거나(속도), 아무도 못 보거나(반경), 텔레포트가 사람 몸에 박힌다(앵커)
	if (ExecuteRadius > 0.f)
	{
		if (ExecuteDuration <= 0.f || StalkSpeed <= 0.f || StalkGazeRadius <= 0.f)
		{
			OutErrors.Add(TEXT("ExecuteRadius > 0 needs ExecuteDuration > 0, StalkSpeed > 0 and StalkGazeRadius > 0 (docs/plan/data/monster/common.md 스토킹과 처형)."));
		}
		if (ExecuteAnchorDistance < Radius + 39.f)
		{
			OutErrors.Add(FString::Printf(TEXT("ExecuteAnchorDistance must be >= Radius + 39 (= %.0f) — the anchor sphere must clear the victim capsule (docs/plan/data/monster/common.md 스토킹과 처형)."), Radius + 39.f));
		}
	}
	// 잡히면 처형인 종은 처형 길이가 있어야 한다 — 0 이면 잡는 순간 죽어 처형 연출(카메라·클립)이 없다
	if (bExecuteOnCatch && ExecuteDuration <= 0.f)
	{
		OutErrors.Add(TEXT("bExecuteOnCatch needs ExecuteDuration > 0 — the catch itself is the execution and lasts ExecuteDuration (docs/plan/data/monster/common.md 스토킹과 처형)."));
	}
	// 후퇴가 켜진 종은 후퇴 속도가 있어야 한다 — 0 이면 "물러난다"가 제자리 걸음이다
	if (StalkRetreatRadius > 0.f && RetreatSpeed <= 0.f)
	{
		OutErrors.Add(TEXT("StalkRetreatRadius > 0 needs RetreatSpeed > 0 — the stalker retreats at RetreatSpeed."));
	}
}
