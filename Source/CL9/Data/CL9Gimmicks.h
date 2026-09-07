// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "CL9Gimmicks.generated.h"

/**
 * 연출용 예시 기믹 — 행의 메시를 제자리에서 회전시킨다. 기믹 레이어를
 * 증명하기 위해 존재하며(build-order 2e — "표에서 기믹 추가하면 그 아이템만
 * 달라진다"), 회전은 렌더링 전용이라 판정에는 절대 관여하지 않는다.
 */
UCLASS()
class UCL9Gimmick_Spin : public UCL9Gimmick
{
	GENERATED_BODY()

public:
	virtual void OnActorLoaded(AActor* Actor) override;

	/** 초당 요(yaw) 회전 속도 (도) */
	UPROPERTY(EditAnywhere, Category="Gimmick")
	float DegreesPerSecond = 90.f;
};
