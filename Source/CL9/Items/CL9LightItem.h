// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Items/CL9Item.h"
#include "CL9LightItem.generated.h"

class ULightComponent;

/**
 * 빛을 내는 아이템의 바탕 (docs/systems/light.md). 행의 광원 수치(LightRadius·
 * LightIntensity)로 라이트 컴포넌트를 만들고, 켜짐 여부(bLightOn)를 호스트가 정해
 * 복제한다 — 각 머신은 OnRep에서 표시만 갱신한다.
 *
 * 기본 동작은 "소지 중이면 켜진다"(발광) — 점광원이며 정전을 무시한다. 손전등처럼
 * 토글·정전 규칙이 있는 것은 파생 클래스(ACL9FlashItem)가 맡는다.
 */
UCLASS(abstract)
class ACL9LightItem : public ACL9Item
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 이 아이템의 조명 스위치가 켜져 있는 동안 true */
	bool IsLightOn() const
	{
		return bLightOn;
	}

protected:
	virtual void LoadData() override;
	virtual void OnAcquired() override;
	virtual void OnReleased() override;

	/** 행으로부터 라이트 컴포넌트를 만든다 — 파생 클래스가 종류(점광원/스포트)를 정한다 */
	virtual ULightComponent* MakeLightComponent();

	/** 이 머신에서 실제로 빛을 그릴지 — 파생 클래스가 정전 같은 조건을 덧붙인다 */
	virtual bool ShouldLightBeVisible() const
	{
		return bLightOn;
	}

	/** 호스트 전용: 스위치를 바꾸고 이 머신의 표시를 바로 갱신한다 */
	void SetLightOn(bool bOn);

	/** ShouldLightBeVisible()를 라이트 컴포넌트에 반영한다 */
	void UpdateLightVisibility();

	UFUNCTION()
	void OnRep_LightOn();

private:
	/** 조명 스위치 상태 — 호스트가 기록하고 복제한다 */
	UPROPERTY(ReplicatedUsing=OnRep_LightOn)
	bool bLightOn = false;

	/** 행이 로드될 때 한 번 만들어지는 라이트 — SubAlias 재로드에도 같은 컴포넌트를 쓴다 */
	UPROPERTY()
	TObjectPtr<ULightComponent> LightComponent;
};
