// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9GameState.h"
#include "Net/UnrealNetwork.h"
#include "CL9.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9EquipmentDef.h"
#include "Data/CL9ItemDef.h"
#include "Data/CL9RulesDef.h"
#include "Data/CL9SiteDef.h"
#include "Data/CL9WaveTable.h"
#include "FX/CL9WaveRenderer.h"
#include "GameMode/CL9GameInstance.h"
#include "EngineUtils.h"
#include "Items/CL9Item.h"
#include "World/CL9SiteMap.h"

void ACL9GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACL9GameState, SiteMapKey);

	DOREPLIFETIME(ACL9GameState, ReturnedItems);
	DOREPLIFETIME(ACL9GameState, Phase);
	DOREPLIFETIME(ACL9GameState, CurrentDay);
	DOREPLIFETIME(ACL9GameState, NightStartServerTime);
	DOREPLIFETIME(ACL9GameState, NightEndServerTime);
	DOREPLIFETIME(ACL9GameState, BlackoutServerTime);
	DOREPLIFETIME(ACL9GameState, CurrentSiteAlias);
	DOREPLIFETIME(ACL9GameState, DangerScale);
	DOREPLIFETIME(ACL9GameState, OfferedOrders);
	DOREPLIFETIME(ACL9GameState, OrdersRequired);
	DOREPLIFETIME(ACL9GameState, AcceptedOrders);
	DOREPLIFETIME(ACL9GameState, SharedMoney);
	DOREPLIFETIME(ACL9GameState, PeriodIncome);
	DOREPLIFETIME(ACL9GameState, LastSettlement);
	DOREPLIFETIME(ACL9GameState, StageStars);
	DOREPLIFETIME(ACL9GameState, WipePenalty);
	DOREPLIFETIME(ACL9GameState, ImposterResult);
	DOREPLIFETIME(ACL9GameState, Contributions);
	DOREPLIFETIME(ACL9GameState, QuotaPenalty);
	DOREPLIFETIME(ACL9GameState, ArrivalServerTime);
	DOREPLIFETIME(ACL9GameState, bArrivalThisRound);
	DOREPLIFETIME(ACL9GameState, ArrivalSeq);
}

void ACL9GameState::SetWipePenalty(const FCL9WipePenalty& Penalty)
{
	if (!HasAuthority())
	{
		return;
	}

	WipePenalty = Penalty;
	ForceNetUpdate();
	// 리슨 호스트에서는 OnRep 이 뜨지 않는다 — 로컬 훅을 직접 발동한다
	OnRep_WipePenalty();
}

void ACL9GameState::OnRep_WipePenalty()
{
	OnWipePenaltyChanged.Broadcast();
}

void ACL9GameState::SetImposterResult(const FCL9ImposterResult& Result)
{
	if (!HasAuthority())
	{
		return;
	}

	ImposterResult = Result;
	ForceNetUpdate();
	// 리슨 호스트에서는 OnRep 이 뜨지 않는다 — 로컬 훅을 직접 발동한다
	OnRep_ImposterResult();
}

void ACL9GameState::OnRep_ImposterResult()
{
	OnImposterResultChanged.Broadcast();
}

void ACL9GameState::SetContributions(const TArray<FCL9Contribution>& InContributions)
{
	if (!HasAuthority())
	{
		return;
	}

	Contributions = InContributions;
	ForceNetUpdate();
	// 리슨 호스트에서는 OnRep 이 뜨지 않는다 — 로컬 훅을 직접 발동한다
	OnRep_Contributions();
}

void ACL9GameState::OnRep_Contributions()
{
	OnContributionsChanged.Broadcast();
}

void ACL9GameState::SetQuotaPenalty(const FCL9WipePenalty& Penalty)
{
	if (!HasAuthority())
	{
		return;
	}

	QuotaPenalty = Penalty;
	ForceNetUpdate();
	// 리슨 호스트에서는 OnRep 이 뜨지 않는다 — 로컬 훅을 직접 발동한다
	OnRep_QuotaPenalty();
}

void ACL9GameState::OnRep_QuotaPenalty()
{
	OnQuotaPenaltyChanged.Broadcast();
}

void ACL9GameState::SetRunCleared(int32 Day, int32 BestDay)
{
	if (!HasAuthority())
	{
		return;
	}

	RunOver.bActive = true;
	RunOver.bCleared = true;
	RunOver.Day = Day;
	RunOver.BestDay = BestDay;
	RunOver.SiteStars = StageStars;
	ForceNetUpdate();
	UE_LOG(LogCL9, Log, TEXT("Run cleared — the last destination was settled on day %d (best day %d, %d destination(s))."),
		Day, BestDay, StageStars.Num());
}

int32 ACL9GameState::GetSiteStars(FName Site) const
{
	const FCL9SiteStars* Found = StageStars.FindByPredicate([Site](const FCL9SiteStars& Entry) { return Entry.Site == Site; });
	return Found != nullptr ? Found->Stars : -1;
}

void ACL9GameState::RecordSiteStars(FName Site, int32 Stars)
{
	if (!HasAuthority() || Site.IsNone())
	{
		return;
	}

	FCL9SiteStars* Found = StageStars.FindByPredicate([Site](const FCL9SiteStars& Entry) { return Entry.Site == Site; });
	if (Found == nullptr)
	{
		FCL9SiteStars& Entry = StageStars.AddDefaulted_GetRef();
		Entry.Site = Site;
		Entry.Stars = FMath::Max(Stars, 0);
	}
	else
	{
		Found->Stars = FMath::Max(Found->Stars, Stars);
	}
	ForceNetUpdate();
}

void ACL9GameState::SetStageStars(const TArray<FCL9SiteStars>& Stars)
{
	if (!HasAuthority())
	{
		return;
	}
	StageStars = Stars;
	ForceNetUpdate();
}

bool ACL9GameState::IsStageOpen(FName Site, const TArray<FName>& Order, int32 UnlockStars) const
{
	const int32 Index = Order.IndexOfByKey(Site);
	if (Index == INDEX_NONE)
	{
		return false;
	}
	return Index == 0 || GetSiteStars(Site) >= 0 || GetSiteStars(Order[Index - 1]) >= UnlockStars;
}

void ACL9GameState::SetSettlementStage(int32 Stars, FName Site, FName OpenedSite, bool bCleared)
{
	if (!HasAuthority())
	{
		return;
	}
	LastSettlement.Stars = Stars;
	LastSettlement.Site = Site;
	LastSettlement.OpenedSite = OpenedSite;
	LastSettlement.bCleared = bCleared;
}

void ACL9GameState::HostReportNoise(ECL9NoiseSource Source, const FVector& Location, float Radius, AActor* NoiseInstigator)
{
	if (!HasAuthority() || Source == ECL9NoiseSource::None)
	{
		return;
	}

	// 꺼진 행(bEnabled false 또는 로드 검사로 비활성)은 사건 자체를 내지 않는다 — 걷기처럼 늘 나는 소음이
	// 아무 그림도 없이 멀티캐스트로 오가지 않게 (2026-09-16 사용자). 몬스터 청각을 이 입구로 통일하면 다시 본다.
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9WaveTable* Table = GameInstance != nullptr ? GameInstance->GetWaveTable() : nullptr;
	if (Table == nullptr || !Table->IsEnabled(Source))
	{
		return;
	}

	// 호스트의 화면은 여기서 그린다 — 아래 방송은 리슨 호스트에서도 실행되지만 그쪽은 곧바로 빠져나간다
	// (그러지 않으면 클라이언트가 낸 소리가 호스트 화면에 두 번 핀다).
	if (UCL9WaveRenderer* Renderer = UCL9WaveRenderer::Get(this))
	{
		Renderer->PlayWave(Source, Location, NoiseInstigator, false);
	}

	Multicast_Noise(Source, Location, NoiseInstigator);
}

void ACL9GameState::Multicast_Noise_Implementation(ECL9NoiseSource Source, FVector_NetQuantize Location, AActor* NoiseInstigator)
{
	if (HasAuthority())
	{
		return;
	}

	UCL9WaveRenderer* Renderer = UCL9WaveRenderer::Get(this);
	if (Renderer == nullptr)
	{
		return;
	}

	// 내가 낸 소리는 서버 왕복을 기다리지 않고 이미 로컬에서 그렸다 (docs/systems/wave.md 소유 클라의 예측)
	if (Renderer->IsLocalInstigator(NoiseInstigator))
	{
		return;
	}

	Renderer->PlayWave(Source, Location, NoiseInstigator, false);
}

void ACL9GameState::AddIncome(int32 Amount)
{
	if (!HasAuthority() || Amount <= 0)
	{
		return;
	}

	SharedMoney += Amount;
	PeriodIncome += Amount;
	// 리슨 호스트에서는 OnRep이 절대 실행되지 않는다 — 로컬 훅을 직접 발동시킨다
	OnRep_Money();
}

int32 ACL9GameState::Deduct(int32 Amount)
{
	if (!HasAuthority() || Amount <= 0)
	{
		return 0;
	}

	// 채무는 없다 — 잔액이 모자라면 있는 만큼만 빠진다(docs/plan/economy.md 이더의 흐름)
	const int32 Taken = FMath::Min(Amount, SharedMoney);
	SharedMoney -= Taken;
	OnRep_Money();
	return Taken;
}

void ACL9GameState::RestoreRun(int32 Day, int32 Money, int32 Income, const TArray<FCL9AcceptedOrder>& Orders)
{
	if (!HasAuthority())
	{
		return;
	}

	CurrentDay = FMath::Max(1, Day);
	SharedMoney = FMath::Max(0, Money);
	PeriodIncome = FMath::Max(0, Income);
	AcceptedOrders = Orders;
	OnRep_Money();
	OnRep_Orders();
}

void ACL9GameState::ResetPeriodIncome()
{
	if (!HasAuthority())
	{
		return;
	}

	PeriodIncome = 0;
	OnRep_Money();
}

void ACL9GameState::Multicast_Settled_Implementation(int32 Day, int32 Earned, int32 Target, bool bPassed)
{
	// 호스트에서는 복제본을 여기서 채운다 — 클라이언트는 복제로 같은 값을 따로 받는다
	if (HasAuthority())
	{
		LastSettlement.Day = Day;
		LastSettlement.Earned = Earned;
		LastSettlement.Target = Target;
		LastSettlement.bPassed = bPassed;
	}
	OnSettled.Broadcast(Day, Earned, Target, bPassed);
}

void ACL9GameState::ClearLastSettlement()
{
	if (!HasAuthority())
	{
		return;
	}

	LastSettlement = FCL9SettlementResult();
}

ECL9BuyResult ACL9GameState::CanBuy(FName EquipAlias) const
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9EquipmentDef* Equip = Registry ? Registry->FindEquipment(EquipAlias) : nullptr;
	if (Equip == nullptr)
	{
		return ECL9BuyResult::NoRow;
	}

	// 트럭 패널은 정차 중에만 열린다 — 밤중에 장비를 불러낼 수는 없다
	if (Phase != ECL9NightPhase::Lobby)
	{
		return ECL9BuyResult::NotInLobby;
	}
	if (!Equip->IsUnlockedOn(CurrentDay))
	{
		return ECL9BuyResult::Locked;
	}
	if (Equip->MaxOwned > 0 && GetOwnedCount(EquipAlias) >= Equip->MaxOwned)
	{
		return ECL9BuyResult::MaxOwned;
	}
	if (SharedMoney < Equip->Price)
	{
		return ECL9BuyResult::NotEnoughMoney;
	}
	return ECL9BuyResult::Ok;
}

int32 ACL9GameState::GetOwnedCount(FName EquipAlias) const
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9EquipmentDef* Equip = Registry ? Registry->FindEquipment(EquipAlias) : nullptr;
	if (Equip == nullptr)
	{
		return 0;
	}

	// 보유 = 월드에 나와 있는 실물이다 — 장비는 두고 오면 잃으므로 세어 둔 숫자를 믿지 않는다.
	// 클라이언트에서는 복제된 실물을 세므로 스폰 복제가 늦으면 잠깐 어긋날 수 있다(표시 전용).
	int32 Count = 0;
	for (TActorIterator<ACL9Item> It(GetWorld()); It; ++It)
	{
		if (It->GetAlias() == Equip->ItemDef.ItemAlias && It->GetSubAlias() == Equip->ItemDef.ItemSubAlias)
		{
			++Count;
		}
	}
	return Count;
}

void ACL9GameState::OnRep_Money()
{
	OnMoneyChanged.Broadcast();
}

void ACL9GameState::SetOfferedOrders(const TArray<FName>& Aliases, int32 Required)
{
	if (!HasAuthority())
	{
		return;
	}

	OfferedOrders = Aliases;
	OrdersRequired = Required;
	// 리슨 호스트에서는 OnRep이 절대 실행되지 않는다 — 로컬 훅을 직접 발동시킨다
	OnRep_Orders();
}

void ACL9GameState::SetAcceptedOrders(const TArray<FCL9AcceptedOrder>& Orders)
{
	if (!HasAuthority())
	{
		return;
	}

	AcceptedOrders = Orders;
	OnRep_Orders();
}

void ACL9GameState::OnRep_Orders()
{
	OnOrdersChanged.Broadcast();
}

void ACL9GameState::RecordReturn(FName ItemAlias, FName ItemSubAlias)
{
	if (!HasAuthority())
	{
		return;
	}

	FCL9CarriedItem& Returned = ReturnedItems.AddDefaulted_GetRef();
	Returned.ItemAlias = ItemAlias;
	Returned.ItemSubAlias = ItemSubAlias;

	// 반납할 때마다 정전 시각이 앞당겨지되, 밤 시작으로부터 DarkMin 지난 시점 아래로는
	// 내려가지 않는다(docs/plan/data/rules.md 밤 — "반납이 시계를 앞당긴다").
	const UCL9RulesDef* Rules = GetRules();
	if (Rules != nullptr && Phase == ECL9NightPhase::Night && BlackoutServerTime > 0.f)
	{
		const float Floor = NightStartServerTime + NightDarkMin;
		BlackoutServerTime = FMath::Max(Floor, BlackoutServerTime - Rules->DarkAccel);
	}

	UE_LOG(LogCL9, Log, TEXT("Jar return #%d: '%s'/'%s'."), ReturnedItems.Num(), *ItemAlias.ToString(), *ItemSubAlias.ToString());
}

void ACL9GameState::SetPhase(ECL9NightPhase NewPhase)
{
	if (!HasAuthority() || Phase == NewPhase)
	{
		return;
	}

	Phase = NewPhase;
	// 리슨 호스트에서는 OnRep이 절대 실행되지 않는다 — 로컬 훅을 직접 발동시킨다.
	OnRep_Phase();
}

void ACL9GameState::StartNightClock(float NightLength, float DarkBase, float DarkMin, bool bNoBlackout)
{
	if (!HasAuthority())
	{
		return;
	}

	NightStartServerTime = GetServerWorldTimeSeconds();
	NightEndServerTime = NightStartServerTime + NightLength;
	// 0이면 밤 내내 IsBlackout이 false로 유지된다; 그래도 ForceBlackout(cl9.blackout)으로 걸 수는 있다
	BlackoutServerTime = bNoBlackout ? 0.f : NightStartServerTime + DarkBase;
	NightDarkMin = DarkMin;
	ArrivalServerTime = 0.f;
	bArrivalThisRound = false;
}

void ACL9GameState::SetArrival()
{
	if (!HasAuthority())
	{
		return;
	}
	ArrivalServerTime = NightStartServerTime;
	bArrivalThisRound = true;
	++ArrivalSeq;
	ForceNetUpdate();
}

float ACL9GameState::GetArrivalAlpha() const
{
	const UCL9RulesDef* Rules = GetRules();
	if (!bArrivalThisRound || Phase != ECL9NightPhase::Night || Rules == nullptr || Rules->ArrivalTime <= 0.f)
	{
		return -1.f;
	}
	return FMath::Max(0.f, static_cast<float>(GetServerWorldTimeSeconds()) - ArrivalServerTime) / Rules->ArrivalTime;
}

void ACL9GameState::SetDay(int32 NewDay)
{
	if (!HasAuthority() || NewDay < 1)
	{
		return;
	}

	CurrentDay = NewDay;
}

void ACL9GameState::SetSiteAndDanger(FName SiteAlias, float NewDangerScale)
{
	if (!HasAuthority())
	{
		return;
	}

	CurrentSiteAlias = SiteAlias;
	DangerScale = FMath::Max(NewDangerScale, UE_KINDA_SMALL_NUMBER);
}

void ACL9GameState::ClearNightClock()
{
	if (!HasAuthority())
	{
		return;
	}

	NightStartServerTime = 0.f;
	NightEndServerTime = 0.f;
	BlackoutServerTime = 0.f;
	ArrivalServerTime = 0.f;
	bArrivalThisRound = false;
	ReturnedItems.Reset();
}

void ACL9GameState::AdvanceDay()
{
	if (!HasAuthority())
	{
		return;
	}

	++CurrentDay;
}

void ACL9GameState::ForceBlackout()
{
	if (!HasAuthority() || Phase != ECL9NightPhase::Night)
	{
		return;
	}

	BlackoutServerTime = GetServerWorldTimeSeconds();
	UE_LOG(LogCL9, Log, TEXT("Blackout forced (cl9.blackout)."));
}

float ACL9GameState::GetNightRemaining() const
{
	if (Phase != ECL9NightPhase::Night || NightEndServerTime <= 0.f)
	{
		return 0.f;
	}
	return FMath::Max(0.f, NightEndServerTime - GetServerWorldTimeSeconds());
}

bool ACL9GameState::IsBlackout() const
{
	return Phase == ECL9NightPhase::Night
		&& BlackoutServerTime > 0.f
		&& GetServerWorldTimeSeconds() >= BlackoutServerTime;
}

void ACL9GameState::OnRep_Phase()
{
	OnNightPhaseChanged.Broadcast(Phase);
}

void ACL9GameState::SetSiteMap(uint32 Seed)
{
	if (!HasAuthority())
	{
		return;
	}

	SiteMapKey.Seed = Seed;
	++SiteMapKey.Build;
	// 리슨 호스트에서는 OnRep이 절대 실행되지 않는다 — 바로 로컬에서 빌드한다
	OnRep_SiteMap();
}

void ACL9GameState::OnRep_SiteMap()
{
	RebuildSiteMap();
}

void ACL9GameState::RebuildSiteMap()
{
	UWorld* World = GetWorld();
	const UCL9GameInstance* GameInstance = World ? World->GetGameInstance<UCL9GameInstance>() : nullptr;
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9SiteDef* Site = Registry ? Registry->FindSite(CurrentSiteAlias) : nullptr;
	if (Site != nullptr && Site->Generator != ECL9Generator::None)
	{
		// 구역이 있는 사이트면 구역마다 한 판 — 시드는 이 하나에서 구역별 하위 시드로 갈라지고, 첫 구역만
		// 지금 짓고 나머지는 프레임마다 하나씩 선다(ACL9SiteMap::IsBuildPending). 호스트·클라이언트 같은 길이다.
		ACL9SiteMap::Rebuild(World, *Site, *Registry, SiteMapKey.Seed, CurrentDay);
	}
	else
	{
		// 구역 액터까지 전부 걷는다
		ACL9SiteMap::Clear(World);
	}
}

const UCL9RulesDef* ACL9GameState::GetRules() const
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	return Registry ? Registry->GetRules() : nullptr;
}
