// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/CL9Types.h"
#include "Templates/SubclassOf.h"
#include "CL9HUDWidget.generated.h"

class ACL9Character;
class ACL9GameState;
class ACL9Item;
class ACL9PlayerState;
class UBorder;
class UCL9ItemDef;
class UCL9ScanLabelWidget;
class UProgressBar;
class UTextBlock;

/**
 * 게임 중 항상 떠 있는 HUD 레이어 콘텐츠(docs/systems/ui.md 게임 중 HUD): HP, 소지
 * 슬롯, 밤 시계, 상호작용 프롬프트, 던지기 차징, 그리고 윤곽선이 그려진 아이템 위의
 * 월드 고정 라벨. 값은 PlayerState/GameState/Character의 변경 델리게이트를 통해
 * 들어온다 — 복제된 상태를 매 틱 폴링하지 않는다(docs/systems/ui_system.md 데이터 연결).
 * WBP_Main의 디자이너에 한 번만 배치된다.
 */
UCLASS(Abstract)
class UCL9HUDWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/**
	 * 스태미나는 변경 이벤트가 없고(CMC 안에서 시뮬레이션됨) 던지기 차징은 버튼을 누르고
	 * 있는 동안의 시간 함수라서 — 둘 다 로컬 값이고 둘 다 여기서 읽는다. 아이템 라벨의
	 * 화면 위치도 여기서 액터를 추적한다(상태가 아니라 표시일 뿐이라서).
	 */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** HP 1당 ♥ 하나(초안 — ui.md의 "HP 표시 방식 미정" 자리), 10 초과는 숫자 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HpText;

	/** 손에 든 것 — PlayerState.HandItem을 그대로 반영 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HandText;

	/** 품에 넣은 아이템 — 개수와 이름 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> PocketText;

	/** 날짜, 페이즈, 흘러가는 밤 시계 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ClockText;

	/** 정규화된 스태미나(2026-09-05 결정 — 이 바는 화면 흔들림을 대체하지 않고 함께 간다) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;

	/** 화면 하단 중앙: 지금 E를 누르면 무슨 일이 일어나는지 — 캐릭터가 이 문구를 구성한다(docs/systems/input.md 상호작용 공통 틀) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> PromptText;

	/** 던지기 차징 0~1, 호스트 시계를 로컬에서 예측한 값(input.md 던지기 차징); 대기 중에는 숨김 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> ChargeBar;

	/** 윤곽선이 그려진 아이템 위의 월드 고정 라벨 — 이름 · 가격 · 소지 정보(2026-09-05 사용자 요청). 매 프레임 화면에 투영된다 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> ItemLabelBox;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ItemNameText;

	/** ValueMin~ValueMax 이더 — 실제 판매가는 이 범위 안에서 굴려진다(docs/systems/economy.md 가격식) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ItemPriceText;

	/** 무게 · 소지 슬롯 · 파손 여부를 ' · '로 이어 붙인 것 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ItemInfoText;

	/** 아이템 꼭대기(Radius)와 라벨 아래쪽 끝 사이의 간격, cm 단위 — WBP_HUD에서 편집 */
	UPROPERTY(EditAnywhere, Category="HUD")
	float ItemLabelLift = 15.f;

	/** 머리에 착용한 장비 — PlayerState.HeadItem을 그대로 반영(헬멧 헤드램프) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HeadText;

	/** 화면 좌하단: 수락한 주문들, 한 줄씩 — 이름 · 재료 · 마감일(docs/systems/order.md) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> OrdersText;

	/** 판 공유 자금 — 판 전체가 하나의 지갑을 쓴다(docs/systems/economy.md) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> MoneyText;

	/** 바라보는 납품 창구에 올려둔 것들 — 창구를 안 보고 있으면 숨는다(docs/systems/order.md 납품) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CounterText;

	/** 아이템 라벨 안의 조리 게이지 — 열을 받는 중일 때만 보인다(docs/systems/cooking.md) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> CookBar;

	/** 이번 기간 수입과 정산까지 남은 날 — 압박이 숫자로 읽히게 한다(docs/systems/economy.md 정산) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> PeriodText;

	/** 스태미나를 쓸 수 있는 동안의 채우기 색 — WBP_HUD에서 편집 */
	UPROPERTY(EditAnywhere, Category="HUD")
	FLinearColor StaminaColor = FLinearColor(0.85f, 0.85f, 0.75f, 1.f);

	/** 탈진 래치가 달리기를 막는 동안의 채우기 색(docs/systems/character.md 세 조항) */
	UPROPERTY(EditAnywhere, Category="HUD")
	FLinearColor StaminaExhaustedColor = FLinearColor(0.75f, 0.25f, 0.2f, 1.f);

	/** 익어가는 동안의 조리 게이지 색 */
	UPROPERTY(EditAnywhere, Category="HUD")
	FLinearColor CookColor = FLinearColor(0.95f, 0.7f, 0.25f, 1.f);

	/** 다음 전이가 "탄 것"일 때의 조리 게이지 색 — 지금 집으라는 신호다 */
	UPROPERTY(EditAnywhere, Category="HUD")
	FLinearColor BurnColor = FLinearColor(0.85f, 0.25f, 0.15f, 1.f);

	/** 이번 기간 수입이 목표를 채웠을 때의 색 */
	UPROPERTY(EditAnywhere, Category="HUD")
	FLinearColor PeriodMetColor = FLinearColor(0.6f, 0.9f, 0.6f, 1.f);

	/** 아직 목표에 못 미칠 때의 색 — 정산일이 다가올수록 이 색이 압박이 된다 */
	UPROPERTY(EditAnywhere, Category="HUD")
	FLinearColor PeriodShortColor = FLinearColor(0.95f, 0.75f, 0.45f, 1.f);

	/** 다음 정산일을 앞으로 몇 날까지 찾아볼지 — 못 찾으면 정산이 없는 판으로 보고 줄을 숨긴다 */
	UPROPERTY(EditAnywhere, Category="HUD")
	int32 PeriodLookahead = 30;

private:
	UFUNCTION()
	void HandleHpChanged();

	UFUNCTION()
	void HandleCarryChanged();

	UFUNCTION()
	void HandlePhaseChanged(ECL9NightPhase NewPhase);

	UFUNCTION()
	void HandleFocusPromptChanged();

	UFUNCTION()
	void HandleOrdersChanged();

	UFUNCTION()
	void HandleMoneyChanged();

	UFUNCTION()

	/** PlayerState/GameState는 construct 이후에 복제되어 들어온다 — 짧은 타이머가 둘 다 존재할 때까지 재시도한다 */
	void TryWire();

	/** Pawn은 생겼다 사라졌다 한다(spawn · 관전 · 로비 복귀) — 바뀔 때마다 캐릭터 델리게이트를 다시 바인딩한다 */
	void WireCharacter();

	void RefreshHp();
	void RefreshCarry();
	void RefreshClock();
	void RefreshStamina();
	void RefreshPrompt();
	void RefreshCharge();

	/** 윤곽선 대상을 따라간다: 내용은 대상이나 행이 바뀔 때만, 위치는 매 프레임 갱신한다 */
	void RefreshItemLabel();
	void FillItemLabel(const ACL9Item& Item);

	/** 조리 진행도는 매 프레임 바뀌므로 라벨 내용과 따로 갱신한다 */
	void RefreshCookBar(const ACL9Item& Item);

	void RefreshHead();
	void RefreshOrders();
	void RefreshMoney();

	/** 이번 기간 수입 · 목표 · 정산까지 남은 날 */
	void RefreshPeriod();

	/** 창구는 남이 올려도 내용이 바뀌므로 이벤트가 아니라 매 프레임 본다 — 바라보는 동안만이다 */
	void RefreshCounter();

	/** 주문 화면은 offer 상태에서 파생된다: GameState.OfferedOrders가 서 있는 동안 열리고, 선택이 이를 비우면 닫힌다 */
	void DeriveOrderScreen();

	/** 아이템 테이블 행의 표시 이름, 없으면 alias로 대체 */
	FText ItemLabel(const FCL9CarriedItem& Item) const;

	ACL9PlayerState* GetOwnPlayerState() const;
	ACL9GameState* GetCL9GameState() const;
	ACL9Character* GetOwnCharacter() const;

	/** 이 위젯이 OnFocusPromptChanged를 바인딩해 둔 캐릭터 */
	TWeakObjectPtr<ACL9Character> WiredCharacter;

	/** 라벨이 현재 설명하고 있는 아이템 — 이 값이 바뀔 때만 텍스트를 다시 그린다 */
	TWeakObjectPtr<ACL9Item> LabeledItem;

	/** …또는 그 아래 행이 바뀔 때도(요리는 같은 액터 위에서 행을 바꿔치기한다) */
	TWeakObjectPtr<const UCL9ItemDef> LabeledDef;

	FTimerHandle WireTimer;

	/** 시계 텍스트만 다시 그린다 — 값 자체는 복제된 타임스탬프에서 파생된다 */
	FTimerHandle ClockTimer;
};
