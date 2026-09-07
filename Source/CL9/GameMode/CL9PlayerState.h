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
 * 미러링은 docs/systems/inventory.md를 따른다: 월드에 든 액터가 진실이고,
 * HandItem은 UI와 판정을 위해 그것을 미러링한다; 주머니에 넣은 아이템은
 * 데이터로만 존재하고 꺼낼 때 다시 스폰된다.
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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool IsHandFull() const
	{
		return HandItem.IsSet();
	}

	/** 손↔주머니 교체 잠금이 아직 유효한 동안 true(서버 월드 시각 기준) */
	bool IsSwapLocked(float ServerTimeNow) const
	{
		return ServerTimeNow < SwapLockUntil;
	}

	/** 호스트 측: Hp를 기록한 뒤 발동한다(OnRep은 클라이언트를 처리할 뿐 권한자는 처리하지 않는다) */
	void NotifyHpChanged()
	{
		OnHpChanged.Broadcast();
	}

	/** 호스트 측: HandItem이나 PocketItems를 기록한 뒤 발동한다 */
	void NotifyCarryChanged()
	{
		OnCarryChanged.Broadcast();
	}

	/** HP가 바뀌었다(굴려졌거나 데미지를 입었거나) — 로컬 HUD 갱신 */
	UPROPERTY(BlueprintAssignable, Category="HUD")
	FCL9PlayerHudChanged OnHpChanged;

	/** 손이나 주머니 내용물이 바뀌었다 — 로컬 HUD 갱신 */
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

	/** 손에 들고 있는 것 — 부착된 아이템 액터의 미러(비어 있으면 None) */
	UPROPERTY(ReplicatedUsing=OnRep_HandItem, BlueprintReadOnly, Category="Carry")
	FCL9CarriedItem HandItem;

	/** 주머니에 넣은 아이템들, 최신이 마지막 — 데이터로만 존재하며 액터는 사라졌다(Pocketable 행만 해당) */
	UPROPERTY(ReplicatedUsing=OnRep_PocketItems, BlueprintReadOnly, Category="Carry")
	TArray<FCL9CarriedItem> PocketItems;

	/** 머리에 얹혀 있는 것(Head carry 슬롯 — 헤드램프) — 착용된 아이템 액터의 미러(맨머리면 None) */
	UPROPERTY(ReplicatedUsing=OnRep_HeadItem, BlueprintReadOnly, Category="Carry")
	FCL9CarriedItem HeadItem;

	/** 이 서버 월드 시각까지 교체/줍기/던지기 입력이 거부된다(docs/systems/input.md) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category="Carry")
	float SwapLockUntil = 0.f;

protected:
	UFUNCTION()
	void OnRep_Hp();

	UFUNCTION()
	void OnRep_HandItem();

	UFUNCTION()
	void OnRep_PocketItems();

	UFUNCTION()
	void OnRep_HeadItem();
};
