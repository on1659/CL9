// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9Item.h"
#include "Character/CL9Character.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicsEngine/BodyInstance.h"
#include "CL9.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ItemDef.h"
#include "Data/CL9Types.h"
#include "GameMode/CL9GameInstance.h"

namespace
{
	/** 충돌 프레임 속도가 이 값 아래로 떨어지면 튀던 아이템이 멈춘다, cm/s 단위 */
	constexpr float RestSpeed = 30.f;

	/**
	 * 물리 행의 자체 잠들기 판정 — 이만큼(cm·도) 안에서 이 시간(초) 동안 머물면 몸체를 재운다. 속도가 아니라 변위로 재는 이유:
	 * 매 틱 주입하는 중력 힘 때문에 바닥에 선 몸체도 g·dt(100fps에 20cm/s, 30fps에 65cm/s)의 속도를 보고해 속도 문턱이
	 * 프레임률을 탄다. 변위는 프레임률과 무관하다. (입력·엔진 상수, 밸런스 값이 아니다)
	 */
	constexpr float RestMoveTolerance = 1.f;
	constexpr float RestTurnTolerance = 2.f;
	constexpr float RestHoldTime = 0.5f;
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

void ACL9Item::HostSetValue(int32 NewValue)
{
	if (!HasAuthority())
	{
		return;
	}

	Value = NewValue;
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

	UStaticMesh* Mesh = Def->Mesh.LoadSynchronous();
	if (Mesh != nullptr)
	{
		MeshComponent->SetStaticMesh(Mesh);
	}

	if (Def->bMeshIsBody)
	{
		// 메시가 곧 몸체(수레): 임포트한 UCX 심플 콜리전을 실물 크기 그대로 쓴다 — 반경 구도, 구에 맞추는 자동
		// 스케일도 없다. 그 자동 스케일이 수레를 계란만 하게 보이게 한 원인이었다 (2026-09-13 사용자 지적).
		MeshComponent->SetRelativeScale3D(FVector::OneVector);
		// 질량 오버라이드는 바디를 새로 만들기 전에 BodyInstance에 심어 두고(재생성된 바디가 이 값으로 태어난다), 만든 뒤 살아 있는
		// 바디에 한 번 더 적용한다 — 2026-09-13 PIE에서 행 25kg 대신 엔진 계산 95.6kg이 보였다.
		MeshComponent->BodyInstance.SetMassOverride(Def->Mass, Def->Mass > 0.f);
		PromoteMeshToBody();
		MeshComponent->SetMassOverrideInKg(NAME_None, Def->Mass, Def->Mass > 0.f);
	}
	else
	{
		SphereComponent->SetSphereRadius(Def->Radius);

		// 임시 메시는 기본 도형이다 — 구매한 에셋이 테이블을 통해 이를 대체하기 전까지는
		// 판정 반지름에 맞춰 크기를 조정해 둔다 (build-order 원칙 2).
		const float MeshRadius = Mesh != nullptr ? Mesh->GetBounds().SphereRadius : 0.f;
		if (MeshRadius > UE_KINDA_SMALL_NUMBER)
		{
			MeshComponent->SetWorldScale3D(FVector(Def->Radius / MeshRadius));
		}
	}

	// 테이블의 마찰/반발 값이 행별 머티리얼을 통해 물리 경로를 움직인다 — 몸체가 구든 메시든 같다
	if (Def->bPhysicsEnabled)
	{
		if (PhysicsMaterial == nullptr)
		{
			PhysicsMaterial = NewObject<UPhysicalMaterial>(this);
		}
		PhysicsMaterial->Friction = Def->Friction;
		PhysicsMaterial->Restitution = Def->Restitution;
		GetBody()->SetPhysMaterialOverride(PhysicsMaterial);
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
	// 수레 짐칸 안의 아이템도 자유 상태라 그대로 집힌다 — 짐칸에서 꺼내는 것은 바닥에서 줍는 것과 똑같이
	// 조준하고 누르면 된다. 메시가 몸체인 물건(수레)은 손에 들리지 않는다 (docs/plan/data/item/cart.md).
	return HoldState == ECL9ItemHoldState::Free
		&& ItemDef != nullptr
		&& ItemDef->CarrySlot != ECL9CarrySlot::CartOnly
		&& !ItemDef->bMeshIsBody;
}

void ACL9Item::OnPickedUp(ACL9Character* Holder)
{
	// 메시가 몸체인 물건(수레)은 CanBePickedUp이 걸러 여기 오지 않는다 — 부착·1인칭 경로는 구가 루트인 아이템 전용이다
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
	// 소켓이 기준 포즈에 멈춰 있으므로, 어느 화면에서든 움직이는 전신 메시가 기준이다. 소유자 본인
	// 화면에서는 UpdateFirstPersonView가 메시만 팔(카메라 자식 뷰모델)로 옮긴다 (docs/systems/inventory.md).
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

void ACL9Item::OnPlaced(const FVector& WorldLocation)
{
	if (!HasAuthority())
	{
		return;
	}

	// 물리 몸체는 손에 들린 동안에도 살아 있다(충돌만 꺼진 키네마틱). 텔레포트 없이 옮기면 키네마틱 목표점만 잡히고, 같은 틱에
	// 시뮬레이션을 켜면 그 목표점은 버려져 몸체가 **손 자리에서** 깨어난다 — 놓은 계란이 놓기 지점이 아니라 오른손 자리(폰 25cm 앞·
	// 24cm 오른쪽)에 떨어지던 원인이다 (2026-09-14 PIE 재현 셋 다 같은 자리). TeleportPhysics는 위치·회전을 바로 쓰고 속도도 0으로.
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocation(WorldLocation, false, nullptr, ETeleportType::TeleportPhysics);
	HoldState = ECL9ItemHoldState::Free;
	SimVelocity = FVector::ZeroVector;
	bAtRest = false;
	ApplyHoldState();

	// 손의 흔들림·그립 회전에서 물려받은 속도가 남지 않게 — 놓은 자리에서 그대로 떨어진다
	UPrimitiveComponent* Body = GetBody();
	if (Body->IsSimulatingPhysics())
	{
		Body->SetPhysicsLinearVelocity(FVector::ZeroVector);
		Body->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}

	OnReleased();
}

void ACL9Item::OnThrown(const FVector& Origin, const FVector& LaunchVelocity)
{
	if (!HasAuthority())
	{
		return;
	}

	// OnPlaced와 같은 이유로 텔레포트 — 아니면 던지기 원점이 무시되고 손 자리에서 출발한다
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorLocation(Origin, false, nullptr, ETeleportType::TeleportPhysics);
	HoldState = ECL9ItemHoldState::Free;
	SimVelocity = LaunchVelocity;
	bAtRest = false;
	ApplyHoldState();

	OnReleased();

	UPrimitiveComponent* Body = GetBody();
	if (Body->IsSimulatingPhysics())
	{
		Body->SetPhysicsLinearVelocity(LaunchVelocity);
		Body->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}
}

void ACL9Item::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 시뮬레이션은 호스트 전용이며 권위를 가진다 — 클라이언트는 복제된 이동값을 보간할 뿐이다
	// (docs/systems/item.md 액터 틀). 자유 상태와 밀리는 수레(Pushed)만 시뮬레이션한다.
	const bool bSimulated = HoldState == ECL9ItemHoldState::Free || HoldState == ECL9ItemHoldState::Pushed;
	if (!HasAuthority() || !bSimulated || ItemDef == nullptr)
	{
		return;
	}

	// 물리 행: 엔진이 시뮬레이션하고, 테이블이 정한 중력값만 (질량과 무관하도록 가속도로) 주입해 준다. 엔진에는 강체별 중력
	// 배율이 없다(5.8: 중력 그룹 인덱스만 있고 그룹 가속은 월드 중력 그룹 0만 엔진이 채운다) — 그래서 힘 주입이고, 힘 주입은
	// 엔진 잠들기를 막으므로(UpdatePhysicsRest 주석) 멈춤은 여기서 직접 판정해 재운다. 잠든 몸체는 힘을 받지 않고, 수레의
	// 매 틱 깨우기·던지기·다른 몸체와의 충돌(Chaos 아일랜드 깨움)로 깨면 다음 틱부터 다시 중력을 받는다.
	// 밀리는 수레(Pushed)는 재우지 않는다 — 수레 틱이 매 틱 힘을 주고 깨우므로 벽에 막혀 서 있어도 자고 깨기를 되풀이할 뿐이다.
	if (ItemDef->bPhysicsEnabled)
	{
		UPrimitiveComponent* Body = GetBody();
		if (Body->IsSimulatingPhysics() && Body->IsAnyRigidBodyAwake())
		{
			if (bRestAsleep)
			{
				// 누군가 깨웠다(수레·충돌·던지기) — 아직 1cm도 못 움직였을 첫 틱에 곧바로 다시 재우지 않도록 판정을 새로 시작한다
				bRestAsleep = false;
				ResetPhysicsRest();
			}
			if (HoldState == ECL9ItemHoldState::Free && UpdatePhysicsRest(DeltaSeconds))
			{
				Body->PutRigidBodyToSleep();
				bRestAsleep = true;
				return;
			}
			Body->AddForce(FVector(0.f, 0.f, -ItemDef->Gravity), NAME_None, true);
		}
		return;
	}

	// 단순 바운스 시뮬레이션(물리 행이 아닌 것)은 자유 상태에서만 돈다
	if (HoldState != ECL9ItemHoldState::Free || bAtRest)
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

bool ACL9Item::UpdatePhysicsRest(float DeltaSeconds)
{
	const UPrimitiveComponent* Body = GetBody();
	const FVector Location = Body->GetComponentLocation();
	const FQuat Rotation = Body->GetComponentQuat();
	const bool bMoved = FVector::DistSquared(Location, RestAnchorLocation) > FMath::Square(RestMoveTolerance)
		|| Rotation.AngularDistance(RestAnchorRotation) > FMath::DegreesToRadians(RestTurnTolerance);
	if (bMoved)
	{
		RestAnchorLocation = Location;
		RestAnchorRotation = Rotation;
		RestTime = 0.f;
		return false;
	}
	RestTime += DeltaSeconds;
	return RestTime >= RestHoldTime;
}

void ACL9Item::ResetPhysicsRest()
{
	const UPrimitiveComponent* Body = GetBody();
	RestAnchorLocation = Body->GetComponentLocation();
	RestAnchorRotation = Body->GetComponentQuat();
	RestTime = 0.f;
	bRestAsleep = false;
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
	// 있으면 소지자를 밀어내 버리므로 충돌을 꺼 둔다.
	SetActorEnableCollision(HoldState != ECL9ItemHoldState::Held && HoldState != ECL9ItemHoldState::Worn);

	// 착용한 램프는 착용자 눈앞에 있다 — 다른 사람에게만 보인다
	MeshComponent->SetOwnerNoSee(HoldState == ECL9ItemHoldState::Worn);
	UpdateFirstPersonView();

	// 실제 물리는 호스트에서 자유 상태(와 밀리는 수레)에만 돌아간다 (2026-09-03 결정) — 그 외 상태는 부착돼 있다.
	// 클라이언트의 시뮬레이션은 여기서 건드리지 않는다: 엔진이 복제된 bRepPhysics에 맞춰 켜고 끈다
	// (AActor::SyncReplicatedPhysicsSimulation). 클라가 여기서 꺼 버리면 복제 목표가 시뮬레이션하지 않는
	// 바디에 버려져(FPhysicsReplication::ApplyRigidBodyState) 아이템이 그 자리에 얼어붙는다 (2026-09-13).
	if (!HasAuthority())
	{
		return;
	}
	const bool bWantPhysics = (HoldState == ECL9ItemHoldState::Free || HoldState == ECL9ItemHoldState::Pushed)
		&& ItemDef != nullptr
		&& ItemDef->bPhysicsEnabled;
	UPrimitiveComponent* Body = GetBody();
	if (Body->IsSimulatingPhysics() != bWantPhysics)
	{
		Body->SetSimulatePhysics(bWantPhysics);
	}
	if (bWantPhysics)
	{
		// 테이블의 Gravity 열이 월드 중력을 대체한다 (Tick에서 주입됨)
		Body->SetEnableGravity(false);
		// 행의 감쇠 — 각감쇠가 구 몸체의 구르기를 세운다(마찰만으로는 안 선다, 엔진 기본 0). 선형 감쇠는 미끄러짐을 줄인다.
		Body->SetAngularDamping(ItemDef->AngularDamping);
		Body->SetLinearDamping(ItemDef->LinearDamping);
		if (ItemDef->bMeshIsBody)
		{
			// 시뮬레이션을 켜며 질량 속성이 다시 계산된다 — 행의 질량이 살아남게 한 번 더 적용한다
			Body->SetMassOverrideInKg(NAME_None, ItemDef->Mass, ItemDef->Mass > 0.f);
		}
		// 멈춤 판정은 새 자리에서 처음부터 — 놓기·던지기·수레 놓기 직후 지난 자리의 머문 시간이 이어지지 않게
		ResetPhysicsRest();
	}
}

void ACL9Item::UpdateFirstPersonView()
{
	// 메시가 몸체인 물건(수레)은 손에 들리지 않고 메시가 루트다 — 구 아래로 되돌려 붙일 것도, 팔로 옮길 것도 없다
	if (ItemDef != nullptr && ItemDef->bMeshIsBody)
	{
		return;
	}

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

	// 1인칭 팔은 카메라의 자식(뷰모델)이라 시선을 통째로 따르고, 몸 포즈와 무관한 카메라 로컬 클립을
	// 재생하므로 손 위치가 전신의 손과 다르다(2026-09-13, 리썰 방식). 액터는 전신 손에 붙어 복제되지만
	// (남의 화면 기준), 소유자 본인 화면에서는 **메시 컴포넌트만** 팔 메시의 같은 소켓으로 옮겨 팔에
	// 정확히 들려 보이게 한다 — 컴포넌트 부착은 복제되지 않으므로 다른 머신에는 영향이 없다. 팔 소켓이
	// 카메라를 따라가니 아이템도 시선을 따라간다. 메시의 자식(ACL9LightItem의 라이트 — 손전등 렌즈 발원)은
	// 부착을 따라 같이 옮겨 간다.
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

UPrimitiveComponent* ACL9Item::GetBody() const
{
	return CastChecked<UPrimitiveComponent>(GetRootComponent());
}

void ACL9Item::PromoteMeshToBody()
{
	if (GetRootComponent() == MeshComponent)
	{
		return;
	}

	// 구는 물러난다 — 시뮬레이션·충돌을 끄고 메시 아래에 붙여 둔다(컴포넌트 포인터를 쓰는 코드가 남아 있어 파괴하지 않는다)
	SphereComponent->SetSimulatePhysics(false);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	SetRootComponent(MeshComponent);
	SphereComponent->AttachToComponent(MeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	// 메시가 구의 역할을 그대로 잇는다: 조준 감지(WorldDynamic 겹침 질의)와 물리, 카메라는 무시. 폰은 막는다 —
	// 계란과 달리 수레는 사람이 지나다닐 수 없는 크기다(미는 사람도 수레에 막힌다). 시뮬레이션하는 루트는 Movable이어야
	// 한다(스태틱 메시 컴포넌트 기본 Mobility가 Static이면 정적 바디가 만들어져 질량도 시뮬도 붙지 않는다).
	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	MeshComponent->SetNotifyRigidBodyCollision(true);
	MeshComponent->OnComponentHit.AddUniqueDynamic(this, &ACL9Item::OnPhysicsHit);

	// 배치·스폰 때 바닥이나 캡슐과 겹쳐 있어도 튕겨 나가지 않는다 — Chaos의 초기 겹침 디페네트레이션은 상한이 없어 25kg 수레를
	// 수천 cm/s로 발사했다(2026-09-13 PIE: 바닥 관통·옆으로 발사). 0은 "겹친 채 잠들고, 이후 보통의 접촉 해소에 맡긴다"는
	// 엔진 문서의 값이다(BodyInstance.h MaxDepenetrationVelocity). CCD는 빠를 때 얇은 바닥·짐칸 바닥판을 뚫지 않게 한다 —
	// 던진 계란은 1680cm/s = 프레임당 28cm이고, 짝 중 하나만 CCD여도 그 짝은 CCD로 푼다.
	MeshComponent->BodyInstance.SetMaxDepenetrationVelocity(0.f);
	MeshComponent->SetUseCCD(true);

	// 바디를 지금의 트랜스폼·BodySetup으로 새로 만든다 — BP 기본값이 메시 콜리전을 켜 두었다면 구의 자식일 때 바디가 먼저 생겼을 수
	// 있고(웰딩·옛 트랜스폼), 그 바디로는 SetSimulatePhysics가 헛돈다.
	MeshComponent->RecreatePhysicsState();
	LogBodyDiagnostics();

	// 클라이언트: 복제 이동이 이미 bRepPhysics=true로 와 있었으면 옛 루트(구)만 시뮬레이션이 켜져 있다 — 새 루트에
	// 다시 맞춘다(호스트에서는 아직 시뮬레이션 전이라 아무 일도 없다). 이걸 빼먹으면 클라의 수레가 얼어붙는다.
	SyncReplicatedPhysicsSimulation();
}

void ACL9Item::LogBodyDiagnostics() const
{
	// 메시 몸체가 실제로 무엇으로 시뮬레이션되는지 로그로 남긴다 — 심플 콜리전이 없으면(ComplexAsSimple·UCX 누락) 바디에 모양이 없어
	// 모든 것을 뚫고 떨어지고, UCX가 배율을 잘못 타면 렌더 바운드보다 훨씬 큰 몸체가 바닥·벽에 깊이 박혀 발사된다.
	const FBodyInstance* Body = MeshComponent->GetBodyInstance();
	const FBox BodyBox = (Body != nullptr && Body->IsValidBodyInstance()) ? Body->GetBodyBounds() : FBox(ForceInit);
	const FVector RenderExtent = MeshComponent->Bounds.BoxExtent;
	if (!BodyBox.IsValid)
	{
		UE_LOG(LogCL9, Error, TEXT("'%s': mesh body has no simple collision — it will fall through everything (import with UCX_ hulls, collision Simple; docs/plan/data/item/cart.md)."),
			*GetNameSafe(this));
		return;
	}
	const FVector BodyExtent = BodyBox.GetExtent();
	if (BodyExtent.GetMax() > RenderExtent.GetMax() * 1.5f)
	{
		UE_LOG(LogCL9, Warning, TEXT("'%s': mesh body half-size %s is far larger than the render half-size %s — the UCX collision looks mis-scaled (docs/plan/data/item/cart.md)."),
			*GetNameSafe(this), *BodyExtent.ToString(), *RenderExtent.ToString());
	}
	else
	{
		UE_LOG(LogCL9, Log, TEXT("'%s': mesh body half-size %s, render half-size %s, mass %.1f kg."),
			*GetNameSafe(this), *BodyExtent.ToString(), *RenderExtent.ToString(), MeshComponent->GetMass());
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
