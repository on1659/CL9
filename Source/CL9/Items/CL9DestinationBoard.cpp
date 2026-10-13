// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9DestinationBoard.h"
#include "Internationalization/StringTableRegistry.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "CL9.h"
#include "Character/CL9Character.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9SiteDef.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameState.h"
#include "GameMode/CL9PlayerController.h"
#include "Data/CL9RulesDef.h"
#include "UI/CL9StageText.h"

ACL9DestinationBoard::ACL9DestinationBoard()
{
}

void ACL9DestinationBoard::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	// 처음 값은 트럭이 떠나온 사이트 — 방을 만들 수 있는 사이트가 아니면 표의 첫 사이트. 별(7l)은 여기서 보지 않는다:
	// BeginPlay 는 게임 모드가 판을 되돌리고 정산하기 전이라 별이 아직 비어 있다. 떠나온 사이트는 정산이 항목을 만들어 늘 열려 있다
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const TArray<FName> Sites = GameInstance != nullptr ? GameInstance->GetHostableSites() : TArray<FName>();
	const FName From = GameInstance != nullptr ? GameInstance->GetRunCarryFromSite() : NAME_None;
	Destination = Sites.Contains(From) ? From : (Sites.Num() > 0 ? Sites[0] : NAME_None);
	UE_LOG(LogCL9, Log, TEXT("Destination board: '%s' (came from '%s', %d site(s) to choose from)."),
		*Destination.ToString(), *From.ToString(), Sites.Num());
}

void ACL9DestinationBoard::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACL9DestinationBoard, Destination);
}

FText ACL9DestinationBoard::GetDestinationName() const
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	return CL9StageText::SiteName(GameInstance != nullptr ? GameInstance->GetDefRegistry() : nullptr, Destination);
}

TArray<FName> ACL9DestinationBoard::GetChoices() const
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const TArray<FName> Sites = GameInstance != nullptr ? GameInstance->GetHostableSites() : TArray<FName>();
	const UCL9DefRegistry* Registry = GameInstance != nullptr ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9RulesDef* Rules = Registry != nullptr ? Registry->GetRules() : nullptr;
	const ACL9GameState* GameState = GetWorld() != nullptr ? GetWorld()->GetGameState<ACL9GameState>() : nullptr;
	if (Rules == nullptr || !Rules->bStageStars || GameState == nullptr)
	{
		return Sites;
	}
	return Sites.FilterByPredicate([&](FName Site) { return GameState->IsStageOpen(Site, Sites, Rules->StageUnlockStars); });
}

FText ACL9DestinationBoard::GetPrompt(const ACL9Character* Who) const
{
	// 고르는 것은 호스트의 몫이다(출발 레버와 같다) — 다른 사람은 지금 고른 곳만 본다.
	// {0} 은 캐릭터가 나중에 키 표기로 채운다(ACL9Character::GetFocusPrompt) — 여기서는 {1}(사이트 이름, 별이 있으면 별까지)만 채운다
	const bool bHost = Who != nullptr && Who->HasAuthority();
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance != nullptr ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9RulesDef* Rules = Registry != nullptr ? Registry->GetRules() : nullptr;
	const ACL9GameState* GameState = GetWorld()->GetGameState<ACL9GameState>();
	const int32 Stars = (Rules != nullptr && Rules->bStageStars && GameState != nullptr) ? GameState->GetSiteStars(Destination) : -1;
	if (Destination != CachedPromptFor || bHost != bCachedPromptHost || Stars != CachedPromptStars || CachedPrompt.IsEmpty())
	{
		CachedPromptFor = Destination;
		bCachedPromptHost = bHost;
		CachedPromptStars = Stars;
		const FText Pattern = bHost ? LOCTABLE("CL9", "Prompt.Destination") : LOCTABLE("CL9", "Prompt.DestinationHostOnly");
		CachedPrompt = FText::Format(Pattern, FText::FromString(TEXT("{0}")), CL9StageText::SiteWithStars(Registry, Destination, Stars));
	}
	return CachedPrompt;
}

bool ACL9DestinationBoard::CanInteract(const ACL9Character* Who) const
{
	// 정차 중에만 — 회사는 늘 정차(로비)다
	const ACL9GameState* GameState = GetWorld()->GetGameState<ACL9GameState>();
	return Who != nullptr && GameState != nullptr && GameState->GetPhase() == ECL9NightPhase::Lobby;
}

void ACL9DestinationBoard::Interact(ACL9Character* Who)
{
	if (!HasAuthority() || Who == nullptr)
	{
		return;
	}

	// 호스트 머신의 로컬 플레이어만 고른다 — 클라이언트의 누름은 프롬프트("호스트가 고른다")로 이미 답했다
	const ACL9PlayerController* Controller = Cast<ACL9PlayerController>(Who->GetController());
	if (Controller == nullptr || !Controller->IsLocalController())
	{
		UE_LOG(LogCL9, Log, TEXT("Destination board: '%s' is not the host — ignored."), *GetNameSafe(Who));
		return;
	}

	const TArray<FName> Sites = GetChoices();
	if (Sites.IsEmpty())
	{
		UE_LOG(LogCL9, Warning, TEXT("Destination board: no open site row has a Level — nothing to choose."));
		return;
	}

	const int32 Current = Sites.IndexOfByKey(Destination);
	Destination = Sites[(Current + 1) % Sites.Num()];
	ForceNetUpdate();
	UE_LOG(LogCL9, Log, TEXT("Destination board: '%s' chosen."), *Destination.ToString());
}
