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

	// -- 맵 생성 (site.md 맵 생성). 격자는 사이트마다 고정돼 있고, 시드는
	//    그 안의 미로 — 우리, 플러그, 항아리, 플랩 — 모양만 정한다.

	/** 이 사이트를 어떤 생성기로 배치할지 — None이면 평지 레벨을 그대로 둔다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	ECL9Generator Generator = ECL9Generator::None;

	/**
	 * 이 사이트를 어느 조각 묶음으로 지을지 — 조각 표(UCL9PieceDef)의 SubAlias를
	 * 가리킨다 (docs/systems/asset-swap.md). 테마에 없는 역할은 테마 칸이 빈 공용
	 * 행으로 떨어진다. 비워 두면 공용 행만 쓴다.
	 *
	 * 격자 치수는 조각 모듈에 맞춰야 예쁘게 떨어지므로, 실질적으로 테마와 아래
	 * 격자 컬럼들은 한 벌로 움직인다 — 같은 배치를 다른 테마로 내려면 이 행을
	 * 복제해서 Theme만 바꾼다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	FName Theme;

	/** 1층 통로 수 (축사를 따라 나 있는 복도) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	int32 AisleCount = 10;

	/** 1층 열 수 (각 통로를 따라 있는 칸) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	int32 ColCount = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	int32 AisleCountB1 = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	int32 ColCountB1 = 6;

	/** 통로 폭 (cm) — 문과 계단 복도가 이 값을 공유한다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	float AisleWidth = 250.f;

	/** 통로를 따라 있는 칸의 길이 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	float ColWidth = 1000.f;

	/** 두 통로 사이 우리 행의 깊이 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	float RowDepth = 120.f;

	/** 우리 행을 가로질러 뚫는 개구부 (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	float OpenWidth = 360.f;

	/** 통로를 가로막는 플러그 (cm, 통로 방향으로) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	float PlugWidth = 180.f;

	/** ★ 난이도 손잡이 — 1층에서 통과 가능한 간선 수, 적을수록 막다른 길이 많아진다 (site.md 개구부) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	int32 TargetOpen1F = 87;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	int32 TargetOpenB1 = 54;

	/** 층마다 있는 반납 항아리 수, 모든 칸 근처에 하나씩 있도록 분산시킨다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	int32 JarCount1F = 7;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Map")
	int32 JarCountB1 = 6;
};
