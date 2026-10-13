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
#include "Items/CL9DestinationBoard.h"
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
#include "NavigationSystem.h"
#include "TimerManager.h"
#include "World/CL9SiteLayout.h"
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

ACL9Item* ACL9GameMode::SpawnItem(FName ItemAlias, FName ItemSubAlias, const FTransform& Transform, int32 PresetValue)
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
		// SetAlias가 호스트에서 LoadData를 바로 돌려 값을 굴린다 — 보존값은 그 뒤에 덮어써야 남는다
		Item->SetAlias(ItemAlias, ItemSubAlias);
		if (PresetValue >= 0)
		{
			Item->HostSetValue(PresetValue);
		}
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
		NightStartWaitBegan = -1.0;
		return;
	}

	// 이미 미뤄 둔 시작이 다음 프레임에 다시 본다 — 두 번째 요청은 그것에 맡긴다
	if (NightStartRetryHandle.IsValid())
	{
		UE_LOG(LogCL9, Log, TEXT("StartNight: already waiting for the site build / navigation."));
		return;
	}

	// 구역은 프레임마다 하나씩 서고 내비는 그 뒤에 빌드된다 — 다 서기 전에는 지점도 길도 없다
	if (DeferNightStartWhileBuilding())
	{
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
	const bool bNoBlackout = Progression != nullptr && Progression->IsBlackoutDisabled(Day);

	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9SiteDef* Site = Registry ? Registry->FindSite(ActiveSiteAlias) : nullptr;
	if (Site == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("StartNight: site row '%s' is missing or broken — no animals or ingredients tonight (set the row's Level, or DefaultSiteAlias in the game mode BP)."),
			*ActiveSiteAlias.ToString());
	}

	// 밤 시계는 사이트가 덮어쓸 수 있다 — 0이면 규칙의 전역값 (docs/plan/data/site.md 밤 시계)
	const float NightLength = (Site ? UCL9SiteDef::OrRule(Site->NightLength, Rules->NightLength) : Rules->NightLength)
		* (Progression ? Progression->NightLengthAt(Day) : 1.f);
	const float DarkBase = Site ? UCL9SiteDef::OrRule(Site->DarkBase, Rules->DarkBase) : Rules->DarkBase;
	const float DarkMin = Site ? UCL9SiteDef::OrRule(Site->DarkMin, Rules->DarkMin) : Rules->DarkMin;
	const float Danger = (Site ? Site->BaseDanger : 1.f) * (Progression ? Progression->DangerAt(Day) : 1.f);
	CL9GameState->SetSiteAndDanger(Site ? Site->Alias : NAME_None, Danger);

	// 로비가 이미 오늘 밤의 미로를 세워뒀다(2026-09-06 결정 — 로비부터 오늘 밤 미로); 어쩌다
	// 미로가 없는 월드만 지금 하나 받는다. 공유되는 것은 시드뿐이다 — 모든 머신이
	// 그 시드로부터 같은 미로를 깐다(docs/systems/map.md).
	if (CL9GameState->GetMapSeed() == 0)
	{
		CL9GameState->SetSiteMap(RollMapSeed(ForcedMapSeed));
		// 구역 사이트는 구역 0 만 지금 섰다 — 나머지를 기다린다. 다시 올 때는 시드가 있어 이 가지를 지나지 않는다
		if (DeferNightStartWhileBuilding())
		{
			return;
		}
	}

	CL9GameState->SetWipePenalty(FCL9WipePenalty());
	// 지난 판의 임포스터 공개도 걷는다 — 판 동안에는 정체가 복제되지 않는다(7g)
	CL9GameState->SetImposterResult(FCL9ImposterResult());
	CL9GameState->SetContributions(TArray<FCL9Contribution>());
	CL9GameState->SetQuotaPenalty(FCL9WipePenalty());
	ImposterWinner.Reset();
	ImposterKiller.Reset();
	RoundImposter.Reset();
	// 판 수입의 기준점(7f) — 판 끝의 기간 수입에서 이 값을 빼면 이번 판에 번 것이다(정산은 판 끝 뒤 EnterLobby 에서야 기간을 되돌린다)
	NightIncomeStart = CL9GameState->GetPeriodIncome();
	CL9GameState->StartNightClock(NightLength, DarkBase, DarkMin, bNoBlackout);
	// 레벨의 첫 판은 트럭이 들어와 서면서 연다(7j) — 판 시계는 도착부터 흐른다. 레버로 연 판은 트럭이 이미 서 있다
	if (bArrivalPending)
	{
		bArrivalPending = false;
		CL9GameState->SetArrival();
		UE_LOG(LogCL9, Log, TEXT("Arrival: the truck backs in — %.1fs, the round clock is already running."), Rules->ArrivalTime);
	}
	CL9GameState->SetPhase(ECL9NightPhase::Night);
	GetWorldTimerManager().SetTimer(NightExpiryHandle, this, &ACL9GameMode::EndNight, NightLength);

	UE_LOG(LogCL9, Log, TEXT("Night %d at '%s' started — %.0fs to dawn, blackout %s, danger ×%.2f."),
		Day, *ActiveSiteAlias.ToString(), NightLength, bNoBlackout ? TEXT("off (day override)") : *FString::Printf(TEXT("base %.0fs"), DarkBase), Danger);

	if (Site != nullptr)
	{
		SpawnNightPopulation(*Site, Progression, Day);
		ArmMidSpawns(*Site, Day);
	}
	// 임포스터(7g) — 판이 열린 뒤 산 사람 수로 굴린다. 본인 머신에만 알려진다
	AssignImposter();
}

bool ACL9GameMode::DeferNightStartWhileBuilding()
{
	UWorld* World = GetWorld();
	const bool bSitePending = ACL9SiteMap::IsBuildPending(World);
	// 더러운 영역이 아직 생성기에 넘어가지 않은 프레임도 "진행 중"으로 본다 — 방금 선 구역의 콜리전이 그 사이에 있다
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(World);
	const bool bNavBusy = NavSystem != nullptr && (NavSystem->IsNavigationBuildInProgress() || NavSystem->HasDirtyAreasQueued());

	if (!bSitePending && !bNavBusy)
	{
		if (NightStartWaitBegan >= 0.0)
		{
			UE_LOG(LogCL9, Log, TEXT("StartNight: site build and navigation done after %.2fs of waiting — starting the night."),
				World->GetRealTimeSeconds() - NightStartWaitBegan);
			NightStartWaitBegan = -1.0;
		}
		return false;
	}

	if (NightStartWaitBegan < 0.0)
	{
		NightStartWaitBegan = World->GetRealTimeSeconds();
		UE_LOG(LogCL9, Log, TEXT("StartNight: deferred — site build %s, navigation %s. Retrying every frame."),
			bSitePending ? TEXT("pending") : TEXT("done"), bNavBusy ? TEXT("building") : TEXT("idle"));
	}
	NightStartRetryHandle = GetWorldTimerManager().SetTimerForNextTick(this, &ACL9GameMode::RetryStartNight);
	return true;
}

void ACL9GameMode::RetryStartNight()
{
	// 실행 중인 타이머의 핸들은 아직 살아 있다 — 비워야 StartNight 의 "이미 기다리는 중" 검사를 지난다
	NightStartRetryHandle.Invalidate();
	StartNight();
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

	// 임포스터 판정은 남겨짐보다 먼저다(7g) — 트럭 밖의 산 사람은 아직 "산 사람"이라 임포스터가 이길 수 없다(남겨짐은 승리로 치지 않는다)
	JudgeImposter();
	// 판이 끝나지 않은 전멸(bWipeEndsRun 꺼짐) — 다음 판으로 가는 대신 이더를 깎는다. 새벽 화면·저장보다 먼저다(EnterLobby)
	ApplyWipePenalty();
	// 처치 기여 목록(7h) — 새벽 화면이 보인다
	PublishContributions();
	// 바깥 재화 귀가 보상(7f) — 새벽 화면을 여는 RPC 보다 먼저 보내야 각 머신의 화면이 이번 판 숫자를 안다
	GrantRoundWallet();

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
	// 새벽 처리: 정산이 먼저다 — 판이 끝나면 새 판 상태를 저장해야 하기 때문이다. 회사가 있으면 정산은 회사에서 한다(7i) —
	// 정산일 로비의 출발 레버가 판 대신 회사로 간다
	if (!HasCompany())
	{
		Settle();
	}
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

FVector ACL9GameMode::WakeSpot(const ACL9TruckPad& Pad, const ACL9Character& Character)
{
	const float HalfHeight = Character.GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float Radius = Character.GetCapsuleComponent()->GetScaledCapsuleRadius();

	// **패드 원점에 세우면 안 된다.** 패드는 바닥 판이 아니라 트럭 몸체까지 포함한 덩어리라
	// (2026-09-10 실측: 400 × 400 × 240), 원점에 놓인 캡슐은 그 콜리전 안에 갇혀 한 발도
	// 못 움직인다. 사이트 맵은 패드에서 **+X 쪽으로** 서므로, 그쪽으로 비켜 세우면
	// 트럭을 등지고 입구를 보게 된다.
	FVector PadOrigin = FVector::ZeroVector;
	FVector PadExtent = FVector::ZeroVector;
	Pad.GetActorBounds(/*bOnlyCollidingComponents*/ true, PadOrigin, PadExtent);

	// 여럿이 같은 자리에 겹치지 않게 옆으로만 흩는다 — 앞뒤로 흩으면 다시 트럭에 박힌다.
	return FVector(
		PadOrigin.X + PadExtent.X + Radius + 60.f,
		PadOrigin.Y + FMath::FRandRange(-60.f, 60.f),
		Pad.GetActorLocation().Z + HalfHeight + 5.f);
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
		// 처형 표시(ExecutedBy·처형 시각)는 죽은 뒤에도 남아 있다 — 부활에서 걷는다 (D9). 처형 중이 아니면 아무 일도 없다
		if (Character != nullptr)
		{
			Character->HostEndExecuted();
		}
		if (Character != nullptr && Character->IsDead())
		{
			Character->HostRevive();
			if (Pad != nullptr)
			{
				Character->SetActorLocationAndRotation(WakeSpot(*Pad, *Character), FRotator::ZeroRotator,
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

	// 지난 밤 큐가 남아 있으면 버린다(보통은 ClearNightPopulation 이 이미 비웠다)
	GetWorldTimerManager().ClearTimer(MonsterSpawnHandle);
	PendingMonsterSpawns.Reset();
	NextPendingMonster = 0;
	PendingMonstersSpawned = 0;

	// 목록은 시작 모듈에서 먼 것부터다 — 앞쪽만 보게 하면 "깊이 들어갈수록
	// 위험하다"가 스폰 규칙 하나 없이 배치만으로 성립한다.
	auto FindPointSpot = [this](const ACL9SiteMap& SiteMap, const TArray<int32>& Points, float DeepShare, FVector& OutLocation)
	{
		if (Points.Num() == 0)
		{
			return false;
		}
		const int32 Limit = FMath::Clamp(FMath::RoundToInt(Points.Num() * DeepShare), 1, Points.Num());
		// 칸 한복판에서 벽 쪽으로 조금 흔들되, 벽에 박히지는 않을 만큼만
		const float Jitter = FMath::Max(SiteMap.GetParams().CellSize * 0.5f - 120.f, 0.f);
		for (int32 Attempt = 0; Attempt < 12; ++Attempt)
		{
			const FCL9LevelPoint& Point = SiteMap.GetPoints()[Points[FMath::RandRange(0, Limit - 1)]];
			FVector Candidate = SiteMap.PointToWorld(Point);
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

	// 한 무리(사이트 열 또는 구역 하나)의 자리를 주어진 지점 목록 위에 정해 큐에 넣는다 — 스폰은 SpawnPendingMonsters 가
	// 프레임마다 나눠 한다. 자리·방향은 지금 정하므로 배치 규칙(지점·깊이 몫·조용한 반경)은 한 번에 스폰하던 때와 같다.
	// SiteMap 이 없으면 트럭 주변 원환이다. Where는 로그에 남길 이름이다.
	auto QueueMonsters = [&](const ACL9SiteMap* SiteMap, const TArray<FCL9SpawnEntry>& Entries, const TArray<int32>& Points, float DeepShare, const TCHAR* Where)
	{
		for (const FCL9SpawnEntry& Entry : Entries)
		{
			const int32 Count = RollCount(Entry, MonsterMult);
			const UCL9MonsterDef* Def = Registry ? Registry->FindMonster(Entry.Alias) : nullptr;
			const float Radius = Def ? Def->Radius : 50.f;
			int32 Queued = 0;
			for (int32 Index = 0; Index < Count; ++Index)
			{
				FVector Location;
				if (!(SiteMap ? FindPointSpot(*SiteMap, Points, DeepShare, Location) : FindSpawnPoint(900.f, 2400.f, Location)))
				{
					continue;
				}
				Location.Z += Radius;
				PendingMonsterSpawns.Add({ Entry.Alias, FTransform(FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f), Location) });
				++Queued;
			}
			UE_LOG(LogCL9, Log, TEXT("Night %d%s: %s ×%d queued (base %d~%d × %.2f = %d)."),
				Day, Where, *Entry.Alias.ToString(), Queued, Entry.MinCount, Entry.MaxCount, MonsterMult, Count);
		}
	};

	auto SpawnItems = [&](const ACL9SiteMap* SiteMap, const TArray<FCL9SpawnEntry>& Entries, const TArray<int32>& Points, const TCHAR* Where)
	{
		for (const FCL9SpawnEntry& Entry : Entries)
		{
			const int32 Count = RollCount(Entry, ItemMult);
			int32 Placed = 0;
			for (int32 Index = 0; Index < Count; ++Index)
			{
				FVector Location;
				if (!(SiteMap ? FindPointSpot(*SiteMap, Points, 1.f, Location) : FindSpawnPoint(400.f, 1800.f, Location)))
				{
					continue;
				}
				if (ACL9Item* Item = SpawnItem(Entry.Alias, NAME_None, FTransform(Location + FVector(0.f, 0.f, 30.f))))
				{
					NightSpawned.Add(Item);
					++Placed;
				}
			}
			UE_LOG(LogCL9, Log, TEXT("Night %d%s: %s ×%d placed (base %d~%d × %.2f = %d)."),
				Day, Where, *Entry.Alias.ToString(), Placed, Entry.MinCount, Entry.MaxCount, ItemMult, Count);
		}
	};

	// 맵 하나(단일 사이트 또는 구역 하나)의 몫. SiteMap 이 없으면 평지 레벨 — 사이트 열도 구역 몫도 트럭 주변 원환에 떨어진다.
	// Prefix 는 로그의 "Night N" 뒤에 붙는다(단일 사이트·평지는 빈 문자열 — 로그가 예전과 같다).
	auto SpawnForMap = [&](const ACL9SiteMap* SiteMap, const TArray<FCL9SiteZone>& Zones, const FString& Prefix)
	{
		TArray<int32> MonsterPoints;
		TArray<int32> LootPoints;
		TArray<int32> JarPoints;
		if (SiteMap != nullptr)
		{
			CL9SiteLayout::PointsOfPurpose(SiteMap->GetLayout(), ECL9PointPurpose::Monster, MonsterPoints);
			CL9SiteLayout::PointsOfPurpose(SiteMap->GetLayout(), ECL9PointPurpose::Loot, LootPoints);
			CL9SiteLayout::PointsOfPurpose(SiteMap->GetLayout(), ECL9PointPurpose::Jar, JarPoints);
		}

		// 고정물(부엌 모듈의 조리대 등)은 뽑힌 소품 세트가 정한 자리에 호스트가 세운다 (docs/goal/loop-7c-kitchen-in-building.md).
		// 재료·몬스터보다 먼저다 — 그 둘은 겹치면 비켜 서는 스폰이라, 조리대 자리에 떨어질 재료가 조리대 속에 박히지 않는다
		if (SiteMap != nullptr)
		{
			TArray<ACL9SiteMap::FFixtureSpawn> Fixtures;
			SiteMap->GetFixtureSpawns(Fixtures);
			TArray<FString> Placed;
			for (const ACL9SiteMap::FFixtureSpawn& Fixture : Fixtures)
			{
				UClass* FixtureClass = Fixture.ActorClass.LoadSynchronous();
				if (FixtureClass == nullptr)
				{
					UE_LOG(LogCL9, Warning, TEXT("Night %d%s: fixture class '%s' of module %d (%s) failed to load — skipped (ModuleTable PropSets.Fixtures)."),
						Day, *Prefix, *Fixture.ActorClass.ToString(), Fixture.Module, *Fixture.ModuleAlias.ToString());
					continue;
				}
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				if (AActor* Spawned = GetWorld()->SpawnActor<AActor>(FixtureClass, Fixture.Transform, SpawnParams))
				{
					NightSpawned.Add(Spawned);
					Placed.Add(FString::Printf(TEXT("%s in module %d (%s) at %s yaw %.0f"), *FixtureClass->GetName(), Fixture.Module,
						*Fixture.ModuleAlias.ToString(), *Fixture.Transform.GetLocation().ToCompactString(), Fixture.Transform.Rotator().Yaw));
				}
			}
			UE_LOG(LogCL9, Log, TEXT("Night %d%s: %d fixture(s) placed%s%s."), Day, *Prefix, Placed.Num(),
				Placed.Num() > 0 ? TEXT(" — ") : TEXT(""), *FString::Join(Placed, TEXT("; ")));
		}

		QueueMonsters(SiteMap, Site.MonsterSpawns, MonsterPoints, 0.6f, *Prefix);
		SpawnItems(SiteMap, Site.ItemSpawns, LootPoints, *Prefix);

		// 깊이 구역: 구역마다 자기 띠의 지점에만 놓는다 (docs/plan/data/site.md 구역, 2026-10-03). 띠 안에서는
		// 몬스터도 재료도 고르게 고른다 — 깊이에 따른 차이는 이미 구역의 행이 말하고 있다. 깊이는 맵(레이아웃)마다 따로 잰다.
		for (const FCL9SiteZone& Zone : Zones)
		{
			TArray<int32> ZoneMonsterPoints;
			TArray<int32> ZoneLootPoints;
			if (SiteMap != nullptr)
			{
				CL9SiteLayout::PointsInDepthBand(SiteMap->GetLayout(), ECL9PointPurpose::Monster, Zone.DepthMin, Zone.DepthMax, ZoneMonsterPoints);
				CL9SiteLayout::PointsInDepthBand(SiteMap->GetLayout(), ECL9PointPurpose::Loot, Zone.DepthMin, Zone.DepthMax, ZoneLootPoints);
			}
			const FString Where = Prefix + FString::Printf(TEXT(" zone %s (depth %.2f~%.2f, %d monster / %d loot points)"),
				*Zone.Name.ToString(), Zone.DepthMin, Zone.DepthMax, ZoneMonsterPoints.Num(), ZoneLootPoints.Num());
			QueueMonsters(SiteMap, Zone.MonsterSpawns, ZoneMonsterPoints, 1.f, *Where);
			SpawnItems(SiteMap, Zone.ItemSpawns, ZoneLootPoints, *Where);
		}

		// 반환용 항아리는 납품 방마다 하나다 — Jar 지점은 그 방의 칸마다 하나씩 나오므로
		// 모듈당 첫 칸만 쓴다 (docs/plan/data/module.md 결정 4). 모듈 번호는 레이아웃마다 따로라 집합도 맵마다 새로 만든다.
		if (SiteMap != nullptr && JarPoints.Num() > 0)
		{
			if (JarClass == nullptr)
			{
				UE_LOG(LogCL9, Warning, TEXT("Night %d%s: no JarClass on the game mode — the delivery rooms stay empty (assign BP_CL9Jar in BP_CL9GameMode)."), Day, *Prefix);
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
				UE_LOG(LogCL9, Log, TEXT("Night %d%s: %d jars placed in %d delivery rooms."), Day, *Prefix, Jars, SeenModules.Num());
			}
		}
	};

	// 생성된 맵: 동물과 재료는 생성기가 내준 지점 위에 떨어진다 (docs/plan/data/module.md 결정 4). 구역 사이트는 구역마다
	// 맵이 하나다(ACL9SiteMap::FindAll, 구역 순) — 맵마다 자기 지점·자기 구역 행(GetZones, 비면 사이트 Zones)을 쓴다.
	// 아직 지어지지 않은 맵은 건너뛰고, 지어진 맵이 하나도 없으면 평지 레벨의 트럭 주변 원형 배치가 대신한다.
	TArray<ACL9SiteMap*> SiteMaps;
	ACL9SiteMap::FindAll(GetWorld(), SiteMaps);
	SiteMaps.RemoveAll([](const ACL9SiteMap* SiteMap) { return SiteMap == nullptr || !SiteMap->GetLayout().bBuilt; });
	if (SiteMaps.Num() == 0)
	{
		SpawnForMap(nullptr, Site.Zones, FString());
	}
	for (const ACL9SiteMap* SiteMap : SiteMaps)
	{
		const int32 District = SiteMap->GetDistrictIndex();
		SpawnForMap(SiteMap, SiteMap->GetZones(), District == INDEX_NONE ? FString() : FString::Printf(TEXT(" district %d"), District));
	}

	// 몬스터는 한꺼번에 올리지 않는다 — 다음 프레임부터 MonsterSpawnPerFrame 마리씩 (docs/goal/giant-stage.md Gate answers)
	if (PendingMonsterSpawns.Num() > 0)
	{
		PendingMonstersBegan = GetWorld()->GetRealTimeSeconds();
		MonsterSpawnHandle = GetWorldTimerManager().SetTimerForNextTick(this, &ACL9GameMode::SpawnPendingMonsters);
	}
}

void ACL9GameMode::SpawnPendingMonsters()
{
	MonsterSpawnHandle.Invalidate();

	// 큐를 다 비우기 전에 밤이 끝났다 — 남은 몫은 버린다(이미 선 것은 로비가 치운다)
	const ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	const int32 Day = CL9GameState ? CL9GameState->GetCurrentDay() : 0;
	if (CL9GameState == nullptr || CL9GameState->GetPhase() != ECL9NightPhase::Night)
	{
		UE_LOG(LogCL9, Log, TEXT("Night %d: monster queue dropped — the night ended with %d of %d still waiting."),
			Day, PendingMonsterSpawns.Num() - NextPendingMonster, PendingMonsterSpawns.Num());
		PendingMonsterSpawns.Reset();
		NextPendingMonster = 0;
		return;
	}

	// 행이 없으면 남은 것을 이번 프레임에 다 — 밤은 규칙 행 없이 시작되지 않으므로 실제로는 오지 않는 길이다
	const UCL9RulesDef* Rules = GetRules();
	const int32 PerFrame = Rules ? Rules->MonsterSpawnPerFrame : PendingMonsterSpawns.Num();
	const int32 End = FMath::Min(NextPendingMonster + PerFrame, PendingMonsterSpawns.Num());
	for (; NextPendingMonster < End; ++NextPendingMonster)
	{
		const FCL9PendingMonsterSpawn& Pending = PendingMonsterSpawns[NextPendingMonster];
		if (ACL9Monster* Monster = SpawnMonster(Pending.Alias, NAME_None, Pending.Transform))
		{
			NightSpawned.Add(Monster);
			++PendingMonstersSpawned;
		}
	}

	if (NextPendingMonster < PendingMonsterSpawns.Num())
	{
		MonsterSpawnHandle = GetWorldTimerManager().SetTimerForNextTick(this, &ACL9GameMode::SpawnPendingMonsters);
		return;
	}

	UE_LOG(LogCL9, Log, TEXT("Night %d: %d of %d queued monsters spawned, %d per frame, %.2fs."),
		Day, PendingMonstersSpawned, PendingMonsterSpawns.Num(), PerFrame, GetWorld()->GetRealTimeSeconds() - PendingMonstersBegan);
	PendingMonsterSpawns.Reset();
	NextPendingMonster = 0;
}

void ACL9GameMode::ArmMidSpawns(const UCL9SiteDef& Site, int32 Day)
{
	GetWorldTimerManager().ClearTimer(MidSpawnHandle);
	MidSpawnCount = 0;
	MidSpawnCap = 0;
	MidSpawnPool.Reset();
	MidSpawnDay = Day;

	// 판 중간 스폰은 생성된 맵(건물 안 Monster 지점)에서만 — 평지 레벨은 끈다 (docs/goal/loop-7e-mid-round-spawn.md)
	const UCL9RulesDef* Rules = GetRules();
	TArray<ACL9SiteMap*> SiteMaps;
	ACL9SiteMap::FindAll(GetWorld(), SiteMaps);
	SiteMaps.RemoveAll([](const ACL9SiteMap* SiteMap) { return SiteMap == nullptr || !SiteMap->GetLayout().bBuilt; });
	if (!HasAuthority() || Rules == nullptr || Rules->MidSpawnInterval <= 0.f || SiteMaps.Num() == 0)
	{
		UE_LOG(LogCL9, Log, TEXT("Night %d: mid-round spawns off (%s)."), Day,
			SiteMaps.Num() == 0 ? TEXT("no generated map") : TEXT("MidSpawnInterval 0"));
		return;
	}

	// 종 풀 — 이 판의 시작 스폰 표(사이트 행 + 지어진 맵의 구역 행) 엔트리마다 무게 Min+Max. 같은 종이 여러 엔트리에 있으면 무게가 더해지는 셈이다
	auto AddEntries = [this](const TArray<FCL9SpawnEntry>& Entries)
	{
		for (const FCL9SpawnEntry& Entry : Entries)
		{
			const float Weight = static_cast<float>(Entry.MinCount + Entry.MaxCount);
			if (Weight > 0.f)
			{
				MidSpawnPool.Emplace(Entry.Alias, Weight);
			}
		}
	};
	AddEntries(Site.MonsterSpawns);
	for (const ACL9SiteMap* SiteMap : SiteMaps)
	{
		for (const FCL9SiteZone& Zone : SiteMap->GetZones())
		{
			AddEntries(Zone.MonsterSpawns);
		}
	}

	// 총량 — 판 시작 큐(일차 배율이 이미 들어간 수)에 비율. 큐는 다음 프레임부터 비워지므로 지금은 아직 다 들어 있다
	const int32 StartCount = PendingMonsterSpawns.Entries.Num();
	MidSpawnCap = FMath::RoundToInt(StartCount * Rules->MidSpawnCapRatio);
	if (MidSpawnCap <= 0 || MidSpawnPool.Num() == 0)
	{
		UE_LOG(LogCL9, Log, TEXT("Night %d: mid-round spawns off (cap %d from %d at start × %.2f, %d species entries)."),
			Day, MidSpawnCap, StartCount, Rules->MidSpawnCapRatio, MidSpawnPool.Num());
		return;
	}
	GetWorldTimerManager().SetTimer(MidSpawnHandle, this, &ACL9GameMode::MidSpawnTick, Rules->MidSpawnInterval, true);
	UE_LOG(LogCL9, Log, TEXT("Night %d: mid-round spawns armed — every %.0fs, cap %d (%d at start × %.2f), %d species entries."),
		Day, Rules->MidSpawnInterval, MidSpawnCap, StartCount, Rules->MidSpawnCapRatio, MidSpawnPool.Num());
}

void ACL9GameMode::MidSpawnTick()
{
	const ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	const UCL9RulesDef* Rules = GetRules();
	if (!HasAuthority() || CL9GameState == nullptr || Rules == nullptr || CL9GameState->GetPhase() != ECL9NightPhase::Night
		|| MidSpawnCount >= MidSpawnCap || MidSpawnPool.Num() == 0)
	{
		GetWorldTimerManager().ClearTimer(MidSpawnHandle);
		return;
	}

	// 종 — 풀의 무게 비율로 하나
	float Total = 0.f;
	for (const TPair<FName, float>& Entry : MidSpawnPool)
	{
		Total += Entry.Value;
	}
	float Roll = FMath::FRandRange(0.f, Total);
	FName Alias = MidSpawnPool.Last().Key;
	for (const TPair<FName, float>& Entry : MidSpawnPool)
	{
		if (Roll < Entry.Value)
		{
			Alias = Entry.Key;
			break;
		}
		Roll -= Entry.Value;
	}
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9MonsterDef* Def = Registry ? Registry->FindMonster(Alias) : nullptr;
	const float Radius = Def ? Def->Radius : 50.f;

	// 살아 있는 사람들 — 그 누구의 눈에서도 자리가 보이면 안 되고, 누구에게서든 최소 거리 밖이어야 한다
	TArray<const ACL9Character*> Living;
	for (const APlayerState* PlayerState : CL9GameState->PlayerArray)
	{
		const ACL9Character* Character = PlayerState ? Cast<ACL9Character>(PlayerState->GetPawn()) : nullptr;
		if (Character != nullptr && !Character->IsDead())
		{
			Living.Add(Character);
		}
	}
	if (Living.IsEmpty())
	{
		return;
	}

	// 후보 — 지어진 맵들의 Monster 지점(마당은 7b 로 지점이 없다)을 무작위 순서로, 많아야 64개만 본다(트레이스 수 상한)
	TArray<TPair<const ACL9SiteMap*, int32>> Candidates;
	TArray<ACL9SiteMap*> SiteMaps;
	ACL9SiteMap::FindAll(GetWorld(), SiteMaps);
	for (const ACL9SiteMap* SiteMap : SiteMaps)
	{
		if (SiteMap == nullptr || !SiteMap->GetLayout().bBuilt)
		{
			continue;
		}
		TArray<int32> Points;
		CL9SiteLayout::PointsOfPurpose(SiteMap->GetLayout(), ECL9PointPurpose::Monster, Points);
		for (const int32 Point : Points)
		{
			Candidates.Emplace(SiteMap, Point);
		}
	}
	for (int32 Index = Candidates.Num() - 1; Index > 0; --Index)
	{
		Candidates.Swap(Index, FMath::RandRange(0, Index));
	}
	constexpr int32 MaxChecks = 64;
	const int32 Checks = FMath::Min(Candidates.Num(), MaxChecks);
	for (int32 Index = 0; Index < Checks; ++Index)
	{
		const ACL9SiteMap* SiteMap = Candidates[Index].Key;
		const FVector Spot = SiteMap->PointToWorld(SiteMap->GetPoints()[Candidates[Index].Value]) + FVector(0.f, 0.f, 5.f + Radius);
		float Nearest = TNumericLimits<float>::Max();
		bool bAccepted = true;
		for (const ACL9Character* Character : Living)
		{
			const float Distance = FVector::Dist(Character->GetActorLocation(), Spot);
			Nearest = FMath::Min(Nearest, Distance);
			FHitResult Block;
			// 지형이 눈과 자리 사이를 막지 않으면 보이는 자리다(바라보는 방향과 무관 — 돌아보면 보이는 곳도 안 된다)
			if (Distance < Rules->MidSpawnMinDistance
				|| !ACL9Monster::TraceWorld(GetWorld(), Character, Character->GetPawnViewLocation(), Spot, Block, nullptr))
			{
				bAccepted = false;
				break;
			}
		}
		if (!bAccepted)
		{
			continue;
		}

		ACL9Monster* Monster = SpawnMonster(Alias, NAME_None, FTransform(FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f), Spot));
		if (!IsValid(Monster))
		{
			return;
		}
		NightSpawned.Add(Monster);
		++MidSpawnCount;
		UE_LOG(LogCL9, Log, TEXT("Night %d: mid-round spawn %d/%d — %s at %s, nearest player %.0f cm, hidden from %d player(s)."),
			MidSpawnDay, MidSpawnCount, MidSpawnCap, *Alias.ToString(), *Spot.ToCompactString(), Nearest, Living.Num());
		if (MidSpawnCount >= MidSpawnCap)
		{
			GetWorldTimerManager().ClearTimer(MidSpawnHandle);
			UE_LOG(LogCL9, Log, TEXT("Night %d: mid-round spawn cap %d reached — no more this round."), MidSpawnDay, MidSpawnCap);
		}
		return;
	}
	UE_LOG(LogCL9, Log, TEXT("Night %d: mid-round spawn skipped — no hidden spot %.0f cm from everyone among %d checked (%s)."),
		MidSpawnDay, Rules->MidSpawnMinDistance, Checks, *Alias.ToString());
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

	// 아직 스폰하지 않은 밤 몬스터 큐도 밤과 함께 버린다
	GetWorldTimerManager().ClearTimer(MonsterSpawnHandle);
	PendingMonsterSpawns.Reset();
	NextPendingMonster = 0;

	// 처형 중·처형된 캐릭터를 몬스터가 사라지기 **전에** 풀어 준다 — 뷰 타깃이 곧 파괴될 몬스터를 가리키거나 ExecutedBy 가 죽은
	// 참조로 남지 않게 (D9). 처형과 무관한 캐릭터에는 아무 일도 없다
	for (TActorIterator<ACL9Character> It(GetWorld()); It; ++It)
	{
		It->HostEndExecuted();
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

FName ACL9GameMode::ResolveSiteAlias() const
{
	// PIE 월드는 패키지 이름에 UEDPIE_N_ 접두가 붙는다 — 떼어 내야 표의 레벨 경로와 맞는다
	const FString LevelPackage = UWorld::RemovePIEPrefix(GetWorld()->GetOutermost()->GetName());
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	if (const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr)
	{
		for (const UCL9SiteDef* Site : Registry->GetSites())
		{
			if (Site != nullptr && !Site->Level.IsNull() && Site->Level.ToSoftObjectPath().GetLongPackageName() == LevelPackage)
			{
				UE_LOG(LogCL9, Log, TEXT("Stage: level %s plays site '%s' (MapTable Level column)."), *LevelPackage, *Site->Alias.ToString());
				return Site->Alias;
			}
		}
	}
	UE_LOG(LogCL9, Log, TEXT("Stage: no site row names level %s — using the game mode's DefaultSiteAlias '%s'."), *LevelPackage, *DefaultSiteAlias.ToString());
	return DefaultSiteAlias;
}

void ACL9GameMode::StartPlay()
{
	Super::StartPlay();

	// 레벨을 건너온 판(7i) — 떠나기 전에 저장한 판을 여기서 되돌린다. 표시는 한 번만 쓴다
	UCL9GameInstance* CarryInstance = GetGameInstance<UCL9GameInstance>();
	const bool bResume = CarryInstance != nullptr && CarryInstance->ConsumeRunCarry();

	// 회사(7i) — 판이 열리지 않는 레벨이다: 부지도 판도 없이 트럭·레버만 세우고, 이어 온 판을 되돌린 뒤 이번 기간을 정산한다.
	// 정산 화면은 각 머신의 HUD 가 복제된 지난 정산에서 파생해 연다 — 클라이언트는 이동 뒤 다시 접속하므로 여기서 RPC 로 열면 놓친다
	if (IsCompanyLevel())
	{
		ActiveSiteAlias = NAME_None;
		SpawnTruck();
		SpawnDepartLever();
		if (bResume)
		{
			LoadRun(1, false);
		}
		UE_LOG(LogCL9, Log, TEXT("Company: arrived from '%s'%s — settling the period."),
			CarryInstance != nullptr ? *CarryInstance->GetRunCarryFromSite().ToString() : TEXT("?"), bResume ? TEXT(" with the run") : TEXT(" (no carried run)"));
		Settle();
		return;
	}

	// 오늘 밤의 미로는 첫 로비에서부터 세워진다 — 정차 지점이 밤이 향할 곳을 보여준다
	ActiveSiteAlias = ResolveSiteAlias();
	// 이어 온 판은 미로보다 먼저 되돌린다 — 일차가 부지 크기를 정한다(docs/plan/data/module.md 결정 5). 장비는 패드 옆에 선다
	if (bResume)
	{
		LoadRun(1, false);
	}
	if (ACL9GameState* CL9GameState = GetGameState<ACL9GameState>())
	{
		CL9GameState->SetSiteAndDanger(ActiveSiteAlias, 1.f);
		CL9GameState->SetSiteMap(RollMapSeed(ForcedMapSeed));
	}

	SpawnTruckCounter();

	// 첫 판의 주문 — OfferOrders 가 끝에서 시스템이 고른 주문을 바로 확정한다(K1). 이후 판은 EnterLobby 에서 뽑는다.
	// 레벨을 건너 이어 온 판(7i)은 떠나기 전 로비에서 이미 받은 주문을 저장과 함께 들고 왔다(위에서 되돌림) — 다시 뽑지 않는다
	if (!bResume)
	{
		OfferOrders();
	}

	bArrivalPending = true;
}

void ACL9GameMode::SpawnTruckCounter()
{
	const UCL9RulesDef* Rules = GetRules();
	const ACL9Truck* Truck = GetTruck();
	if (!HasAuthority() || Rules == nullptr || Truck == nullptr)
	{
		return;
	}
	if (Rules->TruckCounterClass.IsNull())
	{
		UE_LOG(LogCL9, Log, TEXT("Truck counter: CL9Rules.TruckCounterClass is empty — no delivery window on the truck."));
		return;
	}
	// 시험 레벨처럼 창구를 손으로 놓아 둔 레벨에는 하나만 — 두 창구가 재고를 나눠 갖지 않게
	for (TActorIterator<ACL9Counter> It(GetWorld()); It; ++It)
	{
		UE_LOG(LogCL9, Log, TEXT("Truck counter: '%s' is already placed in this level — none on the truck."), *GetNameSafe(*It));
		return;
	}
	UClass* CounterClass = Rules->TruckCounterClass.LoadSynchronous();
	if (CounterClass == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("Truck counter: class '%s' did not load."), *Rules->TruckCounterClass.ToString());
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FTransform CounterTransform = Rules->TruckCounterOffset * Truck->GetActorTransform();
	const ACL9Counter* Counter = GetWorld()->SpawnActor<ACL9Counter>(CounterClass, CounterTransform, SpawnParams);
	if (Counter == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("Truck counter: spawn failed beside '%s'."), *GetNameSafe(Truck));
		return;
	}
	UE_LOG(LogCL9, Log, TEXT("Truck counter: '%s' spawned at %s beside '%s'."), *GetNameSafe(Counter),
		*Counter->GetActorLocation().ToCompactString(), *GetNameSafe(Truck));
}


void ACL9GameMode::ApplyWipePenalty()
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	const UCL9RulesDef* Rules = GetRules();
	if (!HasAuthority() || CL9GameState == nullptr || Rules == nullptr || FindLivingPawn(nullptr, nullptr) != nullptr)
	{
		return;
	}

	// 판의 누적 이더만 깎는다 — 정산 판정은 기간 수입이라 건드리지 않는다(docs/goal/loop-7a-wipe-penalty.md)
	const int32 Before = CL9GameState->GetSharedMoney();
	FCL9WipePenalty Penalty;
	Penalty.bActive = true;
	Penalty.Taken = CL9GameState->Deduct(Rules->WipePenaltyFor(Before));
	UE_LOG(LogCL9, Log, TEXT("Wipe penalty: %.0f%% of %d ether — %d taken. Money %d -> %d."),
		Rules->WipePenaltyRate * 100.f, Before, Penalty.Taken, Before, CL9GameState->GetSharedMoney());
	CL9GameState->SetWipePenalty(Penalty);
}

void ACL9GameMode::GrantRoundWallet()
{
	const ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	const UCL9RulesDef* Rules = GetRules();
	if (!HasAuthority() || CL9GameState == nullptr || Rules == nullptr)
	{
		return;
	}

	// 바깥 재화는 판 수입에 비례한다(2026-10-12 사용자) — 판 수입은 코인이 판 동안 늘어난 만큼이고, 살아서 트럭으로 돌아온 사람만 받는다.
	// 모두에게 한 번씩 보낸다(죽었으면 0) — 각 머신의 "이번 판 지급"이 늘 이번 판 값이 되게
	const int32 RoundIncome = FMath::Max(CL9GameState->GetPeriodIncome() - NightIncomeStart, 0);
	const int32 Reward = Rules->WalletIncomeFor(RoundIncome);
	for (APlayerState* PlayerState : CL9GameState->PlayerArray)
	{
		ACL9PlayerController* Controller = PlayerState ? Cast<ACL9PlayerController>(PlayerState->GetPlayerController()) : nullptr;
		const ACL9Character* Character = PlayerState ? Cast<ACL9Character>(PlayerState->GetPawn()) : nullptr;
		if (Controller == nullptr)
		{
			continue;
		}
		const bool bSurvived = Character != nullptr && !Character->IsDead();
		int32 Amount = bSurvived ? Reward : 0;
		// 임포스터 보상(7g)은 같은 한 번의 지급에 접는다 — 이긴 임포스터는 귀가 보상 대신 성공 보상, 임포스터를 죽인 사람은 처치 보상을 더 받는다
		FString Extra;
		if (PlayerState == ImposterWinner.Get())
		{
			Extra += FString::Printf(TEXT(" — imposter won: %d instead of %d"), Rules->ImposterWinReward, Amount);
			Amount = Rules->ImposterWinReward;
		}
		if (PlayerState == ImposterKiller.Get())
		{
			Amount += Rules->ImposterKillReward;
			Extra += FString::Printf(TEXT(" — +%d for killing the imposter"), Rules->ImposterKillReward);
		}
		if (PlayerState == RoundImposter.Get())
		{
			// 유도 보상(7h) — 임포스터가 깨운 몬스터에게 죽은 사람 수만큼(이겼든 졌든)
			int32 Induced = 0;
			for (const APlayerState* Other : CL9GameState->PlayerArray)
			{
				const ACL9PlayerState* OtherState = Cast<ACL9PlayerState>(Other);
				Induced += (OtherState != nullptr && OtherState != PlayerState && OtherState->InducedByPlayer.Get() == PlayerState) ? 1 : 0;
			}
			if (Induced > 0)
			{
				Amount += Induced * Rules->ImposterAssistReward;
				Extra += FString::Printf(TEXT(" — +%d × %d induced death(s)"), Rules->ImposterAssistReward, Induced);
			}
		}
		Controller->Client_WalletGrant(Amount);
		UE_LOG(LogCL9, Log, TEXT("Wallet grant: '%s' %s — round income %d coins × %.4f -> %d ether%s."),
			*PlayerState->GetPlayerName(), bSurvived ? TEXT("came home") : TEXT("did not survive"), RoundIncome, Rules->WalletIncomeRate, Amount, *Extra);
	}
}

void ACL9GameMode::AssignImposter()
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	const UCL9RulesDef* Rules = GetRules();
	if (!HasAuthority() || CL9GameState == nullptr || Rules == nullptr)
	{
		return;
	}

	// 남아 있는 역할을 먼저 거둔다(판정 없이 끝난 판 — 판 종료 박자 등). 후보는 판을 시작할 때 살아 있는 사람이다
	TArray<ACL9PlayerState*> Candidates;
	for (APlayerState* PlayerState : CL9GameState->PlayerArray)
	{
		ACL9PlayerState* CL9PlayerState = Cast<ACL9PlayerState>(PlayerState);
		if (CL9PlayerState == nullptr)
		{
			continue;
		}
		CL9PlayerState->HostSetImposter(false);
		const ACL9Character* Character = Cast<ACL9Character>(CL9PlayerState->GetPawn());
		if (Character != nullptr && !Character->IsDead())
		{
			Candidates.Add(CL9PlayerState);
		}
	}

	const float Chance = Rules->ImposterChanceFor(Candidates.Num());
	ACL9PlayerState* Picked = nullptr;
	FString How;
	if (ForcedImposterIndex != NoForcedImposter)
	{
		// cl9.imposter 의 지정은 한 판만 쓴다 — -1 은 "이번엔 없음"
		Picked = CL9GameState->PlayerArray.IsValidIndex(ForcedImposterIndex) ? Cast<ACL9PlayerState>(CL9GameState->PlayerArray[ForcedImposterIndex]) : nullptr;
		if (Picked != nullptr && !Candidates.Contains(Picked))
		{
			Picked = nullptr;
		}
		How = FString::Printf(TEXT("forced by cl9.imposter %d"), ForcedImposterIndex);
		ForcedImposterIndex = NoForcedImposter;
	}
	else
	{
		How = FString::Printf(TEXT("chance %.0f%% for %d player(s)"), Chance * 100.f, Candidates.Num());
		if (Candidates.Num() > 0 && FMath::FRand() < Chance)
		{
			Picked = Candidates[FMath::RandRange(0, Candidates.Num() - 1)];
		}
	}

	// 이 로그는 호스트 머신에만 남는다 — 다른 사람에게는 본인 머신의 알림 말고는 아무것도 가지 않는다
	if (Picked != nullptr)
	{
		Picked->HostSetImposter(true);
		UE_LOG(LogCL9, Log, TEXT("Imposter: '%s' this round (%s)."), *Picked->GetPlayerName(), *How);
	}
	else
	{
		UE_LOG(LogCL9, Log, TEXT("Imposter: none this round (%s)."), *How);
	}
}

ACL9PlayerState* ACL9GameMode::FindImposter() const
{
	const AGameStateBase* State = GameState;
	if (State == nullptr)
	{
		return nullptr;
	}
	for (APlayerState* PlayerState : State->PlayerArray)
	{
		ACL9PlayerState* CL9PlayerState = Cast<ACL9PlayerState>(PlayerState);
		if (CL9PlayerState != nullptr && CL9PlayerState->IsImposter())
		{
			return CL9PlayerState;
		}
	}
	return nullptr;
}

void ACL9GameMode::JudgeImposter()
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	ACL9PlayerState* Imposter = FindImposter();
	if (!HasAuthority() || CL9GameState == nullptr || Imposter == nullptr)
	{
		return;   // 임포스터가 없던 판 — 결과는 판 시작에 비워 둔 그대로다
	}

	FCL9ImposterResult Result;
	Result.bActive = true;
	Result.ImposterName = Imposter->GetPlayerName();
	FString Reason;

	RoundImposter = Imposter;
	const ACL9Character* ImposterCharacter = Cast<ACL9Character>(Imposter->GetPawn());
	if (ImposterCharacter == nullptr || ImposterCharacter->IsDead())
	{
		// 죽은 임포스터는 진다 — 플레이어가 죽였으면 그 사람이 처치 보상을 받는다(사용자 결정 4). 그를 죽인 몬스터를 깨운 사람도
		// 처치로 친다(7h, 2026-10-12 사용자 "임포스터 처치에도 인정")
		ACL9PlayerState* Killer = Imposter->KilledByPlayer != nullptr ? Imposter->KilledByPlayer.Get() : Imposter->InducedByPlayer.Get();
		if (Killer != nullptr)
		{
			Result.Outcome = ECL9ImposterOutcome::Killed;
			Result.KillerName = Killer->GetPlayerName();
			ImposterKiller = Killer;
			Reason = Imposter->KilledByPlayer != nullptr ? TEXT("struck") : TEXT("woke the monster");
		}
		else
		{
			Reason = TEXT("dead (not by a player)");
		}
	}
	else
	{
		int32 Others = 0;
		bool bSomeoneAlive = false;
		FString StruckName;
		for (APlayerState* PlayerState : CL9GameState->PlayerArray)
		{
			const ACL9PlayerState* Other = Cast<ACL9PlayerState>(PlayerState);
			if (Other == nullptr || Other == Imposter)
			{
				continue;
			}
			++Others;
			const ACL9Character* OtherCharacter = Cast<ACL9Character>(Other->GetPawn());
			bSomeoneAlive |= OtherCharacter != nullptr && !OtherCharacter->IsDead();
			if (Other->KilledByPlayer == Imposter)
			{
				StruckName = Other->GetPlayerName();
			}
		}

		// 트럭이 없는 레벨은 남겨짐 경계가 없다(LeaveBehindOutsideTruck 과 같다) — 안에 있는 것으로 본다
		const ACL9Truck* Truck = GetTruck();
		if (!StruckName.IsEmpty())
		{
			// 직접 때려 죽이면 무조건 실패(결정 3)
			Reason = FString::Printf(TEXT("struck '%s' directly"), *StruckName);
		}
		else if (Others == 0)
		{
			Reason = TEXT("nobody else was in the round");
		}
		else if (bSomeoneAlive)
		{
			Reason = TEXT("someone else is still alive (left behind does not count)");
		}
		else if (Truck != nullptr && !Truck->IsInside(ImposterCharacter->GetActorLocation()))
		{
			Reason = TEXT("not inside the truck");
		}
		else
		{
			Result.Outcome = ECL9ImposterOutcome::Won;
			ImposterWinner = Imposter;
		}
	}

	if (Result.Outcome == ECL9ImposterOutcome::Won)
	{
		UE_LOG(LogCL9, Log, TEXT("Imposter: '%s' won — everyone else is down and the imposter left in the truck."), *Result.ImposterName);
	}
	else if (Result.Outcome == ECL9ImposterOutcome::Killed)
	{
		UE_LOG(LogCL9, Log, TEXT("Imposter: '%s' was killed by '%s' (%s)."), *Result.ImposterName, *Result.KillerName, *Reason);
	}
	else
	{
		UE_LOG(LogCL9, Log, TEXT("Imposter: '%s' failed — %s."), *Result.ImposterName, *Reason);
	}

	// 새벽 화면이 모두에게 공개한다. 역할은 여기서 거둔다 — 다음 판에 같은 사람이 다시 뽑혀도 알림이 다시 뜨게
	CL9GameState->SetImposterResult(Result);
	Imposter->HostSetImposter(false);
}

bool ACL9GameMode::HasCompany() const
{
	const UCL9RulesDef* Rules = GetRules();
	return Rules != nullptr && !Rules->CompanyLevel.IsNull();
}

bool ACL9GameMode::IsCompanyLevel() const
{
	const UCL9RulesDef* Rules = GetRules();
	return Rules != nullptr && Rules->IsCompanyWorld(GetWorld());
}

bool ACL9GameMode::IsCompanyDue() const
{
	const ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	const UCL9ProgressionDef* Progression = GetProgression();
	return HasCompany() && !IsCompanyLevel() && CL9GameState != nullptr && Progression != nullptr
		&& CL9GameState->GetPhase() == ECL9NightPhase::Lobby && Progression->IsSettlementDay(CL9GameState->GetCurrentDay());
}

bool ACL9GameMode::TravelForRun(const TSoftObjectPtr<UWorld>& Level, FName FromSite, const TCHAR* Why)
{
	UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	if (!HasAuthority() || GameInstance == nullptr || Level.IsNull() || bTravelling)
	{
		return false;
	}
	// 판은 저장으로 건넌다 — 일차·코인·기간 수입·받은 주문·장비(세계에 있는 장비 전부)
	if (!SaveRun(1))
	{
		return false;
	}
	GameInstance->SetRunCarry(FromSite);

	FString URL = Level.ToSoftObjectPath().GetLongPackageName();
#if !UE_BUILD_SHIPPING
	// 리슨으로 다시 열어야 클라이언트가 따라온다 — HostGame 과 같은 규칙(Shipping 에는 리슨이 없다)
	URL += TEXT("?listen");
#endif
	bTravelling = true;
	UE_LOG(LogCL9, Log, TEXT("Travel (%s): '%s' — run saved, %d player(s) follow."), Why, *URL, GetNumPlayers());
	// 다음 틱에 떠난다 — 레버·치트의 호출 스택 안에서 월드를 내리지 않는다(세션 API 와 같은 규칙). 클라이언트에게 가는 ClientTravel 도
	// 그 틱에 나간다
	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this, URL]()
	{
		GetWorld()->ServerTravel(URL, false);
	}));
	return true;
}

FName ACL9GameMode::GetCompanyDestination() const
{
	for (TActorIterator<ACL9DestinationBoard> It(GetWorld()); It; ++It)
	{
		if (!It->GetDestination().IsNone())
		{
			return It->GetDestination();
		}
	}
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const TArray<FName> Sites = GameInstance != nullptr ? GameInstance->GetHostableSites() : TArray<FName>();
	const FName From = GameInstance != nullptr ? GameInstance->GetRunCarryFromSite() : NAME_None;
	return Sites.Contains(From) ? From : (Sites.Num() > 0 ? Sites[0] : NAME_None);
}

void ACL9GameMode::PublishContributions()
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	if (!HasAuthority() || CL9GameState == nullptr)
	{
		return;
	}

	TArray<FCL9Contribution> List;
	for (APlayerState* PlayerState : CL9GameState->PlayerArray)
	{
		const ACL9PlayerState* Victim = Cast<ACL9PlayerState>(PlayerState);
		if (Victim == nullptr || Victim->InducedByPlayer == nullptr || !Victim->IsDead())
		{
			continue;
		}
		FCL9Contribution& Entry = List.AddDefaulted_GetRef();
		Entry.VictimName = Victim->GetPlayerName();
		Entry.WakerName = Victim->InducedByPlayer->GetPlayerName();
		Entry.MonsterName = Victim->InducedByMonsterName;
		UE_LOG(LogCL9, Log, TEXT("Contribution: '%s' died to a monster woken by '%s'."), *Entry.VictimName, *Entry.WakerName);
	}
	CL9GameState->SetContributions(List);
}

void ACL9GameMode::CheatSetImposter(int32 Index)
{
	ACL9GameState* CL9GameState = GetGameState<ACL9GameState>();
	if (!HasAuthority() || CL9GameState == nullptr)
	{
		return;
	}

	if (CL9GameState->GetPhase() != ECL9NightPhase::Night)
	{
		ForcedImposterIndex = Index < 0 ? -1 : Index;
		UE_LOG(LogCL9, Log, TEXT("cl9.imposter: next round %s."), ForcedImposterIndex < 0
			? TEXT("has no imposter") : *FString::Printf(TEXT("makes player %d the imposter"), ForcedImposterIndex));
		return;
	}

	ACL9PlayerState* Target = (Index >= 0 && CL9GameState->PlayerArray.IsValidIndex(Index)) ? Cast<ACL9PlayerState>(CL9GameState->PlayerArray[Index]) : nullptr;
	for (APlayerState* PlayerState : CL9GameState->PlayerArray)
	{
		ACL9PlayerState* CL9PlayerState = Cast<ACL9PlayerState>(PlayerState);
		if (CL9PlayerState != nullptr && CL9PlayerState != Target)
		{
			CL9PlayerState->HostSetImposter(false);
		}
	}
	if (Target != nullptr)
	{
		Target->HostSetImposter(true);
	}
	UE_LOG(LogCL9, Log, TEXT("cl9.imposter: %s."), Target != nullptr
		? *FString::Printf(TEXT("'%s' is the imposter now"), *Target->GetPlayerName()) : TEXT("no imposter this round"));
}

void ACL9GameMode::LogImposter() const
{
	const ACL9PlayerState* Imposter = FindImposter();
	UE_LOG(LogCL9, Log, TEXT("cl9.imposter: now %s, next round %s."),
		Imposter != nullptr ? *FString::Printf(TEXT("'%s'"), *Imposter->GetPlayerName()) : TEXT("none"),
		ForcedImposterIndex == NoForcedImposter ? TEXT("rolled") : *FString::Printf(TEXT("forced to %d"), ForcedImposterIndex));
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
	Save->StageStars = CL9GameState->GetStageStars();

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

bool ACL9GameMode::LoadRun(int32 Slot, bool bOfferOrders)
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
	// 스테이지 별(7l) — 판과 함께 건너온 목적지별 별. 표에서 사라진 사이트는 버린다
	TArray<FCL9SiteStars> Stars;
	for (const FCL9SiteStars& Entry : Save->StageStars)
	{
		if (Registry != nullptr && Registry->FindSite(Entry.Site) == nullptr)
		{
			UE_LOG(LogCL9, Warning, TEXT("Load: site '%s' has no row any more — its stars are dropped."), *Entry.Site.ToString());
			continue;
		}
		Stars.Add(Entry);
	}
	CL9GameState->SetStageStars(Stars);

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

	// 제시는 저장하지 않는다 — 불러온 일차 기준으로 다시 뽑는다. 레벨을 건너 이어 온 판은 그날 주문을 이미 들고 있다(7i)
	if (bOfferOrders)
	{
		OfferOrders();
	}
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

	// 스테이지 별(7l) — 이 기간의 목적지에 별을 매기고, 별이 모자라지 않으면 다음 목적지를 열거나(표 순서) 마지막이면 클리어.
	// 미달 판정·패널티는 아래에서 옛대로 돈다
	const UCL9RulesDef* StageRules = GetRules();
	bool bCleared = false;
	if (StageRules != nullptr && StageRules->bStageStars)
	{
		const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
		const TArray<FName> Order = GameInstance != nullptr ? GameInstance->GetHostableSites() : TArray<FName>();
		// 회사에서는 트럭이 떠나온 사이트, 회사가 없으면 이 레벨의 사이트가 이 기간의 목적지다
		const FName Site = IsCompanyLevel() ? (GameInstance != nullptr ? GameInstance->GetRunCarryFromSite() : NAME_None) : ActiveSiteAlias;
		const int32 Index = Order.IndexOfByKey(Site);
		if (Index == INDEX_NONE)
		{
			UE_LOG(LogCL9, Warning, TEXT("Stage: the period's site '%s' is not a hostable site — no stars this settlement."), *Site.ToString());
		}
		else
		{
			const int32 Stars = StageRules->StageStarsFor(Earned, Target);
			const FName Next = Order.IsValidIndex(Index + 1) ? Order[Index + 1] : NAME_None;
			const bool bNextWasOpen = !Next.IsNone() && CL9GameState->IsStageOpen(Next, Order, StageRules->StageUnlockStars);
			CL9GameState->RecordSiteStars(Site, Stars);
			const bool bEnough = Stars >= StageRules->StageUnlockStars;
			bCleared = bEnough && Next.IsNone();
			const FName Opened = (bEnough && !bNextWasOpen && !Next.IsNone()) ? Next : NAME_None;
			CL9GameState->SetSettlementStage(Stars, Site, Opened, bCleared);
			UE_LOG(LogCL9, Log, TEXT("Stage: '%s' settled with %d star(s) (%d / %d)%s%s."), *Site.ToString(), Stars, Earned, Target,
				Opened.IsNone() ? TEXT("") : *FString::Printf(TEXT(" — '%s' opens"), *Opened.ToString()),
				bCleared ? TEXT(" — the last destination: the run is cleared") : TEXT(""));
		}
	}

	// **상태를 되돌리기 전에** 알린다 — 통과하든 미달하든 기간 수입이 0이 되므로,
	// 이 뒤에는 새벽 화면이 보여줄 숫자를 어디서도 구할 수 없다
	CL9GameState->Multicast_Settled(Day, Earned, Target, bPassed);

	if (bCleared)
	{
		// 클리어(7l) — 판이 끝난다. 화면은 게임 오버와 같은 자리(HUD 가 판 종료 상태에서 파생), 레버는 판 종료면 막힌다
		CL9GameState->ResetPeriodIncome();
		CL9GameState->SetRunCleared(Day, RecordBestDay(Day));
		return true;
	}

	if (!bPassed)
	{
		// 미달은 판을 끝내지 않는다(2026-10-12 사용자 "게임 오버 대신 패널티 후 계속", 7i) — 판의 코인에서 깎고 다음 기간으로 이어 간다.
		// 1일차로 되돌리는 StartNewRun 은 `cl9.settle force` 에만 남는다
		const UCL9RulesDef* Rules = GetRules();
		const int32 Before = CL9GameState->GetSharedMoney();
		FCL9WipePenalty Penalty;
		Penalty.bActive = true;
		Penalty.Taken = Rules != nullptr ? CL9GameState->Deduct(Rules->QuotaMissPenaltyFor(Before)) : 0;
		CL9GameState->SetQuotaPenalty(Penalty);
		CL9GameState->ResetPeriodIncome();
		UE_LOG(LogCL9, Log, TEXT("Settlement failed on day %d: earned %d < %d — %d coins taken, the run goes on. Money %d -> %d."),
			Day, Earned, Target, Penalty.Taken, Before, CL9GameState->GetSharedMoney());
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
	CL9GameState->SetStageStars(TArray<FCL9SiteStars>());
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
