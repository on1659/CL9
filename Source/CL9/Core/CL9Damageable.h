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
 * 데미지가 들어오는 유일한 통로 (docs/systems/character.md — TakeDamage 대체, 2026-09-02
 * 확정). 캐릭터와 몬스터는 베이스가 다르기 때문에 각자 따로 구현한다. 호스트의
 * 판정 코드(몬스터 붙잡기, 프라이팬 밀치기)만 호출하며, 클라이언트나 표시용
 * 코드는 절대 호출하지 않는다.
 */
class ICL9Damageable
{
	GENERATED_BODY()

public:
	/** 호스트 전용. Amount는 그대로 HP 수치이고, Source는 공격자다 */
	virtual void OnDamage(int32 Amount, AActor* Source) = 0;
};
