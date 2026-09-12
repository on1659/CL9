// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Items/CL9LightItem.h"
#include "CL9LampItem.generated.h"

/**
 * 스탠드 램프 — 놓으면 주변을 밝히는 운반 장비 (docs/plan/data/equipment.md `standing_lamp`).
 * 발광(ACL9LightItem)과 달리 **내려놓아도 꺼지지 않는다** — 들고 다니는 동안도, 바닥에 세워
 * 둔 동안도 늘 켜져 있다 (2026-09-11 사용자: "내려두어도 불은 켜져 있어야지"). 정전은 부지의
 * 설비를 죽이는 것이지 가져온 물건을 죽이는 게 아니라, 정전 뒤에 남는 광원이 헤드램프와
 * 이것뿐이다 (docs/systems/light.md).
 */
UCLASS(abstract)
class ACL9LampItem : public ACL9LightItem
{
	GENERATED_BODY()

protected:
	virtual void LoadData() override;
	virtual void OnReleased() override;
};
