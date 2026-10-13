// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Data/CL9SiteDef.h"
#include "GameFramework/Actor.h"
#include "World/CL9SiteLayout.h"
#include "CL9SiteMap.generated.h"

class UCL9DefRegistry;
class UCL9SiteDef;
class UInstancedStaticMeshComponent;
class UPointLightComponent;

/**
 * 생성된 사이트를 액터로 구현한 것(docs/systems/map.md 생성 흐름). 단일 사이트는 월드에 하나,
 * 구역이 있는 사이트(UCL9SiteDef::Districts)는 구역마다 하나씩 존재하며 절대 복제되지 않는다 —
 * 모든 머신이 게임 스테이트의 시드로부터 각자 다시 만들어내기 때문에, 벽 하나 전송하지 않고도
 * 호스트와 클라이언트가 같은 배치 안에 서게 된다. 박스는 seg kind 테이블에서 읽어온 대로 seg kind별
 * 인스턴스 메시로 그려지고, 일방향 여닫이문만 별도의 액터다. 트럭 패드를 기준점으로 삼고(구역은 거기에
 * Offset·Yaw 를 더한다) 지면은 Z = 0이다.
 */
UCLASS(NotBlueprintable, NotPlaceable)
class ACL9SiteMap : public AActor
{
	GENERATED_BODY()

public:
	ACL9SiteMap();

	/** 정전이면 시설 조명을 끈다 — 판정은 게임 스테이트, 여기는 표현만 (docs/systems/light.md 정전) */
	virtual void Tick(float DeltaSeconds) override;

	/** 파괴·판 종료 때 구역 나눠 짓기 티커를 내린다 */
	virtual void Destroyed() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 이 월드의 사이트맵, 아직 처음 빌드되지 않았다면 null — 구역이 여럿이면 구역 0 */
	static ACL9SiteMap* Find(UWorld* World);

	/** 이 월드의 사이트맵 전부, 구역 인덱스 순. 단일 사이트면 하나다 */
	static void FindAll(UWorld* World, TArray<ACL9SiteMap*>& OutMaps);

	/**
	 * 프레임 나눠 짓기가 아직 안 끝났으면 true — 구역이 여럿이면 Rebuild 가 프레임마다 하나씩 짓는다.
	 * 밤 시작은 이것이 false 가 될 때까지 기다린다 (docs/goal/giant-stage.md Gate answers).
	 */
	static bool IsBuildPending(UWorld* World);

	/**
	 * 주어진 시드로 사이트를 빌드(또는 재빌드)한다 — 0이면 아무것도 짓지 않는다.
	 * Day는 한 판의 모듈 수를 정하는 데만 쓰인다(docs/plan/data/module.md 결정 5).
	 * 에디터 월드에서는 임시(transient)·에디터 전용 액터로 만들어서 맵이 레벨에
	 * 저장되지 않도록 한다(build-order 5a 맵 배치 툴).
	 *
	 * 사이트에 구역이 있으면 구역마다 액터를 하나씩 세우고, **이 호출에서 구역 0 만 짓고 나머지는 프레임마다
	 * 하나씩** 짓는다(사용자 결정 2026-10-04 "한꺼번에 짓지 않는다"). 돌려주는 것은 구역 0 이다.
	 */
	static ACL9SiteMap* Rebuild(UWorld* World, const UCL9SiteDef& Site, const UCL9DefRegistry& Registry, uint32 Seed, int32 Day);

	/** 월드에서 사이트맵을 제거한다(있는 경우) — 구역이 여럿이면 전부 */
	static void Clear(UWorld* World);

	const FCL9SiteLayout& GetLayout() const { return Layout; }
	const FCL9SiteParams& GetParams() const { return Params; }

	/** 스폰이 고를 수 있는 지점들 (docs/plan/data/module.md 결정 4) */
	const TArray<FCL9LevelPoint>& GetPoints() const { return Layout.Points; }

	/** 이 액터가 짓는 구역의 인덱스 (UCL9SiteDef::Districts) — 단일 사이트면 INDEX_NONE */
	int32 GetDistrictIndex() const { return DistrictIndex; }

	/** 이 판의 깊이 구역 — 구역에 Zones 가 있으면 그것, 비면 사이트의 Zones */
	const TArray<FCL9SiteZone>& GetZones() const { return Zones; }

	/** 지점 하나의 월드 좌표 — 그 모듈의 바닥 높이 위다 */
	FVector PointToWorld(const FCL9LevelPoint& Point) const;

	/** 사이트 로컬 XY 좌표를 월드 좌표로 변환한다 — 액터 변환(구역의 회전 포함)을 거친다 */
	FVector SiteToWorld(const FVector2D& Local) const;

	/** 호스트가 판 시작에 세울 고정물 하나 — 클래스, 월드 변환, 그 모듈(로그용) */
	struct FFixtureSpawn
	{
		TSoftClassPtr<AActor> ActorClass;
		FTransform Transform;
		int32 Module = INDEX_NONE;
		FName ModuleAlias;
	};

	/**
	 * 배치된 모듈마다 뽑힌 소품 세트의 고정물(`FCL9PropSet::Fixtures`)을 월드 변환으로 돌려준다 (docs/goal/loop-7c-kitchen-in-building.md).
	 * 세트는 소품을 그릴 때와 **같은 함수**로 고른다 — 소품과 고정물이 서로 다른 세트를 보는 일이 없다. 판이 없으면 비어 있다
	 */
	void GetFixtureSpawns(TArray<FFixtureSpawn>& OutSpawns) const;

private:
	void Build(const UCL9SiteDef& Site, const UCL9DefRegistry& Registry, uint32 Seed, int32 Day);
	void ClearBuilt();

	/** 대기 중인 구역 하나를 짓는다 — 맡겨 둔 사이트·레지스트리·시드·일자로 */
	void BuildPending();

	/**
	 * 구역 0 이 티커에서 부른다 — 이번 프레임에 아직 아무 구역도 안 지었으면 대기 중인 구역 하나를 짓는다.
	 * 더 지을 것이 남았으면 true (티커를 계속 둔다).
	 */
	bool BuildNextDistrict();

	/** 나눠 짓기 티커를 내린다 (있으면) */
	void StopDistrictTicker();

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

	/**
	 * 시설 조명 — 빛 값(`FCL9PropPlacement::LightIntensity`)이 있는 소품마다 하나. 인스턴스 메시는
	 * 빛을 못 품으므로 따로 둔다. 정전이면 꺼진다 (docs/systems/light.md 시설 조명, 2026-09-16).
	 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UPointLightComponent>> LampComponents;

	/** 깜박이는 램프 하나 — LampComponents 인덱스와 표의 진폭·속도, 램프마다 다른 노이즈 위상 */
	struct FFlickerLamp
	{
		int32 LampIndex = INDEX_NONE;
		float BaseIntensity = 0.f;
		float Amplitude = 0.f;
		float Speed = 0.f;
		float Phase = 0.f;
	};

	/** 깜박임 진폭이 있는 램프만 — Tick 이 이 목록만 돈다 */
	TArray<FFlickerLamp> FlickerLamps;

	/** 램프를 켜고 끈다 — 상태가 바뀔 때만 컴포넌트를 건드린다 */
	void SetLampsOn(bool bOn);

	/** 깜박이는 램프의 세기를 노이즈로 흔든다 — 켜져 있을 때만 */
	void TickFlicker();

	/** 마지막으로 램프에 적용한 켜짐 상태 */
	bool bLampsOn = true;

	/** UCL9SiteDef::Districts 인덱스 — 단일 사이트면 INDEX_NONE */
	int32 DistrictIndex = INDEX_NONE;

	/** 이 판의 깊이 구역 사본 — 빌드 때 구역 Zones(비면 사이트 Zones)에서 복사한다 */
	TArray<FCL9SiteZone> Zones;

	// -- 프레임 나눠 짓기 (구역이 있는 사이트만). 맡겨 둔 행은 지을 때까지 붙들어야 한다 — 치트의 독립
	//    레지스트리는 어디에도 루트가 없어서 다음 프레임 전에 GC 될 수 있다. 짓고 나면 놓는다.

	/** 아직 안 지은 구역인가 */
	bool bBuildPending = false;

	UPROPERTY(Transient)
	TObjectPtr<UCL9SiteDef> PendingSite;

	UPROPERTY(Transient)
	TObjectPtr<UCL9DefRegistry> PendingRegistry;

	/** 이 구역의 하위 시드 (CL9SiteDistrict::SubSeed) */
	uint32 PendingSeed = 0;

	int32 PendingDay = 1;

	/** 구역 0 만 쓴다 — 나눠 짓기 티커와 마지막으로 구역을 지은 프레임 */
	FTSTicker::FDelegateHandle DistrictTicker;
	uint64 LastDistrictBuildFrame = 0;

	FCL9SiteParams Params;
	FCL9SiteLayout Layout;
};
