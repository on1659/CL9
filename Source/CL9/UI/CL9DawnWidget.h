// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/CL9Widget.h"
#include "CL9DawnWidget.generated.h"

class ACL9GameState;
class UButton;
class UTextBlock;

/**
 * 새벽 결과 화면 (docs/systems/ui.md 새벽 · economy.md 정산): UI 테이블의 `Dawn` 행이다.
 * 밤이 끝나고 정산이 돌 때 무엇이 벌어졌는지 보여주는 자리다.
 *
 * **숫자는 `OnSettled`가 실어다 준다.** 정산은 값을 정리하면서(기간 수입 0으로, 미달이면
 * 1일차로) 되돌리기 때문에, 화면이 나중에 GameState를 읽으면 이미 아무것도 남아 있지 않다.
 * 그래서 화면은 상태를 조회하지 않고 신호가 준 값을 그대로 보여준다.
 *
 * 화면을 여는 것은 게임 쪽이다(새벽 진입 때 Client RPC) — 구매 화면과 같은 경로다.
 */
UCLASS(Abstract)
class UCL9DawnWidget : public UCL9Widget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** "N일차 새벽" 또는 "정산" */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	/** "이번 기간 1,240 / 목표 800" */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ResultText;

	/** 통과·미달의 뜻을 한 줄로 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> VerdictText;

	// [닫기] 버튼은 부모 UCL9Widget의 공통 CloseButton이 CloseSelf에 배선한다 (docs/systems/ui_system.md)

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CloseText;

	UPROPERTY(EditAnywhere, Category="Dawn")
	FLinearColor PassColor = FLinearColor(0.6f, 0.9f, 0.6f, 1.f);

	UPROPERTY(EditAnywhere, Category="Dawn")
	FLinearColor FailColor = FLinearColor(0.9f, 0.45f, 0.35f, 1.f);

private:
	UFUNCTION()
	void HandleSettled(int32 Day, int32 Earned, int32 Target, bool bPassed);

	/** GameState는 construct 이후에 복제되어 들어올 수 있다 — 존재할 때까지 재시도한 뒤 구독한다 */
	void TryWire();

	ACL9GameState* GetCL9GameState() const;

	/** 아직 정산 신호가 오지 않았을 때 보여줄 내용 — 밤이 끝났다는 것만 */
	void ShowWaiting();

	FTimerHandle WireTimer;
};
