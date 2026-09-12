// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9SettingsWidget.h"
#include "Internationalization/StringTableRegistry.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "PlayerMappableKeySettings.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/TextLocalizationManager.h"
#include "TimerManager.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "CL9.h"
#include "Core/CL9InputText.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameUserSettings.h"
#include "UI/CL9UITable.h"

/**
 * 키 캡처 프리프로세서 — 위젯·뷰포트 라우팅보다 먼저 모든 키·마우스 버튼을 받는다(docs/systems/input.md 캡처).
 * 앱 전역이라 위젯이 사라지면 스스로 물러난다: 위젯이 무효면 소비하지 않고 등록을 푼다.
 */
class FCL9KeyCaptureProcessor : public IInputProcessor
{
public:
	explicit FCL9KeyCaptureProcessor(UCL9SettingsWidget* InWidget)
		: Widget(InWidget)
	{
	}

	TWeakPtr<IInputProcessor> Self;

	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override
	{
	}

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
	{
		return Forward(InKeyEvent.GetKey());
	}

	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
	{
		return IsActive();
	}

	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
	{
		return Forward(MouseEvent.GetEffectingButton());
	}

	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
	{
		return IsActive();
	}

	virtual bool HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
	{
		return IsActive();
	}

	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent) override
	{
		return Forward(EKeys::MouseWheelAxis);
	}

	virtual const TCHAR* GetDebugName() const override
	{
		return TEXT("CL9KeyCapture");
	}

private:
	bool IsActive() const
	{
		const UCL9SettingsWidget* Live = Widget.Get();
		return Live != nullptr && Live->IsCapturing();
	}

	bool Forward(const FKey& Key)
	{
		UCL9SettingsWidget* Live = Widget.Get();
		if (Live == nullptr)
		{
			// 위젯이 먼저 사라졌다(PIE 종료·트래블) — 에디터 키를 먹지 않도록 물러난다
			if (TSharedPtr<IInputProcessor> Pinned = Self.Pin(); Pinned.IsValid() && FSlateApplication::IsInitialized())
			{
				FSlateApplication::Get().UnregisterInputPreProcessor(Pinned);
			}
			return false;
		}
		return Live->HandleCapturedKey(Key);
	}

	TWeakObjectPtr<UCL9SettingsWidget> Widget;
};

void UCL9SettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (TitleText != nullptr)
	{
		TitleText->SetText(LOCTABLE("CL9", "Settings.Title"));
	}
	SetHint(FText::GetEmpty());

	if (UEnhancedInputUserSettings* UserSettings = GetUserSettings())
	{
		UserSettings->OnSettingsChanged.AddUniqueDynamic(this, &UCL9SettingsWidget::HandleUserSettingsChanged);
	}
	Rebuild();
}

void UCL9SettingsWidget::NativeDestruct()
{
	CancelCapture();
	if (UEnhancedInputUserSettings* UserSettings = GetUserSettings())
	{
		UserSettings->OnSettingsChanged.RemoveDynamic(this, &UCL9SettingsWidget::HandleUserSettingsChanged);
	}
	Super::NativeDestruct();
}

void UCL9SettingsWidget::OnClosing()
{
	// 페이드 뒤의 NativeDestruct를 기다리면 그동안 프리프로세서가 M·클릭을 전부 먹는다 — 닫히는 순간 푼다
	CancelCapture();
}

void UCL9SettingsWidget::Rebuild()
{
	if (RowsBox == nullptr)
	{
		return;
	}

	RowsBox->ClearChildren();
	KeyRows.Reset();
	Relays.Reset();
	LanguageButtons.Reset();
	VolumeValueLabels.Reset();
	SensitivityValueLabel = nullptr;

	// 키
	RebuildKeySection();

	// 마우스
	const UCL9GameUserSettings* Settings = GetGameSettings();
	AddSectionHeader(LOCTABLE("CL9", "Settings.Section.Mouse"));
	const float Sensitivity = Settings ? Settings->MouseSensitivity : 1.f;
	AddSlider(LOCTABLE("CL9", "Settings.MouseSensitivity"), Sensitivity, SensitivityRange.X, SensitivityRange.Y, UCL9SettingsRelay::EKind::SensitivitySlider, 0);
	AddToggle(LOCTABLE("CL9", "Settings.InvertY"), Settings ? Settings->bInvertY : false, UCL9SettingsRelay::EKind::InvertToggle);

	// 오디오 — 순서는 HandleVolumeChanged의 필드 순서와 같다
	AddSectionHeader(LOCTABLE("CL9", "Settings.Section.Audio"));
	const FText VolumeLabels[] = {
		LOCTABLE("CL9", "Settings.Volume.Master"),
		LOCTABLE("CL9", "Settings.Volume.Effects"),
		LOCTABLE("CL9", "Settings.Volume.UI"),
		LOCTABLE("CL9", "Settings.Volume.Ambience"),
		LOCTABLE("CL9", "Settings.Volume.Music"),
		LOCTABLE("CL9", "Settings.Volume.VO"),
	};
	const float Volumes[] = {
		Settings ? Settings->MasterVolume : 1.f,
		Settings ? Settings->EffectsVolume : 1.f,
		Settings ? Settings->UIVolume : 1.f,
		Settings ? Settings->AmbienceVolume : 1.f,
		Settings ? Settings->MusicVolume : 1.f,
		Settings ? Settings->VOVolume : 1.f,
	};
	for (int32 Index = 0; Index < static_cast<int32>(UE_ARRAY_COUNT(VolumeLabels)); ++Index)
	{
		AddSlider(VolumeLabels[Index], Volumes[Index], 0.f, 1.f, UCL9SettingsRelay::EKind::VolumeSlider, Index);
	}

	// 언어 — 라벨은 원어명(잘못 바꾼 사람이 자기 언어를 읽어야 한다)
	AddSectionHeader(LOCTABLE("CL9", "Settings.Section.Language"));
	TArray<FText> LanguageNames;
	int32 CurrentLanguage = INDEX_NONE;
	const FString CurrentCulture = GetCurrentCulture();
	for (int32 Index = 0; Index < Languages.Num(); ++Index)
	{
		const FCulturePtr Culture = FInternationalization::Get().GetCulture(Languages[Index]);
		LanguageNames.Add(FText::AsCultureInvariant(Culture.IsValid() ? Culture->GetNativeName() : Languages[Index]));
		if (Languages[Index] == CurrentCulture)
		{
			CurrentLanguage = Index;
		}
	}
	AddChoice(LOCTABLE("CL9", "Settings.Language"), LanguageNames, CurrentLanguage, UCL9SettingsRelay::EKind::LanguageChoice);
}

void UCL9SettingsWidget::RebuildKeySection()
{
	AddSectionHeader(LOCTABLE("CL9", "Settings.Section.Keys"), UCL9SettingsRelay::EKind::ResetKeys, LOCTABLE("CL9", "Settings.ResetKeys"), true);

	const UEnhancedInputUserSettings* UserSettings = GetUserSettings();
	if (UserSettings == nullptr)
	{
		return;
	}

	// 메뉴 키(IMC_UI에 있는 이름)는 마우스 버튼을 받지 않는다 — 덮개 클릭이 PlayerInput까지 내려가 메뉴를 닫는다
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9UITable* UITable = (GameInstance != nullptr) ? GameInstance->GetUITable() : nullptr;
	const UInputMappingContext* UIContext = (UITable != nullptr) ? UITable->UIMappingContext.Get() : nullptr;
	TSet<FName> UINames;
	if (UIContext != nullptr)
	{
		for (const FEnhancedActionKeyMapping& Mapping : UIContext->GetMappings())
		{
			if (Mapping.IsPlayerMappable())
			{
				UINames.Add(Mapping.GetMappingName());
			}
		}
	}

	// 행 순서는 등록된 IMC(게임플레이 → 관전 → UI)의 매핑 순서다 — 프로필의 TMap 순서를 쓰지 않는다
	TSet<FName> Seen;
	for (const UInputMappingContext* Context : UserSettings->GetRegisteredInputMappingContexts())
	{
		if (Context == nullptr)
		{
			continue;
		}
		for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings())
		{
			if (!Mapping.IsPlayerMappable() || Mapping.Key.IsGamepadKey())
			{
				continue;
			}
			const FName MappingName = Mapping.GetMappingName();
			if (MappingName.IsNone() || Seen.Contains(MappingName))
			{
				continue;
			}
			Seen.Add(MappingName);

			FKeyRow& Row = KeyRows.AddDefaulted_GetRef();
			Row.MappingName = MappingName;
			Row.bRejectsMouse = UINames.Contains(MappingName);

			// 동작 이름은 IA 에셋의 String Table 참조여야 한다 — 직접 친 텍스트는 규칙 위반이라 소리 내고 키 문자열을 보인다
			const FText DisplayName = Mapping.GetPlayerMappableKeySettings() ? Mapping.GetPlayerMappableKeySettings()->DisplayName : FText::GetEmpty();
			FName TableId;
			FTextKey TableKey;
			if (FTextInspector::GetTableIdAndKey(DisplayName, TableId, TableKey) && TableId == TEXT("CL9"))
			{
				Row.DisplayName = DisplayName;
			}
			else
			{
				UE_LOG(LogCL9, Error, TEXT("Settings: mapping '%s' display name is not a CL9 string table reference (docs/systems/input.md)."), *MappingName.ToString());
				Row.DisplayName = FText::FromName(MappingName);
			}
			AddKeyRow(KeyRows.Num() - 1);
		}
	}
	RefreshKeyLabels();
}

void UCL9SettingsWidget::AddSectionHeader(const FText& Title, UCL9SettingsRelay::EKind ButtonKind, const FText& ButtonLabel, bool bWithButton)
{
	UHorizontalBox* Line = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	if (Line == nullptr || Label == nullptr)
	{
		return;
	}
	Label->SetText(Title);
	Label->SetFont(ScaledFont(Label->GetFont(), HeaderSize, TEXT("Bold")));
	Label->SetColorAndOpacity(TextColor);
	if (UHorizontalBoxSlot* LabelSlot = Line->AddChildToHorizontalBox(Label))
	{
		LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		LabelSlot->SetVerticalAlignment(VAlign_Center);
	}

	if (bWithButton)
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		if (Button != nullptr && ButtonText != nullptr)
		{
			ButtonText->SetText(ButtonLabel);
			ButtonText->SetFont(ScaledFont(ButtonText->GetFont(), RowSize, TEXT("Regular")));
			Button->AddChild(ButtonText);
			Button->SetBackgroundColor(RowColor);
			Button->OnClicked.AddDynamic(MakeRelay(ButtonKind, 0), &UCL9SettingsRelay::HandleClicked);
			Line->AddChildToHorizontalBox(Button);
		}
	}

	if (UVerticalBoxSlot* LineSlot = RowsBox->AddChildToVerticalBox(Line))
	{
		LineSlot->SetPadding(FMargin(0.f, 14.f, 0.f, 4.f));
		LineSlot->SetHorizontalAlignment(HAlign_Fill);
	}
}

UHorizontalBox* UCL9SettingsWidget::AddRow(const FText& Label)
{
	UHorizontalBox* Line = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UTextBlock* LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	if (Line == nullptr || LabelText == nullptr)
	{
		return nullptr;
	}
	LabelText->SetText(Label);
	LabelText->SetFont(ScaledFont(LabelText->GetFont(), RowSize, TEXT("Regular")));
	LabelText->SetColorAndOpacity(TextColor);
	if (UHorizontalBoxSlot* LabelSlot = Line->AddChildToHorizontalBox(LabelText))
	{
		LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		LabelSlot->SetVerticalAlignment(VAlign_Center);
	}
	if (UVerticalBoxSlot* LineSlot = RowsBox->AddChildToVerticalBox(Line))
	{
		LineSlot->SetPadding(FMargin(8.f, 3.f));
		LineSlot->SetHorizontalAlignment(HAlign_Fill);
	}
	return Line;
}

UCL9SettingsRelay* UCL9SettingsWidget::MakeRelay(UCL9SettingsRelay::EKind Kind, int32 Index)
{
	UCL9SettingsRelay* Relay = NewObject<UCL9SettingsRelay>(this);
	Relay->Owner = this;
	Relay->Kind = Kind;
	Relay->Index = Index;
	Relays.Add(Relay);
	return Relay;
}

void UCL9SettingsWidget::AddKeyRow(int32 RowIndex)
{
	UHorizontalBox* Line = AddRow(KeyRows[RowIndex].DisplayName);
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	UTextBlock* KeyLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	if (Line == nullptr || Button == nullptr || KeyLabel == nullptr)
	{
		return;
	}
	KeyLabel->SetFont(ScaledFont(KeyLabel->GetFont(), RowSize, TEXT("Bold")));
	KeyLabel->SetJustification(ETextJustify::Center);
	KeyLabel->SetMinDesiredWidth(140.f);
	Button->AddChild(KeyLabel);
	Button->SetBackgroundColor(RowColor);
	Button->OnClicked.AddDynamic(MakeRelay(UCL9SettingsRelay::EKind::KeyButton, RowIndex), &UCL9SettingsRelay::HandleClicked);
	Line->AddChildToHorizontalBox(Button);
	KeyRows[RowIndex].KeyLabel = KeyLabel;
}

USlider* UCL9SettingsWidget::AddSlider(const FText& Label, float Value, float Min, float Max, UCL9SettingsRelay::EKind Kind, int32 Index)
{
	UHorizontalBox* Line = AddRow(Label);
	USlider* Slider = WidgetTree->ConstructWidget<USlider>(USlider::StaticClass());
	UTextBlock* ValueLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	if (Line == nullptr || Slider == nullptr || ValueLabel == nullptr)
	{
		return nullptr;
	}
	Slider->SetMinValue(Min);
	Slider->SetMaxValue(Max);
	Slider->SetStepSize(SliderStep);
	Slider->SetValue(Value);
	UCL9SettingsRelay* Relay = MakeRelay(Kind, Index);
	Slider->OnValueChanged.AddDynamic(Relay, &UCL9SettingsRelay::HandleValueChanged);
	Slider->OnMouseCaptureEnd.AddDynamic(Relay, &UCL9SettingsRelay::HandleCaptureEnd);
	if (UHorizontalBoxSlot* SliderSlot = Line->AddChildToHorizontalBox(Slider))
	{
		SliderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		SliderSlot->SetVerticalAlignment(VAlign_Center);
		SliderSlot->SetPadding(FMargin(8.f, 0.f));
	}

	ValueLabel->SetText(MakePercentText(Value));
	ValueLabel->SetFont(ScaledFont(ValueLabel->GetFont(), RowSize, TEXT("Regular")));
	ValueLabel->SetColorAndOpacity(TextColor);
	ValueLabel->SetMinDesiredWidth(64.f);
	ValueLabel->SetJustification(ETextJustify::Right);
	if (UHorizontalBoxSlot* ValueSlot = Line->AddChildToHorizontalBox(ValueLabel))
	{
		ValueSlot->SetVerticalAlignment(VAlign_Center);
	}

	if (Kind == UCL9SettingsRelay::EKind::SensitivitySlider)
	{
		SensitivityValueLabel = ValueLabel;
	}
	else
	{
		VolumeValueLabels.SetNum(FMath::Max(VolumeValueLabels.Num(), Index + 1));
		VolumeValueLabels[Index] = ValueLabel;
	}
	return Slider;
}

void UCL9SettingsWidget::AddToggle(const FText& Label, bool bValue, UCL9SettingsRelay::EKind Kind)
{
	UHorizontalBox* Line = AddRow(Label);
	UCheckBox* CheckBox = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass());
	if (Line == nullptr || CheckBox == nullptr)
	{
		return;
	}
	CheckBox->SetIsChecked(bValue);
	CheckBox->OnCheckStateChanged.AddDynamic(MakeRelay(Kind, 0), &UCL9SettingsRelay::HandleCheckChanged);
	if (UHorizontalBoxSlot* CheckSlot = Line->AddChildToHorizontalBox(CheckBox))
	{
		CheckSlot->SetVerticalAlignment(VAlign_Center);
	}
}

void UCL9SettingsWidget::AddChoice(const FText& Label, const TArray<FText>& Options, int32 Current, UCL9SettingsRelay::EKind Kind)
{
	// 세그먼트: 선택지마다 버튼 하나, 현재 값은 색으로 — ◀▶는 다음 값을 숨긴다
	UHorizontalBox* Line = AddRow(Label);
	if (Line == nullptr)
	{
		return;
	}
	for (int32 Index = 0; Index < Options.Num(); ++Index)
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		if (Button == nullptr || Text == nullptr)
		{
			continue;
		}
		Text->SetText(Options[Index]);
		Text->SetFont(ScaledFont(Text->GetFont(), RowSize, TEXT("Regular")));
		Button->AddChild(Text);
		Button->SetBackgroundColor(Index == Current ? SelectedColor : RowColor);
		Button->OnClicked.AddDynamic(MakeRelay(Kind, Index), &UCL9SettingsRelay::HandleClicked);
		if (UHorizontalBoxSlot* ButtonSlot = Line->AddChildToHorizontalBox(Button))
		{
			ButtonSlot->SetPadding(FMargin(2.f, 0.f));
		}
		if (Kind == UCL9SettingsRelay::EKind::LanguageChoice)
		{
			LanguageButtons.Add(Button);
		}
	}
}

// ---- 키 캡처

void UCL9SettingsWidget::HandleKeyButtonClicked(int32 RowIndex)
{
	if (!KeyRows.IsValidIndex(RowIndex))
	{
		return;
	}
	if (IsCapturing())
	{
		CancelCapture();
	}
	BeginCapture(RowIndex);
}

void UCL9SettingsWidget::BeginCapture(int32 RowIndex)
{
	CaptureRowIndex = RowIndex;
	if (UTextBlock* KeyLabel = KeyRows[RowIndex].KeyLabel.Get())
	{
		KeyLabel->SetText(LOCTABLE("CL9", "Settings.PressKey"));
	}
	SetHint(LOCTABLE("CL9", "Settings.CaptureHint"));

	if (!CaptureProcessor.IsValid())
	{
		CaptureProcessor = MakeShared<FCL9KeyCaptureProcessor>(this);
		CaptureProcessor->Self = CaptureProcessor;
	}
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().RegisterInputPreProcessor(CaptureProcessor, 0);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(CaptureTimer, this, &UCL9SettingsWidget::CancelCapture, CaptureTimeout, false);
	}
}

void UCL9SettingsWidget::EndCapture()
{
	CaptureRowIndex = INDEX_NONE;
	if (CaptureProcessor.IsValid() && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(CaptureProcessor);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CaptureTimer);
	}
}

void UCL9SettingsWidget::CancelCapture()
{
	if (!IsCapturing())
	{
		return;
	}
	EndCapture();
	RefreshKeyLabels();
	SetHint(FText::GetEmpty());
}

bool UCL9SettingsWidget::HandleCapturedKey(const FKey& Key)
{
	if (!IsCapturing())
	{
		return false;
	}
	if (Key == EKeys::Escape)
	{
		CancelCapture();
		return true;
	}

	// 거부: 게임패드·축·휠·콘솔(~)·전체화면 토글(F11)·메뉴 키에 마우스 버튼 — 힌트만 띄우고 계속 기다린다
	const bool bRejected = Key.IsGamepadKey() || Key.IsAxis1D() || Key.IsAxis2D() || Key.IsAxis3D()
		|| Key == EKeys::MouseWheelAxis || Key == EKeys::Tilde || Key == EKeys::F11
		|| (Key.IsMouseButton() && KeyRows[CaptureRowIndex].bRejectsMouse);
	if (bRejected)
	{
		SetHint(LOCTABLE("CL9", "Settings.KeyRejected"));
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(CaptureTimer, this, &UCL9SettingsWidget::CancelCapture, CaptureTimeout, false);
		}
		return true;
	}

	const int32 RowIndex = CaptureRowIndex;
	EndCapture();
	SetHint(FText::GetEmpty());
	ApplyCapturedKey(RowIndex, Key);
	RefreshKeyLabels();
	return true;
}

void UCL9SettingsWidget::ApplyCapturedKey(int32 RowIndex, const FKey& Key)
{
	UEnhancedInputUserSettings* UserSettings = GetUserSettings();
	if (UserSettings == nullptr)
	{
		return;
	}

	const FKeyRow& Row = KeyRows[RowIndex];
	const FPlayerKeyMapping* Current = CL9Input::FindKeyboardMapping(UserSettings, Row.MappingName);
	if (Current != nullptr && Current->GetCurrentKey() == Key)
	{
		return;
	}

	// Slot·장치를 기존 매핑의 값으로 채운다 — 기본 인자로 부르면 못 찾고 새 매핑을 추가해 옛 키와 새 키가 공존한다
	auto MakeArgs = [](const FPlayerKeyMapping* Mapping, FName MappingName, const FKey& NewKey)
	{
		FMapPlayerKeyArgs Args;
		Args.MappingName = MappingName;
		Args.NewKey = NewKey;
		if (Mapping != nullptr)
		{
			Args.Slot = Mapping->GetSlot();
			Args.HardwareDeviceId = Mapping->GetHardwareDeviceId().HardwareDeviceIdentifier;
			Args.bCreateMatchingSlotIfNeeded = false;
		}
		else
		{
			// 키보드 매핑이 하나도 없는 행(비어 있던 동작) — 첫 슬롯을 새로 만든다
			Args.Slot = EPlayerMappableKeySlot::First;
			Args.bCreateMatchingSlotIfNeeded = true;
		}
		return Args;
	};

	// 충돌: 같은 키를 쓰는 첫 행과 맞바꾼다 — 빈 행이 생기지 않고 막히는 경우도 없다 (docs/systems/input.md 충돌)
	FGameplayTagContainer Failure;
	for (int32 OtherIndex = 0; OtherIndex < KeyRows.Num(); ++OtherIndex)
	{
		if (OtherIndex == RowIndex)
		{
			continue;
		}
		const FPlayerKeyMapping* Other = CL9Input::FindKeyboardMapping(UserSettings, KeyRows[OtherIndex].MappingName);
		if (Other == nullptr || Other->GetCurrentKey() != Key)
		{
			continue;
		}
		const FKey OldKey = (Current != nullptr) ? Current->GetCurrentKey() : EKeys::Invalid;
		FMapPlayerKeyArgs SwapArgs = MakeArgs(Other, KeyRows[OtherIndex].MappingName, OldKey);
		SwapArgs.bDeferOnSettingsChangedBroadcast = true;
		UserSettings->MapPlayerKey(SwapArgs, Failure);
		SetHint(FText::Format(LOCTABLE("CL9", "Settings.KeySwapped"),
			Row.DisplayName, CL9Input::GetKeyText(Key), KeyRows[OtherIndex].DisplayName, CL9Input::GetKeyText(OldKey)));
		break;
	}

	UserSettings->MapPlayerKey(MakeArgs(Current, Row.MappingName, Key), Failure);
	if (!Failure.IsEmpty())
	{
		UE_LOG(LogCL9, Warning, TEXT("Settings: mapping '%s' to %s failed (%s)."), *Row.MappingName.ToString(), *Key.ToString(), *Failure.ToStringSimple());
	}
	UserSettings->SaveSettings();
}

void UCL9SettingsWidget::HandleResetKeysClicked()
{
	CancelCapture();
	UEnhancedInputUserSettings* UserSettings = GetUserSettings();
	if (UserSettings == nullptr)
	{
		return;
	}
	FGameplayTagContainer Failure;
	UserSettings->ResetKeyProfileIdToDefault(UserSettings->GetActiveKeyProfileId(), Failure);
	UserSettings->SaveSettings();
	RefreshKeyLabels();
	SetHint(FText::GetEmpty());
}

void UCL9SettingsWidget::HandleUserSettingsChanged(UEnhancedInputUserSettings* Settings)
{
	RefreshKeyLabels();
}

void UCL9SettingsWidget::RefreshKeyLabels()
{
	const UEnhancedInputUserSettings* UserSettings = GetUserSettings();
	for (int32 Index = 0; Index < KeyRows.Num(); ++Index)
	{
		UTextBlock* KeyLabel = KeyRows[Index].KeyLabel.Get();
		if (KeyLabel == nullptr || Index == CaptureRowIndex)
		{
			continue;
		}
		const FPlayerKeyMapping* Mapping = CL9Input::FindKeyboardMapping(UserSettings, KeyRows[Index].MappingName);
		const FText KeyText = (Mapping != nullptr) ? CL9Input::GetKeyText(Mapping->GetCurrentKey()) : FText::GetEmpty();
		KeyLabel->SetText(KeyText.IsEmpty() ? LOCTABLE("CL9", "Settings.Unbound") : KeyText);
	}
}

// ---- 마우스·오디오·언어

void UCL9SettingsWidget::HandleSensitivityChanged(float Value)
{
	if (UCL9GameUserSettings* Settings = GetGameSettings())
	{
		Settings->MouseSensitivity = Value;
	}
	if (UTextBlock* Label = SensitivityValueLabel.Get())
	{
		Label->SetText(MakePercentText(Value));
	}
}

void UCL9SettingsWidget::HandleInvertChanged(bool bChecked)
{
	if (UCL9GameUserSettings* Settings = GetGameSettings())
	{
		Settings->bInvertY = bChecked;
		Settings->SaveSettings();
	}
}

void UCL9SettingsWidget::HandleVolumeChanged(int32 VolumeIndex, float Value)
{
	UCL9GameUserSettings* Settings = GetGameSettings();
	if (Settings == nullptr)
	{
		return;
	}
	float* Fields[] = {&Settings->MasterVolume, &Settings->EffectsVolume, &Settings->UIVolume, &Settings->AmbienceVolume, &Settings->MusicVolume, &Settings->VOVolume};
	if (VolumeIndex < 0 || VolumeIndex >= static_cast<int32>(UE_ARRAY_COUNT(Fields)))
	{
		return;
	}
	*Fields[VolumeIndex] = Value;
	Settings->ApplyAudio(GetWorld());
	if (VolumeValueLabels.IsValidIndex(VolumeIndex))
	{
		if (UTextBlock* Label = VolumeValueLabels[VolumeIndex].Get())
		{
			Label->SetText(MakePercentText(Value));
		}
	}
}

void UCL9SettingsWidget::HandleSliderReleased()
{
	// 슬라이더는 드래그 중 적용, 놓을 때 저장 — 한 번 끌 때마다 파일을 쓰지 않는다
	if (UCL9GameUserSettings* Settings = GetGameSettings())
	{
		Settings->SaveSettings();
	}
}

void UCL9SettingsWidget::HandleLanguageClicked(int32 LanguageIndex)
{
	if (!Languages.IsValidIndex(LanguageIndex))
	{
		return;
	}
	UCL9GameUserSettings::ApplyLanguage(Languages[LanguageIndex]);
	// 화면의 문구는 FText가 지연 재빌드로 스스로 다시 그린다 — 행을 다시 만들지 않고 강조만 옮긴다
	for (int32 Index = 0; Index < LanguageButtons.Num(); ++Index)
	{
		if (LanguageButtons[Index] != nullptr)
		{
			LanguageButtons[Index]->SetBackgroundColor(Index == LanguageIndex ? SelectedColor : RowColor);
		}
	}
}

// ---- 보조

void UCL9SettingsWidget::SetHint(const FText& Text)
{
	if (HintText != nullptr)
	{
		HintText->SetText(Text);
		HintText->SetVisibility(Text.IsEmpty() ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
	}
}

void UCL9SettingsWidget::RefreshLanguageButtons()
{
	const FString CurrentCulture = GetCurrentCulture();
	for (int32 Index = 0; Index < LanguageButtons.Num(); ++Index)
	{
		if (LanguageButtons[Index] != nullptr && Languages.IsValidIndex(Index))
		{
			LanguageButtons[Index]->SetBackgroundColor(Languages[Index] == CurrentCulture ? SelectedColor : RowColor);
		}
	}
}

FString UCL9SettingsWidget::GetCurrentCulture() const
{
#if WITH_EDITOR
	if (GIsEditor)
	{
		// 에디터/PIE의 게임 언어는 미리보기다 — 현재 언어를 물으면 에디터 언어가 나온다
		const FString Preview = FTextLocalizationManager::Get().GetConfiguredGameLocalizationPreviewLanguage();
		if (!Preview.IsEmpty())
		{
			return Preview;
		}
	}
#endif
	return FInternationalization::Get().GetCurrentLanguage()->GetName();
}

FText UCL9SettingsWidget::MakePercentText(float Value)
{
	return FText::Format(LOCTABLE("CL9", "Settings.Percent"), FText::AsNumber(FMath::RoundToInt(Value * 100.f)));
}

UEnhancedInputUserSettings* UCL9SettingsWidget::GetUserSettings() const
{
	return CL9Input::GetUserSettings(GetOwningPlayer());
}

UCL9GameUserSettings* UCL9SettingsWidget::GetGameSettings() const
{
	return UCL9GameUserSettings::Get();
}

FSlateFontInfo UCL9SettingsWidget::ScaledFont(FSlateFontInfo Font, int32 Size, FName Typeface)
{
	Font.Size = Size;
	Font.TypefaceFontName = Typeface;
	return Font;
}

// ---- 릴레이

void UCL9SettingsRelay::HandleClicked()
{
	if (Owner == nullptr)
	{
		return;
	}
	switch (Kind)
	{
	case EKind::KeyButton: Owner->HandleKeyButtonClicked(Index); break;
	case EKind::ResetKeys: Owner->HandleResetKeysClicked(); break;
	case EKind::LanguageChoice: Owner->HandleLanguageClicked(Index); break;
	default: break;
	}
}

void UCL9SettingsRelay::HandleValueChanged(float Value)
{
	if (Owner == nullptr)
	{
		return;
	}
	switch (Kind)
	{
	case EKind::SensitivitySlider: Owner->HandleSensitivityChanged(Value); break;
	case EKind::VolumeSlider: Owner->HandleVolumeChanged(Index, Value); break;
	default: break;
	}
}

void UCL9SettingsRelay::HandleCaptureEnd()
{
	if (Owner != nullptr)
	{
		Owner->HandleSliderReleased();
	}
}

void UCL9SettingsRelay::HandleCheckChanged(bool bChecked)
{
	if (Owner != nullptr && Kind == EKind::InvertToggle)
	{
		Owner->HandleInvertChanged(bChecked);
	}
}
