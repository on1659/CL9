// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Internationalization/StringTableRegistry.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9RulesDef.h"
#include "Data/CL9SiteDef.h"

/**
 * 스테이지 별의 표시 문구 (docs/goal/loop-7l-stage-stars.md) — 목적지 판의 프롬프트, 정산 화면, 클리어 화면이 같은 모양을 쓴다.
 * 별 글자는 문자열 키(`Stage.StarOn`/`Stage.StarOff`)라 글꼴에 없으면 표에서 바꾼다.
 */
namespace CL9StageText
{
	/** 별 `Count` 개를 채우고 나머지를 빈 별로 — 최대는 `CL9Rules.StageStarRatios` 개수(표가 없으면 3) */
	inline FText Stars(const UCL9DefRegistry* Registry, int32 Count)
	{
		const UCL9RulesDef* Rules = Registry != nullptr ? Registry->GetRules() : nullptr;
		const int32 Max = (Rules != nullptr && !Rules->StageStarRatios.IsEmpty()) ? Rules->StageStarRatios.Num() : 3;
		const FString On = LOCTABLE("CL9", "Stage.StarOn").ToString();
		const FString Off = LOCTABLE("CL9", "Stage.StarOff").ToString();
		FString Out;
		for (int32 Index = 0; Index < Max; ++Index)
		{
			Out += Index < Count ? On : Off;
		}
		return FText::FromString(Out);
	}

	/** 사이트의 표시 이름(행의 DisplayName, 행이 없으면 alias) */
	inline FText SiteName(const UCL9DefRegistry* Registry, FName Site)
	{
		const UCL9SiteDef* Row = Registry != nullptr ? Registry->FindSite(Site) : nullptr;
		return (Row != nullptr && !Row->DisplayName.IsEmpty()) ? Row->DisplayName : FText::FromName(Site);
	}

	/** "이름 ★★☆" — 별이 음수(정산한 적 없음)면 이름만 */
	inline FText SiteWithStars(const UCL9DefRegistry* Registry, FName Site, int32 SiteStars)
	{
		const FText Name = SiteName(Registry, Site);
		return SiteStars < 0 ? Name : FText::Format(LOCTABLE("CL9", "Stage.NameStars"), Name, Stars(Registry, SiteStars));
	}
}
