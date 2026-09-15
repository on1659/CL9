// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/CL9Interactable.h"
#include "Items/CL9Item.h"
#include "CL9Cart.generated.h"

class ACL9GameState;
class UBoxComponent;

/**
 * 수레(docs/plan/data/item/cart.md): 밀고 다닐 뿐 손에 들거나 퀵슬롯에 넣을 수 없는 아이템이다.
 * 수레 행의 ActorClass 열은 이 클래스의 BP 자식을 가리킨다 — 행별 전용 동작을 넣는 정식 경로가 바로 이것이다.
 *
 * 실물 크기의 물리 강체다 (2026-09-13, 사용자: "실제 현실에서 옮기는 느낌"): 행의 bMeshIsBody가 메시(UCX 콜리전)를
 * 몸체로 쓰게 하고, 미는 동안 호스트가 매 틱 손잡이 소켓 `Grip`을 미는 사람 앞 목표점으로 끄는 스프링·감쇠 힘과
 * yaw 스프링 토크를 준다 — 수레가 반 박자 늦게 따라오고, 멈추면 미끄러지다 서고, 제자리 회전이면 원호를 그린다.
 * 손잡이가 목표점에서 GripBreakDistance보다 벌어지면 손을 놓친다. 밀기는 캐릭터 행의 CartMult로 미는 사람의 속도를
 * 늦춘다(예측 이동 경로). 클라이언트는 복제된 물리 상태를 보간한다(호스트 권위).
 *
 * 적재는 부착이 아니라 물리다: 메시 소켓 `Cargo`를 중심으로 하는 CargoExtent 상자(CargoVolume)와 겹치는 자유 상태
 * 아이템이 곧 "실린 것"이다. 싣기는 짐칸을 보고 G(손 자리에서 짐칸으로 벽을 넘는 낮은 호를 그려 보낸다 —
 * 2026-09-15, 짐칸을 보지 않으면 G는 손 자리 낙하다)와 던지기(달리기+G)뿐이고, 꺼내기는 바닥 아이템과
 * 같은 E 줍기다. 상호작용 키는 밀기/놓기만 — 활성 칸이 빈 채(빈손)로 조준하면 밀고, 미는 중이면 놓는다. 활성 칸에
 * 뭔가 든 채 조준하면 프롬프트도 E도 없다. 미는 동안은 퀵슬롯 전환·줍기가 잠긴다(ACL9Character::CanSwitchSlot).
 * 항아리·창구는 짐칸 안 것을 한 번에 가져간다. 적재 한도는 없다(무게 제한 없음 철학과 일치).
 */
UCLASS(abstract)
class ACL9Cart : public ACL9Item, public ICL9Interactable
{
	GENERATED_BODY()

public:
	ACL9Cart();

	virtual void Tick(float DeltaSeconds) override;

	virtual FText GetPrompt(const ACL9Character* Who) const override;
	virtual bool CanInteract(const ACL9Character* Who) const override;
	virtual void Interact(ACL9Character* Who) override;

	/** 호스트 전용: 미는 사람에게서 분리한다 — 수레는 제 속도로 미끄러지다 선다 */
	void StopPull(ACL9Character* Who);

	/** 호스트 전용: 짐칸 안 아이템 전부를 반납 처리하고 액터를 파괴한다 (항아리 수레째 반납) */
	void ReturnAllLoaded(ACL9GameState* GameState);

	/**
	 * 호스트 전용: 짐칸 안 아이템들을 넘겨준다 — 넘겨받은 쪽이 처분을 정한다(항아리는 반납으로,
	 * 창구는 납품으로 센다). 액터를 파괴하지는 않는다.
	 */
	void TakeAllLoaded(TArray<ACL9Item*>& OutItems);

	/** 짐칸 상자 안에 자유 상태 아이템이 하나라도 있는가 — 프롬프트가 읽으므로 모든 머신에서 답한다 */
	bool HasLoadedItems() const;

	/**
	 * 짐칸에 얹으려 할 때 겨냥할 월드 지점 — 짐칸 상자의 중심 XY에, 높이는 상자 밑면(= 짐칸 바닥)에서
	 * 아이템 반지름만큼 올린 자리다. 그 아이템이 실제로 얹혀 멈출 자리라서, 놓기가 여기를 겨냥하면
	 * 착지 속도를 그대로 예측할 수 있다 (ACL9Character의 짐칸 놓기). 반지름이 상자 반높이보다 크면
	 * 상자 중심을 쓴다. 판정은 호스트지만 계산 자체는 모든 머신에서 답한다.
	 */
	FVector GetCargoDropPoint(float ItemRadius) const;

	/**
	 * 짐칸 밖의 한 점에서 짐칸 가운데로 가는 **수평 직선**이 짐칸 상자의 옆면(= 짐칸 벽 안쪽 면)을 넘는 지점 —
	 * `OutFraction`은 그 지점까지의 비율(0=출발점, 1=짐칸 가운데), `OutTopZ`는 상자 윗면의 월드 높이(= 벽 윗단)다.
	 * 짐칸으로 던져 넣는 호가 **벽을 넘는지** 보는 데 쓴다 (ACL9Character의 짐칸 놓기). 출발점이 이미 짐칸 위면
	 * 사이에 넘을 벽이 없으므로 false.
	 */
	bool GetCargoWallCrossing(const FVector& FromWorld, float& OutFraction, float& OutTopZ) const;

protected:
	virtual void LoadData() override;

private:
	/** 짐칸 상자(CargoVolume)와 겹치는 자유 상태 아이템 — 부착이 아니라 위치가 진실이다 */
	void GatherLoadedItems(TArray<ACL9Item*>& OutItems) const;

	/** 적재 판정 상자 — 메시 원점에 붙어 로드 때 소켓 Cargo의 위치로 옮겨지고(스케일 1 고정), 반폭은 행의 CargoExtent. 충돌체가 아니라 겹침 질의의 자리·크기만 준다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> CargoVolume;

	/** 밀리는 방향 — 손잡이 소켓에서 수레 중심을 향하는 수레 로컬 XY 단위 벡터(로드 때 소켓에서 계산, 기본은 +X 손잡이 → −X) */
	FVector PushForwardLocal = FVector(-1.f, 0.f, 0.f);

	/** 잡은 직후 힘·속도를 로그로 남길 남은 틱 수 — PIE 판독용 (호스트 전용) */
	int32 PushLogTicksLeft = 0;

	/** 이 수레를 미는 사람 — 호스트 전용이며, 클라이언트는 대신 캐릭터의 복제된 포인터를 읽는다 */
	UPROPERTY()
	TObjectPtr<ACL9Character> Puller;
};
