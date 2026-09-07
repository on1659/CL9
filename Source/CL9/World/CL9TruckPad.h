// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CL9TruckPad.generated.h"

class ACL9Character;
class UBoxComponent;
class UStaticMeshComponent;

/**
 * 트럭 패드 — 유일한 회복 수단이다(docs/plan/data/rules.md 죽음과 회복,
 * docs/plan/loop.md 귀환). 살아있으면서 부상당한 캐릭터가 HealZone 안에
 * TruckPadHealTime초만큼 서 있으면 최대 체력까지 회복하고, 밖으로 나가면
 * 그 시간이 초기화된다. 죽은 캐릭터는 여기서 회복되지 않는다(부활은 없다) —
 * 다음 로비에서 이 패드 위에 다시 세워진다(ACL9GameMode::ReviveDead). 고정
 * 시설물에 대한 판정은 호스트만 하며, 아무것도 복제되지 않고 체력값은
 * PlayerState를 통해 전달된다. 메시와 존 크기는 BP 자식 클래스에 있다.
 */
UCLASS(abstract)
class ACL9TruckPad : public AActor
{
	GENERATED_BODY()

public:
	ACL9TruckPad();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

private:
	/** 패드 본체 — 메시는 BP 자식 클래스에서 지정된다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** 안에 서 있으면 카운트된다 — 폰(pawn)과의 오버랩만 감지한다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UBoxComponent> HealZone;

	/** 호스트 전용: 부상당한 캐릭터가 나가지 않고 여기 서 있었던 시간(초) */
	TMap<TWeakObjectPtr<ACL9Character>, float> TimeOnPad;
};
