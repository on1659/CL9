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
	// BP 자식 클래스가 패드 메시를 넓적하게 스케일하므로, 존은 자기 크기를 그대로 유지한다
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

	// 판정은 호스트의 몫이다 — 클라이언트는 체력값이 도착하는 것만 본다
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
			continue;   // 죽었거나, 다치지 않았거나, 아직 롤되지 않았거나 — 회복할 게 없다
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

	// 밖으로 나가면 카운트를 잊는다 — 머무름은 끊기지 않고 이어져야 한다
	for (auto It = TimeOnPad.CreateIterator(); It; ++It)
	{
		if (!Present.Contains(It.Key().Get()))
		{
			It.RemoveCurrent();
		}
	}
}
