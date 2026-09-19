// Copyright Epic Games, Inc. All Rights Reserved.

#include "Monsters/CL9Monster.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerState.h"
#include "Materials/MaterialInterface.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "CL9.h"
#include "Character/CL9Character.h"
#include "Character/CL9CharacterMovement.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ItemDef.h"
#include "Data/CL9MonsterDef.h"
#include "Data/CL9RulesDef.h"
#include "Data/CL9SoundTable.h"
#include "Data/CL9WaveTable.h"
#include "Items/CL9Item.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameMode.h"
#include "GameMode/CL9GameState.h"

namespace
{
	/** 이 거리 이내면 경유점에 도달한 것으로 친다, cm 단위 */
	constexpr float WaypointTolerance = 25.f;

	/** 배회 구간 사이의 대기 시간, 초 단위 */
	constexpr float WanderDwellMin = 1.f;
	constexpr float WanderDwellMax = 3.f;

	/** 20Hz 루트 스냅에 대한 클라이언트 메시 보간 속도 */
	constexpr float SmoothingInterpSpeed = 12.f;

	/** 추격 중 움직이는 대상으로 경로를 다시 잡는 주기, 초 단위 */
	constexpr float ChaseRepathInterval = 0.5f;

	/** 마지막 감지 지점 주변에서 조사 이동이 흩어지는 범위, cm 단위 */
	constexpr float SearchHopRadius = 300.f;

	/** navmesh 투영 범위 — 몸체는 바닥 위로 Radius만큼 떠 있다 */
	const FVector NavProjectExtent(500.f, 500.f, 500.f);

	/** 잡기 성공 뒤의 공격 휴지기, 초 단위 — 위의 대기 시간과 같은 연출용 상수다 */
	constexpr float CatchPauseSeconds = 0.8f;

	/** 이 시선 내적값 이상이면 잠든 대상을 조준한 것으로 친다 (docs WakeSightRadius 기상 보류) */
	constexpr float GazeMinAimDot = 0.75f;

	/** 이 속도 이상이면 걷는 것으로 쳐서 움직임으로 판정한다, cm/s 단위 (거위 WakeRadius 트리거) */
	constexpr float WakeMoveMinSpeed = 10.f;
}

ACL9Monster::ACL9Monster()
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicateMovement(true);
	Kind = ECL9ActorKind::Monster;

	// 벽(WorldStatic)은 막지만, 폰·아이템·다른 몬스터는 서로 겹쳐 지나간다 —
	// 복도의 위협은 몸으로 막는 게 아니라 발각이다 (2026-09-03 결정).
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
	DOREPLIFETIME(ACL9Monster, StateStartServerTime);
	DOREPLIFETIME(ACL9Monster, bPushStun);
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
	// 액터가 정면을 보게 한다 — 팩의 모델 축은 행 값으로 보정된다
	MeshComponent->SetRelativeRotation(FRotator(0.f, Def->MeshYawOffset, 0.f));

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

	// 개체별 HP는 호스트에서 한 번만 굴린다 (conventions § 4)
	if (HasAuthority() && Hp == 0)
	{
		Hp = FMath::RandRange(Def->HpMin, Def->HpMax);

		// 잠 많은 종(거위)은 잠든 채로 시작한다 — 맵 스포너가 나중에(⑤) 이를 덮어쓸 수 있다
		if (Def->SleepTime > 0.f)
		{
			EnterState(ECL9MonsterState::Asleep);
		}
	}
}

FText ACL9Monster::GetDisplayName() const
{
	return (MonsterDef != nullptr && !MonsterDef->DisplayName.IsEmpty()) ? MonsterDef->DisplayName : Super::GetDisplayName();
}

void ACL9Monster::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 타격 플래시·소리·퍼프·히트스톱은 호스트와 클라 모두에서 같은 복제 시계로 굴러간다 — 아래 호스트 분기가 먼저 돌아가
	// 버리기 전에 부른다
	TickHitFlash();
	TickHitFeel();

	if (HasAuthority())
	{
		const FVector Before = GetActorLocation();
		HostTickDeathLaunch(DeltaSeconds);
		HostTickAI(DeltaSeconds);
		DisplaySpeed = (GetActorLocation() - Before).Size2D() / FMath::Max(DeltaSeconds, UE_KINDA_SMALL_NUMBER);
		TickFootstep();
		return;
	}

	// 일반 액터는 20Hz 이동 복제로 인해 위치가 툭툭 튄다 — 대신 시각 메시가
	// 루트를 뒤쫓아가게 해서 클라이언트에게 매끄러운 움직임으로 보이게 한다
	// (docs/systems/monster.md 3c).
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
	const float YawOffset = MonsterDef ? MonsterDef->MeshYawOffset : 0.f;
	MeshComponent->SetWorldLocationAndRotation(SmoothedLocation + FVector(0.f, 0.f, MeshZ), SmoothedRotation + FRotator(0.f, YawOffset, 0.f));
	TickFootstep();
}

void ACL9Monster::HostTickDeathLaunch(float DeltaSeconds)
{
	if (DeathLaunchVelocity.IsZero())
	{
		return;
	}
	// Dead 는 HostTickAI 가 먼저 걸러 이동이 없다 — 죽음 비행만 여기서 적분한다. 수직은 월드 중력이 끌어내린다.
	// 몸체 구(AddActorWorldOffset 스윕)와 WorldStatic 오브젝트 질의는 둘 다 PIE 에서 바닥을 지나쳤다(2026-09-18 리드 측정 —
	// Lvl_Flat 바닥 `Floor_0` 은 프로필 BlockAllDynamic = 오브젝트 타입 WorldDynamic 이고 맵 조각도 프로필이 제각각일 수 있다).
	// 그래서 **Visibility 채널** 선 트레이스로 벽(수평)·바닥(수직)을 검사하고, 세상 지형이 아닌 것(폰·아이템·다른 몬스터)은
	// 맞아도 무시하고 그 뒤를 다시 긋는다
	const float Radius = MonsterDef ? MonsterDef->Radius : 0.f;
	DeathLaunchVelocity.Z += GetWorld()->GetGravityZ() * DeltaSeconds;
	const FVector Start = GetActorLocation();
	FVector Next = Start + DeathLaunchVelocity * DeltaSeconds;

	auto TraceWorld = [World = GetWorld(), Self = this](const FVector& From, const FVector& To, FHitResult& OutHit) -> bool
	{
		FCollisionQueryParams Params(SCENE_QUERY_STAT(CL9DeathLaunch), false, Self);
		// 앞을 가린 폰·아이템·몬스터를 하나씩 무시 목록에 넣고 다시 긋는다 — 시체가 사람 몸이나 계란 위에서 멈추지 않게
		for (int32 Retry = 0; Retry < 4; ++Retry)
		{
			if (!World->LineTraceSingleByChannel(OutHit, From, To, ECC_Visibility, Params))
			{
				return false;
			}
			AActor* HitActor = OutHit.GetActor();
			if (HitActor == nullptr
				|| (!HitActor->IsA<ACL9Character>() && !HitActor->IsA<ACL9Item>() && !HitActor->IsA<ACL9Monster>()))
			{
				return true;
			}
			Params.AddIgnoredActor(HitActor);
		}
		return false;
	};
	FHitResult Hit;

	// 벽: 지금 자리 → 다음 자리의 수평 선분이 막히면 그 자리에서 수평 속도 0 (남은 낙하만 이어진다)
	if (!FVector2D(DeathLaunchVelocity).IsNearlyZero())
	{
		const FVector HorizontalEnd(Next.X, Next.Y, Start.Z);
		if (TraceWorld(Start, HorizontalEnd, Hit))
		{
			Next.X = Start.X;
			Next.Y = Start.Y;
			DeathLaunchVelocity.X = 0.f;
			DeathLaunchVelocity.Y = 0.f;
		}
	}

	// 바닥: 몸체 중심에서 다음 자리의 발밑(중심 − Radius)까지 내려 그으며 닿으면 발이 바닥에 오도록 스냅하고 비행 끝
	if (DeathLaunchVelocity.Z < 0.f)
	{
		const FVector FloorStart(Next.X, Next.Y, Start.Z);
		const FVector FloorEnd(Next.X, Next.Y, Next.Z - Radius);
		if (TraceWorld(FloorStart, FloorEnd, Hit))
		{
			Next.Z = Hit.ImpactPoint.Z + Radius;
			DeathLaunchVelocity = FVector::ZeroVector;
		}
	}

	SetActorLocation(Next, false, nullptr, ETeleportType::TeleportPhysics);
}

void ACL9Monster::TickHitFeel()
{
	if (MeshComponent == nullptr)
	{
		return;
	}

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9RulesDef* Rules = Registry ? Registry->GetRules() : nullptr;
	if (Rules == nullptr)
	{
		return;
	}

	// TickHitFlash 와 같은 "맞은 직후" 창 — 그 창의 상승 에지가 타격의 순간이다. 창 안에서 상태 시각만 바뀌면 경직 중에
	// 다시 맞은 것(둘이 동시에 때림)이라 그것도 에지다
	const bool bStruckState = State == ECL9MonsterState::Stun || State == ECL9MonsterState::Dead;
	const float Elapsed = GetStateElapsed();
	const bool bInWindow = bStruckState && Elapsed < Rules->HitFlashTime;
	const bool bEdge = bInWindow && (!bHitFeelInWindow || StateStartServerTime != LastSeenStateStartTime);
	const bool bPrimed = bHitFeelPrimed;
	bHitFeelPrimed = true;
	bHitFeelInWindow = bInWindow;
	LastSeenStateStartTime = StateStartServerTime;

	// 접속 직후·스폰 직후의 첫 틱은 기준값만 적는다 — 이미 지나간 타격을 되풀이하지 않는다
	if (bEdge && bPrimed)
	{
		const bool bDeath = State == ECL9MonsterState::Dead;
		const FVector Center = MeshComponent->Bounds.Origin;
		CL9Sound::PlayAt(this, CL9SoundNames::Hit, Center);
		if (bDeath)
		{
			CL9Sound::PlayAt(this, CL9SoundNames::MonsterDeath, Center);
		}
		SpawnHitPuff(bDeath);
	}

	// 히트스톱: 진입 뒤 HitStopTime 동안 메시가 그 프레임에 머문다. 판정·이동·AI는 그대로 흐른다 (전역 시간 지연 금지)
	const bool bWantStop = bStruckState && Elapsed < Rules->HitStopTime;
	if (bWantStop != bHitStopOn)
	{
		bHitStopOn = bWantStop;
		MeshComponent->bPauseAnims = bWantStop;
	}
}

void ACL9Monster::SpawnHitPuff(bool bDeath)
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9RulesDef* Rules = Registry ? Registry->GetRules() : nullptr;
	if (Rules == nullptr)
	{
		return;
	}
	if (!bHitPuffLoaded)
	{
		bHitPuffLoaded = true;
		HitPuffSystem = Rules->HitPuffSystem.LoadSynchronous();
	}
	// 행이 비어 있으면 퍼프는 없다 — 소리·플래시·히트스톱은 그대로 난다
	if (HitPuffSystem == nullptr)
	{
		return;
	}

	// 메시 중심: `spine` 소켓(뼈)이 있으면 그것, 없으면 경계 중심 — 발밑(액터 자리)이 아니라 몸통에서 터진다
	static const FName SpineSocket(TEXT("spine"));
	const FVector Location = MeshComponent->DoesSocketExist(SpineSocket)
		? MeshComponent->GetSocketLocation(SpineSocket)
		: MeshComponent->Bounds.Origin;
	UNiagaraComponent* Puff = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitPuffSystem, Location, FRotator::ZeroRotator, FVector::OneVector, true, true);
	if (Puff != nullptr)
	{
		// 나이아가라가 받는 것은 파라미터 이름과 파생된 값뿐이다 — 죽는 타격은 세 배 크게
		Puff->SetVariableFloat(TEXT("User.Scale"), bDeath ? 3.f : 1.f);
	}
}

void ACL9Monster::TickFootstep()
{
	// 호스트는 액터 자리, 클라는 보간된 메시 자리 — 20Hz 스냅으로 한 틱에 뭉쳐 오는 거리에 걸음이 몰리지 않게
	const FVector Foot = HasAuthority() ? GetActorLocation() : SmoothedLocation;
	if (!LastFootstepLocation.IsSet())
	{
		LastFootstepLocation = Foot;
		return;
	}
	const float Moved = FVector::Dist2D(LastFootstepLocation.GetValue(), Foot);
	LastFootstepLocation = Foot;

	// 서 있으면 안 난다. 죽은 몸이 날아가는 것도 걸음이 아니다
	if (Moved <= 0.f || State == ECL9MonsterState::Dead)
	{
		FootstepDistanceAccum = 0.f;
		return;
	}

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9RulesDef* Rules = Registry ? Registry->GetRules() : nullptr;
	if (Rules == nullptr)
	{
		return;
	}
	// 종별 보폭 열은 없다 — 추격은 달리기 보폭, 나머지는 걷기 보폭 (docs/plan/data/rules.md FootstepStride*)
	const float Stride = State == ECL9MonsterState::Chase ? Rules->FootstepStrideRun : Rules->FootstepStrideWalk;
	if (Stride <= 0.f)
	{
		FootstepDistanceAccum = 0.f;
		return;
	}
	FootstepDistanceAccum += Moved;
	if (FootstepDistanceAccum >= Stride)
	{
		FootstepDistanceAccum = 0.f;
		const float FootZ = MonsterDef ? MonsterDef->Radius : 0.f;
		CL9Sound::PlayAt(this, CL9SoundNames::Footstep, Foot - FVector(0.f, 0.f, FootZ), CL9Sound::FootstepPitchJitter);
	}
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

	HostTickProxNoise(DeltaSeconds);

	// 분노는 지속되는 동안 게이지를 고정한다 — 죽음의 비명 이후의 사냥 모드는
	// ChaseCooldown을 우회하며 일찍 가라앉는 법이 없다 (docs/systems/monster.md 경보).
	if (RageRemaining > 0.f)
	{
		RageRemaining -= DeltaSeconds;
		DetectGauge = 1.f;
	}

	// 수면, 기상 예고, 스턴 상태에서는 감각이 작동하지 않는다
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
		// 기상 트리거: WakeRadius 안에서 걷는 것(걷기는 조용하다는 예외 규칙),
		// HearRunRadius 안에서 달리는 것, 혹은 대기 중인 경보 지점.
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
		// 몬스터가 실제로 움직이기 전, 모든 플레이어가 날개짓과 울음소리를 본다
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
		// 잠 많은 종은 평온한 시간이 SleepTime만큼 지나면 다시 잠으로 빠져든다
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

		// 게이지가 가득 차면 경계 태세로 들어간다 — 대상이 시야 안에 있으면
		// 곧장 추격, 아니면 마지막 감지 지점으로 향한다 (docs/systems/monster.md).
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

		// 뭐든 다 내려놓으면 DeaggroRadius를 넘어가는 순간 추격에서 풀려난다
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

		// 경보를 울리는 개체는 경보가 발동하기 전 멈춰 서서 운다 (거위 AlertWindup)
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

		// 잡기: CatchRadius 안에 있고 KillCooldown이 다 재충전됐을 때
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
			// EnterState를 거치지 않고 곧장 추격으로 돌아간다 — 예산과 놓침
			// 타이머는 계속 흘러가며, 잡기는 이 값들을 다시 채워 주지 않는다.
			State = ECL9MonsterState::Chase;
			StateStartServerTime = GetServerTime();
			ChaseRepathRemaining = 0.f;
			ApplyState();
		}
		break;
	}

	case ECL9MonsterState::Stun:
	{
		// 떠밀리면 스턴이 지속되는 동안 뒤로 튕겨나간다 — 벽에 닿으면 미끄러짐이 멈춘다
		if (KnockbackRemaining > 0.f)
		{
			const float StepDist = FMath::Min(KnockbackSpeed * DeltaSeconds, KnockbackRemaining);
			FHitResult Hit;
			AddActorWorldOffset(KnockbackDir * StepDist, true, &Hit);
			KnockbackRemaining = Hit.bBlockingHit ? 0.f : KnockbackRemaining - StepDist;
		}
		StunRemaining -= DeltaSeconds;
		if (StunRemaining <= 0.f)
		{
			// 얻어맞으면 그 자체가 도발이다 — 쿨다운과 무관하게 공격자를 추격한다
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
		// Dead는 함수 맨 위에서 걸러지므로, 남는 경우가 없다
		break;
	}
}

void ACL9Monster::SenseTick(float DeltaSeconds)
{
	// 입력 세 가지, 게이지 하나 (docs/systems/monster.md 감각 파이프라인):
	// 히스테리시스가 있는 시야, 행동 테이블의 달리기 소음, 근접도.
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

		const float SightRange = (bHadSight ? MonsterDef->SightOut : MonsterDef->SightIn) * GetDangerScale();
		const bool bInSight = Distance <= SightRange && HasLineOfSight(*Candidate);

		bool bHeard = false;
		if (const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>())
		{
			const UCL9RulesDef* Rules = GameInstance->GetDefRegistry() ? GameInstance->GetDefRegistry()->GetRules() : nullptr;
			// 걷기는 어그로 0 — 소리는 달릴 때만 실린다 (docs/plan/data/rules.md 소리)
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

		const float Rate = (BestDistance <= MonsterDef->DetectNearRadius ? MonsterDef->DetectRateNear : MonsterDef->DetectRateFar) * GetDangerScale();
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
	StateStartServerTime = GetServerTime();

	switch (NewState)
	{
	case ECL9MonsterState::Chase:
		// 일자 곡선이 예산을 늘린다 — 같은 무리라도 밤이 깊을수록 추격이 길어진다 (docs/systems/monster.md 일자 스케일링)
		ChaseBudgetRemaining = MonsterDef->ChaseBudget * GetDangerScale();
		ChaseLoseRemaining = MonsterDef->ChaseLoseTime;
		ChaseRepathRemaining = 0.f;
		// 한 에피소드의 첫 경보가 팩 전체를 깨운다 — 경보를 울리는 개체(거위)는
		// 준비 동작 뒤에 울고, 나머지는 그 즉시 소리 없이 경계 태세로 들어간다
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

	// 몸통 구체는 바닥 위로 Radius만큼 떠 있어서, 원본 위치 그대로는 navmesh를
	// 벗어난다 — 경로를 잡기 전에 양 끝 지점을 먼저 투영한다.
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

	// 회전 모델: 진행 방향이 TurnRate 아래에서 목표를 뒤쫓아가고, 남은 회전각이
	// 클수록 더 느려진다(TurnDrag) — 코너가 시간을 벌어주는 이유다.
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
		// 경로가 놓친 무언가에 끼여 버렸다 — 다음 틱에 다시 경로를 짠다
		PathPoints.Reset();
		return true;
	}
	return false;
}

void ACL9Monster::HostCatch(ACL9Character& Target)
{
	// 한 번 잡으면 KillCooldown이 재충전돼야 한다 — 연속 세 번 타격은
	// 불가능해야 한다 (docs/systems/monster.md 잡기 = 공격).
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

	// 즉사가 아닌 타격은 경직시키고, 이후 공격자가 사냥당한다
	if (ACL9Character* Attacker = Cast<ACL9Character>(Source))
	{
		SensedCharacter = Attacker;
		LastSensedLocation = Attacker->GetActorLocation();
		bHadSight = true;
		DetectGauge = 1.f;
	}
	KnockbackRemaining = 0.f;
	StunRemaining = MonsterDef->HitStunMs / 1000.f;
	bPushStun = false;
	EnterState(ECL9MonsterState::Stun);
}

void ACL9Monster::OnShove(AActor* Shover)
{
	if (!HasAuthority() || MonsterDef == nullptr || State == ECL9MonsterState::Dead || Shover == nullptr)
	{
		return;
	}

	// 떠민 쪽 반대 방향으로 넉백하며, 그 이동은 스턴 시간에 걸쳐 나뉜다 (common.md 반격).
	FVector Away = GetActorLocation() - Shover->GetActorLocation();
	Away.Z = 0.f;
	KnockbackDir = Away.GetSafeNormal();
	if (KnockbackDir.IsNearlyZero())
	{
		KnockbackDir = -GetActorForwardVector();
	}
	KnockbackRemaining = MonsterDef->PushKnockback;
	KnockbackSpeed = MonsterDef->PushKnockback / FMath::Max(MonsterDef->PushStun, UE_KINDA_SMALL_NUMBER);

	// 떠밀림도 도발이다 — 스턴이 끝나면 범인을 쫓아간다
	if (ACL9Character* Attacker = Cast<ACL9Character>(Shover))
	{
		SensedCharacter = Attacker;
		LastSensedLocation = Attacker->GetActorLocation();
		bHadSight = true;
		DetectGauge = 1.f;

		// 밀치는 소리는 밀린 몬스터가 아니라 **민 사람의 발밑**에서 난다 (docs/systems/wave.md 소음 이벤트)
		const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
		const UCL9DefRegistry* Registry = GameInstance != nullptr ? GameInstance->GetDefRegistry() : nullptr;
		const UCL9RulesDef* Rules = Registry != nullptr ? Registry->GetRules() : nullptr;
		if (ACL9GameState* GameState = GetWorld()->GetGameState<ACL9GameState>())
		{
			GameState->HostReportNoise(ECL9NoiseSource::Push, Attacker->GetFootLocation(),
				Rules != nullptr ? Rules->PushNoiseRadius : 0.f, Attacker->GetPlayerState());
		}
	}
	StunRemaining = MonsterDef->PushStun;
	bPushStun = true;
	EnterState(ECL9MonsterState::Stun);
}

void ACL9Monster::OnStrike(int32 Damage, ECL9HitStun Stun, AActor* Source)
{
	if (!HasAuthority() || MonsterDef == nullptr || State == ECL9MonsterState::Dead)
	{
		return;
	}

	// 피해가 먼저 — 죽으면 경직은 없다. 피해 있는 타격은 OnDamage가 HitStun 경직까지 주므로
	// Hit 종류는 거기서 끝나고, Push는 그 위에 밀쳐냄으로 덮는다(경직 시간도 PushStun으로).
	if (Damage > 0)
	{
		OnDamage(Damage, Source);
		if (State == ECL9MonsterState::Dead || Stun != ECL9HitStun::Push)
		{
			return;
		}
	}
	switch (Stun)
	{
	case ECL9HitStun::Hit:
		OnDamage(0, Source);
		break;
	case ECL9HitStun::Push:
		OnShove(Source);
		break;
	default:
		break;
	}
}

void ACL9Monster::Die(AActor* Source)
{
	EnterState(ECL9MonsterState::Dead);
	DetectGauge = 0.f;
	KnockbackRemaining = 0.f;

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9RulesDef* Rules = Registry ? Registry->GetRules() : nullptr;

	// 죽음 발사: 타격 원점 반대편으로 날아간다 — 비행은 HostTickDeathLaunch 가 매 틱 적분하고 트랜스폼 복제로 퍼진다
	// (docs/systems/monster.md 타격의 표현). 원점이 없거나 겹쳐 있으면 몬스터의 뒤쪽으로
	DeathLaunchVelocity = FVector::ZeroVector;
	if (Rules != nullptr && (Rules->DeathLaunchSpeed > 0.f || Rules->DeathLaunchUp > 0.f))
	{
		FVector Away = Source != nullptr ? GetActorLocation() - Source->GetActorLocation() : -GetActorForwardVector();
		Away.Z = 0.f;
		FVector Dir = Away.GetSafeNormal();
		if (Dir.IsNearlyZero())
		{
			Dir = -GetActorForwardVector();
		}
		DeathLaunchVelocity = Dir * Rules->DeathLaunchSpeed + FVector(0.f, 0.f, Rules->DeathLaunchUp);
	}

	// 사냥 보상은 아이템으로 떨어질 뿐, 직접 에테르로 주어지는 법은 없다 (docs/plan/data/monster/common.md).
	// 몬스터 중심 높이·옆 30~90cm 자리에서 앙각 45~70°·요 무작위로 던져 흩어진다 — 옆 오프셋을 남기는 이유: 비거리
	// 30~46cm 호만으로는 둘이 30cm 이상 떨어질 확률이 3/4 뿐이라 C5 검수가 흔들린다(balance 2026-09-18). 이후는 아이템 물리 그대로
	if (ACL9GameMode* GameMode = GetWorld()->GetAuthGameMode<ACL9GameMode>())
	{
		const FVector Center = GetActorLocation();
		const float LootSpeed = Rules != nullptr ? Rules->DeathLootSpeed : 0.f;
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
				const FVector Origin = Center + Sideways;
				ACL9Item* Item = GameMode->SpawnItem(Drop.ItemAlias, Drop.ItemSubAlias, FTransform(Origin));
				if (Item != nullptr && LootSpeed > 0.f)
				{
					const FRotator Lob(FMath::FRandRange(45.f, 70.f), FMath::FRandRange(0.f, 360.f), 0.f);
					Item->OnThrown(Origin, Lob.Vector() * LootSpeed);
				}
			}
		}
	}

	// 죽음의 비명: 직접 들은 개체는 자신의 RageTime만큼 분노하고, 그 분노는
	// RevengeRadius/RevengeTime을 통해 한 번 더 퍼져 나간다. 구역 판정은 맵에
	// 구역이 생기면(⑤) 함께 들어온다 — 지금의 평지 맵은 구역이 하나뿐이다.
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

void ACL9Monster::HostTickProxNoise(float DeltaSeconds)
{
	// 잠든 계열(Asleep·WakeTelegraph)은 소나를 내지 않는다 — 잠든 닭까지 자기 자리를 알려 주면 기상 예고로
	// 배우던 것이 값싸진다 (2026-09-16 사용자). 깨어 있는 상태(순찰·조사·추격…)만 낸다. Dead는 HostTickAI가 먼저 거른다.
	if (State == ECL9MonsterState::Asleep || State == ECL9MonsterState::WakeTelegraph)
	{
		return;
	}

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9WaveTable* Table = GameInstance != nullptr ? GameInstance->GetWaveTable() : nullptr;
	const FCL9WaveInfo* Row = Table != nullptr ? Table->FindRow(ECL9NoiseSource::MonsterProx) : nullptr;
	// 반복 간격이 없으면 근접 소나는 울리지 않는다 — 이 행은 반복으로만 나는 소음원이다
	if (Row == nullptr || Row->RepeatInterval <= 0.f)
	{
		return;
	}

	ProxNoiseRemaining -= DeltaSeconds;
	if (ProxNoiseRemaining > 0.f)
	{
		return;
	}
	ProxNoiseRemaining = Row->RepeatInterval;

	// TriggerRadius 안에 산 플레이어가 있을 때만 — 0이면 조건 없음
	if (Row->TriggerRadius > 0.f)
	{
		bool bPlayerNear = false;
		for (TActorIterator<ACL9Character> It(GetWorld()); It; ++It)
		{
			const ACL9Character* Candidate = *It;
			if (!Candidate->IsDead() && FVector::Dist2D(GetActorLocation(), Candidate->GetActorLocation()) <= Row->TriggerRadius)
			{
				bPlayerNear = true;
				break;
			}
		}
		if (!bPlayerNear)
		{
			return;
		}
	}

	if (ACL9GameState* GameState = GetWorld()->GetGameState<ACL9GameState>())
	{
		// 판정 반경이 없는 소음원이다 — 링 크기는 표의 FallbackRadius가 정하고, 몬스터 청각은 이 사건을 듣지 않는다
		GameState->HostReportNoise(ECL9NoiseSource::MonsterProx, GetActorLocation(), 0.f, this);
	}
}

void ACL9Monster::FirePackAlarm()
{
	// 울음 파동은 이 경보의 시각 표현이다 — 링 반경이 곧 PackAlarmRadius라
	// 링이 닿은 동료가 실제로 깨어난다 (docs/systems/wave.md `monster_call`)
	if (ACL9GameState* GameState = GetWorld()->GetGameState<ACL9GameState>())
	{
		GameState->HostReportNoise(ECL9NoiseSource::MonsterCall, GetActorLocation(), MonsterDef->PackAlarmRadius, this);
	}

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
		// 경보는 새 경계 — ChaseCooldown이 여전히 이를 막는 문턱으로 작동한다 (docs/systems/monster.md)
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
		// 분노는 설계상 ChaseCooldown을 무시한다 (경보 문서: 쿨다운은 만능 방패가 아니다)
		EnterState(bHadSight && SensedCharacter.IsValid() ? ECL9MonsterState::Chase : ECL9MonsterState::Investigate);
		break;
	default:
		break;
	}
}

void ACL9Monster::TryWake()
{
	// 누군가 지켜보고 있는 잠든 개체는 계속 잠들어 있는다 — 플레이어의 시선
	// 안에서는 깨어나지 않는다 (WakeSightRadius, WakeRetryTime마다 재시도).
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

float ACL9Monster::GetStateElapsed() const
{
	// 클라의 서버 시각 추정이 조금 앞설 수 있어 0 아래로 내려가지 않게
	return FMath::Max(0.f, GetServerTime() - StateStartServerTime);
}

float ACL9Monster::GetServerTime() const
{
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	return GameState ? GameState->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
}

void ACL9Monster::ApplyState()
{
	// 이제 AnimBP가 복제된 상태에 따라 이동/피격/공격/사망 애니메이션을 전환한다
	// (docs/systems/animation.md); 여기서의 표현 처리는 행 값에 따른 메시 방향
	// 보정만 담당한다.
	if (MeshComponent != nullptr)
	{
		const float YawOffset = MonsterDef ? MonsterDef->MeshYawOffset : 0.f;
		MeshComponent->SetRelativeRotation(FRotator(0.f, YawOffset, 0.f));
	}
}

void ACL9Monster::TickHitFlash()
{
	if (MeshComponent == nullptr)
	{
		return;
	}

	// 맞은 직후만 빨갛다: 경직(Stun)과 죽음(Dead)에 들어간 뒤 HitFlashTime 동안. 상태로 거르므로 레벨에 처음부터
	// 놓여 있어 StateStartServerTime이 0인(= 경과가 아주 큰) 개체는 애초에 걸리지 않고, 배회 중에도 번쩍이지 않는다.
	bool bWantFlash = false;
	if (State == ECL9MonsterState::Stun || State == ECL9MonsterState::Dead)
	{
		const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
		const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
		const UCL9RulesDef* Rules = Registry ? Registry->GetRules() : nullptr;
		if (Rules != nullptr && GetStateElapsed() < Rules->HitFlashTime)
		{
			if (!bHitFlashLoaded)
			{
				bHitFlashLoaded = true;
				HitFlashMaterial = Rules->HitFlashMaterial.LoadSynchronous();
			}
			bWantFlash = HitFlashMaterial != nullptr;
		}
	}

	// 오버레이는 바뀔 때만 건드린다 — 매 틱 다시 세팅하면 렌더 상태가 매번 갱신된다
	if (bWantFlash == bHitFlashOn)
	{
		return;
	}
	bHitFlashOn = bWantFlash;
	MeshComponent->SetOverlayMaterial(bWantFlash ? HitFlashMaterial.Get() : nullptr);
}

float ACL9Monster::GetDangerScale() const
{
	// 게임 상태에서 파생되는 런타임 값 (부지 BaseDanger × DangerCurve, 로비에서는 1)
	const ACL9GameState* CL9GameState = GetWorld() ? GetWorld()->GetGameState<ACL9GameState>() : nullptr;
	return CL9GameState != nullptr ? CL9GameState->GetDangerScale() : 1.f;
}
