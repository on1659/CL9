// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/CL9Widget.h"
#include "CL9SpectateWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * 관전 화면(docs/systems/ui.md 관전): UI 테이블의 "Spectate" 행이
 * ACL9PlayerController::bSpectating이 켜지면 이 위젯을 열고, 플래그가 꺼지면 닫는다.
 * 지금 누구의 시점을 빌리고 있는지 보여주고 다음 동료로 전환하는 버튼을 제공하는데,
 * 이 버튼은 IA_SpectateNext와 같은 SpectateNext()로 연결된다.
 * 부활 UI는 없다 — 회복은 다음 로비의 트럭 패드에서 이루어진다(rules.md 죽음과 회복).
 */
UCLASS(Abstract)
class UCL9SpectateWidget : public UCL9Widget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** "관전 중 · <동료 이름>" — 뷰 타겟 폰의 PlayerState 이름을 표시한다 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TargetNameText;

	/** 살아있는 다음 동료로 전환한다 — 키 입력과 동일하게 호스트가 판정한다 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> NextButton;

private:
	UFUNCTION()
	void HandleNextClicked();

	/** 뷰 타겟은 ClientSetViewTarget으로 도착할 뿐 로컬 이벤트가 없으므로, 짧은 타이머로 이름을 다시 읽어온다 */
	void RefreshTarget();

	FTimerHandle RefreshTimer;
};
