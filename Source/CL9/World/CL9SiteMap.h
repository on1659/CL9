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
 * 생성된 사이트를 액터로 구현한 것(docs/systems/map.md 생성 흐름). 월드마다 하나씩만
 * 존재하며 절대 복제되지 않는다 — 모든 머신이 게임 스테이트의 시드로부터 각자
 * 다시 만들어내기 때문에, 벽 하나 전송하지 않고도 호스트와 클라이언트가 같은
 * 배치 안에 서게 된다. 박스는 seg kind 테이블에서 읽어온 대로 seg kind별 인스턴스
 * 메시로 그려지고, 일방향 여닫이문만 별도의 액터다. 트럭 패드를 기준점으로 삼고
 * 지면은 Z = 0이다.
 */
UCLASS(NotBlueprintable, NotPlaceable)
class ACL9SiteMap : public AActor
{
	GENERATED_BODY()

public:
	ACL9SiteMap();

	/** 이 월드의 사이트맵, 아직 처음 빌드되지 않았다면 null */
	static ACL9SiteMap* Find(UWorld* World);

	/**
	 * 주어진 시드로 사이트를 빌드(또는 재빌드)한다 — 0이면 아무것도 짓지 않는다.
	 * Day는 한 판의 모듈 수를 정하는 데만 쓰인다(docs/plan/data/module.md 결정 5).
	 * 에디터 월드에서는 임시(transient)·에디터 전용 액터로 만들어서 맵이 레벨에
	 * 저장되지 않도록 한다(build-order 5a 맵 배치 툴).
	 */
	static ACL9SiteMap* Rebuild(UWorld* World, const UCL9SiteDef& Site, const UCL9DefRegistry& Registry, uint32 Seed, int32 Day);

	/** 월드에서 사이트맵을 제거한다(있는 경우) */
	static void Clear(UWorld* World);

	const FCL9SiteLayout& GetLayout() const { return Layout; }
	const FCL9SiteParams& GetParams() const { return Params; }

	/** 스폰이 고를 수 있는 지점들 (docs/plan/data/module.md 결정 4) */
	const TArray<FCL9LevelPoint>& GetPoints() const { return Layout.Points; }

	/** 지점 하나의 월드 좌표 — 그 모듈의 바닥 높이 위다 */
	FVector PointToWorld(const FCL9LevelPoint& Point) const;

	/** 사이트 로컬 XY 좌표를 월드 좌표로 변환한다 */
	FVector SiteToWorld(const FVector2D& Local) const;

private:
	void Build(const UCL9SiteDef& Site, const UCL9DefRegistry& Registry, uint32 Seed, int32 Day);
	void ClearBuilt();

	/**
	 * 조각의 실측 치수를 파라미터에 채워 넣는다 — 벽 두께는 `barn` 조각의 얇은 축,
	 * 문 폭은 `door` 조각의 넓은 축이다. 손으로 옮겨 적은 숫자는 팩이 바뀌는 순간
	 * 조용히 틀린 값이 된다 (docs/systems/asset-swap.md).
	 */
	void MeasurePieces(const UCL9SiteDef& Site, const UCL9DefRegistry& Registry);

	/**
	 * 모듈마다 소품 세트 하나를 뽑아 깐다 (module.md 결정 7 + REPO의 변 연결 조건). 문 자리와
	 * 스폰 지점을 몸 있는 소품이 가리면 경고한다. 놓은 인스턴스 수를 돌려준다.
	 */
	int32 PlaceProps(const TArray<FCL9SiteSeg>& Segs, int32 FirstComponentIndex);

	/** seg kind별로 하나씩 두는 인스턴스 컴포넌트 — 빌드 중 필요할 때 생성된다 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> KindComponents;

	/** 여닫이문처럼 액터로 만들 수밖에 없었던 것들 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> SpawnedActors;

	FCL9SiteParams Params;
	FCL9SiteLayout Layout;
};
