// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CL9DefRegistry.generated.h"

class UCL9BuffDef;
class UCL9CharacterDef;
class UCL9DefBase;
class UCL9EquipmentDef;
class UCL9ItemDef;
class UCL9ModuleDef;
class UCL9MonsterDef;
class UCL9OrderDef;
class UCL9PieceDef;
class UCL9ProgressionDef;
class UCL9RulesDef;
class UCL9SegKindDef;
class UCL9SiteDef;

/**
 * 데이터 종류마다 테이블 애셋 하나씩, 엑셀 방식이다 (2026-09-02 결정):
 * 애셋을 열고 +로 행을 추가한 뒤 모든 컬럼을 그 자리에서 편집한다. 행은
 * 인라인 UCL9DefBase 오브젝트라서 Instanced 기믹이 계속 동작한다 —
 * UDataTable을 채택하지 않은 이유가 이것이다 (docs/plan/data/README.md).
 */
UCLASS()
class UCL9DefTable : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 행 목록 — 종류를 섞어도 동작은 하지만, 종류별로 테이블을 나누는 편이 읽기 쉽다 */
	UPROPERTY(EditAnywhere, Instanced, Category="Data", meta=(TitleProperty="Alias"))
	TArray<TObjectPtr<UCL9DefBase>> Rows;
};

/**
 * 로드해야 할 모든 def 테이블(또는 독립 행)을 나열하는 유일한 애셋이다. C++는
 * 애셋 경로를 절대 하드코딩하지 않고, 대신 레지스트리가 이 목록을 순회한다
 * (docs/plan/data/README.md).
 *
 * 목록은 스스로 채워진다 — 이 애셋을 저장하면 자신의 폴더(와 하위 폴더)를
 * 다시 스캔하므로, 새 테이블은 그 밑에 두기만 하면 등록된다. 런타임은
 * 여전히 저장된 목록을 읽는데, 패키징 빌드에서 쿠킹이 보장되는 것은 바로
 * 이 명시적 참조들 덕분이다.
 */
UCLASS()
class UCL9DefList : public UDataAsset
{
	GENERATED_BODY()

public:
	/** UCL9DefTable 애셋이거나 독립된 UCL9DefBase 애셋 */
	UPROPERTY(EditAnywhere, Category="Data")
	TArray<TSoftObjectPtr<UDataAsset>> Defs;

#if WITH_EDITOR
	/** 이 애셋의 폴더를 다시 스캔해 def 테이블을 찾고 목록을 재구성한다 */
	UFUNCTION(CallInEditor, Category="Data")
	void RefreshFromFolder();

	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif
};

/**
 * 게임 인스턴스가 소유하는 로컬 테이블 조회기다. 모든 머신이 같은 애셋을
 * 로드하고 같은 로드 검사를 실행하며, 오직 런타임 상태만 리플리케이트된다
 * (conventions § 5). 검증에 실패한 행은 Find*로 절대 나가지 않는다 —
 * 조용히 섞여 들어가는 대신 로그를 남기고 제외된다 (docs/data/schema.md).
 */
UCLASS()
class UCL9DefRegistry : public UObject
{
	GENERATED_BODY()

public:
	/** 목록의 모든 행을 로드하고 검증한다. null을 넘겨도 안전하다 (로그만 남기고 조회 결과를 비워둔다) */
	void Init(const UCL9DefList* DefList);

	/** 키에 해당하는 사용 가능한 캐릭터 행을 반환한다. 없거나 깨졌으면 null */
	const UCL9CharacterDef* FindCharacter(FName Alias, FName SubAlias = NAME_None) const;

	/** 키에 해당하는 사용 가능한 아이템 행을 반환한다. 없거나 깨졌으면 null */
	const UCL9ItemDef* FindItem(FName Alias, FName SubAlias = NAME_None) const;

	/** 키에 해당하는 사용 가능한 몬스터 행을 반환한다. 없거나 깨졌으면 null */
	const UCL9MonsterDef* FindMonster(FName Alias, FName SubAlias = NAME_None) const;

	/** 전역 규칙 행 하나. 테이블에 사용 가능한 행이 없으면 null */
	const UCL9RulesDef* GetRules() const
	{
		return Rules;
	}

	/** 하루 진행도 행 하나 (docs/plan/data/progression.md). 로드된 게 없으면 null — 호출부는 null을 "스케일링 없음"으로 취급한다 */
	const UCL9ProgressionDef* GetProgression() const
	{
		return Progression;
	}

	/** 별칭에 해당하는 사용 가능한 사이트 행을 반환한다. 없거나 깨졌으면 null */
	const UCL9SiteDef* FindSite(FName Alias) const;

	/** 목록 순서대로 정렬된, 사용 가능한 사이트 행들 */
	const TArray<TObjectPtr<const UCL9SiteDef>>& GetSites() const
	{
		return OrderedSites;
	}

	/** 별칭에 해당하는 사용 가능한 주문 행을 반환한다. 없거나 깨졌으면 null */
	const UCL9OrderDef* FindOrder(FName Alias) const;

	/** 목록 순서대로 정렬된, 사용 가능한 주문 행들 — 로비 뽑기가 이걸 날짜로 걸러낸다 (docs/systems/order.md) */
	const TArray<TObjectPtr<const UCL9OrderDef>>& GetOrders() const
	{
		return OrderedOrders;
	}

	/** 별칭에 해당하는 사용 가능한 장비 행을 반환한다. 없거나 깨졌으면 null (docs/plan/data/equipment.md) */
	const UCL9EquipmentDef* FindEquipment(FName Alias) const;

	/** 목록 순서대로 정렬된, 사용 가능한 장비 행들 — 상점 목록이 이 순서로 뜬다 (docs/systems/shop.md) */
	const TArray<TObjectPtr<const UCL9EquipmentDef>>& GetEquipment() const
	{
		return OrderedEquipment;
	}

	/** 별칭에 해당하는 사용 가능한 맵 조각 행을 반환한다. 없거나 깨졌으면 null (docs/plan/data/segkind.md) */
	const UCL9SegKindDef* FindSegKind(FName Alias) const;

	/**
	 * 한 테마에서 그 역할이 쓸 조각 행을 반환한다 (docs/systems/asset-swap.md).
	 * 테마가 그 역할을 갖추지 않았으면 테마 칸이 빈 공용 행으로 떨어지고, 그것도
	 * 없으면 null이다.
	 */
	const UCL9PieceDef* FindPiece(FName Role, FName Theme = NAME_None) const;

	/** 목록 순서대로 정렬된, 사용 가능한 조각 행들 */
	const TArray<TObjectPtr<const UCL9PieceDef>>& GetPieces() const
	{
		return OrderedPieces;
	}

	/** 키에 해당하는 사용 가능한 맵 모듈 행을 반환한다. 없거나 깨졌으면 null (docs/plan/data/module.md) */
	const UCL9ModuleDef* FindModule(FName Alias, FName Stage = NAME_None) const;

	/** 별칭에 해당하는 사용 가능한 버프 행을 반환한다. 없거나 깨졌으면 null (docs/plan/data/buff.md) */
	const UCL9BuffDef* FindBuff(FName Alias) const;

	/**
	 * 한 스테이지가 쓸 모듈 행들을 목록 순서대로 모은다 (module.md). 스테이지는 모듈
	 * 행의 SubAlias이며 사이트의 Theme와 같은 이름을 쓴다 — 조각 표와 같은 규약이다.
	 * 배치기가 이 배열을 그대로 받으므로 순서가 곧 결정성의 일부다.
	 */
	void GatherModules(FName Stage, TArray<const UCL9ModuleDef*>& OutModules) const;

	/** 목록 순서대로 정렬된, 사용 가능한 맵 조각 행들 */
	const TArray<TObjectPtr<const UCL9SegKindDef>>& GetSegKinds() const
	{
		return OrderedSegKinds;
	}

	/** 목록 순서대로 정렬된, 사용 가능한 아이템 행들 — 치트 숫자 인자가 이 목록을 인덱싱한다 (docs/systems/cheats.md) */
	const TArray<TObjectPtr<const UCL9ItemDef>>& GetItems() const
	{
		return OrderedItems;
	}

	/** 목록 순서대로 정렬된, 사용 가능한 캐릭터 행들 */
	const TArray<TObjectPtr<const UCL9CharacterDef>>& GetCharacters() const
	{
		return OrderedCharacters;
	}

	/** 목록 순서대로 정렬된, 사용 가능한 몬스터 행들 */
	const TArray<TObjectPtr<const UCL9MonsterDef>>& GetMonsters() const
	{
		return OrderedMonsters;
	}

	/** cl9.list용으로, 깨진 행과 그 사유들 (schema.md EDefStatus) */
	const TArray<FString>& GetBrokenRows() const
	{
		return BrokenRows;
	}

private:
	/** 행 하나를 검증하고 색인한다 — 테이블 행과 독립 애셋이 공유해서 쓴다 */
	void RegisterDef(const UCL9DefBase* Def);

	/** 모든 몬스터 행을 상대로 캐릭터 행 하나씩 A1/A2 관계를 검사한다 (docs/plan/economy.md) */
	void CheckCharacterRelations(const UCL9CharacterDef& Row) const;

	/** 행들 사이의 참조 무결성(요리 사슬, 드롭)을 검사한다 — 모든 행이 로드된 뒤 실행된다 */
	void CheckItemReferences() const;

	/** 사이트 스폰 항목은 실제로 로드된 행을 가리켜야 한다 — 그렇지 않으면 오타 하나로 아무것도 스폰되지 않는데도 티가 안 난다 */
	void CheckSiteReferences() const;

	/** 주문 재료는 반드시 아이템 행(alias+sub)이어야 한다 — 아무도 채울 수 없는 주문은 깨진 행이다 */
	void CheckOrderReferences() const;

	/** 장비의 ItemDef는 반드시 아이템 행이어야 한다 — 끊어져 있으면 사도 실물이 안 나온다 */
	void CheckEquipmentReferences() const;

	/** GC를 위해 사용 가능한 모든 행을 살려둔다. 조회용 맵들은 여기를 가리킨다 */
	UPROPERTY()
	TArray<TObjectPtr<const UCL9DefBase>> LoadedDefs;

	UPROPERTY()
	TArray<TObjectPtr<const UCL9ItemDef>> OrderedItems;

	UPROPERTY()
	TArray<TObjectPtr<const UCL9CharacterDef>> OrderedCharacters;

	UPROPERTY()
	TArray<TObjectPtr<const UCL9MonsterDef>> OrderedMonsters;

	UPROPERTY()
	TObjectPtr<const UCL9RulesDef> Rules;

	UPROPERTY()
	TObjectPtr<const UCL9ProgressionDef> Progression;

	UPROPERTY()
	TArray<TObjectPtr<const UCL9SiteDef>> OrderedSites;

	TMap<TPair<FName, FName>, const UCL9CharacterDef*> Characters;
	TMap<TPair<FName, FName>, const UCL9ItemDef*> Items;
	TMap<TPair<FName, FName>, const UCL9MonsterDef*> Monsters;
	TMap<FName, const UCL9SiteDef*> Sites;

	UPROPERTY()
	TArray<TObjectPtr<const UCL9OrderDef>> OrderedOrders;

	TMap<FName, const UCL9OrderDef*> Orders;

	UPROPERTY()
	TArray<TObjectPtr<const UCL9EquipmentDef>> OrderedEquipment;

	TMap<FName, const UCL9EquipmentDef*> Equipment;

	UPROPERTY()
	TArray<TObjectPtr<const UCL9SegKindDef>> OrderedSegKinds;

	TMap<FName, const UCL9SegKindDef*> SegKinds;

	UPROPERTY()
	TArray<TObjectPtr<const UCL9PieceDef>> OrderedPieces;

	TMap<TPair<FName, FName>, const UCL9PieceDef*> Pieces;

	UPROPERTY()
	TArray<TObjectPtr<const UCL9ModuleDef>> OrderedModules;

	TMap<TPair<FName, FName>, const UCL9ModuleDef*> Modules;

	UPROPERTY()
	TArray<TObjectPtr<const UCL9BuffDef>> OrderedBuffs;

	TMap<FName, const UCL9BuffDef*> Buffs;
	TArray<FString> BrokenRows;
};
