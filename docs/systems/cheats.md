# 치트 시스템 (CL9CheatManager)

> 개발·테스트용 콘솔 명령. **치트 하나 추가 = 함수 하나 + `HandleCommand` 분기 한 줄 + 콘솔 등록 블록 하나** — 스텝 검증
> ("표에 행 추가하면 바닥에 뜬다")을 루프 전체를 돌지 않고 바로 확인하는 도구다.

## 원칙

- **접두는 `cl9.`** — 프로젝트 치트·디버그 명령을 전부 이 접두로 통일한다 (`bp_` 접두 폐기).
- **인자는 Alias 또는 번호** — 문자면 Alias(+SubAlias), 숫자면 `cl9.list`가 찍어준 번호.
- **치트도 판정 규약을 탄다** — 스폰·상태 변경은 호스트에서만. 클라 콘솔에서 치면 Server RPC로
  올라가 호스트가 실행한다. 치트로 소환한 액터도 실제와 같은 경로(SetAlias → 복제 → OnRep →
  LoadData)를 타므로, **치트 테스트가 곧 네트워크 테스트다.**
- **Shipping 빌드에서 컴파일 제외** (`#if !UE_BUILD_SHIPPING`).

## 구조

```
UCL9CheatManager : UCheatManager          ← 치트 본체 (Source/CL9/GameMode/CL9CheatManager.*). 치트 = 멤버 함수 하나씩
  ├─ HandleCommand(CommandLine)           ← "additem egg 2" 를 공백으로 나눠 첫 토큰으로 else-if 분기 (#if !UE_BUILD_SHIPPING)
  ├─ AddItem · AddMonster · HitMonster · ListDefs · Night · Blackout · Damage · Day · Orders · Pick
  │  · Money · Buy · Eat · Settle · SaveRun / LoadRun · Seed · Wave · Execute   (각 TArray<FString> Args)
  └─ ResolveItem(Token) / ResolveMonster(Token) → 행    ← 공용: 번호/Alias 해석

CL9Cheats.cpp (Source/CL9/GameMode/)       ← 등록부. 명령 이름·도움말 ↔ HandleCommand 연결
  static FAutoConsoleCommandWithWorldAndArgs GCL9Cheat<Name>(TEXT("cl9.<name>"), 도움말, 람다)
    람다 → RouteCL9Cheat("<name>", Args, World, bLocalOnly)
  RouteCL9Cheat: 호스트(또는 bLocalOnly)면 첫 PC 의 CheatManager->HandleCommand(명령줄),
                 클라면 ACL9PlayerController::Server_CL9Cheat(명령줄) 로 호스트에 올린다
  로컬 스위치(cl9.shownoise · cl9.lang)는 CheatManager 를 거치지 않고 람다 안에서 바로 처리
```

`cl9.` 처럼 점(.)이 들어간 명령은 exec 함수명으로 안 되므로, `IConsoleManager` 등록
(`FAutoConsoleCommandWithWorldAndArgs`)으로 명령 이름을 만들고 `RouteCL9Cheat` 가 CheatManager의
`HandleCommand` 로 포워딩한다. PlayerController에는 범용 RPC 하나만 둔다:

```cpp
// 클라에서 친 치트를 호스트로 — 콘솔 콜백이 로컬 PlayerController의 이 RPC를 직접 부른다
// (원격 PC 사본에는 CheatManager가 없으므로 CheatManager를 경유하지 않는다)
UFUNCTION(Server, Reliable, WithValidation)
void Server_CL9Cheat(const FString& CommandLine);
// 함정 셋 대비:
// 1) AllowCheats: 패키지 개발 빌드에선 기본 false → GameMode에서 비Shipping 상시 true 오버라이드
// 2) 번호 인자는 명령줄 그대로 호스트로 가고 호스트가 자기 레지스트리 순서로 푼다 — 클라·호스트가 같은 표라 순서가 같다
// 3) #if !UE_BUILD_SHIPPING으로 UFUNCTION 선언을 통째로 제외하지 않는다 (RPC 테이블 불일치)
//    — 선언은 유지하고 함수 본문만 비운다
```

### 치트 추가 절차 (이게 전부다 — 세 곳)

```cpp
// 1. UCL9CheatManager 에 함수 하나 (CL9CheatManager.h 선언 + .cpp 정의)
void UCL9CheatManager::AddItem(const TArray<FString>& Args) {
  // 호스트에서만 불린다 — 클라 입력은 RouteCL9Cheat 가 Server_CL9Cheat RPC 로 보낸다 (구조 절)
  const UCL9ItemDef* Def = ResolveItem(Args[0]);            // 없으면 로그 한 줄("cl9.list items 참고") + return
  const int32 Count = Args.IsValidIndex(1) ? FMath::Clamp(FCString::Atoi(*Args[1]), 1, 50) : 1;
  // 플레이어 전방에 GameMode->SpawnItem — drop.md 의 스폰과 같은 경로
}

// 2. HandleCommand 의 else-if 사슬에 분기 한 줄 (CL9CheatManager.cpp)
else if (Command == TEXT("additem")) { AddItem(Tokens); }

// 3. CL9Cheats.cpp 에 콘솔 등록 블록 하나 — 도움말 문자열은 여기 한 곳
static FAutoConsoleCommandWithWorldAndArgs GCL9CheatAddItem(
	TEXT("cl9.additem"),
	TEXT("cl9.additem <alias|number> [count] — spawns items in front of the player (host ruling)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		RouteCL9Cheat(TEXT("additem"), Args, World, /*bLocalOnly*/ false);   // 로컬 읽기 전용(cl9.list)만 true
	}));
```

판정에 안 닿는 로컬 스위치(`cl9.shownoise`·`cl9.lang`)는 2·3 없이 `FAutoConsoleCommand` 람다 하나로 끝난다.

### 번호/Alias 해석 — ResolveItem / ResolveMonster

```
cl9.additem egg           → Alias "egg"
cl9.additem egg+cooked    → Alias "egg" + SubAlias "cooked"  ('+' 구분)
cl9.additem 1             → cl9.list가 마지막으로 찍은 목록의 1번
```

- 숫자 해석은 **레지스트리 등록 순**(= `cl9.list`가 찍는 순서, `Registry->GetItems()[n-1]`) 기준이다.
- 번호는 행 추가·삭제로 밀릴 수 있으므로 **타이핑 편의용**이고, 재현 가능한 리포트·스크립트에는
  Alias를 쓴다.

## 명령 목록 (코드 등록 21개와 1:1 — `grep -n 'TEXT("cl9\.' Source/CL9/GameMode/CL9Cheats.cpp`, 2026-09-21)

| 명령 | 인자 | 하는 일 |
|---|---|---|
| `cl9.list <종류>` | items · monsters · characters (기본 items) — 로컬 읽기 전용(`bLocalOnly`) | 번호+Alias(+SubAlias) 출력, 뒤에 Broken 행과 사유 ([../data/schema.md](../data/schema.md) EDefStatus). 다른 종류는 경고 한 줄 — 그 표의 치트가 생기면 추가한다. 도움말 문자열은 `<items\|characters>` 로 낡아 있으나 monsters 도 받는다 |
| `cl9.additem <alias\|번호> [수량]` | 수량 1~50 | 플레이어 전방에 아이템 실물 스폰 (`GameMode->SpawnItem`) |
| `cl9.addmonster <alias\|번호> [수량]` | 수량 1~20 | 전방 바닥에 몬스터 스폰 (AI 즉시 가동) |
| `cl9.hitmonster [데미지]` | 기본 1 · 호스트 판정 | 가장 가까운 몬스터에 데미지 — 프라이팬 히트박스가 생기기 전 킬 경보·경직 검증용으로 남아 있다 |
| `cl9.damage [양] [플레이어 번호]` | 기본 1 · 번호 생략 = 자기 | 캐릭터에 데미지 — 잡힘과 같은 OnDamage 경로(넘어짐·죽음 — 짐 흘림은 HP 0 일 때만, 2026-09-15). 번호는 접속 순서(PlayerArray 인덱스, 호스트 0)라 리슨 호스트 콘솔에서 클라를 죽여 관전을 검증할 수 있다 (4a). **2026-09-20**: `Source`로 **가장 가까운 살아 있는 몬스터**(`Dead` 제외)를 넘긴다 — 피격 사건이 진짜 잡힘과 같은 그림(카메라 요·방향 호, [character.md](character.md) "잡히는 순간 카메라가 범인을 본다")이 되고 로그에 source 이름이 찍힌다. 몬스터가 하나도 없으면 `nullptr` 그대로 → 카메라 피치만 내려가고 방향 호는 숨는다(`HasHurtDirection()` false). **넘어져 있는 동안은 무시된다** — 넘어짐 무적 게이트(`OnDamage` 머리의 `IsKnockedDown()`)가 치트에도 걸려 우회가 없다(로그 `hit by … while knocked down — ignored`). HP 0 까지 치려면 1.5초 간격으로 |
| `cl9.night` | 호스트 판정 | 로비에서는 밤을 시작하고, 밤 중에는 밤을 끝낸다 — 로비 UI [밤 시작] 의 대역 ([ui.md](ui.md) 로비) |
| `cl9.blackout` | 호스트 판정 · 밤 중에만 | 정전 시각을 지금으로 당긴다 — 손전등·시설 조명 소등 검증 ([light.md](light.md) 구현 상태) |
| `cl9.day <N>` | 로비에서만 | 로비의 일차를 N으로 — 다음 `cl9.night`가 그 일차의 곡선으로 굴리고, 건너뛴 날의 주문 마감을 처리한 뒤(위약금) 제시를 다시 뽑는다 (4b 구현 · 4e 마감 처리, 구 bp_debug_day) |
| `cl9.orders` | 로컬 읽기 | 로비 제시(번호·재료·보상·마감)와 수락한 주문(마감 일차) 출력 (4c) |
| `cl9.pick <n> [m] …` | 로비에서만 | 제시 번호로 주문 확정 — 정확히 M개 (주문 선택 화면의 스탠드인, 4c) |
| `cl9.buy [alias\|번호]` | 호스트 판정 | 인자가 없으면 장비 목록(가격·해금 일차·보유 수·구매 가능 여부)을 출력하고, 있으면 산다 — 트럭 패널의 스탠드인 (4f) |
| `cl9.eat` | 호스트 판정 | 활성 퀵슬롯 칸의 요리를 먹는다 — 좌클 길게 누르기(먹기)의 대역 (4i). 품·교체 치트는 없다 — 퀵슬롯 전환은 휠·숫자키가 곧 테스트 경로다 |
| `cl9.settle [force]` | 호스트 판정 | 정산 상태(일차·목표·기간 수입) 출력 + 정산일이면 판정 실행; `force`면 정산일이 아니어도 판을 지금 끝낸다 (정산) |
| `cl9.money [양]` | 호스트 판정 | 판 공유 자금(GameState.SharedMoney) 출력; 양이 붙으면 지급(양수)하거나 차감(음수) — 4f 구매 검증 대역 (4e 구현) |
| `cl9.save [슬롯]` | 호스트 판정 | 판(일차·이더·기간 수입·진행 중 주문·보유 장비)을 지금 그 슬롯에 저장 — 로비 [이어하기] UI 의 대역 ([save.md](save.md)) |
| `cl9.load [슬롯]` | 호스트 판정 · 로비에서만 | 그 슬롯에서 판을 되돌린다 |
| `cl9.seed <N>` | 0 = 외곽만 | 지금 그 시드로 맵을 다시 깔고 다음 밤도 그 시드로 고정 (호스트). **PIE 없이 에디터 콘솔에서 치면** 같은 생성기로 레벨에 트랜지언트 액터를 깐다 — 5a 맵 배치 툴의 창 없는 판 (`cl9.seed clear`로 제거, 두 번째 인자로 목적지 alias) |
| `cl9.wave <alias\|enum>` | 호스트 판정 | 내 발밑에서 그 소음원의 파동을 한 번 낸다 — `run` · `pickup_loud` 처럼 표의 alias나 enum 이름(`PickupLoud`)을 받는다 ([wave.md](wave.md) 소음원 enum). 발신자가 사람이라 `monster_call`은 읽을 몬스터 행이 없어 조용히 끝난다 |
| `cl9.execute` | 호스트 판정 | **가장 가까운 살아 있는 처형 종**(`MonsterTable` 의 `ExecuteRadius > 0` 또는 `bExecuteOnCatch` — `IsExecutioner()`; `Dead`·`Execute` 제외) 몬스터가 나를 처형한다. 스토커(너구리)는 스토킹 계수기·거리·등 뒤·미발견·시야·`KillCooldown` 검사를 **우회**, 타깃 상태(넘어짐·처형 중·죽음)와 **앵커 스윕**·`HostBeginExecuted` 는 그대로; 잡히면 처형인 종(돼지)은 타깃 상태 검사만 하고 앵커 스윕·텔레포트 없이 그 자리에서 즉시 진입 ([monster.md](monster.md) 스토킹과 처형). 처형 종이 없으면 로그 한 줄(`cl9.addmonster pig` 또는 `raccoon` 먼저), 앵커가 막히면 몬스터가 사유(`anchor-blocked`)를 찍는다. 내가 죽었거나 처형 중이면 무시 |
| `cl9.shownoise` | 로컬 전용 | 소음 판정 반경을 디버그 원으로 그리고, 파동마다 반경·차폐 광선 수를 `LogCL9`에 찍는다 — 링이 멈추는 자리와 판정 반경이 같은지 대조하는 용도 ([wave.md](wave.md) PIE 확인) |
| `cl9.lang [ko\|en\|ja\|zh-Hans]` | 로컬 전용 (에디터/PIE는 게임 언어 미리보기) | 인자 없으면 현재 언어 출력, 있으면 지금 바꾸고 `GameUserSettings.ini`에 저장 — 설정 화면 언어 항목과 같은 `UCL9GameUserSettings::ApplyLanguage`를 부른다 ([localization.md](localization.md)) |
| `cl9.shownav` | **미구현** | 몬스터 이동·경로 시각화 (구 bp_show_cells) |
| `cl9.showjars` | **미구현** | 항아리 배치·1500u 커버리지 확인 (구 bp_show_jars) — 2026-09-07 확인: 소스 어디에도 없다 |

삭제된 계획 명령(2026-09-21 감사): `cl9.resetkeys`(키 초기화는 설정 화면 [키 초기화] 버튼 `HandleResetKeysClicked` 가 한다 —
[settings.md](settings.md)) · `cl9.playseq`(시퀀스 시스템 미구현) · `cl9.testwake`/`cl9.testvoice`/`cl9.fxoff`(연출 시스템 없음).
필요해지면 위 절차로 만든다.

목록은 열려 있다 — 스텝을 만들며 그 스텝을 검증하는 치트를 같이 추가한다
(예: ② 던지기에서 `cl9.breaktest`, ③ 발각에서 `cl9.showdetect`).

## 네트워크 정리

| 상황 | 동작 |
|---|---|
| 호스트 콘솔에서 침 | CheatManager 함수가 바로 판정 실행 |
| 클라 콘솔에서 침 | `Server_CL9Cheat(명령줄)` → 호스트 CheatManager가 해석·실행 |
| 로컬 전용 (`cl9.list`·`cl9.shownoise`·`cl9.lang`) | 로컬 읽기·렌더·설정 — RPC 안 탄다 |
| 결과 | 스폰물은 표준 복제로 전 클라에 나타난다 — 유령 액터 없음 |

## 참고

- 스폰 경로: [actor-base.md](actor-base.md) (SetAlias → LoadData) · [drop.md](drop.md)
- 데이터 조회: [../data/schema.md](../data/schema.md) (`UCL9DefRegistry`)
- 각 시스템 문서의 디버그 절이 이 접두 체계를 쓴다: [balance.md](balance.md) ·
  [map.md](map.md) · [warning.md](warning.md)
