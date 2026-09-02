// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CL9GameInstance.generated.h"

class UCL9DefList;
class UCL9DefRegistry;

/**
 * Owns the local def registry (docs/data/schema.md 조회). The def list asset
 * is assigned in the BP child so C++ holds zero asset paths.
 */
UCLASS(abstract)
class UCL9GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UCL9DefRegistry* GetDefRegistry() const
	{
		return DefRegistry;
	}

protected:
	/** Every table row loads from this list — assigned in BP_CL9GameInstance */
	UPROPERTY(EditAnywhere, Category="Data")
	TSoftObjectPtr<UCL9DefList> DefListAsset;

private:
	UPROPERTY()
	TObjectPtr<UCL9DefRegistry> DefRegistry;
};
