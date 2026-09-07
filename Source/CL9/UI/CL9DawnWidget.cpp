// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9DawnWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameMode/CL9GameState.h"

void UCL9DawnWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseButton != nullptr)
	{
		CloseButton->OnClicked.AddDynamic(this, &UCL9DawnWidget::HandleCloseClicked);
	}
	if (CloseText != nullptr)
	{
		CloseText->SetText(NSLOCTEXT("CL9", "DawnClose", "계속"));
	}

	ShowWaiting();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(WireTimer, this, &UCL9DawnWidget::TryWire, 0.2f, true, 0.f);
	}
}

void UCL9DawnWidget::NativeDestruct()
{
	if (ACL9GameState* GameState = GetCL9GameState())
	{
		GameState->OnSettled.RemoveDynamic(this, &UCL9DawnWidget::HandleSettled);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(WireTimer);
	}

	Super::NativeDestruct();
}

void UCL9DawnWidget::TryWire()
{
	ACL9GameState* GameState = GetCL9GameState();
	if (GameState == nullptr)
	{
		return;   // 복제 도착 전 — 타이머가 다시 부른다
	}

	GetWorld()->GetTimerManager().ClearTimer(WireTimer);
	GameState->OnSettled.AddDynamic(this, &UCL9DawnWidget::HandleSettled);

	// 클라이언트에서는 화면 열기(컨트롤러 RPC)와 정산(GameState 멀티캐스트)이 다른 채널이라
	// 도착 순서가 보장되지 않는다 — 이미 지난 정산이 있으면 복제된 값으로 먼저 채우고,
	// 그 뒤에 오는 신호로 갱신한다. 그래야 순서와 무관하게 늘 옳은 숫자가 뜬다.
	// 밤이 시작될 때 지워지므로 `Day != 0`은 곧 "이번 새벽에 정산이 있었다"는 뜻이다.
	// 날짜를 대조하지 않는다 — 미달이면 그 직후 1일차로 되돌아가서 대조가 어긋난다.
	const FCL9SettlementResult& Last = GameState->GetLastSettlement();
	if (Last.Day > 0)
	{
		HandleSettled(Last.Day, Last.Earned, Last.Target, Last.bPassed);
		return;
	}
	ShowWaiting();
}

void UCL9DawnWidget::ShowWaiting()
{
	const ACL9GameState* GameState = GetCL9GameState();
	if (TitleText != nullptr)
	{
		TitleText->SetText(GameState != nullptr
			? FText::Format(NSLOCTEXT("CL9", "DawnTitle", "{0}일차 새벽"), GameState->GetCurrentDay())
			: NSLOCTEXT("CL9", "DawnTitlePlain", "새벽"));
	}
	// 정산일이 아닌 새벽에는 견줄 목표가 없다 — 이번 기간에 얼마를 벌었는지만 알려 준다
	if (ResultText != nullptr)
	{
		ResultText->SetText(GameState != nullptr
			? FText::Format(NSLOCTEXT("CL9", "DawnEarned", "이번 기간 {0} 이더"), GameState->GetPeriodIncome())
			: FText::GetEmpty());
	}
	if (VerdictText != nullptr)
	{
		VerdictText->SetText(FText::GetEmpty());
		VerdictText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UCL9DawnWidget::HandleSettled(int32 Day, int32 Earned, int32 Target, bool bPassed)
{
	// 신호가 준 값을 그대로 쓴다 — 이 뒤에 GameState는 기간 수입을 0으로 되돌리고,
	// 미달이면 1일차로 돌아가므로 나중에 조회하면 아무것도 남아 있지 않다
	if (TitleText != nullptr)
	{
		TitleText->SetText(FText::Format(NSLOCTEXT("CL9", "DawnSettleTitle", "{0}일차 정산"), Day));
	}
	if (ResultText != nullptr)
	{
		ResultText->SetText(FText::Format(NSLOCTEXT("CL9", "DawnSettle", "이번 기간 {0} / 목표 {1} 이더"), Earned, Target));
	}
	if (VerdictText != nullptr)
	{
		VerdictText->SetVisibility(ESlateVisibility::HitTestInvisible);
		VerdictText->SetColorAndOpacity(FSlateColor(bPassed ? PassColor : FailColor));
		VerdictText->SetText(bPassed
			? NSLOCTEXT("CL9", "DawnPassed", "통과 — 다음 기간이 시작된다")
			: FText::Format(NSLOCTEXT("CL9", "DawnFailed", "미달 — 판이 끝났다. {0}일차까지 버텼다"), Day));
	}
	if (CloseText != nullptr)
	{
		CloseText->SetText(bPassed
			? NSLOCTEXT("CL9", "DawnClose", "계속")
			: NSLOCTEXT("CL9", "DawnCloseFailed", "새 판으로"));
	}
}

void UCL9DawnWidget::HandleCloseClicked()
{
	CloseSelf();
}

ACL9GameState* UCL9DawnWidget::GetCL9GameState() const
{
	const UWorld* World = GetWorld();
	return (World != nullptr) ? World->GetGameState<ACL9GameState>() : nullptr;
}
