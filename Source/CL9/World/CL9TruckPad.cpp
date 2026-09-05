// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/CL9TruckPad.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "CL9.h"
#include "Character/CL9Character.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9RulesDef.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9PlayerState.h"

ACL9TruckPad::ACL9TruckPad()
{
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(MeshComponent);

	HealZone = CreateDefaultSubobject<UBoxComponent>(TEXT("HealZone"));
	HealZone->SetupAttachment(MeshComponent);
	// The BP child scales the pad mesh flat and wide — the zone keeps its own size
	HealZone->SetUsingAbsoluteScale(true);
	HealZone->SetBoxExtent(FVector(200.f, 200.f, 120.f));
	HealZone->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	HealZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HealZone->SetCollisionObjectType(ECC_WorldStatic);
	HealZone->SetCollisionResponseToAllChannels(ECR_Ignore);
	HealZone->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	HealZone->SetGenerateOverlapEvents(true);
}

void ACL9TruckPad::BeginPlay()
{
	Super::BeginPlay();

	// The ruling is the host's — clients only see the HP arrive
	SetActorTickEnabled(HasAuthority());
}

void ACL9TruckPad::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9RulesDef* Rules = Registry ? Registry->GetRules() : nullptr;
	if (Rules == nullptr)
	{
		return;
	}

	TArray<AActor*> Overlapping;
	HealZone->GetOverlappingActors(Overlapping, ACL9Character::StaticClass());

	TSet<ACL9Character*> Present;
	for (AActor* Actor : Overlapping)
	{
		ACL9Character* Character = Cast<ACL9Character>(Actor);
		const ACL9PlayerState* PlayerState = Character ? Character->GetPlayerState<ACL9PlayerState>() : nullptr;
		if (PlayerState == nullptr || PlayerState->IsDead() || PlayerState->Hp >= PlayerState->MaxHp)
		{
			continue;   // dead, unhurt or not rolled yet — nothing to heal
		}

		Present.Add(Character);
		float& Seconds = TimeOnPad.FindOrAdd(Character);
		Seconds += DeltaSeconds;
		if (Seconds >= Rules->TruckPadHealTime)
		{
			Character->HostHealFull();
			TimeOnPad.Remove(Character);
			UE_LOG(LogCL9, Log, TEXT("Truck pad healed '%s' after %.1fs."), *GetNameSafe(Character), Rules->TruckPadHealTime);
		}
	}

	// Stepping off forgets the count — the stay has to be continuous
	for (auto It = TimeOnPad.CreateIterator(); It; ++It)
	{
		if (!Present.Contains(It.Key().Get()))
		{
			It.RemoveCurrent();
		}
	}
}
