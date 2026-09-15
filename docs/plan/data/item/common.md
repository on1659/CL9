# 아이템 공통 — `UCL9ItemDef`

주울 수 있는 물건의 **컬럼 정의**. 개별 아이템의 값과 서사는 이 폴더의 각 문서에 있다 —
[egg.md](egg.md) · [meat.md](meat.md) · [cart.md](cart.md) · [flashlight.md](flashlight.md).

공통 컬럼(`Alias`·`SubAlias`·`Mesh`·`Icon`·`Gimmicks` 등)은 [../README.md](../README.md) 참조.

---

## 운반

| 컬럼 | 타입 | 뜻 | 기본값 |
|---|---|---|---|
| `CarrySlot` | enum | `Hand` / `Pocketable` — 옛 손·품 구분으로 enum은 남아 있지만 **퀵슬롯 규칙은 `HoldPose`가 정한다**(2026-09-13, 둘 다 칸에 들어간다) / `CartOnly` 수레만(칸에도 손에도 못 든다) / `Head` 머리에 쓴다(주우면 착용, 퀵슬롯과 무관 — [headlamp.md](headlamp.md), 2026-09-05) | `Pocketable` |
| `Weight` | float | 무게 — 들었을 때의 감속이 여기서 계산된다 | **미정** |
| `GripOffset` | Vector | 손에 들었을 때 손 소켓 `HandGrip_R` 기준 상대 위치(cm) — 손잡이가 메시 원점에 없는 물건용. 두 손 물건(냄비·작업등)은 양손 중점 (12.3, −5.4, 17.3) | (0,0,0) |
| `GripRotation` | Rotator | 손 소켓 기준 상대 회전 — 손전등은 렌즈(+X)가 카메라 정면을 보도록 (−17.8, 136.8, 69.3), 두 손 물건은 똑바로 서도록 (1.6, 81.6, 51.8). 1인칭 Hold 자세의 손 방향에서 역산한 값 ([../../../systems/inventory.md](../../../systems/inventory.md)) | (0,0,0) |

그립 두 열의 **정본은 `ItemTable` 행이고 값은 art가 메시에서 역산한다**(고기는 뼈를 잡고, 프라이팬은 손잡이를 —
`Tools/Blender/measure_item_grip.py`, 2026-09-13). 위 표의 수치는 예시일 뿐 표 행이 바뀌면 여기를 고치지 않는다.
| `HoldPose` | enum | 들었을 때 팔 자세 범주 — `Small` 한 손(달걀·고기·팬) / `Flashlight` 앞을 향해 쥠(손전등·헤드램프) / `Large` 두 손(냄비·작업등) — **들고 있는 동안 퀵슬롯 전환·줍기가 잠긴다**(리썰 twoHanded, 2026-09-13) / `None` 자세 없음(수레 — 앞에서 미는 액터라 손에 들지 않는다) | `Small` |
| `bStackable` | bool | 겹치나 | false |

## 사용 (좌클) — 2026-09-12, 2026-09-13 대기→타격

| 컬럼 | 타입 | 뜻 | 기본값 |
|---|---|---|---|
| `UseAction` | enum `ECL9UseAction` | 든 채 좌클을 **누르면** 하는 일 — `None` 무동작 / `ToggleLight` 누르는 순간 든 손전등 켜고 끄기 / `Swing` 누르면 **공격 대기**(무한, 자세만 바뀐다), **떼면** 정면으로 휘둘러 몬스터 타격 — 평범하게 클릭하면 평범한 공격이고, 누르고 있으면 타이밍을 고른다(리썰 삽 방식). 대기 시간은 피해·사거리를 바꾸지 않는다. **먹기는 좌클이 아니라 우클 길게 누르기**(`CL9Rules.UseHoldTime` 0.4초)이고 `Dish` 행만 먹힌다(별도 플래그 없음) — 익은 고기는 좌클이면 휘두르고 우클 길게면 먹는다 | `None` |
| `HitDamage` | int | `Swing`으로 맞힌 몬스터의 HP 피해 — **0이면 피해 없음**. 죽으면 경직은 없다 | 0 |
| `HitStun` | enum `ECL9HitStun` | `Swing`으로 맞힌 몬스터의 경직 종류 — `None` / `Hit`(몬스터 행 `HitStunMs`) / `Push`(몬스터 행 `PushStun` 경직 + `PushKnockback` 넉백 + 범인 추격). 시간·거리 값은 몬스터 행이 갖는다 | `None` |

사거리 200cm·정면 부채꼴·회복 0.6초는 아직 상수다(컬럼 아님). 현재 값: 프라이팬 `Swing`/0/`Push`,
고기(생·익·탄) `Swing`/0/`Hit`, 손전등 `ToggleLight`, 나머지 `None`. `Swing`이 아닌 행에 피해·경직 값이 있으면
로드 검사가 깨진 행으로 잡는다.

**첫 줍기 힌트는 설명이 필요한 종류만** — `UseAction != None`이거나 `Dish`인 행(`UCL9ItemDef::HasUseHint()`).
본 적 있음은 `Alias`별로 내 PC 설정(`UCL9GameUserSettings.SeenItemHints`)에 남는다 (2026-09-13).

**운반은 `QuickSlots`(4) 칸 퀵슬롯이다** ([../rules.md](../rules.md), 2026-09-13). 활성 칸 하나만 손에 실물로
들리고 나머지 칸은 데이터(별칭·상태·굴린 값)만 남는다 — **비활성 칸에서는 빛만 잃고 들키는 건 그대로다.**
칸 전환은 휠·숫자키로 즉시이며 잠금 시간이 없다. 규칙 전문은
[../../../systems/inventory.md](../../../systems/inventory.md).

**들었을 때 얼마나 느려지는지는 유효 무게에서 나온다** — 유효 무게 = `Weight` × 캐릭터의
`CarryMult`([../character.md](../character.md)). 캐릭터와 먹기 버프는 속도가 아니라 무게 축으로
작용한다. 무게→감속 환산은 아이템이 아니라 전역 시스템의 몫이고, 식과 값은 미정이다.

---

## 물리

| 컬럼 | 타입 | 뜻 |
|---|---|---|
| `Radius` | float | 반경 (cm) |
| `bFragile` | bool | 깨지나 |
| `CrackSpeed` | float | 금 가는 충격 (cm/s) |
| `BreakSpeed` | float | 성한 것이 깨지는 충격 (cm/s) |
| `CrackedBreakSpeed` | float | 금 간 것이 깨지는 충격 (cm/s) |
| `bPhysicsEnabled` | bool | 놓인 물건이 **진짜 물리**를 타나. 끄면 단순 바운스 시뮬레이션 |
| `Gravity` | float | 낙하 가속 (cm/s²) |
| `Restitution` | float | 바닥 반발 |
| `Friction` | float | 바닥 마찰 (수평 감쇠) |
| `AngularDamping` | float | 각감쇠 (1/s) — 각속도가 매초 이 비율로 준다. 구 몸체는 마찰만으로 구르기가 서지 않아 이 값이 세운다. 물리 행만. **초안** 3.0 |
| `LinearDamping` | float | 선형 감쇠 (1/s) — 공기 저항처럼 속도가 매초 이 비율로 준다. 물리 행만. **초안** 0.3 |

**`bPhysicsEnabled`는 기본이 켜짐이다** (2026-09-03 결정) — 놓인 물건은 구르고 미끄러지다
자연스럽게 멈춘다. 수레도 켠다 — `bMeshIsBody`로 메시가 몸체가 되어 구가 수레째 굴러가던 문제가
없다 (2026-09-13, [cart.md](cart.md)). 끈 행은 호스트가 도는 단순 바운스 시뮬레이션으로 대체된다.
멈춘 물건이 잠드는 규칙(변위로 판정)은 코드 쪽 문서 [../../../systems/item.md](../../../systems/item.md) 물리 절.

**세 문턱의 순서가 규칙이다** — `CrackSpeed < CrackedBreakSpeed < BreakSpeed`.
금이 가면 더 약해진다는 게 이 순서에 들어 있다. 로드 검사가 확인한다.

깨짐은 의도된 마찰이다 — 전력질주가 위험해야 게임이 성립한다.

---

## 소리와 빛

| 컬럼 | 타입 | 뜻 |
|---|---|---|
| `ActorClass` | class | 빛을 내려면 `BP_CL9FlashItem`(스포트, 좌클 토글) — 2026-09-11 클래스 분리 전에는 `bEmitsLight`+`LightKind` 컬럼이었다 |
| `LightRadius` | float | 발광 반경 (cm) |
| `bLightLostInPocket` | bool | 활성이 아닌 퀵슬롯 칸에 있을 때 빛을 잃나 (열 이름은 옛 품 시절 그대로 — 비활성 칸은 액터가 없어 자연히 꺼진다) |
| `CarrierSeenRadius` | float | 발광 소지자가 정전 때 보이는 거리 (cm) |

**발광이 세지면 손전등의 역할이 무의미해지고, 약하면 "들고 있으면 들킨다"가 사라진다.**
이 둘 사이가 좁다.

---

## 표시

| 컬럼 | 타입 | 뜻 | 기본값 |
|---|---|---|---|
| `bShowOnMinimap` | bool | 미니맵에 그려지나 — 종별 값 (전역 원칙 아님) | false |

---

## 가치

| 컬럼 | 타입 | 뜻 |
|---|---|---|
| `ValueMin` / `ValueMax` | int | 판매가 범위 (이더) — 개체마다 굴린다. **구현(2026-09-06)**: `ACL9Item::Value`(복제) — 호스트가 행을 로드할 때 굴리고 상태가 바뀌면(익음·탐·깨짐) 새 행 범위로 다시 굴린다. UI는 범위가 아니라 이 값을 보여 준다. 배율은 4e |
| `bWorthlessIfBroken` | bool | 깨지면 값이 0인가 |
| `bWorthlessIfBurnt` | bool | 타면 값이 0인가 |

여기에 목적지 배율과 일자 배율이 곱해진다. **계란이 눈금의 기준이다** — 계산식과 화폐 단위는
[../../economy.md](../../economy.md).

---

## 요리

| 컬럼 | 타입 | 뜻 | 기본값 | 구 CCL8 상수 |
|---|---|---|---|---|
| `CookResult` | `Alias`+`SubAlias` 쌍 | 익히면 뭐가 되나 (`egg`+`cooked`) | — | |
| `BurnResult` | `Alias`+`SubAlias` 쌍 | 더 두면 뭐가 되나 (`egg`+`burnt`) | — | |
| `CookTime` | float | 익는 데 걸리는 시간 (초) | 12.0 | `CookT` |
| `BurnTime` | float | 타는 데 걸리는 시간 (초) | 24.0 | `BurnT` |
| `BurnWarnTime` | float | 연기 기둥 예고 (초) | 6.0 | `BurnWarnT` |

요리 조작은 놓고·기다리고·집기다 (뒤집기·불 조절 없음 — 현재 설계).

`BurnWarnTime`은 연기 기둥 예고이고 **렌더 전용**이라 판정에 안 들어간다.

탄 요리는 미끼로 쓰인다 — 미끼(`DecoyRadius`)에 연결된 시스템은 현재 2건이다 (거위 잠듦, 탄 요리).

부패(시간 경과로 재료가 상함)는 없다 — 현재 아이템의 상태 변화는 요리 체인뿐이다.

---

## 타입과 먹기

| 컬럼 | 타입 | 뜻 | 기본값 |
|---|---|---|---|
| `ItemType` | enum `ECL9ItemType` | `Ingredient` 재료 / `Dish` 요리 … | `Ingredient` |
| `EatBuffs` | `TArray<FName>` | 먹으면 걸리는 버프들의 별칭 ([../buff.md](../buff.md)) | — |

**먹을 수 있는 것은 `Dish`(요리) 타입뿐이다** — 별도의 "먹을 수 있나" 플래그를 두지 않고
타입이 가른다.

**효과 값은 여기 적지 않는다** (2026-09-08 변경). 아이템 행은 버프 표의 별칭을 가리키기만
하고, 스탯·양·지속초는 [../buff.md](../buff.md)의 행이 갖는다. 전에는 `EatEffects`로 값을
여기 인라인으로 적었는데 그러면 **효과에 이름이 없어서** 화면에 "달리기 +60" 같은 수치밖에
띄울 수 없었고, 같은 효과를 두 음식이 주면 값을 두 번 적어야 했다.

여기 적은 별칭이 버프 표에 없으면 로드 검사가 깨진 행으로 잡는다 — 오타가 조용히 넘어가면
"먹었는데 아무 일도 없다"가 되는데, 그건 화면에도 로그에도 안 남아서 못 찾는다.

효과의 대상·크기·지속을 코드가 아니라 표에서 정한다는 원칙은 그대로다. **HP는 효과 대상이
아니다** — 회복 채널은 트럭 패드다 ([../rules.md](../rules.md)). 먹기는 시간이 지나면
사라지는 일시 효과다 (영구 성장은 현재 범위에 없다).

---

## 지금 있는 것

| 아이템 | 문서 | 상태 |
|---|---|---|
| 계란 (`egg` + cooked/burnt) | [egg.md](egg.md) | 물리·발광·요리 값 있음 — **눈금의 기준** |
| 고기 (`meat` + cooked/burnt) | [meat.md](meat.md) | 안 깨진다, 값 초안 |
| 수레 (`cart`) | [cart.md](cart.md) | CartOnly 운반을 연다 — 상점 행은 [../equipment.md](../equipment.md) |
| 손전등 (`flashlight`) | [flashlight.md](flashlight.md) | 손에 드는 빛 — 값 초안 |

밤당 몇 개가 나오는지는 [../site.md](../site.md)가 정한다.

---

## 기믹 예

- 금계란은 주울 때 소리가 크다
- 특정 아이템은 항아리에 안 들어간다

**판정에 영향을 주는 기믹은 값으로 표현한다.** 컴포넌트가 액터에서 직접 상태를 바꾸면 클라와
갈라진다 ([../README.md](../README.md) 규칙 4).

---

## 미결

- **먹기 효과 값** — 어떤 요리가 어떤 스탯을 얼마나 움직이나. **4i(2026-09-07)에서 구현은 들어갔고
  값은 여전히 초안이다**: `egg`+`cooked`에 `RunSpeed +60 / 15초` 하나만 넣어 검증했다(달리기 654 기준 +9%).
  나머지 요리의 값과, 어떤 요리를 `Dish`로 볼지는 미정 — 지금은 익은 계란·익은 고기가 `Dish`다
- **무게 값과 무게→감속 환산식** — 지친 운반 속도가 탈출 부등식의 항이라, 식이 정해지면
  최대 무게 기준으로 로드 검사에 들어가야 한다
- 요리 체인이 계란 말고 어디까지 가나 — 꿀·버터 같은 재료는 목적지가 생겨야 정해진다
