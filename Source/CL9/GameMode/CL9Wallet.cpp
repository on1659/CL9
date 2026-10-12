// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9Wallet.h"
#include "CL9.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"

void UCL9WalletSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 이 머신의 파일을 읽는다 — 없으면 0 에서 시작한다(첫 실행)
	const FString Slot = GetSlotName();
	if (const UCL9WalletSave* Saved = Cast<UCL9WalletSave>(UGameplayStatics::LoadGameFromSlot(Slot, 0)))
	{
		Ether = FMath::Max(Saved->Ether, 0);
	}
	UE_LOG(LogCL9, Log, TEXT("Wallet: '%s' loaded — %d ether."), *Slot, Ether);
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

void UCL9WalletSubsystem::Save() const
{
	UCL9WalletSave* Saved = Cast<UCL9WalletSave>(UGameplayStatics::CreateSaveGameObject(UCL9WalletSave::StaticClass()));
	if (Saved == nullptr)
	{
		return;
	}
	Saved->Ether = Ether;
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
