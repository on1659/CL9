// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Items/CL9LightItem.h"
#include "CL9FlashItem.generated.h"

/**
 * 손전등·헤드램프 — 전방 스포트라이트를 F(IA_Flashlight)로 켜고 끈다 (docs/systems/light.md).
 * 발광과 달리 들면 꺼진 채로 시작하고, 정전 중에는 켤 수 없으며 켜져 있어도 정전이 시작되는
 * 틱에 꺼진다. 착용한 것(헤드램프)은 예외다 — 정전은 부지의 설비를 죽이는 것이지 가져온
 * 물건을 죽이는 게 아니다 (2026-09-05).
 */
UCLASS(abstract)
class ACL9FlashItem : public ACL9LightItem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaSeconds) override;

	/** 호스트 전용: 소지자가 F를 눌렀다 — 정전 중이면 켜기를 거부한다 */
	void ToggleLight();

protected:
	virtual ULightComponent* MakeLightComponent() override;
	virtual bool ShouldLightBeVisible() const override;
	virtual void OnAcquired() override;

private:
	/** 정전이 이 손전등을 끄는 상황인지 — 착용 중이면 정전을 무시한다 */
	bool IsKilledByBlackout() const;
};
