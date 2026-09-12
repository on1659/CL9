// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9ShopWidget.h"
#include "Internationalization/StringTableRegistry.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9EquipmentDef.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameState.h"
#include "GameMode/CL9PlayerController.h"

void UCL9ShopWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseText != nullptr)
	{
		CloseText->SetText(LOCTABLE("CL9", "Shop.Close"));
	}
	if (TitleText != nullptr)
	{
		TitleText->SetText(LOCTABLE("CL9", "Shop.Title"));
	}
	if (HintText != nullptr)
	{
		HintText->SetText(FText::GetEmpty());
	}

	Rebuild();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(WireTimer, this, &UCL9ShopWidget::TryWire, 0.2f, true, 0.f);
	}
}

void UCL9ShopWidget::NativeDestruct()
{
	if (ACL9GameState* GameState = GetCL9GameState())
	{
		GameState->OnMoneyChanged.RemoveDynamic(this, &UCL9ShopWidget::HandleMoneyChanged);
	}
	if (ACL9PlayerController* Controller = Cast<ACL9PlayerController>(GetOwningPlayer()))
	{
		Controller->OnBuyRejected.RemoveDynamic(this, &UCL9ShopWidget::HandleBuyRejected);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WireTimer);
	}

	Super::NativeDestruct();
}

void UCL9ShopWidget::TryWire()
{
	ACL9GameState* GameState = GetCL9GameState();
	ACL9PlayerController* Controller = Cast<ACL9PlayerController>(GetOwningPlayer());
	if (GameState == nullptr || Controller == nullptr)
	{
		return;   // 복제 도착 전 — 타이머가 다시 부른다
	}

	GetWorld()->GetTimerManager().ClearTimer(WireTimer);

	GameState->OnMoneyChanged.AddDynamic(this, &UCL9ShopWidget::HandleMoneyChanged);
	Controller->OnBuyRejected.AddDynamic(this, &UCL9ShopWidget::HandleBuyRejected);

	RefreshAffordability();
}

void UCL9ShopWidget::Rebuild()
{
	if (ItemsBox == nullptr)
	{
		return;
	}

	ItemsBox->ClearChildren();
	CardAliases.Reset();
	CardButtons.Reset();
	CardNames.Reset();
	CardDetails.Reset();
	Relays.Reset();

	const UCL9DefRegistry* Registry = GetRegistry();
	if (Registry == nullptr)
	{
		return;
	}

	for (const TObjectPtr<const UCL9EquipmentDef>& Equip : Registry->GetEquipment())
	{
		// 파는 것은 실물뿐이다 — 기준은 표의 컬럼이 지킨다(docs/systems/shop.md)
		if (Equip == nullptr || !Equip->bPhysicalProp)
		{
			continue;
		}

		UButton* Card = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		UVerticalBox* CardStack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UTextBlock* NameLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		UTextBlock* DetailLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		if (Card == nullptr || CardStack == nullptr || NameLabel == nullptr || DetailLabel == nullptr)
		{
			continue;
		}

		NameLabel->SetText(Equip->DisplayName.IsEmpty() ? FText::FromName(Equip->Alias) : Equip->DisplayName);
		NameLabel->SetFont(ScaledFont(NameLabel->GetFont(), CardNameSize, TEXT("Bold")));
		NameLabel->SetJustification(ETextJustify::Center);

		DetailLabel->SetFont(ScaledFont(DetailLabel->GetFont(), CardDetailSize, TEXT("Regular")));
		DetailLabel->SetJustification(ETextJustify::Center);

		CardStack->AddChildToVerticalBox(NameLabel);
		CardStack->AddChildToVerticalBox(DetailLabel);
		Card->AddChild(CardStack);
		Card->SetBackgroundColor(CardColor);

		UCL9ShopCardRelay* Relay = NewObject<UCL9ShopCardRelay>(this);
		Relay->Owner = this;
		Relay->Alias = Equip->Alias;
		Card->OnClicked.AddDynamic(Relay, &UCL9ShopCardRelay::HandleClicked);

		if (UVerticalBoxSlot* CardSlot = ItemsBox->AddChildToVerticalBox(Card))
		{
			CardSlot->SetPadding(FMargin(0.f, 4.f));
			CardSlot->SetHorizontalAlignment(HAlign_Fill);
		}

		CardAliases.Add(Equip->Alias);
		CardButtons.Add(Card);
		CardNames.Add(NameLabel);
		CardDetails.Add(DetailLabel);
		Relays.Add(Relay);
	}

	RefreshAffordability();
}

void UCL9ShopWidget::RefreshAffordability()
{
	const ACL9GameState* GameState = GetCL9GameState();
	if (MoneyText != nullptr && GameState != nullptr)
	{
		MoneyText->SetText(FText::Format(LOCTABLE("CL9", "Shop.Money"), GameState->GetSharedMoney()));
	}

	const UCL9DefRegistry* Registry = GetRegistry();
	for (int32 Index = 0; Index < CardAliases.Num(); ++Index)
	{
		const FName Alias = CardAliases[Index];
		const UCL9EquipmentDef* Equip = (Registry != nullptr) ? Registry->FindEquipment(Alias) : nullptr;
		UButton* Button = CardButtons.IsValidIndex(Index) ? CardButtons[Index].Get() : nullptr;
		UTextBlock* Detail = CardDetails.IsValidIndex(Index) ? CardDetails[Index].Get() : nullptr;
		if (Equip == nullptr || Button == nullptr || Detail == nullptr || GameState == nullptr)
		{
			continue;
		}

		// 판정은 GameState의 것을 그대로 읽는다 — 호스트든 클라든 같은 함수 하나를 본다(docs/systems/shop.md)
		const ECL9BuyResult Result = GameState->CanBuy(Alias);
		const bool bBuyable = Result == ECL9BuyResult::Ok;

		Button->SetIsEnabled(bBuyable);
		if (UTextBlock* Name = CardNames.IsValidIndex(Index) ? CardNames[Index].Get() : nullptr)
		{
			Name->SetColorAndOpacity(FSlateColor(bBuyable ? CardTextColor : CardLockedColor));
		}
		Detail->SetColorAndOpacity(FSlateColor(bBuyable ? CardTextColor : CardLockedColor));

		TArray<FText> Parts;
		Parts.Add(FText::Format(LOCTABLE("CL9", "Shop.Price"), Equip->Price));
		if (Equip->MaxOwned > 0)
		{
			// 몇 개까지 살 수 있는 물건이면 지금 몇 개인지가 곧 살지 말지의 판단 근거다
			Parts.Add(FText::Format(LOCTABLE("CL9", "Shop.Owned"),
				GameState->GetOwnedCount(Alias), Equip->MaxOwned));
		}
		if (!bBuyable)
		{
			Parts.Add(MakeReasonText(Result, *Equip));
		}
		Detail->SetText(FText::Join(LOCTABLE("CL9", "Shop.DetailSep"), Parts));
	}
}

FText UCL9ShopWidget::MakeReasonText(ECL9BuyResult Reason, const UCL9EquipmentDef& Equip) const
{
	switch (Reason)
	{
	case ECL9BuyResult::Locked:
		return FText::Format(LOCTABLE("CL9", "Shop.Locked"), Equip.UnlockDay);
	case ECL9BuyResult::MaxOwned:
		return FText::Format(LOCTABLE("CL9", "Shop.MaxOwned"), Equip.MaxOwned);
	case ECL9BuyResult::NotEnoughMoney:
		return LOCTABLE("CL9", "Shop.NoMoney");
	case ECL9BuyResult::NotInLobby:
		return LOCTABLE("CL9", "Shop.NotInLobby");
	default:
		return LOCTABLE("CL9", "Shop.NoRow");
	}
}

void UCL9ShopWidget::HandleBuyClicked(FName EquipAlias)
{
	if (ACL9PlayerController* Controller = Cast<ACL9PlayerController>(GetOwningPlayer()))
	{
		Controller->BuyEquipment(EquipAlias);
	}
	if (HintText != nullptr)
	{
		HintText->SetText(FText::GetEmpty());
	}
}

void UCL9ShopWidget::HandleMoneyChanged()
{
	// 남이 사도 잔액이 줄어든다 — 그때 살 수 있는 것도 함께 바뀐다
	RefreshAffordability();
}

void UCL9ShopWidget::HandleBuyRejected(FName EquipAlias, ECL9BuyResult Reason)
{
	const UCL9DefRegistry* Registry = GetRegistry();
	const UCL9EquipmentDef* Equip = (Registry != nullptr) ? Registry->FindEquipment(EquipAlias) : nullptr;
	if (HintText != nullptr && Equip != nullptr)
	{
		HintText->SetText(FText::Format(LOCTABLE("CL9", "Shop.Rejected"),
			Equip->DisplayName.IsEmpty() ? FText::FromName(EquipAlias) : Equip->DisplayName,
			MakeReasonText(Reason, *Equip)));
	}
	RefreshAffordability();
}

const UCL9DefRegistry* UCL9ShopWidget::GetRegistry() const
{
	const UWorld* World = GetWorld();
	const UCL9GameInstance* GameInstance = (World != nullptr) ? World->GetGameInstance<UCL9GameInstance>() : nullptr;
	return (GameInstance != nullptr) ? GameInstance->GetDefRegistry() : nullptr;
}

ACL9GameState* UCL9ShopWidget::GetCL9GameState() const
{
	const UWorld* World = GetWorld();
	return (World != nullptr) ? World->GetGameState<ACL9GameState>() : nullptr;
}

FSlateFontInfo UCL9ShopWidget::ScaledFont(FSlateFontInfo Font, int32 Size, FName Typeface)
{
	Font.Size = Size;
	Font.TypefaceFontName = Typeface;
	return Font;
}

void UCL9ShopCardRelay::HandleClicked()
{
	if (Owner != nullptr)
	{
		Owner->HandleBuyClicked(Alias);
	}
}
