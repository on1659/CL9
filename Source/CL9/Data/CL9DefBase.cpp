// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/CL9DefBase.h"
#include "Internationalization/Text.h"

namespace
{
	/**
	 * 화면 문자열은 String Table "CL9"의 키여야 한다 (docs/systems/localization.md).
	 * 행에 한글을 직접 치면 언어를 바꿔도 안 바뀌므로 Broken으로 잡는다.
	 */
	bool IsFromCL9Table(const FText& Text)
	{
		FName TableId;
		FString Key;
		return FTextInspector::GetTableIdAndKey(Text, TableId, Key) && TableId == TEXT("CL9");
	}
}

void UCL9DefBase::Validate(TArray<FString>& OutErrors) const
{
	if (Alias.IsNone())
	{
		OutErrors.Add(TEXT("Alias is empty — it is the lookup key, not the file name (docs/data/schema.md)."));
	}
	if (!DisplayName.IsEmpty() && !IsFromCL9Table(DisplayName))
	{
		OutErrors.Add(TEXT("DisplayName is typed text — pick a String Table 'CL9' key instead (docs/systems/localization.md)."));
	}
	if (!Description.IsEmpty() && !IsFromCL9Table(Description))
	{
		OutErrors.Add(TEXT("Description is typed text — pick a String Table 'CL9' key instead (docs/systems/localization.md)."));
	}
}
