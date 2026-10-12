// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/CL9YardTension.h"
#include "Animation/AnimationAsset.h"
#include "Components/PointLightComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInterface.h"
#include "CL9.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ModuleDef.h"
#include "Data/CL9RulesDef.h"
#include "Data/CL9SoundTable.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameState.h"
#include "World/CL9SiteLayout.h"
#include "World/CL9SiteMap.h"

namespace
{
	const UCL9RulesDef* YardRules(const AActor& Actor)
	{
		const UCL9GameInstance* GameInstance = Actor.GetGameInstance<UCL9GameInstance>();
		const UCL9DefRegistry* Registry = GameInstance != nullptr ? GameInstance->GetDefRegistry() : nullptr;
		return Registry != nullptr ? Registry->GetRules() : nullptr;
	}

	/** 한 칸이 실외인가 — 격자 밖(패드 쪽 띠)도 마당으로 본다 */
	bool IsOutdoorCell(const FCL9SiteLayout& Layout, FIntPoint Cell)
	{
		const int32 Module = Layout.CellAt(0, Cell);
		if (Module == INDEX_NONE || !Layout.Modules.IsValidIndex(Module))
		{
			return true;
		}
		const UCL9ModuleDef* Def = Layout.Modules[Module].Def;
		return Def == nullptr || Def->bOutdoor;
	}

	/** 문이 걸린 칸이 정말 모듈인가(빈 격자가 아닌가) */
	bool IsModuleCell(const FCL9SiteLayout& Layout, FIntPoint Cell)
	{
		return Layout.CellAt(0, Cell) != INDEX_NONE;
	}
}

ACL9YardTension::ACL9YardTension()
{
	// 틱 이유(docs/systems/performance.md 틱): 등의 깜박임·울음 시각·실루엣 걸음이 매 프레임 바뀐다. 마당 밖·로비면 몇 번의 비교로 나간다
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}

void ACL9YardTension::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const UWorld* World = GetWorld();
	const ACL9GameState* GameState = World != nullptr ? World->GetGameState<ACL9GameState>() : nullptr;
	if (GameState == nullptr)
	{
		return;
	}

	// 판마다 새 미로 — 시드가 바뀌고 부지가 다 서면 문을 다시 찾는다
	const uint32 Seed = GameState->GetMapSeed();
	if (Seed != 0 && Seed != BuiltForSeed && !ACL9SiteMap::IsBuildPending(GetWorld()))
	{
		Rebuild(Seed);
	}

	const bool bNight = GameState->GetPhase() == ECL9NightPhase::Night;
	const float Now = World->GetTimeSeconds();
	TickLamps(Now, bNight);

	// 로컬 플레이어의 자리와 시선 — 없거나 판이 아니면 실루엣만 마저 걷는다
	APlayerController* Controller = World->GetFirstPlayerController();
	const APawn* Pawn = Controller != nullptr ? Controller->GetPawn() : nullptr;
	// 트럭 도착(7j) 동안은 카메라가 화면을 잡고 있다 — 마당의 연출은 내린 뒤부터
	const float Arrival = GameState->GetArrivalAlpha();
	const bool bArriving = Arrival >= 0.f && Arrival < 1.f;
	const bool bBeforePower = bNight && !bArriving && GameState->GetPowerStage() == ECL9PowerStage::Dark;
	bool bYard = false;
	FVector ViewLocation = FVector::ZeroVector;
	FVector ViewDirection = FVector::ForwardVector;
	if (Pawn != nullptr && bBeforePower)
	{
		bYard = IsInYard(Pawn->GetActorLocation());
		FRotator ViewRotation;
		Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
		ViewDirection = ViewRotation.Vector();
		if (bYard)
		{
			TickCries(Now, Pawn->GetActorLocation());
		}
	}
	TickSilhouette(DeltaSeconds, ViewLocation, ViewDirection, bYard);
}

void ACL9YardTension::Rebuild(uint32 Seed)
{
	BuiltForSeed = Seed;
	Entrances.Reset();
	SilhouetteShown.Reset();
	NextCryTime = 0.f;
	EndSilhouette();
	ClearLamps();

	TArray<ACL9SiteMap*> Maps;
	ACL9SiteMap::FindAll(GetWorld(), Maps);
	for (const ACL9SiteMap* Map : Maps)
	{
		const FCL9SiteLayout& Layout = Map->GetLayout();
		const float Cell = Map->GetParams().CellSize;
		if (!Layout.bBuilt || Layout.FloorCount <= 0 || Cell <= 0.f)
		{
			continue;
		}
		const FTransform Frame = Map->GetActorTransform();
		auto AddEntrance = [&](FIntPoint A, FIntPoint B, const FVector2D& LocalDoor, const FVector2D& AToB)
		{
			// 한쪽은 실외(또는 격자 밖), 다른 쪽은 실내 모듈이어야 마당 → 건물 문이다
			const bool bAOut = IsOutdoorCell(Layout, A);
			const bool bBOut = IsOutdoorCell(Layout, B);
			if (bAOut == bBOut || (!bAOut && !IsModuleCell(Layout, A)) || (!bBOut && !IsModuleCell(Layout, B)))
			{
				return;
			}
			const FVector2D In2D = bAOut ? AToB : -AToB;
			FEntrance& Entry = Entrances.AddDefaulted_GetRef();
			Entry.Door = Frame.TransformPosition(FVector(LocalDoor.X, LocalDoor.Y, 0.f));
			Entry.Inward = Frame.TransformVectorNoScale(FVector(In2D.X, In2D.Y, 0.f)).GetSafeNormal();
			Entry.Lateral = FVector::CrossProduct(FVector::UpVector, Entry.Inward).GetSafeNormal();
		};
		for (int32 GY = 0; GY < Layout.GridSize.Y; ++GY)
		{
			for (int32 GX = 0; GX <= Layout.GridSize.X; ++GX)
			{
				if (Layout.EdgeAtX(0, GX, GY) == ECL9GridEdge::Door)
				{
					AddEntrance(FIntPoint(GX - 1, GY), FIntPoint(GX, GY),
						FVector2D(Layout.Origin.X + GX * Cell, Layout.Origin.Y + (GY + 0.5f) * Cell), FVector2D(1.f, 0.f));
				}
			}
		}
		for (int32 GY = 0; GY <= Layout.GridSize.Y; ++GY)
		{
			for (int32 GX = 0; GX < Layout.GridSize.X; ++GX)
			{
				if (Layout.EdgeAtY(0, GX, GY) == ECL9GridEdge::Door)
				{
					AddEntrance(FIntPoint(GX, GY - 1), FIntPoint(GX, GY),
						FVector2D(Layout.Origin.X + (GX + 0.5f) * Cell, Layout.Origin.Y + GY * Cell), FVector2D(0.f, 1.f));
				}
			}
		}
	}

	// 문마다 바깥 위에 작업등 하나 — 메시가 없으면 빛만 건다
	const UCL9RulesDef* Rules = YardRules(*this);
	UStaticMesh* LampMesh = (Rules != nullptr && !Rules->YardLampMesh.IsNull()) ? Rules->YardLampMesh.LoadSynchronous() : nullptr;
	if (Rules != nullptr && Rules->YardLampIntensity > 0.f)
	{
		for (int32 Index = 0; Index < Entrances.Num(); ++Index)
		{
			const FEntrance& Entry = Entrances[Index];
			const FVector Spot = Entry.Door - Entry.Inward * 25.f + FVector::UpVector * Rules->YardLampHeight;
			const FRotator Facing = (-Entry.Inward).Rotation();
			if (LampMesh != nullptr)
			{
				UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(this, FName(*FString::Printf(TEXT("YardLampMesh_%d"), Index)));
				Mesh->SetMobility(EComponentMobility::Movable);
				Mesh->SetStaticMesh(LampMesh);
				Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				Mesh->SetupAttachment(GetRootComponent());
				Mesh->RegisterComponent();
				Mesh->SetWorldLocationAndRotation(Spot, Facing);
				LampMeshes.Add(Mesh);
			}
			UPointLightComponent* Lamp = NewObject<UPointLightComponent>(this, FName(*FString::Printf(TEXT("YardLamp_%d"), Index)));
			Lamp->SetMobility(EComponentMobility::Movable);
			Lamp->SetIntensityUnits(ELightUnits::Candelas);
			Lamp->SetIntensity(Rules->YardLampIntensity);
			Lamp->SetAttenuationRadius(FMath::Max(Rules->YardLampRadius, 50.f));
			Lamp->SetLightColor(FLinearColor(1.f, 0.8f, 0.55f));
			Lamp->SetCastShadows(false);
			Lamp->SetupAttachment(GetRootComponent());
			Lamp->RegisterComponent();
			Lamp->SetWorldLocation(Spot - FVector::UpVector * 15.f);
			Lamps.Add(Lamp);
		}
	}
	UE_LOG(LogCL9, Log, TEXT("Yard: %d door(s) between the yard and the building, %d lamp(s) hung (seed %u)."), Entrances.Num(), Lamps.Num(), Seed);
}

void ACL9YardTension::ClearLamps()
{
	for (UStaticMeshComponent* Mesh : LampMeshes)
	{
		if (Mesh != nullptr)
		{
			Mesh->DestroyComponent();
		}
	}
	for (UPointLightComponent* Lamp : Lamps)
	{
		if (Lamp != nullptr)
		{
			Lamp->DestroyComponent();
		}
	}
	LampMeshes.Reset();
	Lamps.Reset();
}

bool ACL9YardTension::IsInYard(const FVector& Location) const
{
	TArray<ACL9SiteMap*> Maps;
	ACL9SiteMap::FindAll(GetWorld(), Maps);
	for (const ACL9SiteMap* Map : Maps)
	{
		int32 Floor = INDEX_NONE;
		FIntPoint Cell;
		if (Map->WorldToCell(Location, Floor, Cell))
		{
			return Floor == 0 && IsOutdoorCell(Map->GetLayout(), Cell);
		}
	}
	// 어느 격자에도 없다 — 트럭 패드 쪽 띠다
	return true;
}

void ACL9YardTension::TickLamps(float Now, bool bNight)
{
	// 판 동안만 켠다. 깜박임은 끊김이 있는 노이즈 — 가끔 거의 꺼졌다 돌아온다
	const UCL9RulesDef* Rules = YardRules(*this);
	for (int32 Index = 0; Index < Lamps.Num(); ++Index)
	{
		UPointLightComponent* Lamp = Lamps[Index];
		if (Lamp == nullptr || Rules == nullptr)
		{
			continue;
		}
		Lamp->SetVisibility(bNight);
		if (bNight)
		{
			const float Noise = FMath::PerlinNoise1D(Now * 3.1f + Index * 17.3f);
			const float Scale = Noise > -0.25f ? 0.8f + 0.2f * Noise : 0.06f;
			Lamp->SetIntensity(Rules->YardLampIntensity * Scale);
		}
	}
}

void ACL9YardTension::TickCries(float Now, const FVector& PawnLocation)
{
	const UCL9RulesDef* Rules = YardRules(*this);
	if (Rules == nullptr || Entrances.IsEmpty() || Rules->YardCryIntervalMax <= 0.f)
	{
		return;
	}
	if (NextCryTime <= 0.f)
	{
		// 마당에 처음 선 순간 — 첫 울음은 간격의 절반 안에
		NextCryTime = Now + FMath::FRandRange(Rules->YardCryIntervalMin, Rules->YardCryIntervalMax) * 0.5f;
		return;
	}
	if (Now < NextCryTime)
	{
		return;
	}
	NextCryTime = Now + FMath::FRandRange(Rules->YardCryIntervalMin, Rules->YardCryIntervalMax);

	// 가까운 문의 건물 안쪽 깊숙이 — 어디서 나는지 대략만 알게
	int32 Nearest = 0;
	for (int32 Index = 1; Index < Entrances.Num(); ++Index)
	{
		if (FVector::DistSquared(PawnLocation, Entrances[Index].Door) < FVector::DistSquared(PawnLocation, Entrances[Nearest].Door))
		{
			Nearest = Index;
		}
	}
	const FEntrance& Entry = Entrances[Nearest];
	const FVector Spot = Entry.Door + Entry.Inward * FMath::FRandRange(700.f, 1300.f) + Entry.Lateral * FMath::FRandRange(-500.f, 500.f) + FVector::UpVector * 80.f;
	CL9Sound::PlayAt(this, CL9SoundNames::YardCry, Spot, 0.12f);
	UE_LOG(LogCL9, Log, TEXT("Yard: cry from inside the building at %s (door %d)."), *Spot.ToCompactString(), Nearest);
}

void ACL9YardTension::TickSilhouette(float DeltaSeconds, const FVector& ViewLocation, const FVector& ViewDirection, bool bYard)
{
	const UCL9RulesDef* Rules = YardRules(*this);
	if (Rules == nullptr)
	{
		return;
	}

	// 걷는 중이면 마저 걷는다 — 다 건너면 걷는다
	USkeletalMeshComponent* Body = FindComponentByClass<USkeletalMeshComponent>();
	if (Body != nullptr && Body->IsVisible())
	{
		SilhouetteTravelled += Rules->SilhouetteSpeed * DeltaSeconds;
		Body->SetWorldLocation(SilhouetteStart + SilhouetteDirection * SilhouetteTravelled);
		if (SilhouetteTravelled >= 700.f)
		{
			EndSilhouette();
		}
		return;
	}

	if (!bYard || Entrances.IsEmpty() || Rules->SilhouetteRange <= 0.f || Rules->SilhouetteMesh.IsNull())
	{
		return;
	}

	for (int32 Index = 0; Index < Entrances.Num(); ++Index)
	{
		if (SilhouetteShown.Contains(Index))
		{
			continue;
		}
		const FEntrance& Entry = Entrances[Index];
		const FVector ToDoor = Entry.Door + FVector::UpVector * 120.f - ViewLocation;
		const float Distance = ToDoor.Size();
		// 문을 마주 보고 멀찍이 — 바로 앞이면 너무 가깝다
		if (Distance > Rules->SilhouetteRange || Distance < 600.f || FVector::DotProduct(ViewDirection, ToDoor / Distance) < 0.9f
			|| FVector::DotProduct(Entry.Inward, ToDoor / Distance) < 0.3f)
		{
			continue;
		}

		USkeletalMesh* Mesh = Rules->SilhouetteMesh.LoadSynchronous();
		if (Mesh == nullptr)
		{
			return;
		}
		SilhouetteShown.Add(Index);
		if (Body == nullptr)
		{
			Body = NewObject<USkeletalMeshComponent>(this, TEXT("Silhouette"));
			Body->SetMobility(EComponentMobility::Movable);
			Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Body->SetCastShadow(false);
			Body->SetupAttachment(GetRootComponent());
			Body->RegisterComponent();
		}
		Body->SetSkeletalMesh(Mesh);
		if (UMaterialInterface* Dark = Rules->SilhouetteMaterial.IsNull() ? nullptr : Rules->SilhouetteMaterial.LoadSynchronous())
		{
			for (int32 Slot = 0; Slot < Body->GetNumMaterials(); ++Slot)
			{
				Body->SetMaterial(Slot, Dark);
			}
		}
		if (UAnimationAsset* Walk = Rules->SilhouetteAnim.IsNull() ? nullptr : Rules->SilhouetteAnim.LoadSynchronous())
		{
			Body->PlayAnimation(Walk, true);
		}
		// 문 너머 안쪽에서 문 폭 방향으로 건넌다 — 시선과 같은 쪽에서 시작해 반대쪽으로
		SilhouetteDirection = FVector::DotProduct(Entry.Lateral, ViewDirection) >= 0.f ? -Entry.Lateral : Entry.Lateral;
		SilhouetteStart = Entry.Door + Entry.Inward * 260.f - SilhouetteDirection * 350.f;
		SilhouetteTravelled = 0.f;
		Body->SetWorldScale3D(FVector(Rules->SilhouetteScale));
		Body->SetWorldLocationAndRotation(SilhouetteStart, SilhouetteDirection.Rotation());
		Body->SetVisibility(true);

		// 뒤에서 비추는 빛 — 어두운 건물 안에서 검은 몸이 윤곽으로 보이게
		if (BackLight == nullptr)
		{
			BackLight = NewObject<UPointLightComponent>(this, TEXT("SilhouetteBackLight"));
			BackLight->SetMobility(EComponentMobility::Movable);
			BackLight->SetIntensityUnits(ELightUnits::Candelas);
			BackLight->SetCastShadows(false);
			BackLight->SetLightColor(FLinearColor(0.75f, 0.85f, 1.f));
			BackLight->SetAttenuationRadius(900.f);
			BackLight->SetupAttachment(GetRootComponent());
			BackLight->RegisterComponent();
		}
		BackLight->SetIntensity(Rules->SilhouetteBackLight);
		BackLight->SetWorldLocation(Entry.Door + Entry.Inward * 650.f + FVector::UpVector * 150.f);
		BackLight->SetVisibility(true);
		UE_LOG(LogCL9, Log, TEXT("Yard: silhouette crosses door %d (%.0f cm away)."), Index, Distance);
		return;
	}
}

void ACL9YardTension::EndSilhouette()
{
	if (USkeletalMeshComponent* Body = FindComponentByClass<USkeletalMeshComponent>())
	{
		Body->SetVisibility(false);
		Body->Stop();
	}
	if (BackLight != nullptr)
	{
		BackLight->SetVisibility(false);
	}
	SilhouetteTravelled = 0.f;
}
