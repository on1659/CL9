// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/CL9ActorBase.h"
#include "Data/CL9Types.h"
#include "CL9Item.generated.h"

class ACL9Character;
class UCL9ItemDef;
class ULightComponent;
class UPhysicalMaterial;
class USphereComponent;
class UStaticMeshComponent;

/** 아이템이 세상에 놓여 있는지, 캐릭터에 들려 있는지, 수레 짐칸에 실려 있는지 */
UENUM()
enum class ECL9ItemHoldState : uint8
{
	Free,
	Held,
	/** 수레 짐칸에 실린 상태 — 물리는 꺼지지만 여전히 조준하고 집을 수 있다 */
	OnCart,
	/** 견인되는 중(수레 그 자체) — 물리는 꺼지지만 조준은 가능하고, 무엇에도 부착되지 않는다 */
	Pulled,
	/** 캐릭터 머리에 착용된 상태(Head 운반 슬롯) — 시점을 따라가며 손은 계속 비어 있다 */
	Worn
};

/**
 * 모든 아이템이 공유하는 하나의 액터(docs/systems/item.md). 아이템 종류가 백 개여도
 * 클래스는 하나다 — 메시·물리·운반 규칙이 전부 테이블 행에서 나오고, 상태 변화(조리됨,
 * 균열, 파손)는 새 액터를 스폰하는 대신 SubAlias를 바꿔치기해서 표현한다. 물리 시뮬레이션과
 * 모든 판정은 호스트가 쥐고 있고, 클라이언트는 복제된 이동값을 보간할 뿐이다.
 */
UCLASS(abstract)
class ACL9Item : public ACL9ActorBase
{
	GENERATED_BODY()

public:
	ACL9Item();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 이 아이템이 지금 표시하는 행 — 별칭이 로드되기 전에는 null */
	const UCL9ItemDef* GetItemDef() const
	{
		return ItemDef;
	}

	/**
	 * 이 개체의 판매가(이더) — 호스트가 행을 로드할 때 [ValueMin, ValueMax]에서 굴린 값이고,
	 * 상태가 바뀌면(익음·탐·깨짐) 새 행의 범위로 다시 굴린다 (docs/plan/data/item/common.md 가치).
	 * 목적지·일자 배율은 판매·납품 시점(4e)에 곱한다. 0 = 값이 없는 물건.
	 */
	UFUNCTION(BlueprintPure, Category="Item")
	int32 GetValue() const
	{
		return Value;
	}

	/**
	 * 호스트 전용: 조리대가 매 틱 이 아이템의 조리 상태를 실어 준다 (docs/systems/cooking.md).
	 * 판정은 조리대가 하고 여기 담기는 것은 표현용 사본이다 — 게이지를 그릴 수 있게 복제된다.
	 * Alpha는 0~1이며, 열원을 벗어나면 조리대가 None으로 지운다.
	 */
	void SetCookState(ECL9CookPhase Phase, float Alpha);

	/** 지금 상태에서 다음 상태(익음 또는 탐)까지의 진행도 0~1 — 열원 위가 아니면 0 */
	UFUNCTION(BlueprintPure, Category="Item")
	float GetCookProgress() const
	{
		return CookAlpha / 255.f;
	}

	/** 열원 위에서 무언가로 변해 가는 중이면 true — 게이지를 띄울지 판단한다 */
	UFUNCTION(BlueprintPure, Category="Item")
	bool IsOnHeat() const
	{
		return CookPhase != ECL9CookPhase::None;
	}

	/** 다음 전이가 "탐"이면 true — 표현이 경고로 바뀌는 지점이다 */
	UFUNCTION(BlueprintPure, Category="Item")
	bool IsNextBurnt() const
	{
		return CookPhase == ECL9CookPhase::Burning;
	}

	UFUNCTION(BlueprintPure, Category="Item")
	ECL9CookPhase GetCookPhase() const
	{
		return CookPhase;
	}

	/** 자유 상태이고, 사용 가능하며, 수레 전용이 아닌지 (docs/plan/data/item/common.md 운반) */
	bool CanBePickedUp() const;

	/** 아이템이 세상에 놓여 있는(누구에게도 부착되지 않은) 동안 true */
	bool IsFree() const
	{
		return HoldState == ECL9ItemHoldState::Free;
	}

	/** 호스트 전용: 소지자의 캡슐에 주어진 상대 위치로 부착한다 */
	void OnPickedUp(ACL9Character* Holder, const FVector& RelativeLocation);

	/**
	 * 호스트 전용: 착용자의 머리에 씌운다(Head 운반 슬롯 — docs/plan/data/item/headlamp.md).
	 * 1인칭 카메라에 붙어 다니므로 빛줄기가 착용자의 시선을 따라가고, 착용자 본인에게는
	 * 램프 자체가 보이지 않는다. 착용한 조명은 정전을 무시한다 — 정전은 부지의 설비를
	 * 죽이는 것이지, 가져온 물건을 죽이는 게 아니다(2026-09-05).
	 */
	void OnWorn(ACL9Character* Wearer);

	/** 머리에 착용된 동안 true — 정전은 착용한 조명을 건드리지 않는다 */
	bool IsWorn() const
	{
		return HoldState == ECL9ItemHoldState::Worn;
	}

	/** 호스트 전용: 수레 짐칸에 주어진 상대 위치로 부착한다 (docs/plan/data/item/cart.md 적재) */
	void OnLoadedIntoCart(AActor* Cart, const FVector& RelativeLocation);

	/** 호스트 전용: 분리한 뒤 주어진 월드 위치에 살며시 내려놓는다 */
	void OnPlaced(const FVector& WorldLocation);

	/** 호스트 전용: 분리한 뒤 던진다 — 충돌 판정은 행의 문턱값에 따라 금이 가거나 부서진다 */
	void OnThrown(const FVector& Origin, const FVector& LaunchVelocity);

	UStaticMeshComponent* GetMeshComponent() const
	{
		return MeshComponent;
	}

	/** 이 아이템의 조명 스위치가 켜져 있는 동안 true (발광: 소지 중이면 켜짐, 손전등: 토글로 켜짐) */
	bool IsLightOn() const
	{
		return bLightOn;
	}

	/** 행이 이 아이템에 토글식 손전등 기능을 부여했으면 true (docs/systems/light.md) */
	bool IsFlashlight() const;

	/** 호스트 전용: 손전등을 켜고 끈다 — 정전 중에는 거부된다 (light.md) */
	void SetLightSwitch(bool bOn);

protected:
	virtual void LoadData() override;

	UFUNCTION()
	void OnRep_HoldState();

	UFUNCTION()
	void OnRep_LightOn();

	/** 행으로부터 점광원/스포트라이트를 만든다 (로드 시 한 번만) */
	void SetupLight();

	/** 이 머신에서의 실효 조명 표시 여부를 반영한다 (정전은 손전등을 강제로 끈다) */
	void UpdateLightVisibility();

	/** 호스트, 물리 행 전용: 실제 충돌로부터 충격 판정을 내린다 */
	UFUNCTION()
	void OnPhysicsHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	/** 이 머신에서 현재 소지 상태에 따른 충돌/시뮬레이션 부수 효과를 적용한다 */
	void ApplyHoldState();

	/** 수레 서브클래스와 공유하는 상태 — 수레는 자기만의 견인 전이를 따로 처리한다 */
	UPROPERTY(ReplicatedUsing=OnRep_HoldState)
	ECL9ItemHoldState HoldState = ECL9ItemHoldState::Free;

	/** 조명 스위치 상태(호스트가 기록) — 발광은 소지 여부를 따르고, 손전등은 토글을 따른다 */
	UPROPERTY(ReplicatedUsing=OnRep_LightOn)
	bool bLightOn = false;

	/** 호스트 전용 시뮬레이션 상태 — 클라이언트는 복제된 이동만 본다 */
	FVector SimVelocity = FVector::ZeroVector;
	bool bAtRest = false;

private:
	/** 호스트 전용: 균열/파손 전이 — 세 문턱값에 따라 SubAlias를 바꾼다 */
	void HandleImpact(float ImpactSpeed);

	/** 충돌 구체 — 루트 컴포넌트이며, 반지름은 행이 정한 판정값이다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> SphereComponent;

	/** 행에서 가져온 시각 메시 — 실제 에셋이 들어오기 전까지는 충돌 반지름에 맞춰 자동으로 크기가 조정된다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	/** bEmitsLight일 때 행으로부터 만들어지는 점광원(발광) 또는 스포트라이트(손전등) — 아니면 null */
	UPROPERTY()
	TObjectPtr<ULightComponent> LightComponent;

	UPROPERTY()
	TObjectPtr<const UCL9ItemDef> ItemDef;

	/** 개체별로 굴린 판매가 — 호스트가 정하고 복제한다 */
	UPROPERTY(Replicated)
	int32 Value = 0;

	/** 조리 진행도를 0~255로 눌러 담은 값 — 표현 전용이라 이 정밀도면 충분하다 */
	UPROPERTY(Replicated)
	uint8 CookAlpha = 0;

	/** 지금 열원 위에서 무엇이 되는 중인지 — 열 밖이면 None */
	UPROPERTY(Replicated)
	ECL9CookPhase CookPhase = ECL9CookPhase::None;

	/** 물리 경로용 행별 마찰/반발 계수 — 로드 시 테이블로부터 만들어진다 */
	UPROPERTY()
	TObjectPtr<UPhysicalMaterial> PhysicsMaterial;
};
