// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CL9Types.generated.h"

/** How an item can be carried (docs/plan/data/item/common.md 운반) */
UENUM(BlueprintType)
enum class ECL9CarrySlot : uint8
{
	/** Hand only — never fits in the pocket */
	Hand,
	/** Fits in the pocket (loses light there, stays detectable) */
	Pocketable,
	/** Cart only — cannot be hand-carried at all */
	CartOnly,
	/** Worn on the head — picking it up puts it on, the hands stay free (headlamp, 2026-09-05) */
	Head
};

/** How a light-emitting item lights (docs/systems/light.md 광원 목록) */
UENUM(BlueprintType)
enum class ECL9ItemLight : uint8
{
	/** Omni glow while carried — egg. Stays lit through the blackout */
	Glow,
	/** Forward spot, toggled with IA_Flashlight. Forced off by the blackout */
	Flashlight
};

/** Item kind — only Dish items are edible, no separate flag (docs/plan/data/item/common.md 타입과 먹기) */
UENUM(BlueprintType)
enum class ECL9ItemType : uint8
{
	Ingredient,
	Dish,
	/** Usable things that are neither ingredient nor dish (cart, flashlight) */
	Tool
};

/**
 * Stats an eat effect can move (docs/plan/data/character.md 스탯 enum).
 * Capsule/eye height stay out (geometry premises), HP stays out (the truck
 * pad is the only heal channel).
 */
UENUM(BlueprintType)
enum class ECL9Stat : uint8
{
	WalkSpeed,
	RunSpeed,
	CarryMult,
	CartMult,
	JumpVelocity,
	JumpGravity,
	StaminaDrainTime,
	StaminaRegenTime,
	StaminaMinResume
};

/**
 * Night lifecycle phase (docs/systems/session.md · docs/plan/loop.md). The host
 * game mode rules every transition; clients derive presentation from the
 * replicated value. Loading joins with seamless travel later.
 */
UENUM(BlueprintType)
enum class ECL9NightPhase : uint8
{
	/** Waiting at the truck between nights — the stop that becomes the lobby */
	Lobby,
	/** The raid — the night clock runs, expiry is a dawn failure */
	Night,
	/** Dawn screen between night end and the next lobby */
	Dawn
};

/** Points at an item row and state — the cook chain and carry mirrors use it (docs/data/schema.md) */
USTRUCT(BlueprintType)
struct FCL9ItemRef
{
	GENERATED_BODY()

	/** Must exist in the item table — the load check verifies it */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemAlias;

	/** State variant (e.g. cooked) — None is the base state */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemSubAlias;

	bool IsSet() const
	{
		return !ItemAlias.IsNone();
	}
};

/** One carried thing in the hand or pocket mirror (docs/systems/inventory.md 상태 모델) */
USTRUCT(BlueprintType)
struct FCL9CarriedItem : public FCL9ItemRef
{
	GENERATED_BODY()
};

/** Item reference plus a count — order ingredients and drops share it (docs/data/schema.md) */
USTRUCT(BlueprintType)
struct FCL9ItemCount : public FCL9ItemRef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Count = 1;
};

/** One monster drop entry — quantity rolls Min..Max, the base Count is unused here */
USTRUCT(BlueprintType)
struct FDropItem : public FCL9ItemCount
{
	GENERATED_BODY()

	/** 0..1 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DropChance = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MinQuantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxQuantity = 1;
};

/** One eat effect row — target, amount and duration all come from the table */
USTRUCT(BlueprintType)
struct FCL9StatEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	ECL9Stat Stat = ECL9Stat::WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Amount = 0.f;

	/** 0 applies once immediately; anything larger lasts that long in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DurationSec = 0.f;
};

/** One spawn baseline row — a site's animals or ingredients; the day curve scales the roll (docs/data/schema.md) */
USTRUCT(BlueprintType)
struct FCL9SpawnEntry
{
	GENERATED_BODY()

	/** Monster or item alias */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName Alias;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MinCount = 0;

	/** The day curve multiplies the roll between Min and Max (docs/plan/data/progression.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxCount = 0;
};

/** Hand-set exceptions for one day — a 0 multiplier means "use the curve" (docs/plan/data/progression.md 특정 일자 덮어쓰기) */
USTRUCT(BlueprintType)
struct FCL9DayOverride
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Day = 1;

	/** Replaces MonsterCountCurve for this day when > 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MonsterCountMult = 0.f;

	/** Replaces DangerCurve for this day when > 0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DangerMult = 0.f;

	/** No blackout this night (the tutorial day) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bNoBlackout = false;
};

namespace CL9
{
	/** Impact state chain — 성함 → 금감 → 깨짐 swaps the SubAlias (docs/systems/item.md 상태 = SubAlias) */
	inline const FName SubAliasCracked = FName("cracked");
	inline const FName SubAliasBroken = FName("broken");
}
