# henhouse_roof 제작 카드

상태: 게임 연결 검수됨 — UE 임포트와 3맵 day1/10 차폐, Farm 실내 화면 확인
작성일: 2026-10-04 · 변경일: 2026-10-04

## 용도와 모습

- 정식 Henhouse/Farm/PigFarm 건물과 마당의 위를 덮는 지붕 및 외곽 상부 띠를 닫는 금속 막음판.
- 지붕은 낮은 골판 금속과 평평한 닫힌 바닥으로 만든다. 25cm 주기의20개 골, 윗면높이16~20cm. 제한 팔레트 burnt red의 큰 색면, flat shading, 텍스처 없음.
- 막음판은 높이 배율에 대응하는 단순한 금속 판이며 작은 상하 접힘으로 실루엣을 준다.
- 정점/단면을 자체 작성한다. 기본Cube·기존천장·외부팩의 메시를 복제하지 않는다.
- 기획/시스템 정본: `docs/plan/data/segkind.md`, `docs/systems/map.md`. 제작 승인은 `indoor-roof-closure` 최신 brief.

## 치수와 기술 계약

| 항목 | 지붕 | 막음판 |
|---|---|---|
| 객체 | SM_Henhouse_Roof_500_A | SM_Henhouse_RoofFascia_500_A |
| 크기cm | 500×500×20 | 500×10×100 |
| 피벗 | XY중심, 바닥Z0 | XY중심, 바닥Z0 |
| 바운드cm | X/Y±250, Z0~20 | X±250/Y±5, Z0~100 |
| 축·단위 | +X정면/+Z위, Metric1 | X길이/Y두께/+Z높이, Metric1 |
| 재질 | 단일 M_CL9_Roof_Metal, Corner Color RGBA | 동일 |
| UV | UV0, 비중첩, 양의면적 | 동일 |
| 정점/삼각형 | 166/328 | 24/44 |
| 충돌 | 닫힌 볼록 UCX1, 시각메시와 같은 바운드 | 동일 |
| 런타임배치 | roof/Grid500 | roofFascia/Tile; 길이/높이 가변, 두께 보존 |
| UE목적 | /Game/Maps/Henhouse/Structure/Roof | 동일 폴더 |
| 텍스처/리그/LOD | 없음 | 없음 |

원본 `ArtSource/map/henhouse_roof/henhouse_roof.blend`, FBX/manifest는 같은 폴더의 `export/`, QA는 `qa/`에 있다. 피벗과 자연500cm 치수는 새 Grid/Tile 계약을 따른다.

## 클립 / 상태별 변형

해당 없음.

## 내보내기와 검수 기록

- Blender5.2.2 LTS, 전용 factory-startup headless 생성.
- FBX: 각 시각메시와 UCX만, +X Forward/Z Up, Scale1, Apply Unit활성, 애니메이션비활성.
- 밝은중립 미리보기: `ArtSource/map/henhouse_roof/preview/roof_neutral.png`, `roof_fascia_neutral.png`.
- 저장 원본 재개방 검수 완료. 각각 단일 연결 닫힌 표면/nonmanifold0/면 winding 충돌0/양의체적으로 외향방향 확인. Color=CORNER/FLOAT_COLOR/RGBA alpha1, UV 퇴화/양의면적겹침0, UCX 바운드 일치.
- 각 메시 5×5 위아래 50ray 모두 hit, 2×2 지붕 타일의 공유변·중앙 모서리 18ray 모두 hit. `qa/roof.json`, `roof_fascia.json`, `tiling.json`에 실측 기록.
- 밝은 중립 스틸 두 장을 view_image로 확인했다. 낮은 골판과 금속 막음판의 상하 접힘이 읽히며, XY 범위를 넘는 장식은 없다. 이 원본 스틸은 UE 화면 검증을 대체하지 않는다.
- UE임포트·OpaqueVertexColor재질·roof/roofFascia표와 배치·빌드·3맵PIE 검수는 리드가 수행한다.
- 리드 UE 임포트: 두 메시 자연 치수 오차<0.01cm, XY중심/바닥Z0, UV1/UCX1/단일 Roof_Metal 슬롯 유지. 실제 정점은 UV/노멀 경계로 분리되어 Roof499/Fascia72개다.
- Henhouse/Farm/PigFarm day1/10의6case에서 실제삼각형 upward ray3,912개 및 fascia ray6,881개 모두 miss0, coverage miss0, 엔진템플릿 참조0. `roof-pie-validation.json` 확인.
- Farm 재현 카메라(650,2250,92cm, yaw270/pitch20)의 lit/unlit 스크린샷을 view_image로 확인했다. 실내에 지붕이 차폐되고 하늘이 사라졌으며, doorway standing capsule trace는 blocked=False였다.
- 실제 Roof_Metal와 기존 Ceiling_Wood의 instanced static mesh 사용 플래그 누락은 리드가 false→true로 복구하고 재컴파일/저장했다. 노드·색·원본은 유지했다.
- 영구 임포터: `Tools/Unreal/import_henhouse_roof.py`. 새 재질은 Opaque/TwoSidedFalse/VertexColor/used_with_instanced_static_meshes=True. 기존 재질은 누락된 사용 플래그만 복구하며 노드/색을 덮지 않는다. 같은 원본 해시는 재임포트하지 않고, 알 수 없는 기존 원본 해시는 RuntimeError로 거부한다. 승인된 최초 원본 해시에서만 변경 원본을 재임포트한다. 자동 표/맵/코드 변경은 없다.
- 리드가 영구 임포터를 실제 UE5.8 Farm/PIE없음/dirtymaps없음에서 실행해 두 메시 reimported=False/changed=False와 material_created=False/instanced_usage_repaired=False를 확인했다. 자연 바운드/UCX1/UV1/원본SHA 검증도 통과했다. UE5.8 충돌 스위치 옵션은 True로 유지하면서 authored UCX와 결과를 검증한다.
- UE 증거: `.claude/runs/indoor-roof-closure/roof-import.json`, `roof-pie-validation.json`, `material-usage.json`, `after-photo-context.json`, `interior-door-after-lit.png`, `interior-door-after-unlit.png`.
