// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9GameState.h"
#include "Net/UnrealNetwork.h"
#include "CL9.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9RulesDef.h"
#include "GameMode/CL9GameInstance.h"

void ACL9GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACL9GameState, ReturnedItems);
	DOREPLIFETIME(ACL9GameState, Phase);
	DOREPLIFETIME(ACL9GameState, CurrentDay);
	DOREPLIFETIME(ACL9GameState, NightStartServerTime);
	DOREPLIFETIME(ACL9GameState, NightEndServerTime);
	DOREPLIFETIME(ACL9GameState, BlackoutServerTime);
	DOREPLIFETIME(ACL9GameState, CurrentSiteAlias);
	DOREPLIFETIME(ACL9GameState, DangerScale);
}

void ACL9GameState::RecordReturn(FName ItemAlias, FName ItemSubAlias)
{
	if (!HasAuthority())
	{
		return;
	}

	FCL9CarriedItem& Returned = ReturnedItems.AddDefaulted_GetRef();
	Returned.ItemAlias = ItemAlias;
	Returned.ItemSubAlias = ItemSubAlias;

	// Each return pulls the blackout forward, floored at DarkMin past night start
	// (docs/plan/data/rules.md 밤 — "반납이 시계를 앞당긴다").
	const UCL9RulesDef* Rules = GetRules();
	if (Rules != nullptr && Phase == ECL9NightPhase::Night && BlackoutServerTime > 0.f)
	{
		const float Floor = NightStartServerTime + Rules->DarkMin;
		BlackoutServerTime = FMath::Max(Floor, BlackoutServerTime - Rules->DarkAccel);
	}

	UE_LOG(LogCL9, Log, TEXT("Jar return #%d: '%s'/'%s'."), ReturnedItems.Num(), *ItemAlias.ToString(), *ItemSubAlias.ToString());
}

void ACL9GameState::SetPhase(ECL9NightPhase NewPhase)
{
	if (!HasAuthority() || Phase == NewPhase)
	{
		return;
	}

	Phase = NewPhase;
	// OnRep never runs on the listen host — fire the local hook directly.
	OnRep_Phase();
}

void ACL9GameState::StartNightClock(const UCL9RulesDef& Rules, float NightLength, bool bNoBlackout)
{
	if (!HasAuthority())
	{
		return;
	}

	NightStartServerTime = GetServerWorldTimeSeconds();
	NightEndServerTime = NightStartServerTime + NightLength;
	// 0 keeps IsBlackout false all night; ForceBlackout (cl9.blackout) can still arm it
	BlackoutServerTime = bNoBlackout ? 0.f : NightStartServerTime + Rules.DarkBase;
}

void ACL9GameState::SetDay(int32 NewDay)
{
	if (!HasAuthority() || NewDay < 1)
	{
		return;
	}

	CurrentDay = NewDay;
}

void ACL9GameState::SetSiteAndDanger(FName SiteAlias, float NewDangerScale)
{
	if (!HasAuthority())
	{
		return;
	}

	CurrentSiteAlias = SiteAlias;
	DangerScale = FMath::Max(NewDangerScale, UE_KINDA_SMALL_NUMBER);
}

void ACL9GameState::ClearNightClock()
{
	if (!HasAuthority())
	{
		return;
	}

	NightStartServerTime = 0.f;
	NightEndServerTime = 0.f;
	BlackoutServerTime = 0.f;
	ReturnedItems.Reset();
}

void ACL9GameState::AdvanceDay()
{
	if (!HasAuthority())
	{
		return;
	}

	++CurrentDay;
}

void ACL9GameState::ForceBlackout()
{
	if (!HasAuthority() || Phase != ECL9NightPhase::Night)
	{
		return;
	}

	BlackoutServerTime = GetServerWorldTimeSeconds();
	UE_LOG(LogCL9, Log, TEXT("Blackout forced (cl9.blackout)."));
}

float ACL9GameState::GetNightRemaining() const
{
	if (Phase != ECL9NightPhase::Night || NightEndServerTime <= 0.f)
	{
		return 0.f;
	}
	return FMath::Max(0.f, NightEndServerTime - GetServerWorldTimeSeconds());
}

bool ACL9GameState::IsBlackout() const
{
	return Phase == ECL9NightPhase::Night
		&& BlackoutServerTime > 0.f
		&& GetServerWorldTimeSeconds() >= BlackoutServerTime;
}

void ACL9GameState::OnRep_Phase()
{
	OnNightPhaseChanged.Broadcast(Phase);
}

const UCL9RulesDef* ACL9GameState::GetRules() const
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	return Registry ? Registry->GetRules() : nullptr;
}
