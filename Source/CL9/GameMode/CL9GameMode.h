// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CL9GameMode.generated.h"

/**
 * Base game mode. Exists on the listen host only — authoritative game-flow
 * logic goes here; state that clients need goes to the game state instead
 * (conventions § 2). DefaultPawnClass is set in the BP child so C++ holds
 * no asset references.
 */
UCLASS(abstract)
class ACL9GameMode : public AGameModeBase
{
	GENERATED_BODY()
};
