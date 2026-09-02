// Copyright Epic Games, Inc. All Rights Reserved.

#include "Monsters/CL9Monster.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Net/UnrealNetwork.h"
#include "CL9.h"
#include "Character/CL9Character.h"
#include "Character/CL9CharacterMovement.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9MonsterDef.h"
#include "Data/CL9RulesDef.h"
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

	/** How often a chase re-paths to the moving target, in seconds */
	constexpr float ChaseRepathInterval = 0.5f;

	/** Investigate hop spread around the last sensed spot, in cm */
	constexpr float SearchHopRadius = 300.f;

	/** Navmesh projection extent — bodies float Radius above the floor */
	const FVector NavProjectExtent(500.f, 500.f, 500.f);
}

ACL9Monster::ACL9Monster()
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicateMovement(true);

	// Walls (WorldStatic) block; pawns, items and other monsters only overlap —
	// the corridor threat is detection, never body blocking (2026-09-03 결정).
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
	DOREPLIFETIME(ACL9Monster, DetectGauge);
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
		const FVector Before = GetActorLocation();
		HostTickAI(DeltaSeconds);
		DisplaySpeed = (GetActorLocation() - Before).Size2D() / FMath::Max(DeltaSeconds, UE_KINDA_SMALL_NUMBER);
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
	const FVector PreviousSmoothed = SmoothedLocation;
	SmoothedLocation = FMath::VInterpTo(SmoothedLocation, GetActorLocation(), DeltaSeconds, SmoothingInterpSpeed);
	SmoothedRotation = FMath::RInterpTo(SmoothedRotation, GetActorRotation(), DeltaSeconds, SmoothingInterpSpeed);
	DisplaySpeed = (SmoothedLocation - PreviousSmoothed).Size2D() / FMath::Max(DeltaSeconds, UE_KINDA_SMALL_NUMBER);
	const float MeshZ = MonsterDef ? -MonsterDef->Radius : 0.f;
	MeshComponent->SetWorldLocationAndRotation(SmoothedLocation + FVector(0.f, 0.f, MeshZ), SmoothedRotation);
}

void ACL9Monster::HostTickAI(float DeltaSeconds)
{
	if (MonsterDef == nullptr || State == ECL9MonsterState::Dead)
	{
		return;
	}

	ChaseCooldownRemaining = FMath::Max(ChaseCooldownRemaining - DeltaSeconds, 0.f);
	SenseTick(DeltaSeconds);

	switch (State)
	{
	case ECL9MonsterState::Wander:
	case ECL9MonsterState::Patrol:
	case ECL9MonsterState::Return:
	{
		// A full gauge alerts — straight to the chase when the target is in
		// sight, to the last sensed spot otherwise (docs/systems/monster.md).
		if (DetectGauge >= 1.f && ChaseCooldownRemaining <= 0.f)
		{
			EnterState(bHadSight && SensedCharacter.IsValid() ? ECL9MonsterState::Chase : ECL9MonsterState::Investigate);
			break;
		}

		if (PathPoints.Num() == 0)
		{
			if (State == ECL9MonsterState::Return)
			{
				EnterState(ECL9MonsterState::Wander);
				break;
			}
			WanderWaitRemaining -= DeltaSeconds;
			if (WanderWaitRemaining <= 0.f)
			{
				StartNewWander();
			}
		}
		else if (MoveAlongPath(DeltaSeconds, State == ECL9MonsterState::Return ? MonsterDef->PatrolSpeed : MonsterDef->WanderSpeed))
		{
			PathPoints.Reset();
			if (State == ECL9MonsterState::Return)
			{
				EnterState(ECL9MonsterState::Wander);
			}
			else
			{
				WanderWaitRemaining = FMath::FRandRange(WanderDwellMin, WanderDwellMax);
			}
		}
		break;
	}

	case ECL9MonsterState::Investigate:
	{
		if (bHadSight && SensedCharacter.IsValid() && ChaseCooldownRemaining <= 0.f)
		{
			EnterState(ECL9MonsterState::Chase);
			break;
		}

		if (PathPoints.Num() > 0)
		{
			if (MoveAlongPath(DeltaSeconds, MonsterDef->PatrolSpeed))
			{
				PathPoints.Reset();
				SearchDwellRemaining = MonsterDef->SearchDwellTime;
			}
		}
		else
		{
			SearchDwellRemaining -= DeltaSeconds;
			if (SearchDwellRemaining <= 0.f)
			{
				if (SearchHopsRemaining <= 0)
				{
					EnterState(ECL9MonsterState::Return);
					break;
				}
				--SearchHopsRemaining;
				UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
				FNavLocation Hop;
				if (NavSystem != nullptr && NavSystem->GetRandomReachablePointInRadius(LastSensedLocation, SearchHopRadius, Hop))
				{
					StartPathTo(Hop.Location);
				}
				else
				{
					SearchDwellRemaining = MonsterDef->SearchDwellTime;
				}
			}
		}
		break;
	}

	case ECL9MonsterState::Chase:
	{
		ACL9Character* Target = SensedCharacter.Get();
		ChaseBudgetRemaining -= DeltaSeconds;

		const bool bSeesTarget = Target != nullptr && bHadSight;
		ChaseLoseRemaining = bSeesTarget ? MonsterDef->ChaseLoseTime : ChaseLoseRemaining - DeltaSeconds;

		// Dropping everything releases the chase past DeaggroRadius
		const float TargetDistance = Target ? FVector::Dist2D(GetActorLocation(), Target->GetActorLocation()) : 0.f;
		const bool bDeaggro = Target != nullptr
			&& MonsterDef->DeaggroRadius > 0.f
			&& TargetDistance > MonsterDef->DeaggroRadius
			&& !Target->IsCarryingAnything();

		const bool bOutOfHome = FVector::Dist2D(GetActorLocation(), HomeLocation) > MonsterDef->HomeRange;
		if (Target == nullptr || ChaseBudgetRemaining <= 0.f || ChaseLoseRemaining <= 0.f || bOutOfHome || bDeaggro)
		{
			EnterState(ECL9MonsterState::Break);
			break;
		}

		ChaseRepathRemaining -= DeltaSeconds;
		if (ChaseRepathRemaining <= 0.f || PathPoints.Num() == 0)
		{
			ChaseRepathRemaining = ChaseRepathInterval;
			StartPathTo(Target->GetActorLocation());
		}
		MoveAlongPath(DeltaSeconds, MonsterDef->ChaseSpeed);
		// Catching inside CatchRadius lands in 3e
		break;
	}

	case ECL9MonsterState::Break:
	{
		BreakRemaining -= DeltaSeconds;
		if (BreakRemaining <= 0.f)
		{
			EnterState(ECL9MonsterState::Return);
		}
		break;
	}

	default:
		// Asleep/WakeTelegraph/Stun/Dead arrive with 3e~3f
		break;
	}
}

void ACL9Monster::SenseTick(float DeltaSeconds)
{
	// Three inputs, one gauge (docs/systems/monster.md 감각 파이프라인):
	// sight with hysteresis, run noise from the action table, and proximity.
	ACL9Character* Best = nullptr;
	float BestDistance = TNumericLimits<float>::Max();
	bool bBestBySight = false;

	for (TActorIterator<ACL9Character> It(GetWorld()); It; ++It)
	{
		ACL9Character* Candidate = *It;
		const float Distance = FVector::Dist2D(GetActorLocation(), Candidate->GetActorLocation());

		const float SightRange = bHadSight ? MonsterDef->SightOut : MonsterDef->SightIn;
		const bool bInSight = Distance <= SightRange && HasLineOfSight(*Candidate);

		bool bHeard = false;
		if (const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>())
		{
			const UCL9RulesDef* Rules = GameInstance->GetDefRegistry() ? GameInstance->GetDefRegistry()->GetRules() : nullptr;
			// 걷기는 어그로 0 — only running carries (docs/plan/data/rules.md 소리)
			bHeard = Rules != nullptr
				&& Distance <= Rules->RunNoiseRadius
				&& Candidate->GetCL9CharacterMovement()->IsRunning();
		}

		if ((bInSight || bHeard) && Distance < BestDistance)
		{
			Best = Candidate;
			BestDistance = Distance;
			bBestBySight = bInSight;
		}
	}

	if (Best != nullptr)
	{
		SensedCharacter = Best;
		LastSensedLocation = Best->GetActorLocation();
		bHadSight = bBestBySight;
		SightGraceRemaining = MonsterDef->DetectGrace;

		const float Rate = BestDistance <= MonsterDef->DetectNearRadius ? MonsterDef->DetectRateNear : MonsterDef->DetectRateFar;
		DetectGauge = FMath::Min(DetectGauge + Rate * DeltaSeconds, 1.f);
	}
	else
	{
		bHadSight = false;
		SightGraceRemaining -= DeltaSeconds;
		if (SightGraceRemaining <= 0.f)
		{
			DetectGauge = FMath::Max(DetectGauge - MonsterDef->DetectDecay * DeltaSeconds, 0.f);
		}
	}
}

bool ACL9Monster::HasLineOfSight(const ACL9Character& Target) const
{
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CL9MonsterSight), false, this);
	QueryParams.AddIgnoredActor(&Target);
	return !GetWorld()->LineTraceTestByChannel(GetActorLocation(), Target.GetActorLocation(), ECC_Visibility, QueryParams);
}

void ACL9Monster::EnterState(ECL9MonsterState NewState)
{
	PathPoints.Reset();
	State = NewState;

	switch (NewState)
	{
	case ECL9MonsterState::Chase:
		ChaseBudgetRemaining = MonsterDef->ChaseBudget;
		ChaseLoseRemaining = MonsterDef->ChaseLoseTime;
		ChaseRepathRemaining = 0.f;
		break;
	case ECL9MonsterState::Investigate:
		SearchHopsRemaining = MonsterDef->SearchHops;
		SearchDwellRemaining = 0.f;
		StartPathTo(LastSensedLocation);
		break;
	case ECL9MonsterState::Break:
		BreakRemaining = MonsterDef->ChaseBreakTime;
		ChaseCooldownRemaining = MonsterDef->ChaseCooldown;
		DetectGauge = 0.f;
		break;
	case ECL9MonsterState::Return:
		StartPathTo(HomeLocation);
		break;
	case ECL9MonsterState::Wander:
		WanderWaitRemaining = FMath::FRandRange(WanderDwellMin, WanderDwellMax);
		break;
	default:
		break;
	}

	ApplyState();
}

void ACL9Monster::StartNewWander()
{
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSystem == nullptr)
	{
		return;
	}

	FNavLocation HomeOnNav;
	if (!NavSystem->ProjectPointToNavigation(HomeLocation, HomeOnNav, NavProjectExtent))
	{
		WanderWaitRemaining = 0.5f;
		return;
	}

	const float Range = FMath::Max(MonsterDef->HomeRange, 200.f);
	FNavLocation Target;
	if (!NavSystem->GetRandomReachablePointInRadius(HomeOnNav.Location, Range, Target))
	{
		WanderWaitRemaining = 0.5f;
		return;
	}

	if (!StartPathTo(Target.Location))
	{
		WanderWaitRemaining = 0.5f;
	}
}

bool ACL9Monster::StartPathTo(const FVector& WorldLocation)
{
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSystem == nullptr)
	{
		return false;
	}

	// The body sphere floats Radius above the floor, so raw locations miss the
	// navmesh — project both ends before pathing.
	FNavLocation StartOnNav;
	FNavLocation TargetOnNav;
	if (!NavSystem->ProjectPointToNavigation(GetActorLocation(), StartOnNav, NavProjectExtent)
		|| !NavSystem->ProjectPointToNavigation(WorldLocation, TargetOnNav, NavProjectExtent))
	{
		return false;
	}

	const UNavigationPath* Path = NavSystem->FindPathToLocationSynchronously(GetWorld(), StartOnNav.Location, TargetOnNav.Location);
	if (Path == nullptr || Path->PathPoints.Num() < 2)
	{
		return false;
	}

	PathPoints = Path->PathPoints;
	PathIndex = 1;
	return true;
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
	// Animation and sound switch here once presentation lands (3e~) — the
	// replicated state is the subscription point (docs/systems/animation.md).
}
