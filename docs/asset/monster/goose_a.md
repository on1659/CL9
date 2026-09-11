# Goose A — 길목의 경보꾼

2026-09-11 · 신규 제작. 닭 임시 메시를 교체할 독립 외형과 종별 리그.

- 긴 S자 목, 작은 머리와 넓적한 주황 부리, 크고 낮은 흰 몸통, 넓은 물갈퀴.
  목을 세우는 예고와 앞으로 뻗으며 우는 동작이 멀리서 읽혀야 한다.
- 목표 높이 약 170cm, 몸통 폭 약 70cm. +X 정면/+Z 위, root 바닥 원점, Blender 단위 1m.
- root/body, 목 3뼈, head/jaw/tail, 좌우 날개 2뼈와 다리 3뼈씩. 목은 연속 메시/혼합 가중치.
- VertexColor 1재질, UV0. 흰 깃털/회색 날개 경계/주황 부리·발/검은 눈을 큰 색 구역으로 분리.
- Idle 2s, Walk 0.8s, Run 0.5s, LookAround 2s, HitReact 0.3s, WakeTelegraph 0.8s,
  Death 1.2s, Sleep 2s 루프, AlertWindup 0.55s, Honk 0.6s 표현 클립.
  기본 30fps. AlertWindup만 60fps/33프레임 간격으로 정확히 0.55초를 보존하고 UE 길이를 확인한다.
- Catch 없음: 현재 거위 CatchRadius=0. 울음은 판정 Notify를 발생시키지 않는다.
- 원본 `ArtSource/monster/goose_a/goose_a.blend`, 개별 SK/AN FBX 및 manifest/중립·야간 미리보기.
- 검수: 목 과신장·꺾임·부리 개폐·날개 접힘·발 접지·루프, UE 크기/축/클립과 실제 상태 연결.

## 실제 제작 결과

[005](../iterations/005.md): 166.21×79.80×167.30cm, 9,516 tris, 4,840 vertices,
20변형뼈(눈 감기 2뼈 추가), 1재질. 위 10클립/FBX/편집 원본 저장.
걷기126cm/s, 달리기528cm/s. 전 프레임 바닥 관통 없음, root 고정, 5루프 경계 일치.
UE 메시/10클립/재질 임포트와 길이 확인. AlertWindup 0.55초 보존.
UE 발 위치와 Blender 차이 <0.000055cm. 실제 AnimBP·2인 PIE/LOD와 이동 영상·손전등은 미완료.
