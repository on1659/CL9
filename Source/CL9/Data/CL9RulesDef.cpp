// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9RulesDef.h"

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
	// 헤더 0 = 미설정. 옛 메시 스윕 경로는 없으므로 둘 다 셀이 있어야 하고, 선분이 앞을 향하려면 Reach >= Radius
	if (!(FMath::IsFinite(SwingHitReach) && SwingHitReach > 0.f && FMath::IsFinite(SwingHitRadius) && SwingHitRadius > 0.f
		&& SwingHitReach >= SwingHitRadius))
	{
		OutErrors.Add(TEXT("SwingHitReach and SwingHitRadius must be set (finite, > 0, Reach >= Radius) — header 0 means unconfigured; there is no mesh-sweep fallback."));
	}
	// 회복은 팔 클립이 도는 동안(SwingClipWindow)은 이어져야 하고, 히트 구간이 끝난 뒤여야 한다 — 아니면 다음 휘두르기의 시작 시각
	// 리셋이 팔 애님 도중이나 히트 구간 도중에 떨어진다
	if (!(FMath::IsFinite(SwingRecovery) && SwingRecovery >= SwingClipWindow && SwingHitEnd < SwingRecovery))
	{
		OutErrors.Add(FString::Printf(TEXT("SwingRecovery needs SwingRecovery >= %.3f (the arm clip window) and SwingHitEnd < SwingRecovery."), SwingClipWindow));
	}
	// 예약 창은 0(끔) 이상이면 된다
	if (!FMath::IsFinite(SwingBufferTime) || SwingBufferTime < 0.f)
	{
		OutErrors.Add(TEXT("SwingBufferTime must be finite and non-negative (0 = no input buffer)."));
	}
	// 넉백 속도 = 거리 / 시간 — 0 이면 무한 속도(순간이동)라 값의 문제가 아니라 오류다
	if (!(FMath::IsFinite(HitKnockbackTime) && HitKnockbackTime > 0.f))
	{
		OutErrors.Add(TEXT("HitKnockbackTime must be a finite positive number — knockback speed is HitKnockback / HitKnockbackTime."));
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
