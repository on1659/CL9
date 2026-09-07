// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CL9FlapIn.generated.h"

class UBoxComponent;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * 축사 벽에 난 일방향 여닫이문(docs/plan/data/segkind.md flapIn) — 바깥에서 보면 벽이고,
 * 안에서 보면 나가는 길이다. 캐릭터가 안쪽 트리거에 닿으면 어느 한쪽으로 트리거를 벗어날
 * 때까지 캡슐이 이 액터를 무시하도록 만들어 바깥으로 밀고 나갈 수 있게 하고, 바깥에서
 * 들어오려는 쪽은 그냥 벽에 막힌다. 동물과 던진 물체는 양쪽 모두 막힌다.
 * ACL9SiteMap이 스폰하며, 직접 배치하지 않는다.
 */
UCLASS(NotBlueprintable, NotPlaceable)
class ACL9FlapIn : public AActor
{
	GENERATED_BODY()

public:
	ACL9FlapIn();

	/** 몸체와 트리거의 크기를 정한다. InsideDirection은 벽에서 축사 안쪽을 향하는 방향이다 */
	void Setup(const FVector& Extent, const FVector& InsideDirection, UStaticMesh* Mesh);

protected:
	UFUNCTION()
	void OnTriggerBegin(UPrimitiveComponent* Overlapped, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Sweep);

	UFUNCTION()
	void OnTriggerEnd(UPrimitiveComponent* Overlapped, AActor* Other, UPrimitiveComponent* OtherComp, int32 BodyIndex);

	/** 벽 본체 — 모든 것을 막는다. 캐릭터는 트리거 안에 있는 동안 move-ignore로 이를 통과한다 */
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UBoxComponent> Body;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Visual;

	/** 벽을 걸치되 안쪽으로 더 깊게 들어간다 — 폰(pawn) 오버랩만 감지한다 */
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UBoxComponent> Trigger;

	FVector InsideDir = -FVector::ForwardVector;
};
