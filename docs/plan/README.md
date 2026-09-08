# 기획 문서 — 색인

이 폴더가 **"무엇을 만드는가"**의 정본이다. 세 층으로 나뉜다.

| 층 | 문서 | 언제 읽나 |
|---|---|---|
| 전체 구조 | [overview.md](overview.md) | 이 게임이 뭔지 처음 볼 때 |
| 전체 기획 | [loop.md](loop.md) | 게임 루프가 어떻게 도는지 알아야 할 때 |
| 전체 기획 | [economy.md](economy.md) | 돈·가격·난이도 곡선·지켜야 할 부등식 |
| 작업 순서 | [build-order.md](build-order.md) | **뭘 만들지 착수하기 전에 반드시** |
| 세부 기획 | [data/](data/) | 표에 행을 추가하거나 컬럼을 만질 때 |
| 참고 게임 | [reference/](reference/) | 리썰컴퍼니·R.E.P.O.가 같은 문제를 어떻게 풀었는지 볼 때 |

---

## 찾는 것 → 갈 곳

| 찾는 것 | 문서 |
|---|---|
| 이 게임이 무슨 게임인가 | [overview.md](overview.md) |
| 하루가 어떻게 흘러가나 (주문 → 원정 → 요리 → 납품) | [loop.md](loop.md) |
| 지금 뭘 만들고 있나, 다음이 뭔가 | [build-order.md](build-order.md) |
| **가격이 어떻게 계산되나 / 무엇으로 사나** | [economy.md](economy.md) |
| 새 아이템을 넣고 싶다 | [data/item/common.md](data/item/common.md) (개별 문서는 item/ 폴더) |
| 새 몬스터를 넣고 싶다 | [data/monster/common.md](data/monster/common.md) (개별 문서는 monster/ 폴더) |
| 새 목적지(양계장 말고 다른 곳)를 넣고 싶다 | [data/site.md](data/site.md) |
| 주문서 종류를 추가하고 싶다 | [data/order.md](data/order.md) |
| 살 수 있는 장비를 추가하고 싶다 | [data/equipment.md](data/equipment.md) |
| **몇 일차에 몬스터가 몇 마리인지 정하고 싶다** | [data/progression.md](data/progression.md) |
| 걷기 속도·스태미나·밤 길이를 바꾸고 싶다 | [data/rules.md](data/rules.md) |
| 플레이어 외형을 추가하고 싶다 | [data/character.md](data/character.md) |
| 벽·상자 같은 맵 조각을 만지고 싶다 | [data/segkind.md](data/segkind.md) |
| 표의 공통 규칙 (조회·기믹·소프트 참조) | [data/README.md](data/README.md) |
| **저쪽 게임은 이걸 어떻게 했나** | [reference/README.md](reference/README.md) |
| 참고 게임에서 CL9로 가져올 것 | [reference/takeaways.md](reference/takeaways.md) |

---

## 이 폴더 밖의 문서

| 문서 | 무엇 |
|---|---|
| [../decisions.md](../decisions.md) | 결정과 번복의 이력. 여기 기록되지 않은 문서 간 충돌은 버그다 |
| [../conventions.md](../conventions.md) | UE 설계 규약 (모듈·클래스·복제 방식) |
| [../systems/](../systems/) | 시스템별 기술 설계 (캐릭터·몬스터·운반·세션 등) |

**이 폴더는 "무엇을 만드는가"다.** UE에서 어떻게 지을지는 conventions.md(규약)와
systems/(시스템별 설계)가 담당한다.

---

## 처음 읽는 순서

```
overview.md  →  loop.md  →  economy.md  →  build-order.md  →  필요할 때 data/
  게임이 뭔지    루프가 어떻게    숫자가 어떻게    지금 뭘          표를 만질 때
                 도는지          엮이는지        만드는지
```
