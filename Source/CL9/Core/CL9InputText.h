// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class APlayerController;
class UInputAction;
struct FKey;
struct FPlayerKeyMapping;
class UEnhancedInputUserSettings;

/**
 * 키 표기 — 프롬프트의 "[{0}] 줍기"와 설정 화면의 키 버튼이 같은 글자를 쓰게 하는 곳
 * (docs/systems/input.md 키 표기 · localization.md `Key.*`). BP에서 부를 곳이 없어 자유 함수다.
 */
namespace CL9Input
{
	/**
	 * FKey 하나의 표기. 시트에 `Key.<FKey 이름>` 행이 있으면 그것(예: Key.SpaceBar "Space"), 없으면 엔진의
	 * 영문 원문(GetDisplayName(false)의 소스 문자열). 엔진 ko 번역은 "스페이스"·"왼쪽 Shift"·"LMB"로 섞이고
	 * 에디터 미리보기는 Engine 타깃을 싣지 않아 PIE와 패키지가 달라지므로 우리 표기를 둔다.
	 */
	FText GetKeyText(const FKey& Key);

	/**
	 * 액션에 지금 바인딩된 키보드/마우스 키의 표기 — 유저 설정 프로필에서 읽는다(PlayerInput의 활성 매핑이 아니다:
	 * Screen이 열리면 IMC_Gameplay가 떨어져 비어 버린다). 액션에 매핑 이름이 없거나 행이 없으면 빈 텍스트.
	 */
	FText GetActionKeyText(const APlayerController* PlayerController, const UInputAction* Action);

	/** 프로필 행에서 표시·수정 대상이 되는 매핑 — 키보드/마우스 키인 첫 것. 게임패드 행이 First 슬롯일 수 있어 슬롯을 믿지 않는다 */
	const FPlayerKeyMapping* FindKeyboardMapping(const UEnhancedInputUserSettings* Settings, FName MappingName);

	/** 로컬 플레이어의 유저 설정 — 없으면 null (호스트의 프록시 컨트롤러, 에디터 도구 등) */
	UEnhancedInputUserSettings* GetUserSettings(const APlayerController* PlayerController);
}
