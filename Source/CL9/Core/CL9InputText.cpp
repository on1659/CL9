// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/CL9InputText.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "Internationalization/StringTable.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/StringTableRegistry.h"
#include "PlayerMappableKeySettings.h"
#include "UserSettings/EnhancedInputUserSettings.h"

namespace CL9Input
{
	static const FName StringTableId(TEXT("CL9"));

	FText GetKeyText(const FKey& Key)
	{
		if (!Key.IsValid())
		{
			return FText::GetEmpty();
		}

		// 시트의 Key.* 행이 있으면 그것 — 시트에 없는 키(문자 키 대부분)는 엔진 영문 원문으로 폴백하므로
		// 키 문자열이 화면에 새지 않는다(localization.md가 명문화한 유일한 파생 키 예외).
		const FString TableKey = TEXT("Key.") + Key.GetFName().ToString();
		if (FStringTableConstPtr Table = FStringTableRegistry::Get().FindStringTable(StringTableId); Table.IsValid() && Table->FindEntry(TableKey).IsValid())
		{
			return FText::FromStringTable(StringTableId, TableKey);
		}
		return FText::AsCultureInvariant(Key.GetDisplayName(false).BuildSourceString());
	}

	UEnhancedInputUserSettings* GetUserSettings(const APlayerController* PlayerController)
	{
		const ULocalPlayer* LocalPlayer = (PlayerController != nullptr) ? PlayerController->GetLocalPlayer() : nullptr;
		const UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
		return (Subsystem != nullptr) ? Subsystem->GetUserSettings() : nullptr;
	}

	const FPlayerKeyMapping* FindKeyboardMapping(const UEnhancedInputUserSettings* Settings, FName MappingName)
	{
		const UEnhancedPlayerMappableKeyProfile* Profile = (Settings != nullptr) ? Settings->GetActiveKeyProfile() : nullptr;
		const FKeyMappingRow* Row = (Profile != nullptr) ? Profile->GetPlayerMappingRows().Find(MappingName) : nullptr;
		if (Row == nullptr)
		{
			return nullptr;
		}
		for (const FPlayerKeyMapping& Mapping : Row->Mappings)
		{
			if (!Mapping.GetCurrentKey().IsGamepadKey())
			{
				return &Mapping;
			}
		}
		return nullptr;
	}

	FText GetActionKeyText(const APlayerController* PlayerController, const UInputAction* Action)
	{
		const UPlayerMappableKeySettings* MappableSettings = (Action != nullptr) ? Action->GetPlayerMappableKeySettings() : nullptr;
		if (MappableSettings == nullptr)
		{
			return FText::GetEmpty();
		}
		const FPlayerKeyMapping* Mapping = FindKeyboardMapping(GetUserSettings(PlayerController), MappableSettings->GetMappingName());
		return (Mapping != nullptr) ? GetKeyText(Mapping->GetCurrentKey()) : FText::GetEmpty();
	}
}
