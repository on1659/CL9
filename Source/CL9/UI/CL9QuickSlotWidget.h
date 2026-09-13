// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CL9QuickSlotWidget.generated.h"

class UBorder;
class UImage;
class UTextBlock;
class UTexture2D;

/**
 * 퀵슬롯 칸 하나 (docs/systems/ui.md 퀵슬롯). HUD가 `CL9Rules.QuickSlots`개를 만들어 `QuickSlotBox`에
 * 나란히 붙이고, PlayerState의 `Slots`·`ActiveSlot`이 바뀔 때마다(`OnCarryChanged`) 내용과 활성
 * 여부를 다시 채운다. 리썰식이다 — 정사각 칸, 검은 반투명 바탕, 얇은 흰 테두리, 아이콘, 글자 없음.
 * 든 칸만 흰색 100%(ActiveColor), 나머지는 흰색 40%(InactiveColor) — 색으로 강조하지 않는다.
 * 칸의 생김새(크기·테두리 두께·배경·글꼴)는 전부 WBP_QuickSlot이 정한다 — C++은 아이콘·이름의 표시
 * 여부와 밝기만 바꾼다. 아이콘이 없는 행은 이름 글자로 물러선다.
 */
UCLASS(Abstract)
class UCL9QuickSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 내용물과 활성 여부 — 칸이 바뀔 때마다 불린다. 아이콘이 있으면 아이콘만, 없으면 이름만, 빈 칸
	 * (Name이 빔)이면 둘 다 숨긴다. 밝기(Active/InactiveColor)는 테두리·아이콘·이름에 같이 건다.
	 */
	void Refresh(const FText& Name, UTexture2D* InIcon, bool bActive);

protected:
	/** 칸 테두리 — 브러시 색으로 밝기를 건다 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> Frame;

	/** 든 아이템의 아이콘(`UCL9DefBase::Icon`) — 텍스처는 C++이 세팅한다. 없어도 된다(이름으로 물러선다) */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> Icon;

	/** 아이콘이 없는 행의 폴백 — 든 아이템의 표시 이름 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> NameText;

	/** 손에 들린(활성) 칸의 색 — 흰색 100% */
	UPROPERTY(EditAnywhere, Category="QuickSlot")
	FLinearColor ActiveColor = FLinearColor(1.f, 1.f, 1.f, 1.f);

	/** 나머지 칸의 색 — 흰색 40% */
	UPROPERTY(EditAnywhere, Category="QuickSlot")
	FLinearColor InactiveColor = FLinearColor(1.f, 1.f, 1.f, 0.4f);
};
