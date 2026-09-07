// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9Counter.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Character/CL9Character.h"
#include "GameMode/CL9GameMode.h"
#include "Items/CL9Cart.h"
#include "Items/CL9Item.h"

ACL9Counter::ACL9Counter()
{
	// 재고는 복제된다 — 프롬프트가 모든 머신에서 옳으려면 클라도 창구가 비었는지 알아야 한다
	// (복제 자체는 ACL9ActorBase가 이미 켜 둔다)
}

void ACL9Counter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACL9Counter, Stock);
}

FText ACL9Counter::GetPrompt(const ACL9Character* Who) const
{
	const ACL9Cart* Cart = Who ? Who->GetPullingCart() : nullptr;
	const bool bBringingSomething = Who != nullptr && (Who->GetHeldItem() != nullptr || (Cart != nullptr && Cart->HasLoadedItems()));
	return bBringingSomething
		? NSLOCTEXT("CL9", "CounterDeliver", "[E] 납품")
		: NSLOCTEXT("CL9", "CounterSell", "[E] 남은 것 팔기");
}

bool ACL9Counter::CanInteract(const ACL9Character* Who) const
{
	if (Who == nullptr)
	{
		return false;
	}

	const ACL9Cart* Cart = Who->GetPullingCart();
	if (Who->GetHeldItem() != nullptr || (Cart != nullptr && Cart->HasLoadedItems()))
	{
		return true;
	}
	// 빈손이면 남은 재고를 파는 것 말고는 할 일이 없다
	return Stock.Num() > 0;
}

void ACL9Counter::Interact(ACL9Character* Who)
{
	if (!HasAuthority() || Who == nullptr)
	{
		return;
	}

	ACL9GameMode* GameMode = GetWorld()->GetAuthGameMode<ACL9GameMode>();
	if (GameMode == nullptr)
	{
		return;
	}

	bool bBroughtSomething = false;

	if (ACL9Item* Held = Who->GetHeldItem())
	{
		AddStock(Held->GetAlias(), Held->GetSubAlias(), Held->GetValue());
		Who->HostConsumeHeldItem();
		bBroughtSomething = true;
	}

	// 수레째 납품 — 항아리 반납과 같은 규칙이다(2026-09-02 적재 규칙)
	if (ACL9Cart* Cart = Who->GetPullingCart())
	{
		TArray<ACL9Item*> Loaded;
		Cart->TakeAllLoaded(Loaded);
		for (ACL9Item* Item : Loaded)
		{
			AddStock(Item->GetAlias(), Item->GetSubAlias(), Item->GetValue());
			Item->Destroy();
			bBroughtSomething = true;
		}
	}

	// 물건을 올렸으면 채워진 주문이 있는지 보고, 빈손으로 왔으면 남은 것을 판다
	if (bBroughtSomething)
	{
		GameMode->SettleOrdersAt(*this);
	}
	else
	{
		GameMode->SellStockAt(*this);
	}
}

void ACL9Counter::AddStock(FName ItemAlias, FName ItemSubAlias, int32 Value)
{
	if (!HasAuthority())
	{
		return;
	}

	FCL9CounterItem& Entry = Stock.AddDefaulted_GetRef();
	Entry.ItemAlias = ItemAlias;
	Entry.ItemSubAlias = ItemSubAlias;
	Entry.Value = Value;
}

bool ACL9Counter::TakeStock(FName ItemAlias, FName ItemSubAlias)
{
	if (!HasAuthority())
	{
		return false;
	}

	const int32 Index = Stock.IndexOfByPredicate([ItemAlias, ItemSubAlias](const FCL9CounterItem& Entry)
	{
		return Entry.ItemAlias == ItemAlias && Entry.ItemSubAlias == ItemSubAlias;
	});
	if (Index == INDEX_NONE)
	{
		return false;
	}

	Stock.RemoveAt(Index);
	return true;
}

void ACL9Counter::TakeAllStock(TArray<FCL9CounterItem>& OutItems)
{
	if (!HasAuthority())
	{
		return;
	}

	OutItems = MoveTemp(Stock);
	Stock.Reset();
}
