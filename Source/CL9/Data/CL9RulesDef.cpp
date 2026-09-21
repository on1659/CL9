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
	// 0이면 반경 0 스윕이라 아무것도 못 맞히고, 음수·NaN 은 구를 만들 수 없다 — 값의 문제가 아니라 오류다
	if (!(FMath::IsFinite(SwingHitScale) && SwingHitScale > 0.f))
	{
		OutErrors.Add(TEXT("SwingHitScale must be a finite positive number — it scales the swing hit sphere radius (1 = mesh bounds)."));
	}
	// 0이면 플래시·비네트가 한 프레임도 보이지 않는다 — "맞았다"를 알리는 표현이 통째로 사라지므로 값의 문제가 아니라 오류다
	if (HitFlashTime <= 0.f || HurtFlashTime <= 0.f || HurtVignetteTime <= 0.f)
	{
		OutErrors.Add(TEXT("HitFlashTime, HurtFlashTime and HurtVignetteTime must be positive — a zero makes the hit feedback invisible."));
	}
	// 피격 카메라: 시간·요 상한은 음수가 될 수 없다(0 은 끔). `HurtLookTime + 0.6 <= KnockdownTime` 은 캐릭터 행이 보이는
	// ACL9Character::ApplyCharacterDef 가 Warning 으로 검사한다
	if (!FMath::IsFinite(HurtLookTime) || HurtLookTime < 0.f || !FMath::IsFinite(HurtLookPitch)
		|| !FMath::IsFinite(HurtLookYawMax) || HurtLookYawMax < 0.f)
	{
		OutErrors.Add(TEXT("HurtLookTime and HurtLookYawMax must be finite and non-negative, HurtLookPitch finite (0 time = hurt look off)."));
	}
	// 처형 카메라 블렌드: 음수·NaN 은 블렌드가 아니다(0 = 즉시)
	if (!FMath::IsFinite(ExecuteCameraBlend) || ExecuteCameraBlend < 0.f)
	{
		OutErrors.Add(TEXT("ExecuteCameraBlend must be finite and non-negative (0 = instant cut to the execution camera)."));
	}
	// 데스캠: 자리·높이는 유한이면 된다(오프셋 (0,0,0) = 데스캠 없음), 블렌드는 음수·NaN 이 아니어야(0 = 즉시)
	if (DeathCameraOffset.ContainsNaN() || !FMath::IsFinite(DeathCameraLookAtZ)
		|| !FMath::IsFinite(DeathCameraBlend) || DeathCameraBlend < 0.f)
	{
		OutErrors.Add(TEXT("DeathCameraOffset and DeathCameraLookAtZ must be finite, DeathCameraBlend finite and non-negative (offset (0,0,0) = no death camera)."));
	}
}
