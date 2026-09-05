// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9CheatManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "CL9.h"
#include "Character/CL9Character.h"
#include "Data/CL9CharacterDef.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ItemDef.h"
#include "Data/CL9MonsterDef.h"
#include "Data/CL9OrderDef.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameMode.h"
#include "GameMode/CL9GameState.h"
#include "Monsters/CL9Monster.h"

void UCL9CheatManager::HandleCommand(const FString& CommandLine)
{
#if !UE_BUILD_SHIPPING
	TArray<FString> Tokens;
	CommandLine.ParseIntoArrayWS(Tokens);
	if (Tokens.Num() == 0)
	{
		return;
	}

	const FString Command = Tokens[0].ToLower();
	Tokens.RemoveAt(0);

	if (Command == TEXT("additem"))
	{
		AddItem(Tokens);
	}
	else if (Command == TEXT("addmonster"))
	{
		AddMonster(Tokens);
	}
	else if (Command == TEXT("hitmonster"))
	{
		HitMonster(Tokens);
	}
	else if (Command == TEXT("list"))
	{
		ListDefs(Tokens);
	}
	else if (Command == TEXT("night"))
	{
		Night(Tokens);
	}
	else if (Command == TEXT("blackout"))
	{
		Blackout(Tokens);
	}
	else if (Command == TEXT("damage"))
	{
		Damage(Tokens);
	}
	else if (Command == TEXT("day"))
	{
		Day(Tokens);
	}
	else if (Command == TEXT("orders"))
	{
		Orders(Tokens);
	}
	else if (Command == TEXT("pick"))
	{
		Pick(Tokens);
	}
	else if (Command == TEXT("seed"))
	{
		Seed(Tokens);
	}
	else
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.%s: unknown cheat."), *Command);
	}
#endif
}

void UCL9CheatManager::AddItem(const TArray<FString>& Args)
{
	if (Args.Num() == 0)
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.additem <alias|number> [count]"));
		return;
	}

	const UCL9ItemDef* Def = ResolveItem(Args[0]);
	if (Def == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.additem: '%s' not found — see cl9.list items."), *Args[0]);
		return;
	}

	APlayerController* Controller = GetOuterAPlayerController();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	ACL9GameMode* GameMode = GetWorld()->GetAuthGameMode<ACL9GameMode>();
	if (Pawn == nullptr || GameMode == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.additem: needs a pawn and the host game mode."));
		return;
	}

	const int32 Count = Args.IsValidIndex(1) ? FMath::Clamp(FCString::Atoi(*Args[1]), 1, 50) : 1;
	for (int32 Index = 0; Index < Count; ++Index)
	{
		// 200cm ahead just above the feet — dropping from the pawn's center
		// would exceed fragile items' CrackSpeed on landing
		const FVector Forward = Pawn->GetActorForwardVector();
		const FVector Side = Pawn->GetActorRightVector() * (Index - (Count - 1) * 0.5f) * Def->Radius * 3.f;
		const float FloorZ = Pawn->GetActorLocation().Z - Pawn->GetSimpleCollisionHalfHeight();
		const FVector Location = Pawn->GetActorLocation() + Forward * 200.f + Side;
		GameMode->SpawnItem(Def->Alias, Def->SubAlias, FTransform(FVector(Location.X, Location.Y, FloorZ + Def->Radius + 5.f)));
	}
}

void UCL9CheatManager::AddMonster(const TArray<FString>& Args)
{
	if (Args.Num() == 0)
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.addmonster <alias|number> [count]"));
		return;
	}

	const UCL9MonsterDef* Def = ResolveMonster(Args[0]);
	if (Def == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.addmonster: '%s' not found — see cl9.list monsters."), *Args[0]);
		return;
	}

	APlayerController* Controller = GetOuterAPlayerController();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	ACL9GameMode* GameMode = GetWorld()->GetAuthGameMode<ACL9GameMode>();
	if (Pawn == nullptr || GameMode == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.addmonster: needs a pawn and the host game mode."));
		return;
	}

	const int32 Count = Args.IsValidIndex(1) ? FMath::Clamp(FCString::Atoi(*Args[1]), 1, 20) : 1;
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const FVector Forward = Pawn->GetActorForwardVector();
		const FVector Side = Pawn->GetActorRightVector() * (Index - (Count - 1) * 0.5f) * Def->SeparationDist;
		const float FloorZ = Pawn->GetActorLocation().Z - Pawn->GetSimpleCollisionHalfHeight();
		const FVector Location = Pawn->GetActorLocation() + Forward * 300.f + Side;
		GameMode->SpawnMonster(Def->Alias, Def->SubAlias, FTransform(FVector(Location.X, Location.Y, FloorZ + Def->Radius + 5.f)));
	}
}

void UCL9CheatManager::HitMonster(const TArray<FString>& Args)
{
	APlayerController* Controller = GetOuterAPlayerController();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	if (Pawn == nullptr || !Pawn->HasAuthority())
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.hitmonster: needs a pawn on the host."));
		return;
	}

	ACL9Monster* Nearest = nullptr;
	float NearestDistance = TNumericLimits<float>::Max();
	for (TActorIterator<ACL9Monster> It(GetWorld()); It; ++It)
	{
		const float Distance = FVector::Dist2D(Pawn->GetActorLocation(), It->GetActorLocation());
		if (It->GetState() != ECL9MonsterState::Dead && Distance < NearestDistance)
		{
			Nearest = *It;
			NearestDistance = Distance;
		}
	}
	if (Nearest == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.hitmonster: no living monster."));
		return;
	}

	const int32 Damage = Args.IsValidIndex(0) ? FMath::Max(FCString::Atoi(*Args[0]), 1) : 1;
	Nearest->OnDamage(Damage, Pawn);
	UE_LOG(LogCL9, Log, TEXT("cl9.hitmonster: -%d on '%s' (%.0fcm away)."), Damage, *GetNameSafe(Nearest), NearestDistance);
}

void UCL9CheatManager::ListDefs(const TArray<FString>& Args)
{
	const UCL9DefRegistry* Registry = GetRegistry();
	if (Registry == nullptr)
	{
		return;
	}

	const FString Kind = Args.Num() > 0 ? Args[0].ToLower() : TEXT("items");

	if (Kind == TEXT("items"))
	{
		int32 Number = 1;
		for (const UCL9ItemDef* Item : Registry->GetItems())
		{
			const FString Key = Item->SubAlias.IsNone()
				? Item->Alias.ToString()
				: FString::Printf(TEXT("%s+%s"), *Item->Alias.ToString(), *Item->SubAlias.ToString());
			UE_LOG(LogCL9, Log, TEXT("%d: %s"), Number++, *Key);
		}
	}
	else if (Kind == TEXT("monsters"))
	{
		int32 Number = 1;
		for (const UCL9MonsterDef* Monster : Registry->GetMonsters())
		{
			const FString Key = Monster->SubAlias.IsNone()
				? Monster->Alias.ToString()
				: FString::Printf(TEXT("%s+%s"), *Monster->Alias.ToString(), *Monster->SubAlias.ToString());
			UE_LOG(LogCL9, Log, TEXT("%d: %s"), Number++, *Key);
		}
	}
	else if (Kind == TEXT("characters"))
	{
		int32 Number = 1;
		for (const UCL9CharacterDef* Character : Registry->GetCharacters())
		{
			UE_LOG(LogCL9, Log, TEXT("%d: %s"), Number++, *Character->Alias.ToString());
		}
	}
	else
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.list <items|monsters|characters> — other kinds arrive with their tables."));
		return;
	}

	for (const FString& Broken : Registry->GetBrokenRows())
	{
		UE_LOG(LogCL9, Warning, TEXT("%s"), *Broken);
	}
}

const UCL9ItemDef* UCL9CheatManager::ResolveItem(const FString& Token) const
{
	const UCL9DefRegistry* Registry = GetRegistry();
	if (Registry == nullptr)
	{
		return nullptr;
	}

	if (Token.IsNumeric())
	{
		const int32 Number = FCString::Atoi(*Token);
		const TArray<TObjectPtr<const UCL9ItemDef>>& Items = Registry->GetItems();
		return Items.IsValidIndex(Number - 1) ? Items[Number - 1].Get() : nullptr;
	}

	FString AliasPart = Token;
	FString SubAliasPart;
	Token.Split(TEXT("+"), &AliasPart, &SubAliasPart);
	return Registry->FindItem(FName(*AliasPart), SubAliasPart.IsEmpty() ? NAME_None : FName(*SubAliasPart));
}

const UCL9MonsterDef* UCL9CheatManager::ResolveMonster(const FString& Token) const
{
	const UCL9DefRegistry* Registry = GetRegistry();
	if (Registry == nullptr)
	{
		return nullptr;
	}

	if (Token.IsNumeric())
	{
		const int32 Number = FCString::Atoi(*Token);
		const TArray<TObjectPtr<const UCL9MonsterDef>>& MonsterRows = Registry->GetMonsters();
		return MonsterRows.IsValidIndex(Number - 1) ? MonsterRows[Number - 1].Get() : nullptr;
	}

	FString AliasPart = Token;
	FString SubAliasPart;
	Token.Split(TEXT("+"), &AliasPart, &SubAliasPart);
	return Registry->FindMonster(FName(*AliasPart), SubAliasPart.IsEmpty() ? NAME_None : FName(*SubAliasPart));
}

void UCL9CheatManager::Night(const TArray<FString>& Args)
{
	UWorld* World = GetWorld();
	ACL9GameMode* GameMode = World ? World->GetAuthGameMode<ACL9GameMode>() : nullptr;
	ACL9GameState* CL9GameState = World ? World->GetGameState<ACL9GameState>() : nullptr;
	if (GameMode == nullptr || CL9GameState == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.night: host only."));
		return;
	}

	switch (CL9GameState->GetPhase())
	{
	case ECL9NightPhase::Lobby:
		GameMode->StartNight();
		break;
	case ECL9NightPhase::Night:
		GameMode->EndNight();
		break;
	default:
		UE_LOG(LogCL9, Log, TEXT("cl9.night: dawn is running — wait for the lobby."));
		break;
	}
}

void UCL9CheatManager::Blackout(const TArray<FString>& Args)
{
	UWorld* World = GetWorld();
	ACL9GameState* CL9GameState = World ? World->GetGameState<ACL9GameState>() : nullptr;
	if (CL9GameState == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.blackout: host only."));
		return;
	}
	if (CL9GameState->GetPhase() != ECL9NightPhase::Night)
	{
		UE_LOG(LogCL9, Log, TEXT("cl9.blackout: only during the night — cl9.night first."));
		return;
	}
	CL9GameState->ForceBlackout();
}

void UCL9CheatManager::Damage(const TArray<FString>& Args)
{
	// Optional second arg picks another player by join order, so the listen host
	// can down a client's character for spectate checks without that console.
	APlayerController* Controller = GetOuterAPlayerController();
	APawn* Pawn = Controller ? Controller->GetPawn() : nullptr;
	if (Args.IsValidIndex(1))
	{
		const AGameStateBase* State = GetWorld()->GetGameState();
		const int32 Index = FCString::Atoi(*Args[1]);
		const APlayerState* Target = (State != nullptr && State->PlayerArray.IsValidIndex(Index)) ? State->PlayerArray[Index] : nullptr;
		Pawn = Target ? Target->GetPawn() : nullptr;
	}

	ACL9Character* Character = Cast<ACL9Character>(Pawn);
	if (Character == nullptr || !Character->HasAuthority())
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.damage: needs a character on the host (cl9.damage [amount] [player index])."));
		return;
	}

	// Same host entry a catch uses — knockdown, spill, death all follow (docs/systems/character.md)
	const int32 Amount = Args.IsValidIndex(0) ? FMath::Max(FCString::Atoi(*Args[0]), 1) : 1;
	Character->OnDamage(Amount, nullptr);
	UE_LOG(LogCL9, Log, TEXT("cl9.damage: -%d on '%s'."), Amount, *GetNameSafe(Character));
}

void UCL9CheatManager::Day(const TArray<FString>& Args)
{
	UWorld* World = GetWorld();
	ACL9GameState* CL9GameState = World ? World->GetGameState<ACL9GameState>() : nullptr;
	if (CL9GameState == nullptr || !CL9GameState->HasAuthority())
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.day: host only."));
		return;
	}
	if (CL9GameState->GetPhase() != ECL9NightPhase::Lobby)
	{
		UE_LOG(LogCL9, Log, TEXT("cl9.day: only in the lobby — the day is fixed once a night starts."));
		return;
	}

	const int32 NewDay = Args.IsValidIndex(0) ? FCString::Atoi(*Args[0]) : 0;
	if (NewDay < 1)
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.day: usage cl9.day <N> (N >= 1)."));
		return;
	}
	CL9GameState->SetDay(NewDay);
	UE_LOG(LogCL9, Log, TEXT("cl9.day: lobby moved to day %d — cl9.night rolls that day's curves."), NewDay);

	// The offer depends on the day (MinDay/MaxDay) — redraw for the new one
	if (ACL9GameMode* GameMode = World->GetAuthGameMode<ACL9GameMode>())
	{
		GameMode->OfferOrders();
	}
}

const UCL9DefRegistry* UCL9CheatManager::GetRegistry() const
{
	const UWorld* World = GetWorld();
	const UCL9GameInstance* GameInstance = World ? World->GetGameInstance<UCL9GameInstance>() : nullptr;
	return GameInstance ? GameInstance->GetDefRegistry() : nullptr;
}

void UCL9CheatManager::Orders(const TArray<FString>& Args)
{
	const UWorld* World = GetWorld();
	const ACL9GameState* CL9GameState = World ? World->GetGameState<ACL9GameState>() : nullptr;
	const UCL9DefRegistry* Registry = GetRegistry();
	if (CL9GameState == nullptr || Registry == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.orders: no game state or registry."));
		return;
	}

	UE_LOG(LogCL9, Log, TEXT("Offer (day %d, take %d):"), CL9GameState->GetCurrentDay(), CL9GameState->OrdersRequired);
	int32 Number = 1;
	for (const FName& Alias : CL9GameState->OfferedOrders)
	{
		const UCL9OrderDef* Order = Registry->FindOrder(Alias);
		FString Needs;
		if (Order != nullptr)
		{
			for (const FCL9ItemCount& Need : Order->Ingredients)
			{
				Needs += FString::Printf(TEXT("%s%s+%s×%d"), Needs.IsEmpty() ? TEXT("") : TEXT(", "),
					*Need.ItemAlias.ToString(), *Need.ItemSubAlias.ToString(), Need.Count);
			}
		}
		UE_LOG(LogCL9, Log, TEXT("  %d: %s — %s — %d~%d ether, %d day(s)"), Number++, *Alias.ToString(), *Needs,
			Order ? Order->RewardMin : 0, Order ? Order->RewardMax : 0, Order ? Order->DeadlineDays : 0);
	}

	UE_LOG(LogCL9, Log, TEXT("Accepted (%d):"), CL9GameState->AcceptedOrders.Num());
	for (const FCL9AcceptedOrder& Accepted : CL9GameState->AcceptedOrders)
	{
		UE_LOG(LogCL9, Log, TEXT("  %s — accepted day %d, deliver by day %d"), *Accepted.OrderAlias.ToString(), Accepted.AcceptedDay, Accepted.DeadlineDay);
	}
}

void UCL9CheatManager::Pick(const TArray<FString>& Args)
{
	UWorld* World = GetWorld();
	ACL9GameMode* GameMode = World ? World->GetAuthGameMode<ACL9GameMode>() : nullptr;
	if (GameMode == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.pick: host only."));
		return;
	}

	TArray<int32> Indices;
	for (const FString& Arg : Args)
	{
		const int32 Number = FCString::Atoi(*Arg);
		if (Number >= 1)
		{
			Indices.Add(Number - 1);
		}
	}
	if (Indices.Num() == 0)
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.pick: usage cl9.pick <n> [m] … (numbers from cl9.orders)."));
		return;
	}
	if (GameMode->AcceptOrders(Indices))
	{
		UE_LOG(LogCL9, Log, TEXT("cl9.pick: %d order(s) accepted."), Indices.Num());
	}
}

void UCL9CheatManager::Seed(const TArray<FString>& Args)
{
	UWorld* World = GetWorld();
	ACL9GameState* CL9GameState = World ? World->GetGameState<ACL9GameState>() : nullptr;
	ACL9GameMode* GameMode = World ? World->GetAuthGameMode<ACL9GameMode>() : nullptr;
	if (CL9GameState == nullptr || GameMode == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.seed: host only."));
		return;
	}
	if (!Args.IsValidIndex(0))
	{
		UE_LOG(LogCL9, Warning, TEXT("cl9.seed: usage cl9.seed <N> (0 = shell only)."));
		return;
	}

	const uint32 Seed = static_cast<uint32>(FCString::Strtoui64(*Args[0], nullptr, 10));
	GameMode->SetForcedMapSeed(Seed);
	CL9GameState->SetSiteMap(Seed);
	UE_LOG(LogCL9, Log, TEXT("cl9.seed: site rebuilt with seed %u%s%s."), Seed,
		Seed == 0 ? TEXT(" (shell only)") : TEXT(" — the next night uses it too"),
		CL9GameState->GetPhase() == ECL9NightPhase::Night ? TEXT("; animals already out may now stand inside walls") : TEXT(""));
}
