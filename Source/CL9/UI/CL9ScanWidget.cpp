// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/CL9ScanWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Camera/PlayerCameraManager.h"
#include "CollisionQueryParams.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Core/CL9ActorBase.h"
#include "Data/CL9ItemDef.h"
#include "Items/CL9Item.h"
#include "UI/CL9ScanLabelWidget.h"

namespace
{
	/** 라벨을 올릴 대상 꼭대기 높이 — 아이템은 행의 Radius, 그 밖(몬스터)은 충돌 반높이 */
	float TopHeight(const ACL9ActorBase& Target)
	{
		if (const ACL9Item* Item = Cast<ACL9Item>(&Target))
		{
			const UCL9ItemDef* Def = Item->GetItemDef();
			return (Def != nullptr) ? Def->Radius : 20.f;
		}
		return Target.GetSimpleCollisionHalfHeight();
	}
}

UCL9ScanWidget::UCL9ScanWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UCL9ScanWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Elapsed = 0.f;
	CollectTargets();

	if (ScanRing != nullptr)
	{
		ScanRing->SetRenderScale(FVector2D(0.f, 0.f));
		ScanRing->SetRenderOpacity(1.f);
		ScanRing->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UCL9ScanWidget::CollectTargets()
{
	Entries.Reset();

	APlayerController* Controller = GetOwningPlayer();
	UWorld* World = GetWorld();
	if (Controller == nullptr || World == nullptr || Controller->PlayerCameraManager == nullptr)
	{
		return;
	}

	// 그 순간 화면에 보이는 것만 (2026-09-06 사용자): 최근 프레임에 렌더됐고(오클루전 포함),
	// 뷰포트 안에 투영되며, 카메라에서 시야가 막히지 않은, ScanKinds에 든 종류의 액터.
	const FVector Eye = Controller->PlayerCameraManager->GetCameraLocation();
	int32 ViewportX = 0;
	int32 ViewportY = 0;
	Controller->GetViewportSize(ViewportX, ViewportY);

	for (TActorIterator<ACL9ActorBase> It(World); It; ++It)
	{
		ACL9ActorBase* Target = *It;
		if (Target == nullptr || !ScanKinds.Contains(Target->GetKind()) || !Target->WasRecentlyRendered(0.1f))
		{
			continue;
		}
		// 아이템은 놓인 것만 — 손에 들린 것은 스캔 대상이 아니다
		const ACL9Item* Item = Cast<ACL9Item>(Target);
		if (Item != nullptr && !Item->IsFree())
		{
			continue;
		}
		FVector2D ScreenPos;
		if (!Controller->ProjectWorldLocationToScreen(Target->GetActorLocation(), ScreenPos, false)
			|| ScreenPos.X < 0.f || ScreenPos.Y < 0.f || ScreenPos.X > ViewportX || ScreenPos.Y > ViewportY)
		{
			continue;
		}
		const float Distance = FVector::Dist(Eye, Target->GetActorLocation());
		if (Distance > ScanRadius)
		{
			continue;
		}
		FCollisionQueryParams Params(SCENE_QUERY_STAT(CL9Scan), false);
		Params.AddIgnoredActor(Controller->GetPawn());
		Params.AddIgnoredActor(Target);
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, Eye, Target->GetActorLocation(), ECC_Visibility, Params) && Hit.bBlockingHit)
		{
			continue;   // 벽 뒤
		}

		FScanEntry Entry;
		Entry.Target = Target;
		Entry.RevealTime = ScanDuration * (Distance / FMath::Max(ScanRadius, KINDA_SMALL_NUMBER));
		Entries.Add(Entry);
	}
	Entries.Sort([](const FScanEntry& A, const FScanEntry& B) { return A.RevealTime < B.RevealTime; });
}

void UCL9ScanWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	Elapsed += InDeltaTime;
	const float Pulse = FMath::Clamp(Elapsed / FMath::Max(ScanDuration, KINDA_SMALL_NUMBER), 0.f, 1.f);
	if (ScanRing != nullptr)
	{
		if (Pulse < 1.f)
		{
			ScanRing->SetRenderScale(FVector2D(Pulse * ScanRingMaxScale, Pulse * ScanRingMaxScale));
			ScanRing->SetRenderOpacity(1.f - Pulse);
		}
		else
		{
			ScanRing->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	APlayerController* Controller = GetOwningPlayer();
	bool bAnyPending = Pulse < 1.f;
	// 이번 프레임에 이미 자리를 잡은 라벨들 — 같은 줄에 선 아이템끼리 글자가 겹치지 않게 위로 밀어낸다.
	// Entries는 거리순이라 가까운 것이 제자리를 갖고 먼 것이 밀려난다.
	TArray<FBox2D> Placed;
	for (FScanEntry& Entry : Entries)
	{
		if (Entry.bDone)
		{
			continue;
		}
		ACL9ActorBase* Target = Entry.Target.Get();
		if (Target == nullptr)
		{
			if (Labels.IsValidIndex(Entry.LabelIndex))
			{
				Labels[Entry.LabelIndex]->Hide();
			}
			Entry.bDone = true;
			continue;
		}
		if (Elapsed < Entry.RevealTime)
		{
			bAnyPending = true;   // 파동이 아직 닿지 않았다
			continue;
		}

		if (Entry.LabelIndex == INDEX_NONE)
		{
			// 파동이 닿았다 — 라벨 하나를 이 위젯의 LabelLayer에 붙이고 켠다
			UCL9ScanLabelWidget* Label = (ScanLabelClass != nullptr && LabelLayer != nullptr)
				? CreateWidget<UCL9ScanLabelWidget>(Controller, ScanLabelClass) : nullptr;
			if (Label == nullptr)
			{
				Entry.bDone = true;
				continue;
			}
			if (UCanvasPanelSlot* CanvasSlot = LabelLayer->AddChildToCanvas(Label))
			{
				CanvasSlot->SetAutoSize(true);
				CanvasSlot->SetAlignment(FVector2D(0.5f, 1.f));
			}
			Entry.LabelIndex = Labels.Add(Label);
			Label->Show(*Target, ScanLabelDuration);
		}

		UCL9ScanLabelWidget* Label = Labels[Entry.LabelIndex];
		if (!Label->IsShowing())
		{
			Entry.bDone = true;   // 라벨이 제 애니메이션을 끝내고 스스로 꺼졌다
			continue;
		}

		bAnyPending = true;
		const FVector Anchor = Target->GetActorLocation() + FVector(0.f, 0.f, TopHeight(*Target) + LabelLift);
		FVector2D ScreenPos;
		if (!UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(Controller, Anchor, ScreenPos, false))
		{
			ScreenPos = FVector2D(-10000.f, -10000.f);   // 카메라 뒤 — 화면 밖에 세워 둔다
		}
		else
		{
			ScreenPos.Y -= ResolveOverlap(ScreenPos, Placed);
		}
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Label->Slot))
		{
			CanvasSlot->SetPosition(ScreenPos);
		}
	}

	if (!bAnyPending)
	{
		// 파동도 끝났고 라벨도 전부 꺼졌다 — 이 위젯이 닫히면서 라벨도 함께 사라진다
		CloseSelf();
	}
}

float UCL9ScanWidget::ResolveOverlap(const FVector2D& ScreenPos, TArray<FBox2D>& Placed) const
{
	// 라벨 슬롯은 정렬이 (0.5, 1)이라 ScreenPos가 라벨의 아래 가운데다
	auto MakeRect = [this, &ScreenPos](float Shift)
	{
		const float Bottom = ScreenPos.Y - Shift;
		return FBox2D(
			FVector2D(ScreenPos.X - LabelSize.X * 0.5f, Bottom - LabelSize.Y),
			FVector2D(ScreenPos.X + LabelSize.X * 0.5f, Bottom));
	};
	auto Overlaps = [&Placed](const FBox2D& Rect)
	{
		for (const FBox2D& Other : Placed)
		{
			if (Rect.Intersect(Other))
			{
				return true;
			}
		}
		return false;
	};

	float Shift = 0.f;
	FBox2D Rect = MakeRect(Shift);
	for (int32 Push = 0; Push < LabelMaxPush && Overlaps(Rect); ++Push)
	{
		Shift += LabelSize.Y + LabelGap;
		Rect = MakeRect(Shift);
	}

	// 밀어낼 횟수를 다 써도 겹치면 그냥 거기 둔다 — 화면 밖으로 올려 보내는 것보다 낫다
	Placed.Add(Rect);
	return Shift;
}
