// Copyright Epic Games, Inc. All Rights Reserved.

// cl9.* 콘솔 명령 등록부 (docs/systems/cheats.md). 점(.)이 들어간 이름은
// exec 함수로 만들 수 없어서, 각 명령은 여기서 등록된 뒤 치트 매니저로
// 전달된다 — 읽기 전용 명령은 로컬에서 바로, 클라이언트 콘솔이 호스트
// 판정을 요청하는 경우는 플레이어 컨트롤러의 Server RPC를 거쳐서 처리한다.

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "HAL/IConsoleManager.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "CL9.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9SiteDef.h"
#include "Core/CL9InputText.h"
#include "GameMode/CL9CheatManager.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameUserSettings.h"
#include "GameMode/CL9PlayerController.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "World/CL9SiteMap.h"

#if !UE_BUILD_SHIPPING

namespace
{
#if WITH_EDITOR
	/**
	 * 창 없이 쓰는 5a 맵 도구: 에디터에서 (PIE 없이) cl9.seed를 실행하면 사이트를
	 * 레벨에 임시(에디터 전용) 액터로 배치한다 — 밤이 실제로 돌리는 것과 동일한
	 * 생성기이므로, 여기서 살펴보는 것이 곧 플레이어가 받게 될 결과다 (build-order 5a).
	 */
	void LayEditorSiteMap(const TArray<FString>& Args, UWorld* World)
	{
		if (!Args.IsValidIndex(0))
		{
			UE_LOG(LogCL9, Warning, TEXT("cl9.seed: usage cl9.seed <N> [site alias] [day] — 0 = build nothing, 'clear' removes the map."));
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
		// 일자는 한 판의 모듈 수만 바꾼다 (docs/plan/data/module.md 결정 5) — 안 적으면 1일차다
		const int32 Day = Args.IsValidIndex(2) ? FMath::Max(1, FCString::Atoi(*Args[2])) : 1;
		ACL9SiteMap::Rebuild(World, *Site, *Registry, Seed, Day);
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

static FAutoConsoleCommandWithWorldAndArgs GCL9CheatMoney(
	TEXT("cl9.money"),
	TEXT("cl9.money [amount] — prints the run's ether; with an amount, pays it in (positive) or takes it out (negative) (host ruling)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		RouteCL9Cheat(TEXT("money"), Args, World, false);
	}));

static FAutoConsoleCommandWithWorldAndArgs GCL9CheatBuy(
	TEXT("cl9.buy"),
	TEXT("cl9.buy [alias|number] — buys equipment (truck panel stand-in, host ruling); with no argument it lists the shop."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		RouteCL9Cheat(TEXT("buy"), Args, World, false);
	}));

static FAutoConsoleCommandWithWorldAndArgs GCL9CheatEat(
	TEXT("cl9.eat"),
	TEXT("cl9.eat — eats the dish in the active quick slot (stands in for the long left-click; host ruling)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		RouteCL9Cheat(TEXT("eat"), Args, World, false);
	}));

static FAutoConsoleCommandWithWorldAndArgs GCL9CheatSettle(
	TEXT("cl9.settle"),
	TEXT("cl9.settle [force] — prints the settlement state and runs the judgement; 'force' ends the run now (host ruling)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		RouteCL9Cheat(TEXT("settle"), Args, World, false);
	}));

static FAutoConsoleCommandWithWorldAndArgs GCL9CheatSave(
	TEXT("cl9.save"),
	TEXT("cl9.save [slot] — writes the run (day, ether, orders in progress) to the slot now (host ruling)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		RouteCL9Cheat(TEXT("save"), Args, World, false);
	}));

static FAutoConsoleCommandWithWorldAndArgs GCL9CheatLoad(
	TEXT("cl9.load"),
	TEXT("cl9.load [slot] — restores the run from the slot; lobby only (host ruling)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		RouteCL9Cheat(TEXT("load"), Args, World, false);
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

/**
 * cl9.lang [ko|en|ja|zh-Hans] — 설정 화면의 언어 항목과 같은 경로(UCL9GameUserSettings::ApplyLanguage)로
 * 언어를 바꾼다: 게임에선 바꾸고 GameUserSettings.ini에 저장, 에디터/PIE에선 게임 언어 미리보기.
 * 로컬 전용 (docs/systems/localization.md).
 */
static FAutoConsoleCommand GCL9CheatLang(
	TEXT("cl9.lang"),
	TEXT("cl9.lang [culture] — prints the current language, or switches to ko/en/ja/zh-Hans now and remembers it (local)."),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		if (!Args.IsValidIndex(0))
		{
			UE_LOG(LogCL9, Log, TEXT("cl9.lang: current language '%s'."), *FInternationalization::Get().GetCurrentLanguage()->GetName());
			return;
		}
		UCL9GameUserSettings::ApplyLanguage(Args[0]);
	}));

#endif
