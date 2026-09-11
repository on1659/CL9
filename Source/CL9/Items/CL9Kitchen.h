// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "World/CL9Fixture.h"
#include "CL9Kitchen.generated.h"

class ACL9Item;
class UBoxComponent;

/**
 * 조리대 — 이 게임의 열원이다(docs/systems/cooking.md). 상판에 재료를 올려두면
 * 시간이 쌓여 생것 → 익음 → 탄 것으로 넘어간다. **조리대 하나가 한 번에 하나를
 * 익힌다**: 상판에 여러 개를 올려도 먼저 자리를 잡은 것 하나만 시계가 돌고, 그것이
 * 내려가면 남아 있던 것 중 하나가 이어받는다. 동시에 몇 개를 요리할 수 있는지는 조리대가 몇
 * 개인가로 정해진다(2026-09-06 사용자 결정 — 프라이팬·냄비는 열원이 아니라 그냥
 * 아이템이다).
 *
 * 조작은 운반 그대로다 — 놓고, 기다리고, 집는다. 요리 전용 상호작용이 없으므로
 * ICL9Interactable도 구현하지 않는다. 판정은 호스트 전용이고, 진행도는 아이템에
 * 실려 복제되어 표현(게이지)이 읽는다. "무엇을 익히는 중인가"는 조리대에 실려 복제된다 —
 * HUD가 조리대 위에 게이지를 띄우려면 클라도 그 짝을 알아야 하기 때문이다(2026-09-10).
 *
 * 지금은 레벨에 손으로 놓는다. 나중에는 던전 안에 맵 생성기가 배치한다.
 */
UCLASS(abstract)
class ACL9Kitchen : public ACL9Fixture
{
	GENERATED_BODY()

public:
	ACL9Kitchen();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 지금 이 조리대가 익히고 있는 것, 없으면 null — 복제되므로 클라의 표현도 읽는다 */
	UFUNCTION(BlueprintPure, Category="Cooking")
	ACL9Item* GetCookingItem() const;

	/** 몸체 꼭대기 한가운데(월드) — HUD가 조리 게이지를 이 위에 띄운다 */
	UFUNCTION(BlueprintPure, Category="Cooking")
	FVector GetTopLocation() const;

protected:
	virtual void BeginPlay() override;

	/** 호스트 전용: 상판 위의 재료 하나를 골라 시간을 쌓고, 다 되면 다음 행으로 넘긴다 */
	void TickCooking(float DeltaSeconds);

	/** 열 구역 안에 놓여 있고(Free) 상태가 변할 수 있는 아이템들, 가까운 순 */
	void GatherOnHeat(TArray<ACL9Item*>& OutItems) const;


	/** 상판 위 열이 닿는 구역 — 이 안에 놓인 것만 익는다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UBoxComponent> HeatZone;

	/** 상판 위로 열이 닿는 높이 (cm) — 재료가 살짝 굴러도 계속 익게 넉넉히 잡는다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cooking")
	float HeatReach = 60.f;

private:
	/** 지금 익히고 있는 것 — 호스트가 정하고 복제된다. 내려가면 다음 재료가 이어받는다 */
	UPROPERTY(Replicated, Transient)
	TObjectPtr<ACL9Item> Cooking;

	/** Cooking이 열원 위에서 보낸 시간(초) */
	float HeatSeconds = 0.f;

	/** 이번 재료에 대해 연기 예고 로그를 이미 냈는지 */
	bool bSmokeWarned = false;
};
