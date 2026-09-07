// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "Data/CL9Types.h"
#include "CL9EquipmentDef.generated.h"

/** 산 장비가 어디에 놓이는가 (docs/plan/data/equipment.md) */
UENUM(BlueprintType)
enum class ECL9EquipSlot : uint8
{
	/** 주방에 고정된다 — 들고 다니지 않는다 */
	Kitchen,
	/** 들고 다니는 물건 — 계란과 같은 운반 규칙을 탄다 */
	Carryable
};

/**
 * 장비 행 하나 (docs/plan/data/equipment.md). 번 이더로 사는 물건이며, **파는 것은
 * 실물뿐이다** — 스탯만 올려주는 장비는 만들지 않는다. 구매는 "인벤토리에 추가"가
 * 아니라 월드에 실물을 스폰하는 것이고(docs/systems/shop.md 구매 흐름), 그 실물이
 * 어떤 아이템으로 취급되는지는 `ItemDef`가 정한다 — 그래서 장비도 아이템 운반 규칙
 * 하나를 그대로 공유한다.
 *
 * 장비는 원정에 들고 갈 수 있고 두고 오면 잃는다 — 다시 사는 값이 그 대가다.
 */
UCLASS()
class UCL9EquipmentDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	/** 해당 일자에 이 장비를 살 수 있으면 true */
	bool IsUnlockedOn(int32 Day) const
	{
		return Day >= UnlockDay;
	}

	/** 이더 가격 — 잔액이 모자라면 구매가 거절된다 (채무 불가) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop")
	int32 Price = 0;

	/** 몇 일차부터 상점에 뜨는가 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop")
	int32 UnlockDay = 1;

	/** 최대 소지 수 — 0이면 무제한 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop")
	int32 MaxOwned = 0;

	/**
	 * 월드에 놓을 수 있는 물건인가. **true인 것만 판다** — 착용형은 헤드램프 하나만
	 * 예외이고(실물이되 주우면 머리에 쓴다), 스탯형 장비는 현재 범위에 없다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop")
	bool bPhysicalProp = true;

	/** 산 물건이 주방에 고정되는지, 들고 다니는지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop")
	ECL9EquipSlot Slot = ECL9EquipSlot::Carryable;

	/** 몬스터를 상대하는 도구로 쓸 수 있나 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop")
	bool bUsableAsTool = false;

	/**
	 * 이 장비의 실물이 어떤 아이템 행으로 스폰되는가 — 반드시 아이템 표에 있어야 한다
	 * (로드 검사가 확인한다). 장비를 아이템 시스템에 연결하는 유일한 끈이다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop")
	FCL9ItemRef ItemDef;
};
