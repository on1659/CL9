// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9GameMode.h"
#include "CL9.h"
#include "Character/CL9Character.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9EquipmentDef.h"
#include "Data/CL9ItemDef.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameState.h"
#include "GameMode/CL9PlayerController.h"
#include "GameMode/CL9PlayerState.h"
#include "GameMode/CL9SaveGame.h"
#include "Items/CL9Counter.h"
#include "Items/CL9Item.h"
#include "Items/CL9Jar.h"
#include "Components/CapsuleComponent.h"
#include "Data/CL9MonsterDef.h"
#include "Data/CL9OrderDef.h"
#include "Data/CL9ProgressionDef.h"
#include "Data/CL9RulesDef.h"
#include "Data/CL9SiteDef.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Monsters/CL9Monster.h"
#include "TimerManager.h"
#include "World/CL9SiteMap.h"
#include "World/CL9TruckPad.h"

namespace
{
	/** 오늘 밤의 맵 시드 — 고정된 값(cl9.seed)이거나 새로 굴린 값이며, 0이 되는 일은 없다. 0은 "껍데기만 있음"을 뜻하기 때문이다 */
	uint32 RollMapSeed(uint32 Forced)
	{
		return Forced != 0 ? Forced : static_cast<uint32>(FMath::RandRange(1, MAX_int32));
	}
}

ACL9GameMode::ACL9GameMode()
{
	GameStateClass = ACL9GameState::StaticClass();
	PlayerStateClass = ACL9PlayerState::StaticClass();
}

void ACL9GameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);

	if (ACL9Character* Character = Cast<ACL9Character>(PlayerPawn))
	{
		Character->SetCharacterAlias(DefaultCharacterAlias);
	}
}

bool ACL9GameMode::AllowCheats(APlayerController* P)
{
#if UE_BUILD_SHIPPING
	return Super::AllowCheats(P);
#else
	// 패키징된 개발 빌드는 기본적으로 치트를 끈다 — cl9.* 명령어는 단계 검증용
	// 도구이므로, shipping이 아닌 한 계속 켜둔다(docs/systems/cheats.md).
	return true;
#endif
}

ACL9Item* ACL9GameMode::SpawnItem(FName ItemAlias, FName ItemSubAlias, const FTransform& Transform)
{
	if (!HasAuthority())
	{
		return nullptr;
	}

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9ItemDef* Def = Registry ? Registry->FindItem(ItemAlias, ItemSubAlias) : nullptr;
	if (Def == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("SpawnItem: item row '%s'/'%s' is missing or broken — nothing spawned."),
			*ItemAlias.ToString(), *ItemSubAlias.ToString());
		return nullptr;
	}

	UClass* SpawnClass = ItemClass;
	if (!Def->ActorClass.IsNull())
	{
		UClass* DedicatedClass = Def->ActorClass.LoadSynchronous();
		if (DedicatedClass != nullptr && DedicatedClass->IsChildOf(ACL9Item::StaticClass()))
		{
			SpawnClass = DedicatedClass;
		}
		else
		{
			UE_LOG(LogCL9, Error, TEXT("SpawnItem: '%s' ActorClass does not derive from ACL9Item — using the shared BP."), *ItemAlias.ToString());
		}
	}
	if (SpawnClass == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("SpawnItem: no item class — assign ItemClass in the game mode BP."));
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ACL9Item* Item = GetWorld()->SpawnActor<ACL9Item>(SpawnClass, Transform, SpawnParameters);
	if (Item != nullptr)
	{
		Item->SetAlias(ItemAlias, ItemSubAlias);
	}
	return Item;
}

ACL9Monster* ACL9GameMode::SpawnMonster(FName MonsterAlias, FName MonsterSubAlias, const FTransform& Transform)
{
	if (!HasAuthority())
	{
		return nullptr;
	}

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9MonsterDef* Def = Registry ? Registry->FindMonster(MonsterAlias, MonsterSubAlias) : nullptr;
	if (Def == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("SpawnMonster: monster row '%s'/'%s' is missing or broken — nothing spawned."),
			*MonsterAlias.ToString(), *MonsterSubAlias.ToString());
		return nullptr;
	}

	UClass* SpawnClass = MonsterClass;
	if (!Def->ActorClass.IsNull())
	{
		UClass* DedicatedClass = Def->ActorClass.LoadSynchronous();
		if (DedicatedClass != nullptr && DedicatedClass->IsChildOf(ACL9Monster::StaticClass()))
		{
			SpawnClass = DedicatedClass;
		}
		else
		{
			UE_LOG(LogCL9, Error, TEXT("SpawnMonster: '%s' ActorClass does not derive from ACL9Monster — using the shared BP."), *MonsterAlias.ToString());
		}
	}
	if (SpawnClass == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("SpawnMonster: no monster class — assign MonsterClass in the game mode BP."));
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ACL9Monster* Monster = GetWorld()->SpawnActor<ACL9Monster>(SpawnClass, Transform, SpawnParameters);
	if (Monster != nullptr)
	{
		Monster->SetAlias(MonsterAlias, MonsterSubAlias);
	}
	return Monster;
}

void ACL9GameMode::StartNight()
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	if (CL9GameState == nullptr || CL9GameState->GetPhase() != ECL9NightPhase::Lobby)
	{
		UE_LOG(LogCL9, Warning, TEXT("StartNight: only the lobby can start a night."));
		return;
	}

	// 새벽이 끝났다 — 지난 정산 결과는 이 밤부터 유효하지 않다(새벽 화면이 그것으로 판단한다)
	CL9GameState->ClearLastSettlement();

	const UCL9RulesDef* Rules = GetRules();
	if (Rules == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("StartNight: no usable rules row — night not started."));
		return;
	}

	// 날짜 곡선이 규칙의 밤 길이와 사이트의 개체 수를 조정한다(docs/plan/data/progression.md);
	// progression 애셋이 없으면 보정이 없다는 뜻이고, site가 없으면 그날 밤은 텅 빈다.
	const int32 Day = CL9GameState->GetCurrentDay();
	const UCL9ProgressionDef* Progression = GetProgression();
	const float NightLength = Rules->NightLength * (Progression ? Progression->NightLengthAt(Day) : 1.f);
	const bool bNoBlackout = Progression != nullptr && Progression->IsBlackoutDisabled(Day);

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9SiteDef* Site = Registry ? Registry->FindSite(DefaultSiteAlias) : nullptr;
	if (Site == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("StartNight: site row '%s' is missing or broken — no animals or ingredients tonight (assign DefaultSiteAlias in the game mode BP)."),
			*DefaultSiteAlias.ToString());
	}
	const float Danger = (Site ? Site->BaseDanger : 1.f) * (Progression ? Progression->DangerAt(Day) : 1.f);
	CL9GameState->SetSiteAndDanger(Site ? Site->Alias : NAME_None, Danger);

	// 로비가 이미 오늘 밤의 미로를 세워뒀다(2026-09-06 결정 — 로비부터 오늘 밤 미로); 어쩌다
	// 미로가 없는 월드만 지금 하나 받는다. 공유되는 것은 시드뿐이다 — 모든 머신이
	// 그 시드로부터 같은 미로를 깐다(docs/systems/map.md).
	if (CL9GameState->GetMapSeed() == 0)
	{
		CL9GameState->SetSiteMap(RollMapSeed(ForcedMapSeed));
	}

	CL9GameState->StartNightClock(*Rules, NightLength, bNoBlackout);
	CL9GameState->SetPhase(ECL9NightPhase::Night);
	GetWorldTimerManager().SetTimer(NightExpiryHandle, this, &ACL9GameMode::EndNight, NightLength);

	UE_LOG(LogCL9, Log, TEXT("Night %d started — %.0fs to dawn, blackout %s, danger ×%.2f."),
		Day, NightLength, bNoBlackout ? TEXT("off (day override)") : *FString::Printf(TEXT("base %.0fs"), Rules->DarkBase), Danger);

	if (Site != nullptr)
	{
		SpawnNightPopulation(*Site, Progression, Day);
	}
}

void ACL9GameMode::EndNight()
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	if (CL9GameState == nullptr || CL9GameState->GetPhase() != ECL9NightPhase::Night)
	{
		UE_LOG(LogCL9, Warning, TEXT("EndNight: no night is running."));
		return;
	}

	GetWorldTimerManager().ClearTimer(NightExpiryHandle);
	CL9GameState->SetPhase(ECL9NightPhase::Dawn);
	UE_LOG(LogCL9, Log, TEXT("Night %d over — dawn."), CL9GameState->GetCurrentDay());

	// DawnLength는 아직 정해지지 않았다(0) — 값이 정해지기 전까지 새벽은 그냥 곧바로 지나간다.
	const UCL9RulesDef* Rules = GetRules();
	const float DawnLength = Rules ? Rules->DawnLength : 0.f;
	if (DawnLength > 0.f)
	{
		GetWorldTimerManager().SetTimer(DawnHandle, this, &ACL9GameMode::EnterLobby, DawnLength);
	}
	else
	{
		EnterLobby();
	}
}

void ACL9GameMode::EnterLobby()
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	if (CL9GameState == nullptr)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(DawnHandle);
	CL9GameState->ClearNightClock();
	CL9GameState->AdvanceDay();
	// 날이 바뀌었으니 어제까지였던 주문은 여기서 끝난다 — 위약금이 먼저, 새 offer가 나중
	ExpireOrders();
	// 두고 온 장비는 부지와 함께 사라진다 — 맵을 새로 깔기 전에 판정한다
	LoseEquipmentLeftBehind();
	ClearNightPopulation();
	CL9GameState->SetSiteAndDanger(CL9GameState->GetCurrentSiteAlias(), 1.f);
	// 새로운 밤, 새로운 미로 — 로비에서부터 세워둬서 정차 지점이 오늘 밤 갈 곳을 보여준다
	CL9GameState->SetSiteMap(RollMapSeed(ForcedMapSeed));
	ReviveDead();
	CL9GameState->SetPhase(ECL9NightPhase::Lobby);
	UE_LOG(LogCL9, Log, TEXT("Lobby — day %d."), CL9GameState->GetCurrentDay());
	// 새벽 화면을 **정산보다 먼저** 연다 — 화면이 먼저 서 있어야 OnSettled를 놓치지 않는다
	// (놓쳐도 GameState.GetLastSettlement()로 읽을 수 있게 해 뒀지만, 순서가 맞으면 그럴 일이 없다)
	OpenDawnScreen();
	// 새벽 처리: 정산이 먼저다 — 판이 끝나면 새 판 상태를 저장해야 하기 때문이다
	Settle();
	RecordBestDay(CL9GameState->GetCurrentDay());
	// 새벽마다 한 번 — 밤을 통째로 치른 뒤에 저장한다(docs/systems/save.md 저장 시점)
	SaveRun();
	OfferOrders();
}

void ACL9GameMode::HandlePlayerDeath(ACL9Character* Dead)
{
	if (Dead == nullptr)
	{
		return;
	}

	// 전멸 — 아무도 서 있지 않다, 트럭이 파티를 두고 떠난다(docs/plan/data/rules.md 실패 축)
	if (FindLivingPawn(nullptr, nullptr) == nullptr)
	{
		UE_LOG(LogCL9, Log, TEXT("Wipe — everyone is down, the truck leaves."));
		const ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
		if (CL9GameState != nullptr && CL9GameState->GetPhase() == ECL9NightPhase::Night)
		{
			EndNight();
		}
		return;
	}

	// 방금 죽은 사람을 보고 있던 관전자는 모두 살아있는 팀원으로 넘어간다
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ACL9PlayerController* Spectator = Cast<ACL9PlayerController>(It->Get());
		if (Spectator != nullptr && Spectator->IsSpectating() && Spectator->GetViewTarget() == Dead)
		{
			SpectateNext(Spectator);
		}
	}

	ACL9PlayerController* DeadController = Cast<ACL9PlayerController>(Dead->GetController());
	if (DeadController == nullptr)
	{
		return;
	}

	const UCL9RulesDef* Rules = GetRules();
	const float Delay = Rules ? Rules->SpectateDelay : 0.f;
	if (Delay <= 0.f)
	{
		BeginSpectating(DeadController);
		return;
	}

	// 발사 후 신경 끄기: BeginSpectating이 플레이어가 여전히 죽어있는지 다시 확인하므로,
	// 지연 시간 동안 로비 부활이 일어나면 그냥 아무 일도 하지 않게 된다.
	TWeakObjectPtr<ACL9PlayerController> WeakController = DeadController;
	FTimerHandle Unused;
	GetWorldTimerManager().SetTimer(Unused, FTimerDelegate::CreateWeakLambda(this, [this, WeakController]()
	{
		BeginSpectating(WeakController.Get());
	}), Delay, false);
}

void ACL9GameMode::SpectateNext(ACL9PlayerController* Spectator)
{
	if (Spectator == nullptr || !Spectator->IsSpectating())
	{
		return;
	}

	AActor* Current = Spectator->GetViewTarget();
	ACL9Character* Next = FindLivingPawn(Spectator->GetPawn(), Current);
	if (Next != nullptr && Next != Current)
	{
		Spectator->SetViewTarget(Next);
	}
}

void ACL9GameMode::BeginSpectating(ACL9PlayerController* Spectator)
{
	ACL9Character* Own = Spectator ? Cast<ACL9Character>(Spectator->GetPawn()) : nullptr;
	if (Own == nullptr || !Own->IsDead())
	{
		return;
	}

	// 볼 대상이 없다(전멸이 이미 밤을 끝냈다) — 바닥 시점 그대로 유지된다
	ACL9Character* Target = FindLivingPawn(Own, nullptr);
	if (Target == nullptr)
	{
		return;
	}

	// 서버 쪽 SetViewTarget은 ClientSetViewTarget을 거쳐 소유 클라이언트에 전달된다;
	// 그 플래그의 OnRep이 거기서 화면과 IMC를 연다(docs/systems/ui_system.md)
	Spectator->SetViewTarget(Target);
	Spectator->HostSetSpectating(true);
	UE_LOG(LogCL9, Log, TEXT("'%s' now spectates '%s'."), *GetNameSafe(Own), *GetNameSafe(Target));
}

ACL9Character* ACL9GameMode::FindLivingPawn(const AActor* Except, const AActor* After) const
{
	if (GameState == nullptr)
	{
		return nullptr;
	}

	TArray<ACL9Character*> Living;
	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
		ACL9Character* Character = PlayerState ? Cast<ACL9Character>(PlayerState->GetPawn()) : nullptr;
		if (Character != nullptr && Character != Except && !Character->IsDead())
		{
			Living.Add(Character);
		}
	}
	if (Living.Num() == 0)
	{
		return nullptr;
	}

	// INDEX_NONE + 1은 첫 번째에 떨어진다 — `After`가 없으면(null이거나 방금 죽었으면) 처음부터 다시 시작한다
	const int32 AfterIndex = Living.IndexOfByPredicate([After](const ACL9Character* Candidate)
	{
		return Candidate == After;
	});
	return Living[(AfterIndex + 1) % Living.Num()];
}

void ACL9GameMode::ReviveDead()
{
	if (GameState == nullptr)
	{
		return;
	}

	// 패드는 파티가 깨어나는 곳이다(docs/plan/loop.md 귀환) — 배치된 게 없으면 제자리에서 그대로
	const ACL9TruckPad* Pad = nullptr;
	for (TActorIterator<ACL9TruckPad> It(GetWorld()); It; ++It)
	{
		Pad = *It;
		break;
	}

	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		ACL9Character* Character = PlayerState ? Cast<ACL9Character>(PlayerState->GetPawn()) : nullptr;
		if (Character != nullptr && Character->IsDead())
		{
			Character->HostRevive();
			if (Pad != nullptr)
			{
				const float HalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
				const FVector Scatter(FMath::FRandRange(-60.f, 60.f), FMath::FRandRange(-60.f, 60.f), HalfHeight + 5.f);
				Character->SetActorLocationAndRotation(Pad->GetActorLocation() + Scatter, Pad->GetActorRotation(),
					false, nullptr, ETeleportType::TeleportPhysics);
			}
		}

		ACL9PlayerController* Controller = PlayerState ? Cast<ACL9PlayerController>(PlayerState->GetPlayerController()) : nullptr;
		if (Controller != nullptr && Controller->IsSpectating())
		{
			Controller->SetViewTarget(Controller->GetPawn());
			Controller->HostSetSpectating(false);
		}
	}
}

const UCL9RulesDef* ACL9GameMode::GetRules() const
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	return Registry ? Registry->GetRules() : nullptr;
}

const UCL9ProgressionDef* ACL9GameMode::GetProgression() const
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	return Registry ? Registry->GetProgression() : nullptr;
}

void ACL9GameMode::SpawnNightPopulation(const UCL9SiteDef& Site, const UCL9ProgressionDef* Progression, int32 Day)
{
	if (!HasAuthority())
	{
		return;
	}

	// 개수 = [Min, Max] 사이에서 굴린 값 × 날짜 배율, 반올림(docs/systems/balance.md).
	// 굴림은 엔트리 단위지 동물 단위가 아니다 — 무리 전체가 함께 규모가 조정된다.
	auto RollCount = [](const FCL9SpawnEntry& Entry, float Multiplier)
	{
		return FMath::Max(0, FMath::RoundToInt(FMath::RandRange(Entry.MinCount, Entry.MaxCount) * Multiplier));
	};

	const float MonsterMult = Progression ? Progression->MonsterCountAt(Day) : 1.f;
	const float ItemMult = Progression ? Progression->ItemCountAt(Day) : 1.f;

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;

	// 생성된 맵: 동물과 재료는 생성기가 내준 지점 위에 떨어진다
	// (docs/plan/data/module.md 결정 4). 맵이 없으면 평지 레벨의 트럭 주변
	// 원형 배치가 대신한다.
	const ACL9SiteMap* SiteMap = ACL9SiteMap::Find(GetWorld());
	const bool bUsePoints = SiteMap != nullptr && SiteMap->GetLayout().bBuilt;

	TArray<int32> MonsterPoints;
	TArray<int32> LootPoints;
	TArray<int32> JarPoints;
	if (bUsePoints)
	{
		CL9SiteLayout::PointsOfPurpose(SiteMap->GetLayout(), ECL9PointPurpose::Monster, MonsterPoints);
		CL9SiteLayout::PointsOfPurpose(SiteMap->GetLayout(), ECL9PointPurpose::Loot, LootPoints);
		CL9SiteLayout::PointsOfPurpose(SiteMap->GetLayout(), ECL9PointPurpose::Jar, JarPoints);
	}

	// 목록은 시작 모듈에서 먼 것부터다 — 앞쪽만 보게 하면 "깊이 들어갈수록
	// 위험하다"가 스폰 규칙 하나 없이 배치만으로 성립한다.
	auto FindPointSpot = [this, SiteMap](const TArray<int32>& Points, float DeepShare, FVector& OutLocation)
	{
		if (Points.Num() == 0)
		{
			return false;
		}
		const int32 Limit = FMath::Clamp(FMath::RoundToInt(Points.Num() * DeepShare), 1, Points.Num());
		// 칸 한복판에서 벽 쪽으로 조금 흔들되, 벽에 박히지는 않을 만큼만
		const float Jitter = FMath::Max(SiteMap->GetParams().CellSize * 0.5f - 120.f, 0.f);
		for (int32 Attempt = 0; Attempt < 12; ++Attempt)
		{
			const FCL9LevelPoint& Point = SiteMap->GetPoints()[Points[FMath::RandRange(0, Limit - 1)]];
			FVector Candidate = SiteMap->PointToWorld(Point);
			Candidate.X += FMath::FRandRange(-Jitter, Jitter);
			Candidate.Y += FMath::FRandRange(-Jitter, Jitter);
			if (!IsInsideCalmRadius(Candidate))
			{
				OutLocation = Candidate + FVector(0.f, 0.f, 5.f);
				return true;
			}
		}
		return false;
	};

	for (const FCL9SpawnEntry& Entry : Site.MonsterSpawns)
	{
		const int32 Count = RollCount(Entry, MonsterMult);
		const UCL9MonsterDef* Def = Registry ? Registry->FindMonster(Entry.Alias) : nullptr;
		const float Radius = Def ? Def->Radius : 50.f;
		int32 Placed = 0;
		for (int32 Index = 0; Index < Count; ++Index)
		{
			FVector Location;
			if (!(bUsePoints ? FindPointSpot(MonsterPoints, 0.6f, Location) : FindSpawnPoint(900.f, 2400.f, Location)))
			{
				continue;
			}
			Location.Z += Radius;
			if (ACL9Monster* Monster = SpawnMonster(Entry.Alias, NAME_None, FTransform(FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f), Location)))
			{
				NightSpawned.Add(Monster);
				++Placed;
			}
		}
		UE_LOG(LogCL9, Log, TEXT("Night %d: %s ×%d placed (base %d~%d × %.2f = %d)."),
			Day, *Entry.Alias.ToString(), Placed, Entry.MinCount, Entry.MaxCount, MonsterMult, Count);
	}

	for (const FCL9SpawnEntry& Entry : Site.ItemSpawns)
	{
		const int32 Count = RollCount(Entry, ItemMult);
		int32 Placed = 0;
		for (int32 Index = 0; Index < Count; ++Index)
		{
			FVector Location;
			if (!(bUsePoints ? FindPointSpot(LootPoints, 1.f, Location) : FindSpawnPoint(400.f, 1800.f, Location)))
			{
				continue;
			}
			if (ACL9Item* Item = SpawnItem(Entry.Alias, NAME_None, FTransform(Location + FVector(0.f, 0.f, 30.f))))
			{
				NightSpawned.Add(Item);
				++Placed;
			}
		}
		UE_LOG(LogCL9, Log, TEXT("Night %d: %s ×%d placed (base %d~%d × %.2f = %d)."),
			Day, *Entry.Alias.ToString(), Placed, Entry.MinCount, Entry.MaxCount, ItemMult, Count);
	}

	// 반환용 항아리는 납품 방마다 하나다 — Jar 지점은 그 방의 칸마다 하나씩 나오므로
	// 모듈당 첫 칸만 쓴다 (docs/plan/data/module.md 결정 4).
	if (bUsePoints && JarPoints.Num() > 0)
	{
		if (JarClass == nullptr)
		{
			UE_LOG(LogCL9, Warning, TEXT("Night %d: no JarClass on the game mode — the delivery rooms stay empty (assign BP_CL9Jar in BP_CL9GameMode)."), Day);
		}
		else
		{
			TSet<int32> SeenModules;
			int32 Jars = 0;
			for (int32 Index : JarPoints)
			{
				const FCL9LevelPoint& Point = SiteMap->GetPoints()[Index];
				bool bAlready = false;
				SeenModules.Add(Point.Module, &bAlready);
				if (bAlready)
				{
					continue;
				}
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				if (AActor* Jar = GetWorld()->SpawnActor<ACL9Jar>(JarClass, SiteMap->PointToWorld(Point), FRotator::ZeroRotator, SpawnParams))
				{
					NightSpawned.Add(Jar);
					++Jars;
				}
			}
			UE_LOG(LogCL9, Log, TEXT("Night %d: %d jars placed in %d delivery rooms."), Day, Jars, SeenModules.Num());
		}
	}
}

bool ACL9GameMode::IsInsideCalmRadius(const FVector& Location) const
{
	const UCL9RulesDef* Rules = GetRules();
	const float CalmRadius = Rules ? Rules->ThreatActMinRadius : 420.f;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		const APawn* Pawn = It->Get() ? It->Get()->GetPawn() : nullptr;
		if (Pawn != nullptr && FVector::Dist2D(Pawn->GetActorLocation(), Location) < CalmRadius)
		{
			return true;
		}
	}
	return false;
}

void ACL9GameMode::ClearNightPopulation()
{
	if (!HasAuthority())
	{
		return;
	}

	// 사이트는 밤과 함께 사라진다 — 스포너가 배치했지만 아무도
	// 가져가지 않은 것은 없어진다. 들고 나간 아이템은 캐릭터에 부착되거나
	// 착용되면서 스포너 목록에서 이미 분리됐으므로 살아남는다.
	int32 Removed = 0;
	for (AActor* Actor : NightSpawned)
	{
		if (!IsValid(Actor))
		{
			continue;
		}
		if (const ACL9Item* Item = Cast<ACL9Item>(Actor); Item != nullptr && !Item->IsFree())
		{
			continue;
		}
		Actor->Destroy();
		++Removed;
	}
	NightSpawned.Reset();
	UE_LOG(LogCL9, Log, TEXT("Night population cleared — %d actors removed."), Removed);
}

bool ACL9GameMode::FindSpawnPoint(float MinDistance, float MaxDistance, FVector& OutLocation) const
{
	// 트럭 패드는 평지 레벨에서 사이트의 기준점이다; 배치된 게 없으면 PlayerStart가 대신한다
	FVector Anchor = FVector::ZeroVector;
	bool bAnchorFound = false;
	for (TActorIterator<ACL9TruckPad> It(GetWorld()); It; ++It)
	{
		Anchor = It->GetActorLocation();
		bAnchorFound = true;
		break;
	}
	if (!bAnchorFound)
	{
		for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
		{
			Anchor = It->GetActorLocation();
			break;
		}
	}

	for (int32 Attempt = 0; Attempt < 12; ++Attempt)
	{
		const float Bearing = FMath::FRandRange(0.f, 2.f * UE_PI);
		const float Distance = FMath::FRandRange(MinDistance, MaxDistance);
		const FVector Candidate = Anchor + FVector(FMath::Cos(Bearing), FMath::Sin(Bearing), 0.f) * Distance;

		// 플레이어의 ThreatActMinRadius 안에서는 아무것도 깨어나거나 스폰되지 않는다(docs/plan/data/rules.md)
		if (IsInsideCalmRadius(Candidate))
		{
			continue;
		}

		// 지면에 붙인다: 위에서 아래로 트레이스한다; 바닥이 없으면 다른 곳을 시도한다
		FHitResult Hit;
		const FVector Start = Candidate + FVector(0.f, 0.f, 500.f);
		const FVector End = Candidate - FVector(0.f, 0.f, 500.f);
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic))
		{
			OutLocation = Hit.ImpactPoint + FVector(0.f, 0.f, 5.f);
			return true;
		}
	}
	return false;
}

void ACL9GameMode::StartPlay()
{
	Super::StartPlay();

	// 오늘 밤의 미로는 첫 로비에서부터 세워진다 — 정차 지점이 밤이 향할 곳을 보여준다
	if (ACL9GameState* CL9GameState = GetGameState<ACL9GameState>())
	{
		CL9GameState->SetSiteAndDanger(DefaultSiteAlias, 1.f);
		CL9GameState->SetSiteMap(RollMapSeed(ForcedMapSeed));
	}

	// 첫 로비도 offer가 필요하다 — 이후 로비들은 EnterLobby에서 뽑는다
	OfferOrders();
}

void ACL9GameMode::OfferOrders()
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	if (!HasAuthority() || CL9GameState == nullptr || CL9GameState->GetPhase() != ECL9NightPhase::Lobby)
	{
		return;
	}

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9ProgressionDef* Progression = GetProgression();
	if (Registry == nullptr || Progression == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("OfferOrders: no registry or progression row — nothing to offer."));
		CL9GameState->SetOfferedOrders(TArray<FName>(), 0);
		return;
	}

	const int32 Day = CL9GameState->GetCurrentDay();
	TArray<const UCL9OrderDef*> Candidates;
	for (const UCL9OrderDef* Order : Registry->GetOrders())
	{
		if (Order->IsOfferedOn(Day))
		{
			Candidates.Add(Order);
		}
	}

	// 중복 없는 가중치 추첨 — N은 progression 행에서 온다(docs/systems/order.md 주문 흐름)
	TArray<FName> Offer;
	const int32 Wanted = FMath::Min(Progression->OrdersOfferedPerDay, Candidates.Num());
	while (Offer.Num() < Wanted && Candidates.Num() > 0)
	{
		float Total = 0.f;
		for (const UCL9OrderDef* Candidate : Candidates)
		{
			Total += Candidate->Weight;
		}
		float Roll = FMath::FRandRange(0.f, Total);
		int32 PickedIndex = Candidates.Num() - 1;
		for (int32 Index = 0; Index < Candidates.Num(); ++Index)
		{
			Roll -= Candidates[Index]->Weight;
			if (Roll <= 0.f)
			{
				PickedIndex = Index;
				break;
			}
		}
		Offer.Add(Candidates[PickedIndex]->Alias);
		Candidates.RemoveAt(PickedIndex);
	}

	const int32 Required = FMath::Min(Progression->OrdersRequiredPerDay, Offer.Num());
	CL9GameState->SetOfferedOrders(Offer, Required);

	FString Names;
	for (const FName& Alias : Offer)
	{
		Names += (Names.IsEmpty() ? TEXT("") : TEXT(", ")) + Alias.ToString();
	}
	UE_LOG(LogCL9, Log, TEXT("Day %d offer: %d orders [%s] — take %d (cl9.orders / cl9.pick)."), Day, Offer.Num(), *Names, Required);
}

bool ACL9GameMode::SaveRun(int32 Slot)
{
	const ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	if (!HasAuthority() || CL9GameState == nullptr)
	{
		return false;
	}

	UCL9SaveGame* Save = Cast<UCL9SaveGame>(UGameplayStatics::CreateSaveGameObject(UCL9SaveGame::StaticClass()));
	if (Save == nullptr)
	{
		return false;
	}

	Save->CurrentDay = CL9GameState->GetCurrentDay();
	Save->SharedMoney = CL9GameState->GetSharedMoney();
	Save->PeriodIncome = CL9GameState->GetPeriodIncome();
	Save->ActiveOrders = CL9GameState->AcceptedOrders;
	ListOwnedEquipment(Save->OwnedEquipment);

	const FString SlotName = UCL9SaveGame::SlotName(Slot);
	if (!UGameplayStatics::SaveGameToSlot(Save, SlotName, 0))
	{
		UE_LOG(LogCL9, Error, TEXT("Save failed: slot '%s' could not be written."), *SlotName);
		return false;
	}

	UE_LOG(LogCL9, Log, TEXT("Saved '%s' — day %d, %d ether, %d order(s) in progress, %d equipment."),
		*SlotName, Save->CurrentDay, Save->SharedMoney, Save->ActiveOrders.Num(), Save->OwnedEquipment.Num());
	return true;
}

bool ACL9GameMode::LoadRun(int32 Slot)
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	if (!HasAuthority() || CL9GameState == nullptr)
	{
		return false;
	}
	if (CL9GameState->GetPhase() != ECL9NightPhase::Lobby)
	{
		UE_LOG(LogCL9, Warning, TEXT("Load refused: a run is only restored in the lobby — the save unit is a day."));
		return false;
	}

	const FString SlotName = UCL9SaveGame::SlotName(Slot);
	const UCL9SaveGame* Save = Cast<UCL9SaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (Save == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("Load: slot '%s' is empty or unreadable."), *SlotName);
		return false;
	}
	if (Save->SaveVersion > UCL9SaveGame::CurrentVersion)
	{
		// 미래 버전은 조용히 버리지 않는다 — 읽지 않고 그대로 둔다(docs/systems/save.md 마이그레이션)
		UE_LOG(LogCL9, Error, TEXT("Load refused: slot '%s' is version %d, this build reads %d."),
			*SlotName, Save->SaveVersion, UCL9SaveGame::CurrentVersion);
		return false;
	}

	// 저장에 없는 Alias는 버린다 — 표에서 사라진 주문을 안고 있으면 영원히 못 내는 주문이 된다
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	TArray<FCL9AcceptedOrder> Restored;
	for (const FCL9AcceptedOrder& Entry : Save->ActiveOrders)
	{
		if (Registry != nullptr && Registry->FindOrder(Entry.OrderAlias) == nullptr)
		{
			UE_LOG(LogCL9, Warning, TEXT("Load: order '%s' has no row any more — dropped from the restored run."), *Entry.OrderAlias.ToString());
			continue;
		}
		Restored.Add(Entry);
	}

	CL9GameState->RestoreRun(Save->CurrentDay, Save->SharedMoney, Save->PeriodIncome, Restored);

	// 장비는 실물이다 — 저장된 alias만큼 트럭 옆에 다시 세운다
	int32 Respawned = 0;
	for (const FName& EquipAlias : Save->OwnedEquipment)
	{
		const UCL9EquipmentDef* Equip = Registry ? Registry->FindEquipment(EquipAlias) : nullptr;
		if (Equip == nullptr)
		{
			UE_LOG(LogCL9, Warning, TEXT("Load: equipment '%s' has no row any more — dropped."), *EquipAlias.ToString());
			continue;
		}
		if (SpawnEquipmentAtTruck(*Equip) != nullptr)
		{
			++Respawned;
		}
	}
	UE_LOG(LogCL9, Log, TEXT("Load: %d equipment respawned by the truck."), Respawned);
	UE_LOG(LogCL9, Log, TEXT("Loaded '%s' — day %d, %d ether, %d order(s) in progress."),
		*SlotName, CL9GameState->GetCurrentDay(), CL9GameState->GetSharedMoney(), Restored.Num());

	// 제시는 저장하지 않는다 — 불러온 일차 기준으로 다시 뽑는다
	OfferOrders();
	return true;
}

int32 ACL9GameMode::SettleOrdersAt(ACL9Counter& Counter)
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	if (!HasAuthority() || CL9GameState == nullptr)
	{
		return 0;
	}

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	if (Registry == nullptr)
	{
		return 0;
	}

	// 마감이 급한 주문부터 채운다 — 같은 완성물을 두 주문이 노릴 때 먼저 끝나는 쪽이 가져간다
	TArray<FCL9AcceptedOrder> Accepted = CL9GameState->AcceptedOrders;
	TArray<int32> ByDeadline;
	for (int32 Index = 0; Index < Accepted.Num(); ++Index)
	{
		ByDeadline.Add(Index);
	}
	ByDeadline.StableSort([&Accepted](int32 A, int32 B)
	{
		return Accepted[A].DeadlineDay < Accepted[B].DeadlineDay;
	});

	int32 Paid = 0;
	TArray<int32> Delivered;
	for (int32 Index : ByDeadline)
	{
		const FCL9AcceptedOrder& Entry = Accepted[Index];
		const UCL9OrderDef* Order = Registry->FindOrder(Entry.OrderAlias);
		if (Order == nullptr || Order->Ingredients.Num() == 0)
		{
			continue;
		}

		// 재료가 하나라도 모자라면 아무것도 소비하지 않는다 — 반쯤 낸 주문은 없다
		bool bComplete = true;
		for (const FCL9ItemCount& Need : Order->Ingredients)
		{
			int32 Found = 0;
			for (const FCL9CounterItem& Stocked : Counter.GetStock())
			{
				if (Stocked.ItemAlias == Need.ItemAlias && Stocked.ItemSubAlias == Need.ItemSubAlias)
				{
					++Found;
				}
			}
			if (Found < Need.Count)
			{
				bComplete = false;
				break;
			}
		}
		if (!bComplete)
		{
			continue;
		}

		for (const FCL9ItemCount& Need : Order->Ingredients)
		{
			for (int32 Taken = 0; Taken < Need.Count; ++Taken)
			{
				Counter.TakeStock(Need.ItemAlias, Need.ItemSubAlias);
			}
		}
		CL9GameState->AddIncome(Entry.Reward);
		Paid += Entry.Reward;
		Delivered.Add(Index);
		UE_LOG(LogCL9, Log, TEXT("Order delivered: %s — %d ether (day %d of %d). Money %d."),
			*Entry.OrderAlias.ToString(), Entry.Reward, CL9GameState->GetCurrentDay(), Entry.DeadlineDay, CL9GameState->GetSharedMoney());
	}

	if (Delivered.Num() > 0)
	{
		Delivered.Sort([](int32 A, int32 B) { return A > B; });
		for (int32 Index : Delivered)
		{
			Accepted.RemoveAt(Index);
		}
		CL9GameState->SetAcceptedOrders(Accepted);
	}
	else
	{
		UE_LOG(LogCL9, Log, TEXT("Counter: %d item(s) waiting, no order filled yet."), Counter.GetStock().Num());
	}
	return Paid;
}

int32 ACL9GameMode::SellStockAt(ACL9Counter& Counter)
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	if (!HasAuthority() || CL9GameState == nullptr)
	{
		return 0;
	}

	TArray<FCL9CounterItem> Sold;
	Counter.TakeAllStock(Sold);
	if (Sold.Num() == 0)
	{
		return 0;
	}

	int32 Total = 0;
	for (const FCL9CounterItem& Item : Sold)
	{
		Total += ComputeSellPrice(Item.Value);
	}
	CL9GameState->AddIncome(Total);
	UE_LOG(LogCL9, Log, TEXT("Counter sale: %d item(s) for %d ether. Money %d."), Sold.Num(), Total, CL9GameState->GetSharedMoney());
	return Total;
}

void ACL9GameMode::ExpireOrders()
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	if (!HasAuthority() || CL9GameState == nullptr)
	{
		return;
	}

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const int32 Day = CL9GameState->GetCurrentDay();

	TArray<FCL9AcceptedOrder> Kept;
	int32 Penalty = 0;
	for (const FCL9AcceptedOrder& Entry : CL9GameState->AcceptedOrders)
	{
		if (Entry.DeadlineDay >= Day)
		{
			Kept.Add(Entry);
			continue;
		}

		// 마감을 놓쳤다 — 벌지 못한 것에 더해 보상의 일정 비율을 물어낸다(docs/plan/data/order.md 마감)
		const UCL9OrderDef* Order = Registry ? Registry->FindOrder(Entry.OrderAlias) : nullptr;
		const int32 Fine = Order ? FMath::RoundToInt(Entry.Reward * Order->FailPenaltyRate) : 0;
		Penalty += Fine;
		UE_LOG(LogCL9, Log, TEXT("Order expired: %s — was due day %d, penalty %d ether."), *Entry.OrderAlias.ToString(), Entry.DeadlineDay, Fine);
	}

	if (Kept.Num() == CL9GameState->AcceptedOrders.Num())
	{
		return;
	}

	CL9GameState->SetAcceptedOrders(Kept);
	if (Penalty > 0)
	{
		const int32 Taken = CL9GameState->Deduct(Penalty);
		UE_LOG(LogCL9, Log, TEXT("Penalties: %d ether due, %d taken. Money %d."), Penalty, Taken, CL9GameState->GetSharedMoney());
	}
}

ACL9Item* ACL9GameMode::SpawnEquipmentAtTruck(const UCL9EquipmentDef& Equip)
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	if (Registry == nullptr)
	{
		return nullptr;
	}

	// 트럭 옆에 내려놓는다 — 산 물건은 인벤토리가 아니라 세상에 나온다 (docs/systems/shop.md)
	FVector Where = FVector::ZeroVector;
	for (TActorIterator<ACL9TruckPad> It(GetWorld()); It; ++It)
	{
		Where = It->GetActorLocation();
		break;
	}
	const UCL9ItemDef* ItemRow = Registry->FindItem(Equip.ItemDef.ItemAlias, Equip.ItemDef.ItemSubAlias);
	const float Radius = ItemRow ? ItemRow->Radius : 20.f;
	// 여러 개가 같은 자리에 겹치지 않게 트럭 주위로 흩는다
	const float Bearing = FMath::FRandRange(0.f, 2.f * UE_PI);
	Where += FVector(FMath::Cos(Bearing), FMath::Sin(Bearing), 0.f) * 200.f;
	Where.Z += Radius + 5.f;

	return SpawnItem(Equip.ItemDef.ItemAlias, Equip.ItemDef.ItemSubAlias, FTransform(Where));
}

void ACL9GameMode::ListOwnedEquipment(TArray<FName>& OutAliases) const
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	if (Registry == nullptr)
	{
		return;
	}

	for (const UCL9EquipmentDef* Equip : Registry->GetEquipment())
	{
		for (TActorIterator<ACL9Item> It(GetWorld()); It; ++It)
		{
			if (It->GetAlias() == Equip->ItemDef.ItemAlias && It->GetSubAlias() == Equip->ItemDef.ItemSubAlias)
			{
				OutAliases.Add(Equip->Alias);
			}
		}
	}
}

int32 ACL9GameMode::LoseEquipmentLeftBehind()
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	if (!HasAuthority() || Registry == nullptr)
	{
		return 0;
	}

	FVector Truck = FVector::ZeroVector;
	bool bFoundTruck = false;
	for (TActorIterator<ACL9TruckPad> It(GetWorld()); It; ++It)
	{
		Truck = It->GetActorLocation();
		bFoundTruck = true;
		break;
	}
	if (!bFoundTruck)
	{
		// 트럭이 없으면 무엇을 두고 왔는지 판정할 기준이 없다 — 아무것도 잃지 않는다
		return 0;
	}

	int32 Lost = 0;
	for (const UCL9EquipmentDef* Equip : Registry->GetEquipment())
	{
		for (TActorIterator<ACL9Item> It(GetWorld()); It; ++It)
		{
			ACL9Item* Item = *It;
			if (Item->GetAlias() != Equip->ItemDef.ItemAlias || Item->GetSubAlias() != Equip->ItemDef.ItemSubAlias)
			{
				continue;
			}
			// 손에 들거나 수레에 실어 온 것은 그 사람과 함께 트럭 곁에 있다 — 거리가 알아서 걸러낸다
			if (FVector::Dist2D(Item->GetActorLocation(), Truck) > EquipmentKeepRadius)
			{
				UE_LOG(LogCL9, Log, TEXT("Equipment left behind: '%s' — lost with the site."), *Equip->Alias.ToString());
				Item->Destroy();
				++Lost;
			}
		}
	}
	return Lost;
}

void ACL9GameMode::OpenDawnScreen()
{
	if (!HasAuthority() || DawnUIRow.IsNone())
	{
		return;
	}

	// 화면은 각자의 것이다 — 호스트가 모든 컨트롤러에 각각 보낸다 (docs/systems/ui_system.md)
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ACL9PlayerController* Controller = Cast<ACL9PlayerController>(It->Get()))
		{
			Controller->Client_OpenScreen(DawnUIRow);
		}
	}
}

bool ACL9GameMode::Settle()
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	const UCL9ProgressionDef* Progression = GetProgression();
	if (!HasAuthority() || CL9GameState == nullptr || Progression == nullptr)
	{
		return false;
	}

	const int32 Day = CL9GameState->GetCurrentDay();
	if (!Progression->IsSettlementDay(Day))
	{
		return false;
	}

	// 판정 대상은 보유 잔액이 아니라 이번 기간에 번 것이다 (docs/plan/data/progression.md 종료 조건)
	const int32 Target = Progression->QuotaAt(Day);
	const int32 Earned = CL9GameState->GetPeriodIncome();
	const bool bPassed = Earned >= Target;

	// **상태를 되돌리기 전에** 알린다 — 통과하면 기간 수입이 0이 되고 미달하면 1일차로 돌아가므로,
	// 이 뒤에는 새벽 화면이 보여줄 숫자를 어디서도 구할 수 없다
	CL9GameState->Multicast_Settled(Day, Earned, Target, bPassed);

	if (!bPassed)
	{
		UE_LOG(LogCL9, Log, TEXT("Settlement failed on day %d: earned %d < %d — the run ends here."), Day, Earned, Target);
		StartNewRun();
		return true;
	}

	UE_LOG(LogCL9, Log, TEXT("Settlement passed on day %d: earned %d >= %d. Next period starts."), Day, Earned, Target);
	CL9GameState->ResetPeriodIncome();
	return true;
}

void ACL9GameMode::StartNewRun()
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	if (!HasAuthority() || CL9GameState == nullptr)
	{
		return;
	}

	// 판이 죽어도 기록은 남는다 — 덮어쓰기 전에 먼저 적는다 (docs/systems/save.md 기록)
	RecordBestDay(CL9GameState->GetCurrentDay());

	// 새 판은 빈손으로 연다 — 산 장비도 이전 판의 것이다
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	if (Registry != nullptr)
	{
		for (const UCL9EquipmentDef* Equip : Registry->GetEquipment())
		{
			for (TActorIterator<ACL9Item> It(GetWorld()); It; ++It)
			{
				if (It->GetAlias() == Equip->ItemDef.ItemAlias && It->GetSubAlias() == Equip->ItemDef.ItemSubAlias)
				{
					It->Destroy();
				}
			}
		}
	}

	CL9GameState->RestoreRun(1, 0, 0, TArray<FCL9AcceptedOrder>());
	UE_LOG(LogCL9, Log, TEXT("A new run opens at day 1 — nothing carries over."));
}

void ACL9GameMode::RecordBestDay(int32 Day)
{
	const FString SlotName = UCL9RecordSave::SlotName();
	UCL9RecordSave* Record = Cast<UCL9RecordSave>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (Record == nullptr)
	{
		Record = Cast<UCL9RecordSave>(UGameplayStatics::CreateSaveGameObject(UCL9RecordSave::StaticClass()));
	}
	if (Record == nullptr || Day <= Record->BestDay)
	{
		return;
	}

	Record->BestDay = Day;
	if (UGameplayStatics::SaveGameToSlot(Record, SlotName, 0))
	{
		UE_LOG(LogCL9, Log, TEXT("Record: best day is now %d."), Day);
	}
}

ECL9BuyResult ACL9GameMode::BuyEquipment(FName EquipAlias)
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	if (!HasAuthority() || CL9GameState == nullptr)
	{
		return ECL9BuyResult::NoRow;
	}

	const ECL9BuyResult Verdict = CL9GameState->CanBuy(EquipAlias);
	if (Verdict != ECL9BuyResult::Ok)
	{
		UE_LOG(LogCL9, Log, TEXT("Buy refused: '%s' — reason %d."), *EquipAlias.ToString(), static_cast<int32>(Verdict));
		return Verdict;
	}

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9EquipmentDef* Equip = Registry ? Registry->FindEquipment(EquipAlias) : nullptr;
	if (Equip == nullptr)
	{
		return ECL9BuyResult::NoRow;
	}

	ACL9Item* Bought = SpawnEquipmentAtTruck(*Equip);
	if (Bought == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("Buy failed: '%s' spawned nothing — no ether was taken."), *EquipAlias.ToString());
		return ECL9BuyResult::NoRow;
	}

	const int32 Paid = CL9GameState->Deduct(Equip->Price);
	UE_LOG(LogCL9, Log, TEXT("Bought '%s' for %d ether (owned %d). Money %d."),
		*EquipAlias.ToString(), Paid, CL9GameState->GetOwnedCount(EquipAlias), CL9GameState->GetSharedMoney());
	return ECL9BuyResult::Ok;
}

int32 ACL9GameMode::ComputeSellPrice(int32 RolledValue) const
{
	if (RolledValue <= 0)
	{
		return 0;
	}

	const ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9SiteDef* Site = (Registry != nullptr && CL9GameState != nullptr) ? Registry->FindSite(CL9GameState->GetCurrentSiteAlias()) : nullptr;
	const UCL9ProgressionDef* Progression = GetProgression();

	// 굴림값 × 목적지 배율 × 일자 배율 — 세 항 모두 표에 있다(docs/plan/economy.md 가격식)
	const float SiteRate = Site ? Site->ValueRate : 1.f;
	const float DayRate = Progression ? Progression->ValueAt(CL9GameState ? CL9GameState->GetCurrentDay() : 1) : 1.f;
	return FMath::RoundToInt(RolledValue * SiteRate * DayRate);
}

bool ACL9GameMode::AcceptOrders(const TArray<int32>& OfferedIndices)
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	if (!HasAuthority() || CL9GameState == nullptr)
	{
		return false;
	}
	if (CL9GameState->GetPhase() != ECL9NightPhase::Lobby)
	{
		UE_LOG(LogCL9, Warning, TEXT("AcceptOrders: orders are picked in the lobby only."));
		return false;
	}

	const TArray<FName>& Offer = CL9GameState->OfferedOrders;
	const int32 Required = CL9GameState->OrdersRequired;
	if (Offer.Num() == 0)
	{
		UE_LOG(LogCL9, Warning, TEXT("AcceptOrders: nothing is on offer (already picked, or no order rows for this day)."));
		return false;
	}

	// 정확히 M개의 서로 다른 선택, 모두 offer 안에 있어야 한다 — 이 선택이 그날의 계획이 된다
	const TSet<int32> Unique(OfferedIndices);
	if (Unique.Num() != OfferedIndices.Num() || OfferedIndices.Num() != Required)
	{
		UE_LOG(LogCL9, Warning, TEXT("AcceptOrders: need exactly %d distinct picks, got %d (%d distinct)."), Required, OfferedIndices.Num(), Unique.Num());
		return false;
	}
	for (int32 Index : OfferedIndices)
	{
		if (!Offer.IsValidIndex(Index))
		{
			UE_LOG(LogCL9, Warning, TEXT("AcceptOrders: index %d is outside the offer of %d."), Index, Offer.Num());
			return false;
		}
	}

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const int32 Day = CL9GameState->GetCurrentDay();

	TArray<FCL9AcceptedOrder> Accepted = CL9GameState->AcceptedOrders;
	for (int32 Index : OfferedIndices)
	{
		const UCL9OrderDef* Order = Registry ? Registry->FindOrder(Offer[Index]) : nullptr;
		if (Order == nullptr)
		{
			UE_LOG(LogCL9, Error, TEXT("AcceptOrders: offered order '%s' has no usable row — pick refused."), *Offer[Index].ToString());
			return false;
		}
		FCL9AcceptedOrder& Entry = Accepted.AddDefaulted_GetRef();
		Entry.OrderAlias = Order->Alias;
		Entry.AcceptedDay = Day;
		Entry.DeadlineDay = Day + Order->DeadlineDays - 1;
		// 보상은 수락하는 순간 굴려 고정한다 — 카드가 범위가 아니라 실제로 받을 값을 보여주고,
		// 위약금도 같은 값에서 나온다(2026-09-06 결정)
		Entry.Reward = FMath::RandRange(Order->RewardMin, Order->RewardMax);
		UE_LOG(LogCL9, Log, TEXT("Order accepted: %s — %d ether, deliver by day %d."), *Order->Alias.ToString(), Entry.Reward, Entry.DeadlineDay);
	}

	CL9GameState->SetAcceptedOrders(Accepted);
	// offer는 소진됐다 — 다음 로비가 다시 뽑는다
	CL9GameState->SetOfferedOrders(TArray<FName>(), 0);
	return true;
}
