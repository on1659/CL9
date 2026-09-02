// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CL9DefBase.generated.h"

class UTexture2D;

/**
 * Presentation-only special behavior attached to a table row via the Instanced
 * Gimmicks column (docs/plan/build-order.md 원칙 4). Gimmicks that affect
 * rulings are expressed as table values instead, read by host ruling code.
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced)
class UCL9Gimmick : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Presentation hook, called on every machine after the actor loads its row
	 * (and again on state swaps — implementations must tolerate re-entry).
	 * Never touch ruling state here: hosts rule through table values only.
	 */
	virtual void OnActorLoaded(AActor* Actor)
	{
	}
};

/**
 * Parent of every game data table row (docs/data/schema.md). The lookup key is
 * Alias + SubAlias, never the file name. UPrimaryDataAsset because Instanced
 * gimmicks need a UObject owner, which DataTable row structs cannot provide.
 */
UCLASS(Abstract)
class UCL9DefBase : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * Appends one message per broken required field. Rows with errors stay out
	 * of the registry so they never mix into gameplay (schema.md EDefStatus).
	 * The required set differs per subclass — the base only demands Alias.
	 */
	virtual void Validate(TArray<FString>& OutErrors) const;

	/** Identifier and lookup key, e.g. egg / chicken / farmer_a */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	FName Alias;

	/** Optional sub-classification, e.g. cooked / burnt — None means the base state */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	FName SubAlias;

	/** Localized display name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	FText DisplayName;

	/** Optional localized description */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	FText Description;

	/** UI icon — soft so opening a table does not load every texture */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	TSoftObjectPtr<UTexture2D> Icon;

	/** World mesh for item-like rows; characters/monsters use their own skeletal mesh columns */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	TSoftObjectPtr<UStaticMesh> Mesh;

	/** Dedicated BP for this row. Empty means the shared per-kind BP, which is the default */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	TSoftClassPtr<AActor> ActorClass;

	/** Presentation-only special behaviors, added per row in the editor */
	UPROPERTY(EditAnywhere, Instanced, Category="Data")
	TArray<TObjectPtr<UCL9Gimmick>> Gimmicks;
};
