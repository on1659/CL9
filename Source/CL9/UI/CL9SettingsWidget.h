// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "UI/CL9Widget.h"
#include "CL9SettingsWidget.generated.h"

class FCL9KeyCaptureProcessor;
class UButton;
class UCheckBox;
class UCL9GameUserSettings;
class UCL9SettingsWidget;
class UEnhancedInputUserSettings;
class UHorizontalBox;
class USlider;
class UTextBlock;
class UVerticalBox;
struct FKey;

/** 행 하나의 위젯 이벤트를 화면으로 되돌리는 릴레이 — UMG 델리게이트는 페이로드가 없어서 행마다 하나씩 둔다(트럭 패널 카드와 같다) */
UCLASS()
class UCL9SettingsRelay : public UObject
{
	GENERATED_BODY()

public:
	enum class EKind : uint8
	{
		KeyButton,
		ResetKeys,
		SensitivitySlider,
		InvertToggle,
		VolumeSlider,
		LanguageChoice,
	};

	UPROPERTY()
	TObjectPtr<UCL9SettingsWidget> Owner;

	EKind Kind = EKind::KeyButton;

	/** 키 행 번호 / 볼륨 번호 / 언어 번호 — 종류에 따라 뜻이 다르다 */
	int32 Index = 0;

	UFUNCTION()
	void HandleClicked();

	UFUNCTION()
	void HandleValueChanged(float Value);

	UFUNCTION()
	void HandleCaptureEnd();

	UFUNCTION()
	void HandleCheckChanged(bool bChecked);
};

/**
 * 설정 화면 (docs/systems/settings.md · ui.md 설정). UI 표의 `Settings` 행이며 메뉴 [설정]이 연다.
 * 섹션 넷(키·마우스·오디오·언어)의 행을 C++가 런타임에 만든다 — 옵션 추가 = 설정 클래스 필드 하나 + 여기 빌더
 * 호출 한 줄 + 문자열 시트 한 행. 값은 바꾸는 순간 적용하고 그 자리에서 저장한다(닫힘에 저장을 걸지 않는다).
 *
 * 키 리바인딩 규칙(등록·슬롯·캡처·충돌)은 docs/systems/input.md 리바인딩 절이 정본이고 이 화면은 그것을 부를 뿐이다.
 * 키 캡처는 Slate 입력 프리프로세서로 받는다 — 위젯 키 이벤트는 버튼이 먼저 먹고 덮개 클릭이 포커스를 빼앗는다.
 */
UCLASS(Abstract)
class UCL9SettingsWidget : public UCL9Widget
{
	GENERATED_BODY()

public:
	/** 릴레이가 부른다 */
	void HandleKeyButtonClicked(int32 RowIndex);
	void HandleResetKeysClicked();
	void HandleSensitivityChanged(float Value);
	void HandleInvertChanged(bool bChecked);
	void HandleVolumeChanged(int32 VolumeIndex, float Value);
	void HandleSliderReleased();
	void HandleLanguageClicked(int32 LanguageIndex);

	/**
	 * 프리프로세서가 부른다 — 캡처 중에 눌린 키·마우스 버튼. Esc는 취소, 거부 키는 힌트만, 나머지는 행에 넣는다.
	 * 캡처 중이 아니면 false(소비하지 않음).
	 */
	bool HandleCapturedKey(const FKey& Key);

	bool IsCapturing() const
	{
		return CaptureRowIndex != INDEX_NONE;
	}

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void OnClosing() override;

	/** "설정" */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	/** 섹션과 행이 런타임에 쌓이는 자리(스크롤 박스 안) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> RowsBox;

	/** 맞바꿈·거부·캡처 안내 줄 — 평소에는 비어 있고 다음 캡처를 시작할 때 지운다 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HintText;

	/** 감도 슬라이더 범위(배율) — 표시는 정수 퍼센트 */
	UPROPERTY(EditAnywhere, Category="Settings")
	FVector2D SensitivityRange = FVector2D(0.2f, 3.f);

	UPROPERTY(EditAnywhere, Category="Settings")
	float SliderStep = 0.05f;

	/** 언어 선택지 — 문화권 코드. 라벨은 그 언어의 원어명이라 시트에 없다(docs/systems/localization.md) */
	UPROPERTY(EditAnywhere, Category="Settings")
	TArray<FString> Languages = {TEXT("ko"), TEXT("en")};

	/** 키를 기다리다 이 시간(초) 동안 아무 입력이 없으면 취소한다 */
	UPROPERTY(EditAnywhere, Category="Settings")
	float CaptureTimeout = 5.f;

	UPROPERTY(EditAnywhere, Category="Settings")
	FLinearColor RowColor = FLinearColor(0.16f, 0.16f, 0.16f, 0.92f);

	/** 세그먼트에서 현재 선택된 값의 버튼 색 */
	UPROPERTY(EditAnywhere, Category="Settings")
	FLinearColor SelectedColor = FLinearColor(0.85f, 0.65f, 0.2f, 1.f);

	UPROPERTY(EditAnywhere, Category="Settings")
	FLinearColor TextColor = FLinearColor::White;

	/** 섹션 헤더 글자 크기 (docs/systems/ui.md 글자 위계 — 강조) */
	UPROPERTY(EditAnywhere, Category="Settings")
	int32 HeaderSize = 20;

	/** 행 라벨·값 글자 크기 (목록) */
	UPROPERTY(EditAnywhere, Category="Settings")
	int32 RowSize = 16;

private:
	/** 키 행 하나 — 매핑 이름과 그 행이 그리는 위젯 */
	struct FKeyRow
	{
		FName MappingName;
		FText DisplayName;
		/** IMC_UI에 같은 이름이 있는 동작(메뉴 키) — 마우스 버튼을 거부한다: 메뉴 덮개 클릭이 PlayerInput까지 내려간다 */
		bool bRejectsMouse = false;
		TWeakObjectPtr<UTextBlock> KeyLabel;
	};

	void Rebuild();
	void RebuildKeySection();

	// 행 빌더 — 옵션 하나 = 호출 한 줄
	void AddSectionHeader(const FText& Title, UCL9SettingsRelay::EKind ButtonKind = UCL9SettingsRelay::EKind::ResetKeys, const FText& ButtonLabel = FText::GetEmpty(), bool bWithButton = false);
	void AddKeyRow(int32 RowIndex);
	USlider* AddSlider(const FText& Label, float Value, float Min, float Max, UCL9SettingsRelay::EKind Kind, int32 Index);
	void AddToggle(const FText& Label, bool bValue, UCL9SettingsRelay::EKind Kind);
	void AddChoice(const FText& Label, const TArray<FText>& Options, int32 Current, UCL9SettingsRelay::EKind Kind);

	/** 라벨 + 오른쪽 위젯 한 줄의 틀 */
	UHorizontalBox* AddRow(const FText& Label);
	UCL9SettingsRelay* MakeRelay(UCL9SettingsRelay::EKind Kind, int32 Index);

	void BeginCapture(int32 RowIndex);
	void EndCapture();
	void CancelCapture();
	void ApplyCapturedKey(int32 RowIndex, const FKey& Key);

	/** 유저 설정이 바뀌면(리바인딩·초기화) 키 행의 글자를 전부 다시 읽는다 */
	UFUNCTION()
	void HandleUserSettingsChanged(UEnhancedInputUserSettings* Settings);
	void RefreshKeyLabels();

	void SetHint(const FText& Text);
	void RefreshLanguageButtons();
	FString GetCurrentCulture() const;
	static FText MakePercentText(float Value);

	UEnhancedInputUserSettings* GetUserSettings() const;
	UCL9GameUserSettings* GetGameSettings() const;

	/** 폰트 에셋은 유지한 채 크기와 굵기만 바꾼 사본 */
	static FSlateFontInfo ScaledFont(FSlateFontInfo Font, int32 Size, FName Typeface);

	TArray<FKeyRow> KeyRows;

	UPROPERTY()
	TArray<TObjectPtr<UCL9SettingsRelay>> Relays;

	/** 언어 세그먼트의 버튼들 — 현재 값을 색으로 강조한다 */
	UPROPERTY()
	TArray<TObjectPtr<UButton>> LanguageButtons;

	TWeakObjectPtr<UTextBlock> SensitivityValueLabel;
	TArray<TWeakObjectPtr<UTextBlock>> VolumeValueLabels;

	/** 지금 키를 기다리는 행, 없으면 INDEX_NONE */
	int32 CaptureRowIndex = INDEX_NONE;

	TSharedPtr<FCL9KeyCaptureProcessor> CaptureProcessor;
	FTimerHandle CaptureTimer;
};
