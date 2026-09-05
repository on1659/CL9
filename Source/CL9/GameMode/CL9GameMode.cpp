// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9GameMode.h"
#include "CL9.h"
#include "Character/CL9Character.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ItemDef.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameState.h"
#include "GameMode/CL9PlayerController.h"
#include "GameMode/CL9PlayerState.h"
#include "Items/CL9Item.h"
#include "Components/CapsuleComponent.h"
#include "Data/CL9MonsterDef.h"
#include "Data/CL9OrderDef.h"
#include "Data/CL9ProgressionDef.h"
#include "Data/CL9RulesDef.h"
#include "Data/CL9SiteDef.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Monsters/CL9Monster.h"
#include "TimerManager.h"
#include "World/CL9TruckPad.h"

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

	// Day curves scale the rules' night and the site's population (docs/plan/data/progression.md);
	// a missing progression asset means no scaling, a missing site means an empty night.
	const int32 Day = CL9GameState->GetCurrentDay();
	const UCL9ProgressionDef* Progression = GetProgression();
	const float NightLength = Rules->NightLength * (Progression ? Progression->NightLengthAt(Day) : 1.f);
	const bool bNoBlackout = Progression != nullptr && Progression->IsBlackoutDisabled(Day);

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9SiteDef* Site = Registry ? Registry->FindSite(DefaultSiteAlias) : nullptr;
	if (Site == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("StartNight: site row '%s' is missing or broken — no animals or ingredients tonight (assign DefaultSiteAlias in the game mode BP)."),
			*DefaultSiteAlias.ToString());
	}
	const float Danger = (Site ? Site->BaseDanger : 1.f) * (Progression ? Progression->DangerAt(Day) : 1.f);
	CL9GameState->SetSiteAndDanger(Site ? Site->Alias : NAME_None, Danger);

	CL9GameState->StartNightClock(*Rules, NightLength, bNoBlackout);
	CL9GameState->SetPhase(ECL9NightPhase::Night);
	GetWorldTimerManager().SetTimer(NightExpiryHandle, this, &ACL9GameMode::EndNight, NightLength);

	UE_LOG(LogCL9, Log, TEXT("Night %d started — %.0fs to dawn, blackout %s, danger ×%.2f."),
		Day, NightLength, bNoBlackout ? TEXT("off (day override)") : *FString::Printf(TEXT("base %.0fs"), Rules->DarkBase), Danger);

	if (Site != nullptr)
	{
		SpawnNightPopulation(*Site, Progression, Day);
	}
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
	ClearNightPopulation();
	CL9GameState->SetSiteAndDanger(CL9GameState->GetCurrentSiteAlias(), 1.f);
	ReviveDead();
	CL9GameState->SetPhase(ECL9NightPhase::Lobby);
	UE_LOG(LogCL9, Log, TEXT("Lobby — day %d."), CL9GameState->GetCurrentDay());
	OfferOrders();
}

void ACL9GameMode::HandlePlayerDeath(ACL9Character* Dead)
{
	if (Dead == nullptr)
	{
		return;
	}

	// Wipe — nobody left standing, the truck leaves the party (docs/plan/data/rules.md 실패 축)
	if (FindLivingPawn(nullptr, nullptr) == nullptr)
	{
		UE_LOG(LogCL9, Log, TEXT("Wipe — everyone is down, the truck leaves."));
		const ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
		if (CL9GameState != nullptr && CL9GameState->GetPhase() == ECL9NightPhase::Night)
		{
			EndNight();
		}
		return;
	}

	// Anyone watching the one who just died moves on to a living teammate
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ACL9PlayerController* Spectator = Cast<ACL9PlayerController>(It->Get());
		if (Spectator != nullptr && Spectator->IsSpectating() && Spectator->GetViewTarget() == Dead)
		{
			SpectateNext(Spectator);
		}
	}

	ACL9PlayerController* DeadController = Cast<ACL9PlayerController>(Dead->GetController());
	if (DeadController == nullptr)
	{
		return;
	}

	const UCL9RulesDef* Rules = GetRules();
	const float Delay = Rules ? Rules->SpectateDelay : 0.f;
	if (Delay <= 0.f)
	{
		BeginSpectating(DeadController);
		return;
	}

	// Fire-and-forget: BeginSpectating re-checks that the player is still dead,
	// so a lobby revive during the delay simply makes it a no-op.
	TWeakObjectPtr<ACL9PlayerController> WeakController = DeadController;
	FTimerHandle Unused;
	GetWorldTimerManager().SetTimer(Unused, FTimerDelegate::CreateWeakLambda(this, [this, WeakController]()
	{
		BeginSpectating(WeakController.Get());
	}), Delay, false);
}

void ACL9GameMode::SpectateNext(ACL9PlayerController* Spectator)
{
	if (Spectator == nullptr || !Spectator->IsSpectating())
	{
		return;
	}

	AActor* Current = Spectator->GetViewTarget();
	ACL9Character* Next = FindLivingPawn(Spectator->GetPawn(), Current);
	if (Next != nullptr && Next != Current)
	{
		Spectator->SetViewTarget(Next);
	}
}

void ACL9GameMode::BeginSpectating(ACL9PlayerController* Spectator)
{
	ACL9Character* Own = Spectator ? Cast<ACL9Character>(Spectator->GetPawn()) : nullptr;
	if (Own == nullptr || !Own->IsDead())
	{
		return;
	}

	// Nobody to watch (the wipe already ended the night) — the floor view stays
	ACL9Character* Target = FindLivingPawn(Own, nullptr);
	if (Target == nullptr)
	{
		return;
	}

	// Server-side SetViewTarget reaches the owning client through ClientSetViewTarget;
	// the flag's OnRep opens the screen and IMC there (docs/systems/ui_system.md)
	Spectator->SetViewTarget(Target);
	Spectator->HostSetSpectating(true);
	UE_LOG(LogCL9, Log, TEXT("'%s' now spectates '%s'."), *GetNameSafe(Own), *GetNameSafe(Target));
}

ACL9Character* ACL9GameMode::FindLivingPawn(const AActor* Except, const AActor* After) const
{
	if (GameState == nullptr)
	{
		return nullptr;
	}

	TArray<ACL9Character*> Living;
	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
		ACL9Character* Character = PlayerState ? Cast<ACL9Character>(PlayerState->GetPawn()) : nullptr;
		if (Character != nullptr && Character != Except && !Character->IsDead())
		{
			Living.Add(Character);
		}
	}
	if (Living.Num() == 0)
	{
		return nullptr;
	}

	// INDEX_NONE + 1 lands on the first — `After` missing (null, or just died) starts over
	const int32 AfterIndex = Living.IndexOfByPredicate([After](const ACL9Character* Candidate)
	{
		return Candidate == After;
	});
	return Living[(AfterIndex + 1) % Living.Num()];
}

void ACL9GameMode::ReviveDead()
{
	if (GameState == nullptr)
	{
		return;
	}

	// The pad is where the party wakes up (docs/plan/loop.md 귀환) — none placed means in place
	const ACL9TruckPad* Pad = nullptr;
	for (TActorIterator<ACL9TruckPad> It(GetWorld()); It; ++It)
	{
		Pad = *It;
		break;
	}

	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		ACL9Character* Character = PlayerState ? Cast<ACL9Character>(PlayerState->GetPawn()) : nullptr;
		if (Character != nullptr && Character->IsDead())
		{
			Character->HostRevive();
			if (Pad != nullptr)
			{
				const float HalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
				const FVector Scatter(FMath::FRandRange(-60.f, 60.f), FMath::FRandRange(-60.f, 60.f), HalfHeight + 5.f);
				Character->SetActorLocationAndRotation(Pad->GetActorLocation() + Scatter, Pad->GetActorRotation(),
					false, nullptr, ETeleportType::TeleportPhysics);
			}
		}

		ACL9PlayerController* Controller = PlayerState ? Cast<ACL9PlayerController>(PlayerState->GetPlayerController()) : nullptr;
		if (Controller != nullptr && Controller->IsSpectating())
		{
			Controller->SetViewTarget(Controller->GetPawn());
			Controller->HostSetSpectating(false);
		}
	}
}

const UCL9RulesDef* ACL9GameMode::GetRules() const
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	return Registry ? Registry->GetRules() : nullptr;
}

const UCL9ProgressionDef* ACL9GameMode::GetProgression() const
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	return Registry ? Registry->GetProgression() : nullptr;
}

void ACL9GameMode::SpawnNightPopulation(const UCL9SiteDef& Site, const UCL9ProgressionDef* Progression, int32 Day)
{
	if (!HasAuthority())
	{
		return;
	}

	// Count = roll in [Min, Max] × the day multiplier, rounded (docs/systems/balance.md).
	// The roll is per entry, not per animal — the whole flock scales together.
	auto RollCount = [](const FCL9SpawnEntry& Entry, float Multiplier)
	{
		return FMath::Max(0, FMath::RoundToInt(FMath::RandRange(Entry.MinCount, Entry.MaxCount) * Multiplier));
	};

	const float MonsterMult = Progression ? Progression->MonsterCountAt(Day) : 1.f;
	const float ItemMult = Progression ? Progression->ItemCountAt(Day) : 1.f;

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;

	// Flat-level stand-in for the generator's coordinates: animals farther out
	// than the truck's calm radius, ingredients closer in (⑤ replaces this).
	for (const FCL9SpawnEntry& Entry : Site.MonsterSpawns)
	{
		const int32 Count = RollCount(Entry, MonsterMult);
		const UCL9MonsterDef* Def = Registry ? Registry->FindMonster(Entry.Alias) : nullptr;
		const float Radius = Def ? Def->Radius : 50.f;
		int32 Placed = 0;
		for (int32 Index = 0; Index < Count; ++Index)
		{
			FVector Location;
			if (!FindSpawnPoint(900.f, 2400.f, Location))
			{
				continue;
			}
			Location.Z += Radius;
			if (ACL9Monster* Monster = SpawnMonster(Entry.Alias, NAME_None, FTransform(FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f), Location)))
			{
				NightSpawned.Add(Monster);
				++Placed;
			}
		}
		UE_LOG(LogCL9, Log, TEXT("Night %d: %s ×%d placed (base %d~%d × %.2f = %d)."),
			Day, *Entry.Alias.ToString(), Placed, Entry.MinCount, Entry.MaxCount, MonsterMult, Count);
	}

	for (const FCL9SpawnEntry& Entry : Site.ItemSpawns)
	{
		const int32 Count = RollCount(Entry, ItemMult);
		int32 Placed = 0;
		for (int32 Index = 0; Index < Count; ++Index)
		{
			FVector Location;
			if (!FindSpawnPoint(400.f, 1800.f, Location))
			{
				continue;
			}
			if (ACL9Item* Item = SpawnItem(Entry.Alias, NAME_None, FTransform(Location + FVector(0.f, 0.f, 30.f))))
			{
				NightSpawned.Add(Item);
				++Placed;
			}
		}
		UE_LOG(LogCL9, Log, TEXT("Night %d: %s ×%d placed (base %d~%d × %.2f = %d)."),
			Day, *Entry.Alias.ToString(), Placed, Entry.MinCount, Entry.MaxCount, ItemMult, Count);
	}
}

void ACL9GameMode::ClearNightPopulation()
{
	if (!HasAuthority())
	{
		return;
	}

	// The site leaves with the night — what the spawner placed and nobody
	// carried out is gone. Carried items detached from the spawner's list by
	// being attached to a character or worn, so those survive.
	int32 Removed = 0;
	for (AActor* Actor : NightSpawned)
	{
		if (!IsValid(Actor))
		{
			continue;
		}
		if (const ACL9Item* Item = Cast<ACL9Item>(Actor); Item != nullptr && !Item->IsFree())
		{
			continue;
		}
		Actor->Destroy();
		++Removed;
	}
	NightSpawned.Reset();
	UE_LOG(LogCL9, Log, TEXT("Night population cleared — %d actors removed."), Removed);
}

bool ACL9GameMode::FindSpawnPoint(float MinDistance, float MaxDistance, FVector& OutLocation) const
{
	// The truck pad is the site's anchor on the flat level; a PlayerStart stands in when none is placed
	FVector Anchor = FVector::ZeroVector;
	bool bAnchorFound = false;
	for (TActorIterator<ACL9TruckPad> It(GetWorld()); It; ++It)
	{
		Anchor = It->GetActorLocation();
		bAnchorFound = true;
		break;
	}
	if (!bAnchorFound)
	{
		for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
		{
			Anchor = It->GetActorLocation();
			break;
		}
	}

	const UCL9RulesDef* Rules = GetRules();
	const float CalmRadius = Rules ? Rules->ThreatActMinRadius : 420.f;

	for (int32 Attempt = 0; Attempt < 12; ++Attempt)
	{
		const float Bearing = FMath::FRandRange(0.f, 2.f * UE_PI);
		const float Distance = FMath::FRandRange(MinDistance, MaxDistance);
		const FVector Candidate = Anchor + FVector(FMath::Cos(Bearing), FMath::Sin(Bearing), 0.f) * Distance;

		// Nothing wakes or spawns inside ThreatActMinRadius of a player (docs/plan/data/rules.md)
		bool bTooClose = false;
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			const APawn* Pawn = It->Get() ? It->Get()->GetPawn() : nullptr;
			if (Pawn != nullptr && FVector::Dist2D(Pawn->GetActorLocation(), Candidate) < CalmRadius)
			{
				bTooClose = true;
				break;
			}
		}
		if (bTooClose)
		{
			continue;
		}

		// Ground it: trace down from above; no floor there means try elsewhere
		FHitResult Hit;
		const FVector Start = Candidate + FVector(0.f, 0.f, 500.f);
		const FVector End = Candidate - FVector(0.f, 0.f, 500.f);
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
		{
			OutLocation = Hit.ImpactPoint + FVector(0.f, 0.f, 5.f);
			return true;
		}
	}
	return false;
}

void ACL9GameMode::StartPlay()
{
	Super::StartPlay();

	// The first lobby needs its offer too — later lobbies draw in EnterLobby
	OfferOrders();
}

void ACL9GameMode::OfferOrders()
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	if (!HasAuthority() || CL9GameState == nullptr || CL9GameState->GetPhase() != ECL9NightPhase::Lobby)
	{
		return;
	}

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9ProgressionDef* Progression = GetProgression();
	if (Registry == nullptr || Progression == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("OfferOrders: no registry or progression row — nothing to offer."));
		CL9GameState->SetOfferedOrders(TArray<FName>(), 0);
		return;
	}

	const int32 Day = CL9GameState->GetCurrentDay();
	TArray<const UCL9OrderDef*> Candidates;
	for (const UCL9OrderDef* Order : Registry->GetOrders())
	{
		if (Order->IsOfferedOn(Day))
		{
			Candidates.Add(Order);
		}
	}

	// Weighted draw without repeats — N from the progression row (docs/systems/order.md 주문 흐름)
	TArray<FName> Offer;
	const int32 Wanted = FMath::Min(Progression->OrdersOfferedPerDay, Candidates.Num());
	while (Offer.Num() < Wanted && Candidates.Num() > 0)
	{
		float Total = 0.f;
		for (const UCL9OrderDef* Candidate : Candidates)
		{
			Total += Candidate->Weight;
		}
		float Roll = FMath::FRandRange(0.f, Total);
		int32 PickedIndex = Candidates.Num() - 1;
		for (int32 Index = 0; Index < Candidates.Num(); ++Index)
		{
			Roll -= Candidates[Index]->Weight;
			if (Roll <= 0.f)
			{
				PickedIndex = Index;
				break;
			}
		}
		Offer.Add(Candidates[PickedIndex]->Alias);
		Candidates.RemoveAt(PickedIndex);
	}

	const int32 Required = FMath::Min(Progression->OrdersRequiredPerDay, Offer.Num());
	CL9GameState->SetOfferedOrders(Offer, Required);

	FString Names;
	for (const FName& Alias : Offer)
	{
		Names += (Names.IsEmpty() ? TEXT("") : TEXT(", ")) + Alias.ToString();
	}
	UE_LOG(LogCL9, Log, TEXT("Day %d offer: %d orders [%s] — take %d (cl9.orders / cl9.pick)."), Day, Offer.Num(), *Names, Required);
}

bool ACL9GameMode::AcceptOrders(const TArray<int32>& OfferedIndices)
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	if (!HasAuthority() || CL9GameState == nullptr)
	{
		return false;
	}
	if (CL9GameState->GetPhase() != ECL9NightPhase::Lobby)
	{
		UE_LOG(LogCL9, Warning, TEXT("AcceptOrders: orders are picked in the lobby only."));
		return false;
	}

	const TArray<FName>& Offer = CL9GameState->OfferedOrders;
	const int32 Required = CL9GameState->OrdersRequired;
	if (Offer.Num() == 0)
	{
		UE_LOG(LogCL9, Warning, TEXT("AcceptOrders: nothing is on offer (already picked, or no order rows for this day)."));
		return false;
	}

	// Exactly M distinct picks, all inside the offer — the choice is the day's plan
	const TSet<int32> Unique(OfferedIndices);
	if (Unique.Num() != OfferedIndices.Num() || OfferedIndices.Num() != Required)
	{
		UE_LOG(LogCL9, Warning, TEXT("AcceptOrders: need exactly %d distinct picks, got %d (%d distinct)."), Required, OfferedIndices.Num(), Unique.Num());
		return false;
	}
	for (int32 Index : OfferedIndices)
	{
		if (!Offer.IsValidIndex(Index))
		{
			UE_LOG(LogCL9, Warning, TEXT("AcceptOrders: index %d is outside the offer of %d."), Index, Offer.Num());
			return false;
		}
	}

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const int32 Day = CL9GameState->GetCurrentDay();

	TArray<FCL9AcceptedOrder> Accepted = CL9GameState->AcceptedOrders;
	for (int32 Index : OfferedIndices)
	{
		const UCL9OrderDef* Order = Registry ? Registry->FindOrder(Offer[Index]) : nullptr;
		if (Order == nullptr)
		{
			UE_LOG(LogCL9, Error, TEXT("AcceptOrders: offered order '%s' has no usable row — pick refused."), *Offer[Index].ToString());
			return false;
		}
		FCL9AcceptedOrder& Entry = Accepted.AddDefaulted_GetRef();
		Entry.OrderAlias = Order->Alias;
		Entry.AcceptedDay = Day;
		Entry.DeadlineDay = Day + Order->DeadlineDays - 1;
		UE_LOG(LogCL9, Log, TEXT("Order accepted: %s — deliver by day %d."), *Order->Alias.ToString(), Entry.DeadlineDay);
	}

	CL9GameState->SetAcceptedOrders(Accepted);
	// The offer is spent — the next lobby draws again
	CL9GameState->SetOfferedOrders(TArray<FName>(), 0);
	return true;
}
