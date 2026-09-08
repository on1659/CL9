// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9ModuleDef.h"

/**
 * 사이트 배치를 담당하는 순수 로직 (docs/plan/data/module.md · docs/systems/map.md).
 * 컨테이너와 수학 연산 외에는 엔진에 손대지 않으므로, 같은 시드는 모든 머신에서 같은
 * 배치를 만들어낸다 — 호스트는 시드 하나만 복제하고 각 클라이언트가 이걸 로컬에서
 * 실행한다. ACL9SiteMap이 그 결과를 액터로 바꾼다.
 *
 * **사이트 전체가 모듈 격자다** — 축사 안쪽만이 아니라 마당까지 전부. 모듈은 프리팹이
 * 아니라 표(UCL9ModuleDef)의 행이고, 이 코드가 하는 일은 "어느 칸에 어느 모듈이
 * 어느 방향으로 앉는가"와 "어느 칸 경계가 벽이고 어느 쪽이 문인가"를 정하는 것까지다.
 * 그 발자국을 어떤 메시로 채울지는 조각 표(UCL9PieceDef)가 정한다.
 *
 * 좌표는 사이트 로컬 센티미터 단위다: (0,0)이 트럭 패드이고, +X가 격자 안쪽(동쪽)을
 * 향하며 +Y는 북쪽이다. **층이 없다** — 높이차는 모듈 안의 계단으로 내고 이웃으로
 * 전파되지 않는다 (module.md 결정 2).
 */

/** 격자 변 하나의 상태. 변은 칸 경계이며, 모듈 안쪽 경계는 Open이다 */
enum class ECL9GridEdge : uint8
{
	/** 뚫려 있다 — 같은 모듈 안이거나 격자 바깥이다 */
	Open,
	/** 막혔다 — 모듈 외곽이거나, 이웃이 있어도 소켓이 안 맞은 자리다 */
	Wall,
	/** 두 모듈을 잇는 문 */
	Door,
	/** 일방향 비상구 (flapIn). 한 판에 하나뿐이다 */
	Flap
};

/** 사이트 행에서 복사해온 배치 설정값들 (docs/plan/data/module.md) */
struct FCL9SiteParams
{
	/** 모듈 표와 조각 표를 함께 가리키는 이름 (UCL9SiteDef::Theme) */
	FName Theme;

	/** 격자 칸 하나의 크기 (cm) */
	float CellSize = 500.f;

	/** 이 판에 놓을 모듈 수 — 사이트 기준값 × 일자 커브를 이미 반영한 값이다 */
	int32 ModuleCount = 24;

	/** 트럭 패드에서 시작 모듈 바깥면까지 (cm) */
	float EntryOffset = 1000.f;

	/**
	 * 벽 두께. 표 컬럼이 아니라 ACL9SiteMap이 `barn` 종류가 쓰는 조각의 실측 두께로
	 * 채워 준다 (docs/systems/asset-swap.md) — 여기 적힌 값은 조각을 아직 못 읽었을
	 * 때의 폴백이다.
	 */
	float WallThickness = 30.f;

	/**
	 * 문 하나의 폭 (cm). 이것도 표가 아니라 `door` 종류가 쓰는 조각의 실측 폭에서
	 * 온다 — 팩을 갈아 끼우면 문 크기가 그 팩 문짝을 따라간다.
	 */
	float DoorWidth = 250.f;
};

/** 격자에 앉은 모듈 하나 */
struct FCL9ModulePlacement
{
	/** 이 자리에 앉은 표의 행 */
	const UCL9ModuleDef* Def = nullptr;

	/** 차지한 칸 사각형의 최소 모서리 */
	FIntPoint Origin = FIntPoint::ZeroValue;

	/** 회전을 적용한 뒤의 칸 크기 */
	FIntPoint Size = FIntPoint(1, 1);

	/** 반시계 90° 단위 회전 (0~3) */
	int32 Rotation = 0;

	/** 회전을 적용한 뒤 열려 있는 변 (ECL9ModuleSide 비트) */
	uint8 Sockets = 0;

	/** 시작 모듈에서 몇 개의 모듈을 거쳐 왔나 — 멀수록 깊은 곳이다 */
	int32 Hops = 0;
};

/**
 * 스폰·순찰이 쓰는 지점 하나 (module.md 결정 4, REPO의 `LevelPoint`에 해당).
 * 지금은 배치된 칸에서 파생해 만들지만, 나중에 소품 세트가 "이 선반 위" 같은 자기
 * 지점을 밀어 넣는 자리도 여기다 — 그래서 칸이 아니라 별도 목록으로 둔다.
 */
struct FCL9LevelPoint
{
	/** 사이트 로컬 cm */
	FVector2D Local = FVector2D::ZeroVector;

	/** ECL9PointPurpose 비트마스크 */
	uint8 Purposes = 0;

	/** 이 지점이 속한 모듈 */
	int32 Module = INDEX_NONE;

	/** 시작 모듈로부터의 홉 수 — 그 모듈의 값을 복사해 둔 것이다 */
	int32 Hops = 0;

	bool Has(ECL9PointPurpose Purpose) const
	{
		return (Purposes & static_cast<uint8>(Purpose)) != 0;
	}
};

struct FCL9SiteLayout
{
	uint32 Seed = 0;

	/** 시드 0이거나 쓸 모듈이 없으면 false — 그 경우 아무것도 짓지 않는다 */
	bool bBuilt = false;

	/** 격자 칸 수 */
	FIntPoint GridSize = FIntPoint::ZeroValue;

	/** 칸 (0,0)의 최소 모서리가 놓이는 사이트 로컬 좌표 */
	FVector2D Origin = FVector2D::ZeroVector;

	/** GridSize.X * GridSize.Y — 그 칸을 차지한 모듈 인덱스, 빈 칸은 INDEX_NONE */
	TArray<int32> CellModule;

	/** (GridSize.X+1) * GridSize.Y — 칸 (gx-1,gy)와 (gx,gy) 사이의 세로 변 */
	TArray<uint8> EdgeX;

	/** GridSize.X * (GridSize.Y+1) — 칸 (gx,gy-1)와 (gx,gy) 사이의 가로 변 */
	TArray<uint8> EdgeY;

	TArray<FCL9ModulePlacement> Modules;

	TArray<FCL9LevelPoint> Points;

	/** 트럭이 붙는 칸 — 시작 모듈의 서쪽 끝이다 */
	FIntPoint EntryCell = FIntPoint::ZeroValue;

	bool IsInside(FIntPoint Cell) const
	{
		return Cell.X >= 0 && Cell.Y >= 0 && Cell.X < GridSize.X && Cell.Y < GridSize.Y;
	}

	int32 CellAt(FIntPoint Cell) const
	{
		return IsInside(Cell) ? CellModule[Cell.X + Cell.Y * GridSize.X] : INDEX_NONE;
	}

	/** 칸 (gx-1,gy)와 (gx,gy) 사이의 세로 변 — gx는 0 ~ GridSize.X */
	ECL9GridEdge EdgeAtX(int32 GX, int32 GY) const
	{
		return static_cast<ECL9GridEdge>(EdgeX[GX + GY * (GridSize.X + 1)]);
	}

	/** 칸 (gx,gy-1)와 (gx,gy) 사이의 가로 변 — gy는 0 ~ GridSize.Y */
	ECL9GridEdge EdgeAtY(int32 GX, int32 GY) const
	{
		return static_cast<ECL9GridEdge>(EdgeY[GX + GY * GridSize.X]);
	}
};

/** 배치할 박스 하나 — kind 이름이 seg kind 테이블을 인덱싱한다(docs/plan/data/segkind.md) */
struct FCL9SiteSeg
{
	FName Kind;

	/** 사이트 로컬 cm 단위의 바닥 면적 */
	FBox2D Rect;

	/**
	 * 이 박스가 기준으로 삼는 Z. 벽은 여기서 위로, 바닥은 여기서 아래로, 천장은
	 * 여기서 위로 자란다 — 어느 쪽인지는 seg kind가 정한다.
	 *
	 * **지금은 바닥 종류가 언제나 0이다** (module.md 결정 2: 높이차는 모듈 밖으로
	 * 전파되지 않는다). 언젠가 진짜 층이 필요해지면 여기에 0이 아닌 값을 넣는 것이
	 * 착수 지점이 된다.
	 */
	float BaseZ = 0.f;

	/**
	 * 이 박스가 바깥으로 보는 방향, 사이트 로컬 XY 단위벡터. 방향이 뜻을 갖는 종류만
	 * 채운다 — 지금은 비상구뿐이고, 여닫이문이 어느 쪽으로만 열릴지를 이걸로 정한다.
	 */
	FVector2D Outward = FVector2D::ZeroVector;
};

namespace CL9SegKind
{
	inline const FName& Wall() { static const FName Name(TEXT("wall")); return Name; }
	inline const FName& Barn() { static const FName Name(TEXT("barn")); return Name; }
	inline const FName& Door() { static const FName Name(TEXT("door")); return Name; }
	inline const FName& FlapIn() { static const FName Name(TEXT("flapIn")); return Name; }
	inline const FName& Floor() { static const FName Name(TEXT("floor")); return Name; }
	inline const FName& Ceiling() { static const FName Name(TEXT("ceiling")); return Name; }
}

namespace CL9SiteLayout
{
	/**
	 * 모듈을 조립해 한 판을 만든다. 시드가 0이거나 쓸 모듈이 없으면 아무것도 짓지
	 * 않은 배치를 돌려준다(bBuilt == false). Modules는 레지스트리 순서 그대로여야
	 * 한다 — 순서가 곧 결정성의 일부다.
	 */
	FCL9SiteLayout Generate(const FCL9SiteParams& Params, const TArray<const UCL9ModuleDef*>& Modules, uint32 Seed);

	/** 맵 액터가 배치할 모든 박스 — 바닥, 천장, 벽, 문틀, 비상구 */
	void BuildSegments(const FCL9SiteParams& Params, const FCL9SiteLayout& Layout, TArray<FCL9SiteSeg>& OutSegs);

	/** 칸 하나의 중심, 사이트 로컬 좌표 */
	FVector2D CellCenter(const FCL9SiteParams& Params, const FCL9SiteLayout& Layout, FIntPoint Cell);

	/**
	 * 그 용도로 쓸 수 있는 지점들의 인덱스를, **시작 모듈에서 먼 것부터** 담는다
	 * (module.md 결정 4). 몬스터는 앞쪽에서, 재료는 전체에서 고르면 "깊이 들어갈수록
	 * 위험하다"가 배치만으로 성립한다.
	 */
	void PointsOfPurpose(const FCL9SiteLayout& Layout, ECL9PointPurpose Purpose, TArray<int32>& OutIndices);

	/** 문만으로 모든 모듈이 서로 도달 가능하면 true */
	bool IsConnected(const FCL9SiteLayout& Layout);

	/** 배치 바이트에 대한 FNV-1a 해시 — 모든 머신에서 로그로 남겨서 불일치가 생기면 값이 서로 달라져 드러난다 */
	uint32 LayoutHash(const FCL9SiteLayout& Layout);
}
