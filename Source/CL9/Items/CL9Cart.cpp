// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9Cart.h"
#include "Internationalization/StringTableRegistry.h"
#include "Character/CL9Character.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Net/UnrealNetwork.h"
#include "Data/CL9ItemDef.h"
#include "GameMode/CL9GameState.h"

void ACL9Cart::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACL9Cart, LoadedItems);
}

FText ACL9Cart::GetPrompt(const ACL9Character* Who) const
{
	if (Who != nullptr && Who->GetHeldItem() != nullptr)
	{
		return LOCTABLE("CL9", "Prompt.CartLoad");
	}
	return Who && Who->GetPullingCart() == this
		? LOCTABLE("CL9", "Prompt.CartRelease")
		: LOCTABLE("CL9", "Prompt.CartPull");
}

bool ACL9Cart::CanInteract(const ACL9Character* Who) const
{
	if (Who == nullptr || GetItemDef() == nullptr)
	{
		return false;
	}
	// 자기가 끌고 있는 캐릭터는 언제든 상호작용할 수 있고, 그 외에는 수레가
	// 자유 상태이고 캐릭터가 아무것도 끌고 있지 않아야 한다 — 빈손이면 끌기,
	// 손에 들고 있으면 싣기다.
	return Who->GetPullingCart() == this
		|| (IsFree() && Who->GetPullingCart() == nullptr);
}

void ACL9Cart::Interact(ACL9Character* Who)
{
	if (!HasAuthority())
	{
		return;
	}

	if (Who->GetHeldItem() != nullptr)
	{
		LoadItem(Who);
		return;
	}
	if (Who->GetPullingCart() == this)
	{
		StopPull(Who);
		return;
	}
	if (Who->GetPullingCart() != nullptr)
	{
		return;
	}

	// 잡는 순간 수레가 튀지 않는다 — 다음 틱부터 미는 사람 앞자리로 스윕 이동한다
	// (2026-09-12: 뒤에서 끌려오던 것을 앞에서 밀고 다니는 것으로 바꿈, plan/data/item/cart.md).
	HoldState = ECL9ItemHoldState::Pushed;
	SimVelocity = FVector::ZeroVector;
	bAtRest = true;
	ApplyHoldState();
	Puller = Who;
	// 연결 거리에서 수레 구체와 캡슐이 스치므로, 밀리는 동안은 미는 사람을 이동 충돌에서 뺀다 —
	// 안 그러면 매 틱 스윕이 그 캡슐에 막혀 수레가 제자리에 남는다 (2026-09-12 PIE 재현).
	CastChecked<UPrimitiveComponent>(GetRootComponent())->IgnoreActorWhenMoving(Who, true);
	Who->SetPullingCart(this);
}

void ACL9Cart::StopPull(ACL9Character* Who)
{
	if (!HasAuthority())
	{
		return;
	}

	// 그 자리에서 손을 놓는다 — 다시 Free 상태가 되어 던지기로 밀 수도 있고, 시뮬레이션이 정착시킨다.
	HoldState = ECL9ItemHoldState::Free;
	SimVelocity = FVector::ZeroVector;
	bAtRest = false;
	ApplyHoldState();
	CastChecked<UPrimitiveComponent>(GetRootComponent())->IgnoreActorWhenMoving(Who, false);
	Puller = nullptr;
	Who->SetPullingCart(nullptr);
}

void ACL9Cart::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 밀기, 호스트 전용: 수레는 미는 사람 정면 연결 거리에 서고 그 사람의 yaw를 따라 돈다.
	// 스윕으로 옮기다 막히면(벽·동물) 못 간 만큼 미는 사람을 뒤로 물려 둘이 함께 멈춘다 —
	// 수레가 벽을 뚫거나 사람 몸속으로 들어오지 않게 하는 최소 규칙이다 (2026-09-12).
	if (!HasAuthority() || Puller == nullptr || GetItemDef() == nullptr)
	{
		return;
	}

	const float HitchDistance = Puller->GetCapsuleComponent()->GetScaledCapsuleRadius() + GetItemDef()->Radius + 20.f;
	const FRotator Facing(0.f, Puller->GetActorRotation().Yaw, 0.f);
	const FVector Ahead = Puller->GetActorLocation() + Facing.Vector() * HitchDistance;
	const FVector Target(Ahead.X, Ahead.Y, GetActorLocation().Z);

	FHitResult Hit;
	SetActorLocationAndRotation(Target, Facing, true, &Hit);
	if (Hit.bStartPenetrating)
	{
		// 시작부터 무언가에 묻혀 있으면(레벨에 손으로 놓은 수레가 바닥에 반경보다 낮게 있는 경우 등)
		// 스윕은 한 발짝도 못 뗀다 — 먼저 법선 방향으로 빼낸 뒤 같은 높이에서 다시 민다.
		AddActorWorldOffset(Hit.Normal * (Hit.PenetrationDepth + 1.f), false);
		SetActorLocationAndRotation(FVector(Target.X, Target.Y, GetActorLocation().Z), Facing, true, &Hit);
	}
	if (Hit.bBlockingHit)
	{
		const FVector Remaining = Target - GetActorLocation();
		FVector Back = FVector::VectorPlaneProject(Remaining, Hit.Normal);
		Back.Z = 0.f;
		// 벽을 따라 미끄러질 수 있는 만큼은 수레가 가고, 남은 만큼 미는 사람이 물러난다
		SetActorLocation(GetActorLocation() + Back, true);
		const FVector Blocked = Remaining - Back;
		if (!Blocked.IsNearlyZero())
		{
			Puller->SetActorLocation(Puller->GetActorLocation() - Blocked, true);
		}
	}
}

void ACL9Cart::LoadItem(ACL9Character* Who)
{
	if (!HasAuthority())
	{
		return;
	}

	ACL9Item* Item = Who->HostTakeHeldItem();
	if (Item == nullptr)
	{
		return;
	}

	const float ItemRadius = Item->GetItemDef() ? Item->GetItemDef()->Radius : 20.f;
	Item->OnLoadedIntoCart(this, GetLoadSlotLocation(LoadedItems.Num(), ItemRadius));
	LoadedItems.Add(Item);
}

void ACL9Cart::RemoveLoadedItem(ACL9Item* Item)
{
	if (!HasAuthority())
	{
		return;
	}

	if (LoadedItems.Remove(Item) > 0)
	{
		RelayoutLoadedItems();
	}
}

void ACL9Cart::ReturnAllLoaded(ACL9GameState* GameState)
{
	if (!HasAuthority() || GameState == nullptr)
	{
		return;
	}

	TArray<ACL9Item*> Taken;
	TakeAllLoaded(Taken);
	for (ACL9Item* Item : Taken)
	{
		GameState->RecordReturn(Item->GetAlias(), Item->GetSubAlias());
		Item->Destroy();
	}
}

void ACL9Cart::TakeAllLoaded(TArray<ACL9Item*>& OutItems)
{
	if (!HasAuthority())
	{
		return;
	}

	for (ACL9Item* Item : LoadedItems)
	{
		if (Item != nullptr)
		{
			OutItems.Add(Item);
		}
	}
	LoadedItems.Reset();
}

FVector ACL9Cart::GetLoadSlotLocation(int32 Index, float ItemRadius) const
{
	// 짐칸을 따라 뒤로 나아가는 느슨한 두 줄 — 순전히 표현용이다.
	const float BedTop = GetItemDef() ? GetItemDef()->Radius : 60.f;
	const float Side = (Index % 2 == 0) ? -26.f : 26.f;
	const float Back = (Index / 2) * 46.f - 23.f;
	return FVector(Back, Side, BedTop + ItemRadius);
}

void ACL9Cart::RelayoutLoadedItems()
{
	for (int32 Index = 0; Index < LoadedItems.Num(); ++Index)
	{
		if (ACL9Item* Item = LoadedItems[Index])
		{
			const float ItemRadius = Item->GetItemDef() ? Item->GetItemDef()->Radius : 20.f;
			Item->SetActorRelativeLocation(GetLoadSlotLocation(Index, ItemRadius));
		}
	}
}
