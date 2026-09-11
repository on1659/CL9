// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9Kitchen.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "CL9.h"
#include "Data/CL9ItemDef.h"
#include "Items/CL9Item.h"

ACL9Kitchen::ACL9Kitchen()
{
	PrimaryActorTick.bCanEverTick = true;


	HeatZone = CreateDefaultSubobject<UBoxComponent>(TEXT("HeatZone"));
	HeatZone->SetupAttachment(MeshComponent);
	// 몸체 스케일에서 분리한다 — 상판을 넓히려고 액터를 늘려도 열 높이까지 같이 늘어나면 안 된다
	HeatZone->SetUsingAbsoluteScale(true);
	HeatZone->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	HeatZone->SetCanEverAffectNavigation(false);
}

void ACL9Kitchen::BeginPlay()
{
	Super::BeginPlay();

	// 열 구역은 몸체의 실제 크기에서 나온다 — BP 자식이 메시나 스케일을 바꿔도 따라간다
	const FVector Extent = MeshComponent->Bounds.BoxExtent;
	const float Top = Extent.Z;
	HeatZone->SetBoxExtent(FVector(Extent.X, Extent.Y, HeatReach * 0.5f));
	HeatZone->SetWorldLocation(MeshComponent->Bounds.Origin + FVector(0.f, 0.f, Top + HeatReach * 0.5f));
}

void ACL9Kitchen::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACL9Kitchen, Cooking);
}

ACL9Item* ACL9Kitchen::GetCookingItem() const
{
	// 익히던 것이 파괴됐을 수 있다(창구 납품 등) — 죽어 가는 액터를 돌려주지 않는다
	return IsValid(Cooking) ? Cooking.Get() : nullptr;
}

FVector ACL9Kitchen::GetTopLocation() const
{
	const FBoxSphereBounds& Bounds = MeshComponent->Bounds;
	return Bounds.Origin + FVector(0.f, 0.f, Bounds.BoxExtent.Z);
}

void ACL9Kitchen::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		TickCooking(DeltaSeconds);
	}
}

void ACL9Kitchen::GatherOnHeat(TArray<ACL9Item*>& OutItems) const
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// 이벤트가 아니라 쿼리다 — 오버랩 알림 없이 굴러 들어온 것도 똑같이 잡힌다
	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CL9Cooking), false, this);
	World->OverlapMultiByObjectType(Overlaps, HeatZone->GetComponentLocation(), HeatZone->GetComponentQuat(),
		ObjectParams, FCollisionShape::MakeBox(HeatZone->GetScaledBoxExtent()), QueryParams);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		ACL9Item* Item = Cast<ACL9Item>(Overlap.GetActor());
		if (Item == nullptr || !Item->IsFree() || OutItems.Contains(Item))
		{
			continue;
		}
		// 생것 행은 익고, 익은 행은 타고, 탄 행과 도구는 아무 데도 가지 않는다
		const UCL9ItemDef* Def = Item->GetItemDef();
		if (Def != nullptr && (Def->CookResult.IsSet() || Def->BurnResult.IsSet()))
		{
			OutItems.Add(Item);
		}
	}
}

void ACL9Kitchen::TickCooking(float DeltaSeconds)
{
	TArray<ACL9Item*> OnHeat;
	GatherOnHeat(OnHeat);

	// 익히던 것이 집혀 갔거나 굴러 나갔으면 시계를 버리고 자리를 비운다
	ACL9Item* Current = GetCookingItem();
	if (Current != nullptr && !OnHeat.Contains(Current))
	{
		Current->SetCookState(ECL9CookPhase::None, 0.f);
		Current = nullptr;
		Cooking = nullptr;
		HeatSeconds = 0.f;
		bSmokeWarned = false;
	}

	// 조리대 하나가 한 번에 하나를 익힌다 — 자리가 비어 있을 때만 다음 재료를 집어 든다
	if (Current == nullptr)
	{
		Cooking = nullptr;   // 파괴된 것을 붙들고 있었을 수 있다
		if (OnHeat.Num() == 0)
		{
			return;
		}
		Current = OnHeat[0];
		Cooking = Current;
		HeatSeconds = 0.f;
		bSmokeWarned = false;
	}

	// 차례를 기다리는 것들은 열을 받지 않는다 — 게이지도 뜨지 않아야 한다
	for (ACL9Item* Waiting : OnHeat)
	{
		if (Waiting != Current)
		{
			Waiting->SetCookState(ECL9CookPhase::None, 0.f);
		}
	}

	const UCL9ItemDef* Def = Current->GetItemDef();
	if (Def == nullptr)
	{
		return;
	}

	const bool bCooks = Def->CookResult.IsSet();
	const FCL9ItemRef& Next = bCooks ? Def->CookResult : Def->BurnResult;
	const float Needed = bCooks ? Def->CookTime : Def->BurnTime;

	HeatSeconds += DeltaSeconds;
	Current->SetCookState(bCooks ? ECL9CookPhase::Cooking : ECL9CookPhase::Burning,
		Needed > 0.f ? HeatSeconds / Needed : 1.f);

	// 타기 직전의 연기 전조 — 순전히 표현용이며 판정에는 영향을 주지 않는다 (cooking.md)
	if (!bCooks && !bSmokeWarned && HeatSeconds >= Needed - Def->BurnWarnTime)
	{
		bSmokeWarned = true;
		UE_LOG(LogCL9, Log, TEXT("'%s' (%s/%s) starts smoking on '%s' — %.0fs to burnt."),
			*GetNameSafe(Current), *Current->GetAlias().ToString(), *Current->GetSubAlias().ToString(),
			*GetNameSafe(this), Def->BurnWarnTime);
	}

	if (HeatSeconds >= Needed)
	{
		UE_LOG(LogCL9, Log, TEXT("'%s': %s/%s → %s/%s after %.1fs on '%s'."),
			*GetNameSafe(Current), *Current->GetAlias().ToString(), *Current->GetSubAlias().ToString(),
			*Next.ItemAlias.ToString(), *Next.ItemSubAlias.ToString(), HeatSeconds, *GetNameSafe(this));
		Current->SetAlias(Next.ItemAlias, Next.ItemSubAlias);
		HeatSeconds = 0.f;
		bSmokeWarned = false;

		// 탄 것은 더 갈 곳이 없다 — 자리를 비워 다음 재료가 올라오게 한다
		const UCL9ItemDef* After = Current->GetItemDef();
		if (After == nullptr || (!After->CookResult.IsSet() && !After->BurnResult.IsSet()))
		{
			Current->SetCookState(ECL9CookPhase::None, 0.f);
			Cooking = nullptr;
		}
	}
}
