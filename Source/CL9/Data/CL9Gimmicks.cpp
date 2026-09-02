// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9Gimmicks.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/RotatingMovementComponent.h"

void UCL9Gimmick_Spin::OnActorLoaded(AActor* Actor)
{
	static const FName SpinnerName(TEXT("CL9GimmickSpinner"));

	if (Actor->FindComponentByClass<URotatingMovementComponent>() != nullptr)
	{
		return;
	}

	UStaticMeshComponent* Mesh = Actor->FindComponentByClass<UStaticMeshComponent>();
	if (Mesh == nullptr)
	{
		return;
	}

	URotatingMovementComponent* Spinner = NewObject<URotatingMovementComponent>(Actor, SpinnerName);
	Spinner->SetUpdatedComponent(Mesh);
	Spinner->RotationRate = FRotator(0.f, DegreesPerSecond, 0.f);
	Spinner->RegisterComponent();
}
