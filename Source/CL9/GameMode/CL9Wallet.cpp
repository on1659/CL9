// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9Wallet.h"
#include "CL9.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9SkinDef.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9PlayerController.h"

void UCL9WalletSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 이 머신의 파일을 읽는다 — 없으면 0 에서 시작한다(첫 실행)
	const FString Slot = GetSlotName();
	if (const UCL9WalletSave* Saved = Cast<UCL9WalletSave>(UGameplayStatics::LoadGameFromSlot(Slot, 0)))
	{
		Ether = FMath::Max(Saved->Ether, 0);
		OwnedSkins = Saved->OwnedSkins;
		EquippedSkin = Saved->EquippedSkin;
	}
	UE_LOG(LogCL9, Log, TEXT("Wallet: '%s' loaded — %d ether, %d skin(s) owned, wearing '%s'."), *Slot, Ether, OwnedSkins.Num(), *EquippedSkin.ToString());
}

FString UCL9WalletSubsystem::GetSlotName() const
{
	const UGameInstance* GameInstance = GetGameInstance();
	const FWorldContext* Context = GameInstance ? GameInstance->GetWorldContext() : nullptr;
	const int32 PIEInstance = Context ? Context->PIEInstance : INDEX_NONE;
	return PIEInstance >= 1 ? FString::Printf(TEXT("CL9Wallet_PIE%d"), PIEInstance) : FString(TEXT("CL9Wallet"));
}

void UCL9WalletSubsystem::ApplyGrant(int32 Amount)
{
	LastGrant = FMath::Max(Amount, 0);
	if (LastGrant > 0)
	{
		Ether += LastGrant;
		Save();
	}
	UE_LOG(LogCL9, Log, TEXT("Wallet: '%s' +%d ether this round — %d total."), *GetSlotName(), LastGrant, Ether);
	OnWalletChanged.Broadcast();
}

void UCL9WalletSubsystem::CheatAdd(int32 Delta)
{
	Ether = FMath::Max(Ether + Delta, 0);
	Save();
	OnWalletChanged.Broadcast();
}

bool UCL9WalletSubsystem::BuySkin(FName Alias, int32 Price)
{
	if (OwnedSkins.Contains(Alias))
	{
		return true;
	}
	if (Alias.IsNone() || Price < 0 || Ether < Price)
	{
		UE_LOG(LogCL9, Log, TEXT("Wallet: '%s' cannot buy skin '%s' — %d ether < %d."), *GetSlotName(), *Alias.ToString(), Ether, Price);
		return false;
	}
	Ether -= Price;
	OwnedSkins.Add(Alias);
	Save();
	UE_LOG(LogCL9, Log, TEXT("Wallet: '%s' bought skin '%s' for %d ether — %d left."), *GetSlotName(), *Alias.ToString(), Price, Ether);
	OnWalletChanged.Broadcast();
	return true;
}

void UCL9WalletSubsystem::EquipSkin(FName Alias)
{
	EquippedSkin = Alias;
	Save();
	UE_LOG(LogCL9, Log, TEXT("Wallet: '%s' wears skin '%s'."), *GetSlotName(), *Alias.ToString());
	OnWalletChanged.Broadcast();
}

void UCL9WalletSubsystem::Save() const
{
	UCL9WalletSave* Saved = Cast<UCL9WalletSave>(UGameplayStatics::CreateSaveGameObject(UCL9WalletSave::StaticClass()));
	if (Saved == nullptr)
	{
		return;
	}
	Saved->Ether = Ether;
	Saved->OwnedSkins = OwnedSkins;
	Saved->EquippedSkin = EquippedSkin;
	if (!UGameplayStatics::SaveGameToSlot(Saved, GetSlotName(), 0))
	{
		UE_LOG(LogCL9, Warning, TEXT("Wallet: '%s' could not be saved."), *GetSlotName());
	}
}

#if !UE_BUILD_SHIPPING
// cl9.wallet [delta] — 이 머신(로컬)의 지갑을 찍고, delta 가 있으면 더한다(시험용, 0 아래로는 안 내려간다)
static FAutoConsoleCommandWithWorldAndArgs GCL9CheatWallet(
	TEXT("cl9.wallet"),
	TEXT("cl9.wallet [delta] — prints this machine's out-game wallet (slot, ether, last round grant); with a delta, adds it (local, for tests)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
		UCL9WalletSubsystem* Wallet = GameInstance ? GameInstance->GetSubsystem<UCL9WalletSubsystem>() : nullptr;
		if (Wallet == nullptr)
		{
			UE_LOG(LogCL9, Warning, TEXT("cl9.wallet: no wallet on this machine."));
			return;
		}
		if (Args.IsValidIndex(0))
		{
			Wallet->CheatAdd(FCString::Atoi(*Args[0]));
		}
		UE_LOG(LogCL9, Log, TEXT("cl9.wallet: '%s' — %d ether, last round grant %d."), *Wallet->GetSlotName(), Wallet->GetEther(), Wallet->GetLastGrant());
	}));
#endif

#if !UE_BUILD_SHIPPING
// cl9.skin [alias] — 이 머신(로컬)의 지갑으로 스킨 목록을 찍거나, 필요하면 사서 입고 호스트에 알린다(docs/goal/loop-7k-skins.md).
// 치트 매니저를 거치지 않는다 — 클라이언트 컨트롤러에는 치트 매니저가 없다(cl9.wallet 과 같은 자리)
static FAutoConsoleCommandWithWorldAndArgs GCL9CheatSkin(
	TEXT("cl9.skin"),
	TEXT("cl9.skin [alias] — no argument lists the skins (owned / worn / price) from this machine's wallet; an alias buys it if needed (out-game ether) and wears it, then tells the host (local, for tests)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UCL9GameInstance* GameInstance = World ? World->GetGameInstance<UCL9GameInstance>() : nullptr;
		const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
		UCL9WalletSubsystem* Wallet = GameInstance ? GameInstance->GetSubsystem<UCL9WalletSubsystem>() : nullptr;
		if (Registry == nullptr || Wallet == nullptr)
		{
			UE_LOG(LogCL9, Warning, TEXT("cl9.skin: no registry or wallet on this machine."));
			return;
		}
		if (!Args.IsValidIndex(0))
		{
			UE_LOG(LogCL9, Log, TEXT("cl9.skin: '%s' %d ether, wearing '%s'."), *Wallet->GetSlotName(), Wallet->GetEther(), *Wallet->GetEquippedSkin().ToString());
			for (const UCL9SkinDef* Skin : Registry->GetSkins())
			{
				UE_LOG(LogCL9, Log, TEXT("cl9.skin:   %s — %d ether%s%s"), *Skin->Alias.ToString(), Skin->Price,
					(Skin->bDefault || Wallet->OwnsSkin(Skin->Alias)) ? TEXT(", owned") : TEXT(""),
					Wallet->GetEquippedSkin() == Skin->Alias ? TEXT(", worn") : TEXT(""));
			}
			return;
		}
		const FName Alias(*Args[0]);
		const UCL9SkinDef* Skin = Registry->FindSkin(Alias);
		if (Skin == nullptr)
		{
			UE_LOG(LogCL9, Warning, TEXT("cl9.skin: no skin row '%s'."), *Alias.ToString());
			return;
		}
		if (!Skin->bDefault && !Wallet->BuySkin(Alias, Skin->Price))
		{
			return;
		}
		Wallet->EquipSkin(Alias);
		if (ACL9PlayerController* Controller = Cast<ACL9PlayerController>(World->GetFirstPlayerController()))
		{
			Controller->RequestSkin(Alias);
		}
	}));
#endif
