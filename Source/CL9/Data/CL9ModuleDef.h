// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "CL9ModuleDef.generated.h"

class UStaticMesh;

/** 모듈이 배치에서 맡는 자리 (docs/plan/data/module.md 표 컬럼) */
UENUM(BlueprintType)
enum class ECL9ModuleType : uint8
{
	/** 트럭이 붙는 입구. 한 판에 정확히 하나 */
	Start,
	/** 방 — 배치의 본체 */
	Normal,
	/** 복도·계단처럼 지나가는 곳 */
	Passage,
	/** 한쪽으로만 뚫린 곁가지 끝 */
	DeadEnd,
	/** 반드시 나와야 하는 방 (납품 항아리·조리대) */
	Special,
	/**
	 * 위층으로 올라가는 계단실. **아래층과 위층의 같은 칸을 함께 차지한다** — 그래서
	 * 이 모듈 하나가 두 층을 잇는 유일한 통로가 된다. 한 판에 몇 개가 서는지는
	 * 사이트 행의 `StairCountMin`·`StairCountMax` 사이에서 시드가 뽑는다.
	 */
	Stair
};

/** 모듈 타입의 가짓수 — 타입별 후보 묶음을 만들 때 쓴다 */
constexpr int32 CL9ModuleTypeCount = 6;

/**
 * 모듈의 네 변. **비트 순서가 곧 시계 방향(N→E→S→W)이라 회전이 비트 회전이 된다** —
 * 반시계 90° × r 회전은 `새 비트[j] = 옛 비트[(j + r) % 4]`다.
 *
 * 소켓의 단위는 변이 아니라 **경계 칸**이다 (module.md 결정 1): 이 플래그는 "그 변이
 * 열린다"까지만 말하고, 그러면 그 변에 닿은 모든 경계 칸이 문 후보가 된다. 변 중앙에
 * 하나로 고정할 수 없는 이유는 2×2·3×2처럼 변이 짝수 칸인 모듈에서 중앙이 칸 경계에
 * 떨어지기 때문이다.
 */
UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ECL9ModuleSide : uint8
{
	/** +Y */
	North = 1 << 0,
	/** +X */
	East = 1 << 1,
	/** -Y */
	South = 1 << 2,
	/** -X */
	West = 1 << 3
};
ENUM_CLASS_FLAGS(ECL9ModuleSide);

/**
 * 이 모듈의 칸이 어떤 지점(`FCL9LevelPoint`)이 되는지 (docs/plan/data/module.md 결정 4,
 * REPO의 `LevelPoint`에 해당). 밤이 시작될 때 스폰이 이 용도로 자리를 고른다 — 그래서
 * "항아리는 납품 방에만" 같은 규칙이 C++에 이름으로 박히지 않고 표에 남는다.
 */
UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ECL9PointPurpose : uint8
{
	/** 몬스터를 놓을 수 있다 */
	Monster = 1 << 0,
	/** 재료를 놓을 수 있다 */
	Loot = 1 << 1,
	/** 반납 항아리를 놓는다 — 납품 방만 켠다 */
	Jar = 1 << 2,
	/** 순찰이 지나갈 만한 자리 */
	Patrol = 1 << 3
};
ENUM_CLASS_FLAGS(ECL9PointPurpose);

/** 소품이 기대는 변 — 회전 전 기준. 순서는 ECL9ModuleSide의 비트 순서(N→E→S→W)와 같다 */
UENUM(BlueprintType)
enum class ECL9PropSide : uint8
{
	None,
	North,
	East,
	South,
	West
};

/**
 * 소품 하나를 그 변의 연결 여부로 켜고 끄는 규칙 (REPO `ModulePropSwitch` — 확률이 아니라
 * "그 변이 이어졌는가"로 결정된다, docs/reference/repo/repo-map.md 소품 교체).
 */
UENUM(BlueprintType)
enum class ECL9PropSideRule : uint8
{
	/** 변과 무관하게 항상 선다 */
	Always,
	/** 소품 발자국이 걸친 경계 칸이 전부 벽일 때만 — 벽걸이 표식·작업대처럼 벽에 기대는 것 */
	SideClosed,
	/** 그 경계 칸 중 하나라도 문이나 비상구일 때만 — 문 옆에만 서는 장식 */
	SideOpen
};

/**
 * 소품 세트 안의 배치 하나. 좌표는 **회전 전 모듈 로컬 cm**다 — 원점은 모듈 사각형의
 * 최소 모서리(회전 전 Size 기준), Z는 그 층 바닥 판 윗면에서 잰다. 모듈이 회전해서
 * 앉으면 생성기가 같이 돌린다 (`CL9SiteLayout::ModuleLocalToSite`).
 */
USTRUCT(BlueprintType)
struct FCL9PropPlacement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop")
	TSoftObjectPtr<UStaticMesh> Mesh;

	/** 회전 전 모듈 로컬 위치 (cm). 메시 피벗이 그 자리에 놓인다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop")
	FVector Local = FVector::ZeroVector;

	/** 회전 전 요 (도). 모듈 회전이 여기에 더해진다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop")
	float Yaw = 0.f;

	/**
	 * 몸이 있는 소품인가. true면 이동·시야를 막고(BlockAll) 내비게이션에 올라가며, false면
	 * 충돌 없이 그림만 그린다 — 벽걸이 표식·작업등처럼 걸리면 안 되는 것.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop")
	bool bSolid = true;

	/** SideRule이 보는 변 — None이면 규칙을 따지지 않는다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop")
	ECL9PropSide Side = ECL9PropSide::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop")
	ECL9PropSideRule SideRule = ECL9PropSideRule::Always;

	/**
	 * 이 소품이 내는 빛의 세기 (칸델라). 0이면 빛이 없다. 작업등처럼 방을 밝히는 **시설 조명**이
	 * 여기로 들어온다 — 배치기가 인스턴스마다 그림자 없는 포인트 라이트를 하나 붙이고, 정전이면
	 * 끈다 (docs/systems/light.md 시설 조명, 2026-09-16).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light", meta=(ClampMin="0.0"))
	float LightIntensity = 0.f;

	/** 빛이 닿는 반경 (cm) — 방 하나쯤 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light", meta=(ClampMin="0.0"))
	float LightRadius = 800.f;

	/** 빛 색 — 작업등은 따뜻한 전구색 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light")
	FLinearColor LightColor = FLinearColor(1.f, 0.82f, 0.6f, 1.f);

	/** 광원이 놓이는 자리 — 소품 로컬(회전 전) 오프셋 cm. 갓 아래 전구 자리로 내리고 벽에서 조금 띄운다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light")
	FVector LightOffset = FVector::ZeroVector;

	/**
	 * 볼류메트릭 안개 속 후광 세기 (라이트 VolumetricScatteringIntensity, 1 = 엔진 기본). 리썰식 안개가 조명
	 * 기본이 되면서(2026-09-17) 광원 둘레의 뿌연 빛도 표 값이다 — 작업등은 옅게, 모닥불은 짙게.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light", meta=(ClampMin="0.0"))
	float LightScattering = 1.f;

	/** 깜박임 진폭 (0~1) — 세기가 (1 ± 진폭 × 노이즈) 로 흔들린다. 0이면 고정. 모닥불·낡은 전구용 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light", meta=(ClampMin="0.0", ClampMax="1.0"))
	float LightFlicker = 0.f;

	/** 깜박임 속도 (Hz) — 노이즈가 초당 몇 번 바뀌나 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Light", meta=(ClampMin="0.0"))
	float LightFlickerSpeed = 6.f;
};

/** 한 모듈에 함께 깔리는 소품 한 벌 (module.md 결정 7). 인스턴스마다 이 중 하나가 뽑힌다 */
USTRUCT(BlueprintType)
struct FCL9PropSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prop")
	TArray<FCL9PropPlacement> Props;
};

/**
 * 맵 모듈 한 종류 (docs/plan/data/module.md). **모듈은 프리팹이 아니라 이 표의 행이다** —
 * 껍데기(바닥·벽·문·천장)는 생성기가 조각 표(UCL9PieceDef)에서 뽑아 세우므로, 손으로
 * 만들어 두는 방 애셋이 없다. 이 표가 정하는 건 "몇 칸짜리 방이고 어느 변이 뚫리나"까지다.
 *
 * `Alias`가 모듈 이름(`coop_hall` · `corridor`)이고 `SubAlias`가 스테이지/테마
 * (`henhouse`)다 — 조각 표와 같은 규약이라, 다음 스테이지는 SubAlias만 다른 한 벌이 된다.
 *
 * **치수를 손으로 적지 않는다는 원칙은 그대로다** — 조각의 크기와 피벗은 메시 바운드에서
 * 읽는다 (docs/systems/asset-swap.md). 여기 적는 `Size`는 조각 치수가 아니라 격자 칸
 * 개수이고, `Height`는 그 방의 천장이 얼마나 높은 방인가라는 기획값이다.
 */
UCLASS()
class UCL9ModuleDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	/** 회전 r(반시계 90° 단위)을 적용한 뒤의 소켓 비트 */
	static uint8 RotateSockets(uint8 Sockets, int32 Rotation);

	/** 회전 r을 적용한 뒤의 칸 크기 — 홀수 회전이면 X와 Y가 바뀐다 */
	static FIntPoint RotateSize(FIntPoint Size, int32 Rotation);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
	ECL9ModuleType Type = ECL9ModuleType::Normal;

	/** 격자 칸 단위 크기, 회전 전 기준 (칸 하나가 UCL9SiteDef::CellSize) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
	FIntPoint Size = FIntPoint(2, 2);

	/** 어느 변이 열리나 — 회전 전 기준. 하나도 안 열린 모듈은 배치될 수 없다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module", meta = (Bitmask, BitmaskEnum = "/Script/CL9.ECL9ModuleSide"))
	int32 Sockets = 0xF;

	/** 뽑힐 가중치 1~3, 클수록 흔하다 (REPO의 ModuleRarity1/2/3) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
	int32 Rarity = 2;

	/** 이 방의 천장 높이 (cm). bOutdoor면 천장을 씌우지 않으므로 무시된다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
	float Height = 400.f;

	/**
	 * 모듈 **안에서** 오르내리는 높이차가 있는가 (module.md 결정 2). 이웃 모듈의 바닥
	 * 높이로 전파되지 않는다 — 계단은 올라간 만큼 도로 내려온다. 그래서 새 계단 모듈을
	 * 표에 추가할 때 격자 규칙을 함께 손볼 일이 없다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
	bool bStairs = false;

	/** 야외 모듈인가 (마당). 천장을 씌우지 않고, 벽 대신 마당 울타리를 세운다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
	bool bOutdoor = false;

	/**
	 * 이 모듈의 칸들이 어떤 스폰 지점이 되는가. 기본값은 "몬스터·재료·순찰 다 된다"이고,
	 * 납품 방만 Jar를 켠다. 입구 모듈은 전부 끄는 편이 낫다 — 트럭 문턱에서 몬스터가
	 * 튀어나오면 시작하자마자 죽는다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module", meta = (Bitmask, BitmaskEnum = "/Script/CL9.ECL9PointPurpose"))
	int32 PointPurposes = 0xB;

	/**
	 * 소품 세트들 (module.md 결정 7 — "모듈마다 소품 세트를 여러 벌 두고 매번 하나를 뽑는다").
	 * 배치기가 모듈 인스턴스마다 자리 좌표 해시로 하나를 고르므로 시드가 같으면 모든 머신이
	 * 같은 세트를 깐다. 비어 있으면 껍데기만 선다. 소품이 스폰 지점(칸 중심 ±130cm)과
	 * 문 자리를 가리면 배치기가 경고한다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Module")
	TArray<FCL9PropSet> PropSets;
};
