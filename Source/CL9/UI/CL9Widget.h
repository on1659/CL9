// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CL9Widget.generated.h"

class UButton;

/**
 * UI 테이블이 여는 모든 CL9 위젯의 베이스 클래스다(docs/systems/ui_system.md).
 * 자신의 테이블 행 이름을 알고 있어서 스스로를 닫을 수 있고,
 * 열리고 닫힐 때의 페이드·슬라이드를 여기서 함께 처리한다(docs/systems/ui.md UI 애니메이션).
 */
UCLASS(Abstract)
class UCL9Widget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** UI 매니저가 열 때 설정한다 — 이 인스턴스가 어느 테이블 행에서 왔는지를 나타낸다 */
	void SetUIName(FName InUIName)
	{
		UIName = InUIName;
	}

	UFUNCTION(BlueprintPure, Category="CL9|UI")
	FName GetUIName() const
	{
		return UIName;
	}

	/** UI 매니저를 통해 이 위젯을 닫는다 — 예: 메뉴의 [계속] 버튼 */
	UFUNCTION(BlueprintCallable, Category="CL9|UI")
	void CloseSelf();

	/**
	 * UI 매니저가 닫을 때 부른다. 매니저의 상태(열린 목록·화면 이름)에서는 이미 빠진 뒤라
	 * 여기서는 사라지는 모습만 남는다 — 페이드가 끝나면 스스로 부모에서 떨어진다.
	 */
	void PlayOutro();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/**
	 * 닫히기 시작하는 순간 한 번 — 페이드 뒤에 오는 NativeDestruct보다 먼저 정리해야 하는 것(설정 화면의
	 * 키 캡처 프리프로세서)이 여기에 건다. 매니저가 어떤 경로로 닫든(M·X·서버가 다른 화면을 엶) 불린다.
	 */
	virtual void OnClosing()
	{
	}

	/** 공통 [X] — WBP에 이 이름의 버튼이 있으면 CloseSelf(히스토리 한 단계 뒤로)에 자동 배선된다 (docs/systems/ui_system.md) */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UButton> CloseButton;

	/** 열리고 닫힐 때 밝아지고 어두워지는 시간(초) — 0이면 즉시 (docs/systems/ui.md) */
	UPROPERTY(EditAnywhere, Category="CL9|UI")
	float FadeTime = 0.3f;

	/** 패널이 제자리로 올라오는 시간(초) — 0이면 움직이지 않는다 */
	UPROPERTY(EditAnywhere, Category="CL9|UI")
	float SlideTime = 0.2f;

	/** 패널이 아래에서 올라오는 거리(픽셀) */
	UPROPERTY(EditAnywhere, Category="CL9|UI")
	float SlideDistance = 24.f;

	/**
	 * 미끄러뜨릴 패널의 위젯 이름 — 비우면 페이드만 한다.
	 * 화면 전체를 움직이면 배경 덮개까지 따라 움직여 가장자리가 새므로 안쪽 패널만 지정한다.
	 */
	UPROPERTY(EditAnywhere, Category="CL9|UI")
	FName SlidePanelName;

private:
	UFUNCTION()
	void HandleCloseClicked();

	/** 지금 경과 시간에 맞춰 투명도와 패널 위치를 찍는다 */
	void ApplyTransition();

	FName UIName;

	TWeakObjectPtr<UWidget> SlidePanel;

	float TransitionElapsed = 0.f;

	/** 아직 페이드·슬라이드가 진행 중인가 — 끝나면 틱에서 손을 뗀다 */
	bool bTransitioning = false;

	/** 닫히는 중인가 — 투명도와 패널이 반대로 간다 */
	bool bClosing = false;
};
