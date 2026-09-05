// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/CL9Types.h"
#include "CL9HUDWidget.generated.h"

class ACL9GameState;
class ACL9PlayerState;
class UProgressBar;
class UTextBlock;

/**
 * The always-on HUD layer content (docs/systems/ui.md 게임 중 HUD): HP, carry
 * slots and the night clock. Values arrive through PlayerState/GameState
 * change delegates — never per-tick polling of replicated state
 * (docs/systems/ui_system.md 데이터 연결). Placed once in WBP_Main's designer.
 */
UCLASS(Abstract)
class UCL9HUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Stamina has no change event — it is simulated in the CMC, so the bar polls */
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

	/** PlayerState/GameState replicate in after construct — a short timer retries until both exist */
	void TryWire();

	void RefreshHp();
	void RefreshCarry();
	void RefreshClock();
	void RefreshStamina();

	/** Display name from the item table row, alias as fallback */
	FText ItemLabel(const FCL9CarriedItem& Item) const;

	ACL9PlayerState* GetOwnPlayerState() const;
	ACL9GameState* GetCL9GameState() const;

	FTimerHandle WireTimer;

	/** Redraws the clock text only — the value itself derives from replicated stamps */
	FTimerHandle ClockTimer;
};
