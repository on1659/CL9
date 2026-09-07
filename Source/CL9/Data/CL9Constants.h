// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * 물리 상수만 모아둔다 — 복도 폭과 통로 간격을 정하는 기준 치수다
 * (docs/plan/data/character.md). 게임 밸런스 값(속도, 스태미나, 가격 등)은
 * 여기 두지 않는다. 그런 값은 DataAssets에 산다 (conventions § 3).
 */
namespace CL9
{
	/** 플레이어 캡슐 반지름 (cm) */
	inline constexpr float CapsuleRadius = 39.f;

	/** 플레이어 캡슐 절반 높이 (cm) */
	inline constexpr float CapsuleHalfHeight = 90.f;

	/** 바닥 기준 1인칭 카메라 높이 (cm) */
	inline constexpr float EyeHeight = 156.f;

	/** 엔진 기본 중력 크기 (cm/s²) — 캐릭터 테이블의 JumpGravity를 이 값으로 나눠 GravityScale을 만든다 */
	inline constexpr float DefaultGravityZ = 980.f;
}
