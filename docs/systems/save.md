# 세이브 시스템 (판)

> **진행은 호스트의 것이다. 판 하나가 세이브 하나** —
> [../plan/data/progression.md](../plan/data/progression.md)가 정본. 이 문서는 저장 구조와
> 시점만 다룬다. 개인 설정은 [settings.md](settings.md) (완전 분리).

## 세이브에 들어가는 것 (progression.md 목록 그대로)

```cpp
UCLASS()
class UCL9SaveGame : public USaveGame {   // Source/CL9/GameMode/CL9SaveGame.h
  GENERATED_BODY()
public:
  static FString SlotName(int32 Slot);      // "CL9Save_<Slot>" — 지금은 1번만 쓰지만 슬롯 번호를 받는다
  static constexpr int32 CurrentVersion = 3;// 필드를 더할 때마다 올린다 — 낮은 버전은 기본값으로 채워 읽는다
  UPROPERTY() int32 SaveVersion = CurrentVersion;

  UPROPERTY() int32 CurrentDay;             // 현재 일차
  UPROPERTY() int32 SharedMoney;            // 누적 자금 (이더, int)
  UPROPERTY() int32 PeriodIncome;           // 이번 정산 기간 수입 (정산 중간 저장 대비)
  UPROPERTY() TArray<FCL9AcceptedOrder> ActiveOrders; // 아직 납품하지 못한 주문 — 마감은 절대 일차, CurrentDay 와 함께 읽는다
  UPROPERTY() TArray<FName> OwnedEquipment; // 보유 장비 (Alias — 실물은 로드 시 트럭 옆에 재스폰, 같은 것이 둘이면 항목 둘).
                                            // 보존 판정: 새벽에 트럭 곁에 있던 실물만 남는다 — "두고 오면 잃는다"의
                                            // 경계는 트럭 존이다. 버전 1 세이브에는 없어 비어 있다
  UPROPERTY() TArray<FCL9SiteStars> StageStars; // 목적지별 이번 판 최고 별(7l — [../goal/loop-7l-stage-stars.md](../goal/loop-7l-stage-stars.md)). 항목이 있으면 그 목적지는 열려 있다.
                                            // 판과 함께 레벨을 건너고 새 판에서 비워진다. 버전 2 이하에는 없어 비어 있다
};
```

- **에셋 참조는 전부 Alias다** — 세이브에 경로·포인터를 넣지 않는다. 로드 시 레지스트리로
  해석하고, 해석 실패(삭제된 Alias)는 로그 + 그 항목만 버린다 (schema.md EDefStatus 원칙).
- 손에 든 것·맵 상태는 저장하지 않는다 — **저장 단위는 "날"이다.** 밤 중 저장 없음.

## 저장 시점

```
밤 종료 → 새벽 처리 (정산 판정 포함) → 저장 → 로비
```

- 새벽마다 1회. 자동 저장뿐 — 수동 저장 없음 (밤은 통째로 치른다 — 추출 게임의 전제).
- 정산 실패 → 새 판이 자동으로 열린다 — 실패한 판은 덮어쓰기 전에 기록만 남긴다 (아래).
  - **2026-10-12 정정(7i)**: 정산 미달은 새 판을 열지 않는다 — 코인만 깎고 이어 간다([balance.md](balance.md)).
- **레벨을 건너는 판(7i, [../goal/loop-7i-company.md](../goal/loop-7i-company.md))**: 회사로 가거나 회사에서 사이트로 갈 때 떠나기 전에 `SaveRun(1)`,
  도착 레벨의 `StartPlay` 가 `LoadRun(1, 주문 다시 뽑지 않음)` — 일차·코인·기간 수입·받은 주문·장비(패드 옆에 다시 선다)가 건넌다.
  "이어 간다" 표시는 게임 인스턴스(`SetRunCarry`/`ConsumeRunCarry`)에 있고 한 번만 쓰인다.

## 슬롯

- 파일명 `CL9Save_<slot>` — 지금은 슬롯 1개만 쓰지만 **구조는 처음부터 슬롯 번호를 받는다**
  (progression.md: "저장 구조가 슬롯 하나를 전제로 굳으면 안 된다"). 나중에 3개로.
- 로비의 [판] 표시·[새 판 시작]이 이 슬롯을 읽고 쓴다 ([session.md](session.md)).

## 버전과 마이그레이션

- `SaveVersion` int 하나. 로드 시 낮은 버전이면 마이그레이션 훅을 거친다 — 필드 추가는
  기본값으로 채우고, 의미 변경은 변환 코드를 쓴다.
- 읽을 수 없는 세이브(깨짐·미래 버전)는 **삭제하지 않고** 백업 이름으로 옮긴 뒤 새 판을
  연다 — 조용한 데이터 소실 금지.

## 기록 (최고 일차)

"클리어는 없고, 며칠까지 버텼는지가 기록이다" (progression.md). 기록은 판과 수명이 다르다 —
판이 죽어도 기록은 남아야 하므로 **별도 파일**:

```cpp
UCLASS()
class UCL9RecordSave : public USaveGame {   // 파일명 CL9Records — 슬롯·판과 무관
  GENERATED_BODY()
public:
  UPROPERTY() int32 RecordVersion;
  UPROPERTY() int32 BestDay;                // 최장 생존 일차
};
```

- 갱신 시점: 판 실패(정산 미달)로 닫힐 때 + 새벽 저장 때 (현재 일차 > BestDay면).
- 표시: 새벽/게임오버 화면과 타이틀 (미결 — UI 배치는 ui.md에서).

## 지갑 — 바깥 재화 "이더" (2026-10-12, [../goal/loop-7f-out-game-currency.md](../goal/loop-7f-out-game-currency.md))

판 세이브·기록과 달리 **모든 머신에 하나씩** 있는 개인 파일이다 — 바깥 재화는 플레이어의 것이라 호스트가 바뀌어도 따라가야 한다.

```cpp
UCLASS()
class UCL9WalletSave : public USaveGame {   // 파일명 CL9Wallet — PIE 인스턴스 1 이상은 CL9Wallet_PIE<n>
  UPROPERTY() int32 WalletVersion;
  UPROPERTY() int32 Ether;                  // 보유 이더(바깥 재화), 0 이상
};
```

- `UCL9WalletSubsystem`(GameInstanceSubsystem, `Source/CL9/GameMode/CL9Wallet.*`)이 시작할 때 읽고, 지급마다 더해 저장한다. "이번 판 지급"은 런타임 값이다.
- **지급 경로**: 판 끝(`EndNight`)에 호스트가 **모든 플레이어에게** `Client_WalletGrant(N)` 을 한 번씩 보낸다 — 살아서 귀가했으면 판 수입 × `WalletIncomeRate`, 아니면 0.
  각 머신이 자기 파일에 더한다. 새벽 화면 열기 RPC 보다 먼저, 같은 채널이다.
- 한 PC 에서 PIE 호스트·클라를 같이 돌려도 파일이 섞이지 않게 PIE 인스턴스 1 이상은 이름에 번호가 붙는다. 같은 PC 의 독립 실행 창 둘은 같은 파일을 쓴다(시험 한계).
- 로컬 파일이라 고칠 수 있다 — 치장 전용이라 받아들인다. Steam 클라우드·인벤토리는 범위 밖.
- 치트 `cl9.wallet [delta]`(로컬).

### 스킨 (2026-10-12, 7k — [../goal/loop-7k-skins.md](../goal/loop-7k-skins.md))

- 지갑 파일 버전 2 — `OwnedSkins`(산 스킨, 기본 스킨은 적지 않는다)·`EquippedSkin`(None = 기본)을 더했다. 1 짜리 파일은 스킨 칸이 빈 채로 읽힌다.
- `UCL9WalletSubsystem::BuySkin`(이더가 모자라면 거절, 사면 깎고 저장)·`EquipSkin`(저장)·`OwnsSkin`·`GetEquippedSkin`. 입은 스킨은 빙의 때 호스트에 알린다(`ACL9PlayerController::RequestSkin`).
- 호스트는 보유를 확인하지 않는다(각자의 파일) — 치장 전용이라 받아들인다(이더 파일과 같다).

## 네트워크

- 저장·로드는 **호스트 전용**이다(판 세이브·기록 — 지갑은 위 절대로 각자). 클라는 세이브 파일을 갖지 않는다 — 접속하면 호스트의
  판 상태가 GameState로 복제될 뿐이다.
- 클라 화면의 [판] 정보(일차·자금)는 GameState에서 읽는다 — 세이브 파일 동기화 없음.

## 참고

- 세이브 내용의 정본: [../plan/data/progression.md](../plan/data/progression.md) "진행 상태는 어디 저장하나"
- 로비 연동: [session.md](session.md)
- 자금 구조: [economy.md](economy.md)
