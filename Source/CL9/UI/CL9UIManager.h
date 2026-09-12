// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CL9UIManager.generated.h"

class APlayerController;
class UCL9MainWidget;
class UCL9UITable;
class UCL9Widget;

/**
 * 로컬 전용 UI 매니저(docs/systems/ui_system.md, 2026-09-05 결정): UI 테이블을 읽어서
 * 이름으로 WBP_Main의 레이어에 위젯을 연다. UI를 추가할 때 C++을 건드릴 일은 없다 —
 * WBP를 만들고 테이블 행을 추가하면 된다. GameInstanceSubsystem이라서 Get()은
 * 자체 수명주기를 가진 싱글턴이다(AddToRoot 불필요).
 */
UCLASS()
class UCL9UIManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** 어떤 월드 오브젝트에서든 싱글턴에 접근한다 */
	static UCL9UIManager* Get(const UObject* WorldContextObject);

	/** 로컬 PlayerController가 BeginPlay에서 한 번 호출한다 — WBP_Main을 생성한다 */
	void InitForPlayer(APlayerController* PlayerController);

	/**
	 * 테이블 행의 위젯을 연다(이미 열려 있으면 그 인스턴스를 반환한다). Screen 행은 이전 화면의 위젯을 닫고
	 * 히스토리에 이름을 쌓는다 — 닫으면 이전 화면으로 돌아간다(docs/systems/ui_system.md 화면 히스토리).
	 * bResetHistory: 상태가 파생시키는 뿌리 화면(관전)이 열릴 때 — 위에 쌓였던 메뉴·설정을 버리고 이 화면만 남긴다.
	 */
	UFUNCTION(BlueprintCallable, Category="CL9|UI")
	UCL9Widget* OpenWidget(FName UIName, bool bResetHistory = false);

	/**
	 * 닫는다. Screen이면 히스토리에서 빼고, 그것이 현재 화면이었으면 바로 아래 화면을 다시 연다(M·[X]·CloseSelf가
	 * 전부 이 경로 — "한 단계 뒤로"). 현재 화면이 아닌 이름(메뉴 아래의 관전)은 히스토리에서만 빠진다.
	 */
	UFUNCTION(BlueprintCallable, Category="CL9|UI")
	void CloseWidget(FName UIName);

	UFUNCTION(BlueprintPure, Category="CL9|UI")
	bool IsOpen(FName UIName) const
	{
		return OpenWidgets.Contains(UIName);
	}

	/** 현재 열려 있는 Screen 레이어 행, 없으면 None — 중간 참가 복원이 페이즈 파생값과 대조한다 */
	UFUNCTION(BlueprintPure, Category="CL9|UI")
	FName GetOpenScreen() const
	{
		return OpenScreenName;
	}

private:
	const UCL9UITable* GetTable() const;

	/** 입력 모드, 커서, IMC_UI와 게임플레이 IMC 모두 "Screen이 열려 있는가"를 따라간다 */
	void UpdateInputMode();

	/** 열린 목록에서 빼고 사라지게 한다 — 히스토리는 건드리지 않는다(OpenWidget의 배타 닫기와 CloseWidget이 같이 쓴다) */
	void RemoveWidget(FName UIName);

	/** 로컬 플레이어마다 한 번 생성되며, 자신의 월드와 함께 소멸한다(outer = PlayerController) */
	UPROPERTY(Transient)
	TObjectPtr<UCL9MainWidget> MainWidget;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UCL9Widget>> OpenWidgets;

	FName OpenScreenName;

	/** Screen 이름의 스택 — 마지막이 현재 화면(OpenScreenName). 위젯은 하나만 살아 있고 이름만 쌓인다 */
	TArray<FName> ScreenHistory;

	TWeakObjectPtr<APlayerController> OwningPlayer;
};
