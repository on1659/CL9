// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CL9FlapIn.generated.h"

class UBoxComponent;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * One-way flap in the barn wall (docs/plan/data/segkind.md flapIn): a wall from
 * outside, a way out from inside. A character touching the inside trigger has
 * its capsule ignore this actor until it leaves the trigger on either side, so
 * it pushes through outward; anyone arriving from outside meets a wall. Animals
 * and thrown things are blocked both ways. Spawned by ACL9SiteMap, never placed.
 */
UCLASS(NotBlueprintable, NotPlaceable)
class ACL9FlapIn : public AActor
{
	GENERATED_BODY()

public:
	ACL9FlapIn();

	/** Sizes the body and trigger; InsideDirection points from the wall into the barn */
	void Setup(const FVector& Extent, const FVector& InsideDirection, UStaticMesh* Mesh);

protected:
	UFUNCTION()
	void OnTriggerBegin(UPrimitiveComponent* Overlapped, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Sweep);

	UFUNCTION()
	void OnTriggerEnd(UPrimitiveComponent* Overlapped, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex);

	/** The wall — blocks everything; characters bypass it via move-ignore while inside the trigger */
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UBoxComponent> Body;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Visual;

	/** Straddles the wall, deeper on the inside — pawn overlaps only */
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UBoxComponent> Trigger;

	FVector InsideDir = -FVector::ForwardVector;
};
