// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/CL9UITable.h"
#include "CL9MainWidget.generated.h"

class UCanvasPanel;
class UCL9Widget;

/**
 * 뷰포트에 존재하는 유일한 위젯(docs/systems/ui_system.md). HUD는 WBP 디자이너
 * 안에 직접 존재하고, 테이블 기반 UI들은 레이어 패널에 붙는다. 열고 닫는 것을
 * 관리하는 건 UI 매니저의 몫이고, 이 클래스는 위젯을 배치하기만 한다.
 */
UCLASS(Abstract)
class UCL9MainWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 레이어의 패널에 앵커를 꽉 채워 붙인다 */
	void AddWidget(ECL9UILayer Layer, UCL9Widget* Widget);

protected:
	/** 전체 화면 — 매니저가 이 레이어를 배타적으로 관리한다 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> Layer_Screen;

	/** 쌓이는 레이어(다이얼로그, 토스트) */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> Layer_Popup;
};
