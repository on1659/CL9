// Copyright Epic Games, Inc. All Rights Reserved.

#include "Monsters/CL9Monster.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "Animation/AnimInstance.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Net/UnrealNetwork.h"
#include "CL9.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9MonsterDef.h"
#include "GameMode/CL9GameInstance.h"

namespace
{
	/** A waypoint counts as reached inside this distance, in cm */
	constexpr float WaypointTolerance = 25.f;

	/** Idle dwell between wander legs, in seconds */
	constexpr float WanderDwellMin = 1.f;
	constexpr float WanderDwellMax = 3.f;

	/** Client mesh smoothing speed against 20Hz root snaps */
	constexpr float SmoothingInterpSpeed = 12.f;
}

ACL9Monster::ACL9Monster()
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicateMovement(true);

	// Walls (WorldStatic) block; pawns, items and other monsters only overlap —
	// body separation is SeparationDist steering, not physics (3c 이후).
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Overlap);
	SphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	SphereComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	SetRootComponent(SphereComponent);

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(SphereComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACL9Monster::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		HomeLocation = GetActorLocation();
		HeadingYaw = GetActorRotation().Yaw;
	}
}

void ACL9Monster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACL9Monster, State);
	DOREPLIFETIME(ACL9Monster, Hp);
}

void ACL9Monster::LoadData()
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9MonsterDef* Def = Registry ? Registry->FindMonster(GetAlias(), GetSubAlias()) : nullptr;

	if (Def == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("'%s': monster row '%s'/'%s' is missing or broken — visuals stay stale (docs/data/schema.md 로드 검사)."),
			*GetNameSafe(this), *GetAlias().ToString(), *GetSubAlias().ToString());
		return;
	}
	MonsterDef = Def;

	SphereComponent->SetSphereRadius(Def->Radius);
	MeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -Def->Radius));

	if (USkeletalMesh* Mesh = Def->SkeletalMesh.LoadSynchronous())
	{
		MeshComponent->SetSkeletalMesh(Mesh);
	}
	if (UClass* AnimClass = Def->AnimBP.LoadSynchronous())
	{
		MeshComponent->SetAnimInstanceClass(AnimClass);
	}

	for (UCL9Gimmick* Gimmick : Def->Gimmicks)
	{
		if (Gimmick != nullptr)
		{
			Gimmick->OnActorLoaded(this);
		}
	}

	// The individual's HP rolls once, on the host (conventions § 4)
	if (HasAuthority() && Hp == 0)
	{
		Hp = FMath::RandRange(Def->HpMin, Def->HpMax);
	}
}

void ACL9Monster::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		HostTickAI(DeltaSeconds);
		return;
	}

	// Plain actors snap on 20Hz movement replication — the visual mesh chases
	// the root instead so clients see smooth motion (docs/systems/monster.md 3c).
	if (!bSmoothingInitialized)
	{
		SmoothedLocation = GetActorLocation();
		SmoothedRotation = GetActorRotation();
		bSmoothingInitialized = true;
	}
	SmoothedLocation = FMath::VInterpTo(SmoothedLocation, GetActorLocation(), DeltaSeconds, SmoothingInterpSpeed);
	SmoothedRotation = FMath::RInterpTo(SmoothedRotation, GetActorRotation(), DeltaSeconds, SmoothingInterpSpeed);
	const float MeshZ = MonsterDef ? -MonsterDef->Radius : 0.f;
	MeshComponent->SetWorldLocationAndRotation(SmoothedLocation + FVector(0.f, 0.f, MeshZ), SmoothedRotation);
}

void ACL9Monster::HostTickAI(float DeltaSeconds)
{
	if (MonsterDef == nullptr || State == ECL9MonsterState::Dead)
	{
		return;
	}

	switch (State)
	{
	case ECL9MonsterState::Wander:
		if (PathPoints.Num() == 0)
		{
			WanderWaitRemaining -= DeltaSeconds;
			if (WanderWaitRemaining <= 0.f)
			{
				StartNewWander();
			}
		}
		else if (MoveAlongPath(DeltaSeconds, MonsterDef->WanderSpeed))
		{
			PathPoints.Reset();
			WanderWaitRemaining = FMath::FRandRange(WanderDwellMin, WanderDwellMax);
		}
		break;

	default:
		// Later steps (3d~3f) fill the remaining states
		break;
	}
}

void ACL9Monster::StartNewWander()
{
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSystem == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("%s: no navigation system — wander stalled."), *GetNameSafe(this));
		return;
	}

	// The body sphere floats Radius above the floor, so raw locations miss the
	// navmesh — project home and start down onto it with a generous extent.
	const FVector ProjectExtent(500.f, 500.f, 500.f);
	FNavLocation HomeOnNav;
	FNavLocation StartOnNav;
	if (!NavSystem->ProjectPointToNavigation(HomeLocation, HomeOnNav, ProjectExtent)
		|| !NavSystem->ProjectPointToNavigation(GetActorLocation(), StartOnNav, ProjectExtent))
	{
		UE_LOG(LogCL9, Warning, TEXT("%s: not over the navmesh at %s."), *GetNameSafe(this), *GetActorLocation().ToCompactString());
		WanderWaitRemaining = 0.5f;
		return;
	}

	const float Range = FMath::Max(MonsterDef->HomeRange, 200.f);
	FNavLocation Target;
	if (!NavSystem->GetRandomReachablePointInRadius(HomeOnNav.Location, Range, Target))
	{
		UE_LOG(LogCL9, Warning, TEXT("%s: no reachable point near home %s."), *GetNameSafe(this), *HomeOnNav.Location.ToCompactString());
		WanderWaitRemaining = 0.5f;
		return;
	}

	const UNavigationPath* Path = NavSystem->FindPathToLocationSynchronously(GetWorld(), StartOnNav.Location, Target.Location);
	if (Path == nullptr || Path->PathPoints.Num() < 2)
	{
		UE_LOG(LogCL9, Warning, TEXT("%s: pathing to %s failed."), *GetNameSafe(this), *Target.Location.ToCompactString());
		WanderWaitRemaining = 0.5f;
		return;
	}

	PathPoints = Path->PathPoints;
	PathIndex = 1;
}

bool ACL9Monster::MoveAlongPath(float DeltaSeconds, float Speed)
{
	if (!PathPoints.IsValidIndex(PathIndex))
	{
		return true;
	}

	FVector ToTarget = PathPoints[PathIndex] - GetActorLocation();
	ToTarget.Z = 0.f;
	if (ToTarget.Size() < WaypointTolerance)
	{
		++PathIndex;
		return !PathPoints.IsValidIndex(PathIndex);
	}

	// Turn model: the heading chases the target under TurnRate, and the wider
	// the remaining turn, the slower the run (TurnDrag) — corners buy time.
	const float DesiredYaw = ToTarget.Rotation().Yaw;
	HeadingYaw = FMath::FixedTurn(HeadingYaw, DesiredYaw, MonsterDef->TurnRate * DeltaSeconds);
	const float RemainingTurn = FMath::Abs(FMath::FindDeltaAngleDegrees(HeadingYaw, DesiredYaw));
	const float DragScale = 1.f - MonsterDef->TurnDrag * FMath::Clamp(RemainingTurn / 90.f, 0.f, 1.f);

	const FVector Step = FRotator(0.f, HeadingYaw, 0.f).Vector() * Speed * FMath::Max(DragScale, 0.1f) * DeltaSeconds;
	FHitResult Hit;
	AddActorWorldOffset(Step, true, &Hit);
	SetActorRotation(FRotator(0.f, HeadingYaw, 0.f));

	if (Hit.bBlockingHit)
	{
		// Wedged against something the path missed — replan next tick
		PathPoints.Reset();
		return true;
	}
	return false;
}

void ACL9Monster::OnRep_State()
{
	ApplyState();
}

void ACL9Monster::ApplyState()
{
	// Animation and sound switch here once presentation lands (3d~) — the
	// replicated state is the subscription point (docs/systems/animation.md).
}
