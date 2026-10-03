# Raccoon A — 너구리 전용 모델

상태: 게임 연결 검수됨 — 2026-09-11 `MonsterTable.raccoon` → SK/`ABP_Raccoon_A`, Lvl_Flat PIE; 09-15 PushReact; 09-20 저녁 스토커·등 뒤 처형 PIE(C-처형-너구리); **09-21 Execute 2.0초 제작(Run 2 — FBX·manifest 완료, UE 임포트·ABP 교체·PIE 는 리드 대기)**. 클립 9개 + Execute(`Content/Monsters/RaccoonA/`), 재질 슬롯 배정 09-20. 2인 PIE·LOD 미검수 · 작성일: 2026-09-11 · 변경일: 2026-09-21

2026-09-11 · 제작 중. 현재 임시 Fox를 대체할 CL9 신규 모델이다.

Alias=raccoon. 검은 눈가 무늬, 둥근 귀, 넓은 뺨과 짧은 주둥이, 굵은 줄무늬 꼬리로
기존 여우와 구별한다. 네 발 이동이며 잡기에서 머리와 앞발을 내민다. 도둑질 등 미정 AI를 추가하지 않는다.
목표 몸통 폭 약 65cm/귀 포함 높이 약 100cm. 꼬리는 판정 Radius=40cm와 별개인 시각 부위다.
축 +X 앞/+Z 위, root는 발 사이 바닥. 전체 길이/바운드는 제작 후 manifest에 기록한다.

- 독립 종 리그: root/body/chest/neck/head/jaw, 양쪽 귀, 네 다리 각 3뼈, 꼬리 4뼈.
- 몸통과 꼬리의 연속 부위는 인접 뼈 가중치를 섞고, 발가락/얼굴 요소는 해당 뼈를 따른다.
- Vertex Color 1개 재질, UV0 포함. 털은 큰 덩어리와 얼굴·꼬리 색 구역으로 표현한다.
- Idle 2s, Walk 0.8s(140cm/s), Run 0.5s(462cm/s), LookAround 2s,
  HitReact 0.3s, WakeTelegraph 0.8s, Catch 0.8s(2026-09-15 재작업 — 아래 절, 처음 0.6s), PushReact 0.8s(2026-09-15 신설), Death 1.2s.
- 30fps, in-place, root 고정, 판정 Notify 없음. SleepTime=0이므로 Sleep은 현행 실제 미사용.
- 원본 `ArtSource/monster/raccoon_a/raccoon_a.blend`; 메시와 각 클립 FBX, manifest와 미리보기 납품.
- 필수 검수: 가중치/축/바닥·발 접지/루프/꼬리 변형, 중립·야간 식별, UE 리그와 실제 상태 연결/2인 표현.

모델/애니메이션 제작과 최종 UE 합격은 별개로 기록한다.

## 제작 및 Blender 검수 결과 — 2026-09-11

- 귀 내부 중복 형상을 정리하고 외부 면 색으로 대체. 14,776 tris / 7,534 vertices / 24뼈 / 1재질.
- 위 8개 Action, 메시 FBX 1개와 애니메이션 FBX 8개, 편집 원본 저장.
- `export/blender_animation_qa.json`: 전 프레임 평가 메시 min Z 약 0.70cm, root 변위 0,
  4개 루프 첫/끝 뼈 행렬 오차 0. 지지 발 이동 속도 오차 최대 0.0004cm/s 미만.
- `preview/neutral.png`, `night.png`, `walk.png`, `run.png`, `catch.png`, `death.png`,
  `waketelegraph.png`, `lookaround.png` 직접 확인. 눈가/꼬리 무늬와 자세 식별 가능.
- 남은 검수: 이동 영상의 속도 연속성, 손전등, UE 임포트/스켈레톤/재질/LOD/실제 행동/2인 표현.

UE 후속 [004](../iterations/004.md): 메시/8클립 임포트, 크기·재질·스켈레톤·길이 및
전 프레임 발 위치 비교 완료. 최대 위치 차이 <0.007582cm. 실제 행동/2인 PIE/LOD는 미완료.

2026-09-15 반응 클립 재작업(animate_creature_reacts.py, [README](README.md) 표): Catch 0.6→**0.8초** 앞발 휘두르기 — 앞이 들리며(몸 +10cm·+8cm, 피치 −0.40rad) 오른 앞발(0.05~0.30초) 다음 왼 앞발(0.28~0.55초)을 접었다 앞으로 뻗어 내리친다(upper −0.9→+0.4rad, lower +0.8→−0.2rad), 입 0.45; HitReact 0.3초 — 정점(몸 −7cm·머리 −0.30rad 옆으로 0.30) 뒤 복귀; **PushReact 0.8초 신설** — 돼지와 같은 사족 규칙. 바닥 최저점 +0.7cm.

2026-09-21 **Execute 2.0초 신설**(`AN_Raccoon_Execute`, 실행 execute-clips — 너구리의 주 공격 = 등 뒤 처형, [README](README.md) "공격"; 격분 뒤 보통 잡기
`AN_Raccoon_Catch` 는 그대로): 피해자 등 뒤 −120(`ExecuteAnchorDistance`, 텔레포트 고정)·요 0 을 가정한 액터 공간 앵커 표 `Tools/Blender/execute_anchor.py`
를 농부 짝 `AN_Farmer_Executed_Raccoon` 과 공유한다. 0~0.3초 웅크림(몸 −5/−12cm, 피치 +0.08, 코 위 −0.25rad, 귀 세움) → 0.3~0.6 도약(포물선
body **+42/+38cm**, 피치 **−0.45rad** — brief 초안 +50/+45·−0.6 은 코끝이 피해자 머리 위 z 180 으로 나가 줄였다; 앞발 IK 목표 = 피해자 어깨뼈 (−15, ±20, 135),
입 0.45) → **접촉 0.60** → 0.75 유지(입 다물기) → 1.0 착지(피해자가 앞으로 넘어지는 동안 제자리로 내려온다, 착지 눌림 −5cm) → 1.25~1.8 시체 쪽으로 코를 내려
킁킁(목 +0.22·머리 +0.35, 5Hz 잔떨림) → **2.0 = Idle 첫 프레임**(시체 발치에 선 자세 — `SpectateDelay` 까지 유지). 접촉 프레임 검산(manifest
`contact_probe_cm`, 피해자 공간): 코끝 (−10, 0, 152) = 목덜미, 앞발 (−19, ±20, 132) = 어깨뼈(캡슐 표면 −39 안). 뒷발은 도약 중 발 고정 IK 가 닿는
데까지 뻗어 뒤로 끌린다. 바닥 최저점 +0.7cm. FBX 61프레임·30fps, sha256 `32887264…`, 미리보기 `preview/react_execute_f{01,10,19,25,28,34,43,52,61}.png`·
`react_execute_sheet.png`(Execute 는 도약이 프레임을 벗어나지 않게 카메라 배율 1.25). 기존 FBX 는 바이트 그대로(`--clips Execute`). UE:
`/Game/Monsters/RaccoonA/AN_Raccoon_Execute`, `ABP_Raccoon_A` Execute 가지 평가기 클립을 `AN_Raccoon_Catch` → 이것으로(docs/systems/animation.md),
표 `ExecuteDuration` 1.2 → 2.0 은 balance.

## v2 후보 — "귀엽게" 1단계(형태+렌더), 2026-09-29, 사용자 승인 대기

실행 monster-cute-raccoon, [design-concept.md](../design-concept.md) 톤 축·규칙 2. 게임에 연결된 실물(`raccoon_a.blend`·`export/`·UE)은 그대로 두고,
`Tools/Blender/build_raccoon_a_v2.py` 가 원본을 열어 메모리에서만 고친 결과를 `ArtSource/monster/raccoon_a_v2/raccoon_a_v2.blend` 에 쓴다.
- 머리 부위(head·jaw·ear 가중 섬) ×1.15(head 뼈 머리점 기준). 머리 코어 지름 ÷ 몸통 지름: 폭 76.6% → 88.0%, 높이 75.0% → 86.3%(실측, 머리 49×45 → 56.3×51.8cm, 몸통 64×60cm 그대로). 몸통 길이·다리·꼬리는 그대로.
- 주둥이 X ×0.72(뒤끝 기준)·단면 ×1.08 — 끝이 뭉툭해진다. 코는 모양 그대로 따라 들어간다. 전체 길이 211 → 207cm.
- 귀 껍질 Z ×0.86(바닥 기준)·Y ×1.08. 발톱 16개는 끝 고리를 뿌리 반경의 0.8배로 벌리고 뿌리 기준 ×0.7.
- 마스크·꼬리 고리 형태는 그대로. 팔레트만 대비 상향(fur 어둡게, light 더 희게, dark 더 검게, 홍채 호박색, 귀 안쪽 분홍 갈색, 발톱 상아색).
- 눈썹 3단계: 100%(현재) / 50%(+1.8cm 올림, 안쪽 끝 +0.15rad, 두께 ×0.6, 머리 쪽으로 1cm 묻기) / 0%(없음). 오브젝트 기본은 50%이고, 나머지 둘은 메시 데이터 `SK_Raccoon_A_v2_Brow100`·`SK_Raccoon_A_v2_Brow0` 로 보관한다.
- 삼각형 14,776개(0%는 14,328개), 뼈 위치와 액션은 원본 그대로다. 머리를 키웠기 때문에 귀 뼈가 귀 메시보다 낮게 남는데, 이 문제는 2단계(리그·클립)에서 다룬다.
- 렌더: `raccoon_a_v2/preview/neutral.png`, `compare_original_v2.png`(왼쪽 원본, 오른쪽 v2), `compare_brow_100_50_0.png`(왼쪽부터 100%, 50%, 0%).

### v2 눈 표정 3단 — "아픈 눈" 후보, 2026-09-30, 사용자 승인 대기

실행 eyes-pain-raccoon. 게임 상태별 눈: 평소(Idle·순찰) = 눈썹 0%, 추격(Chase) = 눈썹 100%(실물값), 피격(HitReact) = 아픈 눈. 앞의 둘은 위 v2 단계를 그대로 쓰는 비교용 임시값이다.
`Tools/Blender/build_raccoon_a_v2_pain.py` 가 `raccoon_a_v2.blend` 의 눈썹 100% 데이터를 복사해 만들고, 고른 안을 메시 데이터 `SK_Raccoon_A_v2_Pain`(fake user)으로 더했다. Brow100/50/0 과 오브젝트 기본(50%)은 그대로다.
- 실측: 눈썹 섬은 길이 24.6cm, 수평 축이 앞쪽 기준 ±28.6°, 원래 기울기는 거의 수평(−0.027rad)이다. 화난 인상은 기울기보다 눈썹이 마스크 위 가장자리에 낮게 걸친 데서 나온다. 마스크 위 끝은 z 86.2cm 이다.
- 아픈 눈(후보 C 채택):
  - 눈썹: 원래 기울기를 지우고 안쪽 끝이 올라가게 +0.6rad 기울였다. 회전 중심을 바깥 끝에 두어 바깥 끝이 마스크 안으로 내려가지 않는다. 전체를 2cm 올리고 두께는 ×0.6 으로 줄였다. 정점은 머리 중심과의 거리를 원래대로 되돌려 표면에 붙였다. 색은 마스크와 같은 검은색(.012/.013/.015)이라 흰 이마 위에서 선으로 읽힌다. 눈썹 z 는 85.9~95.4cm 이고, 렌더에서 마스크와의 사이에 흰 틈이 남는다.
  - 눈(홍채·동공): 눈 중심보다 홍채 반높이의 25% 아래를 중심으로 세로 ×0.55 눌렀다. 동공은 ×0.8 로 줄였고, 눈을 눈썹 축 방향으로 0.3rad 기울여 안쪽 끝이 올라가게 했다.
- 기각한 후보: A는 흰 눈썹(색 그대로)에 찡그린 눈이다. 흰 이마 위의 흰 눈썹은 음영으로만 보여서 사선이 약하게 읽힌다. B는 C에서 눈 기울기를 뺀 것으로, 눈썹은 걱정하는 모양인데 눈이 수평으로 눌려 "졸린 눈"에 가깝다.
- 알려진 문제: neutral 방향(옆 3/4)에서는 먼 쪽 눈썹의 올라간 안쪽 끝이 머리 윤곽 위로 작은 검은 돌기처럼 보인다. 거위 원본의 먼 쪽 눈썹 돌기와 같은 종류다.
- 렌더: `raccoon_a_v2/preview/compare_pain_candidates.png`(얼굴 쪽 3/4 근접, 왼쪽부터 A, B, C), `compare_three_states.png`(neutral.png 와 같은 카메라 방향·조명, 왼쪽부터 평소 | 추격 | 피격). 자세는 모두 Idle 1프레임이다.
