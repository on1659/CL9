// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9Item.h"
#include "Character/CL9Character.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CL9.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ItemDef.h"
#include "Data/CL9Types.h"
#include "GameMode/CL9GameInstance.h"

namespace
{
	/** 충돌 프레임 속도가 이 값 아래로 떨어지면 튀던 아이템이 멈춘다, cm/s 단위 */
	constexpr float RestSpeed = 30.f;
}

ACL9Item::ACL9Item()
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicateMovement(true);
	Kind = ECL9ActorKind::Item;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Block);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	SphereComponent->SetNotifyRigidBodyCollision(true);
	SphereComponent->OnComponentHit.AddDynamic(this, &ACL9Item::OnPhysicsHit);
	SetRootComponent(SphereComponent);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(SphereComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACL9Item::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACL9Item, HoldState);
	DOREPLIFETIME(ACL9Item, Value);
	DOREPLIFETIME(ACL9Item, CookAlpha);
	DOREPLIFETIME(ACL9Item, CookPhase);
}

FText ACL9Item::GetDisplayName() const
{
	return (ItemDef != nullptr && !ItemDef->DisplayName.IsEmpty()) ? ItemDef->DisplayName : Super::GetDisplayName();
}

void ACL9Item::SetCookState(ECL9CookPhase Phase, float Alpha)
{
	if (!HasAuthority())
	{
		return;
	}

	CookPhase = Phase;
	CookAlpha = (Phase == ECL9CookPhase::None) ? 0 : static_cast<uint8>(FMath::Clamp(Alpha, 0.f, 1.f) * 255.f);
}

void ACL9Item::LoadData()
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9ItemDef* Def = Registry ? Registry->FindItem(GetAlias(), GetSubAlias()) : nullptr;

	if (Def == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("'%s': item row '%s'/'%s' is missing or broken — visuals stay stale (docs/data/schema.md 로드 검사)."),
			*GetNameSafe(this), *GetAlias().ToString(), *GetSubAlias().ToString());
		return;
	}
	// 개체마다 값을 굴린다 — 행이 바뀔 때마다(첫 로드·상태 전환) 호스트가 새 범위에서 뽑고 복제한다
	const bool bRowChanged = ItemDef != Def;
	ItemDef = Def;
	if (HasAuthority() && bRowChanged)
	{
		Value = FMath::RandRange(Def->ValueMin, Def->ValueMax);
	}

	SphereComponent->SetSphereRadius(Def->Radius);

	// 테이블의 마찰/반발 값이 행별 머티리얼을 통해 물리 경로를 움직인다
	if (Def->bPhysicsEnabled)
	{
		if (PhysicsMaterial == nullptr)
		{
			PhysicsMaterial = NewObject<UPhysicalMaterial>(this);
		}
		PhysicsMaterial->Friction = Def->Friction;
		PhysicsMaterial->Restitution = Def->Restitution;
		SphereComponent->SetPhysMaterialOverride(PhysicsMaterial);
	}

	if (UStaticMesh* Mesh = Def->Mesh.LoadSynchronous())
	{
		MeshComponent->SetStaticMesh(Mesh);

		// 임시 메시는 기본 도형이다 — 구매한 에셋이 테이블을 통해 이를 대체하기 전까지는
		// 판정 반지름에 맞춰 크기를 조정해 둔다 (build-order 원칙 2).
		const float MeshRadius = Mesh->GetBounds().SphereRadius;
		if (MeshRadius > UE_KINDA_SMALL_NUMBER)
		{
			MeshComponent->SetWorldScale3D(FVector(Def->Radius / MeshRadius));
		}
	}

	for (UCL9Gimmick* Gimmick : Def->Gimmicks)
	{
		if (Gimmick != nullptr)
		{
			Gimmick->OnActorLoaded(this);
		}
	}

	// 시뮬레이션 방식(물리냐 단순 바운스냐)은 행이 결정한다 — 다시 적용해 준다
	ApplyHoldState();
}

bool ACL9Item::CanBePickedUp() const
{
	// 수레 짐칸에 실린 아이템도 계속 집을 수 있다 — 짐칸에서 꺼내는 것은 바닥에서
	// 줍는 것과 똑같이 조준하고 누르면 된다 (2026-09-02 상호작용 통합).
	return (HoldState == ECL9ItemHoldState::Free || HoldState == ECL9ItemHoldState::OnCart)
		&& ItemDef != nullptr
		&& ItemDef->CarrySlot != ECL9CarrySlot::CartOnly;
}

void ACL9Item::OnPickedUp(ACL9Character* Holder)
{
	if (!HasAuthority() || Holder == nullptr)
	{
		return;
	}

	HoldState = ECL9ItemHoldState::Held;
	SimVelocity = FVector::ZeroVector;
	bAtRest = true;
	ApplyHoldState();

	OnAcquired();

	// 전신 메시의 손에 붙는다 — 팔 메시(OnlyOwnerSee)는 남의 화면에서는 애니메이션이 돌지 않아
	// 소켓이 기준 포즈에 멈춰 있으므로, 어느 화면에서든 움직이는 전신 메시가 기준이다. 팔 메시는
	// 전신 포즈를 복사(ABP_FP_Copy)하니 자기 화면에서도 같은 자리다 (docs/systems/inventory.md).
	// 소켓은 마네킹 스켈레톤이 이미 갖고 있는 `HandGrip_R`(손바닥 앞으로 +X) — 농부 A도 같은
	// 뼈대라 그대로 쓴다. 없는 뼈대면 `hand_r` 뼈에 직접 붙는다.
	USkeletalMeshComponent* Body = Holder->GetMesh();
	static const FName GripSocket(TEXT("HandGrip_R"));
	static const FName HandBone(TEXT("hand_r"));
	const FName Socket = Body->DoesSocketExist(GripSocket) ? GripSocket : HandBone;
	AttachToComponent(Body, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
	SetActorRelativeLocation(ItemDef != nullptr ? ItemDef->GripOffset : FVector::ZeroVector);
	SetActorRelativeRotation(ItemDef != nullptr ? ItemDef->GripRotation : FRotator::ZeroRotator);
}

void ACL9Item::OnRep_Owner()
{
	Super::OnRep_Owner();
	UpdateFirstPersonView();
}

void ACL9Item::OnWorn(ACL9Character* Wearer)
{
	if (!HasAuthority() || Wearer == nullptr)
	{
		return;
	}

	HoldState = ECL9ItemHoldState::Worn;
	SimVelocity = FVector::ZeroVector;
	bAtRest = true;
	ApplyHoldState();

	OnAcquired();

	// 카메라는 소유자의 시선을 온전히 따라간다. 다른 머신에서는 몸통과 함께
	// 요(yaw)만 돌아간다(피치 추종은 나중 단계). 눈 바로 위에 자리한다.
	AttachToComponent(Wearer->GetFirstPersonCameraComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorRelativeLocation(FVector(8.f, 0.f, 10.f));
}

void ACL9Item::OnLoadedIntoCart(AActor* Cart, const FVector& RelativeLocation)
{
	if (!HasAuthority())
	{
		return;
	}

	HoldState = ECL9ItemHoldState::OnCart;
	SimVelocity = FVector::ZeroVector;
	bAtRest = true;
	ApplyHoldState();

	AttachToComponent(Cart->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorRelativeLocation(RelativeLocation);
}

void ACL9Item::OnPlaced(const FVector& WorldLocation)
{
	if (!HasAuthority())
	{
		return;
	}

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocation(WorldLocation);
	HoldState = ECL9ItemHoldState::Free;
	SimVelocity = FVector::ZeroVector;
	bAtRest = false;
	ApplyHoldState();

	OnReleased();
}

void ACL9Item::OnThrown(const FVector& Origin, const FVector& LaunchVelocity)
{
	if (!HasAuthority())
	{
		return;
	}

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocation(Origin);
	HoldState = ECL9ItemHoldState::Free;
	SimVelocity = LaunchVelocity;
	bAtRest = false;
	ApplyHoldState();

	OnReleased();

	if (SphereComponent->IsSimulatingPhysics())
	{
		SphereComponent->SetPhysicsLinearVelocity(LaunchVelocity);
	}
}

void ACL9Item::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 시뮬레이션은 호스트 전용이며 권위를 가진다 — 클라이언트는 복제된 이동값을
	// 보간할 뿐이다 (docs/systems/item.md 액터 틀).
	if (!HasAuthority() || HoldState != ECL9ItemHoldState::Free || ItemDef == nullptr)
	{
		return;
	}

	// 물리 행: 엔진이 시뮬레이션하고, 테이블이 정한 중력값만 (질량과 무관하도록
	// 가속도로) 주입해 준다.
	if (ItemDef->bPhysicsEnabled)
	{
		if (SphereComponent->IsSimulatingPhysics() && SphereComponent->IsAnyRigidBodyAwake())
		{
			SphereComponent->AddForce(FVector(0.f, 0.f, -ItemDef->Gravity), NAME_None, true);
		}
		return;
	}

	if (bAtRest)
	{
		return;
	}

	SimVelocity.Z -= ItemDef->Gravity * DeltaSeconds;

	FHitResult Hit;
	AddActorWorldOffset(SimVelocity * DeltaSeconds, true, &Hit);

	if (Hit.bBlockingHit)
	{
		const float ImpactSpeed = -FVector::DotProduct(SimVelocity, Hit.Normal);
		HandleImpact(ImpactSpeed);

		const FVector NormalPart = FVector::DotProduct(SimVelocity, Hit.Normal) * Hit.Normal;
		const FVector TangentPart = SimVelocity - NormalPart;
		SimVelocity = TangentPart * (1.f - ItemDef->Friction) - NormalPart * ItemDef->Restitution;

		const bool bOnGround = Hit.Normal.Z > 0.7f;
		if (bOnGround && SimVelocity.SizeSquared() < RestSpeed * RestSpeed)
		{
			SimVelocity = FVector::ZeroVector;
			bAtRest = true;
		}
	}
}

void ACL9Item::HandleImpact(float ImpactSpeed)
{
	if (ItemDef == nullptr || !ItemDef->bFragile)
	{
		return;
	}

	// 기본 상태와 균열 상태만 충격 연쇄를 탄다 — 조리됨/탄 것 변형은 조리 상태를
	// 그대로 유지한다 (조합 상태는 미정).
	const bool bCracked = GetSubAlias() == CL9::SubAliasCracked;
	if (!bCracked && !GetSubAlias().IsNone())
	{
		return;
	}

	const float BreakThreshold = bCracked ? ItemDef->CrackedBreakSpeed : ItemDef->BreakSpeed;
	if (ImpactSpeed >= BreakThreshold)
	{
		SetAlias(GetAlias(), CL9::SubAliasBroken);
	}
	else if (!bCracked && ImpactSpeed >= ItemDef->CrackSpeed)
	{
		SetAlias(GetAlias(), CL9::SubAliasCracked);
	}
}

void ACL9Item::OnRep_HoldState()
{
	ApplyHoldState();
}

void ACL9Item::ApplyHoldState()
{
	// 손에 든 아이템은 소지자의 캡슐에 얹혀 있다 — 자기 자신의 충돌체가 켜져
	// 있으면 소지자를 밀어내 버리므로 충돌을 꺼 둔다. 수레 짐칸의 아이템은
	// 조준 감지가 계속 찾을 수 있도록 쿼리 충돌은 유지한다.
	SetActorEnableCollision(HoldState != ECL9ItemHoldState::Held && HoldState != ECL9ItemHoldState::Worn);

	// 착용한 램프는 착용자 눈앞에 있다 — 다른 사람에게만 보인다
	MeshComponent->SetOwnerNoSee(HoldState == ECL9ItemHoldState::Worn);
	UpdateFirstPersonView();

	// 실제 물리는 호스트에서 자유 상태인 아이템에만 돌아간다 (2026-09-03 결정) —
	// 클라이언트는 복제된 이동을 렌더링할 뿐이고, 그 외 모든 상태는 부착돼 있다.
	const bool bWantPhysics = HoldState == ECL9ItemHoldState::Free
		&& HasAuthority()
		&& ItemDef != nullptr
		&& ItemDef->bPhysicsEnabled;
	if (SphereComponent->IsSimulatingPhysics() != bWantPhysics)
	{
		SphereComponent->SetSimulatePhysics(bWantPhysics);
	}
	if (bWantPhysics)
	{
		// 테이블의 Gravity 열이 월드 중력을 대체한다 (Tick에서 주입됨)
		SphereComponent->SetEnableGravity(false);
	}
}

void ACL9Item::UpdateFirstPersonView()
{
	// 1인칭 팔은 별도 FOV·스케일(FirstPersonFieldOfView·FirstPersonScale)로 그려진다. 손에 든
	// 아이템을 월드 공간 그대로 두면 소유자 화면에서 팔과 어긋난 자리에 뜨므로, 소유자 본인이
	// 들고 있는 동안만 팔과 같은 1인칭 프리미티브로 그린다. 남의 화면·바닥에서는 월드 공간이다.
	ACL9Character* Holder = Cast<ACL9Character>(GetOwner());
	const bool bInLocalHand = HoldState == ECL9ItemHoldState::Held && Holder != nullptr && Holder->IsLocallyControlled();
	const EFirstPersonPrimitiveType Wanted = bInLocalHand ? EFirstPersonPrimitiveType::FirstPerson : EFirstPersonPrimitiveType::None;
	if (MeshComponent->FirstPersonPrimitiveType != Wanted)
	{
		MeshComponent->SetFirstPersonPrimitiveType(Wanted);
	}

	// 1인칭 팔은 전신 포즈를 복사한 뒤 컨트롤 릭(CtrlRig_FPWarp)으로 카메라에 맞게 비틀어서 손 위치가
	// 전신의 손과 십수 cm 어긋난다. 액터는 전신 손에 붙어 복제되지만(남의 화면 기준), 소유자 본인
	// 화면에서는 **메시 컴포넌트만** 팔 메시의 같은 소켓으로 옮겨 팔에 정확히 들려 보이게 한다 —
	// 컴포넌트 부착은 복제되지 않으므로 다른 머신에는 영향이 없다.
	static const FName GripSocket(TEXT("HandGrip_R"));
	static const FName HandBone(TEXT("hand_r"));
	USkeletalMeshComponent* Arms = bInLocalHand ? Holder->GetFirstPersonMesh() : nullptr;
	if (Arms != nullptr && Arms->GetSkeletalMeshAsset() != nullptr)
	{
		if (MeshComponent->GetAttachParent() != Arms)
		{
			const FName Socket = Arms->DoesSocketExist(GripSocket) ? GripSocket : HandBone;
			MeshComponent->AttachToComponent(Arms, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
			MeshComponent->SetRelativeLocation(ItemDef != nullptr ? ItemDef->GripOffset : FVector::ZeroVector);
			MeshComponent->SetRelativeRotation(ItemDef != nullptr ? ItemDef->GripRotation : FRotator::ZeroRotator);
		}
	}
	else if (MeshComponent->GetAttachParent() != SphereComponent)
	{
		MeshComponent->AttachToComponent(SphereComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		MeshComponent->SetRelativeLocation(FVector::ZeroVector);
		MeshComponent->SetRelativeRotation(FRotator::ZeroRotator);
	}
}

void ACL9Item::OnPhysicsHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority() || ItemDef == nullptr || !ItemDef->bFragile || HoldState != ECL9ItemHoldState::Free)
	{
		return;
	}

	// 충격량 ≈ m·(1+e)·v_충돌 이므로, 여기서 접근 속도를 역산한다
	const float Mass = FMath::Max(HitComponent->GetMass(), UE_KINDA_SMALL_NUMBER);
	const float ImpactSpeed = NormalImpulse.Size() / (Mass * (1.f + ItemDef->Restitution));
	HandleImpact(ImpactSpeed);
}
