// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9RulesDef.h"
#include "Misc/AutomationTest.h"

#if WITH_AUTOMATION_TESTS

// 임포스터 배정 확률 표 (docs/goal/loop-7g-imposter.md) — 번호 = 사람 수 − 1, 끝을 넘으면 마지막 값, 0~1 로 자른다
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCL9ImposterTest, "CL9.Run.Imposter",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCL9ImposterTest::RunTest(const FString& Parameters)
{
	UCL9RulesDef* Rules = NewObject<UCL9RulesDef>();

	// 헤더 초안 — 2026-10-12 사용자 "3인 이상일 때 10%, 4인일 때 25%"
	TestEqual(TEXT("0명 → 0"), Rules->ImposterChanceFor(0), 0.f);
	TestEqual(TEXT("1인 → 0"), Rules->ImposterChanceFor(1), 0.f);
	TestEqual(TEXT("2인 → 0"), Rules->ImposterChanceFor(2), 0.f);
	TestEqual(TEXT("3인 → 10%"), Rules->ImposterChanceFor(3), 0.1f);
	TestEqual(TEXT("4인 → 25%"), Rules->ImposterChanceFor(4), 0.25f);
	TestEqual(TEXT("표 끝을 넘는 사람 수는 마지막 값"), Rules->ImposterChanceFor(6), 0.25f);
	TestTrue(TEXT("성공 보상이 처치 보상보다 크다(초안)"), Rules->ImposterWinReward > Rules->ImposterKillReward);

	Rules->ImposterChanceByPlayers = { 0.5f };
	TestEqual(TEXT("한 칸짜리 표는 모든 인원에 같은 값"), Rules->ImposterChanceFor(3), 0.5f);
	Rules->ImposterChanceByPlayers.Reset();
	TestEqual(TEXT("빈 표 → 0"), Rules->ImposterChanceFor(4), 0.f);
	Rules->ImposterChanceByPlayers = { 0.f, 1.5f };
	TestEqual(TEXT("1 초과는 1 로 자른다(로드 검사가 먼저 막는다)"), Rules->ImposterChanceFor(2), 1.f);

	TArray<FString> Errors;
	Rules->Validate(Errors);
	TestTrue(TEXT("범위 밖 확률은 로드 오류"), Errors.ContainsByPredicate([](const FString& Error) { return Error.Contains(TEXT("ImposterChanceByPlayers")); }));

	Errors.Reset();
	Rules->ImposterChanceByPlayers = { 0.f, 0.f, 0.1f, 0.25f };
	Rules->ImposterKillReward = -1;
	Rules->Validate(Errors);
	TestTrue(TEXT("음수 보상은 로드 오류"), Errors.ContainsByPredicate([](const FString& Error) { return Error.Contains(TEXT("ImposterKillReward")); }));

	return true;
}

#endif
