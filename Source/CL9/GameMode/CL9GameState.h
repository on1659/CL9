// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Data/CL9Types.h"
#include "CL9GameState.generated.h"

class UCL9RulesDef;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCL9NightPhaseChanged, ECL9NightPhase, NewPhase);

/**
 * Shared runtime state (conventions § 4·5) — static tables never replicate,
 * only what changes during a night. Transition rulings live in the game mode;
 * this class only carries the results.
 */
UCLASS()
class ACL9GameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Host only: records one jar return — during the night it also pulls the blackout forward (docs/plan/data/rules.md 밤) */
	void RecordReturn(FName ItemAlias, FName ItemSubAlias);

	/** Host only: flips the phase and fires the local change hook (clients fire it via OnRep) */
	void SetPhase(ECL9NightPhase NewPhase);

	/** Host only: arms the night clock from the rules — start/end/blackout stamped in server time */
	void StartNightClock(const UCL9RulesDef& Rules);

	/** Host only: zeroes the clock (back at the lobby) */
	void ClearNightClock();

	/** Host only: next day — the lobby between nights carries the new number */
	void AdvanceDay();

	/** Host only: pull the blackout to now (cl9.blackout — verification without waiting out the clock) */
	void ForceBlackout();

	/** Seconds until night expiry — 0 outside the night */
	UFUNCTION(BlueprintPure, Category="Night")
	float GetNightRemaining() const;

	/** True once the blackout moment passed this night */
	UFUNCTION(BlueprintPure, Category="Night")
	bool IsBlackout() const;

	UFUNCTION(BlueprintPure, Category="Night")
	ECL9NightPhase GetPhase() const { return Phase; }

	UFUNCTION(BlueprintPure, Category="Night")
	int32 GetCurrentDay() const { return CurrentDay; }

	/** Local presentation hook — lighting/UI subscribe here (fires on every machine) */
	UPROPERTY(BlueprintAssignable, Category="Night")
	FCL9NightPhaseChanged OnNightPhaseChanged;

	/** Everything returned to jars this night, in return order */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Night")
	TArray<FCL9CarriedItem> ReturnedItems;

protected:
	UFUNCTION()
	void OnRep_Phase();

	UPROPERTY(ReplicatedUsing=OnRep_Phase, BlueprintReadOnly, Category="Night")
	ECL9NightPhase Phase = ECL9NightPhase::Lobby;

	/** Day of the host's run — starts at 1 (docs/systems/session.md) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Night")
	int32 CurrentDay = 1;

	/** Server time the night started (0 outside the night) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Night")
	float NightStartServerTime = 0.f;

	/** Server time the night expires (0 outside the night) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Night")
	float NightEndServerTime = 0.f;

	/** Server time the blackout hits — returns pull it forward, never below DarkMin (0 outside the night) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Night")
	float BlackoutServerTime = 0.f;

	const UCL9RulesDef* GetRules() const;
};
