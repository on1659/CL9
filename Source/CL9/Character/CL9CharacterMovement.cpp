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

	// Same formula on server and autonomous proxy — this is what keeps the
	// prediction from diverging (docs/systems/character.md 달리기 패턴 4).
	const bool bRunSpeedActive = bWantsToRun && !bExhausted;
	float MaxSpeed = bRunSpeedActive ? CharacterDef->RunSpeed : CharacterDef->WalkSpeed;

	// Cart drag rides a replicated flag so every machine derives the same
	// speed (docs/plan/data/item/cart.md · systems/character.md 유효 무게 절).
	const ACL9Character* CL9Owner = Cast<ACL9Character>(GetCharacterOwner());
	if (CL9Owner != nullptr && CL9Owner->GetPullingCart() != nullptr)
	{
		MaxSpeed *= CharacterDef->CartMult;
	}

	// Knockdown pins the character — the replicated end time makes server and
	// autonomous proxy agree on the zero (docs/systems/character.md 넘어짐).
	if (CL9Owner != nullptr && CL9Owner->IsKnockedDown())
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

	// 실이동 조건: holding the run key while standing still drains nothing —
	// dropping any of the three clauses breaks the escape-guarantee math
	// (docs/systems/character.md 스태미나 세 조항).
	const bool bReallyMoving = Velocity.SizeSquared2D() > UE_KINDA_SMALL_NUMBER;
	const bool bDraining = bWantsToRun && !bExhausted && bReallyMoving;

	if (bDraining)
	{
		Stamina -= DeltaSeconds / FMath::Max(CharacterDef->StaminaDrainTime, UE_KINDA_SMALL_NUMBER);
		if (Stamina <= 0.f)
		{
			Stamina = 0.f;
			bExhausted = true;
		}
	}
	else
	{
		Stamina = FMath::Min(Stamina + DeltaSeconds / FMath::Max(CharacterDef->StaminaRegenTime, UE_KINDA_SMALL_NUMBER), 1.f);
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

	// Replays restart from the snapshot so a server correction re-derives the
	// same stamina instead of double-draining it.
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
