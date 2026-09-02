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
#include "Data/CL9ItemDef.h"
#include "Data/CL9MonsterDef.h"
#include "Data/CL9RulesDef.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameMode.h"

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

	/** Attack pause after landing a catch, in seconds — presentation constant like the dwell above */
	constexpr float CatchPauseSeconds = 0.8f;

	/** A gaze counts as aimed at the sleeper above this view dot (docs WakeSightRadius 기상 보류) */
	constexpr float GazeMinAimDot = 0.75f;

	/** Walking counts as movement above this speed, in cm/s (goose WakeRadius trigger) */
	constexpr float WakeMoveMinSpeed = 10.f;
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

		// Sleepy species (goose) start asleep — the map spawner may override later (⑤)
		if (Def->SleepTime > 0.f)
		{
			EnterState(ECL9MonsterState::Asleep);
		}
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
	CatchCooldownRemaining = FMath::Max(CatchCooldownRemaining - DeltaSeconds, 0.f);
	AlertCooldownRemaining = FMath::Max(AlertCooldownRemaining - DeltaSeconds, 0.f);

	// Rage pins the gauge for its whole run — hunt mode after a death scream
	// bypasses ChaseCooldown and never calms down early (docs/systems/monster.md 경보).
	if (RageRemaining > 0.f)
	{
		RageRemaining -= DeltaSeconds;
		DetectGauge = 1.f;
	}

	// Sleep, the wake telegraph and stun have no working senses
	const bool bSensesAwake = State != ECL9MonsterState::Asleep
		&& State != ECL9MonsterState::WakeTelegraph
		&& State != ECL9MonsterState::Stun;
	if (bSensesAwake)
	{
		SenseTick(DeltaSeconds);
	}

	const bool bAlertGateOpen = ChaseCooldownRemaining <= 0.f || RageRemaining > 0.f;

	switch (State)
	{
	case ECL9MonsterState::Asleep:
	{
		// Wake triggers: walking inside WakeRadius (the walking-is-silent
		// exception), running inside HearRunRadius, or a pending alarm spot.
		bool bTriggered = PendingInvestigateSpot.IsSet();
		for (TActorIterator<ACL9Character> It(GetWorld()); !bTriggered && It; ++It)
		{
			const ACL9Character* Candidate = *It;
			if (Candidate->IsDead())
			{
				continue;
			}
			const float Distance = FVector::Dist2D(GetActorLocation(), Candidate->GetActorLocation());
			const float Speed = Candidate->GetVelocity().Size2D();
			const bool bWalkWake = MonsterDef->WakeRadius > 0.f && Distance <= MonsterDef->WakeRadius && Speed > WakeMoveMinSpeed;
			const bool bRunWake = MonsterDef->HearRunRadius > 0.f && Distance <= MonsterDef->HearRunRadius
				&& Candidate->GetCL9CharacterMovement()->IsRunning();
			bTriggered = bWalkWake || bRunWake;
		}

		WakeRetryRemaining = FMath::Max(WakeRetryRemaining - DeltaSeconds, 0.f);
		if (bTriggered && WakeRetryRemaining <= 0.f)
		{
			TryWake();
		}
		break;
	}

	case ECL9MonsterState::WakeTelegraph:
	{
		// Every player sees the flap-and-cry before the monster is live
		WakeTelegraphRemaining -= DeltaSeconds;
		if (WakeTelegraphRemaining <= 0.f)
		{
			if (PendingInvestigateSpot.IsSet())
			{
				LastSensedLocation = PendingInvestigateSpot.GetValue();
				PendingInvestigateSpot.Reset();
				EnterState(ECL9MonsterState::Investigate);
			}
			else
			{
				EnterState(ECL9MonsterState::Wander);
			}
		}
		break;
	}

	case ECL9MonsterState::Wander:
	case ECL9MonsterState::Patrol:
	case ECL9MonsterState::Return:
	{
		// Sleepy species drift back to sleep after SleepTime calm seconds
		if (State == ECL9MonsterState::Wander && MonsterDef->SleepTime > 0.f && DetectGauge <= 0.f)
		{
			CalmSeconds += DeltaSeconds;
			if (CalmSeconds >= MonsterDef->SleepTime)
			{
				EnterState(ECL9MonsterState::Asleep);
				break;
			}
		}
		else
		{
			CalmSeconds = 0.f;
		}

		// A full gauge alerts — straight to the chase when the target is in
		// sight, to the last sensed spot otherwise (docs/systems/monster.md).
		if (DetectGauge >= 1.f && bAlertGateOpen)
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
		if (bHadSight && SensedCharacter.IsValid() && bAlertGateOpen)
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
		if (Target == nullptr || Target->IsDead() || ChaseBudgetRemaining <= 0.f || ChaseLoseRemaining <= 0.f || bOutOfHome || bDeaggro)
		{
			EnterState(ECL9MonsterState::Break);
			break;
		}

		// The alarmist stands and cries before the alarm lands (goose AlertWindup)
		if (AlertWindupRemaining > 0.f)
		{
			AlertWindupRemaining -= DeltaSeconds;
			if (AlertWindupRemaining <= 0.f)
			{
				FirePackAlarm();
				bPackAlarmFired = true;
				AlertCooldownRemaining = MonsterDef->AlertCooldown;
			}
			break;
		}

		// The catch: inside CatchRadius with the KillCooldown reloaded
		if (MonsterDef->CatchRadius > 0.f && TargetDistance <= MonsterDef->CatchRadius && CatchCooldownRemaining <= 0.f)
		{
			HostCatch(*Target);
			break;
		}

		ChaseRepathRemaining -= DeltaSeconds;
		if (ChaseRepathRemaining <= 0.f || PathPoints.Num() == 0)
		{
			ChaseRepathRemaining = ChaseRepathInterval;
			StartPathTo(Target->GetActorLocation());
		}
		MoveAlongPath(DeltaSeconds, MonsterDef->ChaseSpeed);
		break;
	}

	case ECL9MonsterState::Catch:
	{
		CatchPauseRemaining -= DeltaSeconds;
		if (CatchPauseRemaining <= 0.f)
		{
			// Straight back to the chase without EnterState — the budget and
			// lose timers keep running, a catch never refills them.
			State = ECL9MonsterState::Chase;
			ChaseRepathRemaining = 0.f;
			ApplyState();
		}
		break;
	}

	case ECL9MonsterState::Stun:
	{
		StunRemaining -= DeltaSeconds;
		if (StunRemaining <= 0.f)
		{
			// A hit is a provocation — the attacker gets chased, cooldown or not
			if (SensedCharacter.IsValid() && !SensedCharacter->IsDead())
			{
				EnterState(ECL9MonsterState::Chase);
			}
			else
			{
				EnterState(ECL9MonsterState::Wander);
			}
		}
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
		// Dead is guarded at the top; nothing else remains
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
		if (Candidate->IsDead())
		{
			continue;
		}
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
		// The first alert of an episode wakes the pack — the alarmist (goose)
		// cries after a wind-up, everyone else alerts silently right away
		// (docs/systems/monster.md 경보).
		if (!bPackAlarmFired && MonsterDef->PackAlarmRadius > 0.f)
		{
			if (MonsterDef->bAlertsOthers)
			{
				if (AlertCooldownRemaining <= 0.f)
				{
					AlertWindupRemaining = FMath::Max(MonsterDef->AlertWindup, UE_KINDA_SMALL_NUMBER);
				}
			}
			else
			{
				FirePackAlarm();
				bPackAlarmFired = true;
			}
		}
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
		bPackAlarmFired = false;
		AlertWindupRemaining = 0.f;
		break;
	case ECL9MonsterState::Return:
		StartPathTo(HomeLocation);
		break;
	case ECL9MonsterState::Wander:
		WanderWaitRemaining = FMath::FRandRange(WanderDwellMin, WanderDwellMax);
		break;
	case ECL9MonsterState::Asleep:
		DetectGauge = 0.f;
		CalmSeconds = 0.f;
		WakeRetryRemaining = 0.f;
		break;
	case ECL9MonsterState::WakeTelegraph:
		WakeTelegraphRemaining = MonsterDef->WakeTelegraph;
		break;
	case ECL9MonsterState::Catch:
		CatchPauseRemaining = CatchPauseSeconds;
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

void ACL9Monster::HostCatch(ACL9Character& Target)
{
	// One catch, then the KillCooldown reload — three quick hits must be
	// impossible (docs/systems/monster.md 잡기 = 공격).
	Target.OnDamage(MonsterDef->CatchDamage, this);
	CatchCooldownRemaining = MonsterDef->KillCooldown;
	EnterState(ECL9MonsterState::Catch);
}

void ACL9Monster::OnDamage(int32 Amount, AActor* Source)
{
	if (!HasAuthority() || MonsterDef == nullptr || State == ECL9MonsterState::Dead)
	{
		return;
	}

	Hp = FMath::Max(Hp - Amount, 0);
	if (Hp <= 0)
	{
		Die(Source);
		return;
	}

	// A non-lethal hit staggers, then the attacker gets hunted
	if (ACL9Character* Attacker = Cast<ACL9Character>(Source))
	{
		SensedCharacter = Attacker;
		LastSensedLocation = Attacker->GetActorLocation();
		bHadSight = true;
		DetectGauge = 1.f;
	}
	StunRemaining = MonsterDef->HitStunMs / 1000.f;
	EnterState(ECL9MonsterState::Stun);
}

void ACL9Monster::Die(AActor* Source)
{
	EnterState(ECL9MonsterState::Dead);
	DetectGauge = 0.f;

	// The hunting reward drops as items, never direct ether (docs/plan/data/monster/common.md)
	if (ACL9GameMode* GameMode = GetWorld()->GetAuthGameMode<ACL9GameMode>())
	{
		const float FloorZ = GetActorLocation().Z - MonsterDef->Radius;
		for (const FDropItem& Drop : MonsterDef->DropItems)
		{
			if (FMath::FRand() > Drop.DropChance)
			{
				continue;
			}
			const int32 Quantity = FMath::RandRange(Drop.MinQuantity, Drop.MaxQuantity);
			for (int32 Index = 0; Index < Quantity; ++Index)
			{
				const FVector Sideways = FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f).Vector() * FMath::FRandRange(30.f, 90.f);
				GameMode->SpawnItem(Drop.ItemAlias, Drop.ItemSubAlias,
					FTransform(FVector(GetActorLocation().X + Sideways.X, GetActorLocation().Y + Sideways.Y, FloorZ + 25.f)));
			}
		}
	}

	// The death scream: direct hearers rage for their RageTime, and the rage
	// spreads once more through RevengeRadius/RevengeTime. Zone checks arrive
	// with the map's zones (⑤) — the flat map is one zone.
	TArray<ACL9Monster*> Hearers;
	for (TActorIterator<ACL9Monster> It(GetWorld()); It; ++It)
	{
		ACL9Monster* Other = *It;
		if (Other == this || Other->State == ECL9MonsterState::Dead || Other->MonsterDef == nullptr)
		{
			continue;
		}
		if (FVector::Dist2D(GetActorLocation(), Other->GetActorLocation()) <= MonsterDef->KillAlarmRadius)
		{
			Hearers.Add(Other);
		}
	}
	for (ACL9Monster* Hearer : Hearers)
	{
		Hearer->ReceiveKillAlarm(GetActorLocation(), Hearer->MonsterDef->RageTime);
	}
	for (TActorIterator<ACL9Monster> It(GetWorld()); It; ++It)
	{
		ACL9Monster* Other = *It;
		if (Other == this || Other->State == ECL9MonsterState::Dead || Other->MonsterDef == nullptr || Hearers.Contains(Other))
		{
			continue;
		}
		for (const ACL9Monster* Hearer : Hearers)
		{
			if (FVector::Dist2D(Hearer->GetActorLocation(), Other->GetActorLocation()) <= Hearer->MonsterDef->RevengeRadius)
			{
				Other->ReceiveKillAlarm(GetActorLocation(), Other->MonsterDef->RevengeTime);
				break;
			}
		}
	}

	UE_LOG(LogCL9, Log, TEXT("'%s' (%s) died to '%s' — kill alarm reached %d monsters."),
		*GetNameSafe(this), *GetAlias().ToString(), *GetNameSafe(Source), Hearers.Num());
}

void ACL9Monster::FirePackAlarm()
{
	const FVector AlarmSpot = SensedCharacter.IsValid() ? LastSensedLocation : GetActorLocation();
	for (TActorIterator<ACL9Monster> It(GetWorld()); It; ++It)
	{
		ACL9Monster* Other = *It;
		if (Other == this || Other->State == ECL9MonsterState::Dead || Other->MonsterDef == nullptr)
		{
			continue;
		}
		if (FVector::Dist2D(GetActorLocation(), Other->GetActorLocation()) <= MonsterDef->PackAlarmRadius)
		{
			Other->ReceivePackAlarm(AlarmSpot);
		}
	}
}

void ACL9Monster::ReceivePackAlarm(const FVector& AlarmLocation)
{
	if (!HasAuthority() || MonsterDef == nullptr)
	{
		return;
	}

	switch (State)
	{
	case ECL9MonsterState::Asleep:
		PendingInvestigateSpot = AlarmLocation;
		break;
	case ECL9MonsterState::Wander:
	case ECL9MonsterState::Patrol:
	case ECL9MonsterState::Return:
		// 경보는 새 경계 — ChaseCooldown still gates it (docs/systems/monster.md)
		if (ChaseCooldownRemaining <= 0.f)
		{
			LastSensedLocation = AlarmLocation;
			EnterState(ECL9MonsterState::Investigate);
		}
		break;
	default:
		break;
	}
}

void ACL9Monster::ReceiveKillAlarm(const FVector& DeathLocation, float HuntSeconds)
{
	if (!HasAuthority() || MonsterDef == nullptr || State == ECL9MonsterState::Dead)
	{
		return;
	}

	RageRemaining = FMath::Max(RageRemaining, HuntSeconds);
	DetectGauge = 1.f;
	LastSensedLocation = DeathLocation;

	switch (State)
	{
	case ECL9MonsterState::Asleep:
		PendingInvestigateSpot = DeathLocation;
		break;
	case ECL9MonsterState::Wander:
	case ECL9MonsterState::Patrol:
	case ECL9MonsterState::Return:
	case ECL9MonsterState::Break:
	case ECL9MonsterState::Investigate:
		// Rage ignores ChaseCooldown by design (경보 문서: 쿨다운은 만능 방패가 아니다)
		EnterState(bHadSight && SensedCharacter.IsValid() ? ECL9MonsterState::Chase : ECL9MonsterState::Investigate);
		break;
	default:
		break;
	}
}

void ACL9Monster::TryWake()
{
	// A watched sleeper stays down — no waking inside a player's gaze
	// (WakeSightRadius, retried every WakeRetryTime).
	if (IsHeldByGaze())
	{
		WakeRetryRemaining = MonsterDef->WakeRetryTime;
		return;
	}
	EnterState(ECL9MonsterState::WakeTelegraph);
}

bool ACL9Monster::IsHeldByGaze() const
{
	if (MonsterDef->WakeSightRadius <= 0.f)
	{
		return false;
	}

	for (TActorIterator<ACL9Character> It(GetWorld()); It; ++It)
	{
		const ACL9Character* Candidate = *It;
		if (Candidate->IsDead())
		{
			continue;
		}
		if (FVector::Dist2D(GetActorLocation(), Candidate->GetActorLocation()) > MonsterDef->WakeSightRadius)
		{
			continue;
		}
		const FVector ToMonster = (GetActorLocation() - Candidate->GetActorLocation()).GetSafeNormal();
		if (FVector::DotProduct(Candidate->GetBaseAimRotation().Vector(), ToMonster) >= GazeMinAimDot
			&& HasLineOfSight(*Candidate))
		{
			return true;
		}
	}
	return false;
}

void ACL9Monster::OnRep_State()
{
	ApplyState();
}

void ACL9Monster::ApplyState()
{
	// The replicated state is the subscription point for presentation
	// (docs/systems/animation.md). Until species ABPs grow death/stun states,
	// a corpse freezes its pose and tips onto its side — readable on every
	// machine without a single animation asset.
	if (MeshComponent != nullptr)
	{
		const bool bDead = State == ECL9MonsterState::Dead;
		MeshComponent->GlobalAnimRateScale = bDead ? 0.f : 1.f;
		MeshComponent->SetRelativeRotation(bDead ? FRotator(0.f, 0.f, 90.f) : FRotator::ZeroRotator);
	}
}
