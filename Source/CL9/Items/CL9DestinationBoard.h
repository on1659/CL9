// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "World/CL9Fixture.h"
#include "Core/CL9Interactable.h"
#include "CL9DestinationBoard.generated.h"

/**
 * 목적지 판 — 회사 레벨에서 다음에 갈 사이트를 고르는 자리다(docs/goal/loop-7i-company.md). 호스트가 누를 때마다
 * 방을 만들 수 있는 사이트(`UCL9GameInstance::GetHostableSites`)를 표 순서로 하나씩 넘기고, 고른 것은 복제되어 모두의 프롬프트에 뜬다.
 * 회사의 출발 레버(`ACL9GameMode::RequestDepart`)가 이 값을 읽어 그 사이트의 레벨로 간다. 처음 값은 트럭이 떠나온 사이트다.
 * 메시는 배치된 인스턴스가 정한다(레벨 경로·메시 경로는 C++ 에 없다).
 */
UCLASS()
class ACL9DestinationBoard : public ACL9Fixture, public ICL9Interactable
{
	GENERATED_BODY()

public:
	ACL9DestinationBoard();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual FText GetPrompt(const ACL9Character* Who) const override;
	virtual bool CanInteract(const ACL9Character* Who) const override;
	virtual void Interact(ACL9Character* Who) override;

	/** 지금 고른 사이트 alias — 없으면 None */
	FName GetDestination() const
	{
		return Destination;
	}

protected:
	/** 고른 사이트 — 호스트만 바꾼다 */
	UPROPERTY(Replicated)
	FName Destination;

	/** 고른 사이트의 표시 이름(행의 DisplayName, 행이 없으면 alias) */
	FText GetDestinationName() const;

	/** 프롬프트는 매 틱 읽힌다 — 목적지·호스트 여부가 바뀔 때만 다시 서식한다(performance.md UI) */
	mutable FName CachedPromptFor;
	mutable bool bCachedPromptHost = false;
	mutable FText CachedPrompt;
};
