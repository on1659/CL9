// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "Data/CL9Types.h"
#include "CL9OrderDef.generated.h"

/**
 * 주문 행 하나 (docs/plan/data/order.md) — 오늘 밤 무엇을 요리해서 팔아야 하는지.
 * 주문은 완성품(달걀+조리됨)만 요구하며 원재료는 절대 요구하지 않는다, 그래서
 * 모든 주문은 요리 과정을 반드시 거친다. 로비는 일자 범위와 가중치로 이 중
 * N개를 뽑고, 호스트가 M개를 확정한다 (docs/systems/order.md 주문 흐름).
 */
UCLASS()
class UCL9OrderDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	/** 해당 일자에 이 행을 제공할 수 있으면 true (MinDay..MaxDay, MaxDay 0이면 무제한) */
	bool IsOfferedOn(int32 Day) const
	{
		return Day >= MinDay && (MaxDay <= 0 || Day <= MaxDay);
	}

	/** 필요한 완성품 — alias+sub는 반드시 아이템 행이어야 한다 (로드 체크가 검증) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Order")
	TArray<FCL9ItemCount> Ingredients;

	/** 납품 시 지급되는 에테르 보상 굴림값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reward")
	int32 RewardMin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Reward")
	int32 RewardMax = 0;

	/** 납품까지 남은 일수 — 1이면 오늘 밤을 뜻한다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Deadline")
	int32 DeadlineDays = 1;

	/** 마감이 지났을 때 런의 에테르에서 차감되는 보상 비율 (초안 0.3) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Deadline")
	float FailPenaltyRate = 0.3f;

	/** 이 주문이 등장할 수 있는 첫 날 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Offer")
	int32 MinDay = 1;

	/** 등장하는 마지막 날 — 0이면 계속 유지된다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Offer")
	int32 MaxDay = 0;

	/** 그 날의 후보들 사이에서의 추첨 가중치 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Offer")
	float Weight = 1.f;
};
