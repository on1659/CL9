// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9FlashItem.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Data/CL9ItemDef.h"
#include "GameMode/CL9GameState.h"

ULightComponent* ACL9FlashItem::MakeLightComponent()
{
	const UCL9ItemDef* Def = GetItemDef();
	USpotLightComponent* Spot = NewObject<USpotLightComponent>(this, TEXT("ItemLight"));
	Spot->SetOuterConeAngle(Def->LightConeAngle);
	Spot->SetInnerConeAngle(FMath::Max(0.f, Def->LightConeAngle - 8.f));
	Spot->SetAttenuationRadius(FMath::Max(Def->LightRadius, 100.f));

	// 광원은 렌즈 앞에서 나와야 한다 — 메시 원점(몸통 한가운데)에 두면 닫힌 메시가
	// 스포트라이트를 통째로 자기 그림자에 가둬 빛이 한 줄기도 안 나온다
	// (2026-09-11 SM_Flashlight_A로 실측). 메시 로컬 바운드의 +X 끝에서 살짝 앞이다.
	if (const UStaticMesh* Mesh = GetMeshComponent()->GetStaticMesh())
	{
		Spot->SetRelativeLocation(FVector(Mesh->GetBoundingBox().Max.X + 1.f, 0.f, 0.f));
	}
	return Spot;
}

void ACL9FlashItem::OnAcquired()
{
	// 손전등은 꺼진 채로 시작해 소지자가 직접 토글해야 한다 (docs/systems/light.md)
	SetLightOn(false);
}

void ACL9FlashItem::ToggleLight()
{
	if (!HasAuthority())
	{
		return;
	}

	// 정전은 손전등의 천적이다 — 정전 중에는 켤 수 없다 (light.md)
	if (!IsLightOn() && IsKilledByBlackout())
	{
		return;
	}
	SetLightOn(!IsLightOn());
}

bool ACL9FlashItem::ShouldLightBeVisible() const
{
	// 모든 머신이 같은 결과를 도출한다: 스위치가 켜져 있어도 정전이면 빔을 그리지 않는다
	return Super::ShouldLightBeVisible() && !IsKilledByBlackout();
}

bool ACL9FlashItem::IsKilledByBlackout() const
{
	if (IsWorn())
	{
		return false;
	}
	const ACL9GameState* CL9GameState = GetWorld() ? GetWorld()->GetGameState<ACL9GameState>() : nullptr;
	return CL9GameState != nullptr && CL9GameState->IsBlackout();
}

void ACL9FlashItem::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 켜진 손전등은 모든 머신에서 정전 여부를 매 틱 확인해서, 정전이 시작되는
	// 순간 빛이 꺼지게 한다 — 반납은 그 순간을 앞당길 수 있으므로 고정 타이머로는
	// 틀린다 (docs/systems/light.md). 호스트는 스위치도 꺼서 이 상태를 유지시킨다.
	if (IsLightOn() && IsKilledByBlackout())
	{
		UpdateLightVisibility();
		SetLightOn(false);
	}
}
