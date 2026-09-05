// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9HUDWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"
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
#include "Items/CL9Item.h"

void UCL9HUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GetWorld()->GetTimerManager().SetTimer(WireTimer, this, &UCL9HUDWidget::TryWire, 0.2f, true, 0.f);
}

void UCL9HUDWidget::NativeDestruct()
{
	if (ACL9Character* Character = WiredCharacter.Get())
	{
		Character->OnFocusPromptChanged.RemoveDynamic(this, &UCL9HUDWidget::HandleFocusPromptChanged);
	}
	WiredCharacter.Reset();

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

	WireCharacter();
	RefreshStamina();
	RefreshCharge();
	RefreshItemLabel();
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

void UCL9HUDWidget::WireCharacter()
{
	ACL9Character* Character = GetOwnCharacter();
	const bool bChanged = Character != WiredCharacter.Get() || (Character == nullptr && WiredCharacter.IsStale());
	if (!bChanged)
	{
		return;
	}

	if (ACL9Character* Old = WiredCharacter.Get())
	{
		Old->OnFocusPromptChanged.RemoveDynamic(this, &UCL9HUDWidget::HandleFocusPromptChanged);
	}
	WiredCharacter = Character;
	if (Character != nullptr)
	{
		Character->OnFocusPromptChanged.AddDynamic(this, &UCL9HUDWidget::HandleFocusPromptChanged);
	}

	RefreshPrompt();
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

void UCL9HUDWidget::HandleFocusPromptChanged()
{
	RefreshPrompt();
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

void UCL9HUDWidget::RefreshPrompt()
{
	if (PromptText == nullptr)
	{
		return;
	}

	const ACL9Character* Character = WiredCharacter.Get();
	const FText Prompt = (Character != nullptr) ? Character->GetFocusPrompt() : FText::GetEmpty();
	PromptText->SetText(Prompt);
	PromptText->SetVisibility(Prompt.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
}

void UCL9HUDWidget::RefreshCharge()
{
	if (ChargeBar == nullptr)
	{
		return;
	}

	// Negative = not charging. The host clocks the real charge; this mirrors the
	// local press so the gauge never waits on a round trip (input.md 던지기 차징).
	const ACL9Character* Character = WiredCharacter.Get();
	const float Alpha = (Character != nullptr) ? Character->GetThrowChargeAlpha() : -1.f;
	if (Alpha < 0.f)
	{
		ChargeBar->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	ChargeBar->SetVisibility(ESlateVisibility::HitTestInvisible);
	ChargeBar->SetPercent(Alpha);
}

void UCL9HUDWidget::RefreshItemLabel()
{
	if (ItemLabelBox == nullptr)
	{
		return;
	}

	// Same target the character outlines, so label and highlight never disagree.
	// Text refreshes only when the target changes; the screen position is
	// presentation and re-projects every frame (docs/systems/ui_system.md 데이터 연결).
	const ACL9Character* Character = WiredCharacter.Get();
	ACL9Item* Item = (Character != nullptr) ? Cast<ACL9Item>(Character->GetFocusedActor()) : nullptr;
	if (Item != LabeledItem.Get())
	{
		LabeledItem = Item;
		if (Item != nullptr)
		{
			FillItemLabel(*Item);
		}
	}
	if (Item == nullptr)
	{
		ItemLabelBox->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const UCL9ItemDef* Def = Item->GetItemDef();
	const float Radius = (Def != nullptr) ? Def->Radius : 20.f;
	const FVector Anchor = Item->GetActorLocation() + FVector(0.f, 0.f, Radius + ItemLabelLift);
	FVector2D ScreenPos;
	if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetOwningPlayer(), Anchor, ScreenPos, false))
	{
		ItemLabelBox->SetVisibility(ESlateVisibility::Collapsed);   // behind the camera
		return;
	}

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ItemLabelBox->Slot))
	{
		CanvasSlot->SetPosition(ScreenPos);
	}
	ItemLabelBox->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCL9HUDWidget::FillItemLabel(const ACL9Item& Item)
{
	const UCL9ItemDef* Def = Item.GetItemDef();

	if (ItemNameText != nullptr)
	{
		ItemNameText->SetText((Def != nullptr && !Def->DisplayName.IsEmpty()) ? Def->DisplayName : FText::FromString(Item.GetName()));
	}

	if (ItemPriceText != nullptr)
	{
		if (Def == nullptr || Def->ValueMax <= 0)
		{
			ItemPriceText->SetText(NSLOCTEXT("CL9", "HudItemNoValue", "값 없음"));
		}
		else if (Def->ValueMin == Def->ValueMax)
		{
			ItemPriceText->SetText(FText::Format(NSLOCTEXT("CL9", "HudItemValue", "{0} 이더"), Def->ValueMin));
		}
		else
		{
			ItemPriceText->SetText(FText::Format(NSLOCTEXT("CL9", "HudItemValueRange", "{0}~{1} 이더"), Def->ValueMin, Def->ValueMax));
		}
	}

	if (ItemInfoText != nullptr)
	{
		TArray<FText> Facts;
		if (Def != nullptr)
		{
			if (Def->Weight > 0.f)
			{
				Facts.Add(FText::Format(NSLOCTEXT("CL9", "HudItemWeight", "무게 {0}"), FText::AsNumber(Def->Weight)));
			}
			switch (Def->CarrySlot)
			{
			case ECL9CarrySlot::Pocketable: Facts.Add(NSLOCTEXT("CL9", "HudItemPocketable", "품에 들어감")); break;
			case ECL9CarrySlot::Hand:       Facts.Add(NSLOCTEXT("CL9", "HudItemHandOnly", "손으로만")); break;
			case ECL9CarrySlot::CartOnly:   Facts.Add(NSLOCTEXT("CL9", "HudItemCartOnly", "수레로만")); break;
			}
			if (Def->bFragile)
			{
				Facts.Add(NSLOCTEXT("CL9", "HudItemFragile", "깨지기 쉬움"));
			}
		}
		ItemInfoText->SetText(FText::Join(NSLOCTEXT("CL9", "HudItemFactSep", " · "), Facts));
		ItemInfoText->SetVisibility(Facts.Num() > 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
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

ACL9Character* UCL9HUDWidget::GetOwnCharacter() const
{
	return Cast<ACL9Character>(GetOwningPlayerPawn());
}
