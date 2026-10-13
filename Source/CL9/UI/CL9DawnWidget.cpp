// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9DawnWidget.h"
#include "Internationalization/StringTableRegistry.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameMode/CL9GameState.h"
#include "GameMode/CL9Wallet.h"
#include "GameMode/CL9GameInstance.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ProgressionDef.h"
#include "Data/CL9RulesDef.h"
#include "UI/CL9StageText.h"

void UCL9DawnWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (CloseText != nullptr)
	{
		CloseText->SetText(LOCTABLE("CL9", "Dawn.Close"));
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
		GameState->OnNightPhaseChanged.RemoveDynamic(this, &UCL9DawnWidget::HandlePhaseChanged);
		if (WipePenaltyHandle.IsValid())
		{
			GameState->OnWipePenaltyChanged.Remove(WipePenaltyHandle);
		}
		if (ImposterHandle.IsValid())
		{
			GameState->OnImposterResultChanged.Remove(ImposterHandle);
		}
		if (ContributionHandle.IsValid())
		{
			GameState->OnContributionsChanged.Remove(ContributionHandle);
		}
		if (QuotaHandle.IsValid())
		{
			GameState->OnQuotaPenaltyChanged.Remove(QuotaHandle);
		}
	}
	WipePenaltyHandle.Reset();
	ImposterHandle.Reset();
	ContributionHandle.Reset();
	QuotaHandle.Reset();
	if (UCL9WalletSubsystem* Wallet = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCL9WalletSubsystem>() : nullptr)
	{
		Wallet->OnWalletChanged.Remove(WalletHandle);
	}
	WalletHandle.Reset();
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
	GameState->OnNightPhaseChanged.AddDynamic(this, &UCL9DawnWidget::HandlePhaseChanged);
	WipePenaltyHandle = GameState->OnWipePenaltyChanged.AddUObject(this, &UCL9DawnWidget::RefreshResult);
	ImposterHandle = GameState->OnImposterResultChanged.AddUObject(this, &UCL9DawnWidget::RefreshResult);
	ContributionHandle = GameState->OnContributionsChanged.AddUObject(this, &UCL9DawnWidget::RefreshResult);
	QuotaHandle = GameState->OnQuotaPenaltyChanged.AddUObject(this, &UCL9DawnWidget::RefreshResult);
	if (UCL9WalletSubsystem* Wallet = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCL9WalletSubsystem>() : nullptr)
	{
		WalletHandle = Wallet->OnWalletChanged.AddUObject(this, &UCL9DawnWidget::RefreshResult);
	}

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
			? FText::Format(LOCTABLE("CL9", "Dawn.Title"), GameState->GetCurrentDay())
			: LOCTABLE("CL9", "Dawn.TitlePlain"));
	}
	// 정산일이 아닌 새벽에는 견줄 목표가 없다 — 이번 기간에 얼마를 벌었는지만 알려 준다
	SetResult(GameState != nullptr
		? FText::Format(LOCTABLE("CL9", "Dawn.Earned"), GameState->GetPeriodIncome())
		: FText::GetEmpty());
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
		TitleText->SetText(FText::Format(LOCTABLE("CL9", "Dawn.SettleTitle"), Day));
	}
	SetResult(FText::Format(LOCTABLE("CL9", "Dawn.Settle"), Earned, Target));
	if (VerdictText != nullptr)
	{
		VerdictText->SetVisibility(ESlateVisibility::HitTestInvisible);
		VerdictText->SetColorAndOpacity(FSlateColor(bPassed ? PassColor : FailColor));
		VerdictText->SetText(bPassed
			? LOCTABLE("CL9", "Dawn.Passed")
			: FText::Format(LOCTABLE("CL9", "Dawn.Failed"), Day));
	}
	if (CloseText != nullptr)
	{
		CloseText->SetText(bPassed
			? LOCTABLE("CL9", "Dawn.Close")
			: LOCTABLE("CL9", "Dawn.CloseFailed"));
	}
}

void UCL9DawnWidget::SetResult(const FText& Base)
{
	ResultBaseText = Base;
	RefreshResult();
}

void UCL9DawnWidget::RefreshResult()
{
	if (ResultText == nullptr)
	{
		return;
	}

	const ACL9GameState* GameState = GetCL9GameState();
	const bool bLeftBehind = GameState != nullptr && !GameState->GetLeftBehindNames().IsEmpty();
	const bool bWiped = GameState != nullptr && GameState->GetWipePenalty().bActive;
	const bool bImposter = GameState != nullptr && GameState->GetImposterResult().bActive;
	const bool bContributions = GameState != nullptr && !GameState->GetContributions().IsEmpty();
	const bool bQuotaPenalty = GameState != nullptr && GameState->GetQuotaPenalty().bActive;
	// 회사에 갈 날(7i) — 회사가 있고, 여기가 회사가 아니고, 로비이고, 오늘이 정산일이다(게임 모드의 IsCompanyDue 와 같은 판정, 표는 모든 머신에 있다)
	const UCL9GameInstance* CL9Instance = Cast<UCL9GameInstance>(GetGameInstance());
	const UCL9DefRegistry* Registry = CL9Instance != nullptr ? CL9Instance->GetDefRegistry() : nullptr;
	const UCL9RulesDef* Rules = Registry != nullptr ? Registry->GetRules() : nullptr;
	const UCL9ProgressionDef* Progression = Registry != nullptr ? Registry->GetProgression() : nullptr;
	const bool bCompanyDue = GameState != nullptr && Rules != nullptr && Progression != nullptr && !Rules->CompanyLevel.IsNull()
		&& !Rules->IsCompanyWorld(GetWorld()) && GameState->GetPhase() == ECL9NightPhase::Lobby && Progression->IsSettlementDay(GameState->GetCurrentDay());
	const UCL9WalletSubsystem* Wallet = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCL9WalletSubsystem>() : nullptr;
	// 스테이지 별(7l) — 이번 새벽(또는 회사)에 별을 매긴 정산이 있었다. 복제된 지난 정산에서 읽는다
	const FCL9SettlementResult* Stage = (GameState != nullptr && GameState->GetLastSettlement().Day > 0 && GameState->GetLastSettlement().Stars >= 0)
		? &GameState->GetLastSettlement() : nullptr;
	if (!bLeftBehind && !bWiped && !bImposter && !bContributions && !bQuotaPenalty && !bCompanyDue && Wallet == nullptr && Stage == nullptr)
	{
		ResultText->SetText(ResultBaseText);
		return;
	}

	// 기본 문장이 비어 있으면(대기 중 GameState가 없었던 경우) 빈 줄 없이 덧붙는 줄만 낸다
	TArray<FText> Lines;
	if (!ResultBaseText.IsEmpty())
	{
		Lines.Add(ResultBaseText);
	}
	if (Stage != nullptr)
	{
		Lines.Add(FText::Format(LOCTABLE("CL9", "Dawn.StageStars"), CL9StageText::SiteWithStars(Registry, Stage->Site, Stage->Stars)));
		if (!Stage->OpenedSite.IsNone())
		{
			Lines.Add(FText::Format(LOCTABLE("CL9", "Dawn.StageOpened"), CL9StageText::SiteName(Registry, Stage->OpenedSite)));
		}
	}
	if (bWiped)
	{
		// 깎은 것이 없어도(잔액 0) 전멸은 전멸이라 0 으로 보인다
		Lines.Add(FText::Format(LOCTABLE("CL9", "Dawn.WipePenalty"), GameState->GetWipePenalty().Taken));
	}
	if (bImposter)
	{
		// 임포스터 공개(7g) — 판이 끝나야 모두가 안다. 이름은 플레이어 이름이라 번역 대상이 아니다
		const FCL9ImposterResult& Imposter = GameState->GetImposterResult();
		const FText Name = FText::FromString(Imposter.ImposterName);
		switch (Imposter.Outcome)
		{
		case ECL9ImposterOutcome::Won:
			Lines.Add(FText::Format(LOCTABLE("CL9", "Dawn.ImposterWon"), Name));
			break;
		case ECL9ImposterOutcome::Killed:
			Lines.Add(FText::Format(LOCTABLE("CL9", "Dawn.ImposterKilled"), Name, FText::FromString(Imposter.KillerName)));
			break;
		default:
			Lines.Add(FText::Format(LOCTABLE("CL9", "Dawn.ImposterFailed"), Name));
			break;
		}
	}
	if (bQuotaPenalty)
	{
		// 정산 미달(7i) — 판은 이어 가고 코인만 깎였다
		Lines.Add(FText::Format(LOCTABLE("CL9", "Dawn.QuotaPenalty"), GameState->GetQuotaPenalty().Taken));
	}
	if (bCompanyDue)
	{
		Lines.Add(LOCTABLE("CL9", "Dawn.CompanyDue"));
	}
	if (bContributions)
	{
		// 처치 기여(7h) — 누가 깨운 몬스터에게 누가 죽었나. 이름은 플레이어 이름, 몬스터는 행의 표시 이름
		for (const FCL9Contribution& Entry : GameState->GetContributions())
		{
			Lines.Add(FText::Format(LOCTABLE("CL9", "Dawn.Contribution"), Entry.MonsterName,
				FText::FromString(Entry.VictimName), FText::FromString(Entry.WakerName)));
		}
	}
	if (Wallet != nullptr)
	{
		// 바깥 재화(이더, 7f) — 이 머신의 지갑. 이번 판에 받았으면 "+N (보유 M)", 아니면 보유만
		Lines.Add(Wallet->GetLastGrant() > 0
			? FText::Format(LOCTABLE("CL9", "Dawn.Wallet"), Wallet->GetLastGrant(), Wallet->GetEther())
			: FText::Format(LOCTABLE("CL9", "Dawn.WalletBalance"), Wallet->GetEther()));
	}
	ResultText->SetText(FText::Join(FText::FromString(TEXT("\n")), Lines));
}


void UCL9DawnWidget::HandlePhaseChanged(ECL9NightPhase NewPhase)
{
	RefreshResult();
}

ACL9GameState* UCL9DawnWidget::GetCL9GameState() const
{
	const UWorld* World = GetWorld();
	return (World != nullptr) ? World->GetGameState<ACL9GameState>() : nullptr;
}
