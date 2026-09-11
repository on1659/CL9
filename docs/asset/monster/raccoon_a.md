# Raccoon A — 너구리 전용 모델

2026-09-11 · 제작 중. 현재 임시 Fox를 대체할 CL9 신규 모델이다.

Alias=raccoon. 검은 눈가 무늬, 둥근 귀, 넓은 뺨과 짧은 주둥이, 굵은 줄무늬 꼬리로
기존 여우와 구별한다. 네 발 이동이며 잡기에서 머리와 앞발을 내민다. 도둑질 등 미정 AI를 추가하지 않는다.
목표 몸통 폭 약 65cm/귀 포함 높이 약 100cm. 꼬리는 판정 Radius=40cm와 별개인 시각 부위다.
축 +X 앞/+Z 위, root는 발 사이 바닥. 전체 길이/바운드는 제작 후 manifest에 기록한다.

- 독립 종 리그: root/body/chest/neck/head/jaw, 양쪽 귀, 네 다리 각 3뼈, 꼬리 4뼈.
- 몸통과 꼬리의 연속 부위는 인접 뼈 가중치를 섞고, 발가락/얼굴 요소는 해당 뼈를 따른다.
- Vertex Color 1개 재질, UV0 포함. 털은 큰 덩어리와 얼굴·꼬리 색 구역으로 표현한다.
- Idle 2s, Walk 0.8s(140cm/s), Run 0.5s(462cm/s), LookAround 2s,
  HitReact 0.3s, WakeTelegraph 0.8s, Catch 0.6s, Death 1.2s.
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
