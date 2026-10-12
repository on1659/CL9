// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9RulesDef.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

// 전멸 패널티 산수 (docs/goal/loop-7a-wipe-penalty.md) — floor(누적 이더 × WipePenaltyRate), 잔액·비율 범위 밖은 자른다
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCL9WipePenaltyTest, "CL9.Run.WipePenalty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCL9WipePenaltyTest::RunTest(const FString& Parameters)
{
	UCL9RulesDef* Rules = NewObject<UCL9RulesDef>();
	TestEqual(TEXT("헤더 기본 비율은 30%"), Rules->WipePenaltyRate, 0.3f);

	TestEqual(TEXT("1000 의 30% = 300"), Rules->WipePenaltyFor(1000), 300);
	TestEqual(TEXT("999 의 30% = 299.7 → 내림 299"), Rules->WipePenaltyFor(999), 299);
	TestEqual(TEXT("10 의 30% = 3 (float 0.3 이 살짝 커도 3)"), Rules->WipePenaltyFor(10), 3);
	TestEqual(TEXT("3 의 30% = 0.9 → 0"), Rules->WipePenaltyFor(3), 0);
	TestEqual(TEXT("잔액 0 → 0"), Rules->WipePenaltyFor(0), 0);
	TestEqual(TEXT("음수 잔액은 0 으로 본다"), Rules->WipePenaltyFor(-50), 0);
	TestEqual(TEXT("큰 잔액도 자릿수를 잃지 않는다"), Rules->WipePenaltyFor(1000000000), 300000000);

	Rules->WipePenaltyRate = 0.7f;
	TestEqual(TEXT("float 0.7(0.6999…) 이어도 10 의 70% = 7"), Rules->WipePenaltyFor(10), 7);
	Rules->WipePenaltyRate = 0.125f;
	TestEqual(TEXT("12.5% 처럼 소수 비율도 정확하다"), Rules->WipePenaltyFor(1000), 125);
	Rules->WipePenaltyRate = 0.f;
	TestEqual(TEXT("비율 0 → 깎지 않는다"), Rules->WipePenaltyFor(1000), 0);
	Rules->WipePenaltyRate = 1.f;
	TestEqual(TEXT("비율 1 → 전부"), Rules->WipePenaltyFor(1000), 1000);
	Rules->WipePenaltyRate = 1.5f;
	TestEqual(TEXT("비율 1 초과는 1 로 자른다(로드 검사가 먼저 막는다)"), Rules->WipePenaltyFor(1000), 1000);
	Rules->WipePenaltyRate = -0.2f;
	TestEqual(TEXT("음수 비율은 0 으로 자른다"), Rules->WipePenaltyFor(1000), 0);

	TArray<FString> Errors;
	Rules->WipePenaltyRate = 1.5f;
	Rules->Validate(Errors);
	TestTrue(TEXT("범위 밖 비율은 로드 오류"), Errors.ContainsByPredicate([](const FString& Error) { return Error.Contains(TEXT("WipePenaltyRate")); }));

	return true;
}

#endif
