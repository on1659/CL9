// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CL9Character.generated.h"

class ACL9Cart;
class ACL9Item;
class ACL9PlayerState;
class UCameraComponent;
class UCL9CharacterDef;
class UCL9CharacterMovement;
class UCL9RulesDef;
class UInputAction;
class UMaterialInterface;
struct FInputActionValue;

/**
 * First person co-op character: capsule, movement, camera and input live here.
 * Every number and mesh comes from the character table row (UCL9CharacterDef)
 * applied via SetCharacterAlias — this class holds zero asset references, so
 * bought assets swap without touching C++ (docs/plan/build-order.md 원칙 2·3).
 *
 * Carry inputs ride Server RPCs on this (owned) actor and the host calls plain
 * functions on the unowned item actors (docs/systems/input.md 상호작용 틀).
 */
UCLASS(abstract)
class ACL9Character : public ACharacter
{
	GENERATED_BODY()

public:
	ACL9Character(const FObjectInitializer& ObjectInitializer);

	/**
	 * Server only: picks the character table row and applies it locally.
	 * Clients follow through the replicated alias (docs/systems/character.md
	 * "캐릭터 Def 적용 경로").
	 */
	void SetCharacterAlias(FName NewAlias);

	/** The table row currently applied — null until the alias arrives */
	const UCL9CharacterDef* GetCharacterDef() const
	{
		return CharacterDef;
	}

	/** The item actor hanging on this character's hand, or null */
	ACL9Item* GetHeldItem() const
	{
		return HeldItem;
	}

	/** The cart this character drags, or null — the predicted movement reads it for CartMult */
	ACL9Cart* GetPullingCart() const
	{
		return PullingCart;
	}

	/** Host only, called by the cart on hitch/release */
	void SetPullingCart(ACL9Cart* Cart);

	/** Host only: destroys the held item and clears the mirrors (jar returns eat it) */
	void HostConsumeHeldItem();

	/** Host only: detaches the held item from the mirrors and hands the actor over (cart loading) */
	ACL9Item* HostTakeHeldItem();

	/** Returns the movement component as the CL9 predicted-movement type */
	UCL9CharacterMovement* GetCL9CharacterMovement() const;

	/** Returns the first person mesh (visible only to the owning player) */
	USkeletalMeshComponent* GetFirstPersonMesh() const
	{
		return FirstPersonMesh;
	}

	/** Returns the first person camera */
	UCameraComponent* GetFirstPersonCameraComponent() const
	{
		return FirstPersonCameraComponent;
	}

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Handles IA_Move (docs/systems/input.md) */
	void MoveInput(const FInputActionValue& Value);

	/** Handles IA_Look */
	void LookInput(const FInputActionValue& Value);

	/** Handles IA_Run press — only sets the movement flag, never a speed */
	void RunInputStart();

	/** Handles IA_Run release */
	void RunInputEnd();

	/** Handles IA_PickPlace (E) — the host resolves pick > interact > place */
	void PickPlaceInput();

	/** Handles IA_Throw press — the host clocks the charge (docs/systems/input.md 던지기 차징) */
	void ThrowStartInput();

	/** Handles IA_Throw release */
	void ThrowEndInput();

	/** Handles IA_Swap (R) — hand↔pocket under the SwapTime lock */
	void SwapInput();

	/** One press of E — priority: pickable item > interactable > place (docs/systems/input.md 감지) */
	UFUNCTION(Server, Reliable)
	void Server_PickPlace();

	UFUNCTION(Server, Reliable)
	void Server_ThrowChargeStart();

	UFUNCTION(Server, Reliable)
	void Server_ThrowRelease();

	UFUNCTION(Server, Reliable)
	void Server_Swap();

	UFUNCTION()
	void OnRep_CharacterAlias();

	/** Reads the row from the local registry and applies ruling values plus presentation */
	void ApplyCharacterDef();

	/** Move input action — assigned in the BP child, never loaded from C++ */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Look input action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> LookAction;

	/** Run input action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> RunAction;

	/** Jump input action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> JumpAction;

	/** Pick/place/interact action (E) */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> PickPlaceAction;

	/** Throw charge action (mouse left) */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> ThrowAction;

	/** Hand↔pocket swap action (R) */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> SwapAction;

	/** Overlay drawn on whatever the aim would interact with — assigned in the BP child */
	UPROPERTY(EditAnywhere, Category="Interaction")
	TObjectPtr<UMaterialInterface> HighlightOverlayMaterial;

private:
	/**
	 * The one target the interaction key would act on: candidates inside
	 * PickRadius and roughly ahead, pickable items before interactables, ties
	 * to the aim (docs/systems/input.md 감지). Runs locally every frame for
	 * the highlight and again on the host for the ruling.
	 */
	AActor* ResolveFocus() const;

	/** Presentation only: toggles the overlay material on the target's mesh */
	void SetFocusHighlight(AActor* Target, bool bOn) const;

	/** What the local player currently aims at (highlight bookkeeping) */
	TWeakObjectPtr<AActor> FocusedActor;

	/** Host only: attaches the item to this character's hand spot and mirrors it */
	void HostPickUp(ACL9Item* Item, const UCL9RulesDef* Rules);

	/** Host only: sets the held item down in front */
	void HostPlaceHeld(const UCL9RulesDef* Rules);

	const UCL9RulesDef* GetRules() const;
	ACL9PlayerState* GetCL9PlayerState() const;
	float GetServerTime() const;

	/** Which character row this pawn plays — set by the server at spawn, applied on every machine */
	UPROPERTY(ReplicatedUsing=OnRep_CharacterAlias)
	FName CharacterAlias;

	/** Cached row the alias resolved to (registry keeps it alive; cached for fast reads) */
	UPROPERTY()
	TObjectPtr<const UCL9CharacterDef> CharacterDef;

	/** The world-truth carry link — PlayerState.HandItem mirrors it for UI (docs/systems/inventory.md) */
	UPROPERTY(Replicated)
	TObjectPtr<ACL9Item> HeldItem;

	UPROPERTY(Replicated)
	TObjectPtr<ACL9Cart> PullingCart;

	/** Host clock when the current throw charge began; negative when idle */
	float ThrowChargeStartTime = -1.f;

	/** First person camera, attached to the capsule at eye height */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	/**
	 * First person view mesh; carried items get attached to its hand sockets.
	 * Attached under the body mesh, template-style: the owned FP rig
	 * (ABP_FP_Copy + CtrlRig_FPWarp) animates a full body mesh in body space
	 * and warps the arms toward the camera — an arms-only viewmodel asset can
	 * take over later through the table columns alone.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;
};
