// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/CL9Interactable.h"
#include "CL9TruckPanel.generated.h"

class UStaticMeshComponent;

/**
 * 트럭 패널 — 번 이더로 장비를 사는 자리다(docs/systems/shop.md). 상인 NPC는 없고
 * 트럭에 붙은 패널 하나가 전부다. 이 액터는 화면을 여는 문일 뿐이며, 살 수 있는지와
 * 값을 치르는 일은 전부 게임 모드가 판정한다.
 *
 * 상호작용 판정은 호스트가 하지만 화면은 누른 사람의 것이므로, 호스트가 그 사람의
 * 컨트롤러로 `Client_OpenScreen`을 보내 그쪽 화면에서만 열리게 한다.
 */
UCLASS(abstract)
class ACL9TruckPanel : public AActor, public ICL9Interactable
{
	GENERATED_BODY()

public:
	ACL9TruckPanel();

	virtual FText GetPrompt(const ACL9Character* Who) const override;
	virtual bool CanInteract(const ACL9Character* Who) const override;
	virtual void Interact(ACL9Character* Who) override;

protected:
	/** 패널 몸체 — 메시는 BP 자식에서 지정된다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/**
	 * 열 UI 표의 행 이름 — C++에 상수로 박지 않고 데이터로 둔다. 상점 화면의 행 이름이
	 * 바뀌어도 코드를 고치지 않는다 (docs/systems/ui_system.md).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shop")
	FName ShopUIRow = FName("Shop");
};
