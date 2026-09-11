# Chicken A — 양계장 주인

2026-09-11 · 모델/8클립 제작 및 UE 임포트 왕복 확인, 실제 행동/2인 PIE 미완료.

[004](../iterations/004.md): 크기 오차 <0.00002cm, 8클립 재생 길이/스켈레톤 연결 확인.
전 프레임 UE 발 위치와 Blender 차이 <0.000046cm. MCP 이미지에서 색과 형태 확인.
실제 AnimBP 연결·추격·경직·잡기·죽음·2인 표현과 LOD/성능은 후속 필수 검수다.

기존 구매 팩의 누락된 애니메이션에 의존하지 않는 CL9 전용 닭이다.
큰 배·짧은 목·작고 매서운 눈·붉은 볏·층진 날개와 꼬리로 어두운 실루엣을 만든다.
몸통 폭 약 66cm, 전체 높이 약 125cm를 시작점으로 사용하며 최종 실측은 manifest에 기록한다.
기존 게임 Radius=36cm는 유지한다. 축 +X 앞/+Z 위, root는 발 사이 바닥, 모델 배율 1.

변형 뼈: root, body, neck, head, jaw, comb, tail, 좌우 wing/tip, 좌우 thigh/shin/foot.
깃털은 묶음으로 모델링하고 몸/머리/날개/다리 관절을 분리한다. 단일 원본과 종 전용 Skeleton을 납품한다.
표면색은 하나의 Vertex Color 머티리얼로 표현하며, UE에서도 VertexColor를 BaseColor로 연결해야 한다.

| 클립 | 초 | 루프 | 기준 |
|---|---:|---|---|
| Idle | 2 | 예 | 발 고정, 호흡/머리 미세 움직임 |
| Walk | 0.8 | 예 | 126cm/s, 지지 구간 발의 후진 속도와 일치 |
| Run | 0.5 | 예 | 396cm/s, 짧은 지지와 공중 구간 |
| LookAround | 2 | 예 | 목/시선 변화 |
| HitReact | 0.3 | 아니오 | HitStunMs=300 |
| WakeTelegraph | 0.8 | 아니오 | 몸 세우기·날개 경고 |
| Catch | 0.6 | 아니오 | 부리/몸 내밀기, 판정 Notify 없음 |
| Death | 1.2 | 아니오 | 옆으로 쓰러짐, 끝 포즈 유지 |

Sleep은 현행 닭의 SleepTime=0으로 실제 미사용이다. 실제 AnimBP가 요구하면 별도 제작하며
다른 종의 Sleep/Alert 요구는 그 종의 카드에서 다룬다. 루트 모션은 없다.

납품: `ArtSource/monster/chicken_a/chicken_a.blend`, Skeletal FBX, 클립별 FBX,
manifest, 중립/야간 렌더, 동작 프리뷰. 기술 측정·시각 검수·UE 상태 연결까지 완료해야 합격이다.

## 2026-09-11 제작 결과

실측 132.81×75.60×133.32cm, 10,196 tris, 5,218 vertices, 17개 변형 뼈, Vertex Color 1슬롯.
8개 Action과 개별 FBX를 저장했다. 원래 목표보다 전체 높이가 약 8cm 커졌으며 볏과 꼬리를 포함한 값이다.
몸통은 기존 Radius=36cm 주변에 두고 전체 외형을 판정 크기로 사용하지 않는다.

첫 감사에서 Catch의 발이 8.2cm, Death가 17.2cm 바닥을 관통했다. Catch는 발 고정 IK,
Death는 실제 변형 메시 최저점에 맞춘 몸 위치 보정으로 수정했다. root는 움직이지 않는다.
Death가 펼친 날개 끝으로 몸을 받쳐 떠 보이는 문제도 발견해 끝 자세의 날개를 접었다.
수정한 Catch/Death/WakeTelegraph와 중립/야간 화면을 다시 확인했다.
`export/blender_animation_qa.json`에 현재 FBX SHA256과 프레임별 실측을 기록했다.
8클립 모두 바닥 최저점 약 +0.88cm, root 이동 <0.001cm. 루프 4종의 시작/끝 행렬 오차 0.
걷기/달리기 지지 구간 발 후진 속도와 126/396cm/s 기준의 최대 오차는 0.0001cm/s 미만이다.
이는 Blender 제자리 동작의 기하 검수이며 UE 실제 이동에서 발 미끄러짐 검수를 대신하지 않는다.
UE/LOD/속도별 재생 및 게임 상태 연결 검수는 남아 있다.
