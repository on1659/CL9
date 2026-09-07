// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "UI/CL9Widget.h"
#include "CL9OrderSelectWidget.generated.h"

class ACL9GameState;
class UButton;
class UCL9DefRegistry;
class UCL9OrderSelectWidget;
class UTextBlock;
class UVerticalBox;

/** 카드별 클릭 릴레이 — UButton::OnClicked는 페이로드를 실어 나르지 않으므로, 카드마다 이걸 하나씩 갖는다 */
UCLASS()
class UCL9OrderCardRelay : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UCL9OrderSelectWidget> Owner;

	int32 Index = 0;

	UFUNCTION()
	void HandleClicked();
};

/**
 * 로비의 주문 선택 화면(docs/systems/ui.md 로비 · order.md 주문 흐름): UI 테이블의
 * "OrderSelect" 행이다 — GameState.OfferedOrders가 비어 있지 않은 동안 HUD가 이 화면을
 * 열고, 호스트의 선택이 offer를 비우면 닫는다. 카드는 offer로부터 런타임에 만들어지고,
 * [확정]은 ACL9PlayerController::AcceptOrders로 이어지는데 호스트만 가능하다(클라이언트도
 * 선택 화면은 보이지만 버튼은 비활성 상태로 남는다).
 */
UCLASS(Abstract)
class UCL9OrderSelectWidget : public UCL9Widget
{
	GENERATED_BODY()

public:
	/** 카드 클릭 — OrdersRequired개까지 선택할 수 있고, 다시 클릭하면 선택이 풀린다 */
	void ToggleCard(int32 Index);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** "주문서 · N개 중 M개 선택" */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	/** 제안된 주문마다 버튼 하나씩, Rebuild에서 만들어진다 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> CardsBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> AcceptButton;

	/** "확정 (선택/M)" */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> AcceptText;

	/** 호스트에게는 고르는 방법을, 클라이언트에게는 누가 확정하는지를 알려준다 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HintText;

	UPROPERTY(EditAnywhere, Category="Orders")
	FLinearColor CardColor = FLinearColor(0.16f, 0.16f, 0.16f, 0.92f);

	UPROPERTY(EditAnywhere, Category="Orders")
	FLinearColor CardSelectedColor = FLinearColor(0.95f, 0.75f, 0.25f, 1.f);

	UPROPERTY(EditAnywhere, Category="Orders")
	FLinearColor CardTextColor = FLinearColor::White;

	/** 카드의 주문 이름 글자 크기 — 상세보다 커야 무엇이 이름인지 눈에 먼저 들어온다 */
	UPROPERTY(EditAnywhere, Category="Orders")
	int32 CardNameSize = 20;

	/** 카드의 재료·보상·마감 줄 글자 크기 */
	UPROPERTY(EditAnywhere, Category="Orders")
	int32 CardDetailSize = 16;

private:
	UFUNCTION()
	void HandleOrdersChanged();

	UFUNCTION()
	void HandleAcceptClicked();

	/** GameState는 construct 이후에 복제되어 들어온다 — 존재할 때까지 재시도한 뒤 구독한다 */
	void TryWire();

	/** GameState.OfferedOrders로부터 카드를 다시 만들고 선택 상태를 비운다 */
	void Rebuild();

	/** 카드 색상, 확정 라벨, 그리고 [확정]을 누를 수 있는지 여부 */
	void RefreshSelection();

	/** 카드 윗줄 — 주문 이름 */
	FText MakeOrderName(FName Alias) const;

	/** 카드 아랫줄 — "재료×n, … · 보상 a~b 이더 · 마감 d일" */
	FText MakeOrderDetail(FName Alias) const;

	const UCL9DefRegistry* GetRegistry() const;

	/** 폰트 에셋은 유지한 채 크기와 굵기만 바꾼 사본 */
	static FSlateFontInfo ScaledFont(FSlateFontInfo Font, int32 Size, FName Typeface);

	ACL9GameState* GetCL9GameState() const;

	UPROPERTY()
	TArray<TObjectPtr<UButton>> Cards;

	UPROPERTY()
	TArray<TObjectPtr<UCL9OrderCardRelay>> Relays;

	TArray<int32> Selected;

	FTimerHandle WireTimer;
	bool bWired = false;
};
