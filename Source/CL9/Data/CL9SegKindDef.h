// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "CL9SegKindDef.generated.h"

class UStaticMesh;

/** Where a segment's box stands (docs/plan/data/segkind.md BaseZMode) */
UENUM(BlueprintType)
enum class ECL9SegBaseZ : uint8
{
	/** Stands on the floor of the level it belongs to */
	LocalFloor,
	/** Stair walls: base on the lower floor, top measured from the upper floor — a throw from the ramp top cannot cross */
	WorldTop
};

/**
 * One kind of map piece (docs/plan/data/segkind.md). The generator emits
 * footprints by kind name; this row says what to stand there. The base Mesh
 * column is scaled to the footprint, so it must be a unit 100 cm cube (gray
 * box now, the real modular piece in ⑤ 5b). Not yet here: CollisionProfile,
 * MinimapColor — each joins with its first consumer.
 */
UCLASS()
class UCL9SegKindDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	/** Box height in cm — the throw-blocking height, not the ceiling (segkind.md 높이 210은 천장이 아니다) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Segment")
	float Height = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Segment")
	ECL9SegBaseZ BaseZMode = ECL9SegBaseZ::LocalFloor;

	/**
	 * ★ One flag for sight, sound and camera occlusion (segkind.md). False kinds
	 * (trough, crate, brooder) ignore the visibility channel so a chicken sees over them.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Segment")
	bool bBlocksSight = true;
};
