// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CL9Interactable.generated.h"

class ACL9Character;

UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UCL9Interactable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 상호작용을 위한 유일한 틀 — 항아리 반납, 트럭 패널, 카운터가 모두 새 입력
 * 경로를 늘리는 대신 이걸 구현한다 (docs/systems/input.md
 * 상호작용 공통 틀). RPC가 아니라 일반 함수다: 이 액터들은 소유자가 없으므로
 * 요청은 캐릭터의 Server RPC를 거쳐 도착하고, 호스트는 재검사 후 Interact를
 * 직접 호출한다.
 */
class ICL9Interactable
{
	GENERATED_BODY()

public:
	/** HUD 프롬프트, 예: "[E] 반납" — 표시 전용 */
	virtual FText GetPrompt(const ACL9Character* Who) const = 0;

	/** 잠금/거리/상태 확인 — 호스트가 Interact 전에 다시 실행한다 */
	virtual bool CanInteract(const ACL9Character* Who) const = 0;

	/** 호스트 전용 */
	virtual void Interact(ACL9Character* Who) = 0;
};
