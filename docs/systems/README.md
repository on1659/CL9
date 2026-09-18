# 시스템 설계 문서 — 색인

이 폴더가 **"UE에서 어떻게 짓는가"**의 정본이다. 무엇을 만드는지(기획)는 [../plan/README.md](../plan/README.md),
전 시스템 공통 규약은 [../conventions.md](../conventions.md). 아래 표에서 **하려는 일**을 찾아 그 문서로 간다 —
문서마다 첫 인용 블록이 한 줄 요약이니 거기까지만 읽고 맞는지 확인한다.

---

## 하려는 일 → 갈 곳

| 하려는 일 | 문서 (절) |
|---|---|
| **화면 문자열을 추가·수정하고 싶다** ("[E] 줍기", 버튼 문구, 아이템 이름) | [localization.md](localization.md) "문자열 추가 — 세 자리" → `CL9_Strings.csv` 한 행 + `sync_strings.py` |
| 번역을 채우고 싶다 / 언어를 추가하고 싶다 | [localization.md](localization.md) "번역", "구조" |
| **옵션(설정 항목)을 추가하고 싶다** | 먼저 [../plan/settings.md](../plan/settings.md) 체크리스트(열어도 되는 값인가) → [settings.md](settings.md) "옵션 추가"(세 곳 수정) |
| 키 배치를 바꾸거나 리바인딩 규칙을 만지고 싶다 | [input.md](input.md) "기본 키 배치", "리바인딩" |
| 새 조작(Input Action)을 넣고 싶다 | [input.md](input.md) IA 표 + "리바인딩 — 에셋"(PlayerMappableKeySettings) |
| **새 화면(WBP)을 만들고 싶다** / 화면 여닫기·뒤로 가기 | [ui_system.md](ui_system.md) (UI 표 행 추가·화면 히스토리) → 내용 규칙은 [ui.md](ui.md) |
| HUD에 값을 띄우고 싶다 / 프롬프트·라벨 | [ui.md](ui.md) "게임 중 HUD" + [ui_system.md](ui_system.md) "데이터 연결" |
| **치트(콘솔 명령)를 추가하고 싶다** | [cheats.md](cheats.md) "치트 추가 절차" — 함수 하나 + 등록 한 줄 |
| 소리를 넣고 싶다 / 볼륨 | [sound.md](sound.md) (사운드 클래스 필수) · 슬라이더는 [settings.md](settings.md) |
| **새 아이템 종류**를 만들고 싶다 | [item.md](item.md) (액터 틀·SubAlias) → 기획 표는 [../plan/data/item/common.md](../plan/data/item/common.md) |
| 줍기·놓기·던지기·수레 규칙 | [inventory.md](inventory.md) · 입력 경로는 [input.md](input.md) |
| **새 몬스터** / AI 상태 기계·감각·경보 | [monster.md](monster.md) → 기획 표는 [../plan/data/monster/common.md](../plan/data/monster/common.md) |
| 몬스터가 죽으면 떨어지는 것 | [drop.md](drop.md) |
| 이동·스태미나·점프 수치 (CMC) | [character.md](character.md) · 값은 [../plan/data/rules.md](../plan/data/rules.md) |
| 애니메이션·스켈레톤·AnimBP | [animation.md](animation.md) · 모델 제작은 [../asset/README.md](../asset/README.md) |
| 손전등·헤드램프·정전·어둠 | [light.md](light.md) |
| 요리(익힘·탐) | [cooking.md](cooking.md) |
| 주문서·납품·마감 | [order.md](order.md) |
| 가격·자금·구매 계산 | [economy.md](economy.md) · 트럭 패널 화면은 [shop.md](shop.md) |
| 일자 진행·난이도 곡선·밤 흐름 | [balance.md](balance.md) |
| 원정(진입·회수·탈출) 흐름 | [raiding.md](raiding.md) |
| **맵을 만지고 싶다** (모듈 조립·배치 툴·시드) | [map.md](map.md) · 조각 표·에셋 팩 교체는 [asset-swap.md](asset-swap.md) |
| 위험을 플레이어에게 알리는 연출 | [warning.md](warning.md) |
| **소리 낸 자리에 퍼지는 링**(파동) — 소음원 추가·색·반경·벽 차폐 | [wave.md](wave.md) → 기획은 [../plan/wave.md](../plan/wave.md) |
| 전이 연출(시퀀스) | [sequence.md](sequence.md) |
| 판 저장·로드 (호스트) | [save.md](save.md) — 로컬 옵션은 여기 아님 → [settings.md](settings.md) |
| 로비·세션·매칭 (미구현) | [session.md](session.md) |
| 모든 액터의 공통 베이스(네트워크·RPC·Alias 로드) | [actor-base.md](actor-base.md) |
| 표 컬럼·조회·Validate 규칙 | [../data/schema.md](../data/schema.md) · 기획 표 공통은 [../plan/data/README.md](../plan/data/README.md) |
| PIE로 검증하는 순서 | [../testing/pie-scenarios.md](../testing/pie-scenarios.md) |

---

## 문서 목록 (가나다순)

| 문서 | 한 줄 |
|---|---|
| [actor-base.md](actor-base.md) | `ACL9ActorBase` — 아이템·몬스터 공통 베이스(네트워크·RPC·사운드·Alias 데이터 로드) |
| [animation.md](animation.md) | 애니메이션은 표현 — 상태에서 파생, 판정에 안 들어감. 스켈레톤·AnimBP·보유 에셋 |
| [asset-swap.md](asset-swap.md) | 맵을 무엇으로 짓는지(조각 표·배치 규약·테마) — 에셋 팩 교체 절차 |
| [balance.md](balance.md) | 일자가 오르면 무엇이 어려워지나, 밤 하나의 흐름, 실패 조건 |
| [character.md](character.md) | 커스텀 CMC — 이동·스태미나·예측 |
| [cheats.md](cheats.md) | 콘솔 치트 — 추가 절차·명령 목록 |
| [cooking.md](cooking.md) | 열원 위 시간 → 날것·익음·탄 것 |
| [drop.md](drop.md) | 몬스터 처치 드롭 |
| [economy.md](economy.md) | 이더의 유입·유출, 가격 계산 |
| [input.md](input.md) | Enhanced Input — IMC·IA 목록, 기본 키, 리바인딩, 키 표기 `{0}` |
| [inventory.md](inventory.md) | 손 하나 + 품 + 수레 — 운반 마찰 |
| [item.md](item.md) | `ACL9Item` 공통 틀 — 상태 = SubAlias, 수명 |
| [light.md](light.md) | 손전등·헤드램프·정전·렌더 세팅 |
| [localization.md](localization.md) | String Table `CL9` — 마스터 시트, 키 규칙, 번역 파이프라인 |
| [map.md](map.md) | 모듈 조립 맵 생성기, 배치 툴, 디버깅 |
| [monster.md](monster.md) | AI 상태 기계 하나 — 감각·이동·잡기·경보·종별 차이 |
| [order.md](order.md) | 주문서 — 루프의 엔진 |
| [raiding.md](raiding.md) | 원정 — 진입·회수·탈출 |
| [save.md](save.md) | 판 세이브 — 호스트의 것 |
| [sequence.md](sequence.md) | 전이 연출 재생 |
| [session.md](session.md) | 로비~시작 흐름 (미구현, 사전 문서) |
| [settings.md](settings.md) | 로컬 옵션 — 저장·적용·설정 화면·옵션 추가 레시피 |
| [shop.md](shop.md) | 트럭 패널 — 장비 구매·잔여 판매 |
| [sound.md](sound.md) | 사운드 분류·이벤트·에셋 위치·볼륨 믹스 |
| [ui.md](ui.md) | 화면에 무엇을 띄우나 — HUD·각 화면의 내용 |
| [ui_system.md](ui_system.md) | UI 코드 구조 — WBP_Main·UI 표·UIManager·화면 히스토리 |
| [warning.md](warning.md) | 위험 전달 — 월드 연출 우선 |
| [wave.md](wave.md) | 파동 — 소음 이벤트 하나로 모아 각 머신이 그리는 바닥 링 (미구현, 사전 문서) |

---

## 이 폴더의 규칙

- 문서 하나 = 시스템 하나. 새 시스템을 만들면 문서를 만들고 **위 두 표에 한 줄씩** 건다.
- "무엇을"은 [../plan/](../plan/README.md), "왜 이렇게 정했나"는 [../decisions.md](../decisions.md) — 여기에는 "어떻게"만.
- 문서와 코드가 어긋나면 문서가 틀린 것이다 — 고칠 때 decisions.md에 한 줄 남긴다.
