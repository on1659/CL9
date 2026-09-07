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

	/** 진폭, 주파수, 블렌드 시간은 모두 이 애셋 안에 있다 — 여기 컬럼으로는 절대 두지 않는다 */
	UPROPERTY(EditAnywhere, Category="Shake")
	TSoftClassPtr<UCameraShakeBase> Shake;

	/**
	 * 구동 게이지(정규화된 0..1 값)가 이 값 아래로 떨어지면 재생되며, 세기는
	 * 0에서 최대치까지 올라간다. 스태미나는 이를 "티가 날 만큼 지쳤다"로 읽는다.
	 */
	UPROPERTY(EditAnywhere, Category="Shake")
	float StartAt = 0.5f;
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

/** C++가 조회하는 행 이름들 — 그 외에는 전부 데이터 전용으로 남는다 */
namespace CL9ShakeNames
{
	/** 지친 숨소리, 정규화된 스태미나로 구동된다 */
	inline const FName Stamina(TEXT("Stamina"));
}
