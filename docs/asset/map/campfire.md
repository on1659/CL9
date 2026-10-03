# 마당 모닥불 `SM_Campfire_A`

상태: UE 임포트 검수됨(033) · 게임 연결 2026-09-17(`ModuleTable.PropSets`, 소품 빛 컬럼) — 레벨 PIE 육안은 기록 없음 · 작성일: 2026-09-21(카드 소급) · 변경일: 2026-09-21

## 용도와 모습

- 분류 / 용도: map 소품. 마당 모듈 `yard_gate`·`yard_open` 의 `PropSets` 에 세트마다 하나씩 놓이는 시설 조명 소품이다
  (`docs/plan/data/module.md` 모듈 표 1·9행, 2026-09-17). 빛은 메시가 아니라 소품 빛 컬럼(`LightIntensity`·`LightRadius`·`LightColor`·
  `LightOffset`·`LightScattering`·`LightFlicker`·`LightFlickerSpeed`)이 붙이고, 정전이면 작업등과 함께 꺼진다(`docs/systems/light.md` 시설 조명 행).
- 기획·시스템 문서: [module.md](../../plan/data/module.md) `PropSets` 컬럼, [light.md](../../systems/light.md) 시설 조명·볼류메트릭 안개.
- 형태: 돌 테두리·재 바닥·엇갈려 얽힌 장작·잉걸불 위에 정점색 그라데이션(밑동 노랑 → 끝 검붉음) 불꽃 혀. 원본 CL9 기하(`source: Original CL9 geometry`).
- 이번 제작 범위: 정적 소품 메시 + 재질 2종. 불꽃 애니메이션은 리그가 아니라 재질(월드 포지션 오프셋 흔들림)이다.

## 치수와 기술 계약 (manifest `ArtSource/map/campfire/export/manifest.json`, evidence 033)

| 항목 | 값 |
|---|---|
| 원본 크기 X×Y×Z (cm) | 111.49 × 115.30 × 60.29 (UE 임포트 오차 <1e-6cm) |
| 런타임 크기·자동 배율 | 배율 1 — 소품은 아이템식 `Radius` 자동 배율이 없다(`runtime_radius_cm` null) |
| 피벗·정면 | 바닥 중앙(`floor center`), +X 전방 / +Z 위. 바운드 중심 (0, 0, 30.15) |
| 앵커(cm) | `Floor` (−0.47, −1.04, 2.28) · `LightOrigin` (−0.47, −1.04, 42.28) — 모듈 표의 `LightOffset` z60 은 별도 값 |
| 충돌 | UCX 1(닫힌 컨벡스), UE LOD 3 |
| 삼각형·정점·UV | 1,296 tris / 714 vertices / UV 1층 |
| 재질 슬롯 | `M_CL9_Campfire_Body`(정점색, roughness 0.88) · `M_CL9_Campfire_Flame`(정점색, 발광 색 (1, 0.52, 0.16), 파라미터 `FlameStrength`) |
| 원본·FBX·UE 경로 | `ArtSource/map/campfire/campfire.blend` · `export/SM_Campfire_A.fbx`(sha256 `e7199d8d…`) · `/Game/Maps/Henhouse/Props/SM_Campfire_A` + 재질 2개 같은 폴더 |
| 리그 | N/A(정적 소품) |

`FlameStrength`: manifest 의 상태 `off` 0 / `on_preview` 3(Blender 미리보기). UE 재질은 `Tools/Unreal/import_campfire.py` 가 2026-09-18 에 다시
짰다 — Unlit·양면, 발광 = 정점색 × `FlameStrength`(스칼라 파라미터 기본 2.0), 월드 포지션 오프셋 = 정점 알파(높이 비율)로 가중한 사인 흔들림
(X 6.0Hz·Y 4.3Hz, 끝에서 4cm). 모듈 표 쪽 빛 값(12cd·반경 700·오프셋 z60·주황·산란 4·깜박임 0.35/5Hz)은 `module.md`·`light.md` 가 정본이다.

## 내보내기와 검수 기록

- 제작: `Tools/Blender/build_campfire.py`(헤드리스, `static_meshkit.StaticKit`, 씬 `CL9_CAMPFIRE_A`) → `campfire.blend`·`export/`.
  미리보기: `Tools/Blender/preview_campfire.py` → `preview/{neutral,night,flashlight}.png`.
- UE 임포트: `Tools/Unreal/import_campfire.py`(`import_portable_lights.py` 공용 임포터 재사용, 목적지 `/Game/Maps/Henhouse/Props`, 재임포트 허용).
  증거 [033-campfire-imports-before-fit.json](../evidence/033-campfire-imports-before-fit.json)(2026-09-17 04:45 UTC, 높이 70.31cm) →
  [033-campfire-imports.json](../evidence/033-campfire-imports.json)(같은 날 15:41 UTC, 높이 60.29cm 로 맞춘 뒤 재임포트, sha 갱신). LOD 3·UCX 1·재질 2 확인.
- 게임 연결: 2026-09-17 `PropSets` yard_gate (500,500)·yard_open (1000,1000) 배치와 빛 컬럼, 2026-09-18 불꽃 혀·정점색 그라데이션·재질 흔들림
  (`module.md` 모듈 표 비고). 임포트 단계의 게임 PIE 는 없다(033 `actual_gameplay_qa: not tested in import step`).
- 미실행: 정전 소등·깜박임 육안, 2인 PIE, 성능.
- 알려진 것: `docs/systems/light.md` 가 불꽃 재질을 `MI_Campfire_Flame` 이라 적었던 것은 2026-09-21 감사에서 디스크 이름 `M_CL9_Campfire_Flame`(MI 없음, `FlameStrength` 기본 2.0)으로 정정했다.
