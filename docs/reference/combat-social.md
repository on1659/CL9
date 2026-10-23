# 참고 — SNS(인스타그램)의 전투·타격 글을 CL9 에 대조 (2026-10-19)

사용자가 손맛 재조사([game-feel-social.md](game-feel-social.md)) 결과를 받고 "손맛 말고 타격이나 이런 거"를 물었다.
그래서 주제를 연출(손맛)에서 **전투 자체 — 보스·근접 전투·공격 예고·전투 설계**로 넓혀 다시 모았다.
이 문서는 **로그인 없이 읽을 수 있었던 만큼만**의 결과이고, 결정은 여기 없다
([../decisions.md](../decisions.md)·[../plan/data/rules.md](../plan/data/rules.md) 의 몫).
같은 날 구조 비교 문서 [combat-gap.md](combat-gap.md)(닭과 싸울 이유가 없다)가 이미 있으므로, 이 표는 그 문서의 관점에서 읽는다.

---

## 1. 무엇을 어떻게 모았나

로그인 없이는 해시태그 탐색 페이지가 **일부 태그만** 열린다. 같은 방식으로 이번에 시험한 태그의 결과다.

| 결과 | 태그 |
|---|---|
| 열림 (각 12개) | `#bossfight` `#souslike` `#swordfight` `#gamedesign` `#gameai` (그 밖에 `#gamedevelopment` `#unrealengine` `#게임개발` `#언리얼` 도 열렸으나 전투 주제가 아니라 캡션을 읽지 않았다) |
| 로그인 화면으로 넘어감 (0개) | `#combat` `#melee` `#combatdesign` `#meleecombat` `#hitbox` `#hitstop` `#gamecombat` `#combatsystem` `#actiongame` `#soulslike` `#levelDesign` `#타격` `#전투` `#근접` `#전투시스템` `#게임전투` `#게임디자인` |

열리는 태그만 읽었다(로그인 벽을 우회하지 않았다). 합계 **캡션 60개를 전부 읽었고**, 그중 전투 설계·연출을 실제로 말하는 글은 **2개**, 내용 없이 제목만 있는 글이 2개다.
나머지 56개는 소울라이크 밈·보스전 영상 반응·검술(HEMA·펜싱 등 실제 스포츠)·어린이 코딩 학원 광고·일반 인디게임 개발 일기라 뺐다.
캡션 기준이므로 영상이 보여준 것은 모른다.

### 관련 게시물

| 계정·날짜 | 캡션 요지 (캡션 그대로, 영상은 못 봄) | 링크 |
|---|---|---|
| arthurmunozanimation 2026-03-10 | 게임 *No Rest for the Wicked* 의 보스 공격 애니메이션. "여러 플레이어가 같은 적과 싸울 때, 공격은 **위협적이면서도 모두에게 읽혀야** 한다. 큰 예비 동작·분명한 위험·묵직한 충격." 멀티플레이 업데이트용 기술이라고 적었다. | [DVs4ltBil7M](https://www.instagram.com/reel/DVs4ltBil7M/) |
| deemasque 2026-04-02 | *Devil May Cry 3* 의 광대 보스 Jester 평. 처음엔 위협적이지 않다가 얕보는 순간 짜증으로 바뀌는 "함정", 계속 놀리는 말투·움직임 — "단순히 어려운 게 아니라 **개성이 있는** 보스". | [DWpfqdaEiOA](https://www.instagram.com/reel/DWpfqdaEiOA/) |
| gladiomorigame 2026-01-18·01-29 | *Gladio Mori* — 해시태그 `#physicsbased #Hema #SwordFight` 와 "Daily Duel" 뿐. 물리 기반 검투 게임이라는 것만 안다(캡션에 설명 없음, 좋아요 27K·3K). | [DTq3IskFGos](https://www.instagram.com/reel/DTq3IskFGos/) |
| contraband.software 2026-08-22 | "This makes games feel GOOD" — 캡션에 내용 없음(제목뿐). 손맛 주제로 보이나 확인 불가. | [DcV3rFQtOzm](https://www.instagram.com/reel/DcV3rFQtOzm/) |

---

## 2. CL9 대조

"CL9 현재"는 2026-10-19 문서·코드 기준이다. 근거 열의 파일이 정본이다.

| 글이 말하는 것 | CL9 현재 | 근거 | 판단 |
|---|---|---|---|
| **공격 예고가 크고, 여러 명이 봐도 읽힌다** (arthurmunoz) | **있음.** 잡기 앞에 읽을 수 있는 예고(Pounce)가 오고, 깨어날 때도 0.8초 예고(`WakeTelegraph`)가 **전 플레이어에게 보인다.** 예고 중 맞으면 취소되어 반격 창이 열린다. 상태 기계가 4인 협동 전제 | [../systems/monster.md](../systems/monster.md) 22행·133~169행 · [../goal/monster-pounce-hurt-readability.md](../goal/monster-pounce-hurt-readability.md) | 있음. combat-gap.md 가 짚은 대로 "읽히는 위협" 쪽으로 이미 쌓였다 |
| **분명한 위험·묵직한 충격** (arthurmunoz) | 부분. 충격은 있다(잡기 히트스톱·셰이크·`Hurt`). 예고 **모양**은 몬스터 종별 전용 애니메이션이 아니라 자세·소리 위주라, "큰 예비 동작"이 얼마나 읽히는지는 영상으로 봐야 안다 | monster.md 잡기 예고 · [game-feel-social.md](game-feel-social.md) 2.1 | 판단 보류 — 캡션만으로는 비교 불가 |
| **적에게 개성이 있다 — 얕보게 했다가 함정** (deemasque) | **없음.** CL9 몬스터의 위협은 "읽히는" 방향 하나이고, 얕보게 만드는 장치·놀리는 행동은 없다. combat-gap.md 2절·3절("몬스터가 놀라게 하지 않는다")과 같은 빈칸 | [combat-gap.md](combat-gap.md) 2.2·3 | 같은 빈칸. 이 글이 새로 주는 것은 "예고가 읽히는 것"과 "행동이 예측 불가한 것"을 **한 몬스터가 둘 다** 하는 예(DMC3 Jester)다 |
| **물리 기반 근접 검투** (Gladio Mori) | 다름, 의도적. CL9 근접은 **카메라 기준 판정 + 뗌 공격 + 히트박스**이고 물리 시뮬레이션 전투가 아니다. 넉백만 등속 이동 | [../goal/combat-feel-pass-1.md](../goal/combat-feel-pass-1.md) · decisions 09-22 | 유지. 캡션에 설명이 없어 이 이상 대조 불가 |
| 손맛 제목 글 (contraband) | 내용 없음 | — | 대조 불가 |

---

## 3. 이 조사로 드러난 것

- **새 빈칸은 없다.** 둘 다 이미 combat-gap.md 에 있는 항목(몬스터의 개성·놀람)이거나 CL9 가 이미 하는 것(읽히는 예고)이다.
- 이번에 얻은 근거는 하나다: *No Rest for the Wicked* 는 "여러 플레이어 + 읽히는 공격" 문제를 CL9 와 같은 방향(큰 예고)으로 풀었다. CL9 의 Pounce 예고 방식이 업계에서 어긋나지 않는다는 방증이다.
- 전투를 **구조** 차원(싸울 이유·비용·선택지)에서 다루는 글은 인스타에서 나오지 않았다. 그쪽은 이미 있는 분석([lethal/](lethal/)·[repo/](repo/)·combat-gap.md)이 더 낫다.

## 4. 이 조사의 한계

- 인스타 로그인 없는 화면은 해시태그가 일부만 열리고 소스마다 12개뿐이다. `#hitbox`·`#combatdesign`·`#타격`·`#전투시스템` 같은 핵심 태그는 열리지 않아 **그쪽 글은 하나도 못 봤다.**
- 60개 중 관련 2~4개라 표본이 작고, 영상을 못 봤다.
- 전투 설계를 제대로 보려면 YouTube(예: 보스 공격 읽기·히트박스·락온 강연)나 개발자 블로그가 낫다. 사용자가 앱 브라우저 패인에 직접 인스타 로그인을 해 두면 막힌 태그도 볼 수 있다.
- 캡션 원문은 세션 스크래치패드에만 있고 저장소에 두지 않았다.
