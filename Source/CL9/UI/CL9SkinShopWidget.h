// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/CL9Widget.h"
#include "CL9SkinShopWidget.generated.h"

class UCL9SkinShopWidget;
class UTextBlock;
class UVerticalBox;
class UWidget;
class UWidgetTree;

/** 스킨 버튼마다의 클릭 릴레이 — UButton::OnClicked 는 페이로드를 싣지 않는다 */
UCLASS()
class UCL9SkinRelay : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UCL9SkinShopWidget> Owner;

	FName SkinAlias;

	UFUNCTION()
	void HandleClicked();
};

/**
 * 옷장 — 스킨 상점과 마이페이지를 한 화면에(docs/goal/loop-7k-skins.md). 회사의 옷장(`ACL9SkinCloset`)이 연다(UI 행 `SkinShop`).
 * 이 머신의 지갑(`UCL9WalletSubsystem`)을 읽어 보유 이더와 스킨마다 한 줄(사기 · 입기 · 입는 중)을 보인다. 누르면 필요하면 사고 입는다 —
 * 입은 스킨은 호스트에 알려(`ACL9PlayerController::RequestSkin`) 모두에게 보인다. 트리는 타이틀처럼 C++ 이 스스로 짓는다.
 */
UCLASS()
class UCL9SkinShopWidget : public UCL9Widget
{
	GENERATED_BODY()

public:
	/** 릴레이가 부른다 — 그 스킨을 (필요하면 사서) 입는다 */
	void HandleSkinClicked(FName SkinAlias);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(EditAnywhere, Category="Skin")
	FLinearColor BackdropColor = FLinearColor(0.f, 0.f, 0.f, 0.75f);

	UPROPERTY(EditAnywhere, Category="Skin")
	FLinearColor ButtonColor = FLinearColor(0.12f, 0.12f, 0.14f, 1.f);

	UPROPERTY(EditAnywhere, Category="Skin")
	FLinearColor TextColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category="Skin")
	int32 TitleSize = 30;

	UPROPERTY(EditAnywhere, Category="Skin")
	int32 BodySize = 20;

	UPROPERTY(EditAnywhere, Category="Skin")
	FVector2D ButtonSize = FVector2D(460.f, 56.f);

private:
	void BuildTree();
	void Refresh();
	UTextBlock* MakeText(const FText& Content, int32 Size, const FLinearColor& Color);
	static bool MountRoot(UWidgetTree& Tree, UWidget& Root);

	UFUNCTION()
	void HandleWardrobeClose();

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> BalanceText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> HintLine;

	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> RowsBox;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UCL9SkinRelay>> Relays;

	FDelegateHandle WalletHandle;
};
