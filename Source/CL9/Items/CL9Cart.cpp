// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9Cart.h"
#include "Character/CL9Character.h"
#include "Components/CapsuleComponent.h"
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
		return NSLOCTEXT("CL9", "CartLoad", "[E] 싣기");
	}
	return Who && Who->GetPullingCart() == this
		? NSLOCTEXT("CL9", "CartRelease", "[E] 수레 놓기")
		: NSLOCTEXT("CL9", "CartPull", "[E] 수레 끌기");
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

	// 수레를 매다는 동작 자체는 수레를 움직이지 않는다 — 제자리에 그대로 있다가
	// 견인자가 움직이면 서 있던 자리부터 뒤따르기 시작한다 (2026-09-02 피드백).
	HoldState = ECL9ItemHoldState::Pulled;
	SimVelocity = FVector::ZeroVector;
	bAtRest = true;
	ApplyHoldState();
	Puller = Who;
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
	Puller = nullptr;
	Who->SetPullingCart(nullptr);
}

void ACL9Cart::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 견인, 호스트 전용: 수레는 연결 거리를 유지하며 뒤따른다 — 견인자가 그 거리를
	// 넘어 벌어진 만큼만 움직이므로, 제자리에서 회전해도 수레가 휘둘리지 않는다.
	if (!HasAuthority() || Puller == nullptr || GetItemDef() == nullptr)
	{
		return;
	}

	FVector Offset = GetActorLocation() - Puller->GetActorLocation();
	Offset.Z = 0.f;
	const float Distance = Offset.Size();
	const float HitchDistance = Puller->GetCapsuleComponent()->GetScaledCapsuleRadius() + GetItemDef()->Radius + 20.f;
	if (Distance > HitchDistance && Distance > UE_KINDA_SMALL_NUMBER)
	{
		const FVector Towed = Puller->GetActorLocation() + Offset * (HitchDistance / Distance);
		const FVector Target(Towed.X, Towed.Y, GetActorLocation().Z);

		// 막히면 벽을 타고 미끄러진다. 스윕 이동은 막히는 순간 이동 전체가 거부되므로,
		// 그냥 두면 견인 직선 위에 벽이나 동물이 하나만 걸려도 수레가 거기 물려서
		// 영영 따라오지 못한다 — 견인은 항상 견인자 쪽 직선으로만 밀기 때문이다
		// (2026-09-08 PIE 재현: 빈 바닥에서는 119cm 간격을 정확히 지키다가,
		// 스켈레탈 메시에 닿는 순간 굳어 다시는 움직이지 않았다).
		FHitResult Hit;
		SetActorLocation(Target, true, &Hit);
		if (Hit.bBlockingHit)
		{
			const FVector Remaining = Target - GetActorLocation();
			SetActorLocation(GetActorLocation() + FVector::VectorPlaneProject(Remaining, Hit.Normal), true);
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
