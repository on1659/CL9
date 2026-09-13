// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Data/CL9Types.h"
#include "CL9PlayerState.generated.h"

/** HUD에 보이는 값이 바뀌면 발동한다 — 로컬 HUD가 구독한다(docs/systems/ui_system.md) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCL9PlayerHudChanged);

/**
 * 플레이어별 판정 상태다(conventions § 4 — 캐릭터에는 절대 두지 않는다). Carry
 * 미러링은 docs/systems/inventory.md를 따른다: 핫바 칸 배열에서 활성 칸 하나만
 * 월드에 실물 액터로 들려 있고(그 액터가 진실, 칸은 미러), 나머지 칸은 데이터로만
 * 존재하다가 활성으로 고르면 다시 스폰된다.
 *
 * HUD 변경 훅: 값은 클라이언트에서 브로드캐스트하는 OnRep과 함께
 * 리플리케이트된다; 리슨 호스트에서는(OnRep이 절대 실행되지 않으므로)
 * 캐릭터가 각 호스트 측 기록 직후에 호출하는 Notify* 함수로 같은
 * 델리게이트를 발동한다.
 */
UCLASS()
class ACL9PlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ACL9PlayerState();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 비어 있는 첫 칸의 번호, 전부 찼으면 INDEX_NONE */
	UFUNCTION(BlueprintPure, Category="Carry")
	int32 FindEmptySlot() const;

	/** 그 칸에 아무것도 없으면 true — 범위 밖 번호도 빈 것으로 본다 */
	UFUNCTION(BlueprintPure, Category="Carry")
	bool IsSlotEmpty(int32 Slot) const;

	/** 어느 칸이든 하나라도 들어 있으면 true */
	bool HasAnyCarriedItem() const;

	/** 호스트 측: Hp를 기록한 뒤 발동한다(OnRep은 클라이언트를 처리할 뿐 권한자는 처리하지 않는다) */
	void NotifyHpChanged()
	{
		OnHpChanged.Broadcast();
	}

	/** 호스트 측: Slots·ActiveSlot·HeadItem을 기록한 뒤 발동한다 */
	void NotifyCarryChanged()
	{
		OnCarryChanged.Broadcast();
	}

	/** HP가 바뀌었다(굴려졌거나 데미지를 입었거나) — 로컬 HUD 갱신 */
	UPROPERTY(BlueprintAssignable, Category="HUD")
	FCL9PlayerHudChanged OnHpChanged;

	/** 핫바 칸·활성 칸·머리 내용물이 바뀌었다 — 로컬 HUD 갱신 */
	UPROPERTY(BlueprintAssignable, Category="HUD")
	FCL9PlayerHudChanged OnCarryChanged;

	/** 굴려서 소진됐다 — 굴리기 전의 0은 "아직 아님"이지 죽음이 아니다(docs/systems/character.md 죽음) */
	UFUNCTION(BlueprintPure, Category="Health")
	bool IsDead() const
	{
		return MaxHp > 0 && Hp <= 0;
	}

	/** 스폰 시 캐릭터 행의 HpMin..HpMax에서 한 번 굴려진다; 호스트만 기록한다. 0 = 사망 */
	UPROPERTY(ReplicatedUsing=OnRep_Hp, BlueprintReadOnly, Category="Health")
	int32 Hp = 0;

	/** Hp가 시작된 굴림값 — 트럭 패드는 이 값까지 회복시키고, 다음 로비는 이 값으로 부활시킨다 */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Health")
	int32 MaxHp = 0;

	/**
	 * 핫바 칸들 — 크기는 CL9Rules.HotbarSlots(호스트가 BeginPlay에서 맞춘다). 활성 칸은 캐릭터
	 * HeldItem 액터의 미러이고, 비활성 칸은 액터 없이 데이터(Alias·SubAlias·Value)만 든다.
	 * 빈 칸은 ItemAlias가 None이다.
	 */
	UPROPERTY(ReplicatedUsing=OnRep_Slots, BlueprintReadOnly, Category="Carry")
	TArray<FCL9CarriedItem> Slots;

	/** 지금 손에 들린 칸의 번호(0부터) — 빈 칸이면 빈손이다 */
	UPROPERTY(ReplicatedUsing=OnRep_ActiveSlot, BlueprintReadOnly, Category="Carry")
	uint8 ActiveSlot = 0;

	/** 머리에 얹혀 있는 것(Head carry 슬롯 — 헤드램프) — 착용된 아이템 액터의 미러(맨머리면 None) */
	UPROPERTY(ReplicatedUsing=OnRep_HeadItem, BlueprintReadOnly, Category="Carry")
	FCL9CarriedItem HeadItem;

protected:
	UFUNCTION()
	void OnRep_Hp();

	UFUNCTION()
	void OnRep_Slots();

	UFUNCTION()
	void OnRep_ActiveSlot();

	UFUNCTION()
	void OnRep_HeadItem();
};
