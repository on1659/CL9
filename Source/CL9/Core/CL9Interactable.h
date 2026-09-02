// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CL9Interactable.generated.h"

class ACL9Character;

UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UCL9Interactable : public UInterface
{
	GENERATED_BODY()
};

/**
 * The one interaction frame — jar returns, truck panels and counters all
 * implement this instead of growing new input paths (docs/systems/input.md
 * 상호작용 공통 틀). Plain functions, not RPCs: these actors are unowned, so
 * requests arrive through the character's Server RPC and the host calls
 * Interact directly after re-checking.
 */
class ICL9Interactable
{
	GENERATED_BODY()

public:
	/** HUD prompt, e.g. "[E] 반납" — presentation only */
	virtual FText GetPrompt(const ACL9Character* Who) const = 0;

	/** Lock/distance/state check — the host re-runs it before Interact */
	virtual bool CanInteract(const ACL9Character* Who) const = 0;

	/** Host only */
	virtual void Interact(ACL9Character* Who) = 0;
};
