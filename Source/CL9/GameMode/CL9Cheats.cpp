// Copyright Epic Games, Inc. All Rights Reserved.

// cl9.* console command registry (docs/systems/cheats.md). Dotted names cannot
// be exec functions, so each command registers here and forwards into the
// cheat manager — locally for read-only commands, via the player controller's
// Server RPC when a client console asks for a host-side ruling.

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "CL9.h"
#include "GameMode/CL9CheatManager.h"
#include "GameMode/CL9PlayerController.h"

#if !UE_BUILD_SHIPPING

namespace
{
	void RouteCL9Cheat(const FString& Command, const TArray<FString>& Args, UWorld* World, bool bLocalOnly)
	{
		if (World == nullptr)
		{
			return;
		}
		APlayerController* Controller = World->GetFirstPlayerController();
		if (Controller == nullptr)
		{
			return;
		}

		FString CommandLine = Command;
		for (const FString& Arg : Args)
		{
			CommandLine += TEXT(" ") + Arg;
		}

		if (Controller->HasAuthority() || bLocalOnly)
		{
			if (UCL9CheatManager* Cheats = Cast<UCL9CheatManager>(Controller->CheatManager))
			{
				Cheats->HandleCommand(CommandLine);
			}
			else
			{
				UE_LOG(LogCL9, Warning, TEXT("cl9.%s: cheat manager missing — cheats disabled here."), *Command);
			}
		}
		else if (ACL9PlayerController* CL9Controller = Cast<ACL9PlayerController>(Controller))
		{
			CL9Controller->Server_CL9Cheat(CommandLine);
		}
	}
}

static FAutoConsoleCommandWithWorldAndArgs GCL9CheatAddItem(
	TEXT("cl9.additem"),
	TEXT("cl9.additem <alias|number> [count] — spawns items in front of the player (host ruling)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		RouteCL9Cheat(TEXT("additem"), Args, World, false);
	}));

static FAutoConsoleCommandWithWorldAndArgs GCL9CheatAddMonster(
	TEXT("cl9.addmonster"),
	TEXT("cl9.addmonster <alias|number> [count] — spawns monsters ahead of the player (host ruling)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		RouteCL9Cheat(TEXT("addmonster"), Args, World, false);
	}));

static FAutoConsoleCommandWithWorldAndArgs GCL9CheatHitMonster(
	TEXT("cl9.hitmonster"),
	TEXT("cl9.hitmonster [damage] — damages the nearest monster (host ruling; pan lands in ④)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		RouteCL9Cheat(TEXT("hitmonster"), Args, World, false);
	}));

static FAutoConsoleCommandWithWorldAndArgs GCL9CheatNight(
	TEXT("cl9.night"),
	TEXT("cl9.night — starts the night from the lobby, ends it during the night (host ruling)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		RouteCL9Cheat(TEXT("night"), Args, World, false);
	}));

static FAutoConsoleCommandWithWorldAndArgs GCL9CheatBlackout(
	TEXT("cl9.blackout"),
	TEXT("cl9.blackout — pulls the blackout to now during the night (host ruling)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		RouteCL9Cheat(TEXT("blackout"), Args, World, false);
	}));

static FAutoConsoleCommandWithWorldAndArgs GCL9CheatDamage(
	TEXT("cl9.damage"),
	TEXT("cl9.damage [amount] [player index] — hurts your own character, or player N by join order (host ruling; death/spectate/truck pad checks)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		RouteCL9Cheat(TEXT("damage"), Args, World, false);
	}));

static FAutoConsoleCommandWithWorldAndArgs GCL9CheatList(
	TEXT("cl9.list"),
	TEXT("cl9.list <items|characters> — numbered rows with broken ones marked (local, read-only)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		RouteCL9Cheat(TEXT("list"), Args, World, true);
	}));

#endif
