// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9WaveTable.h"
#include "UObject/Class.h"
#include "CL9.h"

namespace CL9Wave
{
	FString ToAlias(ECL9NoiseSource Source)
	{
		// 값 이름의 소문자 스네이크 표기가 문서·표가 부르는 이름이다 (docs/systems/wave.md 소음원 enum) —
		// 목록을 두 벌 두지 않으려고 enum 이름에서 그때그때 만든다
		const FString Name = StaticEnum<ECL9NoiseSource>()->GetNameStringByValue(static_cast<int64>(Source));
		FString Alias;
		for (int32 Index = 0; Index < Name.Len(); ++Index)
		{
			const TCHAR Letter = Name[Index];
			if (Index > 0 && FChar::IsUpper(Letter))
			{
				Alias.AppendChar(TEXT('_'));
			}
			Alias.AppendChar(FChar::ToLower(Letter));
		}
		return Alias;
	}

	bool FromAlias(const FString& Token, ECL9NoiseSource& OutSource)
	{
		// alias("pickup_loud")도 enum 이름("PickupLoud")도 받는다 — 밑줄을 떼고 대소문자를 무시하면 같은 글자다
		const FString Wanted = Token.Replace(TEXT("_"), TEXT("")).ToLower();
		const UEnum* EnumType = StaticEnum<ECL9NoiseSource>();
		for (int32 Index = 0; Index < EnumType->NumEnums() - 1; ++Index)
		{
			const ECL9NoiseSource Value = static_cast<ECL9NoiseSource>(EnumType->GetValueByIndex(Index));
			if (Value == ECL9NoiseSource::None)
			{
				continue;
			}
			if (EnumType->GetNameStringByIndex(Index).ToLower() == Wanted)
			{
				OutSource = Value;
				return true;
			}
		}
		return false;
	}
}

const FCL9WaveInfo* UCL9WaveTable::FindRow(ECL9NoiseSource Source) const
{
	if (DisabledRows.Contains(Source))
	{
		return nullptr;
	}
	return Rows.Find(Source);
}

void UCL9WaveTable::ValidateRows()
{
	DisabledRows.Reset();

	// 전역 상한이 없으면 파티클이 프레임을 먹는다 — 표 전체의 문제라 행을 끄지는 않는다
	if (MaxConcurrent <= 0 || CullDistance <= 0.f)
	{
		UE_LOG(LogCL9, Error, TEXT("'%s': wave table has no limits (MaxConcurrent %d, CullDistance %.0f) — both must be above zero (docs/systems/wave.md 성능 상한)."),
			*GetNameSafe(this), MaxConcurrent, CullDistance);
	}

	for (const TPair<ECL9NoiseSource, FCL9WaveInfo>& Pair : Rows)
	{
		const ECL9NoiseSource Key = Pair.Key;
		const FCL9WaveInfo& Row = Pair.Value;
		const FString Alias = CL9Wave::ToAlias(Key);
		bool bBroken = false;

		if (Row.Speed <= 0.f || Row.RadiusScale <= 0.f)
		{
			UE_LOG(LogCL9, Error, TEXT("wave row '%s': Speed %.0f and RadiusScale %.2f must both be above zero — the ring would never spread."),
				*Alias, Row.Speed, Row.RadiusScale);
			bBroken = true;
		}

		if (Row.RadiusFrom != ECL9NoiseRadiusSource::None && Row.FallbackRadius != 0.f)
		{
			UE_LOG(LogCL9, Error, TEXT("wave row '%s': RadiusFrom is set, so FallbackRadius must be 0 (it is %.0f) — otherwise the table does not say which one wins."),
				*Alias, Row.FallbackRadius);
			bBroken = true;
		}

		if (Row.RadiusFrom == ECL9NoiseRadiusSource::None && Row.FallbackRadius <= 0.f)
		{
			UE_LOG(LogCL9, Error, TEXT("wave row '%s': RadiusFrom is None, so FallbackRadius must be above zero (it is %.0f) — a ring with no radius is nothing."),
				*Alias, Row.FallbackRadius);
			bBroken = true;
		}

		// PackAlarm은 발신한 몬스터의 행에서 읽는다 — 플레이어에게는 읽을 몬스터 행이 없다
		const bool bMonsterSource = Key == ECL9NoiseSource::MonsterProx || Key == ECL9NoiseSource::MonsterCall
			|| Key == ECL9NoiseSource::MonsterPounce;
		if (Row.RadiusFrom == ECL9NoiseRadiusSource::PackAlarm && !bMonsterSource)
		{
			UE_LOG(LogCL9, Error, TEXT("wave row '%s': RadiusFrom is PackAlarm but this noise comes from a player — there is no monster row to read."), *Alias);
			bBroken = true;
		}

		// 링이 플레이어에게 닿으면 공포 발신기가 레이더가 된다 (docs/plan/wave.md 규칙 7)
		if (Key == ECL9NoiseSource::MonsterProx && Row.TriggerRadius > 0.f
			&& Row.RadiusFrom == ECL9NoiseRadiusSource::None
			&& Row.FallbackRadius * Row.RadiusScale >= Row.TriggerRadius)
		{
			UE_LOG(LogCL9, Error, TEXT("wave row '%s': the ring (%.0f cm) reaches the TriggerRadius (%.0f cm) — the sonar would become a radar."),
				*Alias, Row.FallbackRadius * Row.RadiusScale, Row.TriggerRadius);
			bBroken = true;
		}

		if (!FMath::IsNearlyEqual(Row.RadiusScale, 1.f))
		{
			UE_LOG(LogCL9, Warning, TEXT("wave row '%s': RadiusScale is %.2f — the ring no longer matches the ruling radius."), *Alias, Row.RadiusScale);
		}

		if (Row.bEnabled && Row.Niagara.IsNull())
		{
			UE_LOG(LogCL9, Warning, TEXT("wave row '%s': enabled with no Niagara system — it rules nothing and draws nothing."), *Alias);
		}

		if (bBroken)
		{
			DisabledRows.Add(Key);
		}
	}
}
