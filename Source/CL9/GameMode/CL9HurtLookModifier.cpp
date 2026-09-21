// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9HurtLookModifier.h"
#include "Camera/CameraTypes.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/World.h"
#include "Character/CL9Character.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9RulesDef.h"
#include "GameMode/CL9GameInstance.h"

namespace
{
	/** EaseOut 의 지수 — 2 면 처음 빠르고 끝이 부드럽다. 곡선 모양이지 밸런스 값이 아니다 */
	constexpr float EaseExponent = 2.f;

	/** 카메라 피치의 한계, 도 단위 — 바닥을 보던 시선에 오프셋을 더해도 카메라가 뒤집히지 않게 */
	constexpr float PitchLimit = 89.f;

	/** 0..1 을 EaseOut 곡선으로 — 1 에서 정확히 1 이라 도달이 보장된다 */
	float EaseOut(float Alpha)
	{
		return FMath::InterpEaseOut(0.f, 1.f, FMath::Clamp(Alpha, 0.f, 1.f), EaseExponent);
	}
}

void UCL9HurtLookModifier::SetCharacter(ACL9Character* InCharacter)
{
	Character = InCharacter;
	ResetOffsets();
}

void UCL9HurtLookModifier::ResetOffsets()
{
	PitchOffset = 0.f;
	YawOffset = 0.f;
	YawTarget = 0.f;
	YawFrom = 0.f;
	YawKnockdownStart = -1000.f;
}

bool UCL9HurtLookModifier::ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
	// 관전 중(뷰 타깃이 남의 폰)이면 그 사람의 카메라에 내 피격을 얹으면 안 된다. 처형 중(뷰 타깃이 몬스터의 처형 카메라 —
	// 블렌드 중이거나 처형의 HostKill 이 넘어짐 시각을 찍은 뒤)도 같다 (D6). 죽어서 데스캠(뷰 타깃은 폰이지만 시체를 보는
	// 3인칭)이면 역시 아무것도 더하지 않는다 — 오프셋 0 롤백이면 옛대로 죽음의 넘어짐 카메라에 유지된다
	ACL9Character* Pawn = Character.Get();
	if (Pawn == nullptr || CameraOwner == nullptr || CameraOwner->GetViewTarget() != Pawn || Pawn->IsExecuted() || Pawn->ShouldUseDeathCamera())
	{
		ResetOffsets();
		return false;
	}

	const UCL9GameInstance* GameInstance = Pawn->GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance != nullptr ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9RulesDef* Rules = Registry != nullptr ? Registry->GetRules() : nullptr;
	if (Rules == nullptr || Rules->HurtLookTime <= 0.f)
	{
		ResetOffsets();
		return false;
	}

	// 넘어져 있지 않으면(일어났거나 부활) 아무것도 더하지 않는다. 죽음은 넘어짐이 무한이라 진입 뒤 그대로 유지된다
	const float Elapsed = Pawn->GetKnockdownElapsed();
	if (Elapsed < 0.f)
	{
		ResetOffsets();
		return false;
	}

	// 진입은 넘어짐 시계만으로(사건 복제가 늦어도 피치는 먼저 간다), 복귀는 일어나기 구간에서 — 로드 검사가 둘이 겹치지 않음을 보장한다
	const float EnterAlpha = EaseOut(Elapsed / Rules->HurtLookTime);
	const float ExitAlpha = EaseOut(Pawn->GetKnockdownRemaining() / CL9HurtLook::GetUpSeconds);
	PitchOffset = Rules->HurtLookPitch * EnterAlpha * ExitAlpha;

	// 요 목표: 이번 넘어짐의 사건이 처음 보이거나(복제 순서 역전 대비) 재피격(Seq 변화)이면 다시 잰다 — 사건 순간의 컨트롤 요 기준으로
	// 공격자가 화면 가장자리에 걸치는 만큼만, HurtLookYawMax 안에서. 피치 진입은 다시 시작하지 않는다
	const FCL9HurtEvent& Event = Pawn->GetHurtEvent();
	const double Now = Pawn->GetWorld()->GetTimeSeconds();
	if (Pawn->IsHurtEventCurrent() && (Event.Seq != LastSeq || Event.ServerTime != YawKnockdownStart))
	{
		FVector ToAttacker = FVector(Event.From) - Pawn->GetActorLocation();
		ToAttacker.Z = 0.f;
		float NewTarget = 0.f;
		if (!ToAttacker.IsNearlyZero() && Rules->HurtLookYawMax > 0.f)
		{
			const float Delta = FRotator::NormalizeAxis(ToAttacker.Rotation().Yaw - Pawn->GetControlRotation().Yaw);
			const float HalfFov = InOutPOV.FOV * 0.5f;
			const float Beyond = FMath::Max(FMath::Abs(Delta) - HalfFov, 0.f);
			NewTarget = FMath::Sign(Delta) * FMath::Min(Beyond, Rules->HurtLookYawMax);
		}
		YawFrom = YawOffset;
		YawTarget = NewTarget;
		YawRetargetTime = static_cast<float>(Now);
		LastSeq = Event.Seq;
		YawKnockdownStart = Event.ServerTime;
	}
	const float YawAlpha = EaseOut((static_cast<float>(Now) - YawRetargetTime) / Rules->HurtLookTime);
	YawOffset = (YawFrom + (YawTarget - YawFrom) * YawAlpha) * ExitAlpha;

	// POV 에만 더한다 — 컨트롤 회전(조준·시선 입력)은 그대로다
	InOutPOV.Rotation.Pitch = FMath::Clamp(FRotator::NormalizeAxis(InOutPOV.Rotation.Pitch + PitchOffset), -PitchLimit, PitchLimit);
	InOutPOV.Rotation.Yaw = FRotator::NormalizeAxis(InOutPOV.Rotation.Yaw + YawOffset);
	return false;
}
