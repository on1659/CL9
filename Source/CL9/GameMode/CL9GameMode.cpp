// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9GameMode.h"
#include "CL9.h"
#include "Character/CL9Character.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ItemDef.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameState.h"
#include "GameMode/CL9PlayerState.h"
#include "Items/CL9Item.h"
#include "Data/CL9MonsterDef.h"
#include "Data/CL9RulesDef.h"
#include "Monsters/CL9Monster.h"
#include "TimerManager.h"

ACL9GameMode::ACL9GameMode()
{
	GameStateClass = ACL9GameState::StaticClass();
	PlayerStateClass = ACL9PlayerState::StaticClass();
}

void ACL9GameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	if (ACL9Character* Character = Cast<ACL9Character>(PlayerPawn))
	{
		Character->SetCharacterAlias(DefaultCharacterAlias);
	}
}

bool ACL9GameMode::AllowCheats(APlayerController* P)
{
#if UE_BUILD_SHIPPING
	return Super::AllowCheats(P);
#else
	// Packaged development builds default to no cheats — cl9.* commands are the
	// step-verification tool, so they stay on outside shipping (docs/systems/cheats.md).
	return true;
#endif
}

ACL9Item* ACL9GameMode::SpawnItem(FName ItemAlias, FName ItemSubAlias, const FTransform& Transform)
{
	if (!HasAuthority())
	{
		return nullptr;
	}

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9ItemDef* Def = Registry ? Registry->FindItem(ItemAlias, ItemSubAlias) : nullptr;
	if (Def == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("SpawnItem: item row '%s'/'%s' is missing or broken — nothing spawned."),
			*ItemAlias.ToString(), *ItemSubAlias.ToString());
		return nullptr;
	}

	UClass* SpawnClass = ItemClass;
	if (!Def->ActorClass.IsNull())
	{
		UClass* DedicatedClass = Def->ActorClass.LoadSynchronous();
		if (DedicatedClass != nullptr && DedicatedClass->IsChildOf(ACL9Item::StaticClass()))
		{
			SpawnClass = DedicatedClass;
		}
		else
		{
			UE_LOG(LogCL9, Error, TEXT("SpawnItem: '%s' ActorClass does not derive from ACL9Item — using the shared BP."), *ItemAlias.ToString());
		}
	}
	if (SpawnClass == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("SpawnItem: no item class — assign ItemClass in the game mode BP."));
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ACL9Item* Item = GetWorld()->SpawnActor<ACL9Item>(SpawnClass, Transform, SpawnParameters);
	if (Item != nullptr)
	{
		Item->SetAlias(ItemAlias, ItemSubAlias);
	}
	return Item;
}

ACL9Monster* ACL9GameMode::SpawnMonster(FName MonsterAlias, FName MonsterSubAlias, const FTransform& Transform)
{
	if (!HasAuthority())
	{
		return nullptr;
	}

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9MonsterDef* Def = Registry ? Registry->FindMonster(MonsterAlias, MonsterSubAlias) : nullptr;
	if (Def == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("SpawnMonster: monster row '%s'/'%s' is missing or broken — nothing spawned."),
			*MonsterAlias.ToString(), *MonsterSubAlias.ToString());
		return nullptr;
	}

	UClass* SpawnClass = MonsterClass;
	if (!Def->ActorClass.IsNull())
	{
		UClass* DedicatedClass = Def->ActorClass.LoadSynchronous();
		if (DedicatedClass != nullptr && DedicatedClass->IsChildOf(ACL9Monster::StaticClass()))
		{
			SpawnClass = DedicatedClass;
		}
		else
		{
			UE_LOG(LogCL9, Error, TEXT("SpawnMonster: '%s' ActorClass does not derive from ACL9Monster — using the shared BP."), *MonsterAlias.ToString());
		}
	}
	if (SpawnClass == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("SpawnMonster: no monster class — assign MonsterClass in the game mode BP."));
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ACL9Monster* Monster = GetWorld()->SpawnActor<ACL9Monster>(SpawnClass, Transform, SpawnParameters);
	if (Monster != nullptr)
	{
		Monster->SetAlias(MonsterAlias, MonsterSubAlias);
	}
	return Monster;
}

void ACL9GameMode::StartNight()
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	if (CL9GameState == nullptr || CL9GameState->GetPhase() != ECL9NightPhase::Lobby)
	{
		UE_LOG(LogCL9, Warning, TEXT("StartNight: only the lobby can start a night."));
		return;
	}

	const UCL9RulesDef* Rules = GetRules();
	if (Rules == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("StartNight: no usable rules row — night not started."));
		return;
	}

	CL9GameState->StartNightClock(*Rules);
	CL9GameState->SetPhase(ECL9NightPhase::Night);
	GetWorldTimerManager().SetTimer(NightExpiryHandle, this, &ACL9GameMode::EndNight, Rules->NightLength);

	UE_LOG(LogCL9, Log, TEXT("Night %d started — %.0fs to dawn, blackout base %.0fs."),
		CL9GameState->GetCurrentDay(), Rules->NightLength, Rules->DarkBase);
}

void ACL9GameMode::EndNight()
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	if (CL9GameState == nullptr || CL9GameState->GetPhase() != ECL9NightPhase::Night)
	{
		UE_LOG(LogCL9, Warning, TEXT("EndNight: no night is running."));
		return;
	}

	GetWorldTimerManager().ClearTimer(NightExpiryHandle);
	CL9GameState->SetPhase(ECL9NightPhase::Dawn);
	UE_LOG(LogCL9, Log, TEXT("Night %d over — dawn."), CL9GameState->GetCurrentDay());

	// DawnLength is undecided (0) — until it lands, dawn passes straight through.
	const UCL9RulesDef* Rules = GetRules();
	const float DawnLength = Rules ? Rules->DawnLength : 0.f;
	if (DawnLength > 0.f)
	{
		GetWorldTimerManager().SetTimer(DawnHandle, this, &ACL9GameMode::EnterLobby, DawnLength);
	}
	else
	{
		EnterLobby();
	}
}

void ACL9GameMode::EnterLobby()
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	if (CL9GameState == nullptr)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(DawnHandle);
	CL9GameState->ClearNightClock();
	CL9GameState->AdvanceDay();
	CL9GameState->SetPhase(ECL9NightPhase::Lobby);
	UE_LOG(LogCL9, Log, TEXT("Lobby — day %d."), CL9GameState->GetCurrentDay());
}

const UCL9RulesDef* ACL9GameMode::GetRules() const
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	return Registry ? Registry->GetRules() : nullptr;
}
