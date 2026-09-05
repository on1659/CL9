// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Data/CL9Types.h"
#include "CL9GameState.generated.h"

class UCL9RulesDef;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCL9NightPhaseChanged, ECL9NightPhase, NewPhase);

/** Fires on every machine when the offered or accepted orders change — the lobby screen and HUD subscribe (docs/systems/order.md UI) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCL9OrdersChanged);

/**
 * Which site map every machine stands in (docs/systems/map.md 생성 흐름). Only the
 * seed travels; Build ticks on every set so the notify fires even when the same
 * seed is laid twice (and on the first join, whose seed 0 equals the default).
 */
USTRUCT()
struct FCL9SiteMapKey
{
	GENERATED_BODY()

	/** 0 = the fixed shell only (lobby), otherwise the night's maze */
	UPROPERTY()
	uint32 Seed = 0;

	UPROPERTY()
	int32 Build = 0;
};

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

	/**
	 * Host only: arms the night clock — start/end/blackout stamped in server time.
	 * NightLength is the day-scaled length; bNoBlackout leaves the blackout unarmed
	 * (docs/plan/data/progression.md 특정 일자 덮어쓰기).
	 */
	void StartNightClock(const UCL9RulesDef& Rules, float NightLength, bool bNoBlackout);

	/** Host only: zeroes the clock (back at the lobby) */
	void ClearNightClock();

	/** Host only: next day — the lobby between nights carries the new number */
	void AdvanceDay();

	/** Host only: jumps to a day (cl9.day — curve checks without playing the nights through) */
	void SetDay(int32 NewDay);

	/** Host only: the destination this run raids and tonight's danger multiplier (site × curve — docs/systems/balance.md) */
	void SetSiteAndDanger(FName SiteAlias, float NewDangerScale);

	/** Multiplier on chase budget, detection speed and sight tonight — 1 outside a night (docs/systems/monster.md 일자 스케일링) */
	UFUNCTION(BlueprintPure, Category="Night")
	float GetDangerScale() const { return DangerScale; }

	/**
	 * Host only: lays the current site's map from a seed on every machine —
	 * 0 lays the fixed shell only. The site row comes from CurrentSiteAlias,
	 * so set that first (docs/systems/map.md 생성 흐름).
	 */
	void SetSiteMap(uint32 Seed);

	/** Tonight's map seed — 0 outside a night */
	uint32 GetMapSeed() const { return SiteMapKey.Seed; }

	/** Host only: the lobby's draw — N candidates and how many must be taken (docs/systems/order.md 주문 흐름) */
	void SetOfferedOrders(const TArray<FName>& Aliases, int32 Required);

	/** Host only: the confirmed pick replaces the offer; accepted orders accumulate across days until delivered/expired (4e) */
	void SetAcceptedOrders(const TArray<FCL9AcceptedOrder>& Orders);

	/** Local presentation hook — order screen and HUD subscribe (fires on every machine) */
	UPROPERTY(BlueprintAssignable, Category="Orders")
	FCL9OrdersChanged OnOrdersChanged;

	/** Tonight's candidates by alias — empty once picked or outside the lobby */
	UPROPERTY(ReplicatedUsing=OnRep_Orders, BlueprintReadOnly, Category="Orders")
	TArray<FName> OfferedOrders;

	/** How many of the offer must be taken (M) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Orders")
	int32 OrdersRequired = 0;

	/** Orders the run is working on, with their deadlines — run progress, saved (docs/systems/save.md) */
	UPROPERTY(ReplicatedUsing=OnRep_Orders, BlueprintReadOnly, Category="Orders")
	TArray<FCL9AcceptedOrder> AcceptedOrders;

	/** The destination being raided (docs/plan/data/site.md) — the game mode's default until 4g adds a choice */
	UFUNCTION(BlueprintPure, Category="Night")
	FName GetCurrentSiteAlias() const { return CurrentSiteAlias; }

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

	UFUNCTION()
	void OnRep_Orders();

	UFUNCTION()
	void OnRep_SiteMap();

	/** Local: rebuilds the site map for the current site row and seed — the shell alone when the seed is 0 */
	void RebuildSiteMap();

	UPROPERTY(ReplicatedUsing=OnRep_Phase, BlueprintReadOnly, Category="Night")
	ECL9NightPhase Phase = ECL9NightPhase::Lobby;

	/** The map every machine builds locally — never the walls themselves */
	UPROPERTY(ReplicatedUsing=OnRep_SiteMap)
	FCL9SiteMapKey SiteMapKey;

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

	/** Destination row alias for this run — derived runtime state, the row itself stays in each machine's registry */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Night")
	FName CurrentSiteAlias;

	/** Tonight's danger multiplier (site BaseDanger × DangerCurve) — 1 outside a night */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Night")
	float DangerScale = 1.f;

	const UCL9RulesDef* GetRules() const;
};
