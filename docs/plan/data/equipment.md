# 장비 — `UCL9EquipmentDef`

번 것으로 사는 물건. 프라이팬 · 그 밖.

---

## 컬럼

| 컬럼 | 타입 | 뜻 |
|---|---|---|
| `Alias` | `FName` | `frying_pan` … |
| `DisplayName` | `FText` | 상점에 뜨는 이름 |
| `Icon` · `Mesh` | 소프트 참조 | |
| `ActorClass` | 소프트 참조 | 전용 BP |
| `Price` | int | 이더 가격 |
| `Slot` | enum | `Kitchen` 주방 고정 / `Carryable` 들고 다님 |
| `bPhysicalProp` | bool | 월드에 놓을 수 있는 물건인가. **true만 판다** (현재 상점 범위 — 스탯형 장비 없음. 착용형은 **헤드램프 하나만** 예외 — 실물이되 주우면 머리에 쓴다, 2026-09-05) |
| `bUsableAsTool` | bool | 몬스터를 상대하는 도구로 쓸 수 있나 |
| `ItemDef` | `Alias` | 들고 다닐 때 어떤 아이템으로 취급하나 |
| `UnlockDay` | int | 몇 일차부터 살 수 있나 |
| `MaxOwned` | int | 최대 소지 수 (0 = 무제한) |
| `Gimmicks` | Instanced | 이 장비에만 있는 동작 |

**`ItemDef`가 장비를 아이템 시스템에 연결한다.** 프라이팬을 들면 운반 속도가 느려지고, 던질 수
있고, 떨어뜨릴 수 있다 — 계란과 같은 규칙을 탄다 (장비도 아이템 운반 규칙 하나를 공유한다).

---

## 트럭 패널

패널 항목은 **장비 구매 목록**이 기본이다 — 기준은 `bPhysicalProp` 컬럼이 지킨다.
시동(밤 시작) 같은 트럭 상호작용을 패널에 함께 둘지는 미정이다.

---

## 원정과 분실

장비는 원정에 들고 갈 수 있고, **두고 오면 잃는다** — 밤이 끝나면 맵과 함께 사라진다.
다시 사는 값이 그 대가다.

---

## 지금 있는 것

| Alias | Price | Slot | 도구? | 비고 |
|---|---|---|---|---|
| `frying_pan` | 200 (초안) | `Carryable` | ✔ | 요리에 필요하고 닭도 밀친다. **4d(2026-09-06)**: 아이템 표 `frying_pan` 행 + `ACL9FryingPan` 실물 — 4f 전엔 `cl9.additem frying_pan`으로 꺼낸다 ([../../systems/cooking.md](../../systems/cooking.md) 구현 상태) |
| `cart` | 미정 | `Carryable` | ✕ | `CartOnly` 아이템 운반을 연다. 끌면 `CartMult` 0.75 — 물건 정의는 [item/cart.md](item/cart.md) |
| `pot` | 미정 | `Carryable` | 미정 | 두 번째 조리 용기 — 동시에 두 요리 |
| `standing_lamp` | 미정 | `Carryable` | ✕ | 놓으면 주변을 밝힌다. 조명 세팅([../../systems/light.md](../../systems/light.md)) 검토 후 값을 정한다 |
| `headlamp` | 미정 | `Carryable` (ItemDef `headlamp`, CarrySlot `Head`) | ✕ | 머리에 쓰는 빛 — 손이 빈다. 정전에 안 꺼진다. 4f 전엔 바닥에 놓아 준다 — [item/headlamp.md](item/headlamp.md) |

---

## 미결

- **추가 품목** — 목록은 열려 있다. 아이디어를 계속 받는다
- 수레·냄비·램프의 가격과 세부 값

이더 눈금은 [../economy.md](../economy.md)에 있다.
