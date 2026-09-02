// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CL9Damageable.generated.h"

UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UCL9Damageable : public UInterface
{
	GENERATED_BODY()
};

/**
 * The one damage inlet (docs/systems/character.md — TakeDamage 대체, 2026-09-02
 * 확정). Characters and monsters implement it separately because their bases
 * differ. Only host ruling code calls it (monster catch, pan shove) — never a
 * client, never presentation.
 */
class ICL9Damageable
{
	GENERATED_BODY()

public:
	/** Host only. Amount is plain HP; Source is the attacker */
	virtual void OnDamage(int32 Amount, AActor* Source) = 0;
};
