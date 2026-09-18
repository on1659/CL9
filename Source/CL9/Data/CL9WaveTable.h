// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9Types.h"
#include "Engine/DataAsset.h"
#include "CL9WaveTable.generated.h"

class UNiagaraSystem;

/** 파동이 언제 보이는가 (docs/systems/wave.md 표시 시점 판정) — 스폰 직전에 한 번만 본다 */
UENUM(BlueprintType)
enum class ECL9WaveVisibility : uint8
{
	/** 언제나 */
	Always,
	/** 밤(ECL9NightPhase::Night) 동안만 */
	NightOnly,
	/** 정전(ACL9GameState::IsBlackout()) 뒤에만 */
	BlackoutOnly
};

/**
 * 링의 반경을 어느 판정 컬럼에서 가져오는가 (docs/systems/wave.md 행 스키마 `RadiusFrom`).
 * **반경은 파동 표가 정하지 않는다** — 판정이 정하고 링은 그것을 비출 뿐이다.
 */
UENUM(BlueprintType)
enum class ECL9NoiseRadiusSource : uint8
{
	/** 가져올 판정 컬럼이 없다 — 행의 `FallbackRadius`를 쓴다 */
	None,
	/** CL9Rules.RunNoiseRadius */
	RunNoise,
	/** CL9Rules.PickupQuietRadius */
	PickupQuiet,
	/** CL9Rules.PickupLoudRadius */
	PickupLoud,
	/** CL9Rules.PushNoiseRadius */
	PushNoise,
	/** CL9Rules.DecoyRadius */
	Decoy,
	/** 발신한 몬스터 행의 PackAlarmRadius — 링이 닿은 동료가 실제로 깨어난다 */
	PackAlarm
};

/**
 * 소음원 하나가 그리는 파동 한 줄 (docs/systems/wave.md 행 스키마 · 값은 docs/plan/data/wave.md).
 * 여기에는 수치가 하나도 없다 — 아래 기본값은 표가 비어 있을 때의 자리표시이며,
 * 실제로 게임이 읽는 것은 저장된 애셋이다.
 */
USTRUCT()
struct FCL9WaveInfo
{
	GENERATED_BODY()

	/** 이 파동이 재생할 나이아가라 시스템. 비어 있으면 그 행은 아무것도 그리지 않는다(나머지 판정·로그는 그대로 돈다) */
	UPROPERTY(EditAnywhere, Category="Wave")
	TSoftObjectPtr<UNiagaraSystem> Niagara;

	/** 이 소음원이 파동을 내는가 — 구현은 전부 되어 있고 이 체크박스가 켜고 끈다 */
	UPROPERTY(EditAnywhere, Category="Wave")
	bool bEnabled = true;

	/** 언제 보이나 */
	UPROPERTY(EditAnywhere, Category="Wave")
	ECL9WaveVisibility Visibility = ECL9WaveVisibility::Always;

	/** 반경을 어느 판정 컬럼에서 가져오나 */
	UPROPERTY(EditAnywhere, Category="Wave")
	ECL9NoiseRadiusSource RadiusFrom = ECL9NoiseRadiusSource::None;

	/** RadiusFrom == None인 행의 반경 (cm) — 판정 반경이 없는 행동만 쓴다 */
	UPROPERTY(EditAnywhere, Category="Wave")
	float FallbackRadius = 0.f;

	/** 최종 반경 = (위 둘 중 하나) × 이 값. 1.0이 아니면 링이 판정과 달라진다(로드 검사가 경고한다) */
	UPROPERTY(EditAnywhere, Category="Wave")
	float RadiusScale = 1.f;

	/** 링이 퍼지는 속도 (cm/s) — 수명 = 반경 ÷ 속도 */
	UPROPERTY(EditAnywhere, Category="Wave")
	float Speed = 0.f;

	/** 링 띠의 두께 (cm) */
	UPROPERTY(EditAnywhere, Category="Wave")
	float Thickness = 0.f;

	/** 기본 색 */
	UPROPERTY(EditAnywhere, Category="Wave")
	FLinearColor Color = FLinearColor::White;

	/** 낸 사람의 슬롯 색으로 덮어쓸지 — 몬스터 행은 false */
	UPROPERTY(EditAnywhere, Category="Wave")
	bool bUseInstigatorColor = false;

	/** 밝기·알파 배율 — 나이아가라에 그대로 넘어간다 */
	UPROPERTY(EditAnywhere, Category="Wave")
	float Power = 1.f;

	/** 자기가 낸 파동에만 곱하는 밝기 — 내 소리와 남의 소리를 가른다 */
	UPROPERTY(EditAnywhere, Category="Wave")
	float OwnerPowerScale = 1.f;

	/** 생성 때 쏘는 수평 광선 수 (개). 0이면 차폐 계산을 하지 않는다 */
	UPROPERTY(EditAnywhere, Category="Wave")
	int32 OcclusionRays = 0;

	/** 광선을 쏘는 높이 (cm, 발원점 기준) */
	UPROPERTY(EditAnywhere, Category="Wave")
	float TraceHeight = 0.f;

	/** 벽에 닿은 자리에 세로 자국을 찍나 */
	UPROPERTY(EditAnywhere, Category="Wave")
	bool bWallStamp = false;

	/** 자국 기둥의 높이 (cm) */
	UPROPERTY(EditAnywhere, Category="Wave")
	float StampHeight = 0.f;

	/** 자국 한 자리에 찍는 점 수의 배율 (에셋 기본에 대한 배율) */
	UPROPERTY(EditAnywhere, Category="Wave")
	float StampDensity = 0.f;

	/** 발신자가 이 거리(cm)를 이동할 때마다 한 번 발동. 0 = 원샷 */
	UPROPERTY(EditAnywhere, Category="Wave")
	float StepDistance = 0.f;

	/** 이 시간(초)마다 반복 발동. 0 = 반복 없음 */
	UPROPERTY(EditAnywhere, Category="Wave")
	float RepeatInterval = 0.f;

	/** 같은 발신자·같은 행이 이 간격(초) 안에 두 번 나지 않는다 — 폭주 안전판 */
	UPROPERTY(EditAnywhere, Category="Wave")
	float MinInterval = 0.f;

	/** 이 안(cm)에 플레이어가 있을 때만 발동. 0 = 조건 없음 (monster_prox 전용) */
	UPROPERTY(EditAnywhere, Category="Wave")
	float TriggerRadius = 0.f;
};

/**
 * 파동 표 (docs/systems/wave.md 표 — `UCL9ShakeTable`과 같은 틀): `UDataAsset` 하나가
 * `TMap`으로 행을 품고, `BP_CL9GameInstance`가 그 에셋을 물고 있어 C++는 에셋 경로를
 * 한 글자도 갖지 않는다 (conventions § 5).
 *
 * `UCL9DefBase` 파생을 쓰지 않은 이유는 문서에 있다 — 파동 행에는 DisplayName·Icon·Mesh·
 * ActorClass가 전부 무의미해서 상속 컬럼 일곱 중 여섯이 빈칸으로 남는다.
 */
UCLASS()
class UCL9WaveTable : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 소음원 → 그 소음원이 그리는 파동 */
	UPROPERTY(EditAnywhere, Category="Wave")
	TMap<ECL9NoiseSource, FCL9WaveInfo> Rows;

	/** 한 머신에서 동시에 살아 있는 파동 수 상한 — 넘으면 가장 오래된 것을 지운다 */
	UPROPERTY(EditAnywhere, Category="Wave")
	int32 MaxConcurrent = 0;

	/** 로컬 카메라에서 이 거리(cm)보다 먼 발원지는 아예 만들지 않는다 */
	UPROPERTY(EditAnywhere, Category="Wave")
	float CullDistance = 0.f;

	/**
	 * 쓸 수 있는 행, 없거나 로드 검사에서 떨어진 행이면 null. 표를 읽는 곳은 전부 이 문을 지난다 —
	 * 검사에 떨어진 행이 "비활성"이 되는 자리가 여기 하나다.
	 */
	const FCL9WaveInfo* FindRow(ECL9NoiseSource Source) const;

	/**
	 * 이 소음원이 살아 있는가 — 행이 있고, 로드 검사를 통과했고, `bEnabled`다. 꺼진 행은 그리지 않을 뿐 아니라
	 * **사건도 내지 않는다**(2026-09-16 사용자 — 걷기처럼 늘 나는 소음이 빈 멀티캐스트로 오가지 않게).
	 */
	bool IsEnabled(ECL9NoiseSource Source) const
	{
		const FCL9WaveInfo* Row = FindRow(Source);
		return Row != nullptr && Row->bEnabled;
	}

	/**
	 * 표를 받은 직후에 한 번 도는 로드 검사 (docs/systems/wave.md 로드 검사) — 어긋난 행을
	 * LogCL9에 찍고 비활성 목록에 넣는다. 게임 인스턴스가 표를 로드한 직후에 부른다.
	 */
	void ValidateRows();

private:
	/** 로드 검사에서 떨어져 비활성이 된 행들 — 애셋에 저장되지 않는 런타임 값이다 */
	TSet<ECL9NoiseSource> DisabledRows;
};

/** 소음원의 문서·표 이름(alias) — 로그와 치트가 읽고 쓴다 (docs/systems/wave.md 소음원 enum) */
namespace CL9Wave
{
	/** `PickupQuiet` → "pickup_quiet" */
	FString ToAlias(ECL9NoiseSource Source);

	/** "pickup_quiet"도 "PickupQuiet"도 받는다 — None과 못 찾은 이름은 false */
	bool FromAlias(const FString& Token, ECL9NoiseSource& OutSource);
}
