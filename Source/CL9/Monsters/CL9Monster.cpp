// Copyright Epic Games, Inc. All Rights Reserved.

#include "Monsters/CL9Monster.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
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
#include "Items/CL9Cart.h"
#include "Items/CL9Item.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameMode.h"
#include "GameMode/CL9GameState.h"
#include "GameMode/CL9PlayerController.h"

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

bool ACL9Monster::bCheatKnockbackLog = false;

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

	// 처형 카메라 — 잠들어 있다가 Execute 에서만 켜진다(ApplyState). 소켓·오프셋은 행 값이라 여기서는 자리만 만든다
	ExecuteCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ExecuteCamera"));
	ExecuteCamera->SetupAttachment(MeshComponent);
	ExecuteCamera->bAutoActivate = false;
	ExecuteCamera->SetActive(false);
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
	DOREPLIFETIME(ACL9Monster, PounceResultServerTime);
	DOREPLIFETIME(ACL9Monster, bPounceHit);
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

	// 타격 플래시·소리·퍼프·히트스톱·전이 소리는 호스트와 클라 모두에서 같은 복제 시계로 굴러간다 — 아래 호스트 분기가 먼저
	// 돌아가 버리기 전에 부른다
	TickHitFlash();
	TickHitFeel();
	TickStateSounds();

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
	if (State == ECL9MonsterState::Execute)
	{
		// 처형은 텔레포트로 시작한다 — 보간하면 메시가 앵커까지 미끄러져 가는 동안 카메라가 흔들리고 피해자 옆이 빈다. 처형 동안은
		// 스무딩을 멈추고 액터 값에 붙인다 (D1)
		SmoothedLocation = GetActorLocation();
		SmoothedRotation = GetActorRotation();
	}
	else
	{
		SmoothedLocation = FMath::VInterpTo(SmoothedLocation, GetActorLocation(), DeltaSeconds, SmoothingInterpSpeed);
		SmoothedRotation = FMath::RInterpTo(SmoothedRotation, GetActorRotation(), DeltaSeconds, SmoothingInterpSpeed);
	}
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
	FHitResult Hit;

	// 벽: 지금 자리 → 다음 자리의 수평 선분이 막히면 그 자리에서 수평 속도 0 (남은 낙하만 이어진다)
	if (!FVector2D(DeathLaunchVelocity).IsNearlyZero())
	{
		const FVector HorizontalEnd(Next.X, Next.Y, Start.Z);
		if (TraceWorld(GetWorld(), this, Start, HorizontalEnd, Hit))
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
		if (TraceWorld(GetWorld(), this, FloorStart, FloorEnd, Hit))
		{
			Next.Z = Hit.ImpactPoint.Z + Radius;
			DeathLaunchVelocity = FVector::ZeroVector;
		}
	}

	SetActorLocation(Next, false, nullptr, ETeleportType::TeleportPhysics);
}

bool ACL9Monster::TraceWorld(const UWorld* World, const AActor* Self, const FVector& From, const FVector& To, FHitResult& OutHit, const AActor* AlsoIgnore)
{
	if (World == nullptr)
	{
		return false;
	}
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CL9TraceWorld), false, Self);
	if (AlsoIgnore != nullptr)
	{
		Params.AddIgnoredActor(AlsoIgnore);
	}
	// 앞을 가린 폰·아이템·몬스터를 하나씩 무시 목록에 넣고 다시 긋는다 — 시체가 사람 몸이나 계란 위에서 멈추지 않고, 동료 폰이
	// 휘두르기를 가리지 않는다. 사람은 맞지도 가리지도 않는다
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

void ACL9Monster::TickStateSounds()
{
	// 전이 = 상태가 바뀌었거나 같은 상태의 시작 시각이 바뀜(연속 예고처럼 같은 상태로 다시 들어옴). 창(elapsed)이 아니라 전이라
	// 20Hz 복제가 늦게 와도 한 번은 반드시 잡힌다
	const bool bPrimed = bStateSoundPrimed;
	const bool bTransition = State != LastSoundState || StateStartServerTime != LastSoundStateStartTime;
	bStateSoundPrimed = true;
	LastSoundState = State;
	LastSoundStateStartTime = StateStartServerTime;

	// 접속·스폰 직후의 첫 틱은 기준값만 — 이미 지나간 예고·잡기를 되풀이하지 않는다
	if (!bPrimed || !bTransition || MeshComponent == nullptr)
	{
		return;
	}

	const FVector Center = MeshComponent->Bounds.Origin;
	switch (State)
	{
	case ECL9MonsterState::Pounce:
		CL9Sound::PlayAt(this, CL9SoundNames::MonsterPounce, Center);
		break;
	case ECL9MonsterState::Catch:
		// 성공 소리는 Catch 전이에서만 — 헛침엔 MonsterPounce 뒤에 아무 소리도 없다
		CL9Sound::PlayAt(this, CL9SoundNames::MonsterCatch, Center);
		break;
	case ECL9MonsterState::Execute:
		// 처형은 Catch 를 지나지 않는다 — 잡히는 순간의 소리는 있어야 하므로 같은 행을 진입 전이에서 낸다 (2026-09-20 리드 확인 ②)
		CL9Sound::PlayAt(this, CL9SoundNames::MonsterCatch, Center);
		break;
	default:
		break;
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
	// 헛침 쿨다운은 여기서만, 틱당 한 번 줄어든다 — 상태 분기 안에서 다시 빼지 않는다
	PounceMissRemaining = FMath::Max(PounceMissRemaining - DeltaSeconds, 0.f);

	HostTickProxNoise(DeltaSeconds);

	// 분노는 지속되는 동안 게이지를 고정한다 — 죽음의 비명 이후의 사냥 모드는
	// ChaseCooldown을 우회하며 일찍 가라앉는 법이 없다 (docs/systems/monster.md 경보).
	if (RageRemaining > 0.f)
	{
		RageRemaining -= DeltaSeconds;
		DetectGauge = 1.f;
	}

	// 수면, 기상 예고, 스턴 상태에서는 감각이 작동하지 않는다. 잡기 예고 동안도 멈춘다 — 타깃은 진입 때 잠근 PounceTarget 이고
	// 감각의 재타깃팅은 예고 동안 무시한다(덮침은 그 타깃의 지금 위치로). 처형 중도 같다 — 타깃은 ExecuteTarget 이다
	const bool bSensesAwake = State != ECL9MonsterState::Asleep
		&& State != ECL9MonsterState::WakeTelegraph
		&& State != ECL9MonsterState::Stun
		&& State != ECL9MonsterState::Pounce
		&& State != ECL9MonsterState::Execute;
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
		// 곧장 사냥(추격, 처형 종은 스토킹), 아니면 마지막 감지 지점으로 향한다 (docs/systems/monster.md).
		if (DetectGauge >= 1.f && bAlertGateOpen)
		{
			EnterState(bHadSight && SensedCharacter.IsValid() ? GetHuntState() : ECL9MonsterState::Investigate);
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
			EnterState(GetHuntState());
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
		if (TickChaseGiveUp(Target, DeltaSeconds))
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

		// 잡기: 잡는 종(CatchRadius > 0)이고 KillCooldown이 다 재충전됐을 때. 넘어져 있는 타깃은 잡지도 예고하지도 않는다 —
		// 넘어진 동안은 무적(2026-09-20 사용자)이라 진입 검사에서 건너뛰고, 추격 자체(예산·놓침·경로)는 그대로 이어져 일어나는
		// 순간 다시 예고가 시작된다(타깃 선택에서 빼면 동료에게 튀거나 시야를 잃어 추격이 끊긴다). 처형 중인 타깃도 같다 (D3)
		const float TargetDistance = FVector::Dist2D(GetActorLocation(), Target->GetActorLocation());
		const bool bCanCatch = MonsterDef->CatchRadius > 0.f && CatchCooldownRemaining <= 0.f && !Target->IsKnockedDown() && !Target->IsExecuted();
		if (bCanCatch)
		{
			// 예고가 켜진 종(PounceWindup > 0)은 PounceRadius 에서 멈춰 예고한다(헛친 뒤 PounceMissCooldown 동안은 다시 예고하지 않음);
			// 꺼진 종은 옛 규칙 그대로 CatchRadius 에서 즉시 잡는다
			if (MonsterDef->PounceWindup > 0.f)
			{
				if (TargetDistance <= MonsterDef->PounceRadius && PounceMissRemaining <= 0.f)
				{
					PounceTarget = Target;
					EnterState(ECL9MonsterState::Pounce);
					break;
				}
			}
			else if (TargetDistance <= MonsterDef->CatchRadius)
			{
				HostCatch(*Target);
				break;
			}
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

	case ECL9MonsterState::Pounce:
	{
		// Chase 의 부분 상태 — 탈출 조건(타깃 소실·사망·예산·놓침·HomeRange·Deaggro)을 덮치기 전에 먼저 평가한다.
		// 타깃은 진입 때 잠근 PounceTarget 이다(접속 끊김이면 null → 놓침 규칙으로 Break)
		ACL9Character* Target = PounceTarget.Get();
		if (TickChaseGiveUp(Target, DeltaSeconds))
		{
			EnterState(ECL9MonsterState::Break);
			break;
		}

		// 제자리에서 타깃을 향해 돈다 — MoveAlongPath 의 회전 저항을 우회하므로 TurnRate 로만 제한한다(홱 돌지 않게)
		FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
		ToTarget.Z = 0.f;
		if (!ToTarget.IsNearlyZero())
		{
			HeadingYaw = FMath::FixedTurn(HeadingYaw, ToTarget.Rotation().Yaw, MonsterDef->TurnRate * DeltaSeconds);
			SetActorRotation(FRotator(0.f, HeadingYaw, 0.f));
		}

		PounceWindupRemaining -= DeltaSeconds;
		if (PounceWindupRemaining <= 0.f)
		{
			HostPounceLunge(*Target);
		}
		break;
	}

	case ECL9MonsterState::Catch:
	{
		CatchPauseRemaining -= DeltaSeconds;
		if (CatchPauseRemaining <= 0.f)
		{
			ResumeChase();
		}
		break;
	}

	case ECL9MonsterState::Stun:
	{
		// 넉백: 타격은 진입 뒤 KnockbackDelay(= HitStopTime, 멈춘 프레임은 밀리지 않는다)를 기다렸다가 HitKnockbackTime 동안 등속,
		// 밀침(OnShove)은 바로 스턴 시간에 걸쳐. 스텝마다 이동 전에 세상 트레이스(TraceWorld — 몸체 앞 가장자리까지)로 벽을 보고
		// 막히면 그 자리에서 끝난다. AddActorWorldOffset 스윕은 WorldStatic 만 막는데 맵 조각 프로필이 제각각이라(2026-09-18) 믿지
		// 않는다 (docs/systems/monster.md 넉백)
		if (KnockbackRemaining > 0.f && GetStateElapsed() >= KnockbackDelay)
		{
			const float StepDist = FMath::Min(KnockbackSpeed * DeltaSeconds, KnockbackRemaining);
			const float Radius = MonsterDef != nullptr ? MonsterDef->Radius : 0.f;
			const FVector From = GetActorLocation();
			FHitResult Hit;
			const bool bBlocked = TraceWorld(GetWorld(), this, From, From + KnockbackDir * (StepDist + Radius), Hit);
			if (!bBlocked)
			{
				AddActorWorldOffset(KnockbackDir * StepDist, true, &Hit);
				++KnockbackSteps;
			}
			KnockbackRemaining = (bBlocked || Hit.bBlockingHit) ? 0.f : KnockbackRemaining - StepDist;
			if (KnockbackRemaining <= 0.f && bCheatKnockbackLog)
			{
				const FVector To = GetActorLocation();
				UE_LOG(LogCL9, Log, TEXT("cl9.knockback: %s#%u from=(%.1f %.1f %.1f) to=(%.1f %.1f %.1f) dist=%.1f steps=%d blocked=%d hitstop=%.3f time=%.3f"),
					*(MonsterDef != nullptr ? MonsterDef->Alias.ToString() : GetName()), GetUniqueID(),
					KnockbackFrom.X, KnockbackFrom.Y, KnockbackFrom.Z, To.X, To.Y, To.Z, FVector::Dist2D(KnockbackFrom, To),
					KnockbackSteps, (bBlocked || Hit.bBlockingHit) ? 1 : 0, KnockbackDelay,
					KnockbackTotal / FMath::Max(KnockbackSpeed, UE_KINDA_SMALL_NUMBER));
			}
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

	case ECL9MonsterState::Stalk:
	{
		// 타깃 잠금·교체: 감각이 다른 산 사람(처형 중 아님)을 잡으면 그쪽으로 옮기고 두 계수기를 처음부터 센다 (D4)
		ACL9Character* Sensed = SensedCharacter.Get();
		if (Sensed != nullptr && Sensed != StalkTarget.Get() && !Sensed->IsDead() && !Sensed->IsExecuted())
		{
			StalkTarget = Sensed;
			StalkUnseenTime = 0.f;
			StalkStaredTime = 0.f;
			PathPoints.Reset();
			ChaseRepathRemaining = 0.f;
			UE_LOG(LogCL9, Log, TEXT("'%s' stalks '%s'."), *GetNameSafe(this), *GetNameSafe(Sensed));
		}

		// 다른 개체에게 처형 중인 타깃은 놓친 것과 같다 — TickChaseGiveUp 은 IsDead 만 보므로 여기서 null 로 만든다
		ACL9Character* Target = StalkTarget.Get();
		if (Target != nullptr && Target->IsExecuted())
		{
			Target = nullptr;
		}
		// 놓침 규칙은 추격과 같다(예산·놓침 시계·HomeRange·Deaggro·타깃 소실)
		if (TickChaseGiveUp(Target, DeltaSeconds))
		{
			EnterState(ECL9MonsterState::Break);
			break;
		}

		const float TargetDistance = FVector::Dist2D(GetActorLocation(), Target->GetActorLocation());
		if (IsWatchedByAnyone())
		{
			// 보인다 — 미발견 줄은 끊기고 응시가 쌓인다. 다 쌓이면 격분: 이후는 닭과 같은 Chase·Pounce·Catch
			StalkUnseenTime = 0.f;
			StalkStaredTime += DeltaSeconds;
			if (StalkStaredTime >= MonsterDef->StalkStareAnger)
			{
				UE_LOG(LogCL9, Log, TEXT("'%s' stared at for %.2fs — angered, chases '%s'."), *GetNameSafe(this), StalkStaredTime, *GetNameSafe(Target));
				EnterState(ECL9MonsterState::Chase);
				break;
			}

			// 보이는 동안은 서 있는다. 타깃이 StalkRetreatRadius 안이면 타깃 반대쪽의 그 반경 지점으로 RetreatSpeed 로 물러난다 —
			// 경로가 없으면 그냥 선다. 후퇴로 바뀌는 순간 접근 경로는 버린다
			const bool bRetreat = MonsterDef->StalkRetreatRadius > 0.f && TargetDistance < MonsterDef->StalkRetreatRadius;
			if (!bRetreat)
			{
				PathPoints.Reset();
				bStalkRetreating = false;
				break;
			}
			if (!bStalkRetreating)
			{
				bStalkRetreating = true;
				PathPoints.Reset();
				ChaseRepathRemaining = 0.f;
				UE_LOG(LogCL9, Verbose, TEXT("'%s' watched at %.0fcm — retreats."), *GetNameSafe(this), TargetDistance);
			}
			ChaseRepathRemaining -= DeltaSeconds;
			if (ChaseRepathRemaining <= 0.f || PathPoints.Num() == 0)
			{
				ChaseRepathRemaining = ChaseRepathInterval;
				FVector Away = GetActorLocation() - Target->GetActorLocation();
				Away.Z = 0.f;
				Away = Away.GetSafeNormal();
				if (Away.IsNearlyZero())
				{
					Away = -GetActorForwardVector().GetSafeNormal2D();
				}
				if (!StartPathTo(Target->GetActorLocation() + Away * MonsterDef->StalkRetreatRadius))
				{
					PathPoints.Reset();
					break;
				}
			}
			MoveAlongPath(DeltaSeconds, MonsterDef->RetreatSpeed);
			break;
		}

		// 안 보인다 — 응시는 같은 속도로 0 까지 감쇠, 미발견이 쌓인다. 후퇴 중이었으면 접근 경로를 다시 잡는다
		StalkStaredTime = FMath::Max(StalkStaredTime - DeltaSeconds, 0.f);
		StalkUnseenTime += DeltaSeconds;
		if (bStalkRetreating)
		{
			bStalkRetreating = false;
			PathPoints.Reset();
			ChaseRepathRemaining = 0.f;
		}

		// 넘어진 타깃은 기다린다 — 계수기는 그대로 흐르므로 일어나는 순간 조건이 맞으면 바로 처형이다
		if (Target->IsKnockedDown())
		{
			PathPoints.Reset();
			break;
		}

		// 처형 진입 검사 — 전부 참이고 앵커가 비어 있으면 텔레포트하고 Execute 로
		if (TryBeginExecute(*Target, /*bCheat*/ false))
		{
			break;
		}

		// 접근: 타깃 등 뒤 지점으로 — 추격과 같은 재경로 주기. 등 뒤 지점이 경로 실패면 타깃 자체로
		ChaseRepathRemaining -= DeltaSeconds;
		if (ChaseRepathRemaining <= 0.f || PathPoints.Num() == 0)
		{
			ChaseRepathRemaining = ChaseRepathInterval;
			if (!StartPathTo(GetBehindPoint(*Target)))
			{
				StartPathTo(Target->GetActorLocation());
			}
		}
		MoveAlongPath(DeltaSeconds, MonsterDef->StalkSpeed);
		break;
	}

	case ECL9MonsterState::Execute:
	{
		// 이동 0. 타깃 소실(접속 끊김)이면 그만둔다
		ACL9Character* Target = ExecuteTarget.Get();
		if (Target == nullptr)
		{
			UE_LOG(LogCL9, Log, TEXT("'%s' lost its execution target — break."), *GetNameSafe(this));
			EnterState(ECL9MonsterState::Break);
			break;
		}

		const float Elapsed = GetStateElapsed();
		if (!bExecuteKilled && Elapsed >= MonsterDef->ExecuteDuration)
		{
			// 결과는 진입에서 정해졌다 — HP 와 무관하게, 넘어짐 무적을 거치지 않고 죽인다 (D3)
			bExecuteKilled = true;
			Target->HostKill(this);
			CatchCooldownRemaining = MonsterDef->KillCooldown;
			UE_LOG(LogCL9, Log, TEXT("'%s' executed '%s' at %.2fs."), *GetNameSafe(this), *GetNameSafe(Target), Elapsed);
		}

		// 죽인 뒤에도 시체 위에 머문다(D6 — 피해자 시점은 죽는 순간 데스캠으로 넘어가고, 시체 옆에 선 이 몬스터가 거기 잡힌다) —
		// 피해자의 관전이 시작되거나 SpectateDelay 가 다 지나면 Break
		if (bExecuteKilled)
		{
			const ACL9PlayerController* VictimPC = Cast<ACL9PlayerController>(Target->GetController());
			const bool bVictimSpectating = VictimPC != nullptr && VictimPC->IsSpectating();
			const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
			const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
			const UCL9RulesDef* Rules = Registry ? Registry->GetRules() : nullptr;
			const float SpectateDelay = Rules != nullptr ? Rules->SpectateDelay : 0.f;
			if (bVictimSpectating || Elapsed >= MonsterDef->ExecuteDuration + SpectateDelay)
			{
				UE_LOG(LogCL9, Log, TEXT("'%s' leaves the execution at %.2fs (%s)."), *GetNameSafe(this), Elapsed,
					bVictimSpectating ? TEXT("victim spectating") : TEXT("spectate delay over"));
				EnterState(ECL9MonsterState::Break);
			}
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
		// 죽은 사람과 처형 중인 사람은 타깃이 아니다 — 둘째 몬스터가 처형 중인 피해자를 넘어뜨려 넘어짐 무적으로 "구해" 주면 안 된다 (D3)
		if (Candidate->IsDead() || Candidate->IsExecuted())
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
	case ECL9MonsterState::Pounce:
		PounceWindupRemaining = MonsterDef->PounceWindup;
		// 예고 파동 — 링이 "뒤에서 온다"를 바닥에 그린다. 소리는 파동 행이 아니라 상태 전이(TickStateSounds)가 낸다
		if (ACL9GameState* GameState = GetWorld()->GetGameState<ACL9GameState>())
		{
			GameState->HostReportNoise(ECL9NoiseSource::MonsterPounce, GetActorLocation(), 0.f, this);
		}
		// 20Hz 는 복제 주기지 AI 틱이 아니다 — 진입 시각을 찍자마자 보내야 클라가 보는 예고 길이가 깎이지 않는다
		ForceNetUpdate();
		break;
	case ECL9MonsterState::Stalk:
		// 추격과 같은 놓침 규칙을 쓰므로 같은 시계를 채운다. 팩 경보는 울리지 않는다 — 스토킹은 조용하고, 격분해 Chase 로 갈 때
		// 그 진입이 (bPackAlarmFired 가 아직 false 라) 경보를 낸다
		ChaseBudgetRemaining = MonsterDef->ChaseBudget * GetDangerScale();
		ChaseLoseRemaining = MonsterDef->ChaseLoseTime;
		ChaseRepathRemaining = 0.f;
		StalkTarget = SensedCharacter;
		StalkUnseenTime = 0.f;
		StalkStaredTime = 0.f;
		bStalkRetreating = false;
		LastExecuteBlock = NAME_None;
		UE_LOG(LogCL9, Log, TEXT("'%s' stalks '%s'."), *GetNameSafe(this), *GetNameSafe(StalkTarget.Get()));
		break;
	case ECL9MonsterState::Execute:
		// 타깃·텔레포트는 HostBeginExecute 가 이미 끝냈다 — 여기서는 킬 플래그와 즉시 복제만
		bExecuteKilled = false;
		ForceNetUpdate();
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
	// 잡히면 처형인 종(bExecuteOnCatch — 돼지)은 잡는 순간이 처형이다: 피해·Catch 휴지기 없이 잡은 자리에서 Execute 로.
	// KillCooldown 은 Execute 틱의 HostKill 자리에서 재충전된다. 넘어짐·처형 중 타깃은 호출자(bCanCatch·덮침 재검사)가 이미 걸렀다
	if (MonsterDef->bExecuteOnCatch)
	{
		HostBeginExecute(Target, GetActorLocation(), /*bTeleport*/ false);
		return;
	}

	// 한 번 잡으면 KillCooldown이 재충전돼야 한다 — 연속 세 번 타격은
	// 불가능해야 한다 (docs/systems/monster.md 잡기 = 공격).
	Target.OnDamage(MonsterDef->CatchDamage, this, 0.f);
	CatchCooldownRemaining = MonsterDef->KillCooldown;
	EnterState(ECL9MonsterState::Catch);
}

bool ACL9Monster::TickChaseGiveUp(ACL9Character* Target, float DeltaSeconds)
{
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
	return Target == nullptr || Target->IsDead() || ChaseBudgetRemaining <= 0.f || ChaseLoseRemaining <= 0.f || bOutOfHome || bDeaggro;
}

void ACL9Monster::ResumeChase()
{
	// EnterState(Chase) 는 예산·놓침 시계를 다시 채우고 경보를 다시 울린다 — 잡기 휴지기·헛침에서 돌아올 때는 그러면 안 된다
	State = ECL9MonsterState::Chase;
	StateStartServerTime = GetServerTime();
	PathPoints.Reset();
	ChaseRepathRemaining = 0.f;
	ApplyState();
}

void ACL9Monster::HostPounceLunge(ACL9Character& Target)
{
	const float Radius = MonsterDef->Radius;
	const FVector Start = GetActorLocation();
	const FVector TargetLocation = Target.GetActorLocation();

	FVector Direction = TargetLocation - Start;
	Direction.Z = 0.f;
	Direction = Direction.GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		Direction = GetActorForwardVector().GetSafeNormal2D();
	}

	// 덮침 이동: 몸체 구를 WorldStatic + WorldDynamic 로 스윕한다(바닥·문 조각의 프로필이 제각각이라 둘 다) — 벽이면 거기까지.
	// 자신·타깃·캐릭터·아이템·몬스터는 무시 목록에 넣는다(닭 무리가 벽을 가리거나 사람 몸에서 멈추지 않게). 죽음 발사의
	// "맞은 것을 하나씩 빼며 4회 재시도" 방식은 쓰지 않는다
	FVector Reached = Start + Direction * MonsterDef->PounceLunge;
	if (MonsterDef->PounceLunge > 0.f)
	{
		FCollisionQueryParams Params(SCENE_QUERY_STAT(CL9Pounce), false, this);
		Params.AddIgnoredActor(&Target);
		for (TActorIterator<ACL9Character> It(GetWorld()); It; ++It)
		{
			Params.AddIgnoredActor(*It);
		}
		for (TActorIterator<ACL9Item> It(GetWorld()); It; ++It)
		{
			Params.AddIgnoredActor(*It);
		}
		for (TActorIterator<ACL9Monster> It(GetWorld()); It; ++It)
		{
			Params.AddIgnoredActor(*It);
		}
		const FCollisionObjectQueryParams ObjectParams(ECC_TO_BITFIELD(ECC_WorldStatic) | ECC_TO_BITFIELD(ECC_WorldDynamic));

		FHitResult Hit;
		if (GetWorld()->SweepSingleByObjectType(Hit, Start, Reached, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(Radius), Params))
		{
			// 스윕이 시작부터 끼어 있으면(bStartPenetrating) Location 이 Start 라 제자리 — 그것도 "벽이면 거기까지"다
			Reached = Hit.Location;
		}
	}
	Reached.Z = Start.Z;
	SetActorLocation(Reached, false, nullptr, ETeleportType::TeleportPhysics);
	HeadingYaw = Direction.Rotation().Yaw;
	SetActorRotation(FRotator(0.f, HeadingYaw, 0.f));
	PathPoints.Reset();

	// 재검사 — 도달 위치 기준. 원격 잡기 금지: 거리·높이 차(위층 타깃 배제)·시야 셋 다 맞아야 잡는다. 넘어져 있는 타깃은
	// 무적(OnDamage 게이트)이라 여기서도 헛침으로 친다. 예고 중에 다른 개체에게 처형당하기 시작한 타깃도 헛침이다 (D3)
	const float HalfHeight = Target.GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const bool bInReach = FVector::Dist2D(Reached, TargetLocation) <= MonsterDef->CatchRadius;
	const bool bLevel = FMath::Abs(Reached.Z - TargetLocation.Z) <= Radius + HalfHeight;
	const bool bHit = bInReach && bLevel && !Target.IsKnockedDown() && !Target.IsExecuted() && HasLineOfSight(Target);

	PounceResultServerTime = GetServerTime();
	bPounceHit = bHit;
	if (bHit)
	{
		HostCatch(Target);
	}
	else
	{
		// 헛침 — KillCooldown 은 건드리지 않는다. 간격이 다시 좁혀지면(멈추거나 돌면) PounceMissCooldown 뒤 재예고
		PounceMissRemaining = MonsterDef->PounceMissCooldown;
		ResumeChase();
	}
	// 결과 시각·성패는 ABP 의 헛침 클립 시계다 — 다음 20Hz 주기를 기다리지 않고 바로 보낸다
	ForceNetUpdate();
}

float ACL9Monster::GetPounceResultElapsed() const
{
	// 결과가 없으면 PounceResultServerTime 이 −1000 이라 서버 시각(0 이상) + 1000 ≥ 1000 — ABP 의 헛침 창(< 0.3)이 스폰 직후 열리지 않는다
	return GetServerTime() - PounceResultServerTime;
}

void ACL9Monster::OnDamage(int32 Amount, AActor* Source, float Knockback)
{
	if (!HasAuthority() || MonsterDef == nullptr || State == ECL9MonsterState::Dead)
	{
		return;
	}

	Hp = FMath::Max(Hp - Amount, 0);
	if (Hp <= 0)
	{
		// 죽이는 타격에 넉백은 없다 — 죽음 발사가 대신한다
		Die(Source);
		return;
	}

	// 구조는 없다 (D4): 처형 중에는 피해만 받고 경직·상태 변경이 없다 — 죽이면 Die 가 피해자를 먼저 죽인다
	if (State == ECL9MonsterState::Execute)
	{
		UE_LOG(LogCL9, Log, TEXT("'%s' hit for %d while executing — no stun (HP %d)."), *GetNameSafe(this), Amount, Hp);
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

	// 타격 넉백(아이템 행 HitKnockback): Source → 몬스터 수평, 없거나 겹치면 뒤쪽(OnShove 와 같은 폴백). Stun 진입 뒤 HitStopTime 을
	// 기다렸다가 HitKnockbackTime 동안 등속으로 — 경직 시간과는 별개 시계다. 0 이면 옛 동작(제자리 경직)
	KnockbackRemaining = 0.f;
	if (Knockback > 0.f)
	{
		const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
		const UCL9DefRegistry* Registry = GameInstance != nullptr ? GameInstance->GetDefRegistry() : nullptr;
		const UCL9RulesDef* Rules = Registry != nullptr ? Registry->GetRules() : nullptr;
		if (Rules != nullptr)
		{
			FVector Away = Source != nullptr ? GetActorLocation() - Source->GetActorLocation() : FVector::ZeroVector;
			Away.Z = 0.f;
			KnockbackDir = Away.GetSafeNormal();
			if (KnockbackDir.IsNearlyZero())
			{
				KnockbackDir = -GetActorForwardVector();
			}
			KnockbackRemaining = Knockback;
			KnockbackSpeed = Knockback / Rules->HitKnockbackTime;
			KnockbackDelay = Rules->HitStopTime;
			KnockbackFrom = GetActorLocation();
			KnockbackTotal = Knockback;
			KnockbackSteps = 0;
		}
	}
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
	// 구조는 없다 (D4): 처형 중에는 밀쳐도 넉백·경직·상태 변경이 없다
	if (State == ECL9MonsterState::Execute)
	{
		UE_LOG(LogCL9, Log, TEXT("'%s' shoved by '%s' while executing — ignored."), *GetNameSafe(this), *GetNameSafe(Shover));
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
	KnockbackDelay = 0.f;
	KnockbackFrom = GetActorLocation();
	KnockbackTotal = MonsterDef->PushKnockback;
	KnockbackSteps = 0;

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

void ACL9Monster::OnStrike(int32 Damage, ECL9HitStun Stun, float Knockback, AActor* Source)
{
	if (!HasAuthority() || MonsterDef == nullptr || State == ECL9MonsterState::Dead)
	{
		return;
	}

	// 피해가 먼저 — 죽으면 경직·넉백은 없다. 피해 있는 타격은 OnDamage가 HitStun 경직과 HitKnockback 넉백까지 주므로
	// Hit 종류는 거기서 끝나고, Push는 그 위에 밀쳐냄으로 덮는다(경직 시간·넉백도 PushStun·PushKnockback 으로).
	if (Damage > 0)
	{
		OnDamage(Damage, Source, Knockback);
		if (State == ECL9MonsterState::Dead || Stun != ECL9HitStun::Push)
		{
			return;
		}
	}
	switch (Stun)
	{
	case ECL9HitStun::Hit:
		OnDamage(0, Source, Knockback);
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
	// 처형 중에 죽어도 피해자는 죽는다 (D4 — 구조 없음). 피해자의 뷰 타깃은 캐릭터의 사망 흐름(HandlePlayerDeath → 관전)이 넘긴다
	if (State == ECL9MonsterState::Execute && !bExecuteKilled)
	{
		if (ACL9Character* Victim = ExecuteTarget.Get(); Victim != nullptr && !Victim->IsDead())
		{
			bExecuteKilled = true;
			Victim->HostKill(this);
			UE_LOG(LogCL9, Log, TEXT("'%s' died mid-execution — '%s' is killed anyway."), *GetNameSafe(this), *GetNameSafe(Victim));
		}
	}

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
	case ECL9MonsterState::Stalk:
		// 무리가 울면 몰래 다가갈 이유가 없다 — 격분해 그대로 추격 (D4)
		EnterState(ECL9MonsterState::Chase);
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
	case ECL9MonsterState::Stalk:
		// 죽음의 비명에 스토킹은 격분으로 바뀐다 — 타깃은 이미 있으니 곧장 Chase (D4)
		EnterState(ECL9MonsterState::Chase);
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
	// 보정과 처형 카메라의 켜고 끔이다.
	if (MeshComponent != nullptr)
	{
		const float YawOffset = MonsterDef ? MonsterDef->MeshYawOffset : 0.f;
		MeshComponent->SetRelativeRotation(FRotator(0.f, YawOffset, 0.f));
	}

	// 처형 카메라: Execute 에서만 켠다 — 호스트는 EnterState, 클라는 OnRep_State 로 같은 자리에 온다 (D6). 소켓은 행 값
	// (본 이름 허용) — 있으면 그 본의 소켓 공간, **없으면 액터 루트(구 콜리전)에 액터 공간**으로 붙는다: 메시는 `MeshYawOffset`
	// 만큼 돌아 있고 `Radius` 만큼 내려가 있어 메시 루트 기준 오프셋은 종마다 축이 달라진다(2026-09-20 돼지 "뒷다리 사이" 사고).
	// 액터 공간이면 표의 `ExecuteCameraOffset` 이 "X = 전방(타깃 쪽)·Z = 위" 그대로 읽힌다. 처형 밖에서는 꺼져 있다
	if (ExecuteCamera != nullptr && MeshComponent != nullptr)
	{
		const bool bExecute = State == ECL9MonsterState::Execute;
		if (bExecute && MonsterDef != nullptr)
		{
			const bool bSocket = MeshComponent->DoesSocketExist(MonsterDef->ExecuteCameraSocket);
			USceneComponent* Parent = bSocket ? static_cast<USceneComponent*>(MeshComponent) : GetRootComponent();
			ExecuteCamera->AttachToComponent(Parent, FAttachmentTransformRules::KeepRelativeTransform, bSocket ? MonsterDef->ExecuteCameraSocket : NAME_None);
			ExecuteCamera->SetRelativeLocationAndRotation(MonsterDef->ExecuteCameraOffset, MonsterDef->ExecuteCameraRotation);
		}
		if (ExecuteCamera->IsActive() != bExecute)
		{
			ExecuteCamera->SetActive(bExecute);
		}
	}
}

ECL9MonsterState ACL9Monster::GetHuntState() const
{
	// 스토커 종은 발각해도 달려들지 않는다 — 스토킹이 사냥이다. 잡히면 처형인 종(bExecuteOnCatch)은 닭처럼 추격한다 — 잡기가 처형이다.
	// 롤백은 표의 ExecuteRadius 0 (빌드 없이 옛 추격으로)
	return MonsterDef != nullptr && MonsterDef->ExecuteRadius > 0.f ? ECL9MonsterState::Stalk : ECL9MonsterState::Chase;
}

bool ACL9Monster::IsWatchedByAnyone() const
{
	if (MonsterDef->StalkGazeRadius <= 0.f)
	{
		return false;
	}

	for (TActorIterator<ACL9Character> It(GetWorld()); It; ++It)
	{
		const ACL9Character* Candidate = *It;
		// 시체와 처형 중인 사람은 보지 못한다 — 시체가 돼지를 붙들어 두면 안 된다 (D5)
		if (Candidate->IsDead() || Candidate->IsExecuted())
		{
			continue;
		}
		if (FVector::Dist2D(GetActorLocation(), Candidate->GetActorLocation()) > MonsterDef->StalkGazeRadius)
		{
			continue;
		}
		// 2D 요만 비교한다 — 바닥을 보든 천장을 보든 화면 가로 안에 있으면 본 것이다. 조준 요는 호스트가 아는 값
		// (원격 폰은 복제된 컨트롤 회전, ~100ms 지연 허용)
		FVector ToMonster = GetActorLocation() - Candidate->GetActorLocation();
		ToMonster.Z = 0.f;
		const float YawDelta = ToMonster.IsNearlyZero()
			? 0.f
			: FRotator::NormalizeAxis(Candidate->GetBaseAimRotation().Yaw - ToMonster.Rotation().Yaw);
		if (FMath::Abs(YawDelta) <= MonsterDef->StalkGazeHalfAngle && HasLineOfSight(*Candidate))
		{
			return true;
		}
	}
	return false;
}

FVector ACL9Monster::GetBehindPoint(const ACL9Character& Target) const
{
	const FVector Forward = FRotator(0.f, Target.GetActorRotation().Yaw, 0.f).Vector();
	return Target.GetActorLocation() - Forward * MonsterDef->ExecuteAnchorDistance;
}

bool ACL9Monster::SweepExecuteAnchor(const ACL9Character& Target, FVector& OutAnchor) const
{
	const float Radius = MonsterDef->Radius;
	const FVector TargetLocation = Target.GetActorLocation();

	// 앵커 높이는 이 몬스터의 지금 Z — 스포너가 바닥 + Radius 에 놓고 덮침도 Z 평탄이라 같은 층이면 바닥에 딱 맞는다.
	// 타깃 발 높이로 새로 재면 구가 바닥에 접해 시작부터 끼인 것(bStartPenetrating)으로 잡힐 수 있다
	OutAnchor = GetBehindPoint(Target);
	OutAnchor.Z = GetActorLocation().Z;
	const FVector Start(TargetLocation.X, TargetLocation.Y, OutAnchor.Z);

	// 덮침과 같은 스윕 — 자신·타깃·캐릭터·아이템·몬스터는 무시. 타깃이 끌던 수레도 무시한다(수레가 등 뒤에 붙어 있어 앵커를 막는다, D1)
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CL9ExecuteAnchor), false, this);
	Params.AddIgnoredActor(&Target);
	for (TActorIterator<ACL9Character> It(GetWorld()); It; ++It)
	{
		Params.AddIgnoredActor(*It);
	}
	for (TActorIterator<ACL9Item> It(GetWorld()); It; ++It)
	{
		Params.AddIgnoredActor(*It);
	}
	for (TActorIterator<ACL9Monster> It(GetWorld()); It; ++It)
	{
		Params.AddIgnoredActor(*It);
	}
	if (const AActor* Cart = Target.GetPullingCart())
	{
		Params.AddIgnoredActor(Cart);
	}
	const FCollisionObjectQueryParams ObjectParams(ECC_TO_BITFIELD(ECC_WorldStatic) | ECC_TO_BITFIELD(ECC_WorldDynamic));

	FHitResult Hit;
	return !GetWorld()->SweepSingleByObjectType(Hit, Start, OutAnchor, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(Radius), Params);
}

bool ACL9Monster::TryBeginExecute(ACL9Character& Target, bool bCheat)
{
	FName Block;
	FVector Anchor = FVector::ZeroVector;
	const float TargetDistance = FVector::Dist2D(GetActorLocation(), Target.GetActorLocation());

	if (!bCheat)
	{
		// 등 뒤 = 타깃 전방 · (타깃→몬스터 단위벡터) <= −cos(ExecuteBehindAngle) — 몬스터가 타깃 뒤쪽 원뿔 안
		const FVector Forward = FRotator(0.f, Target.GetActorRotation().Yaw, 0.f).Vector();
		FVector ToMonster = GetActorLocation() - Target.GetActorLocation();
		ToMonster.Z = 0.f;
		const float BehindDot = FVector::DotProduct(Forward, ToMonster.GetSafeNormal());
		const bool bBehind = !ToMonster.IsNearlyZero() && BehindDot <= -FMath::Cos(FMath::DegreesToRadians(MonsterDef->ExecuteBehindAngle));

		if (StalkUnseenTime < MonsterDef->ExecuteUnseenTime)
		{
			Block = TEXT("unseen-time");
		}
		else if (TargetDistance > MonsterDef->ExecuteRadius)
		{
			Block = TEXT("radius");
		}
		else if (!bBehind)
		{
			Block = TEXT("behind-cone");
		}
		else if (!HasLineOfSight(Target))
		{
			Block = TEXT("line-of-sight");
		}
		else if (CatchCooldownRemaining > 0.f)
		{
			Block = TEXT("kill-cooldown");
		}
	}
	if (Block.IsNone())
	{
		// 치트도 건너뛰지 않는 검사: 타깃 상태와 앵커
		if (Target.IsKnockedDown())
		{
			Block = TEXT("target-knocked-down");
		}
		else if (Target.IsExecuted())
		{
			Block = TEXT("target-executed");
		}
		else if (Target.IsDead())
		{
			Block = TEXT("target-dead");
		}
		else if (!SweepExecuteAnchor(Target, Anchor))
		{
			Block = TEXT("anchor-blocked");
		}
	}

	if (!Block.IsNone())
	{
		// 같은 사유가 틱마다 반복되면 소음이다 — 바뀔 때만. 치트는 한 번뿐이라 Log
		if (bCheat)
		{
			UE_LOG(LogCL9, Log, TEXT("'%s' cannot execute '%s': %s."), *GetNameSafe(this), *GetNameSafe(&Target), *Block.ToString());
		}
		else if (Block != LastExecuteBlock)
		{
			LastExecuteBlock = Block;
			UE_LOG(LogCL9, Verbose, TEXT("'%s' execute check on '%s' fails: %s (unseen %.2f/%.2f s, dist %.0f/%.0f cm)."),
				*GetNameSafe(this), *GetNameSafe(&Target), *Block.ToString(),
				StalkUnseenTime, MonsterDef->ExecuteUnseenTime, TargetDistance, MonsterDef->ExecuteRadius);
		}
		return false;
	}

	LastExecuteBlock = NAME_None;
	HostBeginExecute(Target, Anchor, /*bTeleport*/ true);
	return true;
}

void ACL9Monster::HostBeginExecute(ACL9Character& Target, const FVector& Anchor, bool bTeleport)
{
	// 몬스터가 움직이지 피해자가 움직이지 않는다 (D1): 스토커는 앵커로 텔레포트하고, 잡히면 처형인 종은 잡은 자리(Anchor = 지금 위치)에
	// 그대로 선다. 어느 쪽이든 타깃을 향하고, 피해자 캡슐·위치는 그대로
	if (bTeleport)
	{
		SetActorLocation(Anchor, false, nullptr, ETeleportType::TeleportPhysics);
	}
	FVector ToTarget = Target.GetActorLocation() - Anchor;
	ToTarget.Z = 0.f;
	if (!ToTarget.IsNearlyZero())
	{
		HeadingYaw = ToTarget.Rotation().Yaw;
	}
	SetActorRotation(FRotator(0.f, HeadingYaw, 0.f));
	SmoothedLocation = GetActorLocation();
	SmoothedRotation = GetActorRotation();

	ExecuteTarget = &Target;
	EnterState(ECL9MonsterState::Execute);
	Target.HostBeginExecuted(this);

	UE_LOG(LogCL9, Log, TEXT("'%s' begins executing '%s' — %s (%s), yaw %.0f."),
		*GetNameSafe(this), *GetNameSafe(&Target), bTeleport ? TEXT("teleported to") : TEXT("executes on catch at"),
		*Anchor.ToCompactString(), HeadingYaw);
}

bool ACL9Monster::HostCheatExecute(ACL9Character& Target)
{
	if (!HasAuthority() || MonsterDef == nullptr || State == ECL9MonsterState::Dead || State == ECL9MonsterState::Execute)
	{
		return false;
	}
	if (!MonsterDef->IsExecutioner())
	{
		UE_LOG(LogCL9, Log, TEXT("'%s' is not an executioner species (ExecuteRadius 0, bExecuteOnCatch false)."), *GetNameSafe(this));
		return false;
	}
	if (MonsterDef->bExecuteOnCatch && MonsterDef->ExecuteRadius <= 0.f)
	{
		// 잡히면 처형인 종은 앵커가 없다 — 잡기와 같은 타깃 상태 검사만 하고 지금 자리에서 시작한다(스윕·텔레포트 없음)
		FName Block;
		if (Target.IsKnockedDown())
		{
			Block = TEXT("target-knocked-down");
		}
		else if (Target.IsExecuted())
		{
			Block = TEXT("target-executed");
		}
		else if (Target.IsDead())
		{
			Block = TEXT("target-dead");
		}
		if (!Block.IsNone())
		{
			UE_LOG(LogCL9, Log, TEXT("'%s' cannot execute '%s': %s."), *GetNameSafe(this), *GetNameSafe(&Target), *Block.ToString());
			return false;
		}
		HostBeginExecute(Target, GetActorLocation(), /*bTeleport*/ false);
		return true;
	}
	return TryBeginExecute(Target, /*bCheat*/ true);
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
