// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9SkinShopWidget.h"
#include "Internationalization/StringTableRegistry.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "CL9.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9SkinDef.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9PlayerController.h"
#include "GameMode/CL9Wallet.h"

void UCL9SkinRelay::HandleClicked()
{
	if (Owner != nullptr)
	{
		Owner->HandleSkinClicked(SkinAlias);
	}
}

void UCL9SkinShopWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildTree();
}

void UCL9SkinShopWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (UCL9WalletSubsystem* Wallet = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCL9WalletSubsystem>() : nullptr)
	{
		WalletHandle = Wallet->OnWalletChanged.AddUObject(this, &UCL9SkinShopWidget::Refresh);
	}
	if (HintLine != nullptr)
	{
		HintLine->SetText(FText::GetEmpty());
	}
	Refresh();
}

void UCL9SkinShopWidget::NativeDestruct()
{
	if (UCL9WalletSubsystem* Wallet = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCL9WalletSubsystem>() : nullptr)
	{
		Wallet->OnWalletChanged.Remove(WalletHandle);
	}
	WalletHandle.Reset();
	Super::NativeDestruct();
}

bool UCL9SkinShopWidget::MountRoot(UWidgetTree& Tree, UWidget& Root)
{
	// 타이틀·게임 오버와 같은 규칙 — 루트가 비었거나 빈 패널 하나뿐이면 스스로 지은 트리를 올린다
	if (Tree.RootWidget == nullptr)
	{
		Tree.RootWidget = &Root;
		return true;
	}
	UPanelWidget* Panel = Cast<UPanelWidget>(Tree.RootWidget);
	if (Panel == nullptr || Panel->GetChildrenCount() > 0)
	{
		return false;
	}
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Panel->AddChild(&Root)))
	{
		CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		CanvasSlot->SetOffsets(FMargin(0.f));
	}
	return true;
}

UTextBlock* UCL9SkinShopWidget::MakeText(const FText& Content, int32 Size, const FLinearColor& Color)
{
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Text->SetText(Content);
	FSlateFontInfo Font = Text->GetFont();
	Font.Size = Size;
	Text->SetFont(Font);
	Text->SetColorAndOpacity(FSlateColor(Color));
	Text->SetJustification(ETextJustify::Center);
	return Text;
}

void UCL9SkinShopWidget::BuildTree()
{
	UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Backdrop"));
	if (!MountRoot(*WidgetTree, *Backdrop))
	{
		UE_LOG(LogCL9, Warning, TEXT("SkinShop: %s already has a designed widget tree — the self-built wardrobe is skipped."), *GetClass()->GetName());
		return;
	}
	Backdrop->SetBrushColor(BackdropColor);
	Backdrop->SetHorizontalAlignment(HAlign_Center);
	Backdrop->SetVerticalAlignment(VAlign_Center);

	UVerticalBox* Panel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SkinPanel"));
	Backdrop->SetContent(Panel);

	auto AddLine = [this, Panel](UWidget* Widget, float Gap)
	{
		if (UVerticalBoxSlot* LineSlot = Panel->AddChildToVerticalBox(Widget))
		{
			LineSlot->SetHorizontalAlignment(HAlign_Center);
			LineSlot->SetPadding(FMargin(0.f, 0.f, 0.f, Gap));
		}
	};
	AddLine(MakeText(LOCTABLE("CL9", "Skin.Title"), TitleSize, TextColor), 12.f);
	BalanceText = MakeText(FText::GetEmpty(), BodySize, TextColor);
	AddLine(BalanceText, 18.f);
	RowsBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("SkinRows"));
	AddLine(RowsBox, 12.f);
	HintLine = MakeText(FText::GetEmpty(), BodySize, FLinearColor(1.f, 0.6f, 0.4f));
	AddLine(HintLine, 12.f);

	UButton* Close = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	Close->SetBackgroundColor(ButtonColor);
	Close->AddChild(MakeText(LOCTABLE("CL9", "Skin.Close"), BodySize, TextColor));
	Close->OnClicked.AddDynamic(this, &UCL9SkinShopWidget::HandleWardrobeClose);
	AddLine(Close, 0.f);
}

void UCL9SkinShopWidget::Refresh()
{
	const UCL9GameInstance* GameInstance = Cast<UCL9GameInstance>(GetGameInstance());
	const UCL9DefRegistry* Registry = GameInstance != nullptr ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9WalletSubsystem* Wallet = GameInstance != nullptr ? GameInstance->GetSubsystem<UCL9WalletSubsystem>() : nullptr;
	if (Registry == nullptr || Wallet == nullptr || RowsBox == nullptr)
	{
		return;
	}
	if (BalanceText != nullptr)
	{
		BalanceText->SetText(FText::Format(LOCTABLE("CL9", "Skin.Balance"), Wallet->GetEther()));
	}

	// 줄은 매번 다시 짓는다 — 스킨은 몇 개뿐이다
	RowsBox->ClearChildren();
	Relays.Reset();
	const FName Worn = Wallet->GetEquippedSkin();
	for (const UCL9SkinDef* Skin : Registry->GetSkins())
	{
		const bool bOwned = Skin->bDefault || Wallet->OwnsSkin(Skin->Alias);
		const bool bWorn = Worn == Skin->Alias || (Worn.IsNone() && Skin->bDefault);
		const FText Label = bWorn
			? FText::Format(LOCTABLE("CL9", "Skin.RowWorn"), Skin->DisplayName)
			: bOwned
				? FText::Format(LOCTABLE("CL9", "Skin.RowOwned"), Skin->DisplayName)
				: FText::Format(LOCTABLE("CL9", "Skin.RowBuy"), Skin->DisplayName, Skin->Price);

		USizeBox* Cell = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		Button->SetBackgroundColor(ButtonColor);
		Button->AddChild(MakeText(Label, BodySize, TextColor));
		Button->SetIsEnabled(!bWorn);
		Cell->SetWidthOverride(ButtonSize.X);
		Cell->SetHeightOverride(ButtonSize.Y);
		Cell->AddChild(Button);
		if (UVerticalBoxSlot* RowSlot = RowsBox->AddChildToVerticalBox(Cell))
		{
			RowSlot->SetHorizontalAlignment(HAlign_Center);
			RowSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));
		}

		UCL9SkinRelay* Relay = NewObject<UCL9SkinRelay>(this);
		Relay->Owner = this;
		Relay->SkinAlias = Skin->Alias;
		Button->OnClicked.AddDynamic(Relay, &UCL9SkinRelay::HandleClicked);
		Relays.Add(Relay);
	}
}

void UCL9SkinShopWidget::HandleSkinClicked(FName SkinAlias)
{
	UCL9GameInstance* GameInstance = Cast<UCL9GameInstance>(GetGameInstance());
	const UCL9DefRegistry* Registry = GameInstance != nullptr ? GameInstance->GetDefRegistry() : nullptr;
	UCL9WalletSubsystem* Wallet = GameInstance != nullptr ? GameInstance->GetSubsystem<UCL9WalletSubsystem>() : nullptr;
	const UCL9SkinDef* Skin = Registry != nullptr ? Registry->FindSkin(SkinAlias) : nullptr;
	if (Wallet == nullptr || Skin == nullptr)
	{
		return;
	}

	// 없으면 산다 — 모자라면 안내만
	if (!Skin->bDefault && !Wallet->OwnsSkin(SkinAlias) && !Wallet->BuySkin(SkinAlias, Skin->Price))
	{
		if (HintLine != nullptr)
		{
			HintLine->SetText(LOCTABLE("CL9", "Skin.NotEnough"));
		}
		return;
	}
	Wallet->EquipSkin(SkinAlias);
	if (HintLine != nullptr)
	{
		HintLine->SetText(FText::GetEmpty());
	}
	if (ACL9PlayerController* Controller = Cast<ACL9PlayerController>(GetOwningPlayer()))
	{
		Controller->RequestSkin(SkinAlias);
	}
}

void UCL9SkinShopWidget::HandleWardrobeClose()
{
	CloseSelf();
}
