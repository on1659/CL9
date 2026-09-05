// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Pure map generation for the henhouse (docs/systems/map.md · docs/plan/data/site.md).
 * Nothing here touches the engine beyond containers and math, so the same seed
 * yields the same layout on every machine — the host replicates only the seed
 * and each client runs this locally. ACL9SiteMap turns the output into actors.
 *
 * Coordinates are site-local centimetres: (0,0) is the truck pad, +X runs east
 * into the barn, +Y crosses the aisles. Floor 0 is the yard/1F level, floor 1
 * the basement FloorDrop below it.
 */

/** Grid knobs copied from the site row (site.md 맵 생성) */
struct FCL9HenhouseParams
{
	int32 AisleCount1F = 8;
	int32 ColCount1F = 6;
	int32 AisleCountB1 = 6;
	int32 ColCountB1 = 4;
	float AisleWidth = 255.f;
	float ColWidth = 900.f;
	float RowDepth = 120.f;
	float OpenWidth = 360.f;
	float PlugWidth = 180.f;
	int32 TargetOpen1F = 50;
	int32 TargetOpenB1 = 25;
	int32 JarCount1F = 4;
	int32 JarCountB1 = 3;
};

/** One floor's maze — an edge is 1 when passable. Cells are (aisle, column). */
struct FCL9FloorLayout
{
	int32 Aisles = 0;
	int32 Cols = 0;

	/** Aisles × (Cols-1): along aisle a between column c and c+1 — 0 means a plug blocks the aisle there */
	TArray<uint8> OpenH;

	/** (Aisles-1) × Cols: through cage row g (between aisle g and g+1) at column c — 1 means an opening */
	TArray<uint8> OpenV;

	/** Return jars, (aisle, column) — farthest-first so every cell has one nearby (map.md 항아리) */
	TArray<FIntPoint> JarCells;

	bool IsOpenH(int32 Aisle, int32 Col) const { return OpenH[Aisle * (Cols - 1) + Col] != 0; }
	bool IsOpenV(int32 Row, int32 Col) const { return OpenV[Row * Cols + Col] != 0; }
	int32 CountOpen() const;
	int32 CellCount() const { return Aisles * Cols; }
};

struct FCL9SiteLayout
{
	uint32 Seed = 0;

	/** False for seed 0 — the fixed shell only (lobby), no maze */
	bool bHasMaze = false;

	/** [0] = 1F, [1] = B1 */
	FCL9FloorLayout Floors[2];

	/** 1F aisle whose east wall carries the one-way flap out of the barn (-1 without a maze) */
	int32 EastHoleAisle = -1;
};

/**
 * The seed-independent frame (site.md: 마당·트럭·계단·축사 외곽은 고정) — where the
 * barns, the stair corridor and the boundary sit. Derived from the grid knobs.
 */
struct FCL9SiteFrame
{
	/** Wall thickness */
	float Wall = 30.f;

	/** How far B1 lies below 1F */
	float FloorDrop = 450.f;

	/** Flat run before and after the ramp, and the ramp's horizontal length */
	float Landing = 150.f;
	float Ramp = 600.f;

	/** The one-way flap opening in the 1F east wall */
	float HoleWidth = 180.f;

	/** Interior of each floor's grid */
	FBox2D Barn[2];

	/** Aisle centred on Y = 0 per floor — the 1F door and the stair corridor line up with it */
	int32 Spine[2] = {0, 0};

	/** Stair corridor X range, from the 1F east wall's outer face to the B1 west wall's outer face */
	float StairX0 = 0.f;
	float StairX1 = 0.f;

	/** Boundary walls */
	FBox2D World;

	float FloorBase(int32 Floor) const { return Floor == 0 ? 0.f : -FloorDrop; }
};

/** One box to place — kind names index the seg kind table (docs/plan/data/segkind.md) */
struct FCL9SiteSeg
{
	FName Kind;

	/** Floor whose base the box stands on */
	int32 Floor = 0;

	/** Footprint in site-local cm */
	FBox2D Rect;

	/** Floor slab sloping down from Rect.Min.X (floor 0 level) to Rect.Max.X (floor 1 level) */
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

	/** Seed 0 gives the shell only; otherwise both floors get a connected maze with the target openings */
	FCL9SiteLayout Generate(const FCL9HenhouseParams& Params, uint32 Seed);

	/** Every box the map actor places — shell, slabs, stairs, then the maze when present */
	void BuildSegments(const FCL9HenhouseParams& Params, const FCL9SiteFrame& Frame, const FCL9SiteLayout& Layout, TArray<FCL9SiteSeg>& OutSegs);

	/**
	 * Where a floor's ceiling slab rests: 1F at the barn height above its floor, B1 at
	 * ground level so the basement is roofed by the yard. A separate number from the
	 * wall Height column on purpose (segkind.md 높이 210은 천장이 아니다).
	 */
	float CeilingBase(const FCL9SiteFrame& Frame, int32 Floor);

	/** Centre of a cell's aisle stretch, site-local (no Z) */
	FVector2D CellCenter(const FCL9HenhouseParams& Params, const FCL9SiteFrame& Frame, int32 Floor, int32 Aisle, int32 Col);

	/** True when every cell reaches every other over open edges */
	bool IsConnected(const FCL9FloorLayout& Floor);

	/** FNV-1a over the maze bytes — logged on every machine so a divergence shows up as different numbers */
	uint32 LayoutHash(const FCL9SiteLayout& Layout);
}
