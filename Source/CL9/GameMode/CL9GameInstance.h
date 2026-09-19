// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CL9GameInstance.generated.h"

class UCL9DefList;
class UCL9DefRegistry;
class UCL9SettingsTable;
class UCL9ShakeTable;
class UCL9SoundTable;
class UCL9UITable;
class UCL9WaveTable;

/**
 * 로컬 def 레지스트리를 소유한다 (docs/data/schema.md 조회). def 리스트 에셋은
 * BP 자식 클래스에서 지정하므로 C++는 에셋 경로를 전혀 갖지 않는다.
 */
UCLASS(abstract)
class UCL9GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UCL9DefRegistry* GetDefRegistry() const
	{
		return DefRegistry;
	}

	/**
	 * 게임 인스턴스 바깥에서 쓰는 레지스트리 — 에디터 도구(PIE 없는 cl9.seed)가
	 * def 리스트를 얻으려고 프로젝트의 게임 인스턴스 기본값을 읽을 때 쓴다.
	 * 프로젝트의 게임 인스턴스가 CL9 게임 인스턴스가 아니면 null을 반환하며
	 * 로그를 남긴다.
	 */
	static UCL9DefRegistry* CreateStandaloneRegistry(UObject* Outer);

	/** UI 매니저가 읽는 UI 테이블이다 (docs/systems/ui_system.md) — null이면 Init에서 한 번 로그를 남긴다 */
	const UCL9UITable* GetUITable() const
	{
		return UITable;
	}

	/** 프레젠테이션 코드가 읽는 카메라 흔들림 테이블이다 (docs/systems/character.md 흔들림) */
	const UCL9ShakeTable* GetShakeTable() const
	{
		return ShakeTable;
	}

	/** 표현 코드(`CL9Sound::PlayAt`)가 읽는 효과음 테이블이다 (docs/systems/sound.md) — null이면 효과음이 하나도 나지 않는다 */
	const UCL9SoundTable* GetSoundTable() const
	{
		return SoundTable;
	}

	/** 파동 렌더러와 소음원이 읽는 파동 표다 (docs/systems/wave.md) — null이면 파동이 하나도 그려지지 않는다 */
	const UCL9WaveTable* GetWaveTable() const
	{
		return WaveTable;
	}

	/** 설정 화면·볼륨 적용이 읽는 에셋 참조 표다 (docs/systems/settings.md) — null이면 볼륨 슬라이더가 닿는 곳이 없다 */
	const UCL9SettingsTable* GetSettingsTable() const
	{
		return SettingsTable;
	}

protected:
	/** 모든 테이블 행은 이 리스트에서 로드된다 — BP_CL9GameInstance에서 지정한다 */
	UPROPERTY(EditAnywhere, Category="Data")
	TSoftObjectPtr<UCL9DefList> DefListAsset;

	/** UCL9UIManager용 UI 행 목록 — BP_CL9GameInstance에서 지정한다 (경로 하드코딩 금지) */
	UPROPERTY(EditAnywhere, Category="UI")
	TSoftObjectPtr<UCL9UITable> UITableAsset;

	/** 카메라 흔들림 행 목록 — BP_CL9GameInstance에서 지정한다 */
	UPROPERTY(EditAnywhere, Category="UI")
	TSoftObjectPtr<UCL9ShakeTable> ShakeTableAsset;

	/** 효과음 행 목록 — BP_CL9GameInstance에서 지정한다 */
	UPROPERTY(EditAnywhere, Category="UI")
	TSoftObjectPtr<UCL9SoundTable> SoundTableAsset;

	/** 파동 행 목록 — BP_CL9GameInstance에서 지정한다 */
	UPROPERTY(EditAnywhere, Category="UI")
	TSoftObjectPtr<UCL9WaveTable> WaveTableAsset;

	/** 볼륨 믹스·사운드 클래스 참조 — BP_CL9GameInstance에서 지정한다 */
	UPROPERTY(EditAnywhere, Category="UI")
	TSoftObjectPtr<UCL9SettingsTable> SettingsTableAsset;

private:
	UPROPERTY()
	TObjectPtr<UCL9DefRegistry> DefRegistry;

	UPROPERTY()
	TObjectPtr<const UCL9UITable> UITable;

	UPROPERTY()
	TObjectPtr<const UCL9ShakeTable> ShakeTable;

	UPROPERTY()
	TObjectPtr<const UCL9SoundTable> SoundTable;

	UPROPERTY()
	TObjectPtr<const UCL9SettingsTable> SettingsTable;

	UPROPERTY()
	TObjectPtr<const UCL9WaveTable> WaveTable;
};
