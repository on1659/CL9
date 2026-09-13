// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/CL9ActorBase.h"
#include "Data/CL9Types.h"
#include "CL9Item.generated.h"

class ACL9Character;
class UCL9ItemDef;
class UPhysicalMaterial;
class UPrimitiveComponent;
class USphereComponent;
class UStaticMeshComponent;

/** 아이템이 세상에 놓여 있는지, 캐릭터에 들려 있는지 — 수레 짐칸의 짐은 따로 상태가 없다(짐칸 안에 놓인 Free) */
UENUM()
enum class ECL9ItemHoldState : uint8
{
	Free,
	Held,
	/** 밀려 다니는 중(수레 그 자체) — 물리는 켜진 채 호스트가 매 틱 스프링 힘을 주고, 조준은 가능하며 무엇에도 부착되지 않는다 */
	Pushed,
	/** 캐릭터 머리에 착용된 상태(Head 운반 슬롯) — 시점을 따라가며 손은 계속 비어 있다 */
	Worn
};

/**
 * 모든 아이템의 바탕 액터(docs/systems/item.md). 메시·물리·운반 규칙이 전부 테이블 행에서
 * 나오고, 상태 변화(조리됨, 균열, 파손)는 새 액터를 스폰하는 대신 SubAlias를 바꿔치기해서
 * 표현한다. 물리 시뮬레이션과 모든 판정은 호스트가 쥐고 있고, 클라이언트는 복제된 이동값을
 * 보간할 뿐이다. 행동이 다른 종류(빛나는 것, 수레)는 파생 클래스로 갈라지고, 행의
 * ActorClass가 그 클래스를 가리킨다 — item · lightitem · flashitem (2026-09-11).
 */
UCLASS(abstract)
class ACL9Item : public ACL9ActorBase
{
	GENERATED_BODY()

public:
	ACL9Item();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual FText GetDisplayName() const override;

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
	 * 호스트 전용: 굴린 값을 덮어쓴다 — 핫바 비활성 칸에서 다시 스폰될 때 칸에 저장해 둔 값을
	 * 되돌리는 용도다(칸 전환으로 가격이 다시 굴려지지 않는다, docs/systems/inventory.md).
	 * 이후 행이 바뀌면(익음·탐·깨짐) 새 범위로 다시 굴리는 규칙은 그대로다.
	 */
	void HostSetValue(int32 NewValue);

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

	/**
	 * 호스트 전용: 소지자 전신 메시의 손 소켓(`HandGrip_R`, 없으면 `hand_r`)에 행의 GripOffset/
	 * GripRotation으로 부착한다. 팔 메시는 전신 포즈를 복사하므로 자기 화면에서도 같은 손에 들려
	 * 보이고, 소유자 화면에서는 팔과 같은 1인칭 렌더 공간(FOV·스케일)으로 그린다.
	 */
	void OnPickedUp(ACL9Character* Holder);

	virtual void OnRep_Owner() override;

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

	/** 호스트 전용: 분리한 뒤 주어진 월드 위치에 살며시 내려놓는다 */
	void OnPlaced(const FVector& WorldLocation);

	/** 호스트 전용: 분리한 뒤 던진다 — 충돌 판정은 행의 문턱값에 따라 금이 가거나 부서진다 */
	void OnThrown(const FVector& Origin, const FVector& LaunchVelocity);

	UStaticMeshComponent* GetMeshComponent() const
	{
		return MeshComponent;
	}

	/**
	 * 실제 물리 몸체 — 루트 프리미티브다. 보통은 반경 구이고, 행의 bMeshIsBody가 켜진 물건(수레)은 로드 때
	 * 루트가 된 메시다. 힘을 주거나 깨우거나 시뮬레이션을 켜고 끄는 코드는 전부 이것을 거친다.
	 */
	UPrimitiveComponent* GetBody() const;

protected:
	virtual void LoadData() override;

	/**
	 * 호스트 전용 훅: 누군가의 손에 들리거나 머리에 씌워진 직후(부착 전) / 내려놓거나
	 * 던져져 세상으로 돌아간 직후. 파생 클래스가 소지 여부에 따르는 상태(빛)를 여기서 바꾼다.
	 */
	virtual void OnAcquired() {}
	virtual void OnReleased() {}

	UFUNCTION()
	void OnRep_HoldState();

	/** 호스트, 물리 행 전용: 실제 충돌로부터 충격 판정을 내린다 */
	UFUNCTION()
	void OnPhysicsHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	/** 이 머신에서 현재 소지 상태에 따른 충돌/시뮬레이션 부수 효과를 적용한다 */
	void ApplyHoldState();

	/** 소유자 본인 화면에서만 손에 든 아이템을 1인칭 팔과 같은 렌더 공간으로 옮긴다 */
	void UpdateFirstPersonView();

	/** 수레 서브클래스와 공유하는 상태 — 수레는 자기만의 견인 전이를 따로 처리한다 */
	UPROPERTY(ReplicatedUsing=OnRep_HoldState)
	ECL9ItemHoldState HoldState = ECL9ItemHoldState::Free;

	/** 호스트 전용 시뮬레이션 상태 — 클라이언트는 복제된 이동만 본다 */
	FVector SimVelocity = FVector::ZeroVector;
	bool bAtRest = false;

private:
	/** 호스트 전용: 균열/파손 전이 — 세 문턱값에 따라 SubAlias를 바꾼다 */
	void HandleImpact(float ImpactSpeed);

	/**
	 * bMeshIsBody 행: 메시를 루트이자 물리 몸체로 승격한다 — 구는 콜리전을 끄고 메시 아래로 내려간다. 모든 머신에서
	 * 로드 때 한 번 일어나며(재로드는 무시), 복제 이동(bRepPhysics)이 새 루트를 따르도록 엔진 동기화를 다시 맞춘다.
	 */
	void PromoteMeshToBody();

	/** 메시 몸체의 실제 시뮬레이션 모양을 로그로 남긴다 — 심플 콜리전 없음은 Error, 렌더 바운드보다 훨씬 크면(UCX 배율) Warning */
	void LogBodyDiagnostics() const;

	/** 충돌 구체 — 보통 루트 컴포넌트이며, 반지름은 행이 정한 판정값이다. bMeshIsBody 행은 로드 때 콜리전 없이 메시 아래로 내려간다 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USphereComponent> SphereComponent;

	/**
	 * 행에서 가져온 시각 메시 — 실제 에셋이 들어오기 전까지는 충돌 반지름에 맞춰 자동으로 크기가 조정된다.
	 * bMeshIsBody 행은 자동 스케일 없이 실물 크기 그대로이고, 임포트한 UCX 콜리전으로 시뮬레이션하는 루트다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UStaticMeshComponent> MeshComponent;

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
