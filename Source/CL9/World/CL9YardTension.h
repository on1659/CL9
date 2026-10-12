// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CL9YardTension.generated.h"

class UPointLightComponent;
class UStaticMeshComponent;

/**
 * 마당을 건너는 길의 긴장 — 보이고 들리기만 하고 판정에는 닿지 않는다(docs/goal/loop-7j-round-start.md). 머신마다 로컬로 하나
 * (로컬 플레이어 컨트롤러가 세운다)이며 복제하지 않는다. 판마다 생성된 배치에서 **마당과 건물 사이의 문**을 찾아
 * - 문 바깥 위에 깜빡이는 작업등을 걸고(`CL9Rules.YardLamp*`),
 * - 로컬 플레이어가 전원을 켜기 전 마당에 있으면 건물 안쪽에서 울음(`YardCry` 소리 행)을 가끔 내고,
 * - 마당에서 문을 바라보면 문마다 판에 한 번, 어두운 실루엣이 문 너머를 가로지른다(`CL9Rules.Silhouette*`, 뒤에서 비추는 빛).
 * 창문이 키트에 없어 실루엣 자리는 문간이다(사용자 답의 "창문"에 가장 가까운 자리).
 */
UCLASS(NotPlaceable)
class ACL9YardTension : public AActor
{
	GENERATED_BODY()

public:
	ACL9YardTension();

	virtual void Tick(float DeltaSeconds) override;

private:
	/** 마당 → 건물 문 하나 — 문 가운데(바닥), 건물 안쪽 방향, 문 폭 방향 */
	struct FEntrance
	{
		FVector Door = FVector::ZeroVector;
		FVector Inward = FVector::ForwardVector;
		FVector Lateral = FVector::RightVector;
	};

	/** 지금 판의 배치에서 문을 다시 찾고 등을 다시 건다 — 시드가 바뀌었을 때만 */
	void Rebuild(uint32 Seed);

	/** 등을 모두 걷는다 */
	void ClearLamps();

	/** 로컬 폰이 마당(실외 모듈 또는 격자 밖)에 있나 */
	bool IsInYard(const FVector& Location) const;

	void TickLamps(float Now, bool bNight);
	void TickCries(float Now, const FVector& PawnLocation);
	void TickSilhouette(float DeltaSeconds, const FVector& ViewLocation, const FVector& ViewDirection, bool bYard);
	void EndSilhouette();

	TArray<FEntrance> Entrances;
	uint32 BuiltForSeed = 0;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> LampMeshes;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UPointLightComponent>> Lamps;

	/** 다음 울음 시각(월드 초) — 0 이면 아직 정하지 않았다 */
	float NextCryTime = 0.f;

	/** 이번 판에 실루엣을 이미 보인 문 */
	TSet<int32> SilhouetteShown;

	UPROPERTY(Transient)
	TObjectPtr<UPointLightComponent> BackLight;

	FVector SilhouetteStart = FVector::ZeroVector;
	FVector SilhouetteDirection = FVector::ForwardVector;
	float SilhouetteTravelled = 0.f;
};
