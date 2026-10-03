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

	/**
	 * 표들이 소프트 참조로 가리키는 에셋을 Init 이 미리 올리고 여기 붙들어 둔다 — 첫 사용 자리(첫 잡힘의 셰이크 클래스, 첫 링의
	 * 나이아가라, 첫 타격의 플래시 재질·퍼프, 종별 첫 스폰의 메시·ABP)에서 게임 스레드가 동기 로드로 멈추지 않게(2026-09-21 실측:
	 * 캐릭터 틱 안의 셰이크 BP 클래스 로드 577ms 히치). 로드만 하고 붙들지 않으면 다음 GC 에 풀려 첫 사용이 다시 콜드가 된다
	 * (docs/systems/performance.md "첫 사용 동기 로드 금지")
	 */
	UPROPERTY()
	TArray<TObjectPtr<UObject>> WarmAssets;

	/** 표의 소프트 참조를 하나 로드한 결과를 WarmAssets 에 붙든다 — null(빈 열)은 조용히 건너뛴다 */
	void Warm(UObject* Asset);
};
