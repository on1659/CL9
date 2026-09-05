// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/CL9Damageable.h"
#include "GameFramework/Character.h"
#include "CL9Character.generated.h"

class ACL9Cart;
class ACL9Item;
class ACL9PlayerState;
class UCameraComponent;
class UCL9CharacterDef;
class UCL9CharacterMovement;
class UCL9RulesDef;
class UCameraShakeBase;
class UInputAction;
class UMaterialInterface;
struct FInputActionValue;

/** Fires on the owning client when the "[E] …" prompt text changes — the HUD subscribes instead of polling (docs/systems/ui_system.md) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCL9FocusPromptChanged);

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
class ACL9Character : public ACharacter, public ICL9Damageable
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

	/** The item worn on the head (headlamp), or null — hands stay free of it */
	ACL9Item* GetWornItem() const
	{
		return WornItem;
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

	/** Hand, pocket or cart — anything at all; empty hands release chases at DeaggroRadius */
	bool IsCarryingAnything() const;

	/**
	 * Host only (docs/systems/character.md 피격·넘어짐·죽음): HP down, knockdown,
	 * spill of hand + pocket. HP 0 stays down for the night and hands the death
	 * to the game mode (wipe check, spectate after SpectateDelay).
	 */
	virtual void OnDamage(int32 Amount, AActor* Source) override;

	/** Host only: HP back to the rolled maximum — the truck pad's heal (docs/plan/data/rules.md 죽음과 회복) */
	void HostHealFull();

	/** Host only: the next lobby stands the dead back up with full HP — 부활은 없다, 밤이 끝나야 돌아온다 */
	void HostRevive();

	/** True while the catch knockdown blocks movement — the CMC reads this for speed 0 */
	UFUNCTION(BlueprintPure, Category="CL9")
	bool IsKnockedDown() const;

	/** HP 0 on the PlayerState — monsters skip dead targets */
	UFUNCTION(BlueprintPure, Category="CL9")
	bool IsDead() const;

	/**
	 * What one press of E would do right now, as the HUD prompt ("[E] 줍기" …) —
	 * mirrors Server_PickPlace's branch order on the local focus; empty when
	 * nothing. Presentation only (docs/systems/input.md 상호작용 프롬프트).
	 */
	UFUNCTION(BlueprintPure, Category="Interaction")
	FText GetFocusPrompt() const;

	/** Owning client: the prompt text changed (target or its state) — HUD refresh */
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FCL9FocusPromptChanged OnFocusPromptChanged;

	/** What the local aim is on right now (the outlined actor) — null when nothing; the HUD's world label follows it */
	UFUNCTION(BlueprintPure, Category="Interaction")
	AActor* GetFocusedActor() const
	{
		return FocusedActor.Get();
	}

	/**
	 * Owning client's local charge read for the HUD gauge: -1 when not charging,
	 * else 0..1 over ThrowChargeTime (docs/systems/input.md 던지기 차징 — 로컬 예측,
	 * the host clocks the ruling separately).
	 */
	UFUNCTION(BlueprintPure, Category="Throw")
	float GetThrowChargeAlpha() const;

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

	/** Handles IA_Flashlight (F) — toggles the held flashlight (docs/systems/light.md) */
	void FlashlightInput();

	/** One press of E — priority: pickable item > interactable > place (docs/systems/input.md 감지) */
	UFUNCTION(Server, Reliable)
	void Server_PickPlace();

	UFUNCTION(Server, Reliable)
	void Server_ThrowChargeStart();

	UFUNCTION(Server, Reliable)
	void Server_ThrowRelease();

	UFUNCTION(Server, Reliable)
	void Server_Swap();

	/** Empty-hand LMB — shoves the monster ahead (frying pan stand-in, docs/systems/input.md) */
	UFUNCTION(Server, Reliable)
	void Server_Shove();

	/** Toggles the held flashlight on the host (item state is replicated — light.md) */
	UFUNCTION(Server, Reliable)
	void Server_Flashlight();

	UFUNCTION()
	void OnRep_CharacterAlias();

	/** Reads the row from the local registry and applies ruling values plus presentation */
	void ApplyCharacterDef();

	/** Owning-client only: stamina below the rule threshold shakes the camera, harder as it empties (docs/systems/character.md) */
	void UpdateStaminaShake();

	/** Active tired-shake instance on the local player — null when rested */
	UPROPERTY(Transient)
	TObjectPtr<UCameraShakeBase> StaminaShakeInstance;

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

	/** Flashlight toggle action (F) */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> FlashlightAction;

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

	/** Last prompt handed to the HUD — OnFocusPromptChanged fires only when the text differs */
	FText LastFocusPrompt;

	/** Host only: spills hand and pocket onto the floor and lets go of the cart (잡힘 짐 흘림) */
	void HostDropEverything();

	/** Host only: attaches the item to this character's hand spot and mirrors it */
	void HostPickUp(ACL9Item* Item, const UCL9RulesDef* Rules);

	/** Host only: puts a Head-slot item on (headlamp) and mirrors it — one press of E, hands untouched */
	void HostWear(ACL9Item* Item);

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

	/** The head-worn item actor (Head carry slot) — PlayerState.HeadItem mirrors it */
	UPROPERTY(Replicated)
	TObjectPtr<ACL9Item> WornItem;

	/** Host clock when the current throw charge began; negative when idle */
	float ThrowChargeStartTime = -1.f;

	/** Owning client's local clock for the same charge (gauge only, never a ruling); negative when idle */
	float LocalThrowChargeStartTime = -1.f;

	/** Host clock until which another shove is refused (swing recovery) */
	float ShoveReadyTime = 0.f;

	/**
	 * Server time until which the knockdown pins the character (0 = up). A
	 * replicated read for the CMC like CartMult — both sides derive speed 0 from
	 * the same value, so the correction stays small (docs/systems/character.md
	 * 넘어짐 이동 차단 — CMC 최대 속도 0 방식 선택, 2026-09-03).
	 */
	UPROPERTY(Replicated)
	float KnockdownEndServerTime = 0.f;

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
