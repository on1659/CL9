# Pig A — 큰 몸통과 짧은 다리

상태: 게임 연결 검수됨 — 2026-09-11 `MonsterTable.pig` → SK/`ABP_Pig_A`, Lvl_Flat PIE; 09-15 PushReact; 09-20 처형 Run 1 이 `AN_Pig_Catch` 평가기를 재사용(PIE C-처형-돼지); **09-21 Execute 2.0초 제작(Run 2 — FBX·manifest 완료, UE 임포트·ABP 교체·PIE 는 리드 대기)**. 클립 9개 + Execute(`Content/Monsters/PigA/`), 재질 슬롯 배정 09-20. 2인 PIE·LOD 미검수 · 작성일: 2026-09-11 · 변경일: 2026-09-21

2026-09-11 · 현행 테스트 종 pig의 구매 팩 임시 외형을 교체하는 신규 제작물.
큰 분홍 몸통, 넓은 코와 두 콧구멍, 접힌 귀, 갈라진 짙은 발굽, 말린 꼬리로 구별한다.
현재 밸런스는 변경하지 않는다. +X 정면/+Z 위, 바닥 root, 단위 1m.

- 원본: `ArtSource/monster/pig_a/pig_a.blend`, 개별 SK/AN FBX, manifest와 미리보기.
- 몸통/가슴 혼합 가중치, 독립 머리/턱/귀, 네 다리 각 3뼈, 꼬리 4뼈.
- 1개 VertexColor 재질/UV0, 정점 최대 2가중치 목표.
- Idle 2s, Walk 0.8s(126cm/s), Run 0.5s(396cm/s), LookAround 2s,
  HitReact 0.3s, WakeTelegraph 0.8s, Catch 0.8s(2026-09-15 재작업 — 아래 절, 처음 0.6s), PushReact 0.8s(2026-09-15 신설), Death 1.2s. 30fps/in-place.
- SleepTime=0이라 Sleep은 미사용. Catch는 머리/몸을 앞으로 내미는 표현이며 판정을 발생시키지 않는다.
- Blender 변형/발/바닥/루프, 중립/야간 식별 및 UE 크기·리그·클립·실제 상태/2인 표현 검수.

## 제작 결과

[006](../iterations/006.md): 187.23×82.00×109.90cm, 8,404 tris/4,278 vertices,
24뼈/1재질. 위 8클립과 원본/FBX 저장. 중립·야간·달리기·잡기·사망 렌더 확인.
Blender 전 프레임 바닥/루프/지지 발 검사 통과. UE 메시/8클립/재질 임포트와 길이 일치,
발 위치 차이 <0.007202cm. 실제 AnimBP 상태/2인 PIE·LOD/이동 영상/손전등은 미완료.

2026-09-15 반응 클립 재작업(animate_creature_reacts.py, [README](README.md) 표): Catch 0.6→**0.8초** 박치기 — 첫 0.1초에 코를 앞으로 들이밀고(몸 +20cm, 머리 0.12rad·목 0.10rad — 땅을 파는 게 아니라 앞으로) 0.15~0.55초에 머리를 좌우로 흔들며 밀어붙인 뒤 복귀, 앞발 +5cm; HitReact 0.3초 — 정점(몸 −7cm·코 들림 0.12·머리 −0.30rad·귀 뒤로·꼬리 위) 뒤 복귀; **PushReact 0.8초 신설** — 앞이 들리며 뒤로 붕 떴다가(피치 −0.42rad) 낮게 착지, 네 발 허우적(대각 교대) 뒤 복귀. 바닥 최저점 +1.1cm.

2026-09-21 **Execute 2.0초 신설**(`AN_Pig_Execute`, 실행 execute-clips — 돼지의 주 공격 = 처형, [README](README.md) "공격"): 정면 박치기. 피해자 정면 +100(`CatchRadius`,
잡힌 자리 — 순간 이동 없음)·요 180 을 가정한 액터 공간 앵커 표 `Tools/Blender/execute_anchor.py` 를 농부 짝 `AN_Farmer_Executed_Pig` 와 공유한다.
0~0.3초 예비(카메라 블렌드 중 — 몸 −10cm·머리 숙임 0.32rad·귀 뒤로) → 0.3~0.5 박치기(가속 곡선, 몸 **+15cm 상한** — 실제 거리가 0~100 가변이라
더 파고들지 않는다, 머리 떠올림 −0.38rad·목 −0.22·입 0.30) → **접촉 0.50** → 0.9 복귀 → 1.0~1.6 머리 좌우로 세 번 털고 오른 앞발로 땅 두 번 긁기 →
**2.0 = Idle 첫 프레임**(시체 옆에 선 자세 — `SpectateDelay` 까지 유지, Break 로 로코모션에 블렌드 없이 이어진다). 접촉 프레임 검산(manifest
`contact_probe_cm`, 피해자 공간): 코끝 (+7, 0, 111) = 가슴 앞, 앞발굽 (+68, ±22, 12). 바닥 최저점 +1.1cm. FBX 61프레임·30fps,
sha256 `b5e6c438…`, 미리보기 `preview/react_execute_f{01,10,16,22,28,34,43,52,61}.png`·`react_execute_sheet.png`. 기존 Catch·HitReact·PushReact FBX 는
바이트 그대로(`--clips Execute`). UE: `/Game/Monsters/PigA/AN_Pig_Execute`, `ABP_Pig_A` Execute 가지 평가기 클립을 `AN_Pig_Catch` → 이것으로
(docs/systems/animation.md), 표 `ExecuteDuration` 1.2 → 2.0 은 balance.

## v2 — 귀여움 1단계(형태+렌더, 사용자 승인 대기)

2026-09-29, 실행 monster-cute-pig, [design-concept.md](../design-concept.md)의 "과장되되 귀엽게" 축. `Tools/Blender/build_pig_a_v2.py`가
실물 `pig_a.blend`를 읽기만 하고, 메시의 정점 위치와 정점 색만 바꾼 사본을 `ArtSource/monster/pig_a_v2/pig_a_v2.blend`에 저장한다.
실물 blend·`export/`·UE 에셋은 그대로다. 토폴로지·가중치·리그·액션이 같아서 tris 8,404와 정점 4,278이 유지된다.

- 머리:몸 실측(머리 타원체 대 몸통 타원체): 높이 50/70cm = **71.4%**, 폭 54/76cm = 71.1%, 길이 60/120cm = 50%. 턱·코·귀까지 합친
  머리 묶음은 높이 60cm로 85.7%다. 70%를 넘으므로 머리 크기는 그대로 두었다. 비교용으로 머리 묶음을 +8% 키운 렌더를 한 장 남겼다(사본에는 넣지 않음).
- 눈썹: 현재 값과 완화 값의 2단계를 렌더했다. 완화는 1.2cm 올리면서 머리 표면을 따라 안쪽으로 붙이고, 두께를 ×0.72, 폭을 ×0.85, 길이를 ×0.92로 줄였으며,
  코 쪽 끝을 들어 올렸다(피치 0.05 → −0.10rad). 사본에는 완화 값이 들어 있다.
- 발굽: 모따기 폭을 1.2cm에서 2.0cm로 늘렸다(정점만 이동).
- 채도: 분홍/살구 4색(skin·light·snout·inner)의 sRGB HSV 채도를 ×1.4로 올렸다. 발굽·눈·콧구멍 색은 그대로다. 귀·코·꼬리 형태는 바꾸지 않았다.
- 렌더(`pig_a_v2/preview/`): `compare_neutral.png`(실물 | v2 | v2 머리 +8%), `compare_brow.png`(실물 | v2 현재 눈썹 | v2 완화 눈썹). 값은 `notes.json`에 있다.

### 눈 표정 3단 — 아픈 눈(사용자 승인 대기)

2026-09-30, 실행 eyes-pain-pig. 게임 상태별 눈썹을 **평소(Idle/순찰) = 완화 눈썹, 추격(Chase) = 현재(실물) 눈썹, 피격(HitReact) = 아픈 눈썹**으로 나눈다.
`Tools/Blender/build_pig_a_v2_pain.py`가 `pig_a_v2.blend`를 열어 눈썹 두 섬의 정점만 옮기고, 다른 형태와 색은 건드리지 않는다.

- 아픈 눈썹은 화난 눈썹과 반대 방향이다. 화난 눈썹은 코 쪽 끝이 눈으로 내려오지만(피치 +0.05), 아픈 눈썹은 코 쪽(안쪽) 끝을 들어 올리고
  바깥 끝을 눈꼬리 쪽으로 처지게 해서 찡그린 인상을 만든다. 위치는 머리 타원체 표면을 따라 맞추는데, 이는 완화 눈썹과 같은 방식이다.
- 후보 3개(값은 실물 눈썹 대비): A는 피치 −0.30, B는 피치 −0.45에 0.6cm를 올렸고, C는 피치 −0.60에 안쪽 끝을 1.5cm 휘었다.
  A는 평소 눈썹과 잘 구별되지 않았다. C는 가장 강하게 읽히지만 먼 쪽 눈썹이 거의 세로로 서서 뿔처럼 보였다.
  그래서 **B를 골랐다**(피치 −0.45, dz +0.6cm, 크기 ×(0.95, 0.85, 0.75)). B는 전신 렌더에서도 아픈 표정으로 읽힌다.
- blend의 기본형(Basis)은 완화 눈썹 그대로다. 셰이프 키 `Brow_Current`(추격)와 `Brow_Pain`(피격)을 값 0으로 더했다.
  토폴로지와 정점 수(tris 8,404)는 그대로다. 게임에서 상태에 따라 눈썹을 바꾸는 방식(모프냐 뼈냐)은 2단계에서 정한다.
- 렌더(`pig_a_v2/preview/`): `compare_pain_candidates.png`는 머리 근접 컷으로, 왼쪽부터 평소 | A | B | C다.
  `compare_three_states.png`는 평소 | 추격 | 피격 순서이고, 윗줄은 `neutral.png`와 같은 카메라·조명의 전신 컷, 아랫줄은 머리 근접 컷이다.
  값은 `notes.json`의 `brow_pain_*`에 있다.
