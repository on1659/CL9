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
 * 미로 안에 서게 된다. 박스는 seg kind 테이블에서 읽어온 대로 seg kind별 인스턴스
 * 메시로 그려지고, 일방향 여닫이문만 별도의 액터다. 트럭 패드를 기준점으로 삼고
 * 지면은 Z = 0이다(평지 테스트 레벨의 바닥 — ⑤ 5b에서는 지형 기준점으로 대체된다).
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
	 * 주어진 시드로 사이트를 빌드(또는 재빌드)한다 — 0이면 고정된 껍데기만 놓는다.
	 * 에디터 월드에서는 임시(transient)·에디터 전용 액터로 만들어서 맵이 레벨에
	 * 저장되지 않도록 한다(build-order 5a 맵 배치 툴).
	 */
	static ACL9SiteMap* Rebuild(UWorld* World, const UCL9SiteDef& Site, const UCL9DefRegistry& Registry, uint32 Seed);

	/** 월드에서 사이트맵을 제거한다(있는 경우) */
	static void Clear(UWorld* World);

	const FCL9SiteLayout& GetLayout() const { return Layout; }
	const FCL9HenhouseParams& GetParams() const { return Params; }
	const FCL9SiteFrame& GetFrame() const { return Frame; }

	/** 셀이 속한 통로 구간 중심의, 그 층 바닥 위 월드 좌표 */
	FVector CellToWorld(int32 Floor, int32 Aisle, int32 Col) const;

	/** 한 층 위 사이트 로컬 XY 좌표를 월드 좌표로 변환한다 */
	FVector SiteToWorld(const FVector2D& Local, int32 Floor) const;

private:
	void Build(const UCL9SiteDef& Site, const UCL9DefRegistry& Registry, uint32 Seed);
	void ClearBuilt();

	/** seg kind별로 하나씩 두는 인스턴스 컴포넌트 — 빌드 중 필요할 때 생성된다 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UInstancedStaticMeshComponent>> KindComponents;

	/** 여닫이문처럼 액터로 만들 수밖에 없었던 것들 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> SpawnedActors;

	FCL9HenhouseParams Params;
	FCL9SiteFrame Frame;
	FCL9SiteLayout Layout;
};
