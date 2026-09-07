// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "CL9GameInstance.generated.h"

class UCL9DefList;
class UCL9DefRegistry;
class UCL9ShakeTable;
class UCL9UITable;

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

private:
	UPROPERTY()
	TObjectPtr<UCL9DefRegistry> DefRegistry;

	UPROPERTY()
	TObjectPtr<const UCL9UITable> UITable;

	UPROPERTY()
	TObjectPtr<const UCL9ShakeTable> ShakeTable;
};
