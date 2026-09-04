// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9CheatManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "CL9.h"
#include "Data/CL9CharacterDef.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ItemDef.h"
#include "Data/CL9MonsterDef.h"
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

const UCL9DefRegistry* UCL9CheatManager::GetRegistry() const
{
	const UWorld* World = GetWorld();
	const UCL9GameInstance* GameInstance = World ? World->GetGameInstance<UCL9GameInstance>() : nullptr;
	return GameInstance ? GameInstance->GetDefRegistry() : nullptr;
}
