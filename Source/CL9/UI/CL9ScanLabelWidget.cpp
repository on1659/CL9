// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9ScanLabelWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Components/TextBlock.h"
#include "HAL/PlatformTime.h"
#include "Core/CL9ActorBase.h"
#include "Data/CL9ItemDef.h"
#include "Items/CL9Item.h"

UCL9ScanLabelWidget::UCL9ScanLabelWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UCL9ScanLabelWidget::Show(const ACL9ActorBase& Target, float Duration)
{
	SetTarget(Target);
	bShowing = true;
	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (ShowAnim != nullptr)
	{
		// 디자이너 애니메이션이 수명이다 — 끝나면 OnAnimationFinished가 숨긴다
		bCodeFade = false;
		SetRenderOpacity(1.f);
		PlayAnimation(ShowAnim);
		return;
	}

	bCodeFade = true;
	ShowStartSeconds = FPlatformTime::Seconds();
	Lifetime = FMath::Max(Duration, KINDA_SMALL_NUMBER);
	SetRenderOpacity(0.f);
}

void UCL9ScanLabelWidget::Hide()
{
	if (ShowAnim != nullptr && IsAnimationPlaying(ShowAnim))
	{
		StopAnimation(ShowAnim);
	}
	bShowing = false;
	bCodeFade = false;
	SetVisibility(ESlateVisibility::Collapsed);
}

void UCL9ScanLabelWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bShowing || !bCodeFade)
	{
		return;
	}

	// 켜져 있는 동안만 돈다: 페이드 인 → 유지 → 페이드 아웃 → 숨김 (실시간 기준)
	const float Elapsed = static_cast<float>(FPlatformTime::Seconds() - ShowStartSeconds);
	if (Elapsed >= Lifetime)
	{
		Hide();
		return;
	}

	float Opacity = 1.f;
	if (FadeInTime > 0.f && Elapsed < FadeInTime)
	{
		Opacity = Elapsed / FadeInTime;
	}
	else if (FadeOutTime > 0.f && Elapsed > Lifetime - FadeOutTime)
	{
		Opacity = (Lifetime - Elapsed) / FadeOutTime;
	}
	SetRenderOpacity(FMath::Clamp(Opacity, 0.f, 1.f));
}

void UCL9ScanLabelWidget::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);
	if (Animation != nullptr && Animation == ShowAnim && bShowing)
	{
		Hide();
	}
}

void UCL9ScanLabelWidget::SetTarget(const ACL9ActorBase& Target)
{
	if (NameText != nullptr)
	{
		NameText->SetText(MakeNameText(Target));
	}
	if (PriceText != nullptr)
	{
		// 값은 아이템에만 있다 — 몬스터 라벨은 이름만 보인다
		const ACL9Item* Item = Cast<ACL9Item>(&Target);
		PriceText->SetText((Item != nullptr) ? MakePriceText(*Item) : FText::GetEmpty());
		PriceText->SetVisibility((Item != nullptr) ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

FText UCL9ScanLabelWidget::MakePriceText(const ACL9Item& Item)
{
	// 행의 범위가 아니라 이 개체가 굴린 실제 값(2026-09-06 사용자) — 호스트가 굴려 복제한다
	const int32 Value = Item.GetValue();
	if (Value <= 0)
	{
		return NSLOCTEXT("CL9", "HudItemNoValue", "값 없음");
	}
	return FText::Format(NSLOCTEXT("CL9", "HudItemValue", "{0} 이더"), Value);
}

FText UCL9ScanLabelWidget::MakeNameText(const ACL9ActorBase& Target)
{
	// 이름은 액터가 스스로 안다(ACL9ActorBase::GetDisplayName) — 아이템은 행 이름, 몬스터는 행 이름,
	// 시설물은 자기 프로퍼티. 화면이 하위 클래스를 알아보려 들면 종류가 늘 때마다 갈래가 늘어난다.
	return Target.GetDisplayName();
}
