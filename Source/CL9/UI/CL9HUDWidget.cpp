// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9HUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Character/CL9Character.h"
#include "Character/CL9CharacterMovement.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ItemDef.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameState.h"
#include "GameMode/CL9PlayerState.h"

void UCL9HUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GetWorld()->GetTimerManager().SetTimer(WireTimer, this, &UCL9HUDWidget::TryWire, 0.2f, true, 0.f);
}

void UCL9HUDWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WireTimer);
		World->GetTimerManager().ClearTimer(ClockTimer);
	}

	Super::NativeDestruct();
}

void UCL9HUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	RefreshStamina();
}

void UCL9HUDWidget::TryWire()
{
	ACL9PlayerState* PlayerState = GetOwnPlayerState();
	ACL9GameState* GameState = GetCL9GameState();
	if (PlayerState == nullptr || GameState == nullptr)
	{
		return;   // 복제 도착 전 — 타이머가 다시 부른다
	}

	GetWorld()->GetTimerManager().ClearTimer(WireTimer);

	PlayerState->OnHpChanged.AddDynamic(this, &UCL9HUDWidget::HandleHpChanged);
	PlayerState->OnCarryChanged.AddDynamic(this, &UCL9HUDWidget::HandleCarryChanged);
	GameState->OnNightPhaseChanged.AddDynamic(this, &UCL9HUDWidget::HandlePhaseChanged);

	GetWorld()->GetTimerManager().SetTimer(ClockTimer, this, &UCL9HUDWidget::RefreshClock, 0.25f, true);

	RefreshHp();
	RefreshCarry();
	RefreshClock();
}

void UCL9HUDWidget::HandleHpChanged()
{
	RefreshHp();
}

void UCL9HUDWidget::HandleCarryChanged()
{
	RefreshCarry();
}

void UCL9HUDWidget::HandlePhaseChanged(ECL9NightPhase NewPhase)
{
	RefreshClock();
}

void UCL9HUDWidget::RefreshHp()
{
	const ACL9PlayerState* PlayerState = GetOwnPlayerState();
	if (HpText == nullptr || PlayerState == nullptr)
	{
		return;
	}

	const int32 Hp = PlayerState->Hp;
	if (Hp <= 10)
	{
		FString Hearts;
		for (int32 Index = 0; Index < Hp; ++Index)
		{
			Hearts += TEXT("♥");
		}
		HpText->SetText(FText::FromString(Hearts));
	}
	else
	{
		HpText->SetText(FText::Format(NSLOCTEXT("CL9", "HudHp", "HP {0}"), Hp));
	}
}

void UCL9HUDWidget::RefreshCarry()
{
	const ACL9PlayerState* PlayerState = GetOwnPlayerState();
	if (PlayerState == nullptr)
	{
		return;
	}

	if (HandText != nullptr)
	{
		HandText->SetText(PlayerState->HandItem.IsSet()
			? FText::Format(NSLOCTEXT("CL9", "HudHand", "손: {0}"), ItemLabel(PlayerState->HandItem))
			: NSLOCTEXT("CL9", "HudHandEmpty", "손: -"));
	}

	if (PocketText != nullptr)
	{
		TArray<FText> Labels;
		for (const FCL9CarriedItem& Item : PlayerState->PocketItems)
		{
			Labels.Add(ItemLabel(Item));
		}
		PocketText->SetText(Labels.Num() > 0
			? FText::Format(NSLOCTEXT("CL9", "HudPocket", "품 {0}: {1}"),
				Labels.Num(), FText::Join(NSLOCTEXT("CL9", "HudPocketSep", ", "), Labels))
			: NSLOCTEXT("CL9", "HudPocketEmpty", "품: -"));
	}
}

void UCL9HUDWidget::RefreshClock()
{
	const ACL9GameState* GameState = GetCL9GameState();
	if (ClockText == nullptr || GameState == nullptr)
	{
		return;
	}

	const int32 Day = GameState->GetCurrentDay();
	switch (GameState->GetPhase())
	{
	case ECL9NightPhase::Night:
	{
		const int32 Remaining = FMath::CeilToInt(GameState->GetNightRemaining());
		FNumberFormattingOptions TwoDigits;
		TwoDigits.MinimumIntegralDigits = 2;
		ClockText->SetText(FText::Format(NSLOCTEXT("CL9", "HudNight", "{0}일차 · {1}:{2}{3}"),
			Day, Remaining / 60, FText::AsNumber(Remaining % 60, &TwoDigits),
			GameState->IsBlackout() ? NSLOCTEXT("CL9", "HudBlackout", " · 정전") : FText::GetEmpty()));
		break;
	}
	case ECL9NightPhase::Dawn:
		ClockText->SetText(FText::Format(NSLOCTEXT("CL9", "HudDawn", "{0}일차 · 새벽"), Day));
		break;
	default:
		ClockText->SetText(FText::Format(NSLOCTEXT("CL9", "HudLobby", "{0}일차 · 정차"), Day));
		break;
	}
}

void UCL9HUDWidget::RefreshStamina()
{
	const ACL9Character* Character = Cast<ACL9Character>(GetOwningPlayerPawn());
	const UCL9CharacterMovement* Movement = (Character != nullptr) ? Character->GetCL9CharacterMovement() : nullptr;
	if (StaminaBar == nullptr || Movement == nullptr)
	{
		return;
	}

	StaminaBar->SetPercent(Movement->GetStamina());
	StaminaBar->SetFillColorAndOpacity(Movement->IsExhausted() ? StaminaExhaustedColor : StaminaColor);
}

FText UCL9HUDWidget::ItemLabel(const FCL9CarriedItem& Item) const
{
	const UWorld* World = GetWorld();
	const UCL9GameInstance* GameInstance = (World != nullptr) ? World->GetGameInstance<UCL9GameInstance>() : nullptr;
	const UCL9DefRegistry* Registry = (GameInstance != nullptr) ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9ItemDef* Def = (Registry != nullptr) ? Registry->FindItem(Item.ItemAlias, Item.ItemSubAlias) : nullptr;
	return (Def != nullptr && !Def->DisplayName.IsEmpty()) ? Def->DisplayName : FText::FromName(Item.ItemAlias);
}

ACL9PlayerState* UCL9HUDWidget::GetOwnPlayerState() const
{
	const APlayerController* PlayerController = GetOwningPlayer();
	return (PlayerController != nullptr) ? Cast<ACL9PlayerState>(PlayerController->PlayerState) : nullptr;
}

ACL9GameState* UCL9HUDWidget::GetCL9GameState() const
{
	const UWorld* World = GetWorld();
	return (World != nullptr) ? World->GetGameState<ACL9GameState>() : nullptr;
}
