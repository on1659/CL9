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

	/** 테이블 행의 위젯을 연다(이미 열려 있으면 그 인스턴스를 반환한다). Screen 행은 이전 화면을 닫는다 */
	UFUNCTION(BlueprintCallable, Category="CL9|UI")
	UCL9Widget* OpenWidget(FName UIName);

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

	/** 로컬 플레이어마다 한 번 생성되며, 자신의 월드와 함께 소멸한다(outer = PlayerController) */
	UPROPERTY(Transient)
	TObjectPtr<UCL9MainWidget> MainWidget;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UCL9Widget>> OpenWidgets;

	FName OpenScreenName;

	TWeakObjectPtr<APlayerController> OwningPlayer;
};
