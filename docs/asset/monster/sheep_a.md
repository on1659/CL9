# Sheep A — 밝은 털과 짙은 얼굴

상태: 게임 연결 검수됨 — 2026-09-11 `MonsterTable.sheep` → SK/`ABP_Sheep_A`, Lvl_Flat PIE 추격→잡기; 09-15 PushReact. 클립 9개(`Content/Monsters/SheepA/`), 재질 슬롯 배정 09-20. 2인 PIE·LOD 미검수 · 작성일: 2026-09-11 · 변경일: 2026-09-21

2026-09-11 · 현재 sheep 테스트 종의 구매 팩 임시 메시를 교체하는 독립 신규 모델.
큰 털 덩어리와 긴 짙은 얼굴, 가로 눈동자, 옆으로 뻗은 귀, 짧은 꼬리와 갈라진 발굽.
게임 역할/밸런스는 기존 표를 유지한다. 뿔 등 새로운 공격 기능을 추가하지 않는다.

- +X 정면/+Z 위, root 바닥, 단위1m. 몸통/가슴 혼합 가중치, 머리/턱/귀와 네 다리, 꼬리 리그.
- 1개 VertexColor 재질/UV0, 최대 2가중치. 원본 `ArtSource/monster/sheep_a/sheep_a.blend`.
- Idle2s, Walk0.8s(126cm/s), Run0.5s(396cm/s), LookAround2s,
  HitReact0.3s, WakeTelegraph0.8s, Catch0.8s(2026-09-15 재작업 — 아래 절, 처음 0.6s), PushReact0.8s(2026-09-15 신설), Death1.2s. 30fps/in-place, 판정 Notify 없음.
- SleepTime=0이므로 수면 클립 미사용. 필요한 상태는 실제 데이터/AnimBP 연결에서 다시 대조한다.
- 납품: SK/개별 AN FBX, 원본, manifest, 중립/야간과 주요 자세. 바닥/루프/발·털 경계 변형,
  UE 크기/축/리그/클립/실제 행동/2인 표현을 검수한다.

제작 결과: 164.90×90.16×116.40cm, 14,776 tris/24뼈/1재질. 8클립 FBX 저장.
중립·야간·달리기·잡기·사망 렌더 확인, Blender 수치 감사 및 UE 크기/길이/발 위치 검수.
[007 기록](../iterations/007.md). 실제 AnimBP/AI·2인 PIE·LOD/성능은 아직 미완료.

2026-09-29 "귀엽게" 1단계 시안(형태+렌더만, 게임 미연결): `ArtSource/monster/sheep_a_v2/`(blend·`preview/compare_*.png`·`measurements.json`),
스크립트 `Tools/Blender/build_sheep_a_v2.py`. 얼굴 ×1.20/1.28/1.35(목 중심 기준), 주둥이 길이 ×0.72로 짧고 둥글게, 귀 길이 ×0.88,
눈썹 현재/완화 2단계, 털 뭉치 ×1.12 참고안, 채도 팔레트(털 밝은 크림, 얼굴 더 짙은 회색, 귀 안쪽 분홍). 원본 형상·클립·FBX는 그대로다.
안 확정 — 사용자 승인 후 리그·클립 재작업(2단계)으로 간다.

2026-09-30 눈 표정 3단 시안(같은 스크립트·blend, 게임 미연결): 평소(Idle/순찰) 눈썹 t=0.5 · 추격(Chase) t=1.0 · 피격(HitReact) **아픈 눈**.
아픈 눈 프리셋은 스크립트의 `PAIN` 표 — 눈썹을 눈 위로 2cm 당기고 안쪽(콧등 쪽) 끝을 0.55rad 들어 바깥 끝이 눈꼬리를 누르게 하며,
눈·눈동자 세로를 ×0.55로 찡그린다(후보 B, 삼각형 수 변화 없음). 후보 A(눈썹만 들기)는 걱정하는 눈, C(거의 감음)는 조는 눈으로 읽혀 B를 골랐다.
렌더: `preview/compare_pain_candidates.png`(근접, 화난 눈 대비), `preview/compare_three_states.png`(neutral.png 카메라). 비교용 얼굴 배율 1.28.

2026-09-15 반응 클립 재작업(animate_creature_reacts.py, [README](README.md) 표): 돼지와 같은 사족 규칙 — Catch 0.6→**0.8초** 박치기(코 앞으로 +20cm, 머리 좌우 흔들기), HitReact 0.3초(정점 뒤 복귀), **PushReact 0.8초 신설**(뒤로 붕 → 착지 → 허우적 → 복귀). 바닥 최저점 +1.2cm.
