// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CL9SettingsTable.generated.h"

class USoundBase;
class USoundClass;
class USoundMix;

/**
 * 설정 화면이 만지는 에셋 참조 — 볼륨 믹스와 사운드 클래스 여섯(docs/systems/settings.md 오디오).
 * conventions § 5의 경로 예외는 MasterSoundMix 하나뿐이라 클래스 참조는 에셋이 들고 BP_CL9GameInstance가
 * 주입한다(UI 표와 같은 방식). 행 목록·라벨·기본값은 여기 없다 — 라벨은 코드의 LOCTABLE 리터럴이어야
 * sync_strings.py가 시트 누락을 잡고, 기본값은 전부 1.0이다.
 */
UCLASS()
class UCL9SettingsTable : public UDataAsset
{
	GENERATED_BODY()

public:
	/** 슬라이더 값이 오버라이드로 들어가는 믹스 — 로컬 PlayerController BeginPlay에서 Push된다 */
	UPROPERTY(EditAnywhere, Category="Audio")
	TObjectPtr<USoundMix> SoundMix;

	UPROPERTY(EditAnywhere, Category="Audio")
	TObjectPtr<USoundClass> MasterClass;

	UPROPERTY(EditAnywhere, Category="Audio")
	TObjectPtr<USoundClass> EffectsClass;

	UPROPERTY(EditAnywhere, Category="Audio")
	TObjectPtr<USoundClass> UIClass;

	UPROPERTY(EditAnywhere, Category="Audio")
	TObjectPtr<USoundClass> AmbienceClass;

	UPROPERTY(EditAnywhere, Category="Audio")
	TObjectPtr<USoundClass> MusicClass;

	UPROPERTY(EditAnywhere, Category="Audio")
	TObjectPtr<USoundClass> VOClass;

	/** 슬라이더를 놓을 때 재생할 확인음 — 에셋이 생기면 채운다(비어 있으면 재생하지 않는다) */
	UPROPERTY(EditAnywhere, Category="Audio")
	TObjectPtr<USoundBase> PreviewSound;
};
