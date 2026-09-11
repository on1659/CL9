# Pig A — 큰 몸통과 짧은 다리

2026-09-11 · 현행 테스트 종 pig의 구매 팩 임시 외형을 교체하는 신규 제작물.
큰 분홍 몸통, 넓은 코와 두 콧구멍, 접힌 귀, 갈라진 짙은 발굽, 말린 꼬리로 구별한다.
현재 밸런스는 변경하지 않는다. +X 정면/+Z 위, 바닥 root, 단위 1m.

- 원본: `ArtSource/monster/pig_a/pig_a.blend`, 개별 SK/AN FBX, manifest와 미리보기.
- 몸통/가슴 혼합 가중치, 독립 머리/턱/귀, 네 다리 각 3뼈, 꼬리 4뼈.
- 1개 VertexColor 재질/UV0, 정점 최대 2가중치 목표.
- Idle 2s, Walk 0.8s(126cm/s), Run 0.5s(396cm/s), LookAround 2s,
  HitReact 0.3s, WakeTelegraph 0.8s, Catch 0.6s, Death 1.2s. 30fps/in-place.
- SleepTime=0이라 Sleep은 미사용. Catch는 머리/몸을 앞으로 내미는 표현이며 판정을 발생시키지 않는다.
- Blender 변형/발/바닥/루프, 중립/야간 식별 및 UE 크기·리그·클립·실제 상태/2인 표현 검수.

## 제작 결과

[006](../iterations/006.md): 187.23×82.00×109.90cm, 8,404 tris/4,278 vertices,
24뼈/1재질. 위 8클립과 원본/FBX 저장. 중립·야간·달리기·잡기·사망 렌더 확인.
Blender 전 프레임 바닥/루프/지지 발 검사 통과. UE 메시/8클립/재질 임포트와 길이 일치,
발 위치 차이 <0.007202cm. 실제 AnimBP 상태/2인 PIE·LOD/이동 영상/손전등은 미완료.
