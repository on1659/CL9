// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "CL9SegKindDef.generated.h"

/** 세그먼트 박스가 서는 기준점 (docs/plan/data/segkind.md BaseZMode) */
UENUM(BlueprintType)
enum class ECL9SegBaseZ : uint8
{
	/** 자신이 속한 레벨의 바닥 위에 선다 */
	LocalFloor,
	/** 계단 벽: 바닥 기준은 아래층, 꼭대기는 위층 기준으로 잰다 — 램프 꼭대기에서 던지면 넘어갈 수 없다 */
	WorldTop
};

/** 조각을 발자국에 어떻게 앉히는지 (docs/systems/asset-swap.md 배치 모드) */
UENUM(BlueprintType)
enum class ECL9SegPlacement : uint8
{
	/**
	 * 조각 하나를 발자국 크기로 늘린다. 회색 상자나 무늬 없는 면(바닥·천장)처럼
	 * 늘어나도 티가 안 나는 것에만 쓴다.
	 */
	Stretch,
	/**
	 * 조각을 자연 크기 그대로 발자국에 깔아 나간다 — 길이 축으로 늘어놓고 높이
	 * 축으로 쌓으며, 두께는 절대 늘리지 않고 발자국 가운데에 세운다. 판자·벽돌
	 * 무늬가 있는 모듈러 조각은 전부 이쪽이다.
	 */
	Tile
};

/**
 * 맵 조각의 한 종류 (docs/plan/data/segkind.md). 생성기는 종류 이름으로
 * 발자국(footprint)을 찍고, 이 행은 그 자리에 무엇을 어떻게 세울지 정한다.
 *
 * 메시를 직접 가리키지 않고 **역할 이름**(`Piece`)을 가리킨다 — 실제 메시는
 * 조각 표(`UCL9PieceDef`)가 테마별로 들고 있다. 그래서 에셋 팩을 갈아 끼울 때
 * 이 표는 손대지 않는다 (docs/systems/asset-swap.md).
 *
 * 아직 없는 것: CollisionProfile, MinimapColor — 각각 첫 사용처가 생길 때 함께 추가한다.
 */
UCLASS()
class UCL9SegKindDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	/** 세울 조각의 역할 이름 — 조각 표(UCL9PieceDef)의 Alias를 가리킨다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Segment")
	FName Piece;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Segment")
	ECL9SegPlacement Placement = ECL9SegPlacement::Stretch;

	/** 박스 높이, cm 단위 — 천장이 아니라 투척 차단 높이다 (segkind.md 높이 210은 천장이 아니다) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Segment")
	float Height = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Segment")
	ECL9SegBaseZ BaseZMode = ECL9SegBaseZ::LocalFloor;

	/**
	 * ★ 시야, 소리, 카메라 가림을 한 플래그로 처리한다 (segkind.md). false인
	 * 종류(여물통, 상자, 부화기)는 가시성 채널을 무시하므로 닭이 그 너머를 볼 수 있다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Segment")
	bool bBlocksSight = true;
};
