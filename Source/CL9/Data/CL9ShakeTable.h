// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Templates/SubclassOf.h"
#include "CL9ShakeTable.generated.h"

class UCameraShakeBase;

/** 흔들림 행 하나: 어떤 애셋을 쓸지, 그리고 그것이 시작되는 게이지 값 */
USTRUCT()
struct FCL9ShakeInfo
{
	GENERATED_BODY()

	/**
	 * 진폭, 주파수, 블렌드 시간은 모두 이 애셋 안에 있다 — 여기 컬럼으로는 두지 않는다. 유일한 예외가 아래 `Scale`
	 * 배율 열이다(2026-09-18 사용자): 애셋을 다시 만들지 않고 표에서 세기만 조절하기 위한 것
	 */
	UPROPERTY(EditAnywhere, Category="Shake")
	TSoftClassPtr<UCameraShakeBase> Shake;

	/**
	 * 구동 게이지(정규화된 0..1 값)가 이 값 아래로 떨어지면 재생되며, 세기는
	 * 0에서 최대치까지 올라간다. 스태미나는 이를 "티가 날 만큼 지쳤다"로 읽는다.
	 */
	UPROPERTY(EditAnywhere, Category="Shake")
	float StartAt = 0.5f;

	/**
	 * 진폭·주파수는 에셋, 이 열은 그 위에 곱하는 배율(표에서 세기 조절). 원샷 행은 `Scale`로 한 번, 게이지 행은
	 * 게이지 세기 × `Scale`로 재생된다 (`StartCameraShake`의 Scale 인자 → `ShakeScale`). 1.0 = 에셋 그대로
	 */
	UPROPERTY(EditAnywhere, Category="Shake")
	float Scale = 1.f;
};

/**
 * 카메라 흔들림 테이블 (2026-09-05 결정, docs/systems/character.md 흔들림): 흔들림을
 * 추가한다는 것은 흔들림 애셋을 만들고 여기에 행을 추가하는 것이다 — rules
 * 애셋에 흔들림마다 컬럼이 늘어나는 일은 없다. UCL9UITable과 같은 구조이며,
 * BP_CL9GameInstance에서 할당되므로 C++는 애셋 경로를 전혀 갖지 않는다
 * (conventions § 5).
 */
UCLASS()
class UCL9ShakeTable : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 이름 → 흔들림. 표현 코드가 조회하는 키다 */
	UPROPERTY(EditAnywhere, Category="Shake")
	TMap<FName, FCL9ShakeInfo> Rows;
};

/**
 * C++가 조회하는 행 이름들 — 그 외에는 전부 데이터 전용으로 남는다.
 *
 * 원샷 행(`Hit`·`Hurt`)에서 `StartAt`은 무시된다: 게이지가 없는 순간 사건이라 세기 `Scale`로 한 번 재생할 뿐이다.
 * `StartAt`은 스태미나처럼 게이지가 구동하는 행에만 뜻이 있다.
 */
namespace CL9ShakeNames
{
	/** 지친 숨소리, 정규화된 스태미나로 구동된다 */
	inline const FName Stamina(TEXT("Stamina"));

	/** 휘두르기가 몬스터에 닿은 순간 공격자 카메라가 한 번 툭 — 짧고 날카롭게 (원샷) */
	inline const FName Hit(TEXT("Hit"));

	/** 잡혀 넘어진 순간 피해자 카메라가 한 번 크게 — 묵직하게 (원샷) */
	inline const FName Hurt(TEXT("Hurt"));
}
