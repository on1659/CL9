// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameUserSettings.h"
#include "CL9GameUserSettings.generated.h"

class UCL9SettingsTable;

/**
 * 로컬 옵션 중 키 리바인딩을 뺀 나머지 — 감도·Y축 반전·볼륨(docs/systems/settings.md). 키는 엔진
 * Enhanced Input User Settings가 따로 저장하고, 언어는 엔진 표준 키(`[Internationalization] Culture`)라
 * 여기에는 필드가 없다. 엔진당 객체 하나라 다중 PIE 인스턴스가 공유한다.
 *
 * 저장은 SaveSettings()(=SaveConfig)만 부른다 — ApplySettings()류는 Scalability를 프로세스 전체에 적용해
 * PIE에서 에디터 품질까지 바꾸므로 절대 부르지 않는다. 우리 적용은 ApplyAudio·ApplyLanguage와
 * ACL9Character::LookInput의 곱뿐이다.
 */
UCLASS()
class UCL9GameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	/** 엔진의 설정 객체를 CL9 것으로 캐스팅한다 — DefaultEngine.ini의 GameUserSettingsClassName이 아니면 null */
	static UCL9GameUserSettings* Get();

	virtual void SetToDefaults() override;

	/** 시점 회전 배율 — AxisConfig 감도 위에 곱해진다 */
	UPROPERTY(Config)
	float MouseSensitivity = 1.f;

	UPROPERTY(Config)
	bool bInvertY = false;

	/** 볼륨 슬라이더 값 — SoundClass의 설계 게인 위에 곱해진다. 기본은 전부 1.0 (docs/systems/sound.md) */
	UPROPERTY(Config)
	float MasterVolume = 1.f;

	UPROPERTY(Config)
	float EffectsVolume = 1.f;

	UPROPERTY(Config)
	float UIVolume = 1.f;

	UPROPERTY(Config)
	float AmbienceVolume = 1.f;

	UPROPERTY(Config)
	float MusicVolume = 1.f;

	UPROPERTY(Config)
	float VOVolume = 1.f;

	/**
	 * 볼륨 6개를 믹스 오버라이드로 적용한다 — 월드와 오디오 디바이스가 있어야 하므로 로컬 PlayerController의
	 * BeginPlay와 슬라이더 변경 시에 부른다(GameInstance::Init에서는 조용히 아무 일도 안 한다).
	 */
	void ApplyAudio(UWorld* World) const;

	/**
	 * 언어를 지금 바꾸고 엔진 표준 키에 저장한다. 에디터/PIE에서는 현재 언어를 바꾸면 에디터 UI까지 바뀌고
	 * 게임 번역이 실리지 않으므로 게임 언어 미리보기만 켜고 저장하지 않는다(docs/systems/localization.md).
	 * cl9.lang 치트와 설정 화면이 같이 부른다.
	 */
	static bool ApplyLanguage(const FString& Culture);
};
