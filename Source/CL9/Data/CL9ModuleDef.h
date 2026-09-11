// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "CL9ModuleDef.generated.h"

/** 모듈이 배치에서 맡는 자리 (docs/plan/data/module.md 표 컬럼) */
UENUM(BlueprintType)
enum class ECL9ModuleType : uint8
{
	/** 트럭이 붙는 입구. 한 판에 정확히 하나 */
	Start,
	/** 방 — 배치의 본체 */
	Normal,
	/** 복도·계단처럼 지나가는 곳 */
	Passage,
	/** 한쪽으로만 뚫린 곁가지 끝 */
	DeadEnd,
	/** 반드시 나와야 하는 방 (납품 항아리·조리대) */
	Special,
	/**
	 * 위층으로 올라가는 계단실. **아래층과 위층의 같은 칸을 함께 차지한다** — 그래서
	 * 이 모듈 하나가 두 층을 잇는 유일한 통로가 된다. 한 판에 몇 개가 서는지는
	 * 사이트 행의 `StairCountMin`·`StairCountMax` 사이에서 시드가 뽑는다.
	 */
	Stair
};

/** 모듈 타입의 가짓수 — 타입별 후보 묶음을 만들 때 쓴다 */
constexpr int32 CL9ModuleTypeCount = 6;

/**
 * 모듈의 네 변. **비트 순서가 곧 시계 방향(N→E→S→W)이라 회전이 비트 회전이 된다** —
 * 반시계 90° × r 회전은 `새 비트[j] = 옛 비트[(j + r) % 4]`다.
 *
 * 소켓의 단위는 변이 아니라 **경계 칸**이다 (module.md 결정 1): 이 플래그는 "그 변이
 * 열린다"까지만 말하고, 그러면 그 변에 닿은 모든 경계 칸이 문 후보가 된다. 변 중앙에
 * 하나로 고정할 수 없는 이유는 2×2·3×2처럼 변이 짝수 칸인 모듈에서 중앙이 칸 경계에
 * 떨어지기 때문이다.
 */
UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ECL9ModuleSide : uint8
{
	/** +Y */
	North = 1 << 0,
	/** +X */
	East = 1 << 1,
	/** -Y */
	South = 1 << 2,
	/** -X */
	West = 1 << 3
};
ENUM_CLASS_FLAGS(ECL9ModuleSide);

/**
 * 이 모듈의 칸이 어떤 지점(`FCL9LevelPoint`)이 되는지 (docs/plan/data/module.md 결정 4,
 * REPO의 `LevelPoint`에 해당). 밤이 시작될 때 스폰이 이 용도로 자리를 고른다 — 그래서
 * "항아리는 납품 방에만" 같은 규칙이 C++에 이름으로 박히지 않고 표에 남는다.
 */
UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ECL9PointPurpose : uint8
{
	/** 몬스터를 놓을 수 있다 */
	Monster = 1 << 0,
	/** 재료를 놓을 수 있다 */
	Loot = 1 << 1,
	/** 반납 항아리를 놓는다 — 납품 방만 켠다 */
	Jar = 1 << 2,
	/** 순찰이 지나갈 만한 자리 */
	Patrol = 1 << 3
};
ENUM_CLASS_FLAGS(ECL9PointPurpose);

/**
 * 맵 모듈 한 종류 (docs/plan/data/module.md). **모듈은 프리팹이 아니라 이 표의 행이다** —
 * 껍데기(바닥·벽·문·천장)는 생성기가 조각 표(UCL9PieceDef)에서 뽑아 세우므로, 손으로
 * 만들어 두는 방 애셋이 없다. 이 표가 정하는 건 "몇 칸짜리 방이고 어느 변이 뚫리나"까지다.
 *
 * `Alias`가 모듈 이름(`coop_hall` · `corridor`)이고 `SubAlias`가 스테이지/테마
 * (`henhouse`)다 — 조각 표와 같은 규약이라, 다음 스테이지는 SubAlias만 다른 한 벌이 된다.
 *
 * **치수를 손으로 적지 않는다는 원칙은 그대로다** — 조각의 크기와 피벗은 메시 바운드에서
 * 읽는다 (docs/systems/asset-swap.md). 여기 적는 `Size`는 조각 치수가 아니라 격자 칸
 * 개수이고, `Height`는 그 방의 천장이 얼마나 높은 방인가라는 기획값이다.
 */
UCLASS()
class UCL9ModuleDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	/** 회전 r(반시계 90° 단위)을 적용한 뒤의 소켓 비트 */
	static uint8 RotateSockets(uint8 Sockets, int32 Rotation);

	/** 회전 r을 적용한 뒤의 칸 크기 — 홀수 회전이면 X와 Y가 바뀐다 */
	static FIntPoint RotateSize(FIntPoint Size, int32 Rotation);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
	ECL9ModuleType Type = ECL9ModuleType::Normal;

	/** 격자 칸 단위 크기, 회전 전 기준 (칸 하나가 UCL9SiteDef::CellSize) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
	FIntPoint Size = FIntPoint(2, 2);

	/** 어느 변이 열리나 — 회전 전 기준. 하나도 안 열린 모듈은 배치될 수 없다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module", meta = (Bitmask, BitmaskEnum = "/Script/CL9.ECL9ModuleSide"))
	int32 Sockets = 0xF;

	/** 뽑힐 가중치 1~3, 클수록 흔하다 (REPO의 ModuleRarity1/2/3) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
	int32 Rarity = 2;

	/** 이 방의 천장 높이 (cm). bOutdoor면 천장을 씌우지 않으므로 무시된다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
	float Height = 400.f;

	/**
	 * 모듈 **안에서** 오르내리는 높이차가 있는가 (module.md 결정 2). 이웃 모듈의 바닥
	 * 높이로 전파되지 않는다 — 계단은 올라간 만큼 도로 내려온다. 그래서 새 계단 모듈을
	 * 표에 추가할 때 격자 규칙을 함께 손볼 일이 없다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
	bool bStairs = false;

	/** 야외 모듈인가 (마당). 천장을 씌우지 않고, 벽 대신 마당 울타리를 세운다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
	bool bOutdoor = false;

	/**
	 * 이 모듈의 칸들이 어떤 스폰 지점이 되는가. 기본값은 "몬스터·재료·순찰 다 된다"이고,
	 * 납품 방만 Jar를 켠다. 입구 모듈은 전부 끄는 편이 낫다 — 트럭 문턱에서 몬스터가
	 * 튀어나오면 시작하자마자 죽는다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module", meta = (Bitmask, BitmaskEnum = "/Script/CL9.ECL9PointPurpose"))
	int32 PointPurposes = 0xB;
};
