// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "Data/CL9Types.h"
#include "UI/CL9Widget.h"
#include "CL9ScanWidget.generated.h"

class ACL9ActorBase;
class UBorder;
class UCanvasPanel;
class UCL9ScanLabelWidget;

/**
 * 우클릭 스캔 한 번 = 이 위젯 하나 (docs/systems/ui.md 스캔 · UI 표 Popup 행 "Scan").
 * 열리는 순간 화면에 보이는 액터 중 `ScanKinds`에 든 종류를 모아 두고, 파동 링을 키우며 파동이 닿는 순서로
 * 라벨(WBP_ScanLabel)을 자기 LabelLayer에 하나씩 붙인다. 라벨 수 제한은 없다.
 * 파동이 끝나고 라벨이 전부 꺼지면 스스로 닫히고, 닫히면서 라벨도 함께 사라진다
 * (2026-09-06 사용자: "scan 메인 UMG 하나, 거기에 라벨이 개별로 붙고, 메인이 사라질 때 같이 사라진다").
 * 우클릭을 다시 누르면 PlayerController가 열린 것을 닫고 새로 연다.
 */
UCLASS(Abstract)
class UCL9ScanWidget : public UCL9Widget
{
	GENERATED_BODY()

public:
	UCL9ScanWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** 화면 중앙에서 ScanDuration 동안 커지는 링 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> ScanRing;

	/** 라벨이 붙는 캔버스 — 이 위젯과 함께 사라진다 */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCanvasPanel> LabelLayer;

	/** WBP_ScanLabel — WBP_Scan에서 지정 */
	UPROPERTY(EditAnywhere, Category="Scan")
	TSubclassOf<UCL9ScanLabelWidget> ScanLabelClass;

	/**
	 * 스캔에 보이는 종류 — WBP_Scan에서 고른다: 아이템만, 또는 아이템+몬스터. 비어 있으면 아무것도
	 * 안 보인다. 액터 쪽 종류는 각 클래스 생성자가 정한 ACL9ActorBase::GetKind() (2026-09-06 사용자)
	 */
	UPROPERTY(EditAnywhere, Category="Scan")
	TArray<ECL9ActorKind> ScanKinds = { ECL9ActorKind::Item };

	/** 파동이 닿는 거리 (cm) */
	UPROPERTY(EditAnywhere, Category="Scan")
	float ScanRadius = 1500.f;

	/** 파동이 ScanRadius에 닿기까지 걸리는 초 — 라벨은 파동이 지나갈 때 드러난다 */
	UPROPERTY(EditAnywhere, Category="Scan")
	float ScanDuration = 0.8f;

	/** 드러난 라벨이 켜져 있는 초(페이드 포함) — WBP_ScanLabel에 ShowAnim이 있으면 그 길이가 우선 */
	UPROPERTY(EditAnywhere, Category="Scan")
	float ScanLabelDuration = 3.f;

	/** 파동이 끝날 때 ScanRing의 렌더 스케일 — 화면 밖으로 나갈 만큼 */
	UPROPERTY(EditAnywhere, Category="Scan")
	float ScanRingMaxScale = 3.f;

	/** 대상 꼭대기(아이템은 행의 Radius, 몬스터는 충돌 반높이)에서 라벨 바닥까지의 여유 (cm) */
	UPROPERTY(EditAnywhere, Category="Scan")
	float LabelLift = 15.f;

	/**
	 * 겹침을 밀어낼 때 쓰는 라벨 한 장의 크기(픽셀). 실제 그려진 크기 대신 이 값으로 판정한다 —
	 * 라벨은 붙은 첫 프레임에 아직 desired size가 없어서 실측하면 그 프레임만 겹치기 때문이다.
	 */
	UPROPERTY(EditAnywhere, Category="Scan")
	FVector2D LabelSize = FVector2D(200.f, 44.f);

	/** 밀어낸 라벨 사이에 남기는 세로 간격(픽셀) */
	UPROPERTY(EditAnywhere, Category="Scan")
	float LabelGap = 4.f;

	/** 한 라벨을 위로 밀어낼 수 있는 최대 횟수 — 한 줄에 잔뜩 몰렸을 때 화면 밖까지 올라가지 않게 막는다 */
	UPROPERTY(EditAnywhere, Category="Scan")
	int32 LabelMaxPush = 6;

private:
	/** 지금 화면에 보이는 ScanKinds 종류의 액터를 거리순으로 모은다 — 렌더됨 + 뷰포트 안 + 시야 트레이스 (아이템은 놓인 것만) */
	void CollectTargets();

	/**
	 * 이미 자리를 잡은 라벨들과 겹치지 않을 때까지 위로 밀어내고, 밀어낸 거리(픽셀)를 돌려준다.
	 * 자리를 잡은 사각형은 Placed에 더해진다 — 한 프레임 안에서만 쓰는 목록이다.
	 */
	float ResolveOverlap(const FVector2D& ScreenPos, TArray<FBox2D>& Placed) const;

	struct FScanEntry
	{
		TWeakObjectPtr<ACL9ActorBase> Target;
		/** 스캔 시작 뒤 파동이 닿는 초 */
		float RevealTime = 0.f;
		int32 LabelIndex = INDEX_NONE;
		bool bDone = false;
	};
	TArray<FScanEntry> Entries;

	/** 이 스캔이 만든 라벨들 — LabelLayer의 자식이라 위젯과 함께 사라진다 */
	UPROPERTY()
	TArray<TObjectPtr<UCL9ScanLabelWidget>> Labels;

	float Elapsed = 0.f;
};
