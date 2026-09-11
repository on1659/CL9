// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9HUDWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Character/CL9Character.h"
#include "Character/CL9CharacterMovement.h"
#include "Data/CL9BuffDef.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ItemDef.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameState.h"
#include "GameMode/CL9PlayerState.h"
#include "Items/CL9Counter.h"
#include "Items/CL9Item.h"
#include "Items/CL9Kitchen.h"
#include "Data/CL9OrderDef.h"
#include "Data/CL9ProgressionDef.h"
#include "UI/CL9CookGaugeWidget.h"
#include "UI/CL9ScanLabelWidget.h"
#include "UI/CL9UIManager.h"
#include "UI/CL9UITable.h"

namespace
{
	/** 버프 줄에 훑을 스탯 9종 — 표시 순서이자 전체 목록이다 (docs/plan/data/character.md 스탯 enum) */
	const ECL9Stat AllStats[] = {
		ECL9Stat::WalkSpeed,
		ECL9Stat::RunSpeed,
		ECL9Stat::CarryMult,
		ECL9Stat::CartMult,
		ECL9Stat::JumpVelocity,
		ECL9Stat::JumpGravity,
		ECL9Stat::StaminaDrainTime,
		ECL9Stat::StaminaRegenTime,
		ECL9Stat::StaminaMinResume
	};

	/** 스탯 이름 — docs/plan/data/character.md 표의 표기를 그대로 쓴다 */
	FText StatLabel(ECL9Stat Stat)
	{
		switch (Stat)
		{
		case ECL9Stat::WalkSpeed:         return NSLOCTEXT("CL9", "StatWalkSpeed", "걷기");
		case ECL9Stat::RunSpeed:          return NSLOCTEXT("CL9", "StatRunSpeed", "달리기");
		case ECL9Stat::CarryMult:         return NSLOCTEXT("CL9", "StatCarryMult", "짐 무게");
		case ECL9Stat::CartMult:          return NSLOCTEXT("CL9", "StatCartMult", "수레 속도");
		case ECL9Stat::JumpVelocity:      return NSLOCTEXT("CL9", "StatJumpVelocity", "점프");
		case ECL9Stat::JumpGravity:       return NSLOCTEXT("CL9", "StatJumpGravity", "점프 중력");
		case ECL9Stat::StaminaDrainTime:  return NSLOCTEXT("CL9", "StatStaminaDrain", "스태미나 지속");
		case ECL9Stat::StaminaRegenTime:  return NSLOCTEXT("CL9", "StatStaminaRegen", "스태미나 회복");
		case ECL9Stat::StaminaMinResume:  return NSLOCTEXT("CL9", "StatStaminaResume", "탈진 회복 문턱");
		}
		return FText::GetEmpty();
	}

	/**
	 * 보정값의 표기 — 스탯마다 단위가 달라서 자릿수도 다르다. 속도·점프는 cm/s라 정수로
	 * 충분하고, 배율과 문턱은 0.05 같은 값이 오므로 소수 둘째 자리까지 본다.
	 * 더하는 값이라 양수에는 부호를 붙인다(음수는 숫자가 부호를 달고 온다).
	 */
	FText StatAmountText(ECL9Stat Stat, float Value)
	{
		FNumberFormattingOptions Options;
		switch (Stat)
		{
		case ECL9Stat::CarryMult:
		case ECL9Stat::CartMult:
		case ECL9Stat::StaminaMinResume:
			Options.MinimumFractionalDigits = 2;
			Options.MaximumFractionalDigits = 2;
			break;
		case ECL9Stat::StaminaDrainTime:
		case ECL9Stat::StaminaRegenTime:
			Options.MinimumFractionalDigits = 1;
			Options.MaximumFractionalDigits = 1;
			break;
		default:
			Options.MaximumFractionalDigits = 0;
			break;
		}

		const FText Number = FText::AsNumber(Value, &Options);
		return Value > 0.f ? FText::Format(NSLOCTEXT("CL9", "StatAmountPlus", "+{0}"), Number) : Number;
	}
}

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
	if (ACL9GameState* GameState = GetCL9GameState())
	{
		GameState->OnOrdersChanged.RemoveDynamic(this, &UCL9HUDWidget::HandleOrdersChanged);
		GameState->OnMoneyChanged.RemoveDynamic(this, &UCL9HUDWidget::HandleMoneyChanged);
	}

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
	RefreshCookGauges();
	RefreshCounter();
	RefreshBuffs();
	RefreshNightWarn(InDeltaTime);
	// 양보했던 선택 화면을 다시 열 때가 됐는지 본다 — 화면이 닫혔다는 이벤트가 없어서 틱에서 본다
	DeriveOrderScreen();
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
	GameState->OnOrdersChanged.AddDynamic(this, &UCL9HUDWidget::HandleOrdersChanged);
	GameState->OnMoneyChanged.AddDynamic(this, &UCL9HUDWidget::HandleMoneyChanged);

	GetWorld()->GetTimerManager().SetTimer(ClockTimer, this, &UCL9HUDWidget::RefreshClock, 0.25f, true);

	RefreshHp();
	RefreshCarry();
	RefreshHead();
	RefreshClock();
	RefreshOrders();
	RefreshMoney();
	RefreshPeriod();
	DeriveOrderScreen();
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
	RefreshHead();
}

void UCL9HUDWidget::HandlePhaseChanged(ECL9NightPhase NewPhase)
{
	RefreshClock();
	RefreshPeriod();   // 날이 넘어가면 목표액과 남은 날이 함께 바뀐다
}

void UCL9HUDWidget::HandleFocusPromptChanged()
{
	RefreshPrompt();
}

void UCL9HUDWidget::HandleOrdersChanged()
{
	RefreshOrders();
	DeriveOrderScreen();
}

void UCL9HUDWidget::HandleMoneyChanged()
{
	RefreshMoney();
	RefreshPeriod();   // 판 자금이 움직였다는 건 기간 수입도 움직였다는 뜻이다
}

void UCL9HUDWidget::RefreshHp()
{
	const ACL9PlayerState* PlayerState = GetOwnPlayerState();
	if (HpText == nullptr || PlayerState == nullptr)
	{
		return;
	}

	const int32 Hp = PlayerState->Hp;
	const int32 Slots = (HpBox != nullptr) ? HpBox->GetChildrenCount() : 0;

	// 칸이 HP를 다 담을 수 있으면 칸으로 보여준다 — 앞에서부터 남은 만큼만 켠다.
	// 칸 안에 무엇이 들어 있는지는 여기서 알 필요가 없다(글자든 이미지든 WBP가 정한다).
	if (Slots >= Hp && Slots > 0)
	{
		for (int32 Index = 0; Index < Slots; ++Index)
		{
			if (UWidget* Heart = HpBox->GetChildAt(Index))
			{
				Heart->SetVisibility(Index < Hp ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
			}
		}
		HpBox->SetVisibility(ESlateVisibility::HitTestInvisible);
		HpText->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// 칸이 모자라거나 아예 없으면 숫자로 물러선다 — 칸을 늘리는 것보다 이쪽이 안전하다
	if (HpBox != nullptr)
	{
		HpBox->SetVisibility(ESlateVisibility::Collapsed);
	}
	HpText->SetVisibility(ESlateVisibility::HitTestInvisible);
	HpText->SetText(FText::Format(NSLOCTEXT("CL9", "HudHp", "HP {0}"), Hp));
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

	// 음수면 차징 중이 아니라는 뜻. 실제 차징 시간은 호스트가 재는데, 이 값은 로컬
	// 입력을 그대로 반영해서 게이지가 왕복 지연을 기다리지 않게 한다(input.md 던지기 차징).
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

	// 캐릭터가 윤곽선을 그리는 대상과 같아서, 라벨과 하이라이트가 서로 어긋날 일이 없다.
	// 텍스트는 대상이 바뀔 때만 갱신되고, 화면 위치는 표시 값이라 매 프레임 다시
	// 투영한다(docs/systems/ui_system.md 데이터 연결).
	const ACL9Character* Character = WiredCharacter.Get();
	ACL9Item* Item = (Character != nullptr) ? Cast<ACL9Item>(Character->GetFocusedActor()) : nullptr;
	// 요리는 같은 액터 위에서 행을 바꿔치기하므로(docs/systems/cooking.md), 행도 정체성의 일부다
	const UCL9ItemDef* CurrentDef = (Item != nullptr) ? Item->GetItemDef() : nullptr;
	if (Item != LabeledItem.Get() || CurrentDef != LabeledDef.Get())
	{
		LabeledItem = Item;
		LabeledDef = CurrentDef;
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
		ItemLabelBox->SetVisibility(ESlateVisibility::Collapsed);   // 카메라 뒤쪽에 있음
		return;
	}

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ItemLabelBox->Slot))
	{
		CanvasSlot->SetPosition(ScreenPos);
	}
	ItemLabelBox->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCL9HUDWidget::RefreshCookGauges()
{
	UWorld* World = GetWorld();
	if (CookLayer == nullptr || CookGaugeClass == nullptr || World == nullptr)
	{
		return;
	}

	// 조리대가 열원이고 진행도는 재료가 들고 있다(docs/systems/cooking.md) — 이름이 바뀌기 전에
	// 얼마나 남았는지 보여주지 않으면 태우지 않을 방법이 없다. 재료를 바라봐야 보이던 라벨 안의 바
	// 대신, 익히는 중인 조리대마다 그 꼭대기 위에 게이지를 하나씩 띄운다(2026-09-10 사용자).
	// 조리대는 판에 몇 개 없으므로 매 프레임 훑는다.
	TSet<ACL9Kitchen*> Cooking;
	for (TActorIterator<ACL9Kitchen> It(World); It; ++It)
	{
		ACL9Kitchen* Kitchen = *It;
		const ACL9Item* Item = Kitchen->GetCookingItem();
		if (Item == nullptr || !Item->IsOnHeat())
		{
			continue;
		}

		UCL9CookGaugeWidget* Gauge = CookGauges.FindRef(Kitchen).Get();
		if (Gauge == nullptr)
		{
			Gauge = CreateWidget<UCL9CookGaugeWidget>(GetOwningPlayer(), CookGaugeClass);
			if (Gauge == nullptr)
			{
				continue;
			}
			if (UCanvasPanelSlot* CanvasSlot = CookLayer->AddChildToCanvas(Gauge))
			{
				CanvasSlot->SetAutoSize(true);
				CanvasSlot->SetAlignment(FVector2D(0.5f, 1.f));
			}
			Gauge->SetVisibility(ESlateVisibility::HitTestInvisible);
			CookGauges.Add(Kitchen, Gauge);
		}
		Cooking.Add(Kitchen);

		// 위치는 표시 값이라 매 프레임 다시 투영한다(docs/systems/ui_system.md 데이터 연결)
		const FVector Anchor = Kitchen->GetTopLocation() + FVector(0.f, 0.f, CookGaugeLift);
		FVector2D ScreenPos;
		if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetOwningPlayer(), Anchor, ScreenPos, false))
		{
			ScreenPos = FVector2D(-10000.f, -10000.f);   // 카메라 뒤 — 화면 밖에 세워 둔다
		}
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Gauge->Slot))
		{
			CanvasSlot->SetPosition(ScreenPos);
		}
		Gauge->Refresh(*Item);
	}

	// 다 익었거나 집혀 간 조리대(또는 사라진 조리대)의 게이지는 뗀다
	for (auto It = CookGauges.CreateIterator(); It; ++It)
	{
		if (Cooking.Contains(It.Key().Get()))
		{
			continue;
		}
		if (UCL9CookGaugeWidget* Gauge = It.Value().Get())
		{
			Gauge->RemoveFromParent();
		}
		It.RemoveCurrent();
	}
}

void UCL9HUDWidget::FillItemLabel(const ACL9Item& Item)
{
	const UCL9ItemDef* Def = Item.GetItemDef();

	// 스캔 라벨과 같은 텍스트를 쓰므로, 둘이 서로 어긋날 일이 없다
	if (ItemNameText != nullptr)
	{
		ItemNameText->SetText(UCL9ScanLabelWidget::MakeNameText(Item));
	}

	if (ItemPriceText != nullptr)
	{
		ItemPriceText->SetText(UCL9ScanLabelWidget::MakePriceText(Item));
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

void UCL9HUDWidget::RefreshHead()
{
	const ACL9PlayerState* PlayerState = GetOwnPlayerState();
	if (HeadText == nullptr || PlayerState == nullptr)
	{
		return;
	}

	HeadText->SetText(PlayerState->HeadItem.IsSet()
		? FText::Format(NSLOCTEXT("CL9", "HudHead", "머리: {0}"), ItemLabel(PlayerState->HeadItem))
		: NSLOCTEXT("CL9", "HudHeadEmpty", "머리: -"));
}

void UCL9HUDWidget::RefreshOrders()
{
	const ACL9GameState* GameState = GetCL9GameState();
	if (OrdersText == nullptr || GameState == nullptr)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const UCL9GameInstance* GameInstance = (World != nullptr) ? World->GetGameInstance<UCL9GameInstance>() : nullptr;
	const UCL9DefRegistry* Registry = (GameInstance != nullptr) ? GameInstance->GetDefRegistry() : nullptr;

	TArray<FText> Lines;
	for (const FCL9AcceptedOrder& Accepted : GameState->AcceptedOrders)
	{
		const UCL9OrderDef* Order = (Registry != nullptr) ? Registry->FindOrder(Accepted.OrderAlias) : nullptr;
		const FText Name = (Order != nullptr && !Order->DisplayName.IsEmpty()) ? Order->DisplayName : FText::FromName(Accepted.OrderAlias);
		TArray<FText> Parts;
		if (Order != nullptr)
		{
			for (const FCL9ItemCount& Ingredient : Order->Ingredients)
			{
				const UCL9ItemDef* ItemDef = Registry->FindItem(Ingredient.ItemAlias, Ingredient.ItemSubAlias);
				const FText ItemName = (ItemDef != nullptr && !ItemDef->DisplayName.IsEmpty()) ? ItemDef->DisplayName : FText::FromName(Ingredient.ItemAlias);
				Parts.Add(FText::Format(NSLOCTEXT("CL9", "HudOrderIngredient", "{0}×{1}"), ItemName, Ingredient.Count));
			}
		}
		// 보상은 수락할 때 굴려 확정된 값이다(2026-09-06 결정) — 카드의 범위가 아니라 실제로 받을 액수를 보여준다
		Lines.Add(FText::Format(NSLOCTEXT("CL9", "HudOrderLine", "{0} · {1} · {2}일차까지 · {3} 이더"),
			Name, FText::Join(NSLOCTEXT("CL9", "HudOrderSep", ", "), Parts), Accepted.DeadlineDay, Accepted.Reward));
	}

	OrdersText->SetText(FText::Join(FText::FromString(TEXT("\n")), Lines));
	OrdersText->SetVisibility(Lines.Num() > 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UCL9HUDWidget::RefreshMoney()
{
	const ACL9GameState* GameState = GetCL9GameState();
	if (MoneyText == nullptr || GameState == nullptr)
	{
		return;
	}

	// 판 전체가 지갑 하나를 쓴다 — 호스트가 지급·차감하면 OnMoneyChanged가 모든 머신에서 발동한다
	MoneyText->SetText(FText::Format(NSLOCTEXT("CL9", "HudMoney", "{0} 이더"), GameState->GetSharedMoney()));
}

void UCL9HUDWidget::DeriveOrderScreen()
{
	const ACL9GameState* GameState = GetCL9GameState();
	UCL9UIManager* UIManager = UCL9UIManager::Get(this);
	if (GameState == nullptr || UIManager == nullptr)
	{
		return;
	}

	// "지금 열려 있어야 할 Screen"은 상태에서 파생한다 (docs/systems/ui_system.md 데이터 연결):
	// 테이블에 offer가 있으면 선택 화면, offer가 비워지면 화면 없음.
	const bool bOffered = GameState->OfferedOrders.Num() > 0;

	// **다른 전면 화면이 떠 있으면 끼어들지 않는다.** Screen 레이어는 배타적이라 여기서 열면
	// 방금 열린 화면(새벽 결과·트럭 패널·메뉴)을 밀어낸다 — 새벽 결과는 로비 진입과 같은
	// 순간에 열리므로 이 규칙이 없으면 플레이어가 결과를 읽을 새가 없다(2026-09-07 실측).
	// 상태에서 파생되는 화면은 사람이 연 화면에 양보하고, 그 화면이 닫히면 틱이 다시 파생한다.
	const FName OpenScreen = UIManager->GetOpenScreen();
	const bool bOtherScreenOpen = !OpenScreen.IsNone() && OpenScreen != CL9UINames::OrderSelect;

	if (bOffered && !bOtherScreenOpen && !UIManager->IsOpen(CL9UINames::OrderSelect))
	{
		UIManager->OpenWidget(CL9UINames::OrderSelect);
	}
	else if (!bOffered && UIManager->IsOpen(CL9UINames::OrderSelect))
	{
		UIManager->CloseWidget(CL9UINames::OrderSelect);
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

void UCL9HUDWidget::RefreshPeriod()
{
	const ACL9GameState* GameState = GetCL9GameState();
	if (PeriodText == nullptr || GameState == nullptr)
	{
		return;
	}

	const UWorld* World = GetWorld();
	const UCL9GameInstance* GameInstance = (World != nullptr) ? World->GetGameInstance<UCL9GameInstance>() : nullptr;
	const UCL9DefRegistry* Registry = (GameInstance != nullptr) ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9ProgressionDef* Progression = (Registry != nullptr) ? Registry->GetProgression() : nullptr;
	if (Progression == nullptr)
	{
		PeriodText->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const int32 Day = GameState->GetCurrentDay();
	// 정산이 언제인지는 진행 표가 정한다 — 화면은 그 판정을 앞으로 훑어 "며칠 남았는지"만 센다
	int32 SettlementDay = INDEX_NONE;
	for (int32 Ahead = 0; Ahead <= PeriodLookahead; ++Ahead)
	{
		if (Progression->IsSettlementDay(Day + Ahead))
		{
			SettlementDay = Day + Ahead;
			break;
		}
	}
	if (SettlementDay == INDEX_NONE)
	{
		PeriodText->SetVisibility(ESlateVisibility::Collapsed);   // 정산이 없는 판이다
		return;
	}

	const int32 Quota = Progression->QuotaAt(SettlementDay);
	if (Quota <= 0)
	{
		PeriodText->SetVisibility(ESlateVisibility::Collapsed);   // 목표 없음 — 빈 커브는 0을 뜻한다
		return;
	}

	const int32 Earned = GameState->GetPeriodIncome();
	const int32 Left = SettlementDay - Day;
	PeriodText->SetVisibility(ESlateVisibility::HitTestInvisible);
	PeriodText->SetText(Left == 0
		? FText::Format(NSLOCTEXT("CL9", "HudPeriodToday", "이번 기간 {0} / 목표 {1} · 오늘 정산"), Earned, Quota)
		: FText::Format(NSLOCTEXT("CL9", "HudPeriod", "이번 기간 {0} / 목표 {1} · 정산까지 {2}일"), Earned, Quota, Left));
	PeriodText->SetColorAndOpacity(FSlateColor(Earned >= Quota ? PeriodMetColor : PeriodShortColor));
}

void UCL9HUDWidget::RefreshCounter()
{
	if (CounterText == nullptr)
	{
		return;
	}

	const ACL9Character* Character = WiredCharacter.Get();
	const ACL9Counter* Counter = (Character != nullptr) ? Cast<ACL9Counter>(Character->GetFocusedActor()) : nullptr;
	if (Counter == nullptr)
	{
		CounterText->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const TArray<FCL9CounterItem>& Stock = Counter->GetStock();
	if (Stock.Num() == 0)
	{
		CounterText->SetText(NSLOCTEXT("CL9", "HudCounterEmpty", "창구가 비어 있다"));
		CounterText->SetVisibility(ESlateVisibility::HitTestInvisible);
		return;
	}

	const UWorld* World = GetWorld();
	const UCL9GameInstance* GameInstance = (World != nullptr) ? World->GetGameInstance<UCL9GameInstance>() : nullptr;
	const UCL9DefRegistry* Registry = (GameInstance != nullptr) ? GameInstance->GetDefRegistry() : nullptr;

	// 같은 물건은 한 줄로 묶는다 — 계란 다섯 개가 다섯 줄이 되면 읽을 수 없다
	struct FLine
	{
		FName Alias;
		FName SubAlias;
		int32 Count = 0;
		int32 Value = 0;
	};
	TArray<FLine> Grouped;
	int32 Total = 0;
	for (const FCL9CounterItem& Entry : Stock)
	{
		Total += Entry.Value;
		FLine* Found = Grouped.FindByPredicate([&Entry](const FLine& Line)
		{
			return Line.Alias == Entry.ItemAlias && Line.SubAlias == Entry.ItemSubAlias;
		});
		if (Found == nullptr)
		{
			Found = &Grouped.AddDefaulted_GetRef();
			Found->Alias = Entry.ItemAlias;
			Found->SubAlias = Entry.ItemSubAlias;
		}
		++Found->Count;
		Found->Value += Entry.Value;
	}

	TArray<FText> Lines;
	Lines.Add(NSLOCTEXT("CL9", "HudCounterTitle", "창구에 올린 것"));
	for (const FLine& Line : Grouped)
	{
		const UCL9ItemDef* ItemDef = (Registry != nullptr) ? Registry->FindItem(Line.Alias, Line.SubAlias) : nullptr;
		const FText Name = (ItemDef != nullptr && !ItemDef->DisplayName.IsEmpty()) ? ItemDef->DisplayName : FText::FromName(Line.Alias);
		Lines.Add(FText::Format(NSLOCTEXT("CL9", "HudCounterLine", "{0} ×{1} · {2} 이더"), Name, Line.Count, Line.Value));
	}
	// 합계는 올릴 때 굴린 값의 합이다 — 실제로 받는 돈은 팔 때 목적지·일자 배율이 곱해진다(economy.md 가격식)
	Lines.Add(FText::Format(NSLOCTEXT("CL9", "HudCounterTotal", "합 {0} 이더"), Total));

	CounterText->SetText(FText::Join(FText::FromString(TEXT("\n")), Lines));
	CounterText->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCL9HUDWidget::RefreshBuffs()
{
	if (BuffText == nullptr)
	{
		return;
	}

	const ACL9Character* Character = GetOwnCharacter();
	if (Character == nullptr)
	{
		BuffText->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const UWorld* World = GetWorld();
	const UCL9GameInstance* GameInstance = (World != nullptr) ? World->GetGameInstance<UCL9GameInstance>() : nullptr;
	const UCL9DefRegistry* Registry = (GameInstance != nullptr) ? GameInstance->GetDefRegistry() : nullptr;

	// 버프 단위로 한 줄씩 — 스탯이 아니라 이름으로 읽히는 것이 버프 표를 만든 이유다
	TArray<FText> Lines;
	for (const FName BuffAlias : Character->GetActiveBuffs())
	{
		const int32 Seconds = FMath::CeilToInt(Character->GetBuffRemaining(BuffAlias));
		if (Seconds <= 0)
		{
			continue;
		}

		// 행을 못 찾거나 이름이 비어 있으면 별칭을 그대로 보여준다 — 표의 구멍이
		// 화면에 드러나야 고치게 된다. 빈 줄로 삼키면 "먹었는데 아무 일도 없다"가 된다.
		const UCL9BuffDef* Buff = (Registry != nullptr) ? Registry->FindBuff(BuffAlias) : nullptr;
		const FText Name = (Buff != nullptr && !Buff->DisplayName.IsEmpty())
			? Buff->DisplayName
			: FText::FromName(BuffAlias);

		// 스탯 하나짜리는 크기까지 붙인다("다리에 힘 +60"). 여럿을 묶은 양날 버프는
		// 한 줄에 늘어놓으면 읽히지 않으므로 이름만 보여준다.
		if (Buff != nullptr && Buff->Effects.Num() == 1)
		{
			const ECL9Stat Stat = Buff->Effects[0].Stat;
			Lines.Add(FText::Format(NSLOCTEXT("CL9", "HudBuffNamedOne", "{0} {1} ({2}초)"),
				Name, StatAmountText(Stat, Character->GetStatBonus(Stat)), Seconds));
		}
		else
		{
			Lines.Add(FText::Format(NSLOCTEXT("CL9", "HudBuffNamed", "{0} ({1}초)"), Name, Seconds));
		}
	}

	// 별칭 없이 걸린 효과는 위 목록에 잡히지 않는다(버프 표로 옮기기 전의 아이템 행).
	// 그때만 예전처럼 스탯 단위로 보여준다 — 표가 덜 채워져도 화면이 비지 않게.
	if (Lines.Num() == 0)
	{
		for (const ECL9Stat Stat : AllStats)
		{
			const float Amount = Character->GetStatBonus(Stat);
			const int32 Seconds = FMath::CeilToInt(Character->GetStatBonusRemaining(Stat));
			// 값이 0이면 서로 상쇄된 것이고, 남은 시간이 0이면 이미 끝난 것이다 — 둘 다 보여줄 게 없다
			if (FMath::IsNearlyZero(Amount) || Seconds <= 0)
			{
				continue;
			}
			Lines.Add(FText::Format(NSLOCTEXT("CL9", "HudBuffLine", "{0} {1} ({2}초)"),
				StatLabel(Stat), StatAmountText(Stat, Amount), Seconds));
		}
	}

	if (Lines.Num() == 0)
	{
		BuffText->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	BuffText->SetText(FText::Join(FText::FromString(TEXT("\n")), Lines));
	BuffText->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCL9HUDWidget::RefreshNightWarn(float InDeltaTime)
{
	if (NightWarnText == nullptr)
	{
		return;
	}

	const ACL9GameState* GameState = GetCL9GameState();
	const float Remaining = (GameState != nullptr && GameState->GetPhase() == ECL9NightPhase::Night)
		? GameState->GetNightRemaining()
		: 0.f;

	if (Remaining <= 0.f || Remaining > NightWarnTime)
	{
		// 꺼져 있는 동안 위상을 되돌린다 — 다음에 켜질 때 항상 가장 밝은 지점에서 시작한다
		NightWarnPhase = 0.f;
		NightWarnText->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	NightWarnText->SetText(FText::Format(NSLOCTEXT("CL9", "HudNightWarn", "밤이 끝난다 · {0}초"),
		FMath::CeilToInt(Remaining)));
	NightWarnText->SetVisibility(ESlateVisibility::HitTestInvisible);

	if (NightWarnFlashTime > 0.f)
	{
		NightWarnPhase = FMath::Fmod(NightWarnPhase + InDeltaTime, NightWarnFlashTime);
		const float Wave = 0.5f * (1.f + FMath::Cos(2.f * PI * NightWarnPhase / NightWarnFlashTime));
		NightWarnText->SetRenderOpacity(FMath::Lerp(NightWarnDimOpacity, 1.f, Wave));
	}
	else
	{
		NightWarnText->SetRenderOpacity(1.f);
	}
}
