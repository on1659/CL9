// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CL9Character.generated.h"

class UCameraComponent;
class UInputAction;
struct FInputActionValue;

/**
 * First person co-op character: capsule, movement, camera and input live here.
 * Meshes, materials and animation blueprints live only in the BP child
 * (BP_CL9Character) — this class holds zero asset references, so bought assets
 * can be swapped without touching C++ (docs/plan/build-order.md principle 2).
 */
UCLASS(abstract)
class ACL9Character : public ACharacter
{
	GENERATED_BODY()

public:
	ACL9Character();

	/** Returns the first person arms mesh (visible only to the owning player) */
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
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Handles IA_Move (docs/systems/input.md) */
	void MoveInput(const FInputActionValue& Value);

	/** Handles IA_Look */
	void LookInput(const FInputActionValue& Value);

	/** Move input action — assigned in the BP child, never loaded from C++ */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Look input action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> LookAction;

	/** Jump input action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> JumpAction;

private:
	/** First person camera, attached to the capsule at eye height */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	/** First person arms mesh, attached to the camera; carried items get attached to its hand sockets */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;
};
