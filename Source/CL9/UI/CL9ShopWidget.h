// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9Types.h"
#include "Fonts/SlateFontInfo.h"
#include "UI/CL9Widget.h"
#include "CL9ShopWidget.generated.h"

class ACL9GameState;
class UButton;
class UCL9DefRegistry;
class UCL9EquipmentDef;
class UCL9ShopWidget;
class UTextBlock;
class UVerticalBox;

/** 품목별 클릭 릴레이 — UButton::OnClicked는 페이로드를 실어 나르지 않으므로, 카드마다 이걸 하나씩 갖는다 */
UCLASS()
class UCL9ShopCardRelay : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UCL9ShopWidget> Owner;

	FName Alias;

	UFUNCTION()
	void HandleClicked();
};

/**
 * 트럭 패널 — 번 이더로 장비를 사는 화면이다 (docs/systems/shop.md). UI 테이블의 `Shop` 행이며,
 * 트럭 패널 액터가 상호작용한 사람에게 Client RPC로 연다(로비 주문 화면처럼 상태에서 파생시킬
 * 것이 없어서, 상호작용 즉시 연다).
 *
 * 파는 것은 실물뿐이고, 구매는 인벤토리에 넣는 게 아니라 트럭 옆에 물건을 스폰한다.
 * **살 수 있는지는 화면이 판단하지 않는다** — `ACL9GameState::CanBuy`를 그대로 읽어서 버튼을
 * 끄고 이유를 적는다. 화면이 규칙을 다시 구현하면 어긋나는 순간 거짓말을 하게 된다.
 * 클라이언트도 살 수 있으므로 그 판정은 GameState에 있다(모든 머신에 복제된다).
 */
UCLASS(Abstract)
class UCL9ShopWidget : public UCL9Widget
{
	GENERATED_BODY()

public:
	/** 카드의 버튼이 눌렸다 — 릴레이가 부른다 */
	void HandleBuyClicked(FName EquipAlias);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** "트럭 패널" */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	/** "보유 N 이더" — 판 공유 자금 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> MoneyText;

	/** 품목 카드가 런타임에 쌓이는 자리 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> ItemsBox;

	/** 거절 사유가 뜨는 줄 — 평소에는 비어 있다 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HintText;

	// [닫기] 버튼은 부모 UCL9Widget의 공통 CloseButton이 CloseSelf에 배선한다 (docs/systems/ui_system.md)

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CloseText;

	UPROPERTY(EditAnywhere, Category="Shop")
	FLinearColor CardColor = FLinearColor(0.16f, 0.16f, 0.16f, 0.92f);

	UPROPERTY(EditAnywhere, Category="Shop")
	FLinearColor CardTextColor = FLinearColor::White;

	/** 못 사는 품목의 글자 색 — 꺼진 버튼과 함께 읽힌다 */
	UPROPERTY(EditAnywhere, Category="Shop")
	FLinearColor CardLockedColor = FLinearColor(0.45f, 0.45f, 0.48f, 1.f);

	/** 품목 이름 글자 크기 (docs/systems/ui.md 글자 위계 — 강조 값) */
	UPROPERTY(EditAnywhere, Category="Shop")
	int32 CardNameSize = 20;

	/** 가격·사유 줄 글자 크기 (목록·힌트) */
	UPROPERTY(EditAnywhere, Category="Shop")
	int32 CardDetailSize = 16;

private:

	UFUNCTION()
	void HandleMoneyChanged();

	UFUNCTION()
	void HandleBuyRejected(FName EquipAlias, ECL9BuyResult Reason);

	/** GameState는 construct 이후에 복제되어 들어온다 — 존재할 때까지 재시도한 뒤 구독한다 */
	void TryWire();

	/** 장비 표에서 카드를 다시 만든다 */
	void Rebuild();

	/** 자금 줄과, 카드마다의 살 수 있음/없음을 다시 칠한다 */
	void RefreshAffordability();

	FText MakeReasonText(ECL9BuyResult Reason, const UCL9EquipmentDef& Equip) const;

	const UCL9DefRegistry* GetRegistry() const;
	ACL9GameState* GetCL9GameState() const;

	/** 폰트 에셋은 유지한 채 크기와 굵기만 바꾼 사본 */
	static FSlateFontInfo ScaledFont(FSlateFontInfo Font, int32 Size, FName Typeface);

	/** 카드 세 줄은 인덱스가 맞물려 있다 — 다시 칠할 때 함께 읽는다 */
	TArray<FName> CardAliases;

	UPROPERTY()
	TArray<TObjectPtr<UButton>> CardButtons;

	UPROPERTY()
	TArray<TObjectPtr<UTextBlock>> CardNames;

	UPROPERTY()
	TArray<TObjectPtr<UTextBlock>> CardDetails;

	UPROPERTY()
	TArray<TObjectPtr<UCL9ShopCardRelay>> Relays;

	FTimerHandle WireTimer;
};
