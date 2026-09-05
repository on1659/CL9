// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CL9GameMode.generated.h"

class ACL9Character;
class ACL9Item;
class ACL9Monster;
class ACL9PlayerController;
class UCL9RulesDef;

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

protected:
	/** Dawn → Lobby with the day advanced — the stop that hosts the next night (docs/plan/loop.md 정차) */
	void EnterLobby();

	/** Host only: SpectateDelay ran out — the view rides the first living teammate */
	void BeginSpectating(ACL9PlayerController* Spectator);

	/** The living player pawn after `After` in join order (wrapping), never `Except`; null when nobody lives */
	ACL9Character* FindLivingPawn(const AActor* Except, const AActor* After) const;

	/** Host only: every dead player stands up on the truck pad with full HP and stops spectating */
	void ReviveDead();

	const UCL9RulesDef* GetRules() const;

	FTimerHandle NightExpiryHandle;
	FTimerHandle DawnHandle;

	/**
	 * Character row every player spawns as, set in the BP child. Interim until
	 * the lobby carries a per-player selection in PlayerState
	 * (docs/systems/character.md "캐릭터 Def 적용 경로").
	 */
	UPROPERTY(EditDefaultsOnly, Category="Character")
	FName DefaultCharacterAlias;

	/** The shared item BP — used whenever a row leaves ActorClass empty */
	UPROPERTY(EditDefaultsOnly, Category="Classes")
	TSubclassOf<ACL9Item> ItemClass;

	/** The shared monster BP — used whenever a row leaves ActorClass empty */
	UPROPERTY(EditDefaultsOnly, Category="Classes")
	TSubclassOf<ACL9Monster> MonsterClass;
};
