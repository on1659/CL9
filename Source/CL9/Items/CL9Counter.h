// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "World/CL9Fixture.h"
#include "Core/CL9Interactable.h"
#include "Data/CL9Types.h"
#include "CL9Counter.generated.h"


/**
 * 납품 창구(docs/systems/order.md 납품 판정). 손에 든 것과 끌고 온 수레의 짐을
 * 받아 보관하고, 보관분이 수락한 주문의 재료를 채우는 순간 호스트가 그 주문을
 * 완납 처리한다. 주문에 쓰이지 않고 남은 것은 빈손으로 한 번 더 상호작용해서
 * 그냥 판다(docs/systems/economy.md 이더의 흐름).
 *
 * 판정은 전부 게임 모드에 있다 — 이 액터는 물건을 받아 두는 자리이지 규칙이
 * 아니다. 4f의 정식 주방·트럭이 들어오기 전까지 임시 부엌 옆에 놓는 상자다.
 */
UCLASS(abstract)
class ACL9Counter : public ACL9Fixture, public ICL9Interactable
{
	GENERATED_BODY()

public:
	ACL9Counter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual FText GetPrompt(const ACL9Character* Who) const override;
	virtual bool CanInteract(const ACL9Character* Who) const override;
	virtual void Interact(ACL9Character* Who) override;

	/** 지금 창구에 올려져 있는 것들 — 주문에 쓰이거나 팔릴 때까지 남아 있다 */
	const TArray<FCL9CounterItem>& GetStock() const
	{
		return Stock;
	}

	/** 호스트 전용: 물건 하나를 창구에 올린다 — 굴림값은 올린 시점의 것을 그대로 들고 있는다 */
	void AddStock(FName ItemAlias, FName ItemSubAlias, int32 Value);

	/** 호스트 전용: alias+sub가 일치하는 것 하나를 빼낸다(주문 재료로 쓰인다) — 없으면 false */
	bool TakeStock(FName ItemAlias, FName ItemSubAlias);

	/** 호스트 전용: 창구를 비우고 있던 것을 전부 넘겨준다(판매) */
	void TakeAllStock(TArray<FCL9CounterItem>& OutItems);

private:
	/**
	 * 올려둔 물건들, 올린 순서대로. 액터가 아니라 값으로 들고 있는다 — 창구에
	 * 올린 물건은 다시 집어갈 수 없으므로 실물을 남겨둘 이유가 없고, 프롬프트가
	 * 모든 머신에서 옳으려면 복제돼야 한다.
	 */
	UPROPERTY(Replicated)
	TArray<FCL9CounterItem> Stock;
};
