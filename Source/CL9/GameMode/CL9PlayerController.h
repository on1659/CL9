// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Data/CL9Types.h"
#include "CL9PlayerController.generated.h"

/** 구매가 거절됐을 때 누른 사람의 화면에서만 발동한다 (docs/systems/shop.md) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCL9BuyRejected, FName, EquipAlias, ECL9BuyResult, Reason);

class UInputAction;
class UInputMappingContext;

/**
 * 1인칭 플레이어 컨트롤러다. 로컬 플레이어를 위한 입력 매핑 컨텍스트를 등록하고,
 * 카메라 매니저(피치 제한)를 오버라이드하며, 로컬 UI를 부팅한다
 * (docs/systems/ui_system.md).
 */
UCLASS(abstract)
class ACL9PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACL9PlayerController();

	/**
	 * 클라이언트 콘솔에 입력된 cl9.* 치트 줄을 호스트로 전달한다
	 * (docs/systems/cheats.md). 선언은 모든 빌드에 남아있어야 RPC 테이블이
	 * 일치한다 — shipping에서는 본문만 비워진다.
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_CL9Cheat(const FString& CommandLine);

	/** 화면이 열려 있는 동안 UI 매니저가 게임플레이 컨텍스트를 꺼둔다(docs/systems/input.md) */
	void SetGameplayInputEnabled(bool bEnabled);

	/** 죽은 뒤 시점이 살아있는 팀원을 타는 동안 true(docs/systems/ui.md 관전) */
	UFUNCTION(BlueprintPure, Category="CL9|Spectate")
	bool IsSpectating() const
	{
		return bSpectating;
	}

	/** 다음으로 살아있는 팀원으로 — IA_SpectateNext와 관전 화면의 버튼이 모두 여기로 들어온다(로컬 → 호스트 판정) */
	UFUNCTION(BlueprintCallable, Category="CL9|Spectate")
	void SpectateNext();

	/** 관전 화면이 "다음 동료 ({0})"의 키 표기를 채울 때 읽는다 */
	const UInputAction* GetSpectateNextAction() const
	{
		return SpectateNextAction;
	}

	/** 호스트 전용: 플래그를 바꾸고 로컬 훅을 발동한다(소유 클라이언트는 OnRep이 처리한다) */
	void HostSetSpectating(bool bNewSpectating);

	/**
	 * 로비의 주문 선택을 확정한다 — GameState.OfferedOrders에 대한 인덱스들이며,
	 * 정확히 OrdersRequired개여야 한다. 리슨 호스트 본인의 컨트롤러만
	 * 확정할 수 있다(docs/systems/order.md 호스트가 확정); 주문 화면의 버튼과
	 * cl9.pick이 모두 여기로 들어온다.
	 */
	UFUNCTION(BlueprintCallable, Category="CL9|Orders")
	void AcceptOrders(const TArray<int32>& OfferedIndices);

	/**
	 * 트럭 패널의 [구매] — 판정은 호스트가 한다(docs/systems/shop.md 구매 흐름).
	 * 주문 확정과 달리 **클라이언트도 살 수 있다**: 자금이 판 공유라 누가 눌러도 같은 지갑에서
	 * 나가고, 산 물건은 트럭 옆 실물로 전원에게 보인다. 거절되면 누른 사람에게만 사유가 돌아간다.
	 */
	UFUNCTION(BlueprintCallable, Category="CL9|Shop")
	void BuyEquipment(FName EquipAlias);

	/** 구매가 거절됐다 — 누른 화면이 사유를 띄운다 (표현 전용) */
	UPROPERTY(BlueprintAssignable, Category="CL9|Shop")
	FCL9BuyRejected OnBuyRejected;

	/**
	 * 상호작용한 사람의 화면에서만 UI 표의 행을 연다 — 상호작용 판정은 호스트가 하는데
	 * 화면은 누른 사람의 것이므로, 호스트가 그 사람의 컨트롤러로 이걸 보낸다.
	 * 상호작용 액터가 호스트에서 부르므로 public이다.
	 */
	UFUNCTION(Client, Reliable)
	void Client_OpenScreen(FName UIName);

	/**
	 * 호스트 → 이 플레이어의 머신: 이번 판 끝의 바깥 재화(이더) 지급 (docs/goal/loop-7f-out-game-currency.md). 판마다 모두에게 한 번 —
	 * 죽었거나 남겨졌으면 0 이다. 그 머신의 `UCL9WalletSubsystem` 이 더하고 저장한다. 새벽 화면 열기와 같은 채널이고 먼저 보낸다
	 */
	UFUNCTION(Client, Reliable)
	void Client_WalletGrant(int32 Amount);

	/**
	 * 스캔 — UI 표의 Popup 행 "Scan"을 새로 연다(열려 있으면 닫고 다시). 우클릭 바인딩은 캐릭터가 갖는다
	 * (탭이면 여기로, `UseHoldTime` 이상 길게 누르면 먹기 — docs/systems/input.md `IA_Scan`)라 컨트롤러는
	 * 표현만 맡고 판정이 없다.
	 */
	UFUNCTION(BlueprintCallable, Category="CL9|Scan")
	void RequestScan();

	// -- 스킨 (docs/goal/loop-7k-skins.md) --

	/** 클라이언트 → 호스트: 내가 입은 스킨 — 호스트는 행이 있는지만 본다(보유는 각자의 파일이라 확인할 수 없다, 치장 전용) */
	UFUNCTION(Server, Reliable)
	void Server_SetSkin(FName SkinAlias);

	/** 내 스킨을 알린다 — 호스트(로컬 컨트롤러)면 바로, 클라이언트면 `Server_SetSkin` 으로 */
	void RequestSkin(FName SkinAlias);

	/** 빙의 — 호스트의 로컬 컨트롤러는 여기서 지갑의 스킨을 알린다 */
	virtual void OnPossess(APawn* InPawn) override;

	/** 클라이언트가 빙의를 받으면 지갑의 스킨을 알린다(레벨을 건너도 새 폰에 다시) */
	virtual void AcknowledgePossession(APawn* P) override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** M(IA_Pause) — 열린 화면이 없으면 메뉴를 열고, 있으면 한 단계 뒤로(히스토리가 아래 화면을 다시 연다 — docs/systems/ui_system.md) */
	void PauseInput();

	UFUNCTION(Server, Reliable)
	void Server_SpectateNext();

	UFUNCTION(Server, Reliable)
	void Server_AcceptOrders(const TArray<int32>& OfferedIndices);

	UFUNCTION(Server, Reliable)
	void Server_BuyEquipment(FName EquipAlias);

	/** 산 사람에게만 돌아가는 거절 통지 — 성공은 자금 변화로 알 수 있어 따로 보내지 않는다 */
	UFUNCTION(Client, Reliable)
	void Client_BuyRejected(FName EquipAlias, ECL9BuyResult Reason);


	/** 로컬: 관전 화면과 IMC_Spectate가 플래그를 함께 따라가므로 둘이 어긋나는 일이 없다 */
	UFUNCTION()
	void OnRep_Spectating();

	/** 로컬 플레이어에게 추가되는 매핑 컨텍스트들 — BP 자식에서 할당한다(docs/systems/input.md) */
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;

	/** 일시정지/닫기 액션 — BP 자식에서 할당하며, 폰 없이도 동작하도록 컨트롤러에 둔다 */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> PauseAction;

	/** IMC_Spectate — 관전 중에 게임플레이 컨텍스트보다 위에 추가된다; DefaultMappingContexts에는 포함되지 않는다 */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> SpectateMappingContext;

	/** 관전 대상 전환 액션 — BP 자식에서 할당한다 */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> SpectateNextAction;

	/** SpectateDelay 이후 호스트가 기록한다; 로비 부활 때 초기화된다. PlayerController 리플리케이션 규칙상 소유자에게만 전달된다 */
	UPROPERTY(ReplicatedUsing=OnRep_Spectating)
	bool bSpectating = false;
	/**
	 * 트럭 도착 카메라(7j, docs/goal/loop-7j-round-start.md) — 판을 여는 도착 동안 트럭 패드 공간 `ArrivalCameraOffset` 의 로컬 카메라가 트럭 몸체를 보고,
	 * 캐릭터는 이 머신에서 숨긴다(트럭에서 내린 것으로). 도착이 끝나면 폰으로 블렌드해 돌아간다. 전원 인트로가 화면을 잡고 있으면 시작하지 않는다
	 */
	void TickArrivalCamera(float DeltaTime);

	/** 도착 카메라를 놓고 캐릭터를 다시 보인다 */
	void EndArrivalCamera(const TCHAR* Why);

	/** 이 머신에서 모든 캐릭터를 숨기거나 보인다(도착 동안) */
	void SetArrivalCharactersHidden(bool bHideCharacters);

	/** 트럭 도착 카메라(7j, 로컬) — null 이 아니면 도착이 화면을 잡고 있다 */
	UPROPERTY(Transient)
	TObjectPtr<ACameraActor> ArrivalCamera;

	/** 이미 처리한 도착(`ACL9GameState::GetArrivalSeq`) — 같은 도착으로 두 번 시작하지 않는다 */
	int32 ArrivalHandledSeq = 0;

};
