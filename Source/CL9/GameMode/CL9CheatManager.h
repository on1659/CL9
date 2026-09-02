// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "CL9CheatManager.generated.h"

class UCL9DefRegistry;
class UCL9ItemDef;
class UCL9MonsterDef;

/**
 * cl9.* console cheats (docs/systems/cheats.md) — the step-verification tool:
 * one cheat = one member function plus one registration line in CL9Cheats.cpp.
 * Spawning cheats run on the host only; client consoles reach here through
 * ACL9PlayerController::Server_CL9Cheat. Cheat spawns ride the standard data
 * path, so a cheat test doubles as a network test.
 */
UCLASS()
class UCL9CheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	/** Splits "additem egg 2" style lines and dispatches to the cheat functions */
	void HandleCommand(const FString& CommandLine);

	/** cl9.additem <alias|번호> [수량] — spawns items in front of this controller's pawn */
	void AddItem(const TArray<FString>& Args);

	/** cl9.addmonster <alias|번호> [수량] — spawns monsters ahead, AI live immediately */
	void AddMonster(const TArray<FString>& Args);

	/** cl9.list <items|monsters|characters> — numbered rows plus broken ones with reasons */
	void ListDefs(const TArray<FString>& Args);

private:
	/** Resolves "egg", "egg+cooked" or a cl9.list number to a usable item row */
	const UCL9ItemDef* ResolveItem(const FString& Token) const;

	/** Resolves an alias(+sub) or a cl9.list number to a usable monster row */
	const UCL9MonsterDef* ResolveMonster(const FString& Token) const;

	const UCL9DefRegistry* GetRegistry() const;
};
