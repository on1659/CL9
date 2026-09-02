// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "CL9Gimmicks.generated.h"

/**
 * Sample presentation gimmick: spins the row's mesh in place. Exists to prove
 * the gimmick layer (build-order 2e — "표에서 기믹 추가하면 그 아이템만
 * 달라진다"); rotation is render-only and never enters a ruling.
 */
UCLASS()
class UCL9Gimmick_Spin : public UCL9Gimmick
{
	GENERATED_BODY()

public:
	virtual void OnActorLoaded(AActor* Actor) override;

	/** Yaw speed in degrees per second */
	UPROPERTY(EditAnywhere, Category="Gimmick")
	float DegreesPerSecond = 90.f;
};
