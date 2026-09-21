// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/CL9CharacterMovement.h"
#include "Character/CL9Character.h"
#include "Data/CL9CharacterDef.h"
#include "GameFramework/Character.h"

float UCL9CharacterMovement::GetMaxSpeed() const
{
	const float BaseSpeed = Super::GetMaxSpeed();

	if (CharacterDef == nullptr || (!IsMovingOnGround() && !IsFalling()))
	{
		return BaseSpeed;
	}

	// 서버와 자율 프록시가 같은 공식을 쓴다 — 이게 예측이 어긋나지 않게
	// 지켜주는 부분이다 (docs/systems/character.md 달리기 패턴 4).
	const bool bRunSpeedActive = bWantsToRun && !bExhausted;
	float MaxSpeed = bRunSpeedActive ? CharacterDef->RunSpeed : CharacterDef->WalkSpeed;

	// 먹기 효과는 복제되고 서버 시각으로 만료되므로 양쪽이 같은 값을 더한다
	// (docs/plan/data/item/common.md 타입과 먹기 — 효과는 표가 정한다)
	const ACL9Character* CL9Owner = Cast<ACL9Character>(GetCharacterOwner());
	if (CL9Owner != nullptr)
	{
		MaxSpeed += CL9Owner->GetStatBonus(bRunSpeedActive ? ECL9Stat::RunSpeed : ECL9Stat::WalkSpeed);
		MaxSpeed = FMath::Max(MaxSpeed, 0.f);
	}

	// 카트 밀기는 리플리케이트되는 플래그를 타므로 모든 머신이 같은
	// 속도를 도출한다 (docs/plan/data/item/cart.md · systems/character.md 유효 무게 절).
	if (CL9Owner != nullptr && CL9Owner->GetPullingCart() != nullptr)
	{
		MaxSpeed *= CharacterDef->CartMult + CL9Owner->GetStatBonus(ECL9Stat::CartMult);
	}

	// 넘어짐은 캐릭터를 붙들어둔다 — 리플리케이트되는 종료 시각 덕분에 서버와
	// 자율 프록시가 속도 0에 대해 서로 합의한다 (docs/systems/character.md 넘어짐).
	// 처형도 같다 — 복제된 처형 시작 시각(ExecuteStartServerTime)에서 양쪽이 같은 0 을 도출한다 (D2)
	if (CL9Owner != nullptr && (CL9Owner->IsKnockedDown() || CL9Owner->IsExecuted()))
	{
		MaxSpeed = 0.f;
	}
	return MaxSpeed;
}

FNetworkPredictionData_Client* UCL9CharacterMovement::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UCL9CharacterMovement* MutableThis = const_cast<UCL9CharacterMovement*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_CL9(*this);
	}
	return ClientPredictionData;
}

void UCL9CharacterMovement::SetCharacterDef(const UCL9CharacterDef* InCharacterDef)
{
	CharacterDef = InCharacterDef;
}

void UCL9CharacterMovement::SetWantsToRun(bool bNewWantsToRun)
{
	bWantsToRun = bNewWantsToRun;
}

bool UCL9CharacterMovement::IsRunning() const
{
	return bWantsToRun && !bExhausted && Velocity.SizeSquared2D() > UE_KINDA_SMALL_NUMBER;
}

void UCL9CharacterMovement::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bWantsToRun = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

void UCL9CharacterMovement::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

	if (CharacterDef == nullptr)
	{
		return;
	}

	// 먹기 효과는 복제되고 시각으로 만료되므로 서버와 자율 프록시가 같은 값을 더한다
	const ACL9Character* CL9Owner = Cast<ACL9Character>(GetCharacterOwner());
	auto Bonus = [CL9Owner](ECL9Stat Stat)
	{
		return CL9Owner != nullptr ? CL9Owner->GetStatBonus(Stat) : 0.f;
	};

	// 실이동 조건: 제자리에 서서 달리기 키만 누르고 있으면 스태미나가 줄지 않는다 —
	// 세 조항 중 하나라도 빠지면 탈출 보장 계산이 깨진다
	// (docs/systems/character.md 스태미나 세 조항).
	const bool bReallyMoving = Velocity.SizeSquared2D() > UE_KINDA_SMALL_NUMBER;
	const bool bDraining = bWantsToRun && !bExhausted && bReallyMoving;

	if (bDraining)
	{
		const float DrainTime = CharacterDef->StaminaDrainTime + Bonus(ECL9Stat::StaminaDrainTime);
		Stamina -= DeltaSeconds / FMath::Max(DrainTime, UE_KINDA_SMALL_NUMBER);
		if (Stamina <= 0.f)
		{
			Stamina = 0.f;
			bExhausted = true;
		}
	}
	else
	{
		const float RegenTime = CharacterDef->StaminaRegenTime + Bonus(ECL9Stat::StaminaRegenTime);
		Stamina = FMath::Min(Stamina + DeltaSeconds / FMath::Max(RegenTime, UE_KINDA_SMALL_NUMBER), 1.f);
		if (bExhausted && Stamina >= CharacterDef->StaminaMinResume)
		{
			bExhausted = false;
		}
	}
}

FSavedMove_CL9::FSavedMove_CL9()
	: bSavedWantsToRun(false)
	, bSavedExhausted(false)
	, SavedStamina(1.f)
{
}

void FSavedMove_CL9::Clear()
{
	Super::Clear();

	bSavedWantsToRun = false;
	bSavedExhausted = false;
	SavedStamina = 1.f;
}

uint8 FSavedMove_CL9::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedWantsToRun)
	{
		Result |= FLAG_Custom_0;
	}
	return Result;
}

bool FSavedMove_CL9::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_CL9* NewMoveCL9 = static_cast<const FSavedMove_CL9*>(NewMove.Get());

	if (NewMoveCL9->bSavedWantsToRun != bSavedWantsToRun)
	{
		return false;
	}
	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FSavedMove_CL9::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	const UCL9CharacterMovement* Movement = static_cast<UCL9CharacterMovement*>(Character->GetCharacterMovement());
	bSavedWantsToRun = Movement->bWantsToRun;
	bSavedExhausted = Movement->bExhausted;
	SavedStamina = Movement->Stamina;
}

void FSavedMove_CL9::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	// 리플레이는 스냅샷에서 다시 시작하므로, 서버 보정이 스태미나를 이중으로
	// 깎는 대신 같은 값을 다시 도출하게 된다.
	UCL9CharacterMovement* Movement = static_cast<UCL9CharacterMovement*>(Character->GetCharacterMovement());
	Movement->bWantsToRun = bSavedWantsToRun;
	Movement->bExhausted = bSavedExhausted;
	Movement->Stamina = SavedStamina;
}

FNetworkPredictionData_Client_CL9::FNetworkPredictionData_Client_CL9(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr FNetworkPredictionData_Client_CL9::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_CL9());
}
