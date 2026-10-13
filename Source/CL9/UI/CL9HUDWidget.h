// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/CL9Types.h"
#include "Templates/SubclassOf.h"
#include "CL9HUDWidget.generated.h"

class ACL9Character;
class ACL9Counter;
class ACL9GameState;
class ACL9Item;
class ACL9Kitchen;
class ACL9PlayerState;
class UBorder;
class UCanvasPanel;
class UCL9CookGaugeWidget;
class UCL9ItemDef;
class UCL9QuickSlotWidget;
class UCL9ScanLabelWidget;
class UHorizontalBox;
class UImage;
class UProgressBar;
class UTextBlock;
class UTexture2D;

/**
 * 게임 중 항상 떠 있는 HUD 레이어 콘텐츠(docs/systems/ui.md 게임 중 HUD): HP, 퀵슬롯,
 * 밤 시계, 상호작용 프롬프트, 던지기 차징, 그리고 윤곽선이 그려진 아이템 위의
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

	/**
	 * 하트 칸들이 나란히 담긴 상자 — **자식 하나가 HP 하나다.** 남은 HP만큼 앞에서부터
	 * 보이고 나머지는 접힌다. 칸의 생김새는 여기서 정하지 않는다: WBP_HUD에서 칸마다
	 * 오버레이를 두고 그 안에 글자든 이미지든 넣으면 되고, 표현을 바꿔도 C++은 그대로다
	 * (2026-09-08 사용자 — "나중에 오버레이 밑에 하트 이미지든 텍스트블록이든 붙이면 된다").
	 * 칸이 아예 없으면 아래 HpText의 숫자 표시로 넘어간다.
	 */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HpBox;

	/** 칸 수보다 HP가 많을 때(또는 칸이 없을 때) 대신 쓰는 숫자 — "HP 12" */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HpText;

	/** 지금 걸려 있는 먹기 효과 — "달리기 +40 (12초)" 한 줄씩, 없으면 접힌다 */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> BuffText;

	/**
	 * 밤이 끝나기 직전에만 번쩍이는 경고(docs/systems/ui.md "중요 알림 번쩍임은 밤 만료
	 * 임박에만 제한적으로"). 만료는 새벽 실패라서, 시계를 안 보고 있어도 눈에 들어와야 한다.
	 */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> NightWarnText;

	/**
	 * 잡혔을 때 화면을 물들이는 빨간 비네트(docs/systems/ui.md 피격 비네트) — 다른 HUD 요소 뒤에 깔린
	 * 전면 이미지다. 세기는 소유 캐릭터의 `GetHurtFlash()`(잡힌 순간 1 → `CL9Rules.HurtVignetteTime`에
	 * 걸쳐 0)를 그대로 불투명도로 옮긴 것이고, 0이면 접혀서 아무것도 그리지 않는다. 이미지가 없으면
	 * 비네트를 그리지 않는다.
	 */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> HurtVignette;

	/**
	 * 잡혔을 때 공격자 쪽을 가리키는 화면 가장자리의 빨간 호(docs/systems/ui.md 피격 방향 호) — 화면 중심을
	 * 피벗으로 도는 이미지 하나다(12시 = 0°). 각도는 캐릭터의 `GetHurtDirectionAngle()`, 불투명도는 비네트와
	 * 같은 `GetHurtFlash()`라 둘이 함께 사그라든다. `CL9Rules.bHurtDirectionIndicator`가 거짓이거나 이번 넘어짐에
	 * 공격자가 없으면(치트 null·관전) 숨긴다. 디자이너가 WBP에서 `Collapsed`로 둔 것은 "끈 것"이라 C++이 절대
	 * 켜지 않는다 — 그래서 WBP 기본값은 `Hidden`이어야 한다. 이미지가 없으면 그리지 않는다.
	 */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> HurtDirection;

	/**
	 * 화면 하단 중앙(프롬프트 위)의 퀵슬롯 — 칸 위젯(WBP_QuickSlot)이 나란히 담기는 상자. HUD가
	 * `CL9Rules.QuickSlots`개를 `QuickSlotClass`로 만들어 붙이고, PlayerState의 `Slots`·`ActiveSlot`을
	 * 그대로 반영한다(docs/systems/ui.md 퀵슬롯). 상자가 없으면 퀵슬롯을 그리지 않는다.
	 */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UHorizontalBox> QuickSlotBox;

	/** WBP_QuickSlot — WBP_HUD에서 지정. 비어 있으면 퀵슬롯을 그리지 않는다 */
	UPROPERTY(EditAnywhere, Category="HUD")
	TSubclassOf<UCL9QuickSlotWidget> QuickSlotClass;

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

	/**
	 * 첫 줍기 힌트 — 프롬프트 위, 가운데 정렬 1~3줄(docs/systems/ui.md "첫 줍기 힌트"). 설명이 필요한 종류
	 * (`UCL9ItemDef::HasUseHint`)를 처음 손에 들었을 때만 "[LMB] 살짝 누르기 — …"를 조립해 보이고 `HintDuration` 뒤
	 * 접는다. 본 적 있음은 이 PC의 `UCL9GameUserSettings`에 종류(Alias)별로 남아 다시는 안 뜬다. 없으면 힌트를 그리지 않는다.
	 */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> HintText;

	/** 힌트가 떠 있는 시간, 초 — WBP_HUD에서 편집 */
	UPROPERTY(EditAnywhere, Category="HUD")
	float HintDuration = 4.f;

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

	/** 화면 좌하단: 수락한 주문들, 한 줄씩 — 이름 · 재료(올린 수/필요 수) · 마감일(docs/systems/order.md) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> OrdersText;

	/** 판 공유 자금 — 판 전체가 하나의 지갑을 쓴다(docs/systems/economy.md) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> MoneyText;

	/** 바라보는 납품 창구에 올려둔 것들 — 창구를 안 보고 있으면 숨는다(docs/systems/order.md 납품) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CounterText;

	/**
	 * 조리 게이지가 붙는 캔버스 — 익히는 중인 조리대마다 WBP_CookGauge 하나가 여기 붙어 조리대
	 * 꼭대기 위로 투영된다. 재료를 바라보지 않아도, 어느 쪽에서 봐도 보인다(docs/systems/cooking.md)
	 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> CookLayer;

	/** WBP_CookGauge — WBP_HUD에서 지정 */
	UPROPERTY(EditAnywhere, Category="HUD")
	TSubclassOf<UCL9CookGaugeWidget> CookGaugeClass;

	/** 조리대 꼭대기와 게이지 아래쪽 끝 사이의 간격, cm 단위 — 상판에 놓인 재료(반지름 20 안팎)에 가리지 않을 만큼 */
	UPROPERTY(EditAnywhere, Category="HUD")
	float CookGaugeLift = 60.f;

	/** 임포스터 알림(본인에게만, docs/goal/loop-7g-imposter.md)이 장소 알림 자리에 머무는 총 시간, 초 — 장소 이름보다 글이 길어 더 오래 둔다 */
	UPROPERTY(EditAnywhere, Category="HUD")
	float ImposterNoticeDuration = 6.f;

	/** 회사에 도착하면 한 번 여는 화면(정산 결과, 7i) — UI 표의 행 이름(데이터) */
	UPROPERTY(EditAnywhere, Category="HUD")
	FName CompanyScreenRow = FName("Dawn");

	/** 판을 여는 "오늘의 주문" 카드(7j)가 화면 가운데에 머무는 총 시간(초, 페이드 포함) — 그 뒤엔 HUD 주문 줄만 남는다 */
	UPROPERTY(EditAnywhere, Category="HUD")
	float OrderCardDuration = 6.f;

	/** 주문 카드 글자 크기 */
	UPROPERTY(EditAnywhere, Category="HUD")
	int32 OrderCardTextSize = 22;

	/** 이번 기간 수입과 정산까지 남은 날 — 압박이 숫자로 읽히게 한다(docs/systems/economy.md 정산) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> PeriodText;

	/** 스태미나를 쓸 수 있는 동안의 채우기 색 — WBP_HUD에서 편집 */
	UPROPERTY(EditAnywhere, Category="HUD")
	FLinearColor StaminaColor = FLinearColor(0.85f, 0.85f, 0.75f, 1.f);

	/** 탈진 래치가 달리기를 막는 동안의 채우기 색(docs/systems/character.md 세 조항) */
	UPROPERTY(EditAnywhere, Category="HUD")
	FLinearColor StaminaExhaustedColor = FLinearColor(0.75f, 0.25f, 0.2f, 1.f);

	/** 이번 기간 수입이 목표를 채웠을 때의 색 */
	UPROPERTY(EditAnywhere, Category="HUD")
	FLinearColor PeriodMetColor = FLinearColor(0.6f, 0.9f, 0.6f, 1.f);

	/** 아직 목표에 못 미칠 때의 색 — 정산일이 다가올수록 이 색이 압박이 된다 */
	UPROPERTY(EditAnywhere, Category="HUD")
	FLinearColor PeriodShortColor = FLinearColor(0.95f, 0.75f, 0.45f, 1.f);

	/** 다음 정산일을 앞으로 몇 날까지 찾아볼지 — 못 찾으면 정산이 없는 판으로 보고 줄을 숨긴다 */
	UPROPERTY(EditAnywhere, Category="HUD")
	int32 PeriodLookahead = 30;

	/** 밤이 이만큼 남은 순간부터 경고가 번쩍인다, 초 (밤 길이는 245초 — rules.md) */
	UPROPERTY(EditAnywhere, Category="HUD")
	float NightWarnTime = 30.f;

	/** 경고가 밝아졌다 어두워졌다 하는 한 주기의 길이, 초 */
	UPROPERTY(EditAnywhere, Category="HUD")
	float NightWarnFlashTime = 0.6f;

	/** 번쩍임이 가장 어두울 때의 불투명도 — 0으로 두면 완전히 사라졌다 돌아온다 */
	UPROPERTY(EditAnywhere, Category="HUD")
	float NightWarnDimOpacity = 0.25f;

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

	/** 내 임포스터 역할이 바뀌었다(본인 머신에만 복제) — 켜졌으면 장소 알림 자리에 임포스터 알림을 띄운다 */
	void HandleImposterChanged();

	UFUNCTION()

	/** PlayerState/GameState는 construct 이후에 복제되어 들어온다 — 짧은 타이머가 둘 다 존재할 때까지 재시도한다 */
	void TryWire();

	/** Pawn은 생겼다 사라졌다 한다(spawn · 관전 · 로비 복귀) — 바뀔 때마다 캐릭터 델리게이트를 다시 바인딩한다 */
	void WireCharacter();

	void RefreshHp();

	/** 퀵슬롯 — 칸 수가 규칙과 다르면 칸 위젯을 다시 만들고, 칸마다 아이콘(없으면 이름)·활성 여부를 채운다 */
	void RefreshCarry();
	void RefreshClock();
	void RefreshStamina();
	void RefreshPrompt();
	void RefreshCharge();

	/**
	 * 활성 칸의 아이템이 바뀌었을 때만 본다 — 처음 드는 설명 필요 종류면 힌트를 띄우고 본 것으로 기록하고,
	 * 아니면 떠 있던 힌트를 접는다(다른 칸이 바뀌거나 머리 장비만 바뀐 경우는 건드리지 않는다)
	 */
	void RefreshHint();

	/** 힌트 문구 — 행의 UseAction·종류에 따라 1~3줄을 줄바꿈으로 잇는다(String Table `Hint.*`) */
	FText BuildHintText(const UCL9ItemDef& Def) const;

	void HideHint();

	/** 윤곽선 대상을 따라간다: 내용은 대상이나 행이 바뀔 때만, 위치는 매 프레임 갱신한다 */
	void RefreshItemLabel();
	void FillItemLabel(const ACL9Item& Item);

	/** 익히는 중인 조리대마다 게이지를 하나씩 두고 매 프레임 꼭대기 위로 투영한다 — 끝난 조리대의 것은 뗀다 */
	void RefreshCookGauges();

	void RefreshHead();

	/**
	 * 수락한 주문 한 줄씩 — 재료는 "이름 올린 수/필요 수"다. 올린 수는 납품 창구의 재고에서만 나오고
	 * (퀵슬롯·수레에 든 것은 세지 않는다 — 정산이 창구 재고만 보기 때문이다), 재고를 주문에 배정하는
	 * 순서도 정산 `ACL9GameMode::SettleOrdersAt`과 같다: 마감이 급한 주문부터 가져간다. 줄의 표시
	 * 순서는 `AcceptedOrders` 그대로다.
	 */
	void RefreshOrders();

	/**
	 * 창구 재고에는 변경 이벤트가 없다(남이 올려도 알림이 오지 않는다) — 매 틱 재고의 서명을 만들어
	 * 지난 틱과 다를 때만 `RefreshOrders`를 부른다. 매 틱 `SetText`하지 않기 위한 것이다.
	 */
	void PollCounterStock();

	/** 납품 창구를 찾아 캐시한다 — 레벨에 하나뿐이고, 아직 없으면(복제 전·창구 없는 레벨) nullptr를 주고 다음 틱에 다시 찾는다 */
	ACL9Counter* FindDeliveryCounter();

	void RefreshMoney();

	/** 먹기 효과는 시각으로 만료되므로 변경 이벤트가 없다 — 스태미나와 같은 이유로 매 프레임 본다 */
	void RefreshBuffs();

	/** 남은 밤이 문턱 아래로 내려간 동안에만 경고를 켜고 불투명도를 흔든다 */
	void RefreshNightWarn(float InDeltaTime);

	/** 잡힌 뒤의 빨간 비네트 — 캐릭터의 GetHurtFlash()를 그대로 이미지 불투명도로 옮긴다(0이면 접는다) */
	void RefreshHurtVignette();

	/**
	 * 피격 방향 호 — 표 bool·공격자 유무·플래시가 다 맞을 때만 각도와 불투명도를 넣고 보인다, 아니면 `Hidden`.
	 * `bHurtDirectionDesignerHidden`이면 아무것도 하지 않는다.
	 */
	void RefreshHurtDirection();

	/** 이번 기간 수입 · 목표 · 정산까지 남은 날 */
	void RefreshPeriod();

	/** 창구는 남이 올려도 내용이 바뀌므로 이벤트가 아니라 매 프레임 본다 — 바라보는 동안만이다 */
	void RefreshCounter();

	/** 주문 화면은 offer 상태에서 파생된다: GameState.OfferedOrders가 서 있는 동안 열리고, 선택이 이를 비우면 닫힌다 */
	void DeriveOrderScreen();

	/**
	 * 회사(7i, docs/goal/loop-7i-company.md)에 도착하면 정산 결과 화면(`CompanyScreenRow`)을 한 번 연다 — 복제된 지난 정산(`LastSettlement.Day > 0`)에서 파생한다.
	 * 접속 순간(PostLogin)에는 UI 매니저가 아직 없어 RPC 로 열 수 없다(클라이언트는 이동 뒤 다시 접속한다)
	 */
	void DeriveCompanyScreen();

	/** `DeriveCompanyScreen` 이 이 레벨에서 이미 열었나 */
	bool bCompanyScreenShown = false;

	/**
	 * "오늘의 주문" 카드(7j, docs/goal/loop-7j-round-start.md) — 판이 열리고 트럭 도착이 끝나면(레버로 연 판은 바로) 한 번, 가운데에 띄운 뒤 흐려진다.
	 * WBP 를 바꾸지 않고 `CookLayer` 캔버스에 런타임으로 만든다. 글은 제목 + 지금 HUD 주문 줄
	 */
	void TickOrderCard(float InDeltaTime);
	void ShowOrderCard();

	UPROPERTY(Transient)
	TObjectPtr<UBorder> OrderCard;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> OrderCardText;

	/** 판으로 넘어왔고 아직 카드를 띄우지 않았다 — 페이즈가 판이 될 때(또는 배선 때 이미 판이면) 선다. 시작 시각은 레벨의 첫 판이면 0 이라 쓰지 않는다 */
	bool bOrderCardPending = false;

	/** 카드가 뜬 뒤 흐른 시간, 음수 = 쉬는 중 */
	float OrderCardElapsed = -1.f;

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

	/** 주문 줄의 "올린 수"가 나오는 납품 창구 — 레벨에 하나다(BP_CL9Counter) */
	TWeakObjectPtr<ACL9Counter> DeliveryCounter;

	/** 마지막으로 주문 줄에 반영한 창구 재고의 서명 — 올린 순서대로의 alias+sub. 이것이 달라질 때만 줄을 다시 그린다 */
	TArray<FCL9ItemRef> CounterStock;

	/** 조리대 → 그 위에 띄운 게이지. 게이지는 CookLayer의 자식이라 소유는 캔버스가 한다 */
	TMap<TWeakObjectPtr<ACL9Kitchen>, TWeakObjectPtr<UCL9CookGaugeWidget>> CookGauges;

	/** 퀵슬롯 칸 위젯들 — 인덱스가 칸 번호(0부터). QuickSlotBox의 자식이라 소유는 상자가 한다 */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UCL9QuickSlotWidget>> QuickSlots;

	/**
	 * 칸마다 마지막으로 그린 아이템 행과 그 아이콘 — 행이 바뀐 칸만 `UCL9DefBase::Icon`을 동기 로드한다
	 * (아이콘은 소프트 참조라 매번 로드하면 안 된다). 아이콘이 없는 행은 nullptr가 남고 이름으로 물러선다.
	 */
	TArray<TWeakObjectPtr<const UCL9ItemDef>> QuickSlotDefs;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UTexture2D>> QuickSlotIcons;

	/** 번쩍임의 위상 — 경고가 꺼져 있는 동안 0으로 돌아가 항상 밝은 지점에서 시작한다 */
	float NightWarnPhase = 0.f;

	/**
	 * 디자이너가 WBP_HUD에서 `HurtDirection`을 `Collapsed`로 두었나 — NativeConstruct에서 한 번 읽어 두고, 참이면
	 * 이후 호를 절대 켜지 않는다. 비네트(`RefreshHurtVignette`)는 Collapsed를 덮어쓰지만 호는 그 규칙을 따르지 않는다.
	 */
	bool bHurtDirectionDesignerHidden = false;

	/** 마지막으로 힌트를 판단한 활성 칸의 아이템(별칭·하위 별칭) — 같은 것이 그대로면 OnCarryChanged가 와도 다시 판단하지 않는다 */
	FCL9ItemRef HintedItem;

	/** HintDuration 뒤 힌트를 접는다 */
	FTimerHandle HintTimer;

	FTimerHandle WireTimer;

	/** 시계 텍스트만 다시 그린다 — 값 자체는 복제된 타임스탬프에서 파생된다 */
	FTimerHandle ClockTimer;
};
