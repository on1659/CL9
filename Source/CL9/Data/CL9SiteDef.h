// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "Data/CL9Types.h"
#include "CL9SiteDef.generated.h"

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
