// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "World/CL9Fixture.h"
#include "Core/CL9Interactable.h"
#include "CL9Jar.generated.h"


/**
 * 반납 항아리(docs/systems/raiding.md 항아리 반납). 손에 든 아이템을 받아 게임
 * 상태의 반납 목록에 기록한다. 정전 시계를 DarkAccel만큼 앞당기는 동작은 밤
 * 시계가 생기면(build-order ④) 함께 들어온다. 정적인 고정물이다 — 상태도
 * 복제도 없으며, 메시는 BP 자식 쪽에 있다.
 */
UCLASS(abstract)
class ACL9Jar : public ACL9Fixture, public ICL9Interactable
{
	GENERATED_BODY()

public:
	ACL9Jar();

	virtual FText GetPrompt(const ACL9Character* Who) const override;
	virtual bool CanInteract(const ACL9Character* Who) const override;
	virtual void Interact(ACL9Character* Who) override;
};
