// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CL9PlayerController.generated.h"

class UInputAction;
class UInputMappingContext;

/**
 * First person player controller. Registers the input mapping contexts for the
 * local player, overrides the camera manager (pitch limits) and bootstraps the
 * local UI (docs/systems/ui_system.md).
 */
UCLASS(abstract)
class ACL9PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACL9PlayerController();

	/**
	 * Forwards a cl9.* cheat line typed on a client console to the host
	 * (docs/systems/cheats.md). The declaration stays in every build so the
	 * RPC table matches — only the body empties out in shipping.
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_CL9Cheat(const FString& CommandLine);

	/** UI manager toggles the gameplay contexts off while a screen is open (docs/systems/input.md) */
	void SetGameplayInputEnabled(bool bEnabled);

	/** True while the view rides a living teammate after death (docs/systems/ui.md 관전) */
	UFUNCTION(BlueprintPure, Category="CL9|Spectate")
	bool IsSpectating() const
	{
		return bSpectating;
	}

	/** Next living teammate — IA_SpectateNext and the spectate screen's button both land here (local → host ruling) */
	UFUNCTION(BlueprintCallable, Category="CL9|Spectate")
	void SpectateNext();

	/** Host only: flips the flag and fires the local hook (the owner's OnRep covers a client) */
	void HostSetSpectating(bool bNewSpectating);

	/**
	 * Confirms the lobby's order pick — indices into GameState.OfferedOrders,
	 * exactly OrdersRequired of them. Only the listen host's own controller may
	 * confirm (docs/systems/order.md 호스트가 확정); the order screen's button
	 * and cl9.pick both land here.
	 */
	UFUNCTION(BlueprintCallable, Category="CL9|Orders")
	void AcceptOrders(const TArray<int32>& OfferedIndices);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** M (IA_Pause) — opens the pause screen, or closes whatever screen is open; a spectator gets the spectate screen back (docs/systems/input.md) */
	void PauseInput();

	UFUNCTION(Server, Reliable)
	void Server_SpectateNext();

	UFUNCTION(Server, Reliable)
	void Server_AcceptOrders(const TArray<int32>& OfferedIndices);

	/** Local: the spectate screen and IMC_Spectate follow the flag together, so the two never diverge */
	UFUNCTION()
	void OnRep_Spectating();

	/** Mapping contexts added for the local player — assigned in the BP child (docs/systems/input.md) */
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;

	/** Pause/close action — assigned in the BP child, lives on the controller so it works without a pawn */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> PauseAction;

	/** IMC_Spectate — added while spectating, above the gameplay contexts; not part of DefaultMappingContexts */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> SpectateMappingContext;

	/** Spectate target switch action — assigned in the BP child */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> SpectateNextAction;

	/** Host-written after SpectateDelay; cleared by the lobby revive. Owner-only by PlayerController replication */
	UPROPERTY(ReplicatedUsing=OnRep_Spectating)
	bool bSpectating = false;
};
