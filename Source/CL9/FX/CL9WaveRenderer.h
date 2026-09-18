// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9Types.h"
#include "Subsystems/WorldSubsystem.h"
#include "UObject/ObjectKey.h"
#include "CL9WaveRenderer.generated.h"

class UCL9WaveTable;
class UNiagaraComponent;
struct FCL9WaveInfo;

/**
 * 파동(소음 반경 시각화)을 이 머신에 그린다 (docs/systems/wave.md 각 머신의 재생).
 * **표현 전용이며 복제에 닿지 않는다** — 판정은 전부 호스트에 있고 여기는 같은 표를 읽어
 * 같은 링을 파생시킬 뿐이다. 레벨과 함께 살고 죽으므로 `UWorldSubsystem`이다
 * (맵이 생성기로 만들어지는 이 게임에서 "레벨에 액터를 하나 놓아야 한다"는 전제는 취약하다).
 *
 * 파동은 틱을 돌지 않는다 — 스폰 뒤의 모든 움직임은 나이아가라 안에서 일어난다.
 */
UCLASS()
class UCL9WaveRenderer : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** 어떤 월드 오브젝트에서든 이 월드의 렌더러를 얻는다 — 월드가 없으면 null */
	static UCL9WaveRenderer* Get(const UObject* WorldContextObject);

	/**
	 * 이 머신에 파동 하나를 그린다. 호스트는 `ACL9GameState::HostReportNoise`에서, 클라이언트는
	 * 방송 수신에서, 소유 클라이언트는 자기 소음을 예측해 직접 부른다.
	 *
	 * `bPredicted`는 "내가 방금 낸 소리를 서버 왕복 없이 먼저 그린다"는 뜻이다 — 자기 파동의
	 * 밝기(`OwnerPowerScale`)는 그 플래그와 `Instigator == 내 PlayerState` 중 하나만 맞아도 붙는다.
	 */
	void PlayWave(ECL9NoiseSource Source, const FVector& Location, AActor* Instigator, bool bPredicted);

	/** 이 소음을 낸 것이 이 머신의 로컬 플레이어인가 — 이중 재생 가드와 자기 밝기 판정이 함께 읽는다 */
	bool IsLocalInstigator(const AActor* Instigator) const;

	/** cl9.shownoise — 판정 반경 디버그 원과 사건 로그를 켜고 끈다. 켠 상태를 반환한다 (로컬 전용) */
	static bool ToggleShowNoise();

	/** 지금 소음 디버그가 켜져 있는가 */
	static bool IsShowingNoise()
	{
		return bShowNoise;
	}

private:
	/** 게임 인스턴스가 물고 있는 파동 표 — 없으면 파동이 하나도 그려지지 않는다 */
	const UCL9WaveTable* GetTable() const;

	/** 행의 `Visibility`가 지금 화면에 이것을 허락하는가 — 스폰 직전에 한 번만 본다 */
	bool IsVisibleNow(const FCL9WaveInfo& Row) const;

	/**
	 * 최종 반경 (cm) — `RadiusFrom`이 가리키는 `CL9Rules` 소리 컬럼, `PackAlarm`이면 발신한 몬스터 행의
	 * `PackAlarmRadius`, `None`이면 `FallbackRadius`. 거기에 `RadiusScale`을 곱한다. 0이면 그릴 것이 없다.
	 */
	float ResolveRadius(const FCL9WaveInfo& Row, const AActor* Instigator) const;

	/** 발원지에서 수평으로 균등하게 `OcclusionRays`개 — 0번이 +X, 반시계. 광선별 최대 거리(cm)를 채운다 */
	void TraceOcclusion(const FVector& Location, const FCL9WaveInfo& Row, float Radius, const AActor* Instigator, TArray<float>& OutDistances) const;

	/** `MaxConcurrent`를 넘기지 않게 가장 오래된 파동부터 지운다 — 거의 다 퍼진 링이 먼저 사라진다 */
	void EnforceConcurrency(int32 MaxConcurrent);

	/** 살아 있는 파동 컴포넌트, 오래된 것이 앞 — 선입선출 상한의 대상이다 */
	TArray<TWeakObjectPtr<UNiagaraComponent>> LiveWaves;

	/** (발신자, 행) → 마지막으로 그린 월드 시각 — `MinInterval` 폭주 안전판 */
	TMap<TPair<FObjectKey, uint8>, double> LastPlayTimes;

	/** cl9.shownoise — 모든 월드가 함께 쓰는 로컬 디버그 스위치다 */
	static bool bShowNoise;
};
