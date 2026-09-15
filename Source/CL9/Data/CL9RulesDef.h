// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "CL9RulesDef.generated.h"

/**
 * 유일한 전역 규칙 애셋 — 어떤 캐릭터도 다르게 가질 수 없는 값들이다: 소음,
 * 죽음, 밤 시계, 소지 마찰 (docs/plan/data/rules.md). 기본값은 문서화된 표를
 * 그대로 반영하며, 미정 컬럼은 결정되기 전까지 0으로 둔다.
 */
UCLASS()
class UCL9RulesDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	// -- 소리 (몬스터가 이 반경 안에서 듣는다 — build-order ③에서 사용) --

	/** 달릴 때의 소음 반경, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float RunNoiseRadius = 390.f;

	/** 걷는 중에 줍는 소음 반경, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float PickupQuietRadius = 150.f;

	/** 달린 직후에 줍는 소음 반경, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float PickupLoudRadius = 390.f;

	/** 밀치기, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float PushNoiseRadius = 255.f;

	/** 미끼, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float DecoyRadius = 330.f;

	/** 이 시간 안에 달린 적이 있으면 줍기가 시끄러운 것으로 간주된다, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float RecentRunTime = 1.f;

	// -- 죽음과 회복 --

	/** 죽은 뒤 팀원 관전으로 전환되기까지의 시간, 초 단위 — 초안 3, 플레이하며 조정 (docs/plan/data/rules.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float SpectateDelay = 3.f;

	/** 트럭 패드에 서서 HP가 다시 차기까지의 시간, 초 단위 — 초안 5, 플레이하며 조정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float TruckPadHealTime = 5.f;

	/** 이탈자의 폰이 남아있는 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float LeaverGraceTime = 3.f;

	/** 이 반경 안에서는 위협이 절대 깨어나거나 스폰되지 않는다, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float ThreatActMinRadius = 420.f;

	// -- 밤 시계 --

	/** 밤 길이, 초 단위 — 시간이 다 되면 새벽 실패가 된다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float NightLength = 245.f;

	/** 정전까지 걸리는 기본 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float DarkBase = 185.f;

	/** 항아리 반납 한 번이 정전 시각을 앞당기는 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float DarkAccel = 4.f;

	/** 가능한 가장 이른 정전 시각, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float DarkMin = 125.f;

	/** 새벽 화면 길이, 초 단위 — 미정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float DawnLength = 0.f;

	// -- 던지기와 소지 --

	/** 퀵슬롯 칸 수 — 한 번에 들고 다닐 수 있는 아이템 수. 손에 실물로 들리는 것은 활성 칸 하나뿐이다 (docs/systems/inventory.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	int32 QuickSlots = 4;

	/** 달리며 소지하는 상태가 지속되어 흔들림 게이지가 가득 차기까지의 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ShakeFullTime = 3.f;

	/** 흔들림 게이지 회복 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ShakeDrainTime = 1.5f;

	/** 충전 0일 때의 투척 속도, cm/s 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ThrowMinSpeed = 600.f;

	/** 완전 충전 시의 투척 속도, cm/s 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ThrowMaxSpeed = 1680.f;

	/** 완전 충전까지 걸리는 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ThrowChargeTime = 0.85f;

	/** 칸에서 꺼내 스폰하는 높이, cm 단위 — 놓기에는 안 쓴다(놓기는 손 자리, 2026-09-14) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float CarryHeight = 120.f;

	/** 줍기 사거리, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float PickRadius = 120.f;

	/**
	 * 우클 길게 누르기 문턱, 초 단위 — 이 시간 안에 떼면 탭(스캔), 넘기면 활성 칸의 요리를 먹는다
	 * (2026-09-13 사용자: "우클 길게 = 먹기"). 소유 클라가 로컬 타이머로 잰다 (docs/systems/inventory.md 조작).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float UseHoldTime = 0.4f;

	// -- 휘두르기 히트 구간 (docs/plan/data/item/common.md 사용) --

	/**
	 * 휘두르기 히트 구간의 시작, 초 단위 — 좌클을 뗀 뒤(`GetSwingElapsed()`) 이 시각부터 손에 든 것의 히트박스가 몬스터를
	 * 맞힌다. 휘두르기 클립은 ABP가 `SwingElapsed + 0.12`로 평가해 `[0, 0.347)` 동안만 돌아가므로 구간은 그 안이어야 한다
	 * (`Validate`). 접촉 프레임에서 정한다 (docs/asset/character/farmer_a.md 026/031). 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Swing")
	float SwingHitStart = 0.05f;

	/** 휘두르기 히트 구간의 끝, 초 단위 — 이 시각부터는 맞지 않는다(구간은 반열림 `[Start, End)`). 초안값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Swing")
	float SwingHitEnd = 0.25f;

};
