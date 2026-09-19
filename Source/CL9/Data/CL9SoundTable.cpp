// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9SoundTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "GameMode/CL9GameInstance.h"

void CL9Sound::PlayAt(const UObject* WorldContext, FName Row, const FVector& Location, float PitchJitter)
{
	// 전용 서버에는 귀가 없다 — 표현 층이므로 아예 들어가지 않는다
	if (IsRunningDedicatedServer())
	{
		return;
	}

	UWorld* World = GEngine != nullptr ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
	const UCL9GameInstance* GameInstance = World != nullptr ? World->GetGameInstance<UCL9GameInstance>() : nullptr;
	const UCL9SoundTable* Table = GameInstance != nullptr ? GameInstance->GetSoundTable() : nullptr;
	const TSoftObjectPtr<USoundBase>* Entry = Table != nullptr ? Table->Rows.Find(Row) : nullptr;
	// 표·행·애셋 어느 것이 비어도 조용히 — 소리가 없다고 로그를 매 걸음마다 찍지 않는다
	USoundBase* Sound = Entry != nullptr ? Entry->LoadSynchronous() : nullptr;
	if (Sound == nullptr)
	{
		return;
	}

	// 볼륨은 애셋(과 SoundClass 게인)에 있다 — 코드는 피치 흔들림만 얹는다
	const float Pitch = PitchJitter > 0.f ? FMath::FRandRange(1.f - PitchJitter, 1.f + PitchJitter) : 1.f;
	UGameplayStatics::PlaySoundAtLocation(World, Sound, Location, 1.f, Pitch);
}
