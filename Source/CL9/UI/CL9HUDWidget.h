// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/CL9Types.h"
#include "CL9HUDWidget.generated.h"

class ACL9Character;
class ACL9GameState;
class ACL9Item;
class ACL9PlayerState;
class UBorder;
class UProgressBar;
class UTextBlock;

/**
 * The always-on HUD layer content (docs/systems/ui.md 게임 중 HUD): HP, carry
 * slots, the night clock, the interaction prompt, the throw charge and the
 * world-anchored label over the outlined item. Values
 * arrive through PlayerState/GameState/Character change delegates — never
 * per-tick polling of replicated state (docs/systems/ui_system.md 데이터 연결).
 * Placed once in WBP_Main's designer.
 */
UCLASS(Abstract)
class UCL9HUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/**
	 * Stamina has no change event (simulated in the CMC) and the throw charge is a
	 * time function while the button is held — both local, both read here. The item
	 * label's screen position also tracks the actor here (presentation, not state).
	 */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** One ♥ per HP point (초안 — ui.md의 "HP 표시 방식 미정" 자리), 10 초과는 숫자 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HpText;

	/** What the hand holds — mirror of PlayerState.HandItem */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HandText;

	/** Pocketed items — count and names */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> PocketText;

	/** Day number, phase and the running night clock */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ClockText;

	/** Normalized stamina (2026-09-05 결정 — the bar joins the shake, it does not replace it) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;

	/** Bottom centre: what E would do right now — the character composes it (docs/systems/input.md 상호작용 공통 틀) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> PromptText;

	/** Throw charge 0~1, local prediction of the host clock (input.md 던지기 차징); collapsed while idle */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> ChargeBar;

	/** World-anchored label over the outlined item — name · value · carry facts (2026-09-05 사용자 요청). Projected each frame */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> ItemLabelBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ItemNameText;

	/** ValueMin~ValueMax 이더 — the sale rolls inside this range (docs/systems/economy.md 가격식) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ItemPriceText;

	/** Weight · carry slot · fragility, joined with ' · ' */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ItemInfoText;

	/** Gap between the item's top (Radius) and the label's bottom edge, in cm — edit in WBP_HUD */
	UPROPERTY(EditAnywhere, Category="HUD")
	float ItemLabelLift = 15.f;

	/** Fill while stamina is spendable — edit in WBP_HUD */
	UPROPERTY(EditAnywhere, Category="HUD")
	FLinearColor StaminaColor = FLinearColor(0.85f, 0.85f, 0.75f, 1.f);

	/** Fill while the exhaustion latch blocks running (docs/systems/character.md 세 조항) */
	UPROPERTY(EditAnywhere, Category="HUD")
	FLinearColor StaminaExhaustedColor = FLinearColor(0.75f, 0.25f, 0.2f, 1.f);

private:
	UFUNCTION()
	void HandleHpChanged();

	UFUNCTION()
	void HandleCarryChanged();

	UFUNCTION()
	void HandlePhaseChanged(ECL9NightPhase NewPhase);

	UFUNCTION()
	void HandleFocusPromptChanged();

	/** PlayerState/GameState replicate in after construct — a short timer retries until both exist */
	void TryWire();

	/** The pawn comes and goes (spawn · 관전 · 로비 복귀) — rebinds the character delegate when it changes */
	void WireCharacter();

	void RefreshHp();
	void RefreshCarry();
	void RefreshClock();
	void RefreshStamina();
	void RefreshPrompt();
	void RefreshCharge();

	/** Follows the outline target: content on target change, position every frame */
	void RefreshItemLabel();
	void FillItemLabel(const ACL9Item& Item);

	/** Display name from the item table row, alias as fallback */
	FText ItemLabel(const FCL9CarriedItem& Item) const;

	ACL9PlayerState* GetOwnPlayerState() const;
	ACL9GameState* GetCL9GameState() const;
	ACL9Character* GetOwnCharacter() const;

	/** The character whose OnFocusPromptChanged this widget is bound to */
	TWeakObjectPtr<ACL9Character> WiredCharacter;

	/** The item the label currently describes — text refreshes only when this changes */
	TWeakObjectPtr<ACL9Item> LabeledItem;

	FTimerHandle WireTimer;

	/** Redraws the clock text only — the value itself derives from replicated stamps */
	FTimerHandle ClockTimer;
};
