// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CL9SoundTable.generated.h"

class USoundBase;

/**
 * 효과음 테이블 (docs/goal/game-feel.md §1, docs/systems/sound.md): 효과음을 추가한다는 것은 사운드 애셋을 만들고
 * 여기에 행을 추가하는 것이다 — rules 애셋에 소리마다 컬럼이 늘어나는 일은 없다. UCL9ShakeTable과 같은 구조이며,
 * BP_CL9GameInstance에서 할당되므로 C++는 애셋 경로를 전혀 갖지 않는다 (conventions § 5).
 *
 * 볼륨·감쇠·동시 발성 제한은 전부 행이 가리키는 사운드 애셋(또는 SoundCue) 안에 있다. 코드는 `Footstep`의 피치만 흔든다.
 */
UCLASS()
class UCL9SoundTable : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 이름 → 사운드. 표현 코드가 조회하는 키다 — 없는 행은 조용히 무시된다(오류가 아니다) */
	UPROPERTY(EditAnywhere, Category="Sound")
	TMap<FName, TSoftObjectPtr<USoundBase>> Rows;
};

/**
 * C++가 조회하는 행 이름들 — 그 외에는 전부 데이터 전용으로 남는다. 재생 지점은 전부 표현 층이고, 이미 복제되는
 * 시계·상태에서 파생한다(새 RPC 없음, conventions § 9). 어느 머신에서 어느 순간 나는지는 docs/systems/monster.md
 * "타격의 표현"·character.md·sound.md 에 있다.
 */
namespace CL9SoundNames
{
	/** 줍는 순간 — 이번 실행(feel-hit-kill)에는 재생 지점이 없다. `PickupServerTime`(다음 실행)이 생기면 그 변화에서 튼다 */
	inline const FName Pickup(TEXT("Pickup"));

	/** 손을 떠나는 순간(놓기·던지기) — 아이템 자리에서, 모든 머신 */
	inline const FName Place(TEXT("Place"));

	/** 휘두르기 시작(`SwingStartServerTime` 변화) — 폰 자리에서, 모든 머신. 맞든 빗나가든 난다 */
	inline const FName Swing(TEXT("Swing"));

	/** 몬스터가 Stun·Dead에 들어오는 순간(타격 플래시와 같은 에지) — 몬스터 자리에서, 모든 머신 */
	inline const FName Hit(TEXT("Hit"));

	/**
	 * 휘두르기가 세상 지형(벽·바닥)에 닿은 순간(`SwingSurfaceHitServerTime` 변화, 조건 없는 복제) — 폰 자리에서, 모든 머신.
	 * 휘두르기당 한 번, 그보다 가까운 몬스터 명중이 없을 때만. 충돌점은 복제하지 않는다 (docs/systems/character.md 표면음)
	 */
	inline const FName HitSurface(TEXT("HitSurface"));

	/** 잡혀 넘어지는 순간(`KnockdownStartServerTime` 변화) — 폰 자리에서, 모든 머신 */
	inline const FName Hurt(TEXT("Hurt"));

	/** 몬스터가 Dead에 들어오는 순간 — `Hit`과 함께 난다 */
	inline const FName MonsterDeath(TEXT("MonsterDeath"));

	/**
	 * 발소리 — 폰·몬스터가 보폭(`CL9Rules.FootstepStrideRun/Walk`)만큼 수평 이동할 때마다 발밑에서 한 번, 모든 머신.
	 * 행은 하나다: 변형 여러 개는 SoundCue 의 Random 노드로 묶는다. 피치만 코드가 ±8 % 흔든다
	 */
	inline const FName Footstep(TEXT("Footstep"));

	/**
	 * 몬스터가 잡기 예고(Pounce)에 들어오는 순간 — 몬스터 자리에서, 모든 머신. 상태 전이 감지(`ACL9Monster::TickStateSounds`:
	 * State == Pounce ∧ (지난 틱 State != Pounce ∨ StateStartServerTime 변화))라 늦게 온 복제도 놓치지 않는다
	 */
	inline const FName MonsterPounce(TEXT("MonsterPounce"));

	/** 몬스터가 Catch 에 들어오는 순간(잡기 성공) — 같은 전이 감지, 몬스터 자리, 모든 머신. 잡힌 쪽 `Hurt` 와 같은 순간에 겹친다. 헛침엔 없다 */
	inline const FName MonsterCatch(TEXT("MonsterCatch"));

	/**
	 * 다가오는 소리 — 파동 표 행의 `SoundRow` 가 이 이름을 가리키면(`monster_prox`) 그 소음 사건마다 `UCL9WaveRenderer::PlayWave` 가
	 * 링과 같은 박자로 튼다(시각 게이트 앞이라 링이 안 그려져도 난다). C++ 에는 재생 지점이 따로 없다 — 이름만 여기 둔다
	 */
	inline const FName MonsterNear(TEXT("MonsterNear"));
}

namespace CL9Sound
{
	/**
	 * 표의 행 하나를 월드 위치에서 한 번 재생한다. 표·행·애셋이 없으면 **조용히** 아무것도 하지 않는다(로그 없음) —
	 * 소리는 있어도 그만 없어도 그만인 표현이다. 전용 서버에서는 return. PitchJitter 는 피치 배율의 반폭(0.08 = ±8 %).
	 */
	void PlayAt(const UObject* WorldContext, FName Row, const FVector& Location, float PitchJitter = 0.f);

	/** `Footstep` 행에 코드가 주는 피치 흔들림 반폭 — 발소리마다 조금씩 다르게 들리기 위한 것이지 밸런스 값이 아니다 */
	constexpr float FootstepPitchJitter = 0.08f;
}
