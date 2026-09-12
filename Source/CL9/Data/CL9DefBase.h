// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CL9DefBase.generated.h"

class UTexture2D;

/**
 * Instanced Gimmicks 컬럼을 통해 테이블 행에 붙는, 연출 전용 특수 동작이다
 * (docs/plan/build-order.md 원칙 4). 판정에 영향을 주는 기믹은 대신 테이블
 * 값으로 표현하고 호스트의 판정 코드가 그 값을 읽는다.
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced)
class UCL9Gimmick : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * 연출용 훅. 액터가 자신의 행을 로드한 뒤 모든 머신에서 호출되며(상태가
	 * 바뀔 때도 다시 호출되므로 구현은 재진입을 견뎌야 한다), 판정 상태는
	 * 절대 여기서 건드리지 않는다 — 호스트는 오직 테이블 값으로만 판정한다.
	 */
	virtual void OnActorLoaded(AActor* Actor)
	{
	}
};

/**
 * 모든 게임 데이터 테이블 행의 부모 클래스다 (docs/data/schema.md). 조회 키는
 * 파일 이름이 아니라 항상 Alias + SubAlias다. Instanced 기믹이 UObject
 * 소유자를 필요로 하기 때문에 (DataTable 행 구조체가 아니라) UObject로
 * 만들었다. 행은 보통 UCL9DefTable 안에 인라인으로 산다 — 종류마다 테이블
 * 애셋 하나씩, 엑셀 방식이다 (2026-09-02 결정) — 하지만 행 하나짜리 독립
 * 애셋도 여전히 로드된다.
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced)
class UCL9DefBase : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * 필수 필드가 비어 있을 때마다 메시지를 하나씩 추가한다. 에러가 있는 행은
	 * 레지스트리에 들어가지 않아서 게임플레이에 섞이지 않는다 (schema.md의
	 * EDefStatus). 필수 필드 목록은 서브클래스마다 다르다 — 베이스 클래스는
	 * Alias만 요구한다.
	 */
	virtual void Validate(TArray<FString>& OutErrors) const;

	/** 식별자이자 조회 키. 예: egg / chicken / farmer_a */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	FName Alias;

	/** 선택적 하위 분류. 예: cooked / burnt — None이면 기본 상태를 뜻한다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	FName SubAlias;

	/** 화면 표시명 — String Table "CL9"의 키 참조여야 한다(Item.egg.Name). 직접 친 텍스트는 Validate가 Broken으로 잡는다 (docs/systems/localization.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	FText DisplayName;

	/** 선택적 설명 — DisplayName과 같은 규칙(Item.egg.Desc) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	FText Description;

	/** UI 아이콘 — 테이블을 열 때 모든 텍스처가 로드되지 않도록 소프트 참조로 둔다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 아이템류 행의 월드 메시. 캐릭터/몬스터는 자신의 스켈레탈 메시 컬럼을 따로 쓴다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	TSoftObjectPtr<UStaticMesh> Mesh;

	/** 이 행 전용 BP. 비워두면 기본값인 종류별 공용 BP를 쓴다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Data")
	TSoftClassPtr<AActor> ActorClass;

	/** 연출 전용 특수 동작들. 에디터에서 행마다 추가한다 */
	UPROPERTY(EditAnywhere, Instanced, Category="Data")
	TArray<TObjectPtr<UCL9Gimmick>> Gimmicks;
};
