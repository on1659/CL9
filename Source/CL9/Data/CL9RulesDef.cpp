// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9RulesDef.h"

namespace
{
	/**
	 * 휘두르기 클립이 ABP에서 도는 구간의 길이, 초 단위 — `ABP_Unarmed`·`ABP_FP_View`가 `SwingElapsed ∈ [0, 0.347)`
	 * 동안만 시퀀스 평가기를 태운다(30fps 0.467초 클립을 0.12초 지점부터, docs/systems/animation.md). 애님 에셋의 사실이지
	 * 밸런스 값이 아니라 여기 상수로 둔다 — 히트 구간이 이 밖으로 나가지 않는지만 검사한다.
	 */
	constexpr float SwingClipWindow = 0.347f;
}

void UCL9RulesDef::Validate(TArray<FString>& OutErrors) const
{
	Super::Validate(OutErrors);

	if (PickRadius <= 0.f || CarryHeight <= 0.f)
	{
		OutErrors.Add(TEXT("PickRadius and CarryHeight must be positive."));
	}
	// 활성 칸 번호가 uint8로 복제된다 — 그 범위를 넘는 칸 수는 표가 아니라 코드를 바꿔야 하는 일이다
	if (QuickSlots < 1 || QuickSlots > 255)
	{
		OutErrors.Add(TEXT("QuickSlots must be 1..255 — the quick slot bar needs at least one slot."));
	}
	if (!(ThrowMinSpeed > 0.f && ThrowMinSpeed <= ThrowMaxSpeed) || ThrowChargeTime <= 0.f)
	{
		OutErrors.Add(TEXT("Throw needs 0 < ThrowMinSpeed <= ThrowMaxSpeed and a positive ThrowChargeTime."));
	}
	// 0이면 우클 탭이 곧 먹기가 되어 스캔이 사라진다
	if (UseHoldTime <= 0.f)
	{
		OutErrors.Add(TEXT("UseHoldTime must be positive — it separates a right-click tap (scan) from a hold (eat)."));
	}
	// 히트 구간은 휘두르기 클립이 도는 동안(ABP 평가 범위 [0, SwingClipWindow))에 들어 있어야 한다 — 밖이면 애님이 끝난 뒤나
	// 시작 전에 맞는 "보이지 않는 타격"이 된다 (docs/systems/animation.md 휘두르기 가지)
	if (!(SwingHitStart >= 0.f && SwingHitStart < SwingHitEnd && SwingHitEnd < SwingClipWindow))
	{
		OutErrors.Add(FString::Printf(TEXT("Swing hit window needs 0 <= SwingHitStart < SwingHitEnd < %.3f (the ABP plays the swing clip only in that range)."), SwingClipWindow));
	}
}
