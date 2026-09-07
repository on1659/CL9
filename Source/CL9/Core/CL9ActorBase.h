// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Data/CL9Types.h"
#include "CL9ActorBase.generated.h"

/**
 * CL9 월드 액터(아이템, 몬스터 — docs/systems/actor-base.md)의 공통 베이스.
 * 리플리케이트되는 Alias+SubAlias 쌍과 데이터 로드 진입점을 담고 있다:
 * 초기값은 BeginPlay, 런타임 변경은 OnRep, 호스트에서는 SetAlias.
 *
 * 여기에는 Server RPC가 없다 — 이 액터들은 어떤 클라이언트에도 소유되지
 * 않으므로, 클라이언트 요청은 항상 소유 캐릭터의 RPC를 거치고 호스트는
 * 이 액터에 대해 일반 함수를 직접 호출한다 (conventions § 10).
 */
UCLASS(abstract)
class ACL9ActorBase : public AActor
{
	GENERATED_BODY()

public:
	ACL9ActorBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 서버 전용: 행 키를 설정하고 로컬에서 로드한다; 클라이언트는 OnRep을 통해 따라간다 */
	void SetAlias(FName InAlias, FName InSubAlias = NAME_None);

	FName GetAlias() const
	{
		return Alias;
	}

	FName GetSubAlias() const
	{
		return SubAlias;
	}

	/**
	 * 화면에 뜰 이름 — 스캔 라벨이 읽는다. 기본은 액터 이름이고, 표가 있는 액터(아이템·몬스터)는
	 * 행의 `DisplayName`을, 시설물은 자기 프로퍼티를 준다. **UI가 하위 클래스를 캐스팅해
	 * 갈래를 타지 않게** 하려고 여기에 둔다 — 새 종류가 생겨도 화면 코드는 그대로다.
	 */
	UFUNCTION(BlueprintPure, Category="CL9")
	virtual FText GetDisplayName() const;

	/** 어떤 종류인지 — 자식 클래스 생성자가 정한다. 스캔처럼 종류를 표로 고르는 곳이 읽는다 */
	UFUNCTION(BlueprintPure, Category="CL9")
	ECL9ActorKind GetKind() const
	{
		return Kind;
	}

protected:
	virtual void BeginPlay() override;

	/** 자식 클래스 생성자에서 한 번 정한다 — 클래스마다 고정이라 복제하지 않는다 */
	ECL9ActorKind Kind = ECL9ActorKind::None;

	/** 로컬 레지스트리에서 이 액터의 행을 읽는다 — 자식 클래스에서 override한다 */
	virtual void LoadData();

	UFUNCTION()
	void OnRep_AliasPair();

private:
	/** 테이블 행 키 — 하드코딩된 값이 아니라 레벨 배치나 스폰 데이터가 설정한다 */
	UPROPERTY(ReplicatedUsing=OnRep_AliasPair, EditAnywhere, BlueprintReadOnly, Category="Data", meta=(AllowPrivateAccess="true"))
	FName Alias;

	/** 상태 변형(egg+cooked, egg+cracked...) — 상태 시스템은 이 값을 바꿀 뿐 상태 변수를 새로 추가하지 않는다 */
	UPROPERTY(ReplicatedUsing=OnRep_AliasPair, EditAnywhere, BlueprintReadOnly, Category="Data", meta=(AllowPrivateAccess="true"))
	FName SubAlias;
};
