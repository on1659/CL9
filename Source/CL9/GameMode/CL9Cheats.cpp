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
#include "Data/CL9DefRegistry.h"
#include "Data/CL9SiteDef.h"
#include "GameMode/CL9CheatManager.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9PlayerController.h"
#include "World/CL9SiteMap.h"

#if !UE_BUILD_SHIPPING

namespace
{
#if WITH_EDITOR
	/**
	 * The 5a map tool without a window: in the editor (no PIE) cl9.seed lays the
	 * site into the level as transient, editor-only actors — the same generator
	 * the night runs, so what you inspect is what players get (build-order 5a).
	 */
	void LayEditorSiteMap(const TArray<FString>& Args, UWorld* World)
	{
		if (!Args.IsValidIndex(0))
		{
			UE_LOG(LogCL9, Warning, TEXT("cl9.seed: usage cl9.seed <N> [site alias] — 0 = shell only, 'clear' removes the map."));
			return;
		}
		if (Args[0].Equals(TEXT("clear"), ESearchCase::IgnoreCase))
		{
			ACL9SiteMap::Clear(World);
			UE_LOG(LogCL9, Log, TEXT("cl9.seed: editor site map removed."));
			return;
		}

		UCL9DefRegistry* Registry = UCL9GameInstance::CreateStandaloneRegistry(GetTransientPackage());
		if (Registry == nullptr)
		{
			return;
		}
		const UCL9SiteDef* Site = nullptr;
		if (Args.IsValidIndex(1))
		{
			Site = Registry->FindSite(FName(*Args[1]));
		}
		else if (Registry->GetSites().Num() > 0)
		{
			Site = Registry->GetSites()[0].Get();
		}
		if (Site == nullptr || Site->Generator == ECL9Generator::None)
		{
			UE_LOG(LogCL9, Warning, TEXT("cl9.seed: no site row with a generator%s."), Args.IsValidIndex(1) ? *FString::Printf(TEXT(" named '%s'"), *Args[1]) : TEXT(""));
			return;
		}
		const uint32 Seed = static_cast<uint32>(FCString::Strtoui64(*Args[0], nullptr, 10));
		ACL9SiteMap::Rebuild(World, *Site, *Registry, Seed);
	}
#endif

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

static FAutoConsoleCommandWithWorldAndArgs GCL9CheatDay(
	TEXT("cl9.day"),
	TEXT("cl9.day <N> — jumps the lobby to day N; the next cl9.night rolls that day's curves (host ruling)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		RouteCL9Cheat(TEXT("day"), Args, World, false);
	}));

static FAutoConsoleCommandWithWorldAndArgs GCL9CheatOrders(
	TEXT("cl9.orders"),
	TEXT("cl9.orders — prints the lobby's numbered offer and the accepted orders with deadlines (local, read-only)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		RouteCL9Cheat(TEXT("orders"), Args, World, true);
	}));

static FAutoConsoleCommandWithWorldAndArgs GCL9CheatPick(
	TEXT("cl9.pick"),
	TEXT("cl9.pick <n> [m] … — confirms the lobby offer by number, exactly the required count (host ruling)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		RouteCL9Cheat(TEXT("pick"), Args, World, false);
	}));

static FAutoConsoleCommandWithWorldAndArgs GCL9CheatSeed(
	TEXT("cl9.seed"),
	TEXT("cl9.seed <N> — rebuilds the site with seed N now (0 = shell only) and pins the next night to it (host ruling). In the editor without PIE it lays the map into the level as transient actors."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
#if WITH_EDITOR
		if (World != nullptr && World->WorldType == EWorldType::Editor)
		{
			LayEditorSiteMap(Args, World);
			return;
		}
#endif
		RouteCL9Cheat(TEXT("seed"), Args, World, false);
	}));

static FAutoConsoleCommandWithWorldAndArgs GCL9CheatList(
	TEXT("cl9.list"),
	TEXT("cl9.list <items|characters> — numbered rows with broken ones marked (local, read-only)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		RouteCL9Cheat(TEXT("list"), Args, World, true);
	}));

#endif
