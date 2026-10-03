# 우리 철망 울타리

상태: 게임 연결 검수됨 — 2026-09-28 UE 임포트(`/Game/Maps/Henhouse/PenFence/SM_Pen_Fence_250_A`, 치수 오차 <0.0001cm, UCX 1개, LOD 3단) ·
`SegKindTable pen`·`PieceTable penFence` 행 등록 · Lvl_Flat 전시 배치 40장 완료(`pen-fence-wire`). henhouse 생성 맵 둘레 연결은 미결
(`ModuleTable PerimeterKind` 열 신설 필요 — 다음 실행 `pen-fence-perimeter`) · 작성일: 2026-09-28 · 변경일: 2026-09-28

`pen`(우리)용 **개방형** 반복 파츠. 판자 울타리 [henhouse_fence.md](henhouse_fence.md)와 같은 `fence` 역할군이지만
성격이 반대다 — 저쪽은 시야를 막는 288cm 판자 벽이고, 이쪽은 **안의 동물이 보여야 하는** 가슴 높이 철망이다.

- 높이 **130cm**. 플레이어 캡슐 반높이 90cm보다 높아 몸통이 넘지 못하고, 눈높이 156cm보다 낮아 우리 안이 내려다보인다.
- 폭 **250cm**(CellSize 500cm의 약수). 로컬 X 길이 / Y 두께 20cm / Z 높이, 바닥 중앙 피벗, 연결면 X±125cm.
  끝 기둥은 반 폭(10cm)씩이라 두 장이 붙으면 기둥 하나(20cm)가 된다 — `henhouse_fence` 관례 그대로.
- 구성: 반 폭 끝 기둥 2 + 가운데 기둥 1 + 위아래 가로 보강대 2 + 칸마다 사선 버팀대 1 + 철망(세로 16 / 가로 7) + 고정 꺾쇠 6.
- 철망 틈은 **시각적으로만** 뚫려 있다. 충돌은 닫힌 UCX 상자 1개(250×20×130cm)로 130cm까지 막아 아이템이 틈으로 새지 않게 한다.
  `blocks_sight=false`로 내보냈다 — 소비 쪽 seg kind 설정에서 Visibility 채널을 막지 않아야 의도대로 보인다.
- 팔레트는 [design-concept.md](../design-concept.md) 규칙 2(채도 상향) 적용. 나무는 `henhouse_fence` 판자(.22/.175/.117)보다
  진하고 선명한 톤(기둥 .150/.100/.048, 보강대 .200/.140/.070), 철망은 어두운 금속(.055/.062/.070, metallic .72).
  형태는 삼각면 로우폴리 유지 — 복셀로 가지 않는다(같은 문서 "실루엣 계열").
- VertexColor 2재질(`M_CL9_Pen_Wood`·`M_CL9_Pen_Wire`), UV0 1장. 정적 구조물이라 리그/클립 N/A.

실행:

- 제작: `py -3 Tools/Blender/run_headless.py build_pen_fence.py`
- 미리보기: `py -3 Tools/Blender/run_headless.py preview_pen_fence.py --blend ArtSource/map/pen_fence/pen_fence.blend`
  → `ArtSource/map/pen_fence/preview/{neutral,night,flashlight}.png`
- UE 임포트(리드/에디터 몫): `Tools/Unreal/import_pen_fence.py` → `/Game/Maps/Henhouse/PenFence`, 증거 `evidence/035-pen-fence-imports.json`

결과(Blender 실측, `ArtSource/map/pen_fence/export/manifest.json`):

| 항목 | 값 |
|---|---|
| 오브젝트 | `SM_Pen_Fence_250_A` |
| 치수 | 250.000 × 20.000 × 130.000 cm |
| 삼각형 / 정점 | 800 / 472 |
| 재질 슬롯 | `M_CL9_Pen_Wood`, `M_CL9_Pen_Wire` |
| UV | UV0 1장 |
| UCX | 1개 — 중심 (0,0,65) 크기 250×20×130 cm |
| 앵커 | `SnapLeft` (-125,0,0) · `SnapRight` (125,0,0) |
| 바운드 중심 | (0, 0, 65) cm |
| FBX SHA256 | 28fa07e1b90413ae57d0fac79967ca362a2c47efc7c9c91d121c2c391a9c6119 |

근거: [파츠 계약](README.md), [소비 경로](../../systems/asset-swap.md), [톤 규칙](../design-concept.md).

미완료: 2장 이음매·90도 회전 검수, LOD·성능·2인 PIE, henhouse 생성 맵 `module_pen` 둘레 연결(`ModuleTable PerimeterKind`).
