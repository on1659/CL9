// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/CL9SiteLayout.h"
#include "CL9SiteMap.generated.h"

class UCL9DefRegistry;
class UCL9SiteDef;
class UInstancedStaticMeshComponent;

/**
 * The generated site as actors (docs/systems/map.md 생성 흐름). One per world,
 * never replicated: every machine rebuilds it from the game state's seed, so
 * host and clients stand in the same maze without sending a single wall.
 * Boxes are instanced meshes per seg kind, read from the seg kind table; the
 * one-way flap is its own actor. Anchored on the truck pad, ground at Z = 0
 * (the flat test level's floor — a terrain anchor replaces this in ⑤ 5b).
 */
UCLASS(NotBlueprintable, NotPlaceable)
class ACL9SiteMap : public AActor
{
	GENERATED_BODY()

public:
	ACL9SiteMap();

	/** The world's site map, or null before the first build */
	static ACL9SiteMap* Find(UWorld* World);

	/**
	 * Builds (or rebuilds) the site for the seed — 0 lays the fixed shell only.
	 * Editor worlds get a transient, editor-only actor so the map never saves
	 * into the level (build-order 5a 맵 배치 툴).
	 */
	static ACL9SiteMap* Rebuild(UWorld* World, const UCL9SiteDef& Site, const UCL9DefRegistry& Registry, uint32 Seed);

	/** Removes the site map from the world, if any */
	static void Clear(UWorld* World);

	const FCL9SiteLayout& GetLayout() const { return Layout; }
	const FCL9HenhouseParams& GetParams() const { return Params; }
	const FCL9SiteFrame& GetFrame() const { return Frame; }

	/** World position on a cell's floor at the centre of its aisle stretch */
	FVector CellToWorld(int32 Floor, int32 Aisle, int32 Col) const;

	/** Site-local XY on a floor to world */
	FVector SiteToWorld(const FVector2D& Local, int32 Floor) const;

private:
	void Build(const UCL9SiteDef& Site, const UCL9DefRegistry& Registry, uint32 Seed);
	void ClearBuilt();

	/** One instanced component per seg kind, created on demand during a build */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> KindComponents;

	/** Flaps and anything else that had to be an actor */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> SpawnedActors;

	FCL9HenhouseParams Params;
	FCL9SiteFrame Frame;
	FCL9SiteLayout Layout;
};
