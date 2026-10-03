// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9DefRegistry.h"
#include "CL9.h"
#include "Data/CL9BuffDef.h"
#include "Data/CL9CharacterDef.h"
#include "Data/CL9EquipmentDef.h"
#include "Data/CL9ItemDef.h"
#include "Data/CL9ModuleDef.h"
#include "Data/CL9MonsterDef.h"
#include "Data/CL9OrderDef.h"
#include "Data/CL9ProgressionDef.h"
#include "Data/CL9RulesDef.h"
#include "Data/CL9PieceDef.h"
#include "Data/CL9SegKindDef.h"
#include "Data/CL9SiteDef.h"
#include "World/CL9SiteLayout.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "UObject/ObjectSaveContext.h"

void UCL9DefList::RefreshFromFolder()
{
	// 스캔 루트는 이 애셋이 있는 위치 그대로다 — 코드에 경로를 적지 않는다.
	// 행은 이 목록의 폴더 밑에 존재하기만 하면 등록된다.
	const FString RootPath = FPackageName::GetLongPackagePath(GetPackage()->GetName());

	IAssetRegistry& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry").Get();
	FARFilter Filter;
	Filter.PackagePaths.Add(*RootPath);
	Filter.bRecursivePaths = true;
	Filter.bRecursiveClasses = true;
	Filter.ClassPaths.Add(UCL9DefTable::StaticClass()->GetClassPathName());
	Filter.ClassPaths.Add(UCL9DefBase::StaticClass()->GetClassPathName());

	TArray<FAssetData> Assets;
	AssetRegistry.GetAssets(Filter, Assets);

	// 순서를 결정적으로 유지해야 재스캔 사이에도 cl9.list 번호가 안정적으로 유지된다
	Assets.Sort([](const FAssetData& A, const FAssetData& B)
	{
		return A.GetSoftObjectPath().ToString() < B.GetSoftObjectPath().ToString();
	});

	Defs.Reset();
	for (const FAssetData& Asset : Assets)
	{
		Defs.Emplace(Asset.GetSoftObjectPath());
	}

	UE_LOG(LogCL9, Log, TEXT("%s: registered %d def assets under %s."), *GetName(), Defs.Num(), *RootPath);
}

void UCL9DefList::PreSave(FObjectPreSaveContext SaveContext)
{
	// 에디터에서 저장할 때마다 다시 스캔하므로 목록이 낡은 채로 남지 않는다.
	// 쿠킹 시에는 저장된 내용을 그대로 유지한다.
	if (!SaveContext.IsCooking())
	{
		RefreshFromFolder();
	}
	Super::PreSave(SaveContext);
}
#endif

void UCL9DefRegistry::Init(const UCL9DefList* DefList)
{
	if (DefList == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("Def registry got no def list — every table lookup will fail (docs/data/schema.md)."));
		return;
	}

	for (const TSoftObjectPtr<UDataAsset>& DefRef : DefList->Defs)
	{
		const UDataAsset* Entry = DefRef.LoadSynchronous();
		if (Entry == nullptr)
		{
			UE_LOG(LogCL9, Error, TEXT("Def list entry '%s' does not resolve — skipped."), *DefRef.ToString());
			BrokenRows.Add(FString::Printf(TEXT("%s ✗ (unresolved reference)"), *DefRef.ToString()));
			continue;
		}

		if (const UCL9DefTable* Table = Cast<UCL9DefTable>(Entry))
		{
			for (const UCL9DefBase* Row : Table->Rows)
			{
				if (Row == nullptr)
				{
					UE_LOG(LogCL9, Error, TEXT("%s: empty row entry — skipped."), *Table->GetName());
					continue;
				}
				RegisterDef(Row);
			}
		}
		else if (const UCL9DefBase* Def = Cast<UCL9DefBase>(Entry))
		{
			RegisterDef(Def);
		}
		else
		{
			UE_LOG(LogCL9, Error, TEXT("%s: neither a def table nor a def row — skipped."), *Entry->GetName());
		}
	}

	CheckItemReferences();
	CheckSiteReferences();
	CheckOrderReferences();
	CheckEquipmentReferences();

	// 관계 검사는 레지스트리 전체가 필요하므로 모든 테이블이 로드된 뒤에
	// 실행한다 — A1/A2는 캐릭터 행을 몬스터 행과 비교한다.
	for (const UCL9CharacterDef* Character : OrderedCharacters)
	{
		CheckCharacterRelations(*Character);
	}

	if (Rules == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("No usable rules row loaded — carry/throw rulings will refuse to run (docs/plan/data/rules.md)."));
	}
}

void UCL9DefRegistry::RegisterDef(const UCL9DefBase* Def)
{
	TArray<FString> Errors;
	Def->Validate(Errors);
	if (Errors.Num() > 0)
	{
		for (const FString& Error : Errors)
		{
			UE_LOG(LogCL9, Error, TEXT("%s: broken row — %s"), *Def->GetName(), *Error);
		}
		BrokenRows.Add(FString::Printf(TEXT("%s ✗ (%s)"), *Def->GetName(), *Errors[0]));
		return;
	}

	LoadedDefs.Add(Def);

	const TPair<FName, FName> Key(Def->Alias, Def->SubAlias);

	if (const UCL9CharacterDef* CharacterDef = Cast<UCL9CharacterDef>(Def))
	{
		if (Characters.Contains(Key))
		{
			UE_LOG(LogCL9, Error, TEXT("%s: duplicate character key '%s'/'%s' — the first row stays."),
				*Def->GetName(), *Def->Alias.ToString(), *Def->SubAlias.ToString());
			return;
		}
		Characters.Add(Key, CharacterDef);
		OrderedCharacters.Add(CharacterDef);
	}
	else if (const UCL9ItemDef* ItemDef = Cast<UCL9ItemDef>(Def))
	{
		if (Items.Contains(Key))
		{
			UE_LOG(LogCL9, Error, TEXT("%s: duplicate item key '%s'/'%s' — the first row stays."),
				*Def->GetName(), *Def->Alias.ToString(), *Def->SubAlias.ToString());
			return;
		}
		Items.Add(Key, ItemDef);
		OrderedItems.Add(ItemDef);
	}
	else if (const UCL9MonsterDef* MonsterDef = Cast<UCL9MonsterDef>(Def))
	{
		if (Monsters.Contains(Key))
		{
			UE_LOG(LogCL9, Error, TEXT("%s: duplicate monster key '%s'/'%s' — the first row stays."),
				*Def->GetName(), *Def->Alias.ToString(), *Def->SubAlias.ToString());
			return;
		}
		Monsters.Add(Key, MonsterDef);
		OrderedMonsters.Add(MonsterDef);
	}
	else if (const UCL9RulesDef* RulesDef = Cast<UCL9RulesDef>(Def))
	{
		if (Rules != nullptr)
		{
			UE_LOG(LogCL9, Error, TEXT("%s: a second rules row — the first one stays (the rules asset is singular)."), *Def->GetName());
			return;
		}
		Rules = RulesDef;
	}
	else if (const UCL9ProgressionDef* ProgressionDef = Cast<UCL9ProgressionDef>(Def))
	{
		if (Progression != nullptr)
		{
			UE_LOG(LogCL9, Error, TEXT("%s: a second progression row — the first one stays (the progression asset is singular)."), *Def->GetName());
			return;
		}
		Progression = ProgressionDef;
	}
	else if (const UCL9SiteDef* SiteDef = Cast<UCL9SiteDef>(Def))
	{
		if (Sites.Contains(Def->Alias))
		{
			UE_LOG(LogCL9, Error, TEXT("%s: duplicate site key '%s' — the first row stays."), *Def->GetName(), *Def->Alias.ToString());
			return;
		}
		Sites.Add(Def->Alias, SiteDef);
		OrderedSites.Add(SiteDef);
	}
	else if (const UCL9OrderDef* OrderDef = Cast<UCL9OrderDef>(Def))
	{
		if (Orders.Contains(Def->Alias))
		{
			UE_LOG(LogCL9, Error, TEXT("%s: duplicate order key '%s' — the first row stays."), *Def->GetName(), *Def->Alias.ToString());
			return;
		}
		Orders.Add(Def->Alias, OrderDef);
		OrderedOrders.Add(OrderDef);
	}
	else if (const UCL9EquipmentDef* EquipmentDef = Cast<UCL9EquipmentDef>(Def))
	{
		if (Equipment.Contains(Def->Alias))
		{
			UE_LOG(LogCL9, Error, TEXT("%s: duplicate equipment key '%s' — the first row stays."), *Def->GetName(), *Def->Alias.ToString());
			return;
		}
		Equipment.Add(Def->Alias, EquipmentDef);
		OrderedEquipment.Add(EquipmentDef);
	}
	else if (const UCL9SegKindDef* SegKindDef = Cast<UCL9SegKindDef>(Def))
	{
		if (SegKinds.Contains(Def->Alias))
		{
			UE_LOG(LogCL9, Error, TEXT("%s: duplicate seg kind key '%s' — the first row stays."), *Def->GetName(), *Def->Alias.ToString());
			return;
		}
		SegKinds.Add(Def->Alias, SegKindDef);
		OrderedSegKinds.Add(SegKindDef);
	}
	else if (const UCL9PieceDef* PieceDef = Cast<UCL9PieceDef>(Def))
	{
		if (Pieces.Contains(Key))
		{
			UE_LOG(LogCL9, Error, TEXT("%s: duplicate piece key '%s'/'%s' — the first row stays."),
				*Def->GetName(), *Def->Alias.ToString(), *Def->SubAlias.ToString());
			return;
		}
		Pieces.Add(Key, PieceDef);
		OrderedPieces.Add(PieceDef);
	}
	else if (const UCL9ModuleDef* ModuleDef = Cast<UCL9ModuleDef>(Def))
	{
		if (Modules.Contains(Key))
		{
			UE_LOG(LogCL9, Error, TEXT("%s: duplicate module key '%s'/'%s' — the first row stays."),
				*Def->GetName(), *Def->Alias.ToString(), *Def->SubAlias.ToString());
			return;
		}
		Modules.Add(Key, ModuleDef);
		OrderedModules.Add(ModuleDef);
	}
	else if (const UCL9BuffDef* BuffDef = Cast<UCL9BuffDef>(Def))
	{
		if (Buffs.Contains(Def->Alias))
		{
			UE_LOG(LogCL9, Error, TEXT("%s: duplicate buff key '%s' — the first row stays."), *Def->GetName(), *Def->Alias.ToString());
			return;
		}
		Buffs.Add(Def->Alias, BuffDef);
		OrderedBuffs.Add(BuffDef);
	}
}

const UCL9BuffDef* UCL9DefRegistry::FindBuff(FName Alias) const
{
	const UCL9BuffDef* const* Found = Buffs.Find(Alias);
	return Found ? *Found : nullptr;
}

const UCL9ModuleDef* UCL9DefRegistry::FindModule(FName Alias, FName Stage) const
{
	const UCL9ModuleDef* const* Found = Modules.Find(TPair<FName, FName>(Alias, Stage));
	return Found ? *Found : nullptr;
}

void UCL9DefRegistry::GatherModules(FName Stage, TArray<const UCL9ModuleDef*>& OutModules) const
{
	OutModules.Reset();
	for (const TObjectPtr<const UCL9ModuleDef>& Module : OrderedModules)
	{
		if (Module->SubAlias == Stage)
		{
			OutModules.Add(Module.Get());
		}
	}
}

const UCL9PieceDef* UCL9DefRegistry::FindPiece(FName Role, FName Theme) const
{
	if (const UCL9PieceDef* const* Found = Pieces.Find(TPair<FName, FName>(Role, Theme)))
	{
		return *Found;
	}
	// 테마가 그 역할을 갖추지 않았으면 테마 칸이 빈 공용 행으로 떨어진다 —
	// 회색 상자처럼 테마를 타지 않는 조각이 여기 산다.
	if (!Theme.IsNone())
	{
		if (const UCL9PieceDef* const* Shared = Pieces.Find(TPair<FName, FName>(Role, NAME_None)))
		{
			return *Shared;
		}
	}
	return nullptr;
}

const UCL9SegKindDef* UCL9DefRegistry::FindSegKind(FName Alias) const
{
	const UCL9SegKindDef* const* Found = SegKinds.Find(Alias);
	return Found ? *Found : nullptr;
}

const UCL9OrderDef* UCL9DefRegistry::FindOrder(FName Alias) const
{
	const UCL9OrderDef* const* Found = Orders.Find(Alias);
	return Found ? *Found : nullptr;
}

void UCL9DefRegistry::CheckOrderReferences() const
{
	for (const UCL9OrderDef* Order : OrderedOrders)
	{
		for (const FCL9ItemCount& Need : Order->Ingredients)
		{
			if (FindItem(Need.ItemAlias, Need.ItemSubAlias) == nullptr)
			{
				UE_LOG(LogCL9, Error, TEXT("%s: Ingredients names '%s'/'%s' which is missing or broken — nobody can fill this order."),
					*Order->GetName(), *Need.ItemAlias.ToString(), *Need.ItemSubAlias.ToString());
			}
		}
	}
}

const UCL9EquipmentDef* UCL9DefRegistry::FindEquipment(FName Alias) const
{
	const UCL9EquipmentDef* const* Found = Equipment.Find(Alias);
	return Found ? *Found : nullptr;
}

void UCL9DefRegistry::CheckEquipmentReferences() const
{
	for (const UCL9EquipmentDef* Equip : OrderedEquipment)
	{
		// 장비를 아이템 시스템에 연결하는 유일한 끈이다 — 끊어져 있으면 사도 실물이 안 나온다
		if (FindItem(Equip->ItemDef.ItemAlias, Equip->ItemDef.ItemSubAlias) == nullptr)
		{
			UE_LOG(LogCL9, Error, TEXT("%s: ItemDef names '%s'/'%s' which is missing or broken — buying it would spawn nothing."),
				*Equip->GetName(), *Equip->ItemDef.ItemAlias.ToString(), *Equip->ItemDef.ItemSubAlias.ToString());
		}
	}
}

const UCL9SiteDef* UCL9DefRegistry::FindSite(FName Alias) const
{
	const UCL9SiteDef* const* Found = Sites.Find(Alias);
	return Found ? *Found : nullptr;
}

void UCL9DefRegistry::CheckSiteReferences() const
{
	for (const UCL9SiteDef* Site : OrderedSites)
	{
		for (const FCL9SpawnEntry& Entry : Site->MonsterSpawns)
		{
			if (FindMonster(Entry.Alias) == nullptr)
			{
				UE_LOG(LogCL9, Error, TEXT("%s: MonsterSpawns names '%s' which is missing or broken — that entry spawns nothing."),
					*Site->GetName(), *Entry.Alias.ToString());
			}
		}
		for (const FCL9SpawnEntry& Entry : Site->ItemSpawns)
		{
			if (FindItem(Entry.Alias) == nullptr)
			{
				UE_LOG(LogCL9, Error, TEXT("%s: ItemSpawns names '%s' which is missing or broken — that entry spawns nothing."),
					*Site->GetName(), *Entry.Alias.ToString());
			}
		}

		if (Site->Generator == ECL9Generator::None)
		{
			continue;
		}

		// 생성기는 조각들을 종류로 지칭한다 — 행이 없으면 맵에 구멍이 생긴다.
		// 천장을 끈 사이트는 천장 조각을 아예 안 쓰므로 없어도 문제가 아니다.
		TArray<FName> Needed = {CL9SegKind::Wall(), CL9SegKind::Barn(), CL9SegKind::FlapIn(), CL9SegKind::Floor(),
			CL9SegKind::Yard(), CL9SegKind::Tread(), CL9SegKind::BarnStub(), CL9SegKind::WallStub()};
		if (Site->bBuildCeilings)
		{
			Needed.Add(CL9SegKind::Ceiling());
		}
		for (const FName& Kind : Needed)
		{
			const UCL9SegKindDef* SegKind = FindSegKind(Kind);
			if (SegKind == nullptr)
			{
				UE_LOG(LogCL9, Error, TEXT("%s: the module generator needs seg kind '%s' which is missing or broken — those boxes are skipped."),
					*Site->GetName(), *Kind.ToString());
				continue;
			}
			// 테마를 절반만 채워 두면 맵이 까맣게 나오고 원인을 찾기 어렵다 —
			// 여기서 미리 잡는다 (docs/systems/asset-swap.md 테마 완결성).
			if (FindPiece(SegKind->Piece, Site->Theme) == nullptr)
			{
				UE_LOG(LogCL9, Error, TEXT("%s: theme '%s' has no piece for role '%s' (needed by seg kind '%s') — those boxes are skipped."),
					*Site->GetName(), *Site->Theme.ToString(), *SegKind->Piece.ToString(), *Kind.ToString());
			}
		}

		// door는 없어도 된다 — 그러면 문 자리가 문틀 없는 개구부가 된다. 지나다니는 데는
		// 지장이 없으므로 에러가 아니라 알림이다 (docs/plan/data/module.md).
		if (FindSegKind(CL9SegKind::Door()) == nullptr)
		{
			UE_LOG(LogCL9, Log, TEXT("%s: no 'door' seg kind — doorways will be bare openings."), *Site->GetName());
		}

		// 모듈 세트는 통째로 한 벌이다 (docs/plan/data/module.md). 시작 모듈이 없으면
		// 한 발도 못 떼고, 나머지 타입이 비면 그 자리를 다른 타입이 메우지 못한다.
		TArray<const UCL9ModuleDef*> StageModules;
		GatherModules(Site->Theme, StageModules);
		if (StageModules.Num() == 0)
		{
			UE_LOG(LogCL9, Error, TEXT("%s: stage '%s' has no module rows — the generator builds nothing (docs/plan/data/module.md)."),
				*Site->GetName(), *Site->Theme.ToString());
			continue;
		}
		bool bSeenType[CL9ModuleTypeCount] = {};
		for (const UCL9ModuleDef* Module : StageModules)
		{
			bSeenType[static_cast<int32>(Module->Type)] = true;
		}
		if (!bSeenType[static_cast<int32>(ECL9ModuleType::Start)])
		{
			UE_LOG(LogCL9, Error, TEXT("%s: stage '%s' has no Start module — the truck has nothing to dock against."),
				*Site->GetName(), *Site->Theme.ToString());
		}
		for (const ECL9ModuleType Type : {ECL9ModuleType::Normal, ECL9ModuleType::Passage, ECL9ModuleType::DeadEnd})
		{
			if (!bSeenType[static_cast<int32>(Type)])
			{
				UE_LOG(LogCL9, Warning, TEXT("%s: stage '%s' has no %s module — that share of the budget goes unspent and the map comes out small."),
					*Site->GetName(), *Site->Theme.ToString(), *UEnum::GetValueAsString(Type));
			}
		}
	}
}

const UCL9CharacterDef* UCL9DefRegistry::FindCharacter(FName Alias, FName SubAlias) const
{
	const UCL9CharacterDef* const* Found = Characters.Find(TPair<FName, FName>(Alias, SubAlias));
	return Found ? *Found : nullptr;
}

const UCL9MonsterDef* UCL9DefRegistry::FindMonster(FName Alias, FName SubAlias) const
{
	const UCL9MonsterDef* const* Found = Monsters.Find(TPair<FName, FName>(Alias, SubAlias));
	return Found ? *Found : nullptr;
}

const UCL9ItemDef* UCL9DefRegistry::FindItem(FName Alias, FName SubAlias) const
{
	const UCL9ItemDef* const* Found = Items.Find(TPair<FName, FName>(Alias, SubAlias));
	return Found ? *Found : nullptr;
}

void UCL9DefRegistry::CheckCharacterRelations(const UCL9CharacterDef& Row) const
{
	// docs/plan/economy.md에서 유도한 값들 — 캐릭터 행마다 붙잡기가 가능한
	// 모든 몬스터 행을 상대로 검사한다. 여기서 조용히 깨지면 모든 걸
	// 내려놔도 도망칠 수 없는 플레이어가 생긴다는 뜻이다.
	const float CycleTime = Row.StaminaDrainTime + Row.StaminaRegenTime;
	const float SustainedTiredSpeed = (Row.RunSpeed * Row.StaminaDrainTime + Row.WalkSpeed * Row.StaminaRegenTime) / FMath::Max(CycleTime, UE_KINDA_SMALL_NUMBER);
	const float SprintBudget = Row.RunSpeed * Row.StaminaDrainTime;

	UE_LOG(LogCL9, Log, TEXT("%s: sustained tired speed %.1f cm/s (A1 left side), sprint budget %.0f cm (A2 left side)."),
		*Row.Alias.ToString(), SustainedTiredSpeed, SprintBudget);

	for (const UCL9MonsterDef* Monster : OrderedMonsters)
	{
		const bool bCatcher = Monster->CatchRadius > 0.f;
		if (bCatcher && SustainedTiredSpeed <= Monster->ChaseSpeed)
		{
			UE_LOG(LogCL9, Error, TEXT("A1 broken: %s sustained tired speed %.1f <= %s chase %.1f — dropping everything no longer escapes."),
				*Row.Alias.ToString(), SustainedTiredSpeed, *Monster->Alias.ToString(), Monster->ChaseSpeed);
		}
		if (SprintBudget <= Monster->SightIn)
		{
			UE_LOG(LogCL9, Error, TEXT("A2 broken: %s sprint budget %.0f <= %s sight %.0f — a straight corridor never breaks sight."),
				*Row.Alias.ToString(), SprintBudget, *Monster->Alias.ToString(), Monster->SightIn);
		}
	}
	// A4 (운반 질주 vs 추격)는 무게→감속 공식이 정해져야 검사할 수 있다 (economy.md 미결)
}

void UCL9DefRegistry::CheckItemReferences() const
{
	// 참조 무결성 검사 (docs/data/schema.md 로드 검사): 요리 사슬과 드롭은
	// 실제로 로드된 행을 가리켜야 한다.
	for (const UCL9ItemDef* Item : OrderedItems)
	{
		const FCL9ItemRef* Refs[] = { &Item->CookResult, &Item->BurnResult };
		for (const FCL9ItemRef* Ref : Refs)
		{
			if (Ref->IsSet() && FindItem(Ref->ItemAlias, Ref->ItemSubAlias) == nullptr)
			{
				UE_LOG(LogCL9, Error, TEXT("%s: cook chain points at '%s'/'%s' which is missing or broken."),
					*Item->GetName(), *Ref->ItemAlias.ToString(), *Ref->ItemSubAlias.ToString());
			}
		}

		// 먹기 효과는 버프 표를 가리킨다 — 오타를 여기서 잡지 않으면 "먹었는데 아무 일도
		// 없다"가 되고, 그건 화면에도 로그에도 안 남아서 못 찾는다 (docs/plan/data/buff.md)
		for (const FName& BuffAlias : Item->EatBuffs)
		{
			if (FindBuff(BuffAlias) == nullptr)
			{
				UE_LOG(LogCL9, Error, TEXT("%s: EatBuffs names '%s' which is missing or broken — eating it would do nothing."),
					*Item->GetName(), *BuffAlias.ToString());
			}
		}
	}

	for (const UCL9MonsterDef* Monster : OrderedMonsters)
	{
		for (const FDropItem& Drop : Monster->DropItems)
		{
			if (Drop.IsSet() && FindItem(Drop.ItemAlias, Drop.ItemSubAlias) == nullptr)
			{
				UE_LOG(LogCL9, Error, TEXT("%s: drop points at '%s'/'%s' which is missing or broken."),
					*Monster->GetName(), *Drop.ItemAlias.ToString(), *Drop.ItemSubAlias.ToString());
			}
		}
	}
}
