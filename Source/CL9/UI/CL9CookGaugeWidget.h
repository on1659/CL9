// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CL9CookGaugeWidget.generated.h"

class ACL9Item;
class UProgressBar;

/**
 * 조리대 위에 떠 있는 조리 게이지 하나 (docs/systems/ui.md 조리 게이지). 예전에는 재료 라벨
 * 안에 가는 바로 있어서 그 재료를 바라봐야만 보였는데, 요리 중인 조리대는 누구나 어디서든
 * 알아야 하므로 HUD가 익히는 중인 조리대마다 하나씩 자기 CookLayer에 붙이고 매 프레임
 * 조리대 꼭대기 위로 투영한다(2026-09-10 사용자 — "요리중이면 부엌 맨 위에 생겨서 누구나
 * 볼 수 있게, 어떤 방향에서도"). 화면 공간이라 조리대를 어느 쪽에서 봐도 같은 모양이다.
 * 값은 재료의 복제된 진행도(`ACL9Item::GetCookProgress`)에서 읽는다.
 */
UCLASS(Abstract)
class UCL9CookGaugeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 진행도와 색을 재료의 현재 조리 상태로 맞춘다 — 매 프레임 불린다 */
	void Refresh(const ACL9Item& Item);

protected:
	/** 진행도 0~1 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> Bar;

	/** 익어가는 동안의 채우기 색 */
	UPROPERTY(EditAnywhere, Category="Cook")
	FLinearColor CookColor = FLinearColor(0.95f, 0.7f, 0.25f, 1.f);

	/** 다음 전이가 "탄 것"일 때의 채우기 색 — 지금 집으라는 신호다 */
	UPROPERTY(EditAnywhere, Category="Cook")
	FLinearColor BurnColor = FLinearColor(0.85f, 0.25f, 0.15f, 1.f);
};
