// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "World/CL9Fixture.h"
#include "Core/CL9Interactable.h"
#include "CL9SkinCloset.generated.h"

/**
 * 옷장 — 회사에서 스킨을 사고 입는 화면(`UCL9SkinShopWidget`, UI 행 `SkinShop`)을 여는 자리(docs/goal/loop-7k-skins.md, 사용자 "상점·마이페이지는 회사 안").
 * 누구나 연다 — 스킨은 각자의 것이라 호스트가 정할 일이 없다. 화면은 누른 사람의 것이므로 호스트가 그 사람의 컨트롤러로만 연다.
 * 메시는 배치된 인스턴스가 정한다.
 */
UCLASS()
class ACL9SkinCloset : public ACL9Fixture, public ICL9Interactable
{
	GENERATED_BODY()

public:
	virtual FText GetPrompt(const ACL9Character* Who) const override;
	virtual bool CanInteract(const ACL9Character* Who) const override;
	virtual void Interact(ACL9Character* Who) override;

protected:
	/** 열 UI 표의 행 이름 — 데이터 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skin")
	FName SkinUIRow = FName("SkinShop");
};
