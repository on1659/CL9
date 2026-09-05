// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CL9GameInstance.generated.h"

class UCL9DefList;
class UCL9DefRegistry;
class UCL9ShakeTable;
class UCL9UITable;

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

	/**
	 * A registry outside any game instance — editor tools (cl9.seed without PIE)
	 * read the project's game instance defaults for the def list. Null, logged,
	 * when the project's game instance is not a CL9 one.
	 */
	static UCL9DefRegistry* CreateStandaloneRegistry(UObject* Outer);

	/** The UI table the UI manager reads (docs/systems/ui_system.md) — null logs once in Init */
	const UCL9UITable* GetUITable() const
	{
		return UITable;
	}

	/** The camera shake table presentation code reads (docs/systems/character.md 흔들림) */
	const UCL9ShakeTable* GetShakeTable() const
	{
		return ShakeTable;
	}

protected:
	/** Every table row loads from this list — assigned in BP_CL9GameInstance */
	UPROPERTY(EditAnywhere, Category="Data")
	TSoftObjectPtr<UCL9DefList> DefListAsset;

	/** UI rows for UCL9UIManager — assigned in BP_CL9GameInstance (경로 하드코딩 금지) */
	UPROPERTY(EditAnywhere, Category="UI")
	TSoftObjectPtr<UCL9UITable> UITableAsset;

	/** Camera shake rows — assigned in BP_CL9GameInstance */
	UPROPERTY(EditAnywhere, Category="UI")
	TSoftObjectPtr<UCL9ShakeTable> ShakeTableAsset;

private:
	UPROPERTY()
	TObjectPtr<UCL9DefRegistry> DefRegistry;

	UPROPERTY()
	TObjectPtr<const UCL9UITable> UITable;

	UPROPERTY()
	TObjectPtr<const UCL9ShakeTable> ShakeTable;
};
