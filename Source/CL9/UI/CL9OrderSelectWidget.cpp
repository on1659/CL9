// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9OrderSelectWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ItemDef.h"
#include "Data/CL9OrderDef.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameState.h"
#include "GameMode/CL9PlayerController.h"

void UCL9OrderCardRelay::HandleClicked()
{
	if (Owner != nullptr)
	{
		Owner->ToggleCard(Index);
	}
}

void UCL9OrderSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (AcceptButton != nullptr)
	{
		AcceptButton->OnClicked.AddDynamic(this, &UCL9OrderSelectWidget::HandleAcceptClicked);
	}

	GetWorld()->GetTimerManager().SetTimer(WireTimer, this, &UCL9OrderSelectWidget::TryWire, 0.2f, true, 0.f);
}

void UCL9OrderSelectWidget::NativeDestruct()
{
	if (AcceptButton != nullptr)
	{
		AcceptButton->OnClicked.RemoveDynamic(this, &UCL9OrderSelectWidget::HandleAcceptClicked);
	}
	if (ACL9GameState* GameState = GetCL9GameState(); GameState != nullptr && bWired)
	{
		GameState->OnOrdersChanged.RemoveDynamic(this, &UCL9OrderSelectWidget::HandleOrdersChanged);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WireTimer);
	}

	Super::NativeDestruct();
}

void UCL9OrderSelectWidget::TryWire()
{
	ACL9GameState* GameState = GetCL9GameState();
	if (GameState == nullptr)
	{
		return;   // 복제 도착 전 — 타이머가 다시 부른다
	}

	GetWorld()->GetTimerManager().ClearTimer(WireTimer);
	GameState->OnOrdersChanged.AddDynamic(this, &UCL9OrderSelectWidget::HandleOrdersChanged);
	bWired = true;
	Rebuild();
}

void UCL9OrderSelectWidget::HandleOrdersChanged()
{
	Rebuild();
}

void UCL9OrderSelectWidget::Rebuild()
{
	const ACL9GameState* GameState = GetCL9GameState();
	if (GameState == nullptr || CardsBox == nullptr || WidgetTree == nullptr)
	{
		return;
	}

	CardsBox->ClearChildren();
	Cards.Reset();
	Relays.Reset();
	Selected.Reset();

	for (int32 Index = 0; Index < GameState->OfferedOrders.Num(); ++Index)
	{
		UButton* Card = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		UVerticalBox* CardStack = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		UTextBlock* NameLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		UTextBlock* DetailLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		if (Card == nullptr || CardStack == nullptr || NameLabel == nullptr || DetailLabel == nullptr)
		{
			continue;
		}

		// 이름과 상세를 다른 크기로 나눈다 — 한 덩어리로 두면 무엇이 주문 이름인지 눈이 못 고른다
		// (docs/systems/ui.md 글자 위계)
		const FName Alias = GameState->OfferedOrders[Index];
		NameLabel->SetText(MakeOrderName(Alias));
		NameLabel->SetFont(ScaledFont(NameLabel->GetFont(), CardNameSize, TEXT("Bold")));
		NameLabel->SetColorAndOpacity(FSlateColor(CardTextColor));
		NameLabel->SetJustification(ETextJustify::Center);

		DetailLabel->SetText(MakeOrderDetail(Alias));
		DetailLabel->SetFont(ScaledFont(DetailLabel->GetFont(), CardDetailSize, TEXT("Regular")));
		DetailLabel->SetColorAndOpacity(FSlateColor(CardTextColor));
		DetailLabel->SetJustification(ETextJustify::Center);

		CardStack->AddChildToVerticalBox(NameLabel);
		CardStack->AddChildToVerticalBox(DetailLabel);
		Card->AddChild(CardStack);
		Card->SetBackgroundColor(CardColor);

		UCL9OrderCardRelay* Relay = NewObject<UCL9OrderCardRelay>(this);
		Relay->Owner = this;
		Relay->Index = Index;
		Card->OnClicked.AddDynamic(Relay, &UCL9OrderCardRelay::HandleClicked);

		if (UVerticalBoxSlot* CardSlot = CardsBox->AddChildToVerticalBox(Card))
		{
			CardSlot->SetPadding(FMargin(0.f, 4.f));
			CardSlot->SetHorizontalAlignment(HAlign_Fill);
		}
		Cards.Add(Card);
		Relays.Add(Relay);
	}

	if (TitleText != nullptr)
	{
		TitleText->SetText(FText::Format(NSLOCTEXT("CL9", "OrderSelectTitle", "주문서 · {0}개 중 {1}개 선택"),
			GameState->OfferedOrders.Num(), GameState->OrdersRequired));
	}

	const APlayerController* PlayerController = GetOwningPlayer();
	const bool bHost = PlayerController != nullptr && PlayerController->HasAuthority();
	if (HintText != nullptr)
	{
		HintText->SetText(bHost
			? NSLOCTEXT("CL9", "OrderSelectHintHost", "받을 주문을 누른 뒤 [확정]을 누르세요")
			: NSLOCTEXT("CL9", "OrderSelectHintClient", "호스트가 주문을 확정합니다"));
	}

	RefreshSelection();
}

void UCL9OrderSelectWidget::ToggleCard(int32 Index)
{
	const ACL9GameState* GameState = GetCL9GameState();
	if (GameState == nullptr || !Cards.IsValidIndex(Index))
	{
		return;
	}

	if (Selected.Remove(Index) == 0 && Selected.Num() < GameState->OrdersRequired)
	{
		Selected.Add(Index);
	}
	RefreshSelection();
}

void UCL9OrderSelectWidget::RefreshSelection()
{
	const ACL9GameState* GameState = GetCL9GameState();
	const int32 Required = (GameState != nullptr) ? GameState->OrdersRequired : 0;

	for (int32 Index = 0; Index < Cards.Num(); ++Index)
	{
		if (Cards[Index] != nullptr)
		{
			Cards[Index]->SetBackgroundColor(Selected.Contains(Index) ? CardSelectedColor : CardColor);
		}
	}

	if (AcceptText != nullptr)
	{
		AcceptText->SetText(FText::Format(NSLOCTEXT("CL9", "OrderSelectAccept", "확정 ({0}/{1})"), Selected.Num(), Required));
	}

	const APlayerController* PlayerController = GetOwningPlayer();
	const bool bHost = PlayerController != nullptr && PlayerController->HasAuthority();
	if (AcceptButton != nullptr)
	{
		AcceptButton->SetIsEnabled(bHost && Required > 0 && Selected.Num() == Required);
	}
}

void UCL9OrderSelectWidget::HandleAcceptClicked()
{
	if (ACL9PlayerController* PlayerController = GetOwningPlayer<ACL9PlayerController>())
	{
		PlayerController->AcceptOrders(Selected);
	}
}

FSlateFontInfo UCL9OrderSelectWidget::ScaledFont(FSlateFontInfo Font, int32 Size, FName Typeface)
{
	// 폰트 에셋은 그대로 두고 크기와 굵기만 바꾼다 — 위젯 기본값이 곧 프로젝트 기본 폰트다
	Font.Size = Size;
	Font.TypefaceFontName = Typeface;
	return Font;
}

const UCL9DefRegistry* UCL9OrderSelectWidget::GetRegistry() const
{
	const UWorld* World = GetWorld();
	const UCL9GameInstance* GameInstance = (World != nullptr) ? World->GetGameInstance<UCL9GameInstance>() : nullptr;
	return (GameInstance != nullptr) ? GameInstance->GetDefRegistry() : nullptr;
}

FText UCL9OrderSelectWidget::MakeOrderName(FName Alias) const
{
	const UCL9DefRegistry* Registry = GetRegistry();
	const UCL9OrderDef* Order = (Registry != nullptr) ? Registry->FindOrder(Alias) : nullptr;
	return (Order != nullptr && !Order->DisplayName.IsEmpty()) ? Order->DisplayName : FText::FromName(Alias);
}

FText UCL9OrderSelectWidget::MakeOrderDetail(FName Alias) const
{
	const UCL9DefRegistry* Registry = GetRegistry();
	const UCL9OrderDef* Order = (Registry != nullptr) ? Registry->FindOrder(Alias) : nullptr;
	if (Order == nullptr)
	{
		return FText::GetEmpty();
	}

	TArray<FText> Parts;
	for (const FCL9ItemCount& Ingredient : Order->Ingredients)
	{
		const UCL9ItemDef* Item = Registry->FindItem(Ingredient.ItemAlias, Ingredient.ItemSubAlias);
		const FText ItemName = (Item != nullptr && !Item->DisplayName.IsEmpty()) ? Item->DisplayName : FText::FromName(Ingredient.ItemAlias);
		Parts.Add(FText::Format(NSLOCTEXT("CL9", "OrderIngredient", "{0}×{1}"), ItemName, Ingredient.Count));
	}

	return FText::Format(NSLOCTEXT("CL9", "OrderCardDetail", "{0} · 보상 {1}~{2} 이더 · 마감 {3}일"),
		FText::Join(NSLOCTEXT("CL9", "OrderIngredientSep", ", "), Parts),
		Order->RewardMin, Order->RewardMax, Order->DeadlineDays);
}

ACL9GameState* UCL9OrderSelectWidget::GetCL9GameState() const
{
	const UWorld* World = GetWorld();
	return (World != nullptr) ? World->GetGameState<ACL9GameState>() : nullptr;
}
