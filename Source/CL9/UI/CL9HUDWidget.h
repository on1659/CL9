// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/CL9Types.h"
#include "CL9HUDWidget.generated.h"

class ACL9GameState;
class ACL9PlayerState;
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

	/** Display name from the item table row, alias as fallback */
	FText ItemLabel(const FCL9CarriedItem& Item) const;

	ACL9PlayerState* GetOwnPlayerState() const;
	ACL9GameState* GetCL9GameState() const;

	FTimerHandle WireTimer;

	/** Redraws the clock text only — the value itself derives from replicated stamps */
	FTimerHandle ClockTimer;
};
