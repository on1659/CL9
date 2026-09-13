// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9Cart.h"
#include "Internationalization/StringTableRegistry.h"
#include "Character/CL9Character.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CL9.h"
#include "Data/CL9ItemDef.h"
#include "GameMode/CL9GameState.h"

namespace
{
	/** 밀리는 수레의 수평 속도 상한 = 미는 사람 최대 속도 × 이 값 — 처진 만큼 따라잡을 여유만 준다 (안전 클램프) */
	constexpr float PushSpeedMargin = 1.2f;

	/** 밀리는 수레의 yaw 각속도 상한 (rad/s) — 급회전을 따라 돌다 굴러 넘어지지 않게 (안전 클램프) */
	constexpr float MaxPushYawRate = 3.f;

	/** 잡은 직후 힘·속도를 로그로 남기는 틱 수 */
	constexpr int32 PushLogTickCount = 5;
}

ACL9Cart::ACL9Cart()
{
	// 적재 판정 상자: 충돌체가 아니라 소켓 Cargo 자리에 둔 크기다. 실제 판정은 이 자리에서 월드 겹침 질의로 한다 —
	// 겹침 이벤트 장부에 기대지 않아 잠든 아이템·막 복제된 아이템도 놓치지 않는다. 소켓에 직접 붙이지 않고(LoadData 참고)
	// 메시 원점에 붙여 두었다가 로드 때 소켓의 위치만 옮겨 받는다.
	CargoVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("CargoVolume"));
	CargoVolume->SetupAttachment(GetMeshComponent());
	CargoVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CargoVolume->SetGenerateOverlapEvents(false);
}

void ACL9Cart::LoadData()
{
	Super::LoadData();

	const UCL9ItemDef* Def = GetItemDef();
	if (Def == nullptr)
	{
		return;
	}
	if (!Def->bMeshIsBody)
	{
		UE_LOG(LogCL9, Error, TEXT("'%s': cart row '%s' has bMeshIsBody off — a cart is a rigid body pushed by its handle and stays unpushable until the row is fixed (docs/plan/data/item/cart.md)."),
			*GetNameSafe(this), *GetAlias().ToString());
		return;
	}

	// 소켓에 직접 붙이지 않는다 — FBX 소켓은 스케일(100)을 달고 들어오고, 이미 같은 부모·소켓에 붙은 컴포넌트에는
	// AttachToComponent의 스케일 규칙이 적용되지 않아(즉시 반환) 상자가 100배가 됐다(2026-09-13 PIE). 소켓의 **위치만** 읽어
	// 메시 원점 기준 상대 위치로 두고 스케일은 1로 못 박는다. 옛 BP 기본값에 소켓 이름이 남아 있어도 여기서 떼어 다시 붙는다.
	UStaticMeshComponent* Body = GetMeshComponent();
	CargoVolume->AttachToComponent(Body, FAttachmentTransformRules::KeepRelativeTransform);
	CargoVolume->SetRelativeScale3D(FVector::OneVector);
	CargoVolume->SetRelativeLocationAndRotation(
		Body->GetSocketTransform(UCL9ItemDef::CargoSocket, RTS_Component).GetLocation(), FRotator::ZeroRotator);
	CargoVolume->SetBoxExtent(Def->CargoExtent);

	// 밀리는 방향은 손잡이 소켓이 정한다 — 손잡이에서 수레 중심 쪽(+X 손잡이면 −X). 소켓이 없으면 기본값(−X)이 남는다.
	if (Body->DoesSocketExist(UCL9ItemDef::GripSocket))
	{
		const FVector GripLocal = Body->GetSocketTransform(UCL9ItemDef::GripSocket, RTS_Component).GetLocation();
		const FVector Toward = -FVector(GripLocal.X, GripLocal.Y, 0.f).GetSafeNormal();
		if (!Toward.IsNearlyZero())
		{
			PushForwardLocal = Toward;
		}
	}
}

FText ACL9Cart::GetPrompt(const ACL9Character* Who) const
{
	return Who != nullptr && Who->GetPullingCart() == this
		? LOCTABLE("CL9", "Prompt.CartRelease")
		: LOCTABLE("CL9", "Prompt.CartPull");
}

bool ACL9Cart::CanInteract(const ACL9Character* Who) const
{
	if (Who == nullptr || GetItemDef() == nullptr || !GetItemDef()->bMeshIsBody)
	{
		return false;
	}
	// 미는 사람은 언제든 놓을 수 있다. 그 외에는 수레가 자유 상태이고, 캐릭터가 아무것도 밀지 않으며, 활성 칸이
	// 비어 있어야(빈손) 한다 — 활성 칸에 뭔가 들었으면 프롬프트도 E도 없다. 싣기는 G·던지기다 (2026-09-13).
	if (Who->GetPullingCart() == this)
	{
		return true;
	}
	return IsFree() && Who->GetPullingCart() == nullptr && Who->GetHeldItem() == nullptr;
}

void ACL9Cart::Interact(ACL9Character* Who)
{
	if (!HasAuthority() || !CanInteract(Who))
	{
		return;
	}

	if (Who->GetPullingCart() == this)
	{
		StopPull(Who);
		return;
	}

	// 잡는 순간 수레가 튀지 않는다 — 다음 틱부터 스프링이 손잡이를 목표점으로 끈다.
	HoldState = ECL9ItemHoldState::Pushed;
	ApplyHoldState();
	Puller = Who;
	PushLogTicksLeft = PushLogTickCount;
	Who->SetPullingCart(this);
	GetBody()->WakeAllRigidBodies();
}

void ACL9Cart::StopPull(ACL9Character* Who)
{
	if (!HasAuthority() || Who == nullptr)
	{
		return;
	}

	// 그 자리에서 손을 놓는다 — 시뮬레이션은 계속 돌아 제 속도로 미끄러지다 서고, 다시 자유 상태라 던진 것에 밀릴 수도 있다.
	HoldState = ECL9ItemHoldState::Free;
	ApplyHoldState();
	Puller = nullptr;
	Who->SetPullingCart(nullptr);
}

void ACL9Cart::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 밀기, 호스트 전용: 손잡이를 미는 사람 앞 목표점으로 끄는 스프링·감쇠 힘 + yaw 스프링 토크 (docs/plan/data/item/cart.md 운반).
	const UCL9ItemDef* Def = GetItemDef();
	if (!HasAuthority() || Puller == nullptr || Def == nullptr)
	{
		return;
	}
	UPrimitiveComponent* Body = GetBody();
	if (!Body->IsSimulatingPhysics())
	{
		return;
	}

	// 목표점: 미는 사람 캡슐 표면에서 GripDistance 앞. 높이는 손잡이의 지금 높이 — 수직으로는 끌지 않아 바퀴가 바닥에 남는다.
	const FVector Grip = GetMeshComponent()->GetSocketLocation(UCL9ItemDef::GripSocket);
	const FVector PullerLocation = Puller->GetActorLocation();
	const FVector Forward = FRotator(0.f, Puller->GetActorRotation().Yaw, 0.f).Vector();
	const float Reach = Puller->GetCapsuleComponent()->GetScaledCapsuleRadius() + Def->GripDistance;
	FVector Error = PullerLocation + Forward * Reach - Grip;
	Error.Z = 0.f;

	// 손 놓침: 걸린 수레에서 옆이나 뒤로 벗어나거나, 수레가 턱 아래로 떨어지면 — "[E] 수레 밀기"가 다시 뜬다
	if (Error.Size() > Def->GripBreakDistance || FMath::Abs(Grip.Z - PullerLocation.Z) > Def->GripBreakDistance)
	{
		StopPull(Puller);
		return;
	}

	// 스프링 + 감쇠. 감쇠는 미는 사람과의 상대 속도다 — 같은 속도로 걸으면 처지지 않고, 걷기 시작·멈춤에서만 반 박자 늦는다.
	// 힘은 **무게중심**에 가속(bAccelChange)으로 준다 — 손잡이(지렛대 73cm)에 주면 감쇠가 회전에도 걸려 유효 각감쇠
	// (≈12r²c/(L²+W²) ≈ 16/s)가 dt=0.125에서 명시적 안정 한계(c·dt<2)를 넘어 발산했다(2026-09-13 PIE 8fps). 회전은 아래
	// yaw 스프링만 맡는다. 스프링·감쇠는 backward Euler로 푼다: v' = (v + dt·k·e) / (1 + dt·c + dt²·k) — 어떤 dt에서도
	// 안정하고, 작은 dt에서는 k·e − c·v와 같다(큰 dt에서 강성이 저절로 줄어든다). 엔진의 AddCustomPhysics는 Chaos에서
	// 델리게이트를 그 자리에서 한 번 실행할 뿐이라(서브스텝 없음) 쓰지 않는다.
	const float Dt = FMath::Max(DeltaSeconds, UE_KINDA_SMALL_NUMBER);
	const FVector CartVelocity = Body->GetPhysicsLinearVelocity();
	const FVector CartVelocityXY(CartVelocity.X, CartVelocity.Y, 0.f);
	FVector RelativeVelocity = CartVelocityXY - Puller->GetVelocity();
	RelativeVelocity.Z = 0.f;
	const float Denominator = 1.f + Dt * Def->PushDamping + Dt * Dt * Def->PushSpring;
	FVector Acceleration = (Error * Def->PushSpring - RelativeVelocity * (Def->PushDamping + Dt * Def->PushSpring)) / Denominator;

	// 안전 클램프: 한 틱 뒤 수평 속도는 미는 사람 최대 속도 × PushSpeedMargin까지, 가속은 놓치기 직전 오차에서의 스프링 가속
	// (PushSpring × GripBreakDistance — 행 값에서 나온다)까지.
	const float MaxSpeed = Puller->GetCharacterMovement()->GetMaxSpeed() * PushSpeedMargin;
	const FVector NextVelocity = (CartVelocityXY + Acceleration * Dt).GetClampedToMaxSize(MaxSpeed);
	Acceleration = ((NextVelocity - CartVelocityXY) / Dt).GetClampedToMaxSize(Def->PushSpring * Def->GripBreakDistance);
	Body->AddForce(Acceleration, NAME_None, true);

	// yaw 스프링: 밀리는 방향(손잡이→짐칸)이 미는 사람의 yaw를 향하게 — 제자리 회전이면 즉시 따라 돌지 않고 원호를 그린다.
	// 같은 backward Euler. 클램프: 한 틱 뒤 각속도는 ±MaxPushYawRate, 각가속은 180° 오차에서의 스프링 각가속(PushYawSpring × π)까지.
	const FVector PushDirection = GetActorQuat().RotateVector(PushForwardLocal).GetSafeNormal2D();
	const float YawError = FMath::Atan2(FVector::CrossProduct(PushDirection, Forward).Z, FVector::DotProduct(PushDirection, Forward));
	const float YawRate = Body->GetPhysicsAngularVelocityInRadians().Z;
	const float YawDenominator = 1.f + Dt * Def->PushYawDamping + Dt * Dt * Def->PushYawSpring;
	float YawAcceleration = (YawError * Def->PushYawSpring - YawRate * (Def->PushYawDamping + Dt * Def->PushYawSpring)) / YawDenominator;
	const float NextYawRate = FMath::Clamp(YawRate + YawAcceleration * Dt, -MaxPushYawRate, MaxPushYawRate);
	YawAcceleration = FMath::Clamp((NextYawRate - YawRate) / Dt, -Def->PushYawSpring * UE_PI, Def->PushYawSpring * UE_PI);
	Body->AddTorqueInRadians(FVector(0.f, 0.f, YawAcceleration), NAME_None, true);

	if (PushLogTicksLeft > 0)
	{
		--PushLogTicksLeft;
		UE_LOG(LogCL9, Log, TEXT("'%s' push tick: dt %.3f | grip error %s (%.0f cm) | rel vel %s | accel %s (%.0f cm/s²) | yaw err %.2f rad, rate %.2f, accel %.2f | cart vel %s (%.0f cm/s), ang vel %s | mass %.1f kg"),
			*GetNameSafe(this), Dt, *Error.ToString(), Error.Size(), *RelativeVelocity.ToString(), *Acceleration.ToString(), Acceleration.Size(),
			YawError, YawRate, YawAcceleration, *CartVelocity.ToString(), CartVelocity.Size(), *Body->GetPhysicsAngularVelocityInRadians().ToString(), Body->GetMass());
	}

	// 잠든 몸체에는 힘이 닿지 않는다 — 수레도, 짐칸 안 짐도 미는 동안 매 틱 깨운다(잠든 짐이 허공에 남지 않게)
	Body->WakeAllRigidBodies();
	TArray<ACL9Item*> Loaded;
	GatherLoadedItems(Loaded);
	for (ACL9Item* Item : Loaded)
	{
		Item->GetBody()->WakeAllRigidBodies();
	}
}

void ACL9Cart::GatherLoadedItems(TArray<ACL9Item*>& OutItems) const
{
	const UCL9ItemDef* Def = GetItemDef();
	if (Def == nullptr || !Def->bMeshIsBody)
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	const FCollisionObjectQueryParams ObjectParams(ECC_WorldDynamic);
	const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CL9Cargo), false, this);
	GetWorld()->OverlapMultiByObjectType(Overlaps, CargoVolume->GetComponentLocation(), CargoVolume->GetComponentQuat(),
		ObjectParams, FCollisionShape::MakeBox(CargoVolume->GetScaledBoxExtent()), QueryParams);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		ACL9Item* Item = Cast<ACL9Item>(Overlap.GetActor());
		// 자유 상태 아이템만 — 들린 것은 충돌이 꺼져 여기 잡히지 않고, 다른 수레는 짐이 아니다
		if (Item != nullptr && Item->IsFree() && Cast<ACL9Cart>(Item) == nullptr)
		{
			OutItems.AddUnique(Item);
		}
	}
}

bool ACL9Cart::HasLoadedItems() const
{
	TArray<ACL9Item*> Loaded;
	GatherLoadedItems(Loaded);
	return Loaded.Num() > 0;
}

void ACL9Cart::ReturnAllLoaded(ACL9GameState* GameState)
{
	if (!HasAuthority() || GameState == nullptr)
	{
		return;
	}

	TArray<ACL9Item*> Taken;
	TakeAllLoaded(Taken);
	for (ACL9Item* Item : Taken)
	{
		GameState->RecordReturn(Item->GetAlias(), Item->GetSubAlias());
		Item->Destroy();
	}
}

void ACL9Cart::TakeAllLoaded(TArray<ACL9Item*>& OutItems)
{
	if (!HasAuthority())
	{
		return;
	}

	GatherLoadedItems(OutItems);
}
