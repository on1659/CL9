# 참고 — SNS(인스타그램)의 게임 손맛(game feel) 글을 CL9 에 대본 기록 (2026-10-17)

사용자가 [graphic.magazine 의 손맛 릴](https://www.instagram.com/reel/DekF45WpB_w/)을 보고
"이런 식으로 게임을 더 재밌게 만드는 내용이 요즘 많이 올라오는데 싹 조사해서, 우리 게임에 적용할 수 있게
정리해 달라"고 했다. 이 문서는 **로그인 없이 읽을 수 있었던 만큼만** 모은 결과와, 그 기법들이 CL9 에
이미 있는지·없는지를 코드와 표 기준으로 대조한 표다. 결정은 여기 없다 — 적용 여부는
[../decisions.md](../decisions.md)·[../plan/data/rules.md](../plan/data/rules.md) 에서 정한다.

> **전제 (2026-10-17 당일 결론과의 관계).** [combat-gap.md](combat-gap.md) 는 "밍밍하다·안 무섭다"의 주범이
> 손맛 층이 아니라 구조(도구 하나·닭이 안전함·타격 비용 0·위협이 HP 뿐)라고 결론냈다. 이 문서의 기법은 전부
> 손맛 층이다. 그러니 아래 "없음" 항목을 채워도 구조 문제는 그대로다 — 이 표는 "구조를 고친 뒤 또는 그와
> 별개로 싸게 넣을 수 있는 표현 후보" 목록으로 읽는다.

---

## 1. 무엇을 어떻게 모았나, 어디까지 닿았나

| 경로 | 결과 |
|---|---|
| `graphic.magazine` 프로필 (로그인 없음) | 최근 게시물 12개 링크만 보인다. 각 게시물은 캡션·해시태그·날짜·썸네일 1장을 읽을 수 있다. **영상 프레임은 로그인 벽 뒤라 못 본다.** |
| 해시태그 탐색 `#gamefeel` `#타격감` `#gamejuice` | 각 12개 링크. `#게임손맛` `#히트스톱` `#juice` `#게임필` 은 0개(페이지가 비어 있다). |
| 스토리 | 로그인 필수 + 24시간 소멸. 접근 불가. |
| 합계 | 48개 게시물 캡션 수집. 그중 **게임 손맛과 관련 있는 것은 6개**. 나머지 42개는 주스(음료)·야구단의 "타격감"·일반 인디게임 홍보라 뺐다. |

캡션만으로는 영상이 실제로 무엇을 보여줬는지 알 수 없으므로, 아래 "릴이 말한 것"은 캡션 문장 그대로다.
같은 계정은 보통 YouTube Shorts·Threads 에 같은 영상을 올리므로, 영상까지 봐야 하면 그쪽이 로그인 없이 열린다.

### 관련 게시물 6개

| 계정·날짜 | 캡션 요지 (캡션 그대로, 영상은 못 봄) | 링크 |
|---|---|---|
| graphic.magazine 2026-10-16 | "판정과 데미지는 그대로 두고 맞는 순간을 보여주는 방식만 바꾸면 더 느낌을 살릴 수 있다. **0.1초 멈추고(히트스톱) 찌그러뜨리고(스쿼시) 흔들고(셰이크) 터뜨리면(파티클)** 그게 손맛이다." | [DekF45WpB_w](https://www.instagram.com/reel/DekF45WpB_w/) |
| graphic.magazine 2026-10-15 | 발이 계단에 붙는 이유 — 발밑 레이로 바닥 높이를 재고 발 자리를 정한 뒤 무릎 각도를 역산(IK). | [DegyAELJQxC](https://www.instagram.com/reel/DegyAELJQxC/) |
| graphic.magazine 2026-10-15 | 넘어지는 순간부터는 애니메이션이 아니라 물리(래그돌)가 몸을 굴려서 매번 다르게 넘어진다. | [DehjR45pGbK](https://www.instagram.com/reel/DehjR45pGbK/) |
| crieseusjogos 2022-10-07 (포르투갈어) | "Game Juice = 모든 행동에 반응을 붙이는 것." 예시: 캐릭터 **그림자**와 **점프 뒤 먼지**. 없어도 플레이는 같지만 게임이 "마른" 느낌이 된다. | [Cjavg39Ol-L](https://www.instagram.com/reel/Cjavg39Ol-L/) |
| walterjunior.dev 2025-12-23 (포르투갈어, UE5) | 주먹 몽타주의 **임팩트 프레임에 Niagara 충격파(Shockwave) 노티파이**를 얹었다. "애니메이션이 예뻐도 플레이어가 타격을 못 느끼면 소용없다." | [DSs2SkJkSuw](https://www.instagram.com/reel/DSs2SkJkSuw/) |
| monolithacademy 2025-08-18 | "What is Game Juice?" — 캡션에 내용 없음(제목뿐). | [DNfyL7uidMM](https://www.instagram.com/reel/DNfyL7uidMM/) |

나머지 graphic.magazine 글(레이마칭·렌더링 파이프라인·셰이더 물결·CS2 연막·툰셰이딩·얼굴 필터·복셀·AI 게임)은
렌더링 교양이라 손맛과 무관하다.

### 재조사 (2026-10-19)

사용자가 "저번 조사 이후 글이 더 나왔다"고 해서 같은 경로(`graphic.magazine` 프로필, `#gamefeel` `#타격감` `#gamejuice`)를
로그인 없이 다시 모았다. 링크 48개, 캡션 48개를 전부 읽었고 못 읽은 게시물은 없다.

- 새로 보인 글은 graphic.magazine 두 편뿐이다. 10-17 테슬라 로드스터(`#게임그래픽`), 10-18 "스파이더맨 거미줄을 게임에서 구현하려면?!"(`#게임물리`) —
  둘 다 캡션이 한 줄이고 손맛(타격 피드백)을 다루지 않아 **관련 글은 늘지 않았다.**
- 해시태그 쪽 36개는 1차와 같은 종류다(주스 음료, 두산 베어스의 "타격감", 모바일 게임 홍보, 캡션 없는 인디게임 영상).
  게임 설계 글로는 emergent gameplay 한 편(guillegamedev 2026-03-22)이 있으나 손맛이 아니라 설계 개념이라 뺐다.
- 따라서 2·3절의 대조 결과는 바뀌지 않는다. 1차 때 "CL9 에 없음"이던 ③ 충격파 링·④ 몬스터 HP 핍·⑥ 줍기 클립은
  2026-10-19 에도 `Content/` 에 `WBP_MonsterHp`·`AN_Farmer_FPPickup` 이 없다(`WBP_QuickSlot` 은 있다). ①② 는 구현돼 있다(아래 3절 끝 상자).
- 로그인 없는 화면은 소스마다 상위 12개만 열려서, 사용자가 로그인 상태 화면에서 본 "더 많은 글"은 여기에 잡히지 않았을 수 있다.

### 보강 — 이 릴들이 요약하고 있는 원전

인스타 캡션은 짧아서 기법 이름만 나온다. 아래 세 원전이 같은 목록을 길게 설명하므로, 2절의 기법 목록은
릴 4가지 + 이 원전들의 항목으로 채웠다(원전 항목은 표에 "보강"으로 표시).

- Martin Jonasson · Petri Purho, "Juice it or lose it" (2012 강연) — 같은 블록 깨기 게임에 효과를 하나씩 켜 보이는 시연.
- Jan Willem Nijman(Vlambeer), "The Art of Screenshake" (2013 강연) — 슈팅 게임에 30가지를 하나씩 켜 보이는 시연.
- Steve Swink, *Game Feel* (2008) — "실시간 조작의 촉감"이라는 정의와 측정 틀. `#gamefeel` 해시태그 페이지 상단 설명도 이 정의를 인용한다.

---

## 2. 기법별 CL9 대조표

"CL9 현재"는 2026-10-17 코드·표 기준이다. 근거 열의 파일이 정본이고, 이 표가 낡으면 그쪽이 맞다.

### 2.1 맞는 순간 (릴의 네 가지)

| 기법 | 릴/원전이 말하는 것 | CL9 현재 | 근거 | 판단 |
|---|---|---|---|---|
| **히트스톱** "0.1초 멈추고" | 맞는 순간 양쪽을 잠깐 멈춘다 | **있음.** 몬스터 메시 `bPauseAnims` + 때린 사람의 1인칭 팔 프레임 유지, `CL9Rules.HitStopTime` 0.08초, 액터별(전역 시간 지연 아님). 이동·AI·히트박스는 흐른다 | [../systems/monster.md](../systems/monster.md) 타격의 표현 · [../systems/character.md](../systems/character.md) 소리·팔 히트스톱 · decisions 09-18 | 있음. 다만 **죽이는 타격과 안 죽이는 타격의 히트스톱이 같은 0.08** — 원전(Vlambeer)은 킬에 더 긴 정지를 준다. 후보 ① |
| **스쿼시** "찌그러뜨리고" | 맞은 몸을 타격 축으로 눌렀다 되돌린다(스쿼시·스트레치) | **없음.** 몬스터 메시 스케일을 건드리는 코드가 없다(`SetRelativeScale3D` 는 아이템·수레·랜드스케이프뿐) | `Source/CL9/Monsters/CL9Monster.cpp` grep | 없음. 후보 ② |
| **셰이크** "흔들고" | 화면을 흔든다 | **있음.** 때린 사람 카메라 킥 `ShakeTable` 행 `Hit`(−3°@4Hz·0.12초, 사인파), 잡힌 사람 `Hurt` 셰이크, 스태미나 셰이크. 킥은 휘두르기당 1회, 본인 화면만 | character.md 맞는 느낌 · [../plan/data/rules.md](../plan/data/rules.md) 타격감 | 있음 |
| **파티클** "터뜨리면" | 맞은 자리에서 뭔가 튄다 | **있음.** `NS_CL9_HitPuff`(깃털·먼지, Stun·Dead 진입 때, 죽으면 3배) | [../goal/game-feel.md](../goal/game-feel.md) §2 | 있음. 충격파 링·임팩트 프레임은 없음 — 후보 ③ |

### 2.2 맞는 순간 — 원전의 나머지 항목

| 기법 | 설명 | CL9 현재 | 근거 | 판단 |
|---|---|---|---|---|
| 피격 플래시 | 맞은 몸을 한 색으로 덮었다 푼다 | 있음. 빨간 오버레이 `M_CL9_HitFlash`, `HitFlashTime` 0.15초, 몬스터·사람 공용 | [../goal/hit-feedback.md](../goal/hit-feedback.md) | 있음 |
| 넉백 | 맞은 쪽이 밀린다 | 있음. 아이템 행 `HitKnockback`(팬 120cm) 을 히트스톱 뒤 `HitKnockbackTime` 동안 등속으로. 죽는 타격은 죽음 발사(300cm/s + 위로)로 대신 | [../goal/combat-feel-pass-1.md](../goal/combat-feel-pass-1.md) · decisions 09-22 | 있음 |
| 타격음·표면음 | 맞을 때와 벽을 칠 때 소리가 다르다 | 있음. `SoundTable` `Swing`·`Hit`·`HitSurface`·`MonsterDeath`(합성 WAV 자리표) | [../systems/sound.md](../systems/sound.md) | 있음. 소리 자체는 합성 임시본 — 에셋 교체는 표 행 수정 |
| 사망 연출 | 죽으면 날아가고 전리품이 흩어진다 | 있음. 죽음 발사 + 드랍 호(`DeathLootSpeed` 300) | game-feel.md §3 | 있음 |
| 킬 슬로모 / 킬 정지 | 죽이는 타격만 더 길게 멈추거나 느려진다 | 없음(위 히트스톱 행) | — | 후보 ① |
| 영속(permanence) | 시체·파편·핏자국이 남는다 | 부분. 시체는 남는다(시체를 쳐도 소리 없음). 파편·자국은 없음 | sound.md `Hit` 행 | 보강 후보, 우선 낮음 |
| 적 HP 표시 | 맞은 적 위에 잠깐 체력이 뜬다 | **없음.** `WBP_MonsterHp` 미제작(game-feel.md §5, `HpPipsTime` 열만 있음) | `Content/` glob 확인 2026-10-17 | 명세는 있고 에셋만 없음 — 후보 ④ |
| 예고·되돌림(anticipation·overshoot) | 휘두르기 전 당김과 맞은 뒤 반동 | 있음. 뗌 공격(누르면 대기, 떼면 히트박스), `SwingRecovery`·`SwingBufferTime` | combat-feel-pass-1.md | 있음 |

### 2.3 맞는 순간 밖 — 수집 글이 짚은 것

| 기법 | 글 | CL9 현재 | 근거 | 판단 |
|---|---|---|---|---|
| 발 IK | graphic.magazine 10-15 | 있음. ABP 끝단 `ControlRig(발 IK)`. 맵에 계단이 있다 | [../systems/animation.md](../systems/animation.md) | 있음 |
| 래그돌 | graphic.magazine 10-15 | 없음, **의도적 반려** — 몬스터 스켈레탈 메시에 피직스 에셋이 없다 | game-feel.md §3 "Rejected: ragdoll" | 유지 |
| 점프·착지 먼지, 그림자 | crieseusjogos | 부분. 착지는 소리(`LandSoft`·`LandHard`)와 카메라 딥(착지 스프링)만. 먼지 파티클 없음(코드가 아는 Niagara 는 `HitPuff`·파동 `DotSystem` 둘뿐). 그림자는 엔진 기본 | sound.md `Jump`·`Land*` 행 · character.md 카메라 손맛 | 후보 ⑤(낮음 — 1인칭이라 본인 착지 먼지는 화면 아래에 가려짐, 남의 착지에만 보인다) |
| 임팩트 프레임의 노티파이 VFX | walterjunior.dev | 다름. CL9 는 노티파이가 아니라 **복제된 시각에서 파생**한다(decisions 09-15) — 같은 자리에 효과를 얹는 길은 이미 있다(`TickHitFeel`) | decisions 09-15 | 구조는 있음, 충격파 에셋만 없음 — 후보 ③ |
| 줍기 반응 | 원전(모든 행동에 반응) | **없음.** 줍기 팔 클립 `AN_Farmer_FPPickup`·퀵슬롯 펄스 미제작(game-feel.md §4). 소리 `Pickup` 은 있음 | `Content/` glob 확인 2026-10-17 | 명세는 있고 클립·위젯만 없음 — 후보 ⑥ |
| 이동 손맛 | Swink | 있음. 가속·제동·질주 램프·보브·롤·FOV·착지 스프링, Gate A 73/73 | [../goal/control-feel.md](../goal/control-feel.md) | 있음 |

---

## 3. 적용 후보 (결정 아님 — 사용자가 고른다)

"있음"이 대부분이라, 이 조사로 새로 드러난 빈칸은 여섯 개뿐이다. 전부 **판정·수치를 건드리지 않는 표현 층**이고,
조절 수치는 코드에 박지 않고 `CL9Rules`(`docs/plan/data/rules.md`) 열로 뺀다.

| # | 후보 | 어디를 만지나 | 새 표 열(초안) | 크기 |
|---|---|---|---|---|
| ① | **킬 히트스톱 분리** — 죽이는 타격만 더 길게 멈춤 | `ACL9Monster::TickHitFeel` 의 `HitStopTime` 을 Stun/Dead 로 가름, 팔 히트스톱도 같이 | `KillHitStopTime` (예: 0.12~0.15, `HitFlashTime` 0.15 안) | 작음 — C++ 한 곳 + 열 하나 |
| ② | **스쿼시** — Stun·Dead 진입 뒤 메시 스케일을 타격 축으로 눌렀다 복원 | `TickHitFeel` 옆에 스케일 보간 추가(모든 머신, 복제된 시각에서 파생 — 09-15 원칙 그대로) | `HitSquashScale`(예: 0.8) · `HitSquashTime`(예: 0.1) | 작음. 다만 **몬스터 캡슐·히트박스는 건드리지 않는다**(메시 컴포넌트만) |
| ③ | **충격파 링** — 맞은 자리에 짧은 링/임팩트 프레임 | `HitPuff` 와 같은 자리에서 두 번째 Niagara 또는 `HitPuff` 안에 링 이미터 | `HitRingSystem`(소프트 참조) 또는 기존 `HitPuffSystem` 에셋 수정 | 에셋 작업 — art 몫 |
| ④ | **몬스터 HP 핍** | game-feel.md §5 명세 그대로 — `WBP_MonsterHp` 제작 + `ACL9Monster` 위젯 컴포넌트 | 열 이미 있음(`HpPipsTime`) | ui + gameplay |
| ⑤ | 착지 먼지(남의 착지용) | `LandHard` 소리 나는 자리에 Niagara | `LandPuffSystem` | 낮음 |
| ⑥ | **줍기 팔 클립·퀵슬롯 펄스** | game-feel.md §4 명세 그대로 — Blender 클립 + ABP 분기 + `UCL9QuickSlotWidget::Pulse` | 없음 | art + ui + gameplay |

우선순위를 굳이 매기면 ①②가 가장 싸고(각각 C++ 한 곳), 릴이 꼽은 네 가지 중 CL9 에 없는 유일한 것이 ②다.
④⑥은 9월에 명세까지 써 둔 미제작분이라 새 결정 없이 바로 만들 수 있다.

> **①② 구현 (2026-10-17, 사용자 지시).** `CL9Rules` 열 `KillHitStopTime` 0.12 · `HitSquashScale` 0.8 · `HitSquashTime` 0.1 —
> [../plan/data/rules.md](../plan/data/rules.md) 타격의 표현, [../decisions.md](../decisions.md) 2026-10-17 (손맛). 나머지 ③~⑥은 그대로 후보다.

---

## 4. 이 조사의 한계

- 영상을 못 봤다. "스쿼시"가 릴에서 실제로 어떤 모양이었는지는 캡션의 단어로만 안다.
- 48개 중 6개만 관련이라 표본이 작다. 인스타 해시태그는 로그인 없이 상위 12개만 열리고, 그마저 주제와 무관한 글이 섞인다.
  같은 주제를 더 넓게 보려면 YouTube Shorts(`#gamefeel`·`#juice`)나 위 원전 강연 두 편이 낫다.
- 수집한 캡션 원문은 세션 스크래치에만 있고 저장소에 두지 않았다(남의 글 전문을 옮겨 적지 않는 이 폴더 규칙).
