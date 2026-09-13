// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CL9HotbarSlotWidget.generated.h"

class UBorder;
class UTextBlock;

/**
 * 핫바 칸 하나 (docs/systems/ui.md 핫바). HUD가 `CL9Rules.HotbarSlots`개를 만들어 `HotbarBox`에
 * 나란히 붙이고, PlayerState의 `Slots`·`ActiveSlot`이 바뀔 때마다(`OnCarryChanged`) 이름과 활성
 * 여부를 다시 채운다. 칸의 생김새(크기·테두리·글꼴)는 전부 WBP_HotbarSlot이 정한다 — C++은
 * 번호·이름 글자와 테두리 색만 바꾼다.
 */
UCLASS(Abstract)
class UCL9HotbarSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 칸 번호(1부터) — 만들 때 한 번 찍는다. 숫자키 표기와 같다 */
	void SetNumber(int32 Number);

	/** 내용물 이름(빈 칸이면 빈 텍스트)과 활성 여부 — 칸이 바뀔 때마다 불린다 */
	void Refresh(const FText& Name, bool bActive);

protected:
	/** 칸 번호 "1"~"4" */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> NumberText;

	/** 든 아이템의 표시 이름 — 빈 칸은 비운다 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> NameText;

	/** 칸 테두리 — 활성 칸만 ActiveFrameColor로 강조한다 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> Frame;

	/** 손에 들린(활성) 칸의 테두리 색 — WBP_HotbarSlot에서 편집 */
	UPROPERTY(EditAnywhere, Category="Hotbar")
	FLinearColor ActiveFrameColor = FLinearColor(0.95f, 0.85f, 0.4f, 1.f);

	/** 나머지 칸의 테두리 색 */
	UPROPERTY(EditAnywhere, Category="Hotbar")
	FLinearColor InactiveFrameColor = FLinearColor(0.3f, 0.3f, 0.3f, 0.8f);
};
