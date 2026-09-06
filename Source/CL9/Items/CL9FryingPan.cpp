// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9FryingPan.h"
#include "CL9.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/CL9ItemDef.h"
#include "Engine/CollisionProfile.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"

namespace
{
	/** 팬은 납작해진 원통이다 — 행의 단위 메시를 이 비율만큼 높이 방향으로 눌러 놓는다 */
	constexpr float PanFlatten = 0.25f;

	/** 상판 위 이 높이까지는 여전히 열을 받는 중으로 친다 (cm) */
	constexpr float HeatReach = 60.f;
}

ACL9FryingPan::ACL9FryingPan()
{
	Bed = CreateDefaultSubobject<UBoxComponent>(TEXT("Bed"));
	Bed->SetupAttachment(GetRootComponent());
	Bed->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	Bed->SetCanEverAffectNavigation(false);

	HeatZone = CreateDefaultSubobject<UBoxComponent>(TEXT("HeatZone"));
	HeatZone->SetupAttachment(GetRootComponent());
	HeatZone->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
	HeatZone->SetCanEverAffectNavigation(false);
}

void ACL9FryingPan::LoadData()
{
	Super::LoadData();

	const UCL9ItemDef* Def = GetItemDef();
	UStaticMeshComponent* Mesh = FindComponentByClass<UStaticMeshComponent>();
	if (Def == nullptr || Mesh == nullptr)
	{
		return;
	}

	// 베이스 클래스는 메시를 판정용 구체에 균일하게 맞추지만, 팬은 납작해야 한다
	const FVector Scale = Mesh->GetRelativeScale3D();
	Mesh->SetRelativeScale3D(FVector(Scale.X, Scale.Y, Scale.X * PanFlatten));
	Mesh->UpdateBounds();
	const float Top = Mesh->Bounds.BoxExtent.Z;

	// 공통 충돌체는 판정 반지름의 구(球)라 납작한 팬 위에 보이지 않는 둥근 껍질이 생기고,
	// 그 위에 얹힌 재료가 굴러 떨어진다 — 구를 팬 두께로 줄이고 상판(Bed)이 받치게 한다
	if (USphereComponent* Sphere = FindComponentByClass<USphereComponent>())
	{
		Sphere->SetSphereRadius(FMath::Max(Top, 6.f));
	}

	const float Half = Def->Radius * 0.7f;
	Bed->SetBoxExtent(FVector(Half, Half, 4.f));
	Bed->SetRelativeLocation(FVector(0.f, 0.f, Top - 4.f));
	HeatZone->SetBoxExtent(FVector(Half, Half, HeatReach * 0.5f));
	HeatZone->SetRelativeLocation(FVector(0.f, 0.f, Top + HeatReach * 0.5f));
}

void ACL9FryingPan::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 판정은 호스트가 내리며, 누군가의 손에 들린 팬은 아무것도 익히지 않는다
	if (HasAuthority() && IsFree())
	{
		TickCooking(DeltaSeconds);
	}
}

void ACL9FryingPan::TickCooking(float DeltaSeconds)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// 지금 이 구역에 놓여 있는 아이템이면 무엇이든 — 이벤트 기반이 아니라 쿼리 기반이라,
	// 오버랩 이벤트 없이 굴러 들어온 것도 여전히 포함된다
	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CL9Cooking), false, this);
	World->OverlapMultiByObjectType(Overlaps, HeatZone->GetComponentLocation(), HeatZone->GetComponentQuat(),
		ObjectParams, FCollisionShape::MakeBox(HeatZone->GetScaledBoxExtent()), QueryParams);

	TSet<ACL9Item*> OnHeat;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		ACL9Item* Item = Cast<ACL9Item>(Overlap.GetActor());
		if (Item != nullptr && Item != this && Item->IsFree())
		{
			OnHeat.Add(Item);
		}
	}

	// 열원을 벗어나면 시계가 초기화된다 — 손에 들고 있는 동안 반쯤 익는 일은 없다
	for (auto It = HeatSeconds.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid() || !OnHeat.Contains(It.Key().Get()))
		{
			SmokeWarned.Remove(It.Key());
			It.RemoveCurrent();
		}
	}

	for (ACL9Item* Item : OnHeat)
	{
		const UCL9ItemDef* Def = Item->GetItemDef();
		if (Def == nullptr)
		{
			continue;
		}
		// 생것 행은 익고, 조리된 행은 타고, 탄 행(및 도구)은 그냥 그대로 있는다
		const bool bCooks = Def->CookResult.IsSet();
		const bool bBurns = Def->BurnResult.IsSet();
		if (!bCooks && !bBurns)
		{
			continue;
		}
		const FCL9ItemRef& Next = bCooks ? Def->CookResult : Def->BurnResult;
		const float Needed = bCooks ? Def->CookTime : Def->BurnTime;

		float& Seconds = HeatSeconds.FindOrAdd(Item);
		Seconds += DeltaSeconds;

		// 타기 직전의 연기 전조 — 순전히 표현용이며 판정에는 영향을 주지 않는다 (cooking.md)
		if (!bCooks && !SmokeWarned.Contains(Item) && Seconds >= Needed - Def->BurnWarnTime)
		{
			SmokeWarned.Add(Item);
			UE_LOG(LogCL9, Log, TEXT("'%s' (%s/%s) starts smoking on '%s' — %.0fs to burnt."),
				*GetNameSafe(Item), *Item->GetAlias().ToString(), *Item->GetSubAlias().ToString(), *GetNameSafe(this), Def->BurnWarnTime);
		}

		if (Seconds >= Needed)
		{
			UE_LOG(LogCL9, Log, TEXT("'%s': %s/%s → %s/%s after %.1fs on '%s'."),
				*GetNameSafe(Item), *Item->GetAlias().ToString(), *Item->GetSubAlias().ToString(),
				*Next.ItemAlias.ToString(), *Next.ItemSubAlias.ToString(), Seconds, *GetNameSafe(this));
			Item->SetAlias(Next.ItemAlias, Next.ItemSubAlias);
			Seconds = 0.f;
			SmokeWarned.Remove(Item);
		}
	}
}
