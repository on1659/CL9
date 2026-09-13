// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9ItemDef.h"
#include "Engine/StaticMesh.h"

const FName UCL9ItemDef::GripSocket(TEXT("Grip"));
const FName UCL9ItemDef::CargoSocket(TEXT("Cargo"));

bool UCL9ItemDef::HasUseHint() const
{
	return UseAction != ECL9UseAction::None || ItemType == ECL9ItemType::Dish;
}

void UCL9ItemDef::Validate(TArray<FString>& OutErrors) const
{
	Super::Validate(OutErrors);

	if (Radius <= 0.f)
	{
		OutErrors.Add(TEXT("Radius must be positive."));
	}
	if (Gravity <= 0.f)
	{
		OutErrors.Add(TEXT("Gravity must be positive."));
	}
	if (bFragile && !(CrackSpeed < CrackedBreakSpeed && CrackedBreakSpeed < BreakSpeed))
	{
		OutErrors.Add(TEXT("Fragile items need CrackSpeed < CrackedBreakSpeed < BreakSpeed — cracked must be weaker (docs/plan/economy.md)."));
	}
	if (ValueMin < 0 || ValueMax < ValueMin)
	{
		OutErrors.Add(TEXT("Value range needs 0 <= ValueMin <= ValueMax."));
	}
	if (HitDamage < 0)
	{
		OutErrors.Add(TEXT("HitDamage must be 0 or more (0 = no damage)."));
	}
	if (UseAction != ECL9UseAction::Swing && (HitDamage > 0 || HitStun != ECL9HitStun::None))
	{
		OutErrors.Add(TEXT("HitDamage/HitStun only apply when UseAction is Swing."));
	}

	// 메시가 몸체인 행(수레)은 스프링 밀기·적재 판정에 필요한 값과 소켓이 전부 있어야 한다 (docs/plan/data/item/cart.md)
	if (bMeshIsBody)
	{
		if (!bPhysicsEnabled)
		{
			OutErrors.Add(TEXT("bMeshIsBody needs bPhysicsEnabled — the mesh body is a real rigid body (docs/plan/data/item/cart.md)."));
		}
		if (Mass <= 0.f)
		{
			OutErrors.Add(TEXT("Mass must be positive when bMeshIsBody is set."));
		}
		if (PushSpring <= 0.f || PushYawSpring <= 0.f)
		{
			OutErrors.Add(TEXT("PushSpring and PushYawSpring must be positive when bMeshIsBody is set."));
		}
		if (PushDamping < 0.f || PushYawDamping < 0.f)
		{
			OutErrors.Add(TEXT("PushDamping and PushYawDamping cannot be negative."));
		}
		if (GripDistance <= 0.f || GripBreakDistance <= 0.f)
		{
			OutErrors.Add(TEXT("GripDistance and GripBreakDistance must be positive when bMeshIsBody is set."));
		}
		if (CargoExtent.GetMin() <= 0.f)
		{
			OutErrors.Add(TEXT("CargoExtent needs every component positive — it is the half-size of the cargo box."));
		}

		const UStaticMesh* BodyMesh = Mesh.LoadSynchronous();
		if (BodyMesh == nullptr)
		{
			OutErrors.Add(TEXT("Mesh is required when bMeshIsBody is set — the mesh is the body."));
		}
		else
		{
			if (BodyMesh->FindSocket(GripSocket) == nullptr)
			{
				OutErrors.Add(FString::Printf(TEXT("Mesh has no '%s' socket — the handle the pusher holds (docs/plan/data/item/cart.md)."), *GripSocket.ToString()));
			}
			if (BodyMesh->FindSocket(CargoSocket) == nullptr)
			{
				OutErrors.Add(FString::Printf(TEXT("Mesh has no '%s' socket — the centre of the cargo box (docs/plan/data/item/cart.md)."), *CargoSocket.ToString()));
			}
		}
	}
}
