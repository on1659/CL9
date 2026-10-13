// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9HUDWidget.h"
#include "Internationalization/StringTableRegistry.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Character/CL9Character.h"
#include "Character/CL9CharacterMovement.h"
#include "Core/CL9InputText.h"
#include "Data/CL9BuffDef.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ItemDef.h"
#include "Data/CL9RulesDef.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameState.h"
#include "GameMode/CL9GameUserSettings.h"
#include "GameMode/CL9PlayerState.h"
#include "Items/CL9Counter.h"
#include "Items/CL9Item.h"
#include "Items/CL9Kitchen.h"
#include "Data/CL9OrderDef.h"
#include "Data/CL9ProgressionDef.h"
#include "UI/CL9CookGaugeWidget.h"
#include "UI/CL9QuickSlotWidget.h"
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
		case ECL9Stat::WalkSpeed:         return LOCTABLE("CL9", "Stat.WalkSpeed");
		case ECL9Stat::RunSpeed:          return LOCTABLE("CL9", "Stat.RunSpeed");
		case ECL9Stat::CarryMult:         return LOCTABLE("CL9", "Stat.CarryMult");
		case ECL9Stat::CartMult:          return LOCTABLE("CL9", "Stat.CartMult");
		case ECL9Stat::JumpVelocity:      return LOCTABLE("CL9", "Stat.JumpVelocity");
		case ECL9Stat::JumpGravity:       return LOCTABLE("CL9", "Stat.JumpGravity");
		case ECL9Stat::StaminaDrainTime:  return LOCTABLE("CL9", "Stat.StaminaDrain");
		case ECL9Stat::StaminaRegenTime:  return LOCTABLE("CL9", "Stat.StaminaRegen");
		case ECL9Stat::StaminaMinResume:  return LOCTABLE("CL9", "Stat.StaminaResume");
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
		return Value > 0.f ? FText::Format(LOCTABLE("CL9", "Stat.AmountPlus"), Number) : Number;
	}
}

void UCL9HUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 디자이너가 방향 호를 Collapsed로 두었으면 "끈 것"으로 읽고 이후 손대지 않는다 — 비네트와 달리 덮어쓰지 않는다
	bHurtDirectionDesignerHidden = (HurtDirection != nullptr) && (HurtDirection->GetVisibility() == ESlateVisibility::Collapsed);

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
		World->GetTimerManager().ClearTimer(HintTimer);
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
	// 창구 재고는 남이 올려도 알림이 없다 — 주문 줄의 "올린 수"도 여기서 본다(바뀐 틱에만 다시 그린다)
	PollCounterStock();
	RefreshBuffs();
	RefreshNightWarn(InDeltaTime);
	// 비네트도 시간 함수다 — 잡힌 서버 시각에서 파생되므로 변경 이벤트가 없고 매 프레임 읽는다
	RefreshHurtVignette();
	// 방향 호도 같은 시계에서 파생된다 — 카메라 요가 매 프레임 바뀌므로 각도도 매 프레임 읽는다
	RefreshHurtDirection();
	// 양보했던 선택 화면을 다시 열 때가 됐는지 본다 — 화면이 닫혔다는 이벤트가 없어서 틱에서 본다
	DeriveOrderScreen();
	DeriveCompanyScreen();
	TickOrderCard(InDeltaTime);
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
	PlayerState->OnImposterChanged.AddUObject(this, &UCL9HUDWidget::HandleImposterChanged);

	GetWorld()->GetTimerManager().SetTimer(ClockTimer, this, &UCL9HUDWidget::RefreshClock, 0.25f, true);

	RefreshHp();
	RefreshCarry();
	RefreshHead();
	RefreshClock();
	RefreshOrders();
	RefreshMoney();
	RefreshPeriod();
	DeriveOrderScreen();
	// 배선 전에 이미 임포스터로 뽑혔으면(복제가 HUD 보다 먼저 옴) 지금 알린다
	HandleImposterChanged();
	// 판 도중에 배선됐으면(늦게 접속) 카드도 한 번 띄운다
	bOrderCardPending = GameState->GetPhase() == ECL9NightPhase::Night;
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
	RefreshHint();
}

void UCL9HUDWidget::HandlePhaseChanged(ECL9NightPhase NewPhase)
{
	// 판이 열렸다 — 도착이 끝나면 "오늘의 주문" 카드를 띄운다(7j)
	bOrderCardPending = NewPhase == ECL9NightPhase::Night;
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

void UCL9HUDWidget::HandleImposterChanged()
{
	const ACL9PlayerState* PlayerState = GetOwnPlayerState();
	if (PlayerState == nullptr || !PlayerState->IsImposter())
	{
		return;   // 거둔 것(판 끝)은 알리지 않는다 — 결과는 새벽 화면이 모두에게 공개한다
	}
	UE_LOG(LogCL9, Log, TEXT("HUD: imposter notice (PlaceText %s)."), (PlaceText != nullptr) ? TEXT("bound") : TEXT("missing"));
	ShowPlaceNotice(LOCTABLE("CL9", "Hud.ImposterAssigned"), ImposterNoticeDuration);
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
	HpText->SetText(FText::Format(LOCTABLE("CL9", "Hud.Hp"), Hp));
}

void UCL9HUDWidget::RefreshCarry()
{
	const ACL9PlayerState* PlayerState = GetOwnPlayerState();
	if (QuickSlotBox == nullptr || QuickSlotClass == nullptr || PlayerState == nullptr)
	{
		return;
	}

	// 칸 수는 규칙이 정한다(CL9Rules.QuickSlots). PlayerState.Slots는 호스트가 BeginPlay에서 그 수로
	// 맞추지만 클라에는 복제가 늦어 잠시 비어 있을 수 있다 — 그동안도 칸은 규칙 수만큼 그리고,
	// 내용은 빈 칸으로 뒀다가 OnCarryChanged에서 다시 채운다(docs/systems/inventory.md 상태 모델).
	const UWorld* World = GetWorld();
	const UCL9GameInstance* GameInstance = (World != nullptr) ? World->GetGameInstance<UCL9GameInstance>() : nullptr;
	const UCL9DefRegistry* Registry = (GameInstance != nullptr) ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9RulesDef* Rules = (Registry != nullptr) ? Registry->GetRules() : nullptr;
	const int32 SlotCount = (Rules != nullptr) ? Rules->QuickSlots : PlayerState->Slots.Num();

	// 칸 위젯은 수가 달라질 때만 다시 만든다 — 규칙은 판 중에 바뀌지 않으므로 사실상 한 번이다
	if (QuickSlots.Num() != SlotCount)
	{
		QuickSlotBox->ClearChildren();
		QuickSlots.Reset();
		QuickSlotDefs.Reset();
		QuickSlotIcons.Reset();
		for (int32 Index = 0; Index < SlotCount; ++Index)
		{
			UCL9QuickSlotWidget* SlotWidget = CreateWidget<UCL9QuickSlotWidget>(GetOwningPlayer(), QuickSlotClass);
			if (SlotWidget == nullptr)
			{
				continue;
			}
			QuickSlotBox->AddChildToHorizontalBox(SlotWidget);
			SlotWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
			QuickSlots.Add(SlotWidget);
			QuickSlotDefs.Add(nullptr);
			QuickSlotIcons.Add(nullptr);
		}
	}

	for (int32 Index = 0; Index < QuickSlots.Num(); ++Index)
	{
		UCL9QuickSlotWidget* SlotWidget = QuickSlots[Index];
		if (SlotWidget == nullptr)
		{
			continue;
		}
		const bool bFilled = PlayerState->Slots.IsValidIndex(Index) && PlayerState->Slots[Index].IsSet();
		const FCL9CarriedItem* Carried = bFilled ? &PlayerState->Slots[Index] : nullptr;
		const UCL9ItemDef* Def = (Registry != nullptr && Carried != nullptr) ? Registry->FindItem(Carried->ItemAlias, Carried->ItemSubAlias) : nullptr;

		// 아이콘은 소프트 참조(UCL9DefBase::Icon) — 그 칸의 행이 바뀔 때만 동기 로드한다
		if (QuickSlotDefs[Index].Get() != Def)
		{
			QuickSlotDefs[Index] = Def;
			QuickSlotIcons[Index] = (Def != nullptr && !Def->Icon.IsNull()) ? Def->Icon.LoadSynchronous() : nullptr;
		}

		SlotWidget->Refresh(Carried != nullptr ? ItemLabel(*Carried) : FText::GetEmpty(),
			QuickSlotIcons[Index], Index == PlayerState->ActiveSlot);
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
		ClockText->SetText(FText::Format(LOCTABLE("CL9", "Hud.Night"),
			Day, Remaining / 60, FText::AsNumber(Remaining % 60, &TwoDigits),
			GameState->IsBlackout() ? LOCTABLE("CL9", "Hud.Blackout") : FText::GetEmpty()));
		break;
	}
	case ECL9NightPhase::Dawn:
		ClockText->SetText(FText::Format(LOCTABLE("CL9", "Hud.Dawn"), Day));
		break;
	default:
		ClockText->SetText(FText::Format(LOCTABLE("CL9", "Hud.Lobby"), Day));
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

void UCL9HUDWidget::RefreshHint()
{
	const ACL9PlayerState* PlayerState = GetOwnPlayerState();
	if (HintText == nullptr || PlayerState == nullptr)
	{
		return;
	}

	// OnCarryChanged는 다른 칸·머리 장비가 바뀌어도 오므로, 활성 칸의 아이템이 정말 바뀌었을 때만 판단한다
	const FCL9ItemRef Active = PlayerState->Slots.IsValidIndex(PlayerState->ActiveSlot)
		? static_cast<const FCL9ItemRef&>(PlayerState->Slots[PlayerState->ActiveSlot])
		: FCL9ItemRef();
	if (Active.ItemAlias == HintedItem.ItemAlias && Active.ItemSubAlias == HintedItem.ItemSubAlias)
	{
		return;
	}
	HintedItem = Active;

	// 설명이 필요한 종류(HasUseHint)를 이 PC에서 처음 들었을 때만 — 본 적 있음은 종류(Alias)별로 로컬 설정에 남는다
	// (docs/systems/ui.md 첫 줍기 힌트). 다른 것으로 바꿔 들면 떠 있던 힌트는 바로 접는다 — 남의 설명이 된다.
	const UWorld* World = GetWorld();
	const UCL9GameInstance* GameInstance = (World != nullptr) ? World->GetGameInstance<UCL9GameInstance>() : nullptr;
	const UCL9DefRegistry* Registry = (GameInstance != nullptr) ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9ItemDef* Def = (Registry != nullptr && Active.IsSet()) ? Registry->FindItem(Active.ItemAlias, Active.ItemSubAlias) : nullptr;
	UCL9GameUserSettings* Settings = UCL9GameUserSettings::Get();
	if (Def == nullptr || Settings == nullptr || !Def->HasUseHint() || Settings->HasSeenItemHint(Def->Alias))
	{
		HideHint();
		return;
	}

	Settings->MarkItemHintSeen(Def->Alias);
	HintText->SetText(BuildHintText(*Def));
	HintText->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (World != nullptr)
	{
		World->GetTimerManager().SetTimer(HintTimer, this, &UCL9HUDWidget::HideHint, HintDuration, false);
	}
}

FText UCL9HUDWidget::BuildHintText(const UCL9ItemDef& Def) const
{
	// 키 표기는 지금 바인딩된 키다 — 캐릭터의 IA_Use/IA_Scan을 유저 설정 프로필에서 읽는다(input.md 키 표기).
	// {0}=사용 키, {1}=스캔 키 — 줄마다 둘 다 넘기고 문구가 필요한 쪽만 쓴다
	const APlayerController* PlayerController = GetOwningPlayer();
	const ACL9Character* Character = GetOwnCharacter();
	const FText UseKey = CL9Input::GetActionKeyText(PlayerController, (Character != nullptr) ? Character->GetUseAction() : nullptr);
	const FText ScanKey = CL9Input::GetActionKeyText(PlayerController, (Character != nullptr) ? Character->GetScanAction() : nullptr);

	TArray<FText> Lines;
	switch (Def.UseAction)
	{
	case ECL9UseAction::Swing:
		Lines.Add(FText::Format(LOCTABLE("CL9", "Hint.UseTap.Swing"), UseKey, ScanKey));
		Lines.Add(FText::Format(LOCTABLE("CL9", "Hint.UseHold.Swing"), UseKey, ScanKey));
		break;
	case ECL9UseAction::ToggleLight:
		Lines.Add(FText::Format(LOCTABLE("CL9", "Hint.UseTap.ToggleLight"), UseKey, ScanKey));
		break;
	default:
		break;
	}
	if (Def.ItemType == ECL9ItemType::Dish)
	{
		Lines.Add(FText::Format(LOCTABLE("CL9", "Hint.ScanHold.Eat"), UseKey, ScanKey));
	}
	return FText::Join(FText::FromString(TEXT("\n")), Lines);
}

void UCL9HUDWidget::HideHint()
{
	if (HintText != nullptr)
	{
		HintText->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HintTimer);
	}
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
				Facts.Add(FText::Format(LOCTABLE("CL9", "Hud.ItemWeight"), FText::AsNumber(Def->Weight)));
			}
			switch (Def->CarrySlot)
			{
			case ECL9CarrySlot::Pocketable: Facts.Add(LOCTABLE("CL9", "Hud.ItemPocketable")); break;
			case ECL9CarrySlot::Hand:       Facts.Add(LOCTABLE("CL9", "Hud.ItemHandOnly")); break;
			case ECL9CarrySlot::CartOnly:   Facts.Add(LOCTABLE("CL9", "Hud.ItemCartOnly")); break;
			}
			if (Def->bFragile)
			{
				Facts.Add(LOCTABLE("CL9", "Hud.ItemFragile"));
			}
		}
		ItemInfoText->SetText(FText::Join(LOCTABLE("CL9", "Hud.ItemFactSep"), Facts));
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
		? FText::Format(LOCTABLE("CL9", "Hud.Head"), ItemLabel(PlayerState->HeadItem))
		: LOCTABLE("CL9", "Hud.HeadEmpty"));
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
	const TArray<FCL9AcceptedOrder>& Accepted = GameState->AcceptedOrders;

	// 창구에 남아 있는 재고를 종류별로 센다 — "올린 수"의 출처는 이것뿐이다. 퀵슬롯이나 수레에 든 것은
	// 세지 않는다: 정산도 창구 재고만 보기 때문에, 그것까지 세면 다 모은 것처럼 보이고 납품은 안 된다.
	struct FStockLeft
	{
		FName Alias;
		FName SubAlias;
		int32 Count = 0;
	};
	TArray<FStockLeft> Left;
	if (const ACL9Counter* Counter = FindDeliveryCounter())
	{
		for (const FCL9CounterItem& Entry : Counter->GetStock())
		{
			FStockLeft* Found = Left.FindByPredicate([&Entry](const FStockLeft& Line)
			{
				return Line.Alias == Entry.ItemAlias && Line.SubAlias == Entry.ItemSubAlias;
			});
			if (Found == nullptr)
			{
				Found = &Left.AddDefaulted_GetRef();
				Found->Alias = Entry.ItemAlias;
				Found->SubAlias = Entry.ItemSubAlias;
			}
			++Found->Count;
		}
	}

	// 배정은 마감 오름차순 StableSort로 돈다 — 같은 재료를 두 주문이 노리면 마감이 급한 쪽이 먼저
	// 가져간다(정산과 같다). 배정만 마감 순이고 줄의 표시 순서는 아래에서 AcceptedOrders 그대로다.
	TArray<int32> ByDeadline;
	for (int32 Index = 0; Index < Accepted.Num(); ++Index)
	{
		ByDeadline.Add(Index);
	}
	ByDeadline.StableSort([&Accepted](int32 A, int32 B)
	{
		return Accepted[A].DeadlineDay < Accepted[B].DeadlineDay;
	});

	// [주문][재료] = 창구에서 이 주문 몫으로 잡힌 수
	TArray<TArray<int32>> Mine;
	Mine.SetNum(Accepted.Num());
	TArray<const UCL9OrderDef*> Orders;
	Orders.SetNumZeroed(Accepted.Num());

	auto FindLeft = [&Left](const FCL9ItemCount& Ingredient) -> FStockLeft*
	{
		return Left.FindByPredicate([&Ingredient](const FStockLeft& Line)
		{
			return Line.Alias == Ingredient.ItemAlias && Line.SubAlias == Ingredient.ItemSubAlias;
		});
	};

	// ① **정산과 같은 단계** — 정산(ACL9GameMode::SettleOrdersAt)은 재료가 전부 있는 주문만 그 자리에서
	//    통째로 소비하고, 하나라도 모자란 주문은 아무것도 가져가지 않는다(반쯤 낸 주문은 없다). 여기서도
	//    그대로 한다. 부분 배정부터 하면 HUD가 거짓말을 한다: 익은 계란 1개가 남은 채 오믈렛(계란 3)과
	//    계란후라이(계란 1)를 같은 마감으로 받으면 "오믈렛 1/3 · 계란후라이 0/1"로 보이지만, 다음 납품에서
	//    정산은 오믈렛을 건너뛰고 그 계란을 계란후라이에 준다.
	TArray<int32> Short;
	for (int32 Index : ByDeadline)
	{
		const UCL9OrderDef* Order = (Registry != nullptr) ? Registry->FindOrder(Accepted[Index].OrderAlias) : nullptr;
		Orders[Index] = Order;
		if (Order == nullptr)
		{
			continue;
		}
		Mine[Index].SetNumZeroed(Order->Ingredients.Num());

		bool bComplete = true;
		for (const FCL9ItemCount& Ingredient : Order->Ingredients)
		{
			const FStockLeft* Found = FindLeft(Ingredient);
			if (Found == nullptr || Found->Count < Ingredient.Count)
			{
				bComplete = false;
				break;
			}
		}
		if (!bComplete)
		{
			Short.Add(Index);
			continue;
		}

		for (int32 Need = 0; Need < Order->Ingredients.Num(); ++Need)
		{
			const FCL9ItemCount& Ingredient = Order->Ingredients[Need];
			Mine[Index][Need] = Ingredient.Count;
			FindLeft(Ingredient)->Count -= Ingredient.Count;
		}
	}

	// ② **미리보기 단계** — ①에서 다 못 채운 주문에만, 다시 마감 순으로(Short가 그 순서로 쌓였다) 남은
	//    재고를 재료별 min(필요, 남은 수)로 나눠 준다. 정산에는 없는 단계다: 무엇을 얼마나 더 훔쳐 와야
	//    하는지 보여주는 것이 이 줄의 일이라, 아직 못 채우는 주문에도 지금까지 모인 수를 보여준다. 완납
	//    가능한 주문의 몫을 ①에서 먼저 떼어 낸 뒤에 나누므로, 다음 납품에 실제로 빠질 주문은 이미 n/n이다.
	for (int32 Index : Short)
	{
		const UCL9OrderDef* Order = Orders[Index];
		for (int32 Need = 0; Need < Order->Ingredients.Num(); ++Need)
		{
			const FCL9ItemCount& Ingredient = Order->Ingredients[Need];
			FStockLeft* Found = FindLeft(Ingredient);
			if (Found == nullptr)
			{
				continue;
			}
			Mine[Index][Need] = FMath::Min(Ingredient.Count, Found->Count);
			Found->Count -= Mine[Index][Need];
		}
	}

	TArray<FText> Lines;
	for (int32 Index = 0; Index < Accepted.Num(); ++Index)
	{
		const FCL9AcceptedOrder& Entry = Accepted[Index];
		const UCL9OrderDef* Order = (Registry != nullptr) ? Registry->FindOrder(Entry.OrderAlias) : nullptr;
		const FText Name = (Order != nullptr && !Order->DisplayName.IsEmpty()) ? Order->DisplayName : FText::FromName(Entry.OrderAlias);
		TArray<FText> Parts;
		if (Order != nullptr)
		{
			for (int32 Need = 0; Need < Order->Ingredients.Num(); ++Need)
			{
				const FCL9ItemCount& Ingredient = Order->Ingredients[Need];
				const UCL9ItemDef* ItemDef = Registry->FindItem(Ingredient.ItemAlias, Ingredient.ItemSubAlias);
				const FText ItemName = (ItemDef != nullptr && !ItemDef->DisplayName.IsEmpty()) ? ItemDef->DisplayName : FText::FromName(Ingredient.ItemAlias);
				// 재료가 다 모이면 정산이 그 자리에서 소비하고 줄이 사라진다 — "완료" 표시는 따로 없다
				Parts.Add(FText::Format(LOCTABLE("CL9", "Hud.OrderIngredientProgress"),
					ItemName, Mine[Index].IsValidIndex(Need) ? Mine[Index][Need] : 0, Ingredient.Count));
			}
		}
		// 보상은 수락할 때 굴려 확정된 값이다(2026-09-06 결정) — 카드의 범위가 아니라 실제로 받을 액수를 보여준다
		Lines.Add(FText::Format(LOCTABLE("CL9", "Hud.OrderLine"),
			Name, FText::Join(LOCTABLE("CL9", "Hud.OrderSep"), Parts), Entry.DeadlineDay, Entry.Reward));
	}

	OrdersText->SetText(FText::Join(FText::FromString(TEXT("\n")), Lines));
	OrdersText->SetVisibility(Lines.Num() > 0 ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UCL9HUDWidget::PollCounterStock()
{
	// 재고에는 OnRep이 없다 — 남이 올려도, 정산이 가져가도 알림이 오지 않는다. 그렇다고 매 틱 주문 줄을
	// 다시 조립하면 바뀐 것이 없어도 SetText를 하게 되므로, 재고의 서명(항목 수 + 항목마다의 alias/sub)이
	// 지난 틱과 다를 때만 다시 그린다. 값(Value)은 주문 줄에 쓰이지 않아 서명에 넣지 않는다.
	const ACL9Counter* Counter = FindDeliveryCounter();
	const int32 Num = (Counter != nullptr) ? Counter->GetStock().Num() : 0;

	bool bChanged = Num != CounterStock.Num();
	for (int32 Index = 0; !bChanged && Index < Num; ++Index)
	{
		const FCL9CounterItem& Entry = Counter->GetStock()[Index];
		bChanged = Entry.ItemAlias != CounterStock[Index].ItemAlias || Entry.ItemSubAlias != CounterStock[Index].ItemSubAlias;
	}
	if (!bChanged)
	{
		return;
	}

	CounterStock.Reset(Num);
	for (int32 Index = 0; Index < Num; ++Index)
	{
		CounterStock.Add(static_cast<const FCL9ItemRef&>(Counter->GetStock()[Index]));
	}
	RefreshOrders();
}

ACL9Counter* UCL9HUDWidget::FindDeliveryCounter()
{
	// 창구는 레벨에 하나다(BP_CL9Counter) — 한 번 찾아 들고 있는다. 클라에 아직 복제되지 않았거나
	// 창구가 없는 레벨이면 nullptr가 돌아가고(재고를 빈 것으로 본다), 다음 틱에 다시 찾는다.
	if (ACL9Counter* Cached = DeliveryCounter.Get())
	{
		return Cached;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return nullptr;
	}
	for (TActorIterator<ACL9Counter> It(World); It; ++It)
	{
		DeliveryCounter = *It;
		return *It;
	}
	return nullptr;
}

void UCL9HUDWidget::RefreshMoney()
{
	const ACL9GameState* GameState = GetCL9GameState();
	if (MoneyText == nullptr || GameState == nullptr)
	{
		return;
	}

	// 판 전체가 지갑 하나를 쓴다 — 호스트가 지급·차감하면 OnMoneyChanged가 모든 머신에서 발동한다
	MoneyText->SetText(FText::Format(LOCTABLE("CL9", "Hud.Money"), GameState->GetSharedMoney()));
}

void UCL9HUDWidget::TickOrderCard(float InDeltaTime)
{
	const ACL9GameState* GameState = GetCL9GameState();
	if (GameState == nullptr)
	{
		return;
	}

	// 판마다 한 번 — 도착이 있으면 끝난 뒤, 없으면(레버로 연 판) 바로
	const float Arrival = GameState->GetArrivalAlpha();
	if (bOrderCardPending && GameState->GetPhase() == ECL9NightPhase::Night && (Arrival < 0.f || Arrival >= 1.f))
	{
		bOrderCardPending = false;
		ShowOrderCard();
	}

	if (OrderCardElapsed < 0.f || OrderCard == nullptr)
	{
		return;
	}
	OrderCardElapsed += InDeltaTime;
	if (OrderCardElapsed >= OrderCardDuration || GameState->GetPhase() != ECL9NightPhase::Night)
	{
		OrderCard->SetVisibility(ESlateVisibility::Collapsed);
		OrderCardElapsed = -1.f;
		return;
	}
	const float Fade = FMath::Min(0.4f, OrderCardDuration * 0.5f);
	const float Alpha = Fade > KINDA_SMALL_NUMBER ? FMath::Min(OrderCardElapsed, OrderCardDuration - OrderCardElapsed) / Fade : 1.f;
	OrderCard->SetRenderOpacity(FMath::Clamp(Alpha, 0.f, 1.f));
}

void UCL9HUDWidget::ShowOrderCard()
{
	if (CookLayer == nullptr || OrdersText == nullptr || WidgetTree == nullptr)
	{
		return;
	}
	if (OrderCard == nullptr)
	{
		// 가운데 카드 — 어두운 반투명 바탕에 글 한 덩이. 글꼴은 HUD 주문 줄의 것(한글 글리프가 있는 글꼴)에서 크기만 바꾼다
		OrderCard = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		OrderCard->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.65f));
		OrderCard->SetPadding(FMargin(28.f, 18.f));
		OrderCardText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		FSlateFontInfo Font = OrdersText->GetFont();
		Font.Size = OrderCardTextSize;
		OrderCardText->SetFont(Font);
		OrderCardText->SetJustification(ETextJustify::Center);
		OrderCardText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		OrderCard->SetContent(OrderCardText);
		if (UCanvasPanelSlot* CardSlot = CookLayer->AddChildToCanvas(OrderCard))
		{
			CardSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			CardSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			CardSlot->SetAutoSize(true);
			CardSlot->SetPosition(FVector2D(0.f, -120.f));
			CardSlot->SetZOrder(100);
		}
	}

	const FText Orders = OrdersText->GetText();
	OrderCardText->SetText(FText::Join(FText::FromString(TEXT("\n")),
		TArray<FText>{ LOCTABLE("CL9", "Hud.OrderCardTitle"), Orders.IsEmpty() ? LOCTABLE("CL9", "Hud.OrderCardNone") : Orders }));
	OrderCard->SetRenderOpacity(0.f);
	OrderCard->SetVisibility(ESlateVisibility::HitTestInvisible);
	OrderCardElapsed = 0.f;
	UE_LOG(LogCL9, Log, TEXT("HUD: order card shown (%d chars of orders)."), Orders.ToString().Len());
}

void UCL9HUDWidget::DeriveCompanyScreen()
{
	if (bCompanyScreenShown)
	{
		return;
	}
	const ACL9GameState* GameState = GetCL9GameState();
	const UCL9GameInstance* GameInstance = Cast<UCL9GameInstance>(GetGameInstance());
	const UCL9DefRegistry* Registry = GameInstance != nullptr ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9RulesDef* Rules = Registry != nullptr ? Registry->GetRules() : nullptr;
	UCL9UIManager* UIManager = UCL9UIManager::Get(this);
	if (GameState == nullptr || Rules == nullptr || UIManager == nullptr || !Rules->IsCompanyWorld(GetWorld())
		|| GameState->GetLastSettlement().Day <= 0)
	{
		return;
	}

	// 한 번만 — 닫으면 회사를 걸어 다닌다
	bCompanyScreenShown = true;
	UIManager->OpenWidget(CompanyScreenRow);
	UE_LOG(LogCL9, Log, TEXT("HUD: company — settlement screen '%s' opened (day %d)."), *CompanyScreenRow.ToString(), GameState->GetLastSettlement().Day);
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
		? FText::Format(LOCTABLE("CL9", "Hud.PeriodToday"), Earned, Quota)
		: FText::Format(LOCTABLE("CL9", "Hud.Period"), Earned, Quota, Left));
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
		CounterText->SetText(LOCTABLE("CL9", "Hud.CounterEmpty"));
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
	Lines.Add(LOCTABLE("CL9", "Hud.CounterTitle"));
	for (const FLine& Line : Grouped)
	{
		const UCL9ItemDef* ItemDef = (Registry != nullptr) ? Registry->FindItem(Line.Alias, Line.SubAlias) : nullptr;
		const FText Name = (ItemDef != nullptr && !ItemDef->DisplayName.IsEmpty()) ? ItemDef->DisplayName : FText::FromName(Line.Alias);
		Lines.Add(FText::Format(LOCTABLE("CL9", "Hud.CounterLine"), Name, Line.Count, Line.Value));
	}
	// 합계는 올릴 때 굴린 값의 합이다 — 실제로 받는 돈은 팔 때 목적지·일자 배율이 곱해진다(economy.md 가격식)
	Lines.Add(FText::Format(LOCTABLE("CL9", "Hud.CounterTotal"), Total));

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
			Lines.Add(FText::Format(LOCTABLE("CL9", "Hud.BuffNamedOne"),
				Name, StatAmountText(Stat, Character->GetStatBonus(Stat)), Seconds));
		}
		else
		{
			Lines.Add(FText::Format(LOCTABLE("CL9", "Hud.BuffNamed"), Name, Seconds));
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
			Lines.Add(FText::Format(LOCTABLE("CL9", "Hud.BuffLine"),
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

	NightWarnText->SetText(FText::Format(LOCTABLE("CL9", "Hud.NightWarn"),
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

void UCL9HUDWidget::RefreshHurtVignette()
{
	if (HurtVignette == nullptr)
	{
		return;   // WBP_HUD에 비네트 이미지를 두지 않은 판 — 아무것도 하지 않는다
	}

	// 폰이 없는 동안(스폰 전·폰 교체 사이)은 0으로 본다 — 붉은 기가 화면에 남지 않게 한다
	const ACL9Character* Character = WiredCharacter.Get();
	const float Flash = (Character != nullptr) ? Character->GetHurtFlash() : 0.f;

	HurtVignette->SetRenderOpacity(Flash);

	// 0이면 접는다: 불투명도 0짜리 전면 이미지를 계속 그리지 않게, 그리고 잔상이 남을 여지를 없앤다
	const ESlateVisibility Wanted = (Flash > 0.f) ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
	if (HurtVignette->GetVisibility() != Wanted)
	{
		HurtVignette->SetVisibility(Wanted);
	}
}

void UCL9HUDWidget::RefreshHurtDirection()
{
	if (HurtDirection == nullptr || bHurtDirectionDesignerHidden)
	{
		return;   // WBP_HUD에 호가 없거나 디자이너가 Collapsed로 꺼 둔 판 — 아무것도 하지 않는다
	}

	// 표에서 끌 수 있다(CL9Rules.bHurtDirectionIndicator) — 규칙은 판 중에 바뀌지 않지만 값은 한 줄이라 매 틱 읽는다
	const UWorld* World = GetWorld();
	const UCL9GameInstance* GameInstance = (World != nullptr) ? World->GetGameInstance<UCL9GameInstance>() : nullptr;
	const UCL9DefRegistry* Registry = (GameInstance != nullptr) ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9RulesDef* Rules = (Registry != nullptr) ? Registry->GetRules() : nullptr;
	const bool bEnabled = (Rules != nullptr) && Rules->bHurtDirectionIndicator;

	// 폰이 없는 동안(스폰 전·관전)은 게터를 부를 캐릭터가 없으니 숨긴다. 치트 null·관전은 HasHurtDirection()이 거짓을 준다
	const ACL9Character* Character = WiredCharacter.Get();
	const float Flash = (bEnabled && Character != nullptr && Character->HasHurtDirection()) ? Character->GetHurtFlash() : 0.f;

	if (Flash <= 0.f)
	{
		// Collapsed가 아니라 Hidden — Collapsed는 디자이너의 신호로 남겨 둔다
		if (HurtDirection->GetVisibility() != ESlateVisibility::Hidden)
		{
			HurtDirection->SetVisibility(ESlateVisibility::Hidden);
		}
		return;
	}

	// 0 = 화면 위(12시), 시계 방향 양수 — 위젯 각도도 시계 방향 양수라 그대로 넣는다. 피벗은 WBP에서 화면 중심(0.5, 0.5)
	HurtDirection->SetRenderTransformAngle(Character->GetHurtDirectionAngle());
	HurtDirection->SetRenderOpacity(Flash);
	if (HurtDirection->GetVisibility() != ESlateVisibility::HitTestInvisible)
	{
		HurtDirection->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}
