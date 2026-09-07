// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CL9CharacterMovement.generated.h"

class UCL9CharacterDef;

/**
 * 예측 무브먼트. 달리기 플래그와 스태미나는 PlayerState가 아니라 여기에 있다 —
 * 그래야 서버와 자율 프록시가 같은 압축 입력 플래그에서 같은 속도를 도출해서
 * 러버밴딩이 생기지 않는다 (docs/systems/character.md,
 * conventions § 4 예외). 속도와 스태미나 타이밍은 코드에 저장하지 않고
 * 캐릭터 테이블 행에서 실시간으로 읽는다.
 */
UCLASS()
class UCL9CharacterMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	virtual float GetMaxSpeed() const override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	/** 캐릭터의 테이블 행이 적용될 때 호출된다; 이 행은 폰보다 오래 산다 (레지스트리 소유) */
	void SetCharacterDef(const UCL9CharacterDef* InCharacterDef);

	/** 로컬 입력 전용 — 이 플래그는 직접적인 속도 변경이 아니라 압축된 무브 플래그 안에 실려 서버로 간다 */
	void SetWantsToRun(bool bNewWantsToRun);

	/** 달리기 속도가 적용되어 캐릭터가 실제로 움직이는 동안 true (애님 블루프린트가 이 값을 읽는다) */
	UFUNCTION(BlueprintPure, Category="CL9")
	bool IsRunning() const;

	/** 0..1로 정규화된 값 (docs/plan/data/character.md 스태미나) */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetStamina() const
	{
		return Stamina;
	}

	/** 탈진 래치 — 스태미나가 StaminaMinResume을 넘기 전까지는 달리기가 불가능하다 */
	UFUNCTION(BlueprintPure, Category="CL9")
	bool IsExhausted() const
	{
		return bExhausted;
	}

protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

private:
	/** 로컬 테이블 행 — 모든 머신에서 같은 에셋이다 (conventions § 5) */
	UPROPERTY()
	TObjectPtr<const UCL9CharacterDef> CharacterDef;

	/** 달리기 입력 상태, FSavedMove_CL9가 무브마다 실어 나른다 */
	bool bWantsToRun = false;

	/** 0..1 — 무브 경로 안에서 시뮬레이션되므로 서버 보정이 이걸 결정론적으로 재생할 수 있다 */
	float Stamina = 1.f;

	/** 탈진 래치 (docs/systems/character.md 스태미나 세 조항) */
	bool bExhausted = false;

	friend class FSavedMove_CL9;
};

/** 달리기 플래그와 스태미나 스냅샷을 함께 실어 나르는 저장된 무브 — 리플레이가 올바른 상태에서 다시 시작하도록 한다 */
class FSavedMove_CL9 : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;

	FSavedMove_CL9();

	uint8 bSavedWantsToRun : 1;
	uint8 bSavedExhausted : 1;
	float SavedStamina;

	virtual void Clear() override;
	virtual uint8 GetCompressedFlags() const override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
	virtual void PrepMoveFor(ACharacter* Character) override;
};

/** 자율 프록시를 위해 FSavedMove_CL9를 할당한다 */
class FNetworkPredictionData_Client_CL9 : public FNetworkPredictionData_Client_Character
{
public:
	typedef FNetworkPredictionData_Client_Character Super;

	FNetworkPredictionData_Client_CL9(const UCharacterMovementComponent& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;
};
