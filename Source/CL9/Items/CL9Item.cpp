// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9Item.h"
#include "Character/CL9Character.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
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
#include "GameMode/CL9GameState.h"

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
	DOREPLIFETIME(ACL9Item, bLightOn);
	DOREPLIFETIME(ACL9Item, Value);
	DOREPLIFETIME(ACL9Item, CookAlpha);
	DOREPLIFETIME(ACL9Item, CookPhase);
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

	SetupLight();

	// 시뮬레이션 방식(물리냐 단순 바운스냐)은 행이 결정한다 — 다시 적용해 준다
	ApplyHoldState();
}

bool ACL9Item::IsFlashlight() const
{
	return ItemDef != nullptr && ItemDef->bEmitsLight && ItemDef->LightKind == ECL9ItemLight::Flashlight;
}

void ACL9Item::SetupLight()
{
	// 행의 조명 필드가 여기서 실제 컴포넌트로 바뀐다 — 발광(달걀)은 점광원으로, 손전등은
	// 앞을 향한 스포트라이트로 (docs/systems/light.md). 한 번만 만들어지며, SubAlias
	// 재로드(조리됨/균열)가 일어나도 같은 컴포넌트를 그대로 쓴다.
	if (LightComponent != nullptr || ItemDef == nullptr || !ItemDef->bEmitsLight)
	{
		return;
	}

	if (ItemDef->LightKind == ECL9ItemLight::Flashlight)
	{
		USpotLightComponent* Spot = NewObject<USpotLightComponent>(this, TEXT("ItemLight"));
		Spot->SetOuterConeAngle(ItemDef->LightConeAngle);
		Spot->SetInnerConeAngle(FMath::Max(0.f, ItemDef->LightConeAngle - 8.f));
		Spot->SetAttenuationRadius(FMath::Max(ItemDef->LightRadius, 100.f));
		LightComponent = Spot;
	}
	else
	{
		UPointLightComponent* Point = NewObject<UPointLightComponent>(this, TEXT("ItemLight"));
		Point->SetAttenuationRadius(FMath::Max(ItemDef->LightRadius, 50.f));
		LightComponent = Point;
	}

	LightComponent->SetIntensity(ItemDef->LightIntensity);
	LightComponent->SetupAttachment(MeshComponent);
	LightComponent->SetVisibility(false);
	LightComponent->RegisterComponent();
	UpdateLightVisibility();
}

void ACL9Item::SetLightSwitch(bool bOn)
{
	if (!HasAuthority() || !IsFlashlight())
	{
		return;
	}

	// 정전은 손전등의 천적이다 — 정전 중에는 켤 수 없다 (light.md).
	// 착용한 램프는 착용자 본인의 장비라 정전의 영향을 받지 않는다.
	const ACL9GameState* CL9GameState = GetWorld() ? GetWorld()->GetGameState<ACL9GameState>() : nullptr;
	if (bOn && !IsWorn() && CL9GameState != nullptr && CL9GameState->IsBlackout())
	{
		return;
	}

	if (bLightOn != bOn)
	{
		bLightOn = bOn;
		OnRep_LightOn();
	}
}

void ACL9Item::OnRep_LightOn()
{
	UpdateLightVisibility();
}

void ACL9Item::UpdateLightVisibility()
{
	if (LightComponent == nullptr)
	{
		return;
	}

	// 모든 머신이 같은 결과를 도출한다: 스위치가 켜져 있으면 켜지되, 손전등이라면
	// 정전을 뺀다 (발광은 계속 켜져 있다 — 남는 쪽은 발광이다, light.md).
	bool bVisible = bLightOn;
	if (bVisible && IsFlashlight() && !IsWorn())
	{
		const ACL9GameState* CL9GameState = GetWorld() ? GetWorld()->GetGameState<ACL9GameState>() : nullptr;
		if (CL9GameState != nullptr && CL9GameState->IsBlackout())
		{
			bVisible = false;
		}
	}
	LightComponent->SetVisibility(bVisible);
}

bool ACL9Item::CanBePickedUp() const
{
	// 수레 짐칸에 실린 아이템도 계속 집을 수 있다 — 짐칸에서 꺼내는 것은 바닥에서
	// 줍는 것과 똑같이 조준하고 누르면 된다 (2026-09-02 상호작용 통합).
	return (HoldState == ECL9ItemHoldState::Free || HoldState == ECL9ItemHoldState::OnCart)
		&& ItemDef != nullptr
		&& ItemDef->CarrySlot != ECL9CarrySlot::CartOnly;
}

void ACL9Item::OnPickedUp(ACL9Character* Holder, const FVector& RelativeLocation)
{
	if (!HasAuthority())
	{
		return;
	}

	HoldState = ECL9ItemHoldState::Held;
	SimVelocity = FVector::ZeroVector;
	bAtRest = true;
	ApplyHoldState();

	// 발광은 소지되는 순간 켜지고, 손전등은 꺼진 채로 시작해 소지자가 직접
	// 토글해야 한다 (docs/systems/light.md).
	if (ItemDef != nullptr && ItemDef->bEmitsLight)
	{
		bLightOn = (ItemDef->LightKind == ECL9ItemLight::Glow);
		OnRep_LightOn();
	}

	AttachToComponent(Holder->GetCapsuleComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	SetActorRelativeLocation(RelativeLocation);
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

	// 손에 든 손전등과 시작 방식이 같다: 착용자가 토글하기 전까지는 꺼져 있다
	if (ItemDef != nullptr && ItemDef->bEmitsLight)
	{
		bLightOn = (ItemDef->LightKind == ECL9ItemLight::Glow);
		OnRep_LightOn();
	}

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

	// 내려놓은 아이템은 불이 꺼진다 — 발광은 소지 중일 때만 빛나고, 손전등은 꺼진다
	if (bLightOn)
	{
		bLightOn = false;
		OnRep_LightOn();
	}
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

	if (bLightOn)
	{
		bLightOn = false;
		OnRep_LightOn();
	}

	if (SphereComponent->IsSimulatingPhysics())
	{
		SphereComponent->SetPhysicsLinearVelocity(LaunchVelocity);
	}
}

void ACL9Item::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 켜진 손전등은 모든 머신에서 정전 여부를 매 틱 확인해서, 정전이 시작되는
	// 순간 빛이 꺼지게 한다 — 반납은 그 순간을 앞당길 수 있으므로 고정 타이머로는
	// 틀린다 (docs/systems/light.md). 호스트는 스위치도 꺼서 이 상태를 유지시킨다.
	if (bLightOn && IsFlashlight() && !IsWorn())
	{
		UpdateLightVisibility();
		if (HasAuthority())
		{
			const ACL9GameState* CL9GameState = GetWorld() ? GetWorld()->GetGameState<ACL9GameState>() : nullptr;
			if (CL9GameState != nullptr && CL9GameState->IsBlackout())
			{
				bLightOn = false;
			}
		}
	}

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
