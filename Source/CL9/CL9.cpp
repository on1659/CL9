// Copyright Epic Games, Inc. All Rights Reserved.

#include "CL9.h"
#include "Internationalization/StringTableRegistry.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"

/**
 * 게임 모듈 — 화면 문자열 표 하나를 등록하는 것 외에는 기본 모듈과 같다.
 * 코드·UMG·데이터 표는 전부 이 표("CL9")의 키만 들고 있고, 한글 원문은
 * Content/Strings/CL9.csv에만 있다 (docs/systems/localization.md).
 */
class FCL9GameModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{
		LOCTABLE_FROMFILE_GAME("CL9", "CL9", "Strings/CL9.csv");
	}

	virtual void ShutdownModule() override
	{
		FStringTableRegistry::Get().UnregisterStringTable(TEXT("CL9"));
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE( FCL9GameModule, CL9, "CL9" );

DEFINE_LOG_CATEGORY(LogCL9)
