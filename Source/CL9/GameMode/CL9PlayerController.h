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

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** M(IA_Pause) — 일시정지 화면을 열거나, 열려 있는 화면을 닫는다; 관전자는 관전 화면으로 돌아간다(docs/systems/input.md) */
	void PauseInput();

	/** 마우스 오른쪽(IA_Scan) — UI 표의 Popup 행 "Scan"을 새로 연다 (열려 있으면 닫고 다시) */
	void ScanInput();

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

	/** 스캔 액션(마우스 오른쪽) — BP 자식에서 할당한다; Pause처럼 컨트롤러에 두어 HUD가 응답을 책임진다 */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> ScanAction;

	/** SpectateDelay 이후 호스트가 기록한다; 로비 부활 때 초기화된다. PlayerController 리플리케이션 규칙상 소유자에게만 전달된다 */
	UPROPERTY(ReplicatedUsing=OnRep_Spectating)
	bool bSpectating = false;
};
