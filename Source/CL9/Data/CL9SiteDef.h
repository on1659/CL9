// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "Data/CL9Types.h"
#include "CL9SiteDef.generated.h"

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
};
