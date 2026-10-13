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

	/** cl9.buy <alias|번호> — 장비를 산다 (트럭 패널 대역, 호스트 판정). 인자가 없으면 목록과 살 수 있는지를 출력한다 */
	void Buy(const TArray<FString>& Args);

	/** cl9.eat — 손에 든 요리를 먹는다 (IA_Eat이 붙기 전까지의 대역) */
	void Eat(const TArray<FString>& Args);

	/** cl9.settle — 지금 정산 상태(목표·기간 수입)를 출력하고, force를 붙이면 정산일이 아니어도 판정을 돌린다 */
	void Settle(const TArray<FString>& Args);

	/** cl9.save [슬롯] / cl9.load [슬롯] — 판을 지금 저장하거나 되돌린다 (로비 [이어하기] UI 대역) */
	void SaveRun(const TArray<FString>& Args);
	void LoadRun(const TArray<FString>& Args);

	/** cl9.seed <N> — 시드 N으로 지금 즉시 사이트를 재생성하고(0이면 껍데기만), 다음 밤도 이 시드로 고정한다 (docs/systems/map.md 디버깅) */
	void Seed(const TArray<FString>& Args);

	/** cl9.wave <alias|enum> — 내 발밑에서 그 소음원의 파동을 한 번 낸다 (호스트 판정, docs/systems/wave.md) */
	void Wave(const TArray<FString>& Args);

	/**
	 * cl9.execute — 가장 가까운 살아 있는 처형 종(`IsExecutioner()` = `ExecuteRadius > 0` 또는 `bExecuteOnCatch`) 몬스터가 나를 처형한다.
	 * 스토커 종은 스토킹 계수기·거리·등 뒤·미발견·시야·KillCooldown 검사를 우회하고 앵커 스윕·`HostBeginExecuted` 는 그대로,
	 * 잡히면-처형 종(돼지)은 스윕 없이 제자리에서 즉시 (docs/systems/cheats.md). 없으면 로그 한 줄
	 */
	void Execute(const TArray<FString>& Args);

	/**
	 * cl9.swingtaps <N> [interval_ms=100] — 호스트 폰에 `Server_UseStart`+`Server_UseRelease` 를 N 번 interval 간격(월드 타이머)으로
	 * 넣고, 탭마다 서버 시각과 결과(swing 즉시 / buffered 예약 / lost 거부)를, 끝에 합계를 로그로 남긴다 — D-8 입력 유실 측정 (docs/systems/cheats.md)
	 */
	void SwingTaps(const TArray<FString>& Args);

	/** cl9.swinglog [0|1] — 호스트가 휘두르기마다 `cl9.swing:` 한 줄(시작·입력·명중·표면·회복·팔 복귀·검사·가림)을 남기는 스위치. 인자 없으면 토글 */
	void SwingLog(const TArray<FString>& Args);

	/** cl9.knockbacklog [0|1] — 호스트가 넉백이 끝날 때마다 `cl9.knockback:` 한 줄(시작·끝·거리·스텝·막힘·히트스톱·시간)을 남기는 스위치. 인자 없으면 토글 */
	void KnockbackLog(const TArray<FString>& Args);

	/** cl9.griplive [0|1] — 켜져 있으면 손에 든 아이템이 매 틱 GripOffset/GripRotation을 다시 읽는다 — ItemTable 값을 PIE 중 튜닝할 때 다시 줍지 않아도 바로 보인다. 인자 없으면 토글 */
	void GripLive(const TArray<FString>& Args);

	/**
	 * cl9.imposter [번호] — 호스트 전용(docs/goal/loop-7g-imposter.md). 인자 없이: 지금 임포스터와 다음 판 지정을 찍는다.
	 * 번호(플레이어 목록 순서): 판 중이면 그 사람을 지금 임포스터로, 로비면 다음 판에. -1 = 없음
	 */
	void Imposter(const TArray<FString>& Args);

private:
	/** cl9.swingtaps 의 타이머 한 틱 — 탭 하나를 넣고 결과를 세며, N 번째 뒤에 합계를 찍고 타이머를 멈춘다 */
	void SwingTapTick();

	FTimerHandle SwingTapTimer;
	int32 SwingTapTotal = 0;
	int32 SwingTapIndex = 0;
	int32 SwingTapIntervalMs = 0;
	int32 SwingTapSwung = 0;
	int32 SwingTapBuffered = 0;
	int32 SwingTapLost = 0;

	/** "egg", "egg+cooked", 또는 cl9.list 번호를 실제 사용 가능한 아이템 행으로 변환한다 */
	const UCL9ItemDef* ResolveItem(const FString& Token) const;

	/** alias(+sub) 또는 cl9.list 번호를 실제 사용 가능한 몬스터 행으로 변환한다 */
	const UCL9MonsterDef* ResolveMonster(const FString& Token) const;

	const UCL9DefRegistry* GetRegistry() const;
};
