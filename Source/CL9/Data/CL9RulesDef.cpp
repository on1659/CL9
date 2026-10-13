// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9RulesDef.h"
#include "Engine/World.h"

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
	// 0 이하면 밤 몬스터 큐가 영영 비지 않는다
	if (MonsterSpawnPerFrame < 1)
	{
		OutErrors.Add(TEXT("MonsterSpawnPerFrame must be at least 1 — the night monster queue would never drain."));
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
	// 판 중간 스폰: 음수·NaN 이면 타이머 간격·거리·총량이 정해지지 않는다(0 = 끔)
	if (!(FMath::IsFinite(MidSpawnInterval) && MidSpawnInterval >= 0.f && FMath::IsFinite(MidSpawnMinDistance) && MidSpawnMinDistance >= 0.f
		&& FMath::IsFinite(MidSpawnCapRatio) && MidSpawnCapRatio >= 0.f))
	{
		OutErrors.Add(TEXT("MidSpawnInterval, MidSpawnMinDistance and MidSpawnCapRatio must be finite and non-negative (0 interval or ratio = no mid-round spawns)."));
	}
	// 귀가 보상(바깥 재화): 비율이라 0~1
	if (!(FMath::IsFinite(WalletIncomeRate) && WalletIncomeRate >= 0.f && WalletIncomeRate <= 1.f))
	{
		OutErrors.Add(TEXT("WalletIncomeRate must be within 0..1 (share of the round's income paid out as out-game currency)."));
	}
	// 임포스터: 확률은 0~1, 보상은 음수가 아니다
	bool bImposterChancesValid = true;
	for (const float Chance : ImposterChanceByPlayers)
	{
		bImposterChancesValid &= FMath::IsFinite(Chance) && Chance >= 0.f && Chance <= 1.f;
	}
	if (!bImposterChancesValid || ImposterWinReward < 0 || ImposterKillReward < 0)
	{
		OutErrors.Add(TEXT("ImposterChanceByPlayers entries must be within 0..1 and ImposterWinReward/ImposterKillReward non-negative."));
	}
	// 판 시작 연출·마당: 시간·거리·밝기는 음수가 아니고, 울음 간격은 최솟값 ≤ 최댓값
	const bool bArrivalValid = FMath::IsFinite(ArrivalTime) && ArrivalTime >= 0.f && FMath::IsFinite(ArrivalDistance) && ArrivalDistance >= 0.f
		&& !ArrivalCameraOffset.ContainsNaN();
	const bool bYardValid = FMath::IsFinite(PrePowerFogScale) && PrePowerFogScale >= 0.f && YardLampHeight >= 0.f && YardLampIntensity >= 0.f
		&& YardLampRadius >= 0.f && YardCryIntervalMin >= 0.f && YardCryIntervalMax >= YardCryIntervalMin
		&& SilhouetteScale >= 0.f && SilhouetteSpeed >= 0.f && SilhouetteRange >= 0.f && SilhouetteBackLight >= 0.f;
	if (!bArrivalValid || !bYardValid)
	{
		OutErrors.Add(TEXT("Arrival*/Yard*/Silhouette*/PrePowerFogScale must be finite and non-negative, with YardCryIntervalMin <= YardCryIntervalMax."));
	}
	// 회사: 정산 미달 패널티는 비율이라 0~1
	if (!(FMath::IsFinite(QuotaMissPenaltyRate) && QuotaMissPenaltyRate >= 0.f && QuotaMissPenaltyRate <= 1.f))
	{
		OutErrors.Add(TEXT("QuotaMissPenaltyRate must be within 0..1 (share of the run's coins taken when a settlement misses its target)."));
	}
	// 스테이지 별: 기준은 오름차순 양수(아니면 별 2 개가 별 1 개보다 쉬워진다), 여는 별 수는 1..기준 개수(넘으면 아무것도 안 열린다)
	bool bStarsValid = !StageStarRatios.IsEmpty();
	for (int32 Index = 0; Index < StageStarRatios.Num(); ++Index)
	{
		const float Ratio = StageStarRatios[Index];
		bStarsValid &= FMath::IsFinite(Ratio) && Ratio > 0.f && (Index == 0 || Ratio >= StageStarRatios[Index - 1]);
	}
	if (!bStarsValid || StageUnlockStars < 1 || StageUnlockStars > StageStarRatios.Num())
	{
		OutErrors.Add(TEXT("StageStarRatios must be a non-empty ascending list of positive numbers and StageUnlockStars within 1..its length."));
	}
	// 처치 기여: 시간은 0 이상(0 = 끔), 유도 보상은 음수가 아니다
	if (!(FMath::IsFinite(ContributionWindow) && ContributionWindow >= 0.f) || ImposterAssistReward < 0)
	{
		OutErrors.Add(TEXT("ContributionWindow must be finite and non-negative (0 = off) and ImposterAssistReward non-negative."));
	}
	// 전멸 패널티: 비율이라 0~1 — 1 을 넘으면 없는 돈을 깎으려 하고, NaN 이면 차감액이 정해지지 않는다
	if (!(FMath::IsFinite(WipePenaltyRate) && WipePenaltyRate >= 0.f && WipePenaltyRate <= 1.f))
	{
		OutErrors.Add(TEXT("WipePenaltyRate must be within 0..1 (share of the run's ether taken on a wipe)."));
	}
}

int32 UCL9RulesDef::WipePenaltyFor(int32 Money) const
{
	// 이더는 정수다 — 내림(2026-10-12 사용자). 비율을 만분율 정수로 반올림한 뒤 정수로 곱하고 나눈다:
	// float 0.7 은 0.69999999 라 그대로 곱해 내리면 10 이더의 70% 가 6 이 되고, 0.3 은 0.30000001 이라 큰 잔액에서 1 이더씩 넘친다
	const int64 Basis = FMath::RoundToInt64(FMath::Clamp(static_cast<double>(WipePenaltyRate), 0.0, 1.0) * 10000.0);
	return static_cast<int32>(static_cast<int64>(FMath::Max(Money, 0)) * Basis / 10000);
}

int32 UCL9RulesDef::WalletIncomeFor(int32 Income) const
{
	// 전멸 패널티와 같은 계산 — 비율을 만분율 정수로 반올림한 뒤 정수로 곱하고 내린다
	const int64 Basis = FMath::RoundToInt64(FMath::Clamp(static_cast<double>(WalletIncomeRate), 0.0, 1.0) * 10000.0);
	return static_cast<int32>(static_cast<int64>(FMath::Max(Income, 0)) * Basis / 10000);
}

float UCL9RulesDef::ImposterChanceFor(int32 Players) const
{
	if (Players <= 0 || ImposterChanceByPlayers.IsEmpty())
	{
		return 0.f;
	}
	const int32 Index = FMath::Min(Players, ImposterChanceByPlayers.Num()) - 1;
	return FMath::Clamp(ImposterChanceByPlayers[Index], 0.f, 1.f);
}

int32 UCL9RulesDef::QuotaMissPenaltyFor(int32 Money) const
{
	// 전멸 패널티와 같은 계산 — 비율을 만분율 정수로 반올림한 뒤 정수로 곱하고 내린다
	const int64 Basis = FMath::RoundToInt64(FMath::Clamp(static_cast<double>(QuotaMissPenaltyRate), 0.0, 1.0) * 10000.0);
	return static_cast<int32>(static_cast<int64>(FMath::Max(Money, 0)) * Basis / 10000);
}

int32 UCL9RulesDef::StageStarsFor(int32 Earned, int32 Target) const
{
	// 코인은 정수다 — 기준을 만분율 정수로 반올림해 정수로 견준다(1.5 × 800 = 1200 이 float 오차로 1199.99 가 되지 않게)
	int32 Stars = 0;
	for (const float Ratio : StageStarRatios)
	{
		const int64 Basis = FMath::RoundToInt64(FMath::Max(static_cast<double>(Ratio), 0.0) * 10000.0);
		if (static_cast<int64>(Earned) * 10000 < static_cast<int64>(FMath::Max(Target, 0)) * Basis)
		{
			break;
		}
		++Stars;
	}
	return Stars;
}

bool UCL9RulesDef::IsCompanyWorld(const UWorld* World) const
{
	if (World == nullptr || CompanyLevel.IsNull())
	{
		return false;
	}
	return UWorld::RemovePIEPrefix(World->GetOutermost()->GetName()) == CompanyLevel.ToSoftObjectPath().GetLongPackageName();
}
