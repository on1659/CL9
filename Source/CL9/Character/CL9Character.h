// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CL9Character.generated.h"

class UCameraComponent;
class UCL9CharacterDef;
class UCL9CharacterMovement;
class UInputAction;
struct FInputActionValue;

/**
 * First person co-op character: capsule, movement, camera and input live here.
 * Every number and mesh comes from the character table row (UCL9CharacterDef)
 * applied via SetCharacterAlias — this class holds zero asset references, so
 * bought assets swap without touching C++ (docs/plan/build-order.md 원칙 2·3).
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

private:
	/** Which character row this pawn plays — set by the server at spawn, applied on every machine */
	UPROPERTY(ReplicatedUsing=OnRep_CharacterAlias)
	FName CharacterAlias;

	/** Cached row the alias resolved to (registry keeps it alive; cached for fast reads) */
	UPROPERTY()
	TObjectPtr<const UCL9CharacterDef> CharacterDef;

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
