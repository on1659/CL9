// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/CL9Interactable.h"
#include "CL9Jar.generated.h"

class UStaticMeshComponent;

/**
 * Return jar (docs/systems/raiding.md 항아리 반납). Takes the hand item and
 * records it in the game state's returned list. Pulling the blackout clock
 * forward by DarkAccel joins in when the night clock exists (build-order ④).
 * A static fixture — no state, no replication; its mesh lives in the BP child.
 */
UCLASS(abstract)
class ACL9Jar : public AActor, public ICL9Interactable
{
	GENERATED_BODY()

public:
	ACL9Jar();

	virtual FText GetPrompt(const ACL9Character* Who) const override;
	virtual bool CanInteract(const ACL9Character* Who) const override;
	virtual void Interact(ACL9Character* Who) override;

private:
	/** Jar body — blocks movement, mesh assigned in the BP child */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};
