# 철판 조리대

상태: 본체 게임 연결 검수됨 — 2026-09-11 `BP_CL9Kitchen` Mesh → `SM_Kitchen_A`(배율 1, Yaw 90·z 0, HeatZone 절대 회전은 C++), 020 PIE 놓기·가열·익음→탐. 2인·LOD 미검수 ·
부엌 세트(꾸밈·불꽃·가스불 소리, 아래 절) 제작됨·Blender 검수됨 2026-10-09 — UE 임포트·BP 연결·PIE 미검수 ·
조리 효과(불꽃 키움·상판 열기·아지랑이·김/연기·덩어리·갈변·소리 넷, 아래 절) 제작됨·Blender 미리보기 2026-10-09 — UE 임포트·BP·PIE 미검수 · 작성일: 2026-09-11 · 변경일: 2026-10-09

현재 소비처는 `ACL9Kitchen`/`ACL9Fixture`, `/Game/Items/BP_CL9Kitchen`.
[실제 Lvl_Flat 조회](../evidence/020-kitchen-consumer.json):100cm 기본 큐브에 배율3/1.2/0.9,
세계 크기300×120×90cm/중심Z45cm. HeatZone은 BeginPlay 때 실제 바운드 상면 위60cm로 정해진다.

자체 원본은120×300×90cm/바닥 중앙 피벗/+X 정면으로 만든다. 원래 배치와 같은 작업 면적은
배율1/Yaw90도/바닥Z0으로 복원한다. 이전 큐브 배율을 새 모델에 그대로 적용하지 않는다.
120×300cm의 넓은 평판 철판, 옅은 법랑 몸체, 탁한 붉은 전면 패널, 고정 손잡이/환기구와 다리.
상판90cm 위로 솟는 부분은 없다. 전면 그림은 열원임을 알리는 장식이며 조절 기능을 추가하지 않는다.

UV0/상수 재질3슬롯/단순 충돌은 상판·몸체·다리로 분리한다. 외부 이미지/글꼴 의존성 없음.
원본은 `ArtSource/map/kitchen/`, 정적 시설로 리그/클립 N/A. 현재 코드에는 가동부나 조절 입력이 없다.
물건을 올려두면 실제 데이터 시간에 따라 생것→익음→탐, 집으면 가열 대상에서 빠지는 것을 PIE로 확인한다.
몸체·바닥 접지·HeatZone·상판 지원/실제 요리 상태 검수와 영구 연결/2인/성능은 구분해 기록한다.

실측 납품:120×300×90cm/2,608 tris/1,424 vertices/UV0/3재질/UCX8/UE LOD3.
앞 손잡이1mm 돌출을 최초 임포트 전에 안쪽으로 맞췄다. manifest `handle_fit=true`, 반복 적용 금지.
3조명 렌더와 Native UE 에셋 이미지 확인. FBX 해시는 UE 임포트 결과와 일치한다.

회전 소비 결함: BeginPlay는 세계축 AABB의 XY 반경을 HeatZone에 넣는데, 기본 HeatZone이
부모 Yaw까지 상속해90도 배치에서 열 영역300×120이120×300으로 다시 회전한다.
검수 인스턴스는 HeatZone **Absolute Rotation=true, 세계 회전0, Absolute Scale=true**로 보정한다.
영구 BP 연결 때 이 설정을 함께 적용해야 한다. 현재 원래 BP는 수정하지 않았으므로 원래 소비처의 결함은 남아 있다.

[020 최종 검수](../iterations/020.md): 보정된 실제 Kitchen BP에서 줍기/놓기/가열 중 집기/
다시 놓고 익음→탐을1월드 PIE로 확인했다. 열 영역과 상판 세계 치수 일치, 구체 지지 확인.
영구 연결/기존 배율·위치 보정/2인/LOD·성능은 미완료다.

## 부엌 세트 — 꾸밈·불꽃·가스불 소리 (2026-10-09)

[kitchen-identity](../../goal/kitchen-identity.md) In-scope 1~4의 아트 몫이다. 사용자: "달랑 식탁 같은 거 하나 있어서 요리하는 곳인지
흔한 책상인지 구분이 안 간다." 본체 `SM_Kitchen_A`·`kitchen.blend`는 그대로 두고, 같은 로컬 좌표계(바닥 중앙 원점, +X 정면)로
만든 메시 둘을 `ACL9Kitchen`의 `DressingMesh`·`PilotFlameMesh`(몸체 컴포넌트 자식, 상대 변환 0)에 넣는다. 열 구역·게이지 자리는
몸체 컴포넌트의 바운드에서만 나오므로 꾸밈이 아무리 높아도 판정은 바뀌지 않는다.

원본은 `ArtSource/map/kitchen/dressing/`(`kitchen_dressing.blend`, `export/` FBX 둘·`manifest.json`)이고,
`py -3 Tools/Blender/run_headless.py build_kitchen_dressing.py`가 빈 씬에서 매번 다시 만든다(헤드리스, MCP 없음). 스크립트는 본체를
`kitchen.blend`에서 읽어 좌표계(X ±60, Y ±150, Z 0~90)를 실측으로 확인하고, 아래 판정 부피를 부품마다 AABB로 검사해 어기면 멈춘다.

| 위치 | 부품 (Blender 좌표, cm) |
|---|---|
| 앞 | 상판 립 밑 버너 관(x 55.6~58.6, z 80.3~83.3, 길이 278)과 화구 28구(10 cm 간격, y ±135). 붉은 앞판에 다이얼 둘(y ±56) — 본체의 ±112 다이얼과 같은 모양이라 합쳐 4개 |
| 뒤 | 벽판: 아래 판(바닥~122, 앞면 x −60.2 — 상판 뒤끝과 2 mm라 물건이 틈에 빠지지 않는다), 붉은 경사(122~130), 위 판(130~206, 앞면 x −68 — 도구가 걸리는 홈). 걸이 막대 z 193에 프라이팬(지름 35)·국자·뒤집개 |
| 위 | 후드: 아랫면 201.2(거름망), 몸통 앞면 x −6, 표지 앞면 x −0.5, 높이 ~260, 경사 덮개 ~278, 연통 지름 26·꼭대기 304 |
| 위 | 부엌 표지: 건물 부엌 방 표지판 `SM_Henhouse_Sign_Kitchen_A`의 원본 메시를 읽어 1.25배(75×50)로 후드 앞면에 구워 넣었다 — 같은 아이콘. 중심 z 230.5 |
| 옆 | +Y 끝(UE에서는 −Y 끝) 선반 x −56~+10 · y 152~190 · 판 위 80, 그 위 냄비(지름 29, 붉은 뚜껑, 꼭대기 108.5) |

- **판정 부피(실측)**: 상판 면적(120×300) 안 90~200 cm에 아무것도 없다. 면적 안에서 상판 아래로 가장 높은 것은 불꽃 끝 85.7 cm,
  상판 위로 가장 낮은 것은 후드 거름망 201.2 cm다. 후드(표지 포함) 앞면은 상판 뒤 가장자리에서 59.5 cm(한도 60). 서 있는 눈(160 cm)과
  상판 사이는 비어 있다 — 후드가 눈보다 40 cm 높다.
- **충돌**: UCX 5개 — 벽판 아래 상자·붉은 경사·위 상자, 옆 선반(바닥~80), 냄비. 열 구역(상판 면적 × 90~150)과 겹치는 것은 없다.
  화구·다이얼·걸린 도구·후드·연통·표지는 UCX가 없다. UE 충돌 복잡도는 Simple And Complex(프로젝트 기본) — "복잡을 단순으로"면 후드까지 막는다.
- **재질 슬롯**: `M_CL9_Kitchen_Steel`·`Enamel`·`Red`(상수색, 본체와 같은 재질)과 `M_CL9_SignKitchen_Face`(정점색 — 표지 파란 판·흰 그림).
  표지 테두리는 `M_CL9_SignKitchen_Frame` 대신 조리대 Steel로 물렸다(색이 거의 같고 슬롯이 하나 준다). 새 재질은 불꽃 하나다.
- **걸린 도구·냄비의 색**: 들 수 있는 아이템 `SM_FryingPan_A`(검은 무쇠)·`SM_Pot_A`(크림·붉은 띠, 뚜껑 없음)와 헷갈리지 않게 강철 몸 +
  붉은 손잡이·뚜껑으로 조리대 색에 맞췄다. 꾸밈은 줍기 대상이 아니다.
- **실측 납품**: `SM_Kitchen_Dressing_A` 133.5×342.6×304 cm(x −74~+59.5, y −152.6~+190, z 0~304) / 2,760 tris / 1,550 vertices / UV0 / 4슬롯 / UCX 5.
  `SM_Kitchen_PilotFlame_A` 5.0×271.5×4.1 cm / 280 tris / 1슬롯 / 충돌 없음.
- **불꽃**: 화구마다 길이 6 cm·3각 뿔 하나(28개), 앞으로 40° 기울어 립 밑에서 빠져나온다. 정점색 밑동 하늘 → 중간 파랑 → 끝 남색,
  정점 알파가 흔들림 가중(밑동 0 → 끝 1). UE 재질 `/Game/Items/Kitchen/M_CL9_Kitchen_PilotFlame`(Unlit·불투명) — 발광 = 정점색 ×
  `FlameStrength`(기본 3) × 깜빡임(0.6~1.0, 월드 위치로 화구마다 위상이 다르다), 월드 포지션 오프셋 최대 0.5 cm.
  캠프파이어 `M_CL9_Campfire_Flame`과 같은 방식이고 광원은 없다.
- **가스불·환풍기 루프**: `Tools/Unreal/synth_kitchen_sfx.py` → `ArtSource/audio/sfx/kitchen_ambience.wav`(44.1 kHz·16-bit 모노, 6.0 s,
  피크 −18 dBFS, RMS −31 dBFS). 잡음을 세 주기 필터링해 셋째 주기만 쓰고 순음은 6 s에 정수 번 돌아 페이드 없이 이어진다.
  애셋 `/Game/Sounds/SFX/Kitchen/S_KitchenAmbience`(Looping, `SC_Ambience` — 설정 "배경음" 슬라이더, 2026-10-09 사용자 결정, 가상화 Restart) · 감쇠 `/Game/Sounds/Master/SA_CL9_Kitchen`
  (구, 안쪽 200 cm, 감쇠 거리 1000 cm, Natural Sound −60 dB, 밖은 무음) · 동시 발성 `/Game/Sounds/Master/SCC_KitchenAmbience`
  (최대 1, Stop Farthest Then Oldest, 밀려날 때 0.25 s 페이드) · `SoundTable` 행 `KitchenAmbience`. 동시 발성에 밀린 루프는 가상 루프로
  남았다가 가장 가까운 것이 되면 다시 산다(UE 5.8 소스 근거는 임포트 스크립트 머리말).
  `SC_Ambience`의 설계 게인이 0.5라(`SC_Effects` 0.8) 효과음 클래스였을 때보다 작게 들린다 — 합성 피크 −18 dBFS는 그대로 두고, PIE 사람 확인에서 작으면 그때 조정한다.
- **임포트**: `Tools/Unreal/import_kitchen_identity.py` — `assets`(메시·불꽃 재질·소리·SoundTable, 빌드 전 가능) → C++ 빌드 →
  `bp`(BP CDO의 `DressingMesh`·`PilotFlameMesh` 메시만) → `verify`(읽기만). 꾸밈 LOD 3단은 화면 크기 1.0/0.15/0.07로 고정해 10 m에서
  LOD0이 보이게 했다(표지 그림을 줄이지 않는다).
- **미리보기**: `ArtSource/map/kitchen/preview/dressing_front_3m.png`·`dressing_45_3m.png`(눈높이 160 cm, 3 m)·
  `dressing_front_10m.png`·`dressing_night_front_3m.png`(Cycles, 본체와 함께).
- **배치 때 볼 것**: 세트의 키가 304 cm이고 바닥 면적이 133.5×342.6 cm로 본체보다 넓다(뒤로 14 cm, 선반 쪽으로 40 cm). 푸드트럭 안
  (바닥~천장 256 cm)에는 후드·연통이 들어가지 않는다. 스테이지·트럭 배치는 이번 범위 밖이다.
- **미검수**: UE 임포트, BP 연결, `Lvl_Flat` PIE(부엌으로 읽히는지, 열 구역·게이지 전후 값, 불꽃 깜빡임, 소리 거리·동시 발성), 2인, LOD 전환.

## 조리 효과 — 불꽃 키움·상판 열기·아지랑이·김/연기·덩어리·갈변·소리 (2026-10-09)

[cooking-progress-feedback](../../goal/cooking-progress-feedback.md)의 아트·소리 몫이다. 사용자: "익을 때 주변에 열기가 나온다던가 그런 거 없어?" →
"요리하는 거 좀 표현해줘". 판은 어두우므로 **광원 없이** 재질 발광·반투명만으로 어둠 속에서 읽히게 했다. 나이아가라는 쓰지 않는다 —
세운 카드 메시 + 움직이는 재질이고, 값은 전부 코드가 `CustomPrimitiveData`(CPD)로 넣는다. **코드가 쓰는 CPD 파라미터의 재질 기본값은
전부 0(= Idle)** 이다 — 엔진은 한 번도 안 쓴 CPD 칸에 0 을 쓰면 같은 값으로 보고 갱신하지 않아 첫 순간에 기본값이 보이기 때문이다.

| 컴포넌트(코드) | 메시 | 재질 | CPD |
|---|---|---|---|
| `PilotFlameMesh`(있던 것) | `SM_Kitchen_PilotFlame_A`(그대로) | `M_CL9_Kitchen_PilotFlame`(고침) | [0] `FlameBoost` 0~1 |
| `PlateGlowMesh` | `SM_Kitchen_PlateGlow_A` — 120×300 판 하나(2 tris), 상판 위 0.3 cm(z 90.3), 본체 로컬 좌표계 | `M_CL9_Kitchen_PlateGlow` — Unlit·가산·한면 | [0] `PlateHeat` · [1] `HotX` · [2] `HotY` |
| `HeatHazeMesh` | `SM_Cook_HeatHaze_A` — 40×60 cm 카드 셋이 60° 간격으로 교차(6 tris), 바닥 중앙 피벗 | `M_CL9_Cook_HeatHaze` — Unlit·반투명·양면·굴절 2D Offset·Before DOF | [0] `HazeAmount` |
| `SteamMesh` | `SM_Cook_Steam_A` — 아래 20 → 위 50 cm 사다리꼴 카드 셋, 높이 80 cm, 세로 4 마디(24 tris), 바닥 중앙 피벗 | `M_CL9_Cook_Steam` — Unlit·반투명·양면 | [0] `SteamDensity` · [1] `SmokeAmount` |
| `PuffMesh` | `SM_Cook_Steam_A` | `M_CL9_Cook_Puff` — Unlit·반투명·양면 | [0] `PuffAge` · [1] `PuffDarkness` |
| `BurntSmokeMesh`(재료) | `SM_Cook_Steam_A` | `M_CL9_Cook_Steam` | [0] 0.25 · [1] 1 |
| 재료 `MeshComponent` 오버레이 | — | `M_CL9_Cook_Overlay` — Unlit·반투명·한면·Before DOF | [0] `CookedTint` · [1] `CharTint` |

메시는 `/Game/Items/Kitchen/`, 노이즈 텍스처 `T_CL9_Cook_Noise`(256×256 타일링 값 노이즈 — R 부드러운 fBm 4칸·G fBm 8칸·B 뭉게 6칸, 선형)도 같은 곳이다.
카드 둘은 코드가 매 틱 **재료 메시 바운드 윗면 가운데**에 피벗을 놓고 절대 스케일로 둔다(몸체 스케일을 받지 않는다).

- **불꽃 키움**: kitchen-identity 의 그래프(정점색 × `FlameStrength` 3 × 깜빡임, 정점 알파로 가중한 흔들림)를 그대로 다시 짜고 `FlameBoost` 항만 더했다 —
  곱하는 항은 `FlameBoost` 0 에서 1, 더하는 항은 0 이라 **0 이면 예전과 같은 값**이다. 1 이면 정점 알파(밑동 0 → 끝 1)만큼 불꽃 축(수평에서 40° 앞)으로
  6 cm × (`BoostLength` 2.5 − 1 ± 0.25 깜빡임)을 늘이고(키 약 2.5 배), 축에서의 거리를 `BoostWidth` 1.6 배, 흔들림 3 배, 색을 밑동 파랑 → 35 % 지점
  주황빛 노랑(1, .55, .12) → 끝 주황(.9, .22, .03)으로, 밝기를 `FireBrightness` 0.7 배로 바꾼다(노랑이 밝으면 톤매핑에서 하얗게 날아간다). 화구 중심은
  로컬 위치에서 구한다(x 59.4·z 81.8 cm, y 는 10 cm 격자의 5 자리 — `build_kitchen_dressing.py` 기하). `MaxWorldPositionOffsetDisplacement` 12 cm 로
  늘어난 불꽃이 바운드 밖에서 잘리지 않게 했다. **`import_kitchen_identity.py assets` 를 다시 돌리면 CPD 없는 옛 그래프로 돌아간다** — 그 뒤에는
  `import_cooking_feedback.py assets` 를 다시 돌린다.
- **상판 열기**: 픽셀의 로컬 위치와 뜨거운 자리의 거리(cm)로 핵 35 cm(제곱 감쇠)·번짐 90 cm, 상판 전체에 옅게 0.04, 가장자리 6 cm 에서 사라지고,
  노이즈 얼룩(0.8~1.2)이 천천히 흐른다. 뜨거운 자리 = `ObjectLocalBounds` 중심 + (`HotX`, `HotY`) × 반폭 — 코드가 재료 위치를 메시 로컬로 옮겨 바운드
  중심·반폭으로 정규화한 식의 역이다(X = 정면 120 cm 쪽, Y = 300 cm 길이 쪽). 색은 둔한 붉은 주황(.9, .18, .03) → 핵에서 주황(1, .42, .08), 핵 최대 발광 약 1.
  조절: `GlowStrength` 1 · `CoreRadius` 35 · `HaloRadius` 90.
- **아지랑이**: 위로 흐르는 노이즈 두 겹이 화면 2D 오프셋(±0.5 × `HazeStrength` 0.008 — 1080p 에서 약 3 픽셀)으로 뒤를 일렁이게 하고, 같은 마스크로
  아주 옅은 따뜻한 일렁임 색(불투명도 `HazeTint` 0.06)을 함께 낸다. 굴절이 컴파일되지 않으면 임포트 스크립트가 굴절을 뺀 색만의 판으로 다시 짓고
  결과 JSON `haze_mode` 에 적는다. 컴파일된다고 화면에 보이는 것은 아니다 — PIE 에서 확인한다(Substrate 켜짐, 2026-10-09 기준 미확인).
- **김/연기**: 위로 흐르는 노이즈 두 겹(`RiseSpeed` 0.32 높이/초 ≈ 26 cm/s — CPD 로 속도를 바꾸지 않는다, 위상이 튄다), 밀도가 기둥 폭(카드 폭의
  36 → 92 %)과 위쪽 사라짐을 정하고, 노이즈가 구멍을 낸다. 하얀 김은 밑동이 상판 열기에 비친 듯 따뜻하고(높이 35 % 까지), 연기는 회색(.24)에
  불투명도가 높다(`SteamOpacity` 0.55 → `SmokeOpacity` 0.85). 발광 `SteamBrightness` 0.6. 높이 제곱으로 최대 4·3 cm 흔들린다(WPO, 세 카드가 함께).
- **덩어리**: 같은 카드 위에서 둥근 덩어리가 솟으며 부푼다 — 중심 높이 8 → 40 cm, 반지름 8 → 26 cm(나이의 ease-out), 노이즈로 흩어지며 사라진다
  (나이 1 에서 0). 하얀(.85) ~ 검은(.05) 연기, `PuffOpacity` 0.8, `PuffBrightness` 0.6.
- **카드가 판판해 보이지 않게**: 카드 셋이 60° 간격이라 수평 어느 방향에서 봐도 한 장은 정면 30° 안에 있다. 재질이 카드마다 노이즈 위상을 달리하고,
  가장자리·위아래를 부드럽게 지우고, 시선과 나란히 선 카드(|N·V| 0.15 → 0.6)와 카메라 30 cm 안(→ 80 cm)을 지우고, 김·덩어리는 재료·상판을 뚫는
  곳을 깊이 페이드 8 cm 로 녹인다.
- **갈변 오버레이**: 열 위의 재료에 `SetOverlayMaterial` 로 걸린다. CPD 값 자체가 불투명도(코드: 익음 0 → 0.45, 그을음 0 → 0.6)이고, 재료 로컬
  위치의 노이즈로 얼룩진다(0.75~1.25). 재료 재질은 건드리지 않는다.
  - **익음 색(2026-10-09 둘째 판, F3)** = 노릇한 금갈색 색조(.85, .55, .20 을 밝기 1 로 맞춘 비율) × max(그 픽셀의 장면 색(`SceneColor`) 밝기,
    `CookedFloor` 0.15). 밝은 곳(손전등·작업등)에서는 밝기를 그대로 두고 색조만 금갈색으로 바꾸고, 밤에는 장면 밝기가 낮아도 하한 0.15(장면 선형 휘도)로
    칠한다 — 달아오른 상판(핵 발광 휘도 약 0.6)에 비친 따뜻한 빛처럼 보이게 한 것이고 광원은 없다. 그래서 열 위의 재료는 익을수록 노릇하게 **조금 밝아진다**.
    첫 판은 같은 밝기의 0.79 배 갈색으로 다시 칠해, 밤에 원래 어두운 날계란을 더 어두운 덩어리로 만들었다(`Saved/Video/stills/raw_egg_on_heat_dark.jpg`).
    `CookedFloor` 는 재질 스칼라 파라미터라 PIE 에서 보고 고친다(값은 `import_cooking_feedback.py` 의 `OVERLAY_COOKED_FLOOR`).
  - **그을음 색** = 장면 색의 7 %(거의 검정) — 첫 판 그대로. 타는 동안 후라이가 어두워지다가 36 초에 탄 후라이(둘째 판, 숯 albedo .02~.08 —
    [egg_states.md](../item/egg_states.md))로 바뀌어 어둠이 이어진다(첫 판의 탄 후라이는 오히려 밝아지는 쪽으로 튀었다).
- **소리**: `Tools/Unreal/synth_cooking_sfx.py` → `ArtSource/audio/sfx/` (44.1 kHz·16-bit 모노, 순수 파이썬, 고정 시드).

  | 파일 → 애셋 → 행 | 길이 | 성격 | 피크 / RMS |
  |---|---|---|---|
  | `cook_sizzle.wav` → `S_CookSizzle` → `CookSizzle` | 루프 4.0 s | 기름 쉬익(2.2~9 kHz) + 잔 튀김 알갱이(초당 650) + 잔 팝(초당 22) | −9 / −29.8 dBFS |
  | `cook_crackle.wav` → `S_CookCrackle` → `CookCrackle` | 루프 3.0 s | 굵은 탁(초당 14, 딸깍 2~5 개 몰림) + 퍽 팝(초당 3) + 거친 잡음층 | −10 / −32.5 dBFS |
  | `cook_done.wav` → `S_CookDone` → `CookDone` | 0.60 s | 밝은 "치익" — 고역 잡음 6 ms 에 솟아 7 → 3.5 kHz 로 잦아듦 + 앞 0.25 s 튀김 | −6 / −28.8 dBFS |
  | `cook_burnt.wav` → `S_CookBurnt` → `CookBurnt` | 0.55 s | 둔한 "퍽"(110 → 45 Hz 쿵 + 저역 잡음) + 0.05 s 부터 짧은 쉿 | −6 / −25.6 dBFS |

  루프는 원형 시간축 + 세 주기 필터의 셋째 주기라 이음매가 없다(스크립트가 `LOOP OK` 로 검사). 지글은 실효(SC_Effects 0.8)로 배경음(가스불, SC_Ambience
  0.5)보다 5 dB 쯤 위이고 대역도 위라 따로 들린다. 애셋은 `/Game/Sounds/SFX/Kitchen/`, 클래스 `SC_Effects`, 감쇠 `SA_CL9_Kitchen`(재사용, 값 그대로).
  루프 둘은 Looping·가상화 Restart·동시 발성 `SCC_CookSizzle`·`SCC_CookCrackle`(`/Game/Sounds/Master/`, 최대 2, Stop Farthest Then Oldest, 0.25 s),
  원샷 둘은 다른 효과음과 같은 `SCC_Effects4`. 사람이 들어 본 소리가 아니다 — 크기·성격은 PIE 에서 듣고 스크립트 상수만 고친다.
- **원본·임포트**: `py -3 Tools/Blender/run_headless.py build_cooking_fx.py` → `ArtSource/map/kitchen/cooking_fx/{cooking_fx.blend, export/*.fbx,
  export/manifest.json, textures/T_CL9_Cook_Noise.png}`(본체 좌표계·상판 윗면 90 cm 를 실측으로 확인하고 어기면 멈춘다). UE 는
  `Tools/Unreal/import_cooking_feedback.py` — `assets`(텍스처·재질 일곱·메시 다섯·소리·SoundTable 4행·ItemTable 계란 둘, 빌드 전 가능) → C++ 빌드 →
  `bp`(BP 의 메시·OverrideMaterials·`CookOverlayMaterial` 만 — 위치·스케일·보이기는 코드가 정하니 건드리지 않고 전후를 확인) → `verify`(읽기만).
  FX 메시 셋은 충돌 없음·거리장 해상도 0(거리장 그림자·Lumen 소프트웨어 추적에서 가리개가 되지 않게)·LOD 1.
  에디터에 넣기 전에 `py -3 Tools/Unreal/check_import_cooking_feedback.py` 로 먼저 검사한다 — 가짜 `unreal` 모듈로 세 모드를 끝까지 돌리며
  재질 식 클래스·프로퍼티(쓰기 가능 여부 포함)·핀·출력·열거값·라이브러리 함수 이름을 엔진 5.8 헤더와 대조한다(2026-10-09 첫 에디터 실행이
  `Graph.unary` 의 인자 `a` 와 ComponentMask 의 `a=` 충돌로 죽은 뒤 만들었다). 엔진 안의 동작(컴파일 오류·Substrate·BP 컴파일)은 못 본다.
- **미리보기**(Cycles, UE 재질을 흉내 낸 Blender 재질 — 발광이 주변을 비추지 않게 카메라 광선에만 보이게 했다. 모양·밀도만 보고, 아지랑이의 굴절은
  보이지 않는다): `ArtSource/map/kitchen/preview/cooking_fx_steam_night_2m.png`(어둠, 김·열기·센 불)·`cooking_fx_smoke_night_2m.png`(회색 연기)·
  `cooking_fx_steam_45_night.png`(45° — 카드가 판판해 보이지 않는지)·`cooking_fx_steam_neutral_2m.png`(중립광 — 발광 김은 밝은 곳에서 거의 안 보인다)·
  `cooking_fx_flame_close_night.png`(센 불 가까이).
- **미검수**: UE 임포트·컴파일(특히 Substrate 에서 굴절·`SceneColor`·`LocalPosition`·`ObjectLocalBounds`), BP 연결, `Lvl_Flat` PIE(어둠 속 가독성·
  카드 판판함·불꽃 0 일 때 예전과 같은지·소리 크기), 2인. (첫 판은 같은 날 임포트·PIE 됐다 — cooking-progress-feedback Results.)
  갈변 둘째 판(F3)은 UE 재빌드(`assets`) 대기 — 밤에 익는 날계란·고기가 노릇하게 읽히는지, 밝은 곳에서 밝기가 그대로인지, `CookedFloor` 0.15 가
  맞는지는 PIE(pie-scenarios Y15·Y8)로 본다.
