// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Physical constants only — geometry the corridor width and chokepoint distances
 * are built on (docs/plan/data/character.md). Game balance values (speeds,
 * stamina, prices...) never go here; they live in DataAssets (conventions § 3).
 */
namespace CL9
{
	/** Player capsule radius in cm */
	inline constexpr float CapsuleRadius = 39.f;

	/** Player capsule half height in cm */
	inline constexpr float CapsuleHalfHeight = 90.f;

	/** First person camera height above the floor in cm */
	inline constexpr float EyeHeight = 156.f;

	/** Engine default gravity magnitude in cm/s² — JumpGravity from the character table divides by this to produce GravityScale */
	inline constexpr float DefaultGravityZ = 980.f;
}
