// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "Data/CL9DefBase.h"
#include "Data/CL9Types.h"
#include "CL9ProgressionDef.generated.h"

/**
 * 유일한 일자 진행 애셋 (docs/plan/data/progression.md): "어느 날에 닭이 몇
 * 마리인가". 사이트가 기준값을 주고, 이 커브들이 일자별 배율을 주며,
 * DayOverrides가 개별 날짜를 손으로 고정한다. 커브는 인라인 FRuntimeFloatCurve로
 * — 애셋 에디터에서 그래프로 드래그하거나 UCurveFloat을 가리킬 수 있으며 —
 * 빈 커브는 1.0으로 읽힌다. 마지막 키 이후로는 그 값이 그대로 유지된다
 * (progression.md "10일 이후 구간은 마지막 키 값으로 고정"). rules 행처럼
 * 단일 애셋이며, 레지스트리는 처음 쓸 수 있는 것 하나만 유지한다.
 */
UCLASS()
class UCL9ProgressionDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	/** 그 날 사이트의 몬스터 수에 곱해질 배율 (오버라이드를 먼저 보고, 없으면 커브를 본다) */
	float MonsterCountAt(int32 Day) const;

	/** 그 날의 추격 예산·탐지 속도·시야에 곱해질 배율 */
	float DangerAt(int32 Day) const;

	/** 그 날 사이트의 재료 수에 곱해질 배율 */
	float ItemCountAt(int32 Day) const;

	/** 그 날 rules의 NightLength에 곱해질 배율 */
	float NightLengthAt(int32 Day) const;

	/** 그 날 사이트의 BaseModuleCount에 곱해질 배율 (docs/plan/data/module.md 결정 5) */
	float SiteSizeAt(int32 Day) const;

	/** 그 날 판매가·납품 정산에 곱해질 인플레이션 배율 (docs/plan/economy.md 가격식) */
	float ValueAt(int32 Day) const;

	/**
	 * 그 정산일에 벌어야 하는 이더 (docs/systems/balance.md 실패 판정).
	 * **빈 커브는 0을 뜻한다** — 다른 커브와 달리 배율이 아니라 목표액이라, 비어 있으면
	 * "목표 없음"이지 "1이더"가 아니다. 곡선을 안 그린 채로 판이 갑자기 실패하지 않게 한다.
	 */
	int32 QuotaAt(int32 Day) const;

	/** 그 날이 정산일이면 true — SettlementIntervalDays가 0 이하면 정산이 없다 */
	bool IsSettlementDay(int32 Day) const;

	/** DayOverride가 그 날의 정전을 끄면 true */
	bool IsBlackoutDisabled(int32 Day) const;

	/** 1일차 0.6 → 5일차 1.0 → 10일차 1.5 (초안) */
	UPROPERTY(EditAnywhere, Category="Curves")
	FRuntimeFloatCurve MonsterCountCurve;

	/** 1일차 0.8 → 10일차 1.3 (초안) */
	UPROPERTY(EditAnywhere, Category="Curves")
	FRuntimeFloatCurve DangerCurve;

	/** 평평하게 1.0 (초안) */
	UPROPERTY(EditAnywhere, Category="Curves")
	FRuntimeFloatCurve ItemCountCurve;

	/** 평평하게 1.0 (초안) */
	UPROPERTY(EditAnywhere, Category="Curves")
	FRuntimeFloatCurve NightLengthCurve;

	/**
	 * 한 판의 모듈 수 배율 — 1일차 0.85 → 5일차 1.0 → 10일차 1.25 (module.md 결정 5).
	 * 사이트의 BaseModuleCount 24에 곱하면 20 · 24 · 30이 나온다. 다른 커브와 달리
	 * 이건 맵 자체의 크기라, 값을 키우면 생성 시간과 인스턴스 수가 함께 늘어난다.
	 */
	UPROPERTY(EditAnywhere, Category="Curves")
	FRuntimeFloatCurve SiteSizeCurve;

	/** 가격 인플레이션 — 목표액과 짝을 이뤄 오른다 (초안 1일차 1.0 → 10일차 1.4) */
	UPROPERTY(EditAnywhere, Category="Curves")
	FRuntimeFloatCurve ValueCurve;

	/** 손으로 고정한 날짜들 — 1일차는 튜토리얼처럼 진행된다 (progression.md) */
	UPROPERTY(EditAnywhere, Category="Overrides")
	TArray<FCL9DayOverride> DayOverrides;

	/** 로비당 제공되는 주문 수, N (progression.md 주문서 — N = M이면 선택의 의미가 없어진다) */
	UPROPERTY(EditAnywhere, Category="Orders")
	int32 OrdersOfferedPerDay = 3;

	/** 반드시 수락해야 하는 주문 수, M */
	UPROPERTY(EditAnywhere, Category="Orders")
	int32 OrdersRequiredPerDay = 2;

	/** 정산일의 목표 이더 — 첫 정산(2일차) 800에서 시작해 10일 이후에도 계속 오른다 (초안) */
	UPROPERTY(EditAnywhere, Category="Settlement")
	FRuntimeFloatCurve QuotaCurve;

	/** 며칠에 한 번 정산하나 (초안 2) — 0 이하면 정산하지 않는다 */
	UPROPERTY(EditAnywhere, Category="Settlement")
	int32 SettlementIntervalDays = 2;

private:
	const FCL9DayOverride* FindOverride(int32 Day) const;

	/** 그 날의 커브 값; 빈 커브면 1.0 */
	static float Eval(const FRuntimeFloatCurve& Curve, int32 Day);
};
