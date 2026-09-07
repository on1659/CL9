// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Templates/SubclassOf.h"
#include "CL9UITable.generated.h"

class UCL9MainWidget;
class UCL9Widget;
class UInputMappingContext;

/** UI가 WBP_Main의 어느 패널에 열리는지를 나타낸다(docs/systems/ui_system.md) */
UENUM()
enum class ECL9UILayer : uint8
{
	/** 전체 화면 — 배타적이며, 하나를 열면 이전 것이 닫힌다 */
	Screen,
	/** 중첩이 허용된다(다이얼로그, 토스트 등 — 아직 존재하지 않음) */
	Popup,
};

/** UI 행 하나: 어떤 위젯을, 어느 레이어에 */
USTRUCT()
struct FCL9UIInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UCL9Widget> WidgetClass;

	UPROPERTY(EditAnywhere, Category="UI")
	ECL9UILayer Layer = ECL9UILayer::Screen;
};

/**
 * UI 테이블(2026-09-05 결정 — docs/systems/ui_system.md): UI를 추가한다는 것은
 * WBP를 만들고 여기에 행을 추가하는 것이며, C++은 건드리지 않는다. BP_CL9GameInstance에서
 * 할당하므로 C++은 에셋 경로를 전혀 가지지 않는다(conventions § 5).
 */
UCLASS()
class UCL9UITable : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 뷰포트에 추가되는 유일한 위젯 — 나머지 전부는 이 아래에 붙는다 */
	UPROPERTY(EditAnywhere, Category="UI")
	TSubclassOf<UCL9MainWidget> MainWidgetClass;

	/** Screen이 열려 있는 동안 추가되고, 없으면 제거된다(docs/systems/input.md IMC_UI) */
	UPROPERTY(EditAnywhere, Category="UI")
	TObjectPtr<UInputMappingContext> UIMappingContext;

	/** 이름 → 위젯/레이어. 이 키가 OpenWidget이 받는 값이다 */
	UPROPERTY(EditAnywhere, Category="UI")
	TMap<FName, FCL9UIInfo> Rows;
};

/** C++ 자신이 직접 여는 행 이름들 — 나머지는 전부 데이터로만 남는다 */
namespace CL9UINames
{
	inline const FName Pause(TEXT("Pause"));
	inline const FName Spectate(TEXT("Spectate"));
	inline const FName OrderSelect(TEXT("OrderSelect"));
	inline const FName Scan(TEXT("Scan"));
}
