// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CL9TruckPad.generated.h"

class ACL9Character;
class UBoxComponent;
class UStaticMeshComponent;

/**
 * The truck pad — the one heal channel (docs/plan/data/rules.md 죽음과 회복,
 * docs/plan/loop.md 귀환). A living, wounded character standing inside HealZone
 * for TruckPadHealTime seconds gets HP back to the roll; stepping off resets
 * the count. The dead are not healed here (부활은 없다) — the next lobby stands
 * them up on this pad (ACL9GameMode::ReviveDead). Host-only ruling on a static
 * fixture: nothing replicates, the HP arrives through the PlayerState. Mesh and
 * zone size live in the BP child.
 */
UCLASS(abstract)
class ACL9TruckPad : public AActor
{
	GENERATED_BODY()

public:
	ACL9TruckPad();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

private:
	/** Pad body — mesh assigned in the BP child */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** Standing inside counts — overlaps pawns only */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> HealZone;

	/** Host only: seconds each wounded character has stood here without leaving */
	TMap<TWeakObjectPtr<ACL9Character>, float> TimeOnPad;
};
