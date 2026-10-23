# 양계장 실내 에셋 사용법 — Claude용 UE 레벨 배치 안내

2026-10-14 작성. 프로젝트: `C:\Work\CL9\CL9.uproject`, Unreal Engine **5.8**.

이 문서는 제작된 양계장 에셋을 받아 **Unreal에 임포트하고, 전체 배치 확인용 레벨을 만들어 저장·재열기하는 절차**다. 기존 랜덤 맵에 연결하는 후속 작업은 §8에 있다. 이 문서를 작성한 시점에는 새 UE 레벨을 아직 만들지 않았다.

## 1. Claude에 그대로 전달할 시작문

```text
C:\Work\CL9\docs\asset\map\henhouse_indoor_usage.md를 읽고 실행해.
이미 만들어진 양계장 실내 에셋으로 UE 5.8 확인용 레벨을 만들어서 열어줘.
assembly_v05의 placements를 기준으로 개별 FBX를 배치하고,
구매 팩은 PACK_COVERAGE.json의 현재 프로젝트 경로에서 재사용해.
목표는 /Game/Maps/Henhouse/Lvl_HenhouseIndoor_Review.
기존 Lvl_Henhouse와 공용 데이터 표는 보존하고, 이 확인용 레벨부터 완성해.
원본 제작·Tripo 재생성은 하지 말고, 달걀 트레이가 아직 없으면 제외해서 진행해.
임포트·배치 스크립트와 결과 매핑을 저장하고, 레벨을 저장한 뒤 다시 열어
전체 모습과 플레이어 눈높이 화면을 확인해줘. 실제로 연 레벨 경로와 스크린샷을 보고해.
```

이는 **확인용 레벨 제작** 지시다. `PropSets`를 바꿔 실제 게임의 모든 시드에 적용하는 작업은 별도로 요청받았을 때 §8을 따른다. FBX 임포트 성공만으로 레벨 제작을 완료 처리하지 않는다.

## 2. 이미 있는 것과 사용 파일

제작 상태는 [PRODUCTION_LOG.md](../../../ArtSource/map/henhouse_indoor/PRODUCTION_LOG.md) 맨 위와 각 export manifest가 정본이다. 아래 수치는 **2026-10-14 12:55 KST 확인값**이며 작업 시작 시 파일로 다시 확인한다.

| 항목 | 확인된 상태 / 사용할 파일 |
|---|---|
| 직접 제작분 | 주 ID 72개 중 **71개 납품**, 변형 포함 **90 FBX**, export 폴더 78개 |
| 파일·해시 점검 | [delivery_file_audit.json](../../../ArtSource/map/henhouse_indoor/_overview/delivery_file_audit.json), 오류 0. Blender 왕복·미리보기 검수이며 UE 검수는 아님 |
| 개별 모델 | `ArtSource/map/henhouse_indoor/<id>/export/manifest.json` 및 같은 폴더의 `SM_*.fbx` |
| 편집 원본 | 각 manifest의 `source_blend`; A/B 등은 `derived_variant_blends`도 확인 |
| 전체 배치 원본 | [henhouse_indoor_assembly_v05.blend](../../../ArtSource/map/henhouse_indoor/_overview/assembly_v05/henhouse_indoor_assembly_v05.blend) |
| **기계가 읽을 배치 목록** | [assembly_v05/manifest.json](../../../ArtSource/map/henhouse_indoor/_overview/assembly_v05/manifest.json)의 **`placements`**: 135개 배치, 서로 다른 메시 70개 |
| 배치 검수 | [assembly_readback.json](../../../ArtSource/map/henhouse_indoor/_overview/assembly_v05/qa/assembly_readback.json): 캐릭터/주요 장애물 삼각형 교차 0, 원본 해시 보존. UE 충돌·내비 검수와는 별개 |
| 구매 팩 25개 ID | [PACK_COVERAGE.json](../../../ArtSource/map/henhouse_indoor/PACK_COVERAGE.json)의 `pack_ids[id].object_path` |
| 구매 팩 현재 위치 | `/Game/Maps/Henhouse/Props`; 필요한 머티리얼·텍스처는 그 아래 `Materials`, `Textures` |
| 아직 없는 모델 | `egg_tray_stack`. 마지막 Tripo 요청 ID `0273137e-023e-4314-b75f-e8aad2439425`가 마지막 확인에서 99%, GLB 없음. [작업 기록](../../../ArtSource/map/henhouse_indoor/egg_tray_stack/tripo/job.md) 참고. **레벨 제작을 막지 않는다** |
| 기존 게임 레벨 | `/Game/Maps/Lvl_Henhouse`. 이번 배치가 반영된 레벨은 아님 |

`wall_500_mesh`는 주 ID이지만 출력은 `wall_500/export`의 B 변형이다. ID마다 export 폴더가 하나씩 있다고 가정하지 말고 **실제 `*/export/manifest.json`의 `assets`를 수집**한다. 추가 크기·색상 변형도 이 방법으로 포함된다.

`IMAGE_CATALOG.json`의 초기 항목 일부에는 `existing` 같은 과거 상태가 남아 있다. 납품 여부는 export manifest와 파일의 존재, 해시 및 최신 제작 기록으로 판단한다. `tripo/**/*.glb`, `blender/qa/**`의 감면 후보·실패 시도는 게임 임포트 대상이 아니다.

### 시각 기준

![전체 배치](../../../ArtSource/map/henhouse_indoor/_overview/assembly_v05/preview/henhouse_assembly_overview.png)

![사람·몬스터 눈높이](../../../ArtSource/map/henhouse_indoor/_overview/assembly_v05/preview/henhouse_assembly_horror.png)

위 그림은 **Blender 정적 검토 장면**이다. 건물 15×30m, 높이 6.3m에 하역장 15×5m가 붙는다. 전체 그림에서는 내부가 보이도록 천장과 한쪽 외벽을 숨겼다. 실제 실내 레벨에서는 다시 켠다.

## 3. 작업 위치와 시작 전 확인

새 임포트 경로는 기존 양계장 부품과 이름이 겹쳐도 덮어쓰지 않도록 다음을 사용한다. 아래 `Indoor` 폴더와 확인용 레벨은 **이번 후속 작업에서 만들 경로**다.

| 용도 | Unreal 패키지 경로 |
|---|---|
| 신규 직접 제작 메시 | `/Game/Maps/Henhouse/Indoor/<id>/<SM_Name>` |
| 신규 공통 정점색 재질 | `/Game/Maps/Henhouse/Indoor/Materials/M_CL9_Henhouse_VertexColor` |
| 확인용 레벨 | `/Game/Maps/Henhouse/Lvl_HenhouseIndoor_Review` |
| 필요한 확인용 BP·카메라 보조 에셋 | `/Game/Maps/Henhouse/Indoor/Review` |
| 구매 팩 메시 | **기존** `/Game/Maps/Henhouse/Props/...`를 참조 |
| 실행 결과·매핑·검수 | `Saved/HenhouseProduction/ue_review/` |

1. `CLAUDE.md`와 [세션/에디터 잠금 규칙](../../workflow/sessions.md)을 확인한다. UE를 조작하기 전에 자신의 editor 임대를 잡고, PIE·빌드·commandlet은 각각 필요한 잠금을 사용한다. 다른 세션이 쓰는 에디터를 강제로 종료하지 않는다.
2. 엔진 실행 파일은 `C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe`. 오래된 레지스트리 연결에 맡기지 않는다.
3. 대상 레벨·임포트 폴더가 이미 생겼으면 먼저 열어 출처와 작업 상태를 읽는다. 자기 작업의 재개라면 재사용하고 기존 변경을 지우지 않는다.
4. 파일 인벤토리를 확인한다. 필요하면 프로젝트 루트에서 아래 **읽기/검사 스크립트**를 실행한다. UE 임포트 기능은 아니다.

   ```powershell
   python -X utf8 Tools/Blender/audit_henhouse_delivery_files.py
   ```

5. 이 안내서 작성 시 **실내 90개를 일괄 임포트하고 v05를 UE에 재현하는 전용 실행 스크립트는 없다.** 후속 작업에서 `Tools/Unreal/import_henhouse_indoor.py`, `Tools/Unreal/build_henhouse_indoor_review.py` 같은 별도 스크립트를 작성하고 실행 근거를 남긴다. 이 이름들은 제안이며 존재하는 도구로 가장하지 않는다.

기존 `import_henhouse_structure.py` 등은 옛 `henhouse_structure` 제작물용이고 일부는 `D:/Work/unreal/CL9`를 하드코딩한다. 그대로 실행하지 않는다. 새 경로는 현재 프로젝트 디렉터리에서 구한다. 전체 에셋 저장 대신 **이번에 수정한 패키지와 레벨만 명시해 저장**한다.

## 4. 개별 FBX 임포트와 재질·충돌

### 4.1 입력 목록을 만들기

모든 `<id>/export/manifest.json`에서 다음을 읽어 메시 이름→실제 FBX 경로→Unreal 목적 경로 매핑을 만든다.

| manifest 필드 | 사용 |
|---|---|
| `assets`의 각 값 `object` | 최종 메시 이름. 없으면 해당 키 사용 |
| `fbx_sha256` | 같은 폴더 `<object>.fbx`의 해시 비교 |
| `dimensions_cm` | **XYZ 순서**의 기대 바운드 크기 |
| `dimensions_length_depth_height_cm` | 길이·깊이·높이 의미상의 치수. XYZ와 혼동 금지 |
| `bounds_min_cm`, `bounds_max_cm` | 바닥 피벗·개구부·추가 배관 배치 확인 |
| `bSolid`, `collision_hulls`, `collision_objects` | 충돌 계약 |
| `previews` | 각 변형의 중립광·작업등·손전등 비교 이미지 |
| `source_blend`, `source_tripo` 또는 `source_glb` | 추적용 원본. 상대 경로는 manifest 폴더 기준 |

파일 이름을 새로 추정하지 않는다. 케이지 최종 이름은 사용자 확정 **`SM_Henhouse_CageRow_A`**다. 추가 사일로 파이프처럼 한 manifest에 여러 메시가 들어 있는 경우도 모두 읽는다. 각 FBX는 해당 렌더 메시와 그 UCX를 포함한다.

### 4.2 처음에는 세 종만 시험

`cage_row`, `wall_500` A, `door_frame_250`을 먼저 임포트한다. 아래는 이 납품의 검증 목표와 설정 출발점이며 **이 에셋들의 UE 임포트가 이미 검증되었다는 뜻은 아니다**.

| 항목 | 설정·검증 목표 |
|---|---|
| 임포트 종류 | Static Mesh. Skeletal Mesh/Animation 끄기 |
| 파일 단위 | 개별 FBX마다 한 렌더 메시. 전체 건물을 단일 FBX로 합치지 않기 |
| 크기·축 | Convert Scene/Convert Scene Unit 켜기, Import Uniform Scale=1, 임포트 오프셋 0. +X 정면/+Z 위가 나오도록 검증 |
| Force Front XAxis | 프로젝트의 기존 static 임포터는 true를 사용한다. 현재 UE 5.8 임포트 경로에서 실제 적용됐는지 세 종으로 확인한 뒤 통일 |
| Vertex Color | 파일 정점색 사용/Replace. 흰색 Override 금지 |
| 노멀 | Import Normals를 출발점으로 faceted/flat 면 보존 확인. 무조건 Smooth 재계산하지 않기 |
| 재질·텍스처 자동 생성 | 끄기. 아래 공통 재질을 직접 지정 |
| 자동 충돌 생성 | 끄기. UCX가 있는 메시만 해당 UCX를 읽기 |
| One Convex Hull per UCX | 제공된 UCX 각 개체가 이미 볼록하므로 개체별 hull 유지 |
| 액터 배율 | `(1,1,1)`. 거꾸로 된 축·100배 크기를 액터 스케일로 숨기지 않기 |

정점색 전달과 UCX 명명·렌더 메시 분리는 [Epic FBX Static Mesh Pipeline](https://dev.epicgames.com/documentation/en-us/unreal-engine/fbx-static-mesh-pipeline-in-unreal-engine), 옵션 의미는 [Epic FBX Import Options Reference](https://dev.epicgames.com/documentation/en-us/unreal-engine/fbx-import-options-reference-in-unreal-engine)를 따른다. UE의 Interchange/기존 FBX 경로에 따라 표시 옵션이나 API 적용이 달라질 수 있으므로 실제 결과를 읽어 검증한다. 프로젝트 전체 임포터 설정을 영구 변경하지 않는다.

기대 XYZ 바운드(cm):

- CageRow A: **100×500×220**. 급이통이 있는 쪽이 +X 정면이다.
- Wall 500 A: **40×500×630**. 이번 키트의 긴 축은 **Y**다.
- DoorFrame 250 A: **40×250×630**, UCX 3개, 출입구 안쪽은 비어 있어야 한다.

범용 옛 문서의 “벽 X=길이”, “H=400/580”를 이 납품에 덮어쓰지 않는다. **이 키트는 H=630cm, 바닥 중앙 피벗, manifest의 실제 XYZ가 우선**이다. 사각형 바운드만으로 축의 앞뒤가 맞았다고 판정하지 말고 케이지 급이통 등 비대칭 형태로 방향도 확인한다.

세 종의 크기·정점색·충돌·정면이 맞으면 나머지 납품 FBX를 같은 경로로 처리한다. 결과 파일 `Saved/HenhouseProduction/ue_review/import_manifest.json`에 원본 FBX 경로/해시, 실제 UE object path, 바운드, 재질, UCX 수, 임포터 설정을 기록한다. 재실행 시 같은 원본 해시는 재사용하고, 출처가 다른 동명 에셋은 덮어쓰지 않는다.

### 4.3 정점색 재질

직접 제작분은 `VertexColor RGB → Base Color`, `Roughness=0.92`, `Metallic=0`, `Opaque`, 일반 조명 재질을 사용한다. 자동 생성된 흰 재질을 두면 색이 사라져 보인다. Blender 노드가 FBX를 통해 UE 재질로 그대로 전달된다고 가정하지 않는다.

각 메시의 재질 슬롯은 원칙상 1개다. 면마다 보이는 세 가지 이내 색은 정점색이다. 손전등 밖에서 어두워지는 것이 의도이므로 Unlit/과도한 Emissive로 밝기를 보정하지 않는다.

### 4.4 충돌

- `bSolid=true`: 제공 UCX 수를 확인하고 이동/시야 충돌을 사용한다. 원본이 단일 상자 UCX인 소품은 내부 구멍까지 통과할 수 있는 정밀 충돌 모델이 아니다.
- `bSolid=false`: **충돌 없음**, 자동 충돌·Complex as Simple로 장애물을 만들지 않는다. 짚·얼룩·깃털·실밥·종이·표지류가 포함된다. 메시별 manifest가 우선이다.
- 문틀은 3개 hull로 개구부가 열린다. 천장 개구부 변형은 둘레 4개 hull이며 중앙을 메우면 안 된다.
- `sack_spill`의 사료는 **별도 `spill_feed` 비충돌 메시**다. 고체 포대의 충돌에 사료 바닥 면적을 합치지 않는다.
- 어두운 환기구/바닥 해치 모델 자체가 벽·바닥 구조 메시를 뚫어 주지는 않는다. 실제 통로·스폰 기능이 필요하면 별도 구조/게임 연결이 필요하다.

## 5. 전체 배치를 UE 확인용 레벨로 재현

### 5.1 배치 데이터 읽기

`ArtSource/map/henhouse_indoor/_overview/assembly_v05/manifest.json`의 `placements`만 **현재 배치 목록**으로 사용한다. `delivered_fbx_inputs`에는 이전 버전에서 사용했다가 빠진 입력도 있으므로 이 배열의 길이를 현재 배치 수로 쓰지 않는다. `removed_placements`를 다시 더하지 않는다.

```json
{
  "object": "SM_Henhouse_Floor_500_A_placed_000",
  "source": "SM_Henhouse_Floor_500_A",
  "position_m": [-5, 2.5, -0.3],
  "yaw_deg": 270
}
```

- `source`: §4에서 만든 메시 이름→UE 경로 매핑의 키.
- `object`: 액터 라벨/재실행 식별자로 쓸 수 있는 고유 이름.
- `position_m`: **Blender 장면의 월드 좌표, 미터**. 모듈 로컬 좌표가 아니다.
- `yaw_deg`: Blender Z축 회전, 도.
- 배율은 기본 1. 바닥의 Z=-0.3m는 판 두께 30cm 때문에 윗면을 Z=0에 맞춘 값이다. 모두 Z=0으로 올리면 안 된다.

### 5.2 좌표 변환을 먼저 확정

FBX의 메시 좌표는 임포터가 변환하지만 JSON의 액터 위치·회전은 별도로 변환해야 한다. **메시는 cm로 임포트한 뒤 또 100배 키우지 않는다.**

프로젝트의 기존 왕복 기준인 Blender +X→UE +X, +Z→+Z, Y 반전을 실제로 확인했을 때의 배치식은 다음과 같다.

```text
UE 위치(cm) = (100 * x, -100 * y, 100 * z)
UE yaw(도)  = -Blender yaw
UE scale    = (1, 1, 1)
```

위 식은 **그 축 변환이 검증된 임포트에만** 적용한다. 임포터에서 추가 90도 정면 보정이 들어갔다면 동일한 변환 기준으로 위치/회전/카메라를 함께 변환한다. 개별 액터에 임의 보정각을 덧붙여 서로 다른 기준을 섞지 않는다. 케이지+벽+바닥 1세트를 배치해 급이통 방향·벽 접촉·좌우 관계가 원본과 일치하는지 확인하고 전체 135개로 확장한다.

### 5.3 레벨을 만드는 순서

1. 새 빈 확인용 레벨을 `/Game/Maps/Henhouse/Lvl_HenhouseIndoor_Review`로 만든다. `Lvl_Henhouse`를 덮어쓰거나 기본 시작 맵으로 바꾸지 않는다.
2. `placements`의 **135개**를 개별 StaticMeshActor 또는 재현 가능한 인스턴스로 배치한다. 라벨과 원본 메시 이름, 변환값을 결과 JSON에 기록한다. 예: `HenhouseReview/Structure`, `Props`, `Traces` 폴더로 정리한다.
3. 해당 레벨에 기존 `CL9SiteMap` 생성 액터를 중복 배치하지 않는다. 확인용 수동 배치 위에 런타임 건물이 다시 생성되는 것을 피한다. 게임 모드/자동 생성 의존성은 실제 레벨 설정에서 확인한다.
4. 전체 보기용 카메라와 눈높이 카메라를 만든다. 비교 기준은 Blender `CAM_WholeSpace_Cutaway`, `CAM_PlayerEye_156cm`다. 눈높이 카메라 원위치는 `(0.15, -1.2, 1.56)m`, 바라보는 점은 `(0, 22, 1.75)m`; 위치와 타깃을 같은 기준으로 변환해서 look-at 회전을 구한다.
5. 전체 보기에서는 천장/우측 외벽을 임시 숨겨 내부를 볼 수 있게 하고, 실내 카메라·충돌 검수 시에는 복구한다. 숨김이 저장된 채 최종 실내 검수가 끝나지 않도록 한다.
6. 먼저 중립 조명으로 색·바운드·겹침을 보고, 그다음 작업등/손전등으로 공포 분위기를 맞춘다. Blender 조명의 `power` 숫자는 UE cd/lumen과 동일 단위가 아니므로 그대로 복사하지 않는다.
7. 추가 배치가 필요하면 §6에 따라 구매 팩을 넣되 **기본 135개 재현과 추가분을 구분해 기록**한다. 이 135개에 모든 납품 소품과 팩 소품이 들어 있는 것은 아니다.
8. 레벨과 수정 에셋을 저장하고 다른 레벨로 전환한 뒤 이 레벨을 다시 연다. 누락 참조·색·충돌·카메라 상태를 확인한다. 마지막에는 사용자가 볼 수 있게 **해당 레벨을 열린 상태로 둔다**.

Blender 전체 `.blend`를 통짜 FBX로 임포트하면 개별 소품 출처·충돌·재사용 계약을 잃는다. 전달된 개별 FBX와 placements를 사용한다.

### 5.4 그림에만 있는 물건·캐릭터

135개 배치는 **납품 키트의 배치 수**다. 그림의 사람·몬스터·손전등, 일부 보조 강철 지지대·조명 갓·멀리 보이는 화물문은 포함되지 않는다. `PREVIEW_ONLY` 그룹은 제작 완료된 소품으로 세지 않는다.

사람/몬스터까지 비교하려면 현재 프로젝트의 승인된 캐릭터/몬스터 에셋을 찾아 확인용으로 배치한다. 원본 위치는 농부 `(1.0,3.8,0)m`, 몬스터 `(-3.05,6.6,0)m`이며, 두 위치도 §5.2 기준으로 변환한다. 원본의 농부 키 약161cm, 몬스터 정지 자세 높이 약390.77cm를 유지하고 통로에 맞추려고 임의로 줄이지 않는다. 기존 런타임 에셋이 그림과 다른 버전이면 차이를 보고한다.

정적 포즈 출처는 assembly manifest의 `cast_sources`, 원본 편집 씬은 v05 `.blend`다. 농부의 손전등을 든 팔과 몬스터 frame525 포즈는 **정적 검토용 복사본**이며, 완료된 게임 애니메이션이라고 쓰지 않는다. 단순 레벨 확인을 위해 원래 리그·애니메이션을 덮어쓰지 않는다.

## 6. 구매 팩 소품 사용

두 팩은 이미 구매·임포트되어 있다. 재구매/재생성/재단 대상이 아니다. `PACK_COVERAGE.json`의 `object_path`로 로드하며, `pallet/pallet_up`, `chair/chair_fallen`처럼 같은 메시를 다른 자세로 쓰는 ID도 있다. ID 수와 서로 다른 메시 수가 같다고 가정하지 않는다.

팩 메시의 실제 피벗·정면·치수는 직접 제작분과 다를 수 있으므로 UE 바운드를 읽고 배치한다. 사무실의 `desk_left`는 실제 책상 상판 높이에, 벽걸이는 해당 벽 표면에 맞춘다. 팩의 1/2/4m 벽·4m 바닥으로 이 키트의 **500cm 격자 껍데기를 대체하지 않는다**.

팩 재질은 기존 텍스처를 보존하면서 전용 Material Instance/필요한 사본을 만들어 목재 황토·금속 청회 팔레트로 틴트한다. 정점색이 있다고 가정해서 신규 정점색 재질을 팩 전체에 덮어씌우지 않는다. 수정 범위는 사용한 에셋과 필요한 의존성으로 제한한다.

선택 에셋25개 메시+의존37개, 총62패키지의 이동·읽기 검증은 이미 끝났다. 그 과정에서 UE가 원래 경로 참조를 고치며 **선택되지 않은 vendor 패키지90개를 자동 재저장**한 이력이 있다. 이 이력을 되돌리려고 `Content/Peb`을 통째로 이동/삭제하거나 `.uasset`를 탐색기로 옮기지 않는다. 기존 redirector와 미사용 팩 에셋을 보존한다. 세부 이력은 [PACK_MIGRATION.json](../../../ArtSource/map/henhouse_indoor/PACK_MIGRATION.json)과 제작 기록에 있다.

## 7. 배치 시 꼭 지킬 양계장 규칙

| 항목 | 적용 |
|---|---|
| 격자·높이 | 칸500×500cm, 이번 벽/문틀/기둥 H630cm |
| 일반 고체 소품 | 벽 안쪽0~120cm 띠. 칸 중심 반경130cm와 문·계단 착지는 비우기 |
| 복도 | 기본 벽 띠0~50cm. 허용 소품만 사용하고 출입구를 막지 않기 |
| 중앙 바닥 | 짚·얼룩·깃털 같은 `bSolid=false` 깔개. 높이·충돌은 manifest대로 |
| 문 후보 변 | 런타임에 열릴 수 있는 변은 `SideClosed` 등 실제 경계 조건을 따름 |
| 큰 둥지 E1 | 예외적으로 깊이160cm. **두 변이 항상 벽인 코너**에만 놓고 빈 둥지 유지 |
| 짚 끌린 자국 E2 | 긴 방향이 큰 둥지 쪽을 가리키도록 회전 |
| 흔적 세트 | 분필벽+종이+분필은 한 세트, 달력과 급히 떠난 사무실 세트는 같은 방에 함께 넣지 않기 |
| 천장/벽 부착 | 배관·그릴은 실제 표면 높이로. 덕트 그릴 바닥은 지면+20cm |
| 사일로 | 본체250cm + 별도 파이프로 H630에 연결. 본체를630cm로 늘리지 않기 |
| 계단 디딤판 | 납품 XYZ264×250×20cm. 원문 프롬프트의26cm와 차이는 기록된 명세 충돌이며 실제 계단 연결에서 확인 |

벽 부착 돌출이나 큰 둥지처럼 일반 규칙과 개별 규격이 충돌하는 항목은 이미 제작 기록에 명시되어 있다. 소품을 몰래 줄여 맞추지 말고 배치를 조정하고 예외를 남긴다. 자세한 개별 방·세트 지정은 [오브젝트 명세](henhouse_indoor_objects.md) §3~4와 해당 manifest를 따른다.

## 8. 실제 랜덤 양계장 맵에 연결할 때

**확인용 레벨 제작 이후의 별도 단계**다. 현재 게임의 방은 통짜 방 FBX가 아니라 데이터 표로 조립된다. 이 단계를 요청받았다면 다음을 확인한다.

> **2026-10-16 현황(문서 감사 표기)** — UE 임포트와 확인용 레벨은 끝났다(`Content/Maps/Henhouse/Indoor` SM 90개, `Lvl_HenhouseIndoor_Review`, `ArtSource/map/henhouse_indoor/PRODUCTION_LOG.md` 2026-10-14). `Content/Data/ModuleTable.uasset`(작업 트리, 미커밋)에는 `Henhouse/Indoor/basket`·`egg_box_stack`·`feathers`·`poster`·`stain` 메시 경로가 들어가 있다 — PropSets 연결이 일부 시작됐다는 뜻이다. 어느 모듈·어느 PropSets 인지는 표를 열어 읽는다. `PRODUCTION_LOG.md` 의 "no PropSets/runtime map connection"(10-14)은 그 이전 시점의 기록이다.

- [모듈 규약](../../plan/data/module.md), [조각 배치](../../systems/asset-swap.md), [사이트](../../plan/data/site.md), [생성기](../../systems/map.md).
- 실제 정의: [CL9ModuleDef.h](../../../Source/CL9/Data/CL9ModuleDef.h), [CL9SiteMap.cpp](../../../Source/CL9/World/CL9SiteMap.cpp), [CL9SiteLayout.cpp](../../../Source/CL9/World/CL9SiteLayout.cpp).
- 현재 데이터 에셋: `/Game/Data/PieceTable`, `/Game/Data/SegKindTable`, `/Game/Data/ModuleTable`. 이름만 보고 구조를 추정하지 말고 현재 행·참조·사이트 테마를 읽는다.

| 연결 대상 | 처리 |
|---|---|
| 벽·바닥·천장·하역장 | PieceTable 역할/테마 변형을 새 메시로 연결하고 SegKind의 배치 모드와 높이 확인 |
| 바닥·천장 판 | 자연500cm 납품은 `Grid` 계약. 100cm 큐브를 전제로 하는 `Stretch`에 그대로 넣지 않기 |
| 벽 긴 축 | 현행 Tile 코드는 바운드 X/Y 중 긴 축을 읽는다. 옛 문서의 X축 가정으로 메시를 재제작하지 않기 |
| 문틀/문짝 | DoorFrame과 FlapDoor는 다른 물건이다. 기존 `door`/`flapIn` 행을 둘 다 문틀로 덮어쓰지 말고 별도 배치 역할 필요 여부부터 확인 |
| 일반 소품 | `ModuleTable.PropSets[].Props[]`의 `Mesh`, `Local`, `Yaw`, `bSolid`, `Side`, `SideRule` |
| 상태 있는 시설 | `Fixtures`의 `ActorClass`, `Local`, `Yaw`. 일반 Static Mesh 소품으로 대체하지 않기 |
| 시설 조명 | `LightIntensity`(cd), `LightRadius`, `LightColor`, `LightOffset` 등 실제 Prop 필드. 그림의 광량을 그대로 복사하지 않기 |

**PropSets.Local은 회전 전 모듈 로컬 cm, 원점은 모듈의 최소 모서리, Z는 바닥 윗면**이다. v05 `position_m`는 Blender 월드 좌표이므로 여기에 그대로 붙여넣을 수 없다. 모듈별 크기와 항상 벽인 변을 읽고 각각 다시 배치한다. 소품의 `Side=None`이면 경계 조건을 검사하지 않으므로, 벽 조건이 필요한 소품에는 실제 변을 지정한다.

같은 시드가 모든 클라이언트에서 같은 배치를 만들도록 현행 좌표 해시 경로를 유지한다. 층고는 H630만 바꾸면 끝나는 것이 아니므로 모듈 Height·바닥/천장 두께·층간 연결을 함께 확인한다. 이전 문서의580/610 수치와 최신 납품630의 차이를 실제 데이터에서 해결한다.

런타임 연결 후에는 여러 시드와 회전0/90/180/270도에서 문·계단·스폰 영역·플레이어 통과를 확인하고, 멀티 배치 일치까지 별도로 검수한다. 정적 v05 씬을 그대로 PropSets 전체의 합격 증거로 쓰지 않는다.

## 9. 레벨을 띄웠다고 보고하기 전

확인용 레벨의 완료 조건:

1. 납품 파일→실제 UE 경로 매핑을 남겼고 누락/추가 항목을 명시했다. 기본 재현분135개와 팩/보조 추가분의 수가 구분된다.
2. 케이지·벽·문틀 크기와 +X 정면을 확인했다. 바닥 윗면, 벽630, 천장, 출입구가 맞으며 임의100배/음수 액터 스케일 보정이 없다.
3. 정점색·flat 면·UCX가 맞고, 비충돌 깔개가 이동/시야를 막지 않는다. 문틀 내부가 열려 있고 플레이어 캡슐 기준 통로를 확인했다.
4. 레벨을 저장하고 **다시 열어** 참조가 유지되는 것을 확인했다. 전체 배치와 지면+156cm 눈높이 스크린샷을 저장했다.
5. 에디터에서 해당 레벨을 실제로 열어 둔다. 가능하면 PIE 진입/종료까지 확인한다. 정적 확인용 비행/관찰 모드만 시험했다면 그렇게 쓰고, 실제 CL9 플레이/AI/내비 검수라고 부르지 않는다. 걷기 검수는 현재 플레이어 BP의 게임 모드·규칙 의존성을 맞춘 뒤 실행한다.
6. `PRODUCTION_LOG.md` 맨 위에 실제 레벨 경로, 임포트 수, 제외한 egg tray, 검수 결과와 증거 경로를 추가한다. 이전 제작 기록과 원본 파일은 보존한다.

Claude의 마지막 보고에는 다음만 분명히 남긴다.

```text
열어 둔 레벨: /Game/Maps/Henhouse/Lvl_HenhouseIndoor_Review
원본 FBX → UE 매핑: Saved/HenhouseProduction/ue_review/import_manifest.json
배치 결과/제외 목록: Saved/HenhouseProduction/ue_review/placement_report.json
저장 후 재열기: 성공/실패 및 이유
전체·눈높이 스크린샷: 실제 저장 경로
PIE/걷기/AI 검수: 실제 실행한 범위만
남은 것: 달걀 트레이 상태, 게임용 PropSets 연결 여부
```

위 경로의 결과 파일들은 **후속 배치 작업에서 생성할 산출물**이다. 이 문서의 존재만으로 UE 임포트·레벨 생성·PIE 완료를 뜻하지 않는다.
