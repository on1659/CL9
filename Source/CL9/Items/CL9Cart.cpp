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
	// Its own puller may always address it; otherwise it must be free and the
	// character not pulling anything — bare hand pulls, a full hand loads.
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

	// Hitching never moves the cart — it stays put and starts trailing the
	// puller from wherever it stands (2026-09-02 피드백).
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

	// Let go in place — Free again so throws can bump it, sim settles it.
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

	// Towing, host only: the cart trails at hitch distance — it moves only by
	// the amount the puller stretches past that distance, so turning in place
	// never swings it around.
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
		SetActorLocation(FVector(Towed.X, Towed.Y, GetActorLocation().Z), true);
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

	for (ACL9Item* Item : LoadedItems)
	{
		if (Item != nullptr)
		{
			GameState->RecordReturn(Item->GetAlias(), Item->GetSubAlias());
			Item->Destroy();
		}
	}
	LoadedItems.Reset();
}

FVector ACL9Cart::GetLoadSlotLocation(int32 Index, float ItemRadius) const
{
	// Two loose columns marching backward along the bed — presentation only.
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
