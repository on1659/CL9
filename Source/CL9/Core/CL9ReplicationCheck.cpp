// Copyright Epic Games, Inc. All Rights Reserved.

// 스냅샷 테스트가 아니라 구조 검사다 (build-order ⑥ 예외): Replicated 필드를
// 추가하면서 GetLifetimeReplicatedProps에 등록하지 않으면 동기화가 조용히
// 깨지는데, 이 테스트가 모든 CL9 액터 클래스에 대해 그것을 잡아낸다.

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "UObject/UObjectIterator.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCL9ReplicationRegistrationTest,
	"CL9.Replication.Registration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FCL9ReplicationRegistrationTest::RunTest(const FString& Parameters)
{
	static const FName CL9Package(TEXT("/Script/CL9"));

	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;
		if (!Class->IsChildOf(AActor::StaticClass()) || Class->GetPackage()->GetFName() != CL9Package)
		{
			continue;
		}

		const AActor* DefaultActor = Class->GetDefaultObject<AActor>();
		TArray<FLifetimeProperty> LifetimeProps;
		DefaultActor->GetLifetimeReplicatedProps(LifetimeProps);

		for (TFieldIterator<FProperty> PropertyIt(Class); PropertyIt; ++PropertyIt)
		{
			FProperty* Property = *PropertyIt;
			if (!Property->HasAnyPropertyFlags(CPF_Net))
			{
				continue;
			}

			bool bRegistered = false;
			for (const FLifetimeProperty& Lifetime : LifetimeProps)
			{
				if (Lifetime.RepIndex >= Property->RepIndex && Lifetime.RepIndex < Property->RepIndex + static_cast<uint16>(Property->ArrayDim))
				{
					bRegistered = true;
					break;
				}
			}

			if (!bRegistered)
			{
				AddError(FString::Printf(TEXT("%s.%s is Replicated but missing from GetLifetimeReplicatedProps — clients will silently desync."),
					*Class->GetName(), *Property->GetName()));
			}
		}
	}

	return true;
}

#endif
