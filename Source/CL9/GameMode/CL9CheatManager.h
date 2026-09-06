// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "CL9CheatManager.generated.h"

class UCL9DefRegistry;
class UCL9ItemDef;
class UCL9MonsterDef;

/**
 * cl9.* 콘솔 치트 (docs/systems/cheats.md) — 단계별 검증 도구다:
 * 치트 하나 = 멤버 함수 하나 + CL9Cheats.cpp에 등록 한 줄. 스폰 계열 치트는
 * 호스트에서만 실행되며, 클라이언트 콘솔은 ACL9PlayerController::Server_CL9Cheat를
 * 거쳐 여기에 도달한다. 치트 스폰도 표준 데이터 경로를 그대로 타므로, 치트
 * 테스트가 곧 네트워크 테스트를 겸한다.
 */
UCLASS()
class UCL9CheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	/** "additem egg 2" 같은 형식의 줄을 나눠서 각 치트 함수로 전달한다 */
	void HandleCommand(const FString& CommandLine);

	/** cl9.additem <alias|번호> [수량] — 이 컨트롤러의 폰 앞에 아이템을 스폰한다 */
	void AddItem(const TArray<FString>& Args);

	/** cl9.addmonster <alias|번호> [수량] — 앞쪽에 몬스터를 스폰하며, AI가 즉시 활성화된다 */
	void AddMonster(const TArray<FString>& Args);

	/** cl9.hitmonster [데미지] — 가장 가까운 몬스터에게 데미지를 준다 (④가 들어오기 전까지 킬 알람/스턴 테스트용) */
	void HitMonster(const TArray<FString>& Args);

	/** cl9.list <items|monsters|characters> — 번호가 매겨진 행과, 문제가 있는 행은 그 사유까지 함께 출력한다 */
	void ListDefs(const TArray<FString>& Args);

	/** cl9.night — 로비에서는 밤을 시작하고, 밤 중에는 밤을 종료한다 (로비 UI 대역) */
	void Night(const TArray<FString>& Args);

	/** cl9.blackout — 블랙아웃을 지금 시점으로 당긴다 (손전등/소리 차단 검증용) */
	void Blackout(const TArray<FString>& Args);

	/** cl9.damage [양] [플레이어 번호] — 자신의 캐릭터, 또는 참가 순서 기준 N번 플레이어에게 피해를 준다 (몬스터 없이도 사망 → 관전 → 트럭 패드 흐름을 점검할 수 있다) */
	void Damage(const TArray<FString>& Args);

	/** cl9.day <N> — 로비를 N일차로 넘겨서, 다음 cl9.night가 그날의 커브로 굴러가게 한다 (docs/systems/balance.md) */
	void Day(const TArray<FString>& Args);

	/** cl9.orders — 로비의 제안 목록(번호순)과, 마감일이 붙은 수락된 주문을 출력한다 (로컬 읽기 전용) */
	void Orders(const TArray<FString>& Args);

	/** cl9.pick <n> [m] … — 번호로 제안을 확정한다 (주문 화면 대역, 호스트 판정) */
	void Pick(const TArray<FString>& Args);

	/** cl9.money [양] — 판의 자금을 출력하고, 양이 붙으면 그만큼 넣거나(양수) 뺀다(음수) — 4f 구매 검증 대역 */
	void Money(const TArray<FString>& Args);

	/** cl9.save [슬롯] / cl9.load [슬롯] — 판을 지금 저장하거나 되돌린다 (로비 [이어하기] UI 대역) */
	void SaveRun(const TArray<FString>& Args);
	void LoadRun(const TArray<FString>& Args);

	/** cl9.seed <N> — 시드 N으로 지금 즉시 사이트를 재생성하고(0이면 껍데기만), 다음 밤도 이 시드로 고정한다 (docs/systems/map.md 디버깅) */
	void Seed(const TArray<FString>& Args);

private:
	/** "egg", "egg+cooked", 또는 cl9.list 번호를 실제 사용 가능한 아이템 행으로 변환한다 */
	const UCL9ItemDef* ResolveItem(const FString& Token) const;

	/** alias(+sub) 또는 cl9.list 번호를 실제 사용 가능한 몬스터 행으로 변환한다 */
	const UCL9MonsterDef* ResolveMonster(const FString& Token) const;

	const UCL9DefRegistry* GetRegistry() const;
};
