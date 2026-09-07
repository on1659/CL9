// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/CL9ActorBase.h"
#include "CL9Fixture.generated.h"

class UStaticMeshComponent;

/**
 * 레벨에 붙박이로 서 있는 것들의 공통 베이스 — 조리대, 납품 창구, 트럭 패널, 반납 항아리.
 * 주워 갈 수 없고, 자리를 알아야 쓸모가 있으며, 대부분 상호작용 대상이다.
 *
 * `ACL9ActorBase`를 상속하는 이유는 **스캔이 이 액터들을 보게 하려는 것**이다
 * (2026-09-07 사용자: "스캔하면 몬스터와 부엌 같은 것도 나오게"). 스캔은 종류(`ECL9ActorKind`)로
 * 대상을 고르는데, 그 종류가 베이스에 있기 때문이다. Alias 칸도 함께 딸려 오지만 지금은
 * 비워 둔다 — 시설물 표가 생기면 그 자리에 그대로 쓰면 된다.
 *
 * 이름은 행이 아니라 `DisplayName`에서 온다. 시설물은 표가 없어서 라벨에 쓸 이름을 따로
 * 들고 있어야 하고, 그 이름은 BP 자식이나 배치된 인스턴스가 정한다.
 */
UCLASS(abstract)
class ACL9Fixture : public ACL9ActorBase
{
	GENERATED_BODY()

public:
	ACL9Fixture();

	virtual FText GetDisplayName() const override;

protected:
	/** 몸체 — 이동을 막으며, 메시는 BP 자식에서 지정된다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** 스캔 라벨에 뜰 이름 — 표가 없는 시설물이므로 여기서 정한다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	FText DisplayName;
};
