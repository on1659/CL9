# 고정 거점 푸드트럭

상태: 게임 연결 검수됨 — 2026-09-11 Lvl_Flat·Lvl_Henhouse 에 `FoodTruck_Body`/`FoodTruck_Steps` StaticMeshActor(패드 −X 650cm·Yaw 180, **위치는 임시 — 사용자 확인 대기**), 023 PIE 계단·통로 캡슐 왕복. 게임 조명·2인·LOD 미검수 · 작성일: 2026-09-11 · 변경일: 2026-09-21

B(걸어 들어가는 트럭·계단·출발 레버·전원 스위치, 2026-10-06)는 맨 아래 [B 절](#b--걸어-들어가는-트럭2026-10-06)이다 — 제작됨·Blender 검수됨, UE 임포트·게임 연결은 아직.

근거: [원정 루프](../../plan/loop.md), [모듈](../../plan/data/module.md).
트럭은 야외에 고정된 거점이며 현재 구동 차량 클래스/치수는 없다. 이 제작은 외형 모델과 별도 뒤 계단이다.
차량 운전/문 조작 기능이나 전체 맵 배치를 추가하지 않는다. 닫힌 운전석, 열린 뒤 작업 공간과 서비스 창구.

제작 초안: 본체 길이 약6m/작업 공간 외폭260cm/바닥64cm/천장 약3m.
뒤쪽 약124cm 출입구와 별도114cm 폭4단 계단(단높이16cm, 총64cm)을 제공한다.
작업 공간은 닫힌 볼록 충돌 하나로 채우지 않고 바닥/벽/지붕/차대/바퀴 구역으로 나눈다.
기존 조리대의300×120cm footprint와 캐릭터 캡슐이 들어가는 공간을 대표 검수한다.

크림색 몸체/탁한 붉은 띠/짙은 운전석 유리·타이어, 큰 헤드라이트와 닭 모양 거점 표시.
불투명 창은 스타일화된 닫힌 운전석용이며 작업 공간의 창구와 뒤 출입구는 실제로 뚫는다.
큰 거점 소품이므로 도장/띠/금속/타이어·불투명 창4재질을 쓴다(공통3슬롯 목표의 기록된 예외).
계단은 금속/안전 표시2재질. 각 UV0와 원본/FBX/3조명 납품. 외부 글꼴/이미지 의존성 없음.

정면+X/바닥 중앙 피벗. 기하 바운드 기준으로 원점이 이동하므로 출입/창구/부착 위치는 manifest 앵커를 쓴다.
현재 가동부 소비 로직이 없어 리그/클립 N/A. 열린 차양과 출입구는 고정 표현이다.
Blender MCP 제작 후 UE 치수/출입구·작업 공간/계단 연결·지원만 작은 검수 공간에서 확인한다.
실제 거점 BP 연결·배치/2인/LOD·성능은 별도 게이트다.

## 제작 결과 — 023

본체630×353.5×325.1cm/5128tris/2850정점/4재질/UCX24,
계단120×114×64cm/224tris/128정점/2재질/UCX4. 각 UV0/FBX/3조명/UE LOD3.
원본은 `ArtSource/map/food_truck/body/food_truck.blend`, `steps/steps.blend`.
납품 UE 경로는 `/Game/Maps/FoodTruck/Body`, `Steps`, 배율1.

본체 원점 기준 UE cm 앵커(Y 축 반전 적용):

| 위치 | X | Y | Z |
|---|---:|---:|---:|
| 계단 원점 | -357 | 78.25 | 0 |
| 후문 바닥 | -297 | 78.25 | 64 |
| 조리대 원점 (Yaw90) | -47 | -33.75 | 64 |
| 검수 통로 중심 | -47 | 70.25 | 64 |

기존 조리대의300×120cm footprint와 뒷바퀴 덮개 사이6cm를 확보했다.
통로 옆 폭119cm, 뒤 출입구124cm. 실제 반경39cm 캐릭터 이동 처리로 4단 상승/통로 왕복/하강 확인.
후문 가장자리의 바퀴 덮개를 피해 위 통로 중심을 사용한다. 계단 철제 면과 충돌의 최대 차이1mm.
차양/거울/환기구/램프 같은 작은 장식에는 별도 충돌이 없다.

제작 재현 순서: build_food_truck → fit_food_truck_surfaces → fit_food_truck_kitchen_clearance
→ fit_food_truck_tail_lamps → clean_food_truck_grille. 기존 납품본에 fit을 다시 적용하지 않는다.
각 manifest에 적용 여부가 있으며 현재 원본은 전부 적용된 상태다.
[023 검수와 남은 게이트](../iterations/023.md). 영구 연결/2인/게임 조명/LOD·성능은 완료로 표시하지 않는다.

## B — 걸어 들어가는 트럭(2026-10-06)

근거: [명세 round-start-truck-intro](../../goal/round-start-truck-intro.md) K3·K4·K8·A8. 4명 이상이 걸어 들어가는 화물칸, 그 앞벽의
출발 레버(ON/OFF 스위치), 건물 첫 방의 전원 스위치. 게임이 `ACL9Truck`·`ACL9DepartLever`·`ACL9PowerSwitch` 로 스폰하고
메시는 `CL9Rules` 셀로 받는다(C++ 에 에셋 경로 없음). 옛 A 에셋은 디스크에 남는다(레벨의 A 액터는 리드가 지운다).

| 메시 | UE 경로 | 치수 cm (X×Y×Z) | tris | 재질 | UCX |
|---|---|---|---:|---|---:|
| `SM_FoodTruck_B` | `/Game/Maps/FoodTruck/B/SM_FoodTruck_B` | 917.5 × 424 × 380 | 2,680 | `M_CL9_FoodTruck_{Steel,Paint,Dark,Red}`(기존) | 18 |
| `SM_FoodTruck_B_Steps` | `/Game/Maps/FoodTruck/B/SM_FoodTruck_B_Steps` | 122 × 279 × 162.3 | 464 | `M_CL9_TruckSteps_{Steel,Mark}`(기존) | 4 |
| `SM_DepartLever_Base_A` | `/Game/Maps/FoodTruck/B/SM_DepartLever_Base_A` | 28.4 × 32 × 18 | 364 | `M_CL9_FoodTruck_{Steel,Dark,Red}`(기존) | 1 |
| `SM_DepartLever_Handle_A` | `/Game/Maps/FoodTruck/B/SM_DepartLever_Handle_A` | 13 × 13 × 68.5 | 420 | `M_CL9_FoodTruck_{Dark,Steel}` + `M_CL9_DepartLever_Knob`(새, 주황) | 1 |
| `SM_PowerSwitch_A` | `/Game/Maps/FoodTruck/B/SM_PowerSwitch_A` | 50 × 55 × 140.5 | 992 | `M_CL9_FoodTruck_{Dark,Steel,Red}` + `M_CL9_PowerSwitch_Hazard`(새, 노랑) | 1 |

트럭 외형 917.5 × 424 은 거울(±212)·바퀴(±205)·뒤 미등(−1.5)·앞 범퍼(916)까지 잰 값이다. 차체 상자는 724 × 360 × 332, 지붕
표지판 위 끝이 380. 재질 4 슬롯은 A 와 같은 기록된 예외. 새 재질 둘은 발광 없음 — 임포터가 없을 때만 상수색으로 만든다.

### 원점과 축

- **트럭 공간**(본체·계단 공통): 원점 = 뒤 입구 가운데 바로 아래 지면(Z 0), +X = 화물칸 안쪽(운전석 쪽), +Z 위. 계단은 같은 원점에서
  −X 로 뻗는다 — 두 컴포넌트 모두 상대 오프셋 0. 게임은 Yaw 180 으로 놓아 입구가 패드를 향한다.
- **레버**(받침·손잡이 공통): 원점 = 경첩 축(레버 로컬 Y) 위의 점, +X = 조작하는 사람 쪽, 손잡이 기본 자세 수직. 받침 바닥 = 경첩 −14.
  UE 피치 −35 = 손잡이 공이 +X(사람 쪽)로, +35 = −X(벽 쪽)로 눕는다. ±35° 에서 손잡이가 앞벽에 5.2 cm 못 미친다.
- **전원 스위치**: 원점 = 기둥 축 아래 바닥, +X = 정면(노란 문·번개 표시), 옆 레버 손잡이는 정면을 볼 때 오른쪽(UE +Y).
  옆 레버 때문에 바운드 중심이 Y 로 2.5 cm 비껴 있다. 발광 없음 — 켜짐 표현은 `SetGlow` 의 빛이 맡는다.
- FBX → UE 는 (X, −Y, Z). 트럭·레버 앵커는 Y 대칭이라 두 좌표가 같다.

### 앵커 — 트럭 공간 UE cm (`body/export/manifest.json` 의 `rules_cells`·`fit_check`)

| 항목 | 값 |
|---|---|
| 내부 상자(`IsInside`) 중심 / 반크기 | (362, 0, 192) / (352, 170, 128) — X 10–714, Y ±170, Z 64–320 |
| 여유 상자(정점·면 0개 확인) | X 10–650, Y ±170, Z 64–314 = 640 × 340 × 250 |
| 뒤 입구 | 폭 260 × 높이 236, 문턱 Z 64, 기둥·상인방 X 0–10 |
| 레버 콘솔(본체 메시·UCX) | X 650–714(앞 붉은 띠 포함), Y ±37, 윗면 Z 164 |
| 레버 경첩 | (669, 0, 178), Yaw 180 — 받침 바닥이 콘솔 윗면에 앉는다 |
| 손잡이 공 중심 | 수직 (669, 0, 236) = 바닥 위 172 · ±35° 바닥 위 161.5 |
| 계단 끝 / 맨 윗단 | (−120, 0, 0) / (−15, 0, 64) — 4단 × 16, 단 깊이 30, 폭 260 |
| 스위치 `GlowLight` / `FocusCenter` | 스위치 공간 (0, 0, 133) / (0, 0, 100) |

패드 기준 제안 `TruckOffset` (−335, 0, 0)·Yaw 180(옛 A 의 뒤끝 자리)이면 계단 끝이 패드 −215 로 패드 가장자리(±200)와 15 cm 떨어지고,
트럭 앞 범퍼는 패드 −1251 까지 간다(옛 A 는 −965). 폭도 353.5 → 424 로 넓어졌다 — 네 레벨에서 겹치는 것은 리드 PIE 로 본다.

### Blender 맞춤 검사 — 통과(`fit_check`)

- 여유 상자 안 본체 정점 0, 상자 면과 교차하는 삼각형 0. 뒤 입구 안쪽 110 광선이 X 650(콘솔 띠)까지 막히지 않는다.
- 계단: 실제 UCX 에 Y 0·±100 세 줄로 1 cm 간격 내려 쏜 높이 — 챌판 16·16·16·16, 최대 16 ≤ 20(MaxStepHeight 45), 맨 윗단과 바닥이 64 로 이어진다.
- 4캡슐(r39·높이 180): 벽 쪽 (180·330, ±125)과 가운데 무리 (200·290, ±45) 두 배치. 서로 거리 150·90 ≥ 78, 전부 내부 상자 안·충돌 밖.
  계단 앞 지면에서 하나씩 걸어 들어가 자리에 서고, 각자 나머지 셋이 선 채로 레버 닿는 자리(캡슐 중심–경첩 ≤ `PickRadius` 120)까지 간다 —
  경로 300·465 cm(벽 쪽), 275·510 cm(무리). 육각 배치로 실내에 동시에 31명이 선다.
- `IsInside`: 실내 X 20 참, 맨 윗단 X −15 거짓, 입구 밖 50 cm(X −50) 거짓, 레버 앞 X 610 참.
- 한계: UCX 상자와 볼록 껍질의 AABB, 5 cm 격자, "45 이하 윗면은 올라선다" 모형이다 — 실제 CharacterMovement 의 계단 오르기·캡슐 쓸기는 PIE A8 이 확인한다.

### 재현과 단계

- 제작: `py -3 Tools/Blender/run_headless.py build_depart_lever.py` → `build_food_truck_b.py`(레버 manifest·blend 를 읽는다) →
  `build_power_switch.py`. 공용 도구 `Tools/Blender/truck_b_common.py`, StaticKit 에 `pivot='origin'`(작성 원점 그대로)을 더했다.
  FBX 는 매번 해시가 바뀐다(내보내기 시각) — 다시 지으면 다시 임포트해야 한다.
- 원본: `ArtSource/map/food_truck_b/{body,steps,lever_base,lever_handle,power_switch}/` — 각 `*.blend`·`export/{FBX,manifest.json}`·`preview/*.png`
  (본체 rear·front·interior_fit·interior_fit_cluster, 레버 neutral·poses).
- UE 임포트(리드): 에디터 콘솔 `py "C:/Work/CL9/Tools/Unreal/import_food_truck_b.py"` → 증거 `docs/asset/evidence/036-food-truck-b-imports.json`
  (SHA 가드·멱등·LOD 3단·치수/바운드/껍질 수 검증).
- 단계: **제작됨** 2026-10-06 · **Blender 검수됨** 2026-10-06(위 맞춤 검사) · UE 임포트 검수됨 — 아직 · 게임 연결 검수됨(Rules 셀·PIE A8) — 아직.
