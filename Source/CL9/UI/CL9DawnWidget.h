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
 * **전멸 패널티**(`GameState->GetWipePenalty()`, docs/goal/loop-7a-wipe-penalty.md)도 같은 방식이다 — 복제 값을 읽고
 * `OnWipePenaltyChanged`로 갱신하며, 전멸로 온 새벽이면 "전멸 — 이더 N 차감" 한 줄을 덧붙인다.
 * **임포스터 공개**(`GameState->GetImposterResult()`, docs/goal/loop-7g-imposter.md)도 같다 — 임포스터가 있던 판이면 누구였고
 * 어떻게 끝났는지(성공·처치·실패) 한 줄을 덧붙인다.
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

	/** 페이즈가 바뀌면(새벽 → 로비) 덧붙는 줄을 다시 짠다 — "오늘은 정산일, 레버를 당기면 회사로"는 로비에서만 맞다(7i) */
	UFUNCTION()
	void HandlePhaseChanged(ECL9NightPhase NewPhase);

	/** GameState는 construct 이후에 복제되어 들어올 수 있다 — 존재할 때까지 재시도한 뒤 구독한다 */
	void TryWire();

	ACL9GameState* GetCL9GameState() const;

	/** 아직 정산 신호가 오지 않았을 때 보여줄 내용 — 밤이 끝났다는 것만 */
	void ShowWaiting();

	/**
	 * 결과 줄의 기본 문장을 정하고 화면에 올린다. 트럭에 못 탄 사람이 있으면 그 줄이 `ResultText` 끝에 덧붙는다 —
	 * 줄을 위한 BindWidget을 새로 두지 않는다(기존 WBP 컴파일이 깨진다). `ShowWaiting`·`HandleSettled` 둘 다 이것을 거친다.
	 */
	void SetResult(const FText& Base);

	/** `ResultBaseText` 뒤에 "트럭에 못 탄 사람: …"과 "전멸 — 이더 N 차감"을 (있을 때만) 붙여 `ResultText`를 다시 쓴다 */
	void RefreshResult();

	/** 못 탄 사람 줄을 붙이기 전의 결과 문장 — `OnSettled`가 준 값이나 대기 문장이다 */
	FText ResultBaseText;

	/** 전멸 패널티 결과가 바뀌었다 — 남겨진 사람 목록과 같은 경로로 `RefreshResult` 를 다시 부른다 */
	FDelegateHandle WipePenaltyHandle;

	/** 임포스터 결과(7g) 구독 — `OnImposterResultChanged` 는 동적 델리게이트가 아니라 핸들로 뗀다 */
	FDelegateHandle ImposterHandle;

	/** 처치 기여(7h) 구독 */
	FDelegateHandle ContributionHandle;

	/** 정산 미달 패널티(7i) 구독 */
	FDelegateHandle QuotaHandle;

	/** 이 머신의 지갑(바깥 재화 "이더", 7f)이 바뀌었다 — 귀가 보상 줄을 다시 쓴다 */
	FDelegateHandle WalletHandle;

	FTimerHandle WireTimer;
};
