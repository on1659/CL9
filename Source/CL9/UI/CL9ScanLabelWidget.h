// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CL9ScanLabelWidget.generated.h"

class ACL9ActorBase;
class ACL9Item;
class UTextBlock;
class UWidgetAnimation;

/**
 * 우클릭 스캔의 라벨 하나 (docs/systems/ui.md 스캔): 스캔 파동이 닿은 대상(아이템·몬스터)의 이름을,
 * 아이템이면 값도 보여준다. 스캔 위젯(UCL9ScanWidget)이 자기 LabelLayer에 하나씩 붙이고 매 프레임
 * 대상 위로 옮기며, 라벨은 **켜져 있는 동안만** 자기 애니메이션을 재생하다 끝나면 스스로 숨는다
 * (2026-09-06 사용자: "WBP 하나 띄워서 애님 재생, UMG 켜져 있는 중에만"). WBP_ScanLabel에 `ShowAnim`
 * 위젯 애니메이션이 있으면 그것이 수명이고, 없으면 C++ 페이드(인/아웃)가 대신한다.
 * 이름·가격 문구는 포커스 라벨과 같은 함수를 쓰므로 둘이 어긋나지 않는다.
 */
UCLASS(Abstract)
class UCL9ScanLabelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UCL9ScanLabelWidget(const FObjectInitializer& ObjectInitializer);

	/** 대상 문구를 채우고 켠다 — ShowAnim이 있으면 그 길이가, 없으면 Duration이 수명 */
	void Show(const ACL9ActorBase& Target, float Duration);

	/** 애니메이션을 멈추고 숨긴다 */
	void Hide();

	/** 켜져서 재생 중인가 — 꺼지면 스캔 위젯이 그 항목을 끝난 것으로 본다 */
	bool IsShowing() const
	{
		return bShowing;
	}

	/** 대상의 현재 표 행에서 이름을 채우고, 아이템이면 값도 채운다(몬스터는 값 줄을 접는다) */
	void SetTarget(const ACL9ActorBase& Target);

	/** "{Value} 이더" — 개체별 실제 값(ACL9Item::GetValue), 0이면 "값 없음" */
	static FText MakePriceText(const ACL9Item& Item);

	/** 아이템·몬스터 행의 DisplayName, 없으면 액터 이름 */
	static FText MakeNameText(const ACL9ActorBase& Target);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> PriceText;

	/** WBP_ScanLabel에 이 이름의 애니메이션을 만들면 켜질 때 재생되고 끝나면 숨는다 (선택) */
	UPROPERTY(Transient, meta=(BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> ShowAnim;

	/** ShowAnim이 없을 때의 C++ 페이드 — 켜진 뒤 불투명해지는 시간 */
	UPROPERTY(EditAnywhere, Category="Scan")
	float FadeInTime = 0.12f;

	/** ShowAnim이 없을 때의 C++ 페이드 — 수명 끝에서 사라지는 데 걸리는 시간 */
	UPROPERTY(EditAnywhere, Category="Scan")
	float FadeOutTime = 0.5f;

private:
	bool bShowing = false;

	/** C++ 페이드용 — 켠 시각(실시간)과 총 수명. 실시간 기준이라 프레임이 느려도 표시 시간이 늘어나지 않는다 */
	double ShowStartSeconds = 0.0;
	float Lifetime = 0.f;
	bool bCodeFade = false;
};
