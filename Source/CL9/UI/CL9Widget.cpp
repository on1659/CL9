// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9Widget.h"
#include "Components/Button.h"
#include "UI/CL9UIManager.h"

void UCL9Widget::CloseSelf()
{
	if (UCL9UIManager* UIManager = UCL9UIManager::Get(this))
	{
		UIManager->CloseWidget(UIName);
	}
}

void UCL9Widget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton != nullptr)
	{
		CloseButton->OnClicked.AddUniqueDynamic(this, &UCL9Widget::HandleCloseClicked);
	}

	SlidePanel = SlidePanelName.IsNone() ? nullptr : GetWidgetFromName(SlidePanelName);
	TransitionElapsed = 0.f;
	bClosing = false;
	bTransitioning = FadeTime > 0.f || (SlidePanel.IsValid() && SlideTime > 0.f);
	ApplyTransition();
}

void UCL9Widget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bTransitioning)
	{
		return;
	}

	TransitionElapsed += InDeltaTime;
	ApplyTransition();

	if (TransitionElapsed < FMath::Max(FadeTime, SlideTime))
	{
		return;
	}

	bTransitioning = false;
	if (bClosing)
	{
		// 다 사라졌다 — 매니저의 상태에서는 진작 빠졌으니 여기서 스스로 떨어지면 끝이다
		RemoveFromParent();
	}
}

void UCL9Widget::PlayOutro()
{
	if (bClosing)
	{
		return;
	}

	bClosing = true;
	TransitionElapsed = 0.f;
	OnClosing();

	if (FadeTime <= 0.f)
	{
		RemoveFromParent();
		return;
	}

	// 이미 닫힌 화면이다 — 사라지는 동안 클릭을 먹지 않게 한다(자식 버튼까지 함께 막힌다)
	SetVisibility(ESlateVisibility::HitTestInvisible);
	bTransitioning = true;
	ApplyTransition();
}

void UCL9Widget::ApplyTransition()
{
	const float FadeAlpha = (FadeTime > 0.f) ? FMath::Clamp(TransitionElapsed / FadeTime, 0.f, 1.f) : 1.f;
	SetRenderOpacity(bClosing ? 1.f - FadeAlpha : FadeAlpha);

	UWidget* Panel = SlidePanel.Get();
	if (Panel == nullptr)
	{
		return;
	}

	const float SlideAlpha = (SlideTime > 0.f) ? FMath::Clamp(TransitionElapsed / SlideTime, 0.f, 1.f) : 1.f;
	// 들어올 때는 끝에서 부드럽게 멈추고(감속), 나갈 때는 왔던 만큼 그대로 되돌아간다
	const float Placed = bClosing ? 1.f - SlideAlpha : 1.f - FMath::Square(1.f - SlideAlpha);
	Panel->SetRenderTranslation(FVector2D(0.f, SlideDistance * (1.f - Placed)));
}

void UCL9Widget::HandleCloseClicked()
{
	CloseSelf();
}
