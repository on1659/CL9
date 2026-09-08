// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "Data/CL9Types.h"
#include "CL9BuffDef.generated.h"

/**
 * 버프 행 하나 (docs/plan/data/buff.md). 먹어서 걸리는 상태 변화에 **이름을 주는** 표다.
 *
 * 전에는 효과가 아이템 행 안에 인라인으로 들어 있어서, 같은 효과를 두 음식이 주면 값을
 * 두 번 적어야 했고 무엇보다 **이름이 없었다** — 화면에 띄울 수 있는 것이 "달리기 +60"
 * 같은 수치뿐이었다. 이제 아이템 행은 `EatBuffs`로 이 표의 별칭을 가리키기만 한다.
 *
 * **좋고 나쁨을 가르는 것은 값의 부호다** — 양수면 버프, 음수면 디버프이며 별도의
 * 플래그를 두지 않는다. 한 행이 양수와 음수를 함께 담으면 그것이 양날 효과다.
 */
UCLASS()
class UCL9BuffDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	/**
	 * 이 버프가 거는 스탯 변화들. 여럿을 담으면 한 번에 같이 걸리고 같이 끝난다 —
	 * 지속시간은 항목마다 따로 적으므로 서로 다르게 둘 수도 있다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Buff")
	TArray<FCL9StatEffect> Effects;
};
