// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CL9GameMode.generated.h"

class ACL9Character;
class ACL9Item;
class ACL9Jar;
class ACL9Monster;
class ACL9PlayerController;
class UCL9ProgressionDef;
class UCL9RulesDef;
class UCL9SiteDef;

/**
 * Base game mode. Exists on the listen host only — authoritative game-flow
 * logic goes here; state that clients need goes to the game state instead
 * (conventions § 2). DefaultPawnClass is set in the BP child so C++ holds
 * no asset references.
 */
UCLASS(abstract)
class ACL9GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACL9GameMode();

	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;
	virtual bool AllowCheats(APlayerController* P) override;

	/**
	 * Host only: spawns one item and runs it through the standard data path
	 * (SetAlias → replication → LoadData). Honors the row's ActorClass column
	 * — a dedicated BP when set, the shared item BP otherwise (docs/plan/data/README.md 규칙 2).
	 */
	ACL9Item* SpawnItem(FName ItemAlias, FName ItemSubAlias, const FTransform& Transform);

	/** Host only: spawns one monster through the standard data path — same ActorClass rule as items */
	ACL9Monster* SpawnMonster(FName MonsterAlias, FName MonsterSubAlias, const FTransform& Transform);

	/** Host only: Lobby → Night — arms the clock and schedules the expiry (docs/plan/data/rules.md 밤) */
	void StartNight();

	/** Host only: Night → Dawn now. Expiry and the wipe both land here */
	void EndNight();

	/**
	 * Host only: a character's HP hit 0 (docs/systems/character.md 죽음). Nobody
	 * left standing is the wipe — the truck leaves and the night ends. Otherwise
	 * the dead player rides a living teammate's view after SpectateDelay.
	 * 부활은 없다 — the next lobby revives (ReviveDead).
	 */
	void HandlePlayerDeath(ACL9Character* Dead);

	/** Host only: moves a spectator's view to the next living teammate (docs/systems/input.md 관전 대상 전환) */
	void SpectateNext(ACL9PlayerController* Spectator);

	/**
	 * Host only, lobby: draws the day's order candidates — MinDay/MaxDay filter,
	 * weighted draw without repeats, N and M from the progression row
	 * (docs/systems/order.md 주문 흐름). Runs at start, every lobby and cl9.day.
	 */
	void OfferOrders();

	/**
	 * Host only, lobby: confirms the pick — exactly M distinct indices into the
	 * offer. Accepted orders carry a deadline day and stay until delivered or
	 * expired (4e). Returns false when the pick is refused.
	 */
	bool AcceptOrders(const TArray<int32>& OfferedIndices);

	/** Pins the next night's map seed (cl9.seed) — 0 rolls a random one again */
	void SetForcedMapSeed(uint32 Seed) { ForcedMapSeed = Seed; }

protected:
	virtual void StartPlay() override;
	/** Dawn → Lobby with the day advanced — the stop that hosts the next night (docs/plan/loop.md 정차) */
	void EnterLobby();

	/** Host only: SpectateDelay ran out — the view rides the first living teammate */
	void BeginSpectating(ACL9PlayerController* Spectator);

	/** The living player pawn after `After` in join order (wrapping), never `Except`; null when nobody lives */
	ACL9Character* FindLivingPawn(const AActor* Except, const AActor* After) const;

	/** Host only: every dead player stands up on the truck pad with full HP and stops spectating */
	void ReviveDead();

	/**
	 * Host only, night start: rolls the site's baselines × the day curves and
	 * spawns the night's animals and ingredients (docs/systems/raiding.md 스폰).
	 * No map generator yet (⑤) — the flat level scatters them around the truck.
	 */
	void SpawnNightPopulation(const UCL9SiteDef& Site, const UCL9ProgressionDef* Progression, int32 Day);

	/** Host only, lobby: the site vanishes with the night — everything the spawner placed goes (docs/plan/data/equipment.md 원정과 분실) */
	void ClearNightPopulation();

	/** A ground spot at a random bearing and distance from the truck, outside ThreatActMinRadius of every player */
	bool FindSpawnPoint(float MinDistance, float MaxDistance, FVector& OutLocation) const;

	/** True inside ThreatActMinRadius of any player — nothing wakes or spawns there (docs/plan/data/rules.md) */
	bool IsInsideCalmRadius(const FVector& Location) const;

	/** Seed pinned by cl9.seed for the next night; 0 = roll one */
	uint32 ForcedMapSeed = 0;

	const UCL9RulesDef* GetRules() const;
	const UCL9ProgressionDef* GetProgression() const;

	FTimerHandle NightExpiryHandle;
	FTimerHandle DawnHandle;

	/** What the night spawner placed — cleared at the next lobby; cheat spawns and level props stay */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> NightSpawned;

	/**
	 * Character row every player spawns as, set in the BP child. Interim until
	 * the lobby carries a per-player selection in PlayerState
	 * (docs/systems/character.md "캐릭터 Def 적용 경로").
	 */
	UPROPERTY(EditDefaultsOnly, Category="Character")
	FName DefaultCharacterAlias;

	/** Site row every night raids, set in the BP child — interim until 4g adds the destination choice */
	UPROPERTY(EditDefaultsOnly, Category="Night")
	FName DefaultSiteAlias;

	/** The shared item BP — used whenever a row leaves ActorClass empty */
	UPROPERTY(EditDefaultsOnly, Category="Classes")
	TSubclassOf<ACL9Item> ItemClass;

	/** The shared monster BP — used whenever a row leaves ActorClass empty */
	UPROPERTY(EditDefaultsOnly, Category="Classes")
	TSubclassOf<ACL9Monster> MonsterClass;

	/** Return jar BP the night spawner places on the generator's jar cells (docs/systems/map.md 항아리) */
	UPROPERTY(EditDefaultsOnly, Category="Classes")
	TSubclassOf<ACL9Jar> JarClass;
};
