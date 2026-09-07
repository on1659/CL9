# 치트 시스템 (CL9CheatManager)

> 개발·테스트용 콘솔 명령. **치트 하나 추가 = 함수 하나 + 등록 한 줄** — 스텝 검증
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
UCL9CheatManager : UCheatManager        ← 치트 본체. 치트 = 멤버 함수 하나씩
  ├─ AddItem(Args)      ├─ AddMonster(Args)   ├─ ListDefs(Args)
  ├─ SetDay(Args)       ├─ SetMoney(Args)     └─ ... (함수 추가로 확장)
  │
  └─ ResolveDef(Kind, Token) → UCL9DefBase*    ← 공용: 번호/Alias 해석

CL9Cheats.cpp                            ← 등록부. 명령 이름 ↔ 함수 연결
  FAutoConsoleCommandWithWorldAndArgs("cl9.additem", ...)   // 한 줄 = 명령 하나
```

`cl9.` 처럼 점(.)이 들어간 명령은 exec 함수명으로 안 되므로, `IConsoleManager` 등록
(`FAutoConsoleCommandWithWorldAndArgs`)으로 명령 이름을 만들고 CheatManager의 함수로
포워딩한다. PlayerController에는 범용 RPC 하나만 둔다:

```cpp
// 클라에서 친 치트를 호스트로 — 콘솔 콜백이 로컬 PlayerController의 이 RPC를 직접 부른다
// (원격 PC 사본에는 CheatManager가 없으므로 CheatManager를 경유하지 않는다)
UFUNCTION(Server, Reliable, WithValidation)
void Server_CL9Cheat(const FString& CommandLine);
// 함정 셋 대비:
// 1) AllowCheats: 패키지 개발 빌드에선 기본 false → GameMode에서 비Shipping 상시 true 오버라이드
// 2) 번호 인자는 클라 쪽에서 Alias로 변환해 보낸다 — 호스트의 cl9.list 상태와 다를 수 있어서
// 3) #if !UE_BUILD_SHIPPING으로 UFUNCTION 선언을 통째로 제외하지 않는다 (RPC 테이블 불일치)
//    — 선언은 유지하고 함수 본문만 비운다
```

### 치트 추가 절차 (이게 전부다)

```cpp
// 1. UCL9CheatManager에 함수 하나
void UCL9CheatManager::AddItem(const TArray<FString>& Args) {
  // 호스트에서만 불린다 — 클라 입력은 콘솔 콜백이 Server_CL9Cheat RPC로 보낸다 (구조 절)
  const UCL9ItemDef* Def = ResolveDef(EDefKind::Item, Args[0]);
  if (!Def) { Log("cl9.additem: '%s' 없음 — cl9.list items 참고"); return; }
  const int32 Count = Args.IsValidIndex(1) ? FCString::Atoi(*Args[1]) : 1;
  // 플레이어 전방 200cm에 SpawnActor<ACL9Item> + SetAlias — drop.md의 스폰과 같은 경로
}

// 2. CL9Cheats.cpp에 등록 한 줄
REGISTER_CL9_CHEAT(additem, AddItem);   // → "cl9.additem"
```

### 번호/Alias 해석 — ResolveDef

```
cl9.additem egg           → Alias "egg"
cl9.additem egg+cooked    → Alias "egg" + SubAlias "cooked"  ('+' 구분)
cl9.additem 1             → cl9.list가 마지막으로 찍은 목록의 1번
```

- 숫자 해석은 **`cl9.list`가 출력한 순서** 기준이다. 목록을 안 찍었으면 레지스트리 등록 순.
- 번호는 행 추가·삭제로 밀릴 수 있으므로 **타이핑 편의용**이고, 재현 가능한 리포트·스크립트에는
  Alias를 쓴다.

## 명령 목록 (초기)

| 명령 | 인자 | 하는 일 |
|---|---|---|
| `cl9.list <종류>` | items · monsters · sites · orders · equipment · characters · sequences | 번호+Alias+상태 출력 — Broken 행은 `✗ (사유)` 표시 ([../data/schema.md](../data/schema.md) EDefStatus) |
| `cl9.additem <alias\|번호> [수량]` | | 플레이어 전방에 아이템 실물 스폰 |
| `cl9.addmonster <alias\|번호> [수량]` | | 전방 바닥에 몬스터 스폰 (AI 즉시 가동) |
| `cl9.damage [양] [플레이어 번호]` | 기본 1 · 번호 생략 = 자기 | 캐릭터에 데미지 — 잡힘과 같은 OnDamage 경로(넘어짐·짐 흘림·죽음). 번호는 접속 순서(PlayerArray 인덱스, 호스트 0)라 리슨 호스트 콘솔에서 클라를 죽여 관전을 검증할 수 있다 (4a) |
| `cl9.day <N>` | 로비에서만 | 로비의 일차를 N으로 — 다음 `cl9.night`가 그 일차의 곡선으로 굴리고, 건너뛴 날의 주문 마감을 처리한 뒤(위약금) 제시를 다시 뽑는다 (4b 구현 · 4e 마감 처리, 구 bp_debug_day) |
| `cl9.orders` | 로컬 읽기 | 로비 제시(번호·재료·보상·마감)와 수락한 주문(마감 일차) 출력 (4c) |
| `cl9.pick <n> [m] …` | 로비에서만 | 제시 번호로 주문 확정 — 정확히 M개 (주문 선택 화면의 스탠드인, 4c) |
| `cl9.buy [alias\|번호]` | 호스트 판정 | 인자가 없으면 장비 목록(가격·해금 일차·보유 수·구매 가능 여부)을 출력하고, 있으면 산다 — 트럭 패널의 스탠드인 (4f) |
| `cl9.eat` | 호스트 판정 | 손에 든 요리를 먹는다 — `IA_Eat`(G)의 대역 (4i) |
| `cl9.settle [force]` | 호스트 판정 | 정산 상태(일차·목표·기간 수입) 출력 + 정산일이면 판정 실행; `force`면 정산일이 아니어도 판을 지금 끝낸다 (정산) |
| `cl9.money [양]` | 호스트 판정 | 판 공유 자금(GameState.SharedMoney) 출력; 양이 붙으면 지급(양수)하거나 차감(음수) — 4f 구매 검증 대역 (4e 구현) |
| `cl9.seed <N>` | 0 = 외곽만 | 지금 그 시드로 맵을 다시 깔고 다음 밤도 그 시드로 고정 (호스트). **PIE 없이 에디터 콘솔에서 치면** 같은 생성기로 레벨에 트랜지언트 액터를 깐다 — 5a 맵 배치 툴의 창 없는 판 (`cl9.seed clear`로 제거, 두 번째 인자로 목적지 alias) |
| `cl9.shownav` | | 몬스터 이동·경로 시각화 (구 bp_show_cells) |
| `cl9.showjars` | | 항아리 배치·1500u 커버리지 확인 (구 bp_show_jars) |
| `cl9.playseq <alias\|번호>` | | 시퀀스 로컬 재생 테스트 ([sequence.md](sequence.md)) — RPC 안 탐 |
| `cl9.testwake` / `cl9.testvoice` | | 기상 예고·반응 음성 연출 테스트 (구 bp_warning_test_*) |
| `cl9.fxoff` | | 경고 연출 비활성화 (구 bp_warning_disable_all) |

목록은 열려 있다 — 스텝을 만들며 그 스텝을 검증하는 치트를 같이 추가한다
(예: ② 던지기에서 `cl9.breaktest`, ③ 발각에서 `cl9.showdetect`).

## 네트워크 정리

| 상황 | 동작 |
|---|---|
| 호스트 콘솔에서 침 | CheatManager 함수가 바로 판정 실행 |
| 클라 콘솔에서 침 | `Server_CL9Cheat(명령줄)` → 호스트 CheatManager가 해석·실행 |
| 시각화 계열 (shownav 등) | 로컬 렌더 전용 — RPC 안 탄다 |
| 결과 | 스폰물은 표준 복제로 전 클라에 나타난다 — 유령 액터 없음 |

## 참고

- 스폰 경로: [actor-base.md](actor-base.md) (SetAlias → LoadData) · [drop.md](drop.md)
- 데이터 조회: [../data/schema.md](../data/schema.md) (`UCL9DefRegistry`)
- 각 시스템 문서의 디버그 절이 이 접두 체계를 쓴다: [balance.md](balance.md) ·
  [map.md](map.md) · [warning.md](warning.md)
