// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "CL9PieceDef.generated.h"

class UStaticMesh;

/**
 * 맵을 조립할 때 쓰는 조각 하나의 역할 (docs/systems/asset-swap.md).
 * 생성기는 "여기에 벽이 선다"까지만 말하고, 그 자리에 어느 메시를 세울지는
 * 이 표가 정한다 — 그래서 에셋 팩을 바꿀 때 고칠 곳이 이 표 하나로 모인다.
 *
 * `Alias`가 역할(`wall` · `wallHalf` · `fence` · `door` · `box`)이고
 * `SubAlias`가 테마(`henhouse` · `warehouse` …)다. 테마 칸이 빈 행은 모든
 * 테마가 함께 쓰는 폴백이라, 테마가 그 역할을 갖추지 않았으면 여기로 떨어진다.
 *
 * **조각의 치수는 컬럼으로 적지 않는다** — ACL9SiteMap이 메시 바운드에서
 * 직접 읽는다. 손으로 옮겨 적은 숫자는 팩이 바뀌는 순간 조용히 틀린 값이
 * 되고, 그게 이 표를 만든 이유를 무너뜨린다.
 */
UCLASS()
class UCL9PieceDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	/**
	 * 이 역할로 쓸 수 있는 메시들. 둘 이상이면 배치할 때 자리마다 하나가
	 * 자리 좌표로 결정되므로(모든 머신이 같은 값을 고른다) 같은 벽이 줄줄이
	 * 늘어서는 걸 막을 수 있다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Piece")
	TArray<TSoftObjectPtr<UStaticMesh>> Variants;
};
