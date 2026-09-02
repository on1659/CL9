// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9CheatManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "CL9.h"
#include "Data/CL9CharacterDef.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ItemDef.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameMode.h"

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
	else if (Command == TEXT("list"))
	{
		ListDefs(Tokens);
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
		UE_LOG(LogCL9, Warning, TEXT("cl9.list <items|characters> — other kinds arrive with their tables."));
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

const UCL9DefRegistry* UCL9CheatManager::GetRegistry() const
{
	const UWorld* World = GetWorld();
	const UCL9GameInstance* GameInstance = World ? World->GetGameInstance<UCL9GameInstance>() : nullptr;
	return GameInstance ? GameInstance->GetDefRegistry() : nullptr;
}
