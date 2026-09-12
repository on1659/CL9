// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/CL9Interactable.h"
#include "Items/CL9Item.h"
#include "CL9Cart.generated.h"

class ACL9GameState;

/**
 * 수레(docs/plan/data/item/cart.md): 밀고 다닐 뿐 주머니에 넣을 수 없는 아이템이다.
 * 수레 행의 ActorClass 열은 이 클래스의 BP 자식을 가리킨다 — 행별 전용 동작을 넣는
 * 정식 경로가 바로 이것이다. 밀기는 예측 이동 경로 안에서 캐릭터 행의 CartMult를
 * 곱해 미는 사람의 속도를 조정한다 (2026-09-12: 뒤에서 끌기 → 앞에서 밀기).
 *
 * 상호작용 키 하나가 전부를 처리한다(2026-09-02): 맨손으로 수레를 조준하면 밀기,
 * 뭔가 들고 조준하면 싣기, 짐칸의 아이템을 조준하면 다시 꺼내기다. 두 손은
 * 배타적이다 — 끌기와 들기는 동시에 있을 수 없다. 항아리 반납은 짐칸 전체를
 * 한 번에 비운다. 적재 한도는 없다(무게 제한 없음 철학과 일치).
 */
UCLASS(abstract)
class ACL9Cart : public ACL9Item, public ICL9Interactable
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual FText GetPrompt(const ACL9Character* Who) const override;
	virtual bool CanInteract(const ACL9Character* Who) const override;
	virtual void Interact(ACL9Character* Who) override;

	/** 호스트 전용: 미는 사람에게서 분리하고 서 있는 자리에 그대로 멈춘다 */
	void StopPull(ACL9Character* Who);

	/** 호스트 전용: 미는 사람이 손에 든 아이템을 짐칸으로 옮긴다 */
	void LoadItem(ACL9Character* Who);

	/** 호스트 전용: 누가 집어 간 짐칸 아이템을 목록에서 지우고 빈 자리를 메운다 */
	void RemoveLoadedItem(ACL9Item* Item);

	/** 호스트 전용: 실린 아이템 전부를 반납 처리하고 액터를 파괴한다 (항아리 수레째 반납) */
	void ReturnAllLoaded(ACL9GameState* GameState);

	/**
	 * 호스트 전용: 짐칸을 비우고 실려 있던 아이템들을 넘겨준다 — 넘겨받은 쪽이
	 * 처분을 정한다(항아리는 반납으로, 창구는 납품으로 센다). 액터를 파괴하지는 않는다.
	 */
	void TakeAllLoaded(TArray<ACL9Item*>& OutItems);

	bool HasLoadedItems() const
	{
		return LoadedItems.Num() > 0;
	}

private:
	/** N번째로 실린 아이템의 짐칸 위치, 수레 루트 기준 상대 좌표 */
	FVector GetLoadSlotLocation(int32 Index, float ItemRadius) const;

	/** 짐칸의 모든 아이템을 각자의 슬롯에 다시 배치한다 (제거로 빈 자리가 생긴 뒤) */
	void RelayoutLoadedItems();

	/** 짐칸에 실린 아이템들, 오래된 순 — 부착된 액터 자체가 곧 진실이다 */
	UPROPERTY(Replicated)
	TArray<TObjectPtr<ACL9Item>> LoadedItems;

	/** 이 수레를 끄는 사람 — 호스트 전용이며, 클라이언트는 대신 캐릭터의 복제된 포인터를 읽는다 */
	UPROPERTY()
	TObjectPtr<ACL9Character> Puller;
};
