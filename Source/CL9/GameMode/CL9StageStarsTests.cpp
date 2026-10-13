// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9RulesDef.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

// 스테이지 별 산수 (docs/goal/loop-7l-stage-stars.md) — 별 n 개 = 기간 수입 ≥ 목표 × StageStarRatios[n-1], 기준은 만분율 정수로 견준다
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCL9StageStarsTest, "CL9.Run.StageStars",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCL9StageStarsTest::RunTest(const FString& Parameters)
{
	UCL9RulesDef* Rules = NewObject<UCL9RulesDef>();
	TestTrue(TEXT("헤더 기본은 켜짐"), Rules->bStageStars);
	TestEqual(TEXT("헤더 기본 기준은 셋"), Rules->StageStarRatios.Num(), 3);
	TestEqual(TEXT("헤더 기본 여는 별은 1"), Rules->StageUnlockStars, 1);

	TestEqual(TEXT("목표 미달 799/800 → 0"), Rules->StageStarsFor(799, 800), 0);
	TestEqual(TEXT("목표 딱 800/800 → 1"), Rules->StageStarsFor(800, 800), 1);
	TestEqual(TEXT("1199/800 → 1"), Rules->StageStarsFor(1199, 800), 1);
	TestEqual(TEXT("1200/800 = 150% → 2 (float 1.5 오차 없이)"), Rules->StageStarsFor(1200, 800), 2);
	TestEqual(TEXT("1599/800 → 2"), Rules->StageStarsFor(1599, 800), 2);
	TestEqual(TEXT("1600/800 = 200% → 3"), Rules->StageStarsFor(1600, 800), 3);
	TestEqual(TEXT("넘쳐도 기준 개수가 최대"), Rules->StageStarsFor(100000, 800), 3);
	TestEqual(TEXT("수입 0 → 0"), Rules->StageStarsFor(0, 800), 0);
	TestEqual(TEXT("목표 0 이면 모든 기준을 넘는다"), Rules->StageStarsFor(0, 0), 3);
	TestEqual(TEXT("큰 수도 자릿수를 잃지 않는다"), Rules->StageStarsFor(1500000000, 1000000000), 2);

	Rules->StageStarRatios = { 1.f, 1.1f, 1.2f, 1.3f };
	TestEqual(TEXT("기준이 넷이면 별도 넷까지"), Rules->StageStarsFor(1300, 1000), 4);
	TestEqual(TEXT("1.1 × 1000 = 1100 딱이면 둘"), Rules->StageStarsFor(1100, 1000), 2);

	TArray<FString> Errors;
	Rules->StageStarRatios = { 1.f, 2.f, 1.5f };
	Rules->Validate(Errors);
	TestTrue(TEXT("내림차순 기준은 로드 오류"), Errors.ContainsByPredicate([](const FString& Error) { return Error.Contains(TEXT("StageStarRatios")); }));

	Errors.Reset();
	Rules->StageStarRatios = { 1.f, 1.5f, 2.f };
	Rules->StageUnlockStars = 4;
	Rules->Validate(Errors);
	TestTrue(TEXT("여는 별이 기준 개수보다 많으면 로드 오류"), Errors.ContainsByPredicate([](const FString& Error) { return Error.Contains(TEXT("StageUnlockStars")); }));

	return true;
}

#endif
