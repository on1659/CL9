// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameMode/CL9GameUserSettings.h"
#include "CL9.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameMode/CL9GameInstance.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/TextLocalizationManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "UI/CL9SettingsTable.h"

UCL9GameUserSettings* UCL9GameUserSettings::Get()
{
	UCL9GameUserSettings* Settings = (GEngine != nullptr) ? Cast<UCL9GameUserSettings>(GEngine->GetGameUserSettings()) : nullptr;
	if (Settings == nullptr)
	{
		UE_LOG(LogCL9, Warning, TEXT("GameUserSettings is not UCL9GameUserSettings — check GameUserSettingsClassName in DefaultEngine.ini."));
	}
	return Settings;
}

void UCL9GameUserSettings::SetToDefaults()
{
	// UEngine::CreateGameUserSettings가 생성 직후 부른다 — 상태에 기대지 말고 값만 놓는다
	Super::SetToDefaults();

	MouseSensitivity = 1.f;
	bInvertY = false;
	MasterVolume = 1.f;
	EffectsVolume = 1.f;
	UIVolume = 1.f;
	AmbienceVolume = 1.f;
	MusicVolume = 1.f;
	VOVolume = 1.f;
}

void UCL9GameUserSettings::ApplyAudio(UWorld* World) const
{
	const UCL9GameInstance* GameInstance = (World != nullptr) ? World->GetGameInstance<UCL9GameInstance>() : nullptr;
	const UCL9SettingsTable* Table = (GameInstance != nullptr) ? GameInstance->GetSettingsTable() : nullptr;
	if (Table == nullptr || Table->SoundMix == nullptr)
	{
		return;
	}

	// 슬라이더 값은 클래스의 설계 게인 위에 곱해진다 — 자식 클래스까지 함께(Master → 전부)
	const TPair<USoundClass*, float> Overrides[] = {
		{Table->MasterClass.Get(), MasterVolume},
		{Table->EffectsClass.Get(), EffectsVolume},
		{Table->UIClass.Get(), UIVolume},
		{Table->AmbienceClass.Get(), AmbienceVolume},
		{Table->MusicClass.Get(), MusicVolume},
		{Table->VOClass.Get(), VOVolume},
	};
	for (const TPair<USoundClass*, float>& Override : Overrides)
	{
		if (Override.Key != nullptr)
		{
			UGameplayStatics::SetSoundMixClassOverride(World, Table->SoundMix, Override.Key, Override.Value, 1.f, 0.1f, true);
		}
	}
	UGameplayStatics::PushSoundMixModifier(World, Table->SoundMix);
}

bool UCL9GameUserSettings::ApplyLanguage(const FString& Culture)
{
#if WITH_EDITOR
	if (GIsEditor)
	{
		// 에디터(PIE)에서는 현재 언어를 바꾸면 에디터 UI까지 바뀌고 게임 번역은 실리지 않는다 —
		// 에디터의 "게임 언어 미리보기"(비동기 로드)로 대신 켠다. 저장은 하지 않는다.
		FTextLocalizationManager::Get().EnableGameLocalizationPreview(Culture);
		UE_LOG(LogCL9, Log, TEXT("Language: game localization preview '%s' (editor only, not saved)."), *Culture);
		return true;
	}
#endif
	if (!FInternationalization::Get().SetCurrentLanguageAndLocale(Culture))
	{
		UE_LOG(LogCL9, Warning, TEXT("Language: '%s' is not an available culture."), *Culture);
		return false;
	}
	GConfig->SetString(TEXT("Internationalization"), TEXT("Culture"), *Culture, GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
	UE_LOG(LogCL9, Log, TEXT("Language: set to '%s' and saved."), *Culture);
	return true;
}
