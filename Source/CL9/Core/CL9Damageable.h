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
	/**
	 * 호스트 전용. Amount는 그대로 HP 수치이고, Source는 공격자다. Knockback 은 Source 반대쪽으로 밀리는 거리(cm) —
	 * 아이템 행 `HitKnockback` 이 몬스터에게 주는 값이고, 잡기·치트는 0 이다. 캐릭터는 받아도 쓰지 않는다
	 */
	virtual void OnDamage(int32 Amount, AActor* Source, float Knockback) = 0;
};
