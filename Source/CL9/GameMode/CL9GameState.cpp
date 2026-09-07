// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9GameState.h"
#include "Net/UnrealNetwork.h"
#include "CL9.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9EquipmentDef.h"
#include "Data/CL9ItemDef.h"
#include "Data/CL9RulesDef.h"
#include "Data/CL9SiteDef.h"
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
		const float Floor = NightStartServerTime + Rules->DarkMin;
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

void ACL9GameState::StartNightClock(const UCL9RulesDef& Rules, float NightLength, bool bNoBlackout)
{
	if (!HasAuthority())
	{
		return;
	}

	NightStartServerTime = GetServerWorldTimeSeconds();
	NightEndServerTime = NightStartServerTime + NightLength;
	// 0이면 밤 내내 IsBlackout이 false로 유지된다; 그래도 ForceBlackout(cl9.blackout)으로 걸 수는 있다
	BlackoutServerTime = bNoBlackout ? 0.f : NightStartServerTime + Rules.DarkBase;
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
		ACL9SiteMap::Rebuild(World, *Site, *Registry, SiteMapKey.Seed);
	}
	else
	{
		ACL9SiteMap::Clear(World);
	}
}

const UCL9RulesDef* ACL9GameState::GetRules() const
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	return Registry ? Registry->GetRules() : nullptr;
}
