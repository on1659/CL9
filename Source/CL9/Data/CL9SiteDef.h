// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "Data/CL9Types.h"
#include "CL9SiteDef.generated.h"

class UMaterialInterface;
class UWorld;

/**
 * 밤 페이즈 하나의 조명 세팅 (docs/systems/light.md 렌더 세팅). 태양·하늘은 레벨에
 * 배치된 라이트의 원래 값에 **곱하는 배율**이고, 안개는 절대값이다 — 배율 1·색 흰색·
 * 안개 0이면 레벨을 손대지 않는 것과 같다. ACL9NightLighting이 페이즈에 맞춰 이 값으로
 * 보간해 간다.
 */
USTRUCT(BlueprintType)
struct FCL9LightingPhase
{
	GENERATED_BODY()

	/** 태양(DirectionalLight) 밝기 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float SunScale = 1.f;

	/** 태양 색에 곱하는 색조 — 흰색이면 원래 색, 밤엔 달빛 청색 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor SunColor = FLinearColor::White;

	/**
	 * 하늘(SkyLight) 밝기 배율. 레벨의 SkyLight는 실시간 캡처라 하늘 자체가 태양을 따라
	 * 어두워진다 — 그래서 밤엔 1보다 훨씬 큰 값(양계장 10)이 정상이다. 태양이 0이면(정전)
	 * 배율이 얼마든 하늘도 0이다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float SkyScale = 1.f;

	/** 하늘 색에 곱하는 색조 — "검정의 색"이다. 차가운 청색이면 어두워도 형태가 읽힌다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor SkyColor = FLinearColor::White;

	/**
	 * 닫힌 실내로 새어 드는 하늘빛의 비율 (Lumen "Skylight Leaking", 0~1). 루멘은 천장이
	 * 덮인 방을 완전한 검정으로 만들기 때문에, 이 값이 0이면 SkyScale을 아무리 올려도
	 * 축사 안은 캄캄하다. 1이면 하늘빛이 벽을 무시하고 실내에 고르게 깔린다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0"))
	float SkyLeak = 0.f;

	/** 안개(ExponentialHeightFog) 밀도 — 0이면 안개 없음. 손전등 빔이 공기 중에 보이게 하는 값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float FogDensity = 0.f;

	/** 안개 산란색 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor FogColor = FLinearColor::Black;

	/**
	 * 보이는 하늘(SkyAtmosphere) 밝기 배율 — 태양 배율과 별개다. 태양을 5%로 낮춰도 대기는 낮의
	 * 색으로 남아 문밖과 마당이 하얗게 타는데, 그것을 여기서 누른다 (2026-09-16). 1이면 레벨 그대로.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float SkyLuminance = 1.f;

	/**
	 * 이 페이즈의 하늘 재질 — 레벨의 하늘 구(`SM_SkySphere`)에 씌운다. 비면 레벨에 놓인 재질(대기를 그리는
	 * 낮 하늘) 그대로. 밤은 별이 있는 어두운 하늘로 바꾼다 (2026-09-18, "스카이박스가 밝다").
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UMaterialInterface> SkyMaterial;

	/** 볼류메트릭 구름을 보이나 — 밤에는 태양이 낮아도 구름이 하얗게 남아 끈다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bClouds = true;

	/**
	 * 하늘빛(SkyLight)이 볼류메트릭 안개를 밝히는 세기 (VolumetricScatteringIntensity, 1 = 엔진 기본). 밤 SkyScale 이
	 * 크면(양계장 5) 안개가 하늘빛을 받아 마당이 뿌옇게 밝아진다 — 밤엔 0.1 로 눌러 광원 둘레만 뿌옇게 남긴다 (2026-09-18).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float SkyScattering = 1.f;
};

/**
 * 깊이 구역 하나 (docs/plan/data/site.md 구역). 깊이는 입구에서 그 지점까지의 홉 거리를 판에서 가장 먼
 * 홉으로 나눈 값이다 — 0이 입구, 1이 가장 깊은 곳. 구역은 자기 띠 안의 스폰 지점에만 자기 몬스터와
 * 재료를 놓는다. 그래서 "깊이 들어갈수록 위험하고 값진 것이 많다"가 C++ 규칙이 아니라 표의 행이 된다
 * (2026-10-03 farm-stage).
 */
USTRUCT(BlueprintType)
struct FCL9SiteZone
{
	GENERATED_BODY()

	/** 로그와 문서에서 부르는 이름 (`outer` · `middle` · `deep`) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Name;

	/** 띠의 시작 깊이 (포함) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0"))
	float DepthMin = 0.f;

	/** 띠의 끝 깊이 (제외 — 단 1이면 가장 깊은 지점까지 포함) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0"))
	float DepthMax = 1.f;

	/** 이 띠에 사는 동물 — 사이트의 MonsterSpawns와 같은 규칙(기준값 × 일자 배율)으로 굴린다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FCL9SpawnEntry> MonsterSpawns;

	/** 이 띠에 널린 재료 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FCL9SpawnEntry> ItemSpawns;
};

/**
 * 구역 하나 (docs/goal/giant-stage.md Design Decisions). 사이트 하나가 트럭 마당 둘레의 건물 여러 채로 설 때,
 * 구역마다 `ACL9SiteMap` 액터 하나가 **고치지 않은 생성기**를 자기 하위 시드·모듈 수·월드 변환으로 돌린다.
 * 구역 액터의 원점은 그 구역의 "트럭 패드 자리"다 — 단일 사이트와 똑같이 거기서 +X 로 `EntryOffset` 만큼
 * 마당을 건너 입구(`yard_gate`)가 나고, 액터 회전(`Yaw`)이 그 전체를 돌린다.
 */
USTRUCT(BlueprintType)
struct FCL9SiteDistrict
{
	GENERATED_BODY()

	/** 구역 액터 원점의 위치 — 레벨의 트럭 패드 기준 cm (월드 축) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D Offset = FVector2D::ZeroVector;

	/** 구역 액터의 회전 (도) — 0·90·180·270 만. 입구가 트럭을 보게 고른다 (0 이면 입구가 −X 쪽을 본다) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0", ClampMax="270"))
	int32 Yaw = 0;

	/** 이 구역 한 판의 기준 모듈 수 — 사이트의 BaseModuleCount 대신 쓴다. 일자 배율(SiteSizeCurve)은 그대로 곱한다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="2"))
	int32 ModuleCount = 200;

	/** 이 구역의 깊이 구역 — 비면 사이트의 Zones 를 쓴다. 먼 구역에 무거운 행을 줄 때 채운다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FCL9SiteZone> Zones;
};

/**
 * 목적지 행 하나 (docs/plan/data/site.md). 사이트는 기준값을 준다 — 어떤
 * 동물과 재료가 몇 마리/몇 개 있는지 — 그리고 일자 커브(UCL9ProgressionDef)가
 * 밤이 시작될 때 그 값에 곱해진다. 맵 생성기를 위한 격자 컬럼들은
 * build-order ⑤에서 합류하며, 그 전까지는 평지 테스트 레벨이 트럭 주변에
 * 굴림값을 흩뿌린다.
 */
UCLASS()
class UCL9SiteDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	UCL9SiteDef();

	virtual void Validate(TArray<FString>& OutErrors) const override;

	/** 여기 사는 동물들 — 기준 마리수이며, MonsterCountCurve가 굴림값을 조정한다 (site.md 서식 동물) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawns")
	TArray<FCL9SpawnEntry> MonsterSpawns;

	/** 주변에 널려 있는 재료 — 기준 개수이며, ItemCountCurve가 굴림값을 조정한다 (site.md 산출 재료) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawns")
	TArray<FCL9SpawnEntry> ItemSpawns;

	/**
	 * 깊이 구역 — 비어 있으면 예전처럼 위 두 열만 쓴다(몬스터는 깊은 쪽 60%, 재료는 어디나). 행이 있으면
	 * 구역마다 자기 띠 안에 자기 몫을 놓고, 위 두 열은 그 위에 예전 방식대로 더해진다 (2026-10-03).
	 * 생성기가 없는 사이트(평지)에서는 띠를 가릴 수 없어 구역 몫이 트럭 주변 원환에 함께 떨어진다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spawns")
	TArray<FCL9SiteZone> Zones;

	/**
	 * 이 스테이지를 플레이하는 레벨. 게임 모드는 지금 열린 레벨과 같은 행을 골라 그 사이트로 밤을 치른다 —
	 * 그래서 스테이지 하나 추가 = 이 표의 행 하나 + 레벨 하나다. 어느 행도 안 맞으면 게임 모드 BP의
	 * DefaultSiteAlias로 떨어진다 (2026-10-03 farm-stage).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stage")
	TSoftObjectPtr<UWorld> Level;

	// -- 밤 시계 덮어쓰기 (site.md 밤 시계). 0이면 CL9Rules의 전역값을 쓴다. 큰 판은 들어갔다 나오는 데
	//    시간이 더 걸리므로 시계도 사이트의 성격이다 (2026-10-03).

	/** 밤 길이 (초) — 일자 곡선의 NightLength 배율은 이 값에도 곱해진다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night", meta=(ClampMin="0.0"))
	float NightLength = 0.f;

	/** 정전까지 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night", meta=(ClampMin="0.0"))
	float DarkBase = 0.f;

	/** 반납이 정전을 앞당겨도 내려가지 않는 바닥 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night", meta=(ClampMin="0.0"))
	float DarkMin = 0.f;

	/** 덮어쓴 값이 있으면 그것, 없으면 전역값 */
	static float OrRule(float SiteValue, float RuleValue) { return SiteValue > 0.f ? SiteValue : RuleValue; }

	/** 일자 커브가 곱하는 위험도 기준값 — 마리수가 아니라 개체당 압박감이다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress")
	float BaseDanger = 1.f;

	/** 이 사이트를 선택할 수 있게 되는 첫 날 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress")
	int32 UnlockDay = 1;

	/**
	 * 여기서 가져온 물건의 판매가에 곱해지는 목적지 배율 (docs/plan/economy.md 가격식).
	 * 먼 곳일수록 값이 나가게 만드는 손잡이다 — 일자 배율과 동시에 만지지 않는다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Progress")
	float ValueRate = 1.f;

	// -- 맵 생성 (docs/plan/data/module.md). 사이트는 "얼마나 큰 판인가"만 정하고,
	//    무엇이 놓이는지는 모듈 표가, 그 모듈을 무엇으로 세우는지는 조각 표가 정한다.

	/** 이 사이트를 어떤 생성기로 배치할지 — None이면 평지 레벨을 그대로 둔다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	ECL9Generator Generator = ECL9Generator::None;

	/**
	 * 이 사이트를 어느 묶음으로 지을지. **두 표를 한 이름으로 가리킨다** — 모듈 표
	 * (UCL9ModuleDef)의 SubAlias이자 조각 표(UCL9PieceDef)의 SubAlias다
	 * (docs/systems/asset-swap.md · docs/plan/data/module.md). 조각 쪽은 테마에 없는
	 * 역할이 테마 칸이 빈 공용 행으로 떨어지지만, 모듈 쪽은 폴백이 없다 — 스테이지의
	 * 모듈 세트는 통째로 한 벌이다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	FName Theme;

	/**
	 * 격자 칸 하나의 크기 (cm). 조각 한 장과 같아야 벽이 안 늘어난다 — 보유 팩의
	 * 모듈이 500이라 500이다 (docs/systems/asset-swap.md). 팩을 갈아 끼울 때 여기와
	 * 모듈 표의 Size가 함께 움직인다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	float CellSize = 500.f;

	/**
	 * 한 판에 놓을 모듈 수의 기준값. 진행 표의 SiteSizeCurve가 여기에 곱해져서
	 * 1일차 20 · 5일차 24 · 10일차 30이 나온다 (module.md 결정 5).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	int32 BaseModuleCount = 24;

	/** 트럭 패드에서 시작 모듈의 바깥면까지의 거리 (cm) — 그 사이가 걸어 들어가는 마당이다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	float EntryOffset = 1000.f;

	/**
	 * 이 목적지가 세울 수 있는 최대 층수. 1이면 단층이고, 3이면 3층까지 올라간다.
	 * **상한일 뿐이다** — 위층은 아래층이 받쳐 주는 자리에만 설 수 있어서, 아래층이
	 * 좁게 나온 판은 여기 적은 것보다 낮게 끝난다 (docs/plan/data/module.md 층).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map", meta=(ClampMin="1", ClampMax="8"))
	int32 MaxFloors = 1;

	/**
	 * 층과 층을 잇는 계단 모듈의 개수 범위. 층 사이마다 시드가 이 안에서 하나를 뽑는다 —
	 * 그래서 같은 목적지라도 판마다 계단 수가 다르다. MaxFloors가 1이면 쓰이지 않는다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map", meta=(ClampMin="1"))
	int32 StairCountMin = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map", meta=(ClampMin="1"))
	int32 StairCountMax = 4;

	/**
	 * 위층 하나가 받는 모듈 예산 — 바로 아래층 예산에 곱한다. 0.5면 위층이 아래층의 절반이고,
	 * 층이 더 있으면 또 그 절반이다. 위층은 아래층이 받쳐 주는 자리에만 서므로 1에 가까워도
	 * 다 서지는 않는다 (2026-09-16, 2층이 계단을 올라가자마자 끝나는 문제로 컬럼화 — 양계장 0.75 초안).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map", meta=(ClampMin="0.0", ClampMax="1.0"))
	float UpperFloorShare = 0.5f;

	/**
	 * 시설 조명 밀도 (0~1) — 빛 값이 있는 소품 가운데 실제로 켜지는 비율. 소품(작업등 몸체)은 그대로 서고
	 * 빛만 시드로 골라 뺀다 — 리썰처럼 불 꺼진 등이 섞인 복도가 된다 (2026-09-18, "조명 개수가 너무 많다").
	 * 1이면 전부 켜진다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map", meta=(ClampMin="0.0", ClampMax="1.0"))
	float LampDensity = 1.f;

	/**
	 * 실내 모듈에 천장을 씌우나. 끄면 위에서 맵 속이 들여다보인다 — 배치를 눈으로
	 * 확인할 때 쓰라고 둔 스위치다 (2026-09-10, 그전에는 seg kind 이름을
	 * `__ceiling_off__`로 바꿔서 껐다).
	 *
	 * **끈다고 위층이 무너지지 않는다** — 층마다 자기 바닥 판을 깔기 때문에 천장이
	 * 없어도 2층은 그대로 밟힌다. 야외 모듈(`bOutdoor`)은 이 값과 무관하게 언제나
	 * 천장이 없다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	bool bBuildCeilings = true;

	/**
	 * 이미 놓인 모듈끼리 맞닿았을 때 문을 뚫을 확률 (0~1). 여기서 고리(우회로)가 생긴다 —
	 * DunGen의 `DoorwayConnectionChance`에 해당한다. 0이면 순수한 나무 구조가 되어 도망갈
	 * 길이 사라진다. (2026-09-15까지 생성기 상수 0.25였다.)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map", meta=(ClampMin="0.0", ClampMax="1.0"))
	float LoopChance = 0.25f;

	/**
	 * 전체 모듈 중 본줄기(입구에서 가장 깊은 자리로 한 줄기로 뻗는 부분)가 차지하는 비율.
	 * 나머지는 곁가지다 — 리썰컴퍼니는 곁가지가 훨씬 많다 (docs/plan/data/module.md 생성 알고리즘).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map", meta=(ClampMin="0.0", ClampMax="1.0"))
	float MainPathShare = 0.4f;

	/**
	 * 곁가지 예산에서 통로(Passage — 복도·코너·계단 통로)가 차지하는 비율. DeadEndShare와
	 * 합쳐 1을 넘지 않고, 둘을 뺀 나머지가 방(Normal)이다. 복도가 적어 방→문→방이 10m마다
	 * 반복되면 이 값을 올린다 (2026-09-15 양계장 0.28 → 0.40 초안).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map", meta=(ClampMin="0.0", ClampMax="1.0"))
	float PassageShare = 0.28f;

	/** 곁가지 예산에서 막다른 방(DeadEnd)이 차지하는 비율 — 곁가지의 끝을 맺는 방이다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map", meta=(ClampMin="0.0", ClampMax="1.0"))
	float DeadEndShare = 0.32f;

	/**
	 * Special 행(납품 방·주방) 하나를 한 판에 몇 번 놓나. 양계장은 1 — 판이 작아 하나로 충분하다. 5배 판에
	 * 납품 방(반납 항아리)이 하나뿐이면 깊은 곳에서 반납하러 판을 가로질러야 해서 늘린다 (2026-10-03 farm).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map", meta=(ClampMin="1", ClampMax="16"))
	int32 SpecialCopies = 1;

	/**
	 * 구역 — 비어 있으면 지금처럼 트럭 패드에 한 판(BaseModuleCount)을 짓는다. 행이 있으면 구역마다 한 판씩,
	 * 구역의 ModuleCount·Offset·Yaw 로 짓고 BaseModuleCount 는 쓰지 않는다. 나머지 맵 열(층·비율·SpecialCopies)은
	 * 구역마다 그대로 적용된다 (docs/goal/giant-stage.md, 2026-10-04).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	TArray<FCL9SiteDistrict> Districts;

	// -- 조명 (docs/systems/light.md). 어둠은 사이트의 성격이다 — 축사의 밤과 다른 목적지의
	//    밤이 다르게 보인다. 값은 레벨 라이트에 곱하는 배율이라 레벨을 바꿔도 그대로 쓴다.

	/** 로비·새벽 — 밤과 밤 사이의 어스름한 저녁 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lighting")
	FCL9LightingPhase LobbyLighting;

	/** 밤 — 습격 시간대. 손전등 밖은 형태만 겨우 읽히는 정도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lighting")
	FCL9LightingPhase NightLighting;

	/** 정전 뒤 — 손전등의 카운터. 여기서 앰비언트를 끄기 때문에 밤 값을 올려도 정전은 캄캄하다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Lighting")
	FCL9LightingPhase BlackoutLighting;
};

/** 구역 슬롯 검사가 받는 값 — 전부 부르는 쪽이 사이트 행·진행 표에서 꺼내 준다 */
struct FCL9DistrictSlotRules
{
	/** 격자 칸 크기 (cm) — UCL9SiteDef::CellSize */
	float CellSize = 0.f;

	/** 구역 원점에서 입구까지 (cm) — UCL9SiteDef::EntryOffset */
	float EntryOffset = 0.f;

	/** 검사할 날의 SiteSizeCurve 배율 — 슬롯은 10일차 판으로 잰다 */
	float SizeMult = 0.f;

	/** 밤 길이 (초) — UCL9SiteDef::NightLength */
	float NightLength = 0.f;

	/** 본줄기 비율 — UCL9SiteDef::MainPathShare */
	float MainPathShare = 0.f;

	/** 마당을 걷는 속도 (cm/s) — 왕복 마당 시간을 낸다 */
	float YardSpeed = 0.f;
};

/**
 * 구역의 순수 계산 (docs/goal/giant-stage.md). 엔진 상태를 안 건드리므로 테스트와 런타임이 같은 답을 낸다.
 * 좌표는 레벨 트럭 패드 기준 cm, 월드 축이다.
 */
namespace CL9SiteDistrict
{
	/**
	 * 구역 i 의 하위 시드 = HashCombine(Seed, i). 결과가 0 이면 1 로 바꾼다 — 생성기는 시드 0 이면 아무것도
	 * 안 짓는다. 복제된 시드 자체가 0 이면("짓지 않음", cl9.seed 0) 하위 시드도 0 이다.
	 */
	uint32 SubSeed(uint32 Seed, int32 Index);

	/** 0·90·180·270 을 반시계 4분의 1 회전 수(0~3)로 — 그 밖의 값은 가장 가까운 90° 배수로 붙인다 */
	int32 QuarterTurns(int32 Yaw);

	/** 평면 벡터를 4분의 1 회전 수만큼 돌린다 — FRotator(0, Yaw, 0) 의 회전과 같은 뜻(+X → +Y) */
	FVector2D Rotate(const FVector2D& Vector, int32 Quarter);

	/**
	 * 모듈 수 하나의 격자 한 변 칸 수 — **CL9SiteLayout::Generate 의 식을 옮겨 적은 것이다**(생성기를 고치지 않으려고).
	 * CL9.Map.Districts 가 실제 생성 결과의 GridSize 와 같은지 본다 — 생성기 식이 바뀌면 거기서 빨간불이 된다.
	 */
	int32 GridSpan(int32 ModuleCount);

	/** 구역 판이 그 날 차지할 수 있는 격자 사각형 — 회전을 적용한 뒤의 축 정렬 사각형 */
	FBox2D SpanRect(const FCL9SiteDistrict& District, int32 DayModuleCount, float CellSize, float EntryOffset);

	/** 트럭 패드에서 구역 입구(격자 서쪽 변 한가운데)까지의 직선 거리 (cm) — 한 방향 마당 거리 */
	float GateDistance(const FCL9SiteDistrict& District, float EntryOffset);

	/**
	 * 그 구역을 들어갔다 나오는 밤 어림 (초) = 양계장·대농장 두 점 직선(docs/systems/stage.md "밤 길이 어림")에
	 * 마당 왕복을 더한 것: 160 + 5.3 × MainPathShare × N + 2 × 마당 거리 ÷ 마당 속도. 밤 600 · 본줄기 0.30 ·
	 * 4.5 m/s 이면 명세의 `1.59·N + 0.444·L ≤ 440` 과 같다.
	 */
	float NightEstimate(int32 BaseModules, float YardDistance, float MainPathShare, float YardSpeed);

	/**
	 * 슬롯 검사 — 구역끼리 그 날(SizeMult)의 회전한 격자 사각형이 겹치지 않는가(맞닿기만 한 것은 괜찮다),
	 * 구역마다 밤 어림이 NightLength 안인가. 어긴 것마다 한 줄씩 담고, 하나도 없으면 true.
	 */
	bool CheckSlots(const TArray<FCL9SiteDistrict>& Districts, const FCL9DistrictSlotRules& Rules, TArray<FString>& OutErrors);
}
