// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Data/CL9Types.h"
#include "CL9SaveGame.generated.h"

/**
 * 판 하나의 저장본(docs/systems/save.md). **진행은 호스트의 것이다** — 클라이언트는
 * 세이브 파일을 갖지 않고 접속하면 호스트의 상태를 복제받을 뿐이다. 저장 단위는
 * "날"이며, 밤 중 저장은 없다: 새벽에 한 번 자동으로 쓴다.
 *
 * 에셋 참조는 전부 Alias다 — 경로나 포인터를 넣지 않는다. 해석에 실패한 Alias는
 * 로그를 남기고 그 항목만 버린다(docs/plan/data/schema.md 원칙).
 */
UCLASS()
class UCL9SaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** 슬롯 파일명 — 지금은 1번만 쓰지만 구조는 처음부터 슬롯 번호를 받는다 */
	static FString SlotName(int32 Slot)
	{
		return FString::Printf(TEXT("CL9Save_%d"), Slot);
	}

	/** 필드를 더할 때마다 올린다 — 낮은 버전은 기본값으로 채워 읽는다 */
	static constexpr int32 CurrentVersion = 1;

	UPROPERTY()
	int32 SaveVersion = CurrentVersion;

	UPROPERTY()
	int32 CurrentDay = 1;

	UPROPERTY()
	int32 SharedMoney = 0;

	UPROPERTY()
	int32 PeriodIncome = 0;

	/** 아직 납품하지 못한 주문들 — 마감일은 절대 일차이며 CurrentDay와 함께 읽는다 */
	UPROPERTY()
	TArray<FCL9AcceptedOrder> ActiveOrders;
};
