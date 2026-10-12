// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "CL9SkinDef.generated.h"

class USkeletalMesh;

/**
 * 스킨 행 하나(docs/goal/loop-7k-skins.md) — 바깥 재화(이더)로 사는 치장이다. 밸런스에 닿지 않는다(loop-rework 결정 1).
 * **몸 전체를 바꾸는 방식**이다: `BodyMesh` 가 있으면 캐릭터 행의 몸 대신 그것을 입는다(같은 스켈레톤이어야 애님이 돈다).
 * 모델이 들어오기 전의 대역은 몸은 그대로 두고 옷 재질의 색만 바꾼다(`Tint*`). 모델이 들어오면 행만 더한다.
 */
UCLASS()
class UCL9SkinDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	/** 몸 전체 메시 — 비면 캐릭터 행의 몸 그대로 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skin")
	TSoftObjectPtr<USkeletalMesh> BodyMesh;

	/** 색을 바꿀 재질 칸 이름(몸·팔 메시 공통) — 대역 스킨용 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skin")
	FName TintSlot = FName("Cloth");

	/** 그 칸 재질의 색 파라미터 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skin")
	FName TintParameter = FName("TintColor");

	/** 바꿀 색 — 알파 0 이면 색을 바꾸지 않는다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skin")
	FLinearColor Tint = FLinearColor(0.f, 0.f, 0.f, 0.f);

	/** 이더 가격 — 잔액이 모자라면 못 산다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skin")
	int32 Price = 0;

	/** 처음부터 누구나 가진 스킨 — 사지 않아도 입는다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Skin")
	bool bDefault = false;
};
