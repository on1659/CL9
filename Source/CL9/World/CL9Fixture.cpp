// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/CL9Fixture.h"
#include "Components/StaticMeshComponent.h"

ACL9Fixture::ACL9Fixture()
{
	Kind = ECL9ActorKind::Facility;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(MeshComponent);
}

FText ACL9Fixture::GetDisplayName() const
{
	// 표가 없으므로 행의 이름을 찾아볼 곳이 없다 — 비어 있으면 베이스가 액터 이름으로 떨어진다
	return DisplayName.IsEmpty() ? Super::GetDisplayName() : DisplayName;
}
