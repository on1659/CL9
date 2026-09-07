// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * 양계장 맵 생성을 담당하는 순수 로직(docs/systems/map.md · docs/plan/data/site.md).
 * 컨테이너와 수학 연산 외에는 엔진에 손대지 않으므로, 같은 시드는 모든 머신에서
 * 같은 배치를 만들어낸다 — 호스트는 시드 하나만 복제하고 각 클라이언트가 이걸
 * 로컬에서 실행한다. ACL9SiteMap이 그 결과를 액터로 바꾼다.
 *
 * 좌표는 사이트 로컬 센티미터 단위다: (0,0)이 트럭 패드이고, +X는 동쪽으로 축사를
 * 향하며, +Y는 통로를 가로지른다. 층 0은 마당/1층 레벨이고, 층 1은 그 아래 FloorDrop
 * 만큼 내려간 지하다.
 */

/** 사이트 테이블 행에서 복사해온 격자 설정값들(site.md 맵 생성) */
struct FCL9HenhouseParams
{
	int32 AisleCount1F = 10;
	int32 ColCount1F = 16;
	int32 AisleCountB1 = 8;
	int32 ColCountB1 = 12;
	float AisleWidth = 250.f;
	float ColWidth = 500.f;
	float RowDepth = 120.f;
	float OpenWidth = 200.f;
	float PlugWidth = 180.f;
	int32 TargetOpen1F = 175;
	int32 TargetOpenB1 = 105;
	int32 JarCount1F = 7;
	int32 JarCountB1 = 6;

	/**
	 * 벽 두께. 표 컬럼이 아니라 ACL9SiteMap이 `barn` 종류가 쓰는 조각의 실측
	 * 두께로 채워 준다 (docs/systems/asset-swap.md) — 여기 적힌 값은 조각을
	 * 아직 못 읽었을 때의 폴백이다. 벽을 세울 조각이 40cm면 프레임도 40cm를
	 * 비워야 통로 폭이 설계대로 나온다.
	 */
	float WallThickness = 30.f;
};

/** 한 층의 미로 — 간선 값이 1이면 통행 가능하다. 셀은 (통로, 열)로 표현한다. */
struct FCL9FloorLayout
{
	int32 Aisles = 0;
	int32 Cols = 0;

	/** Aisles × (Cols-1) 크기: 통로 a에서 열 c와 c+1 사이 — 값이 0이면 그 지점을 플러그가 막고 있다 */
	TArray<uint8> OpenH;

	/** (Aisles-1) × Cols 크기: 케이지 열 g(통로 g와 g+1 사이)를 열 c에서 관통 — 값이 1이면 통로가 열려 있다 */
	TArray<uint8> OpenV;

	/** 회수용 항아리 위치, (통로, 열) — 가장 먼 곳부터 배치해서 모든 셀 근처에 하나씩 두게 한다(map.md 항아리) */
	TArray<FIntPoint> JarCells;

	bool IsOpenH(int32 Aisle, int32 Col) const { return OpenH[Aisle * (Cols - 1) + Col] != 0; }
	bool IsOpenV(int32 Row, int32 Col) const { return OpenV[Row * Cols + Col] != 0; }
	int32 CountOpen() const;
	int32 CellCount() const { return Aisles * Cols; }
};

struct FCL9SiteLayout
{
	uint32 Seed = 0;

	/** 시드 0일 때는 false — 고정된 껍데기(로비)만 있고 미로는 없다 */
	bool bHasMaze = false;

	/** [0] = 1층, [1] = 지하 1층 */
	FCL9FloorLayout Floors[2];

	/** 동쪽 벽에 축사 밖으로 나가는 일방향 여닫이문이 달린 1층 통로(미로가 없으면 -1) */
	int32 EastHoleAisle = -1;
};

/**
 * 시드와 무관하게 고정되는 뼈대(site.md: 마당·트럭·계단·축사 외곽은 고정) — 축사,
 * 계단 통로, 경계가 놓이는 위치다. 격자 설정값에서 파생된다.
 */
struct FCL9SiteFrame
{
	/** 벽 두께 — ComputeFrame이 FCL9HenhouseParams::WallThickness에서 복사한다 */
	float Wall = 30.f;

	/** 지하 1층이 1층보다 얼마나 아래에 있는지 */
	float FloorDrop = 450.f;

	/** 경사로 앞뒤의 평지 구간, 그리고 경사로 자체의 수평 길이 */
	float Landing = 150.f;
	float Ramp = 600.f;

	/** 1층 동쪽 벽에 뚫린 일방향 여닫이문 개구부 */
	float HoleWidth = 180.f;

	/** 각 층 격자의 내부 영역 */
	FBox2D Barn[2];

	/** 각 층에서 Y = 0에 중심을 둔 통로 — 1층 문과 계단 통로가 여기에 맞춰 정렬된다 */
	int32 Spine[2] = {0, 0};

	/** 계단 통로의 X 범위 — 1층 동쪽 벽 바깥면부터 지하 1층 서쪽 벽 바깥면까지 */
	float StairX0 = 0.f;
	float StairX1 = 0.f;

	/** 경계 벽 */
	FBox2D World;

	float FloorBase(int32 Floor) const { return Floor == 0 ? 0.f : -FloorDrop; }
};

/** 배치할 박스 하나 — kind 이름이 seg kind 테이블을 인덱싱한다(docs/plan/data/segkind.md) */
struct FCL9SiteSeg
{
	FName Kind;

	/** 이 박스가 바닥을 딛고 서 있는 층 */
	int32 Floor = 0;

	/** 사이트 로컬 cm 단위의 바닥 면적 */
	FBox2D Rect;

	/** Rect.Min.X(층 0 레벨)에서 Rect.Max.X(층 1 레벨)로 내려가는 경사 바닥 슬래브 */
	bool bRamp = false;
};

namespace CL9SegKind
{
	inline const FName& Wall() { static const FName Name(TEXT("wall")); return Name; }
	inline const FName& Barn() { static const FName Name(TEXT("barn")); return Name; }
	inline const FName& Cage() { static const FName Name(TEXT("cage")); return Name; }
	inline const FName& Plug() { static const FName Name(TEXT("plug")); return Name; }
	inline const FName& Stair() { static const FName Name(TEXT("stair")); return Name; }
	inline const FName& FlapIn() { static const FName Name(TEXT("flapIn")); return Name; }
	inline const FName& Floor() { static const FName Name(TEXT("floor")); return Name; }
	inline const FName& Ceiling() { static const FName Name(TEXT("ceiling")); return Name; }
}

namespace CL9SiteLayout
{
	FCL9SiteFrame ComputeFrame(const FCL9HenhouseParams& Params);

	/** 시드 0이면 껍데기만 만들고, 그 외에는 두 층 모두 목표 개방 수를 만족하는 연결된 미로를 만든다 */
	FCL9SiteLayout Generate(const FCL9HenhouseParams& Params, uint32 Seed);

	/** 맵 액터가 배치할 모든 박스 — 껍데기, 슬래브, 계단, 그리고 있다면 미로까지 */
	void BuildSegments(const FCL9HenhouseParams& Params, const FCL9SiteFrame& Frame, const FCL9SiteLayout& Layout, TArray<FCL9SiteSeg>& OutSegs);

	/**
	 * 각 층 천장 슬래브가 놓이는 위치: 1층은 그 층 바닥 위 축사 높이에, 지하 1층은
	 * 지면 높이에 두어 마당이 지하의 지붕 역할을 하게 한다. 벽 Height 컬럼과는
	 * 의도적으로 다른 숫자다(segkind.md 높이 210은 천장이 아니다).
	 */
	float CeilingBase(const FCL9SiteFrame& Frame, int32 Floor);

	/** 셀이 속한 통로 구간의 중심, 사이트 로컬 좌표(Z 없음) */
	FVector2D CellCenter(const FCL9HenhouseParams& Params, const FCL9SiteFrame& Frame, int32 Floor, int32 Aisle, int32 Col);

	/** 열린 간선만으로 모든 셀이 서로 도달 가능하면 true */
	bool IsConnected(const FCL9FloorLayout& Floor);

	/** 미로 바이트에 대한 FNV-1a 해시 — 모든 머신에서 로그로 남겨서 불일치가 생기면 값이 서로 달라져 드러난다 */
	uint32 LayoutHash(const FCL9SiteLayout& Layout);
}
