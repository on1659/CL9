// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9LightItem.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Data/CL9ItemDef.h"

void ACL9LightItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACL9LightItem, bLightOn);
}

void ACL9LightItem::LoadData()
{
	Super::LoadData();

	// 행의 광원 수치가 여기서 실제 컴포넌트로 바뀐다. 한 번만 만들어지며, SubAlias
	// 재로드(조리됨/균열)가 일어나도 같은 컴포넌트를 그대로 쓴다.
	if (LightComponent != nullptr || GetItemDef() == nullptr)
	{
		return;
	}

	LightComponent = MakeLightComponent();
	if (LightComponent == nullptr)
	{
		return;
	}

	LightComponent->SetIntensity(GetItemDef()->LightIntensity);
	LightComponent->SetVolumetricScatteringIntensity(GetItemDef()->LightScattering);
	LightComponent->SetVisibility(false);
	LightComponent->RegisterComponent();

	// 라이트는 메시의 자식이다 — 소유자 화면에서 메시가 1인칭 팔 소켓으로 옮겨 가면(UpdateFirstPersonView) 빛도
	// 따라가고, 방향은 언제나 메시 +X다. 발원점은 메시 로컬 오프셋이라 메시 스케일(반경 맞춤)을 그대로 탄다 —
	// 1인칭 렌더 스케일(FirstPersonScale 0.6)은 그리기에만 걸리고 월드 트랜스폼은 건드리지 않으므로, 라이트의
	// 월드 위치와 화면의 렌즈는 카메라에서 같은 시선 위에 놓인다.
	LightComponent->AttachToComponent(GetMeshComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	LightComponent->SetRelativeLocation(GetLightOrigin());
	UpdateLightVisibility();
}

ULightComponent* ACL9LightItem::MakeLightComponent()
{
	// 발광: 들고 있는 동안 전방향으로 빛난다
	UPointLightComponent* Point = NewObject<UPointLightComponent>(this, TEXT("ItemLight"));
	Point->SetAttenuationRadius(FMath::Max(GetItemDef()->LightRadius, 50.f));
	return Point;
}

void ACL9LightItem::OnAcquired()
{
	// 발광은 소지되는 순간 켜진다 (docs/systems/light.md)
	SetLightOn(true);
}

void ACL9LightItem::OnReleased()
{
	// 내려놓거나 던진 아이템은 불이 꺼진다 — 빛은 소지 중일 때만 난다
	SetLightOn(false);
}

void ACL9LightItem::SetLightOn(bool bOn)
{
	if (!HasAuthority() || bLightOn == bOn)
	{
		return;
	}

	bLightOn = bOn;
	OnRep_LightOn();
}

void ACL9LightItem::OnRep_LightOn()
{
	UpdateLightVisibility();
}

void ACL9LightItem::UpdateLightVisibility()
{
	if (LightComponent != nullptr)
	{
		LightComponent->SetVisibility(ShouldLightBeVisible());
	}
}
