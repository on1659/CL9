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
