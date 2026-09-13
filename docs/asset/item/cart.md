# 양계장 운반 수레 — 물리 몸체용 (2026-09-13 재제작)

실제 `cart/None`, `BP_CL9Cart`, CartOnly 행을 따른다. cart-physics 실행부터 수레는 **물리 강체**다 — 메시가 루트이고
UCX 심플 콜리전이 몸체, `Radius` 자동 배율은 쓰지 않는다(배율 1). 짐칸에 놓기/던지기로 떨어뜨린 아이템이 물리로
들어가므로 짐칸 안쪽 콜리전은 비어 있어야 한다.
황색 철제 프레임, 바닥 판재 7장, 널 3장짜리 측판, 고무 바퀴 네 개(짐칸 밑), 앞쪽(+X) 밀기 손잡이의 농장용 수레다.

## 치수 (Blender 지면 기준 → 납품 피벗 = 바운드 중심)

| 항목 | 값 | 피벗 기준 |
|---|---|---|
| 바운드 | 167.7 × 97.2 × 103.4 cm | 중심 (0,0,0), 바닥 Z −51.7 |
| 짐칸 바닥 안치수 | 120 × 80 cm | 바닥 면 Z **−6.0** (`deck_height_cm`) |
| 벽(측판) 높이 | 40 cm (짐칸 바닥에서) | 벽 위 Z +34.0 |
| 짐칸 상자 반폭 | **[60, 40, 20]** (`cargo_extent_cm`) | 상자 중심 = 소켓 `Cargo` (**−17.85, 0, 14.0**) |
| 손잡이 가로봉 | 지면 100 cm, 고무 그립 r 3.7 | 소켓 `Grip` (**80.15, 0, 48.0**) |
| 바퀴 | 반경 18.7 cm, 회전축 Y, 4개 | 중심 (−59.85 / 24.15, ±40, −33) — UE 는 Y 부호 반대 |

+X 손잡이 방향 / +Z 위. 손잡이는 짐칸 앞 벽 밖으로 뻗어 개구부를 가리지 않는다.
소켓 좌표는 Blender 값이며 UE 임포트(force_front_x_axis)가 Y 를 뒤집지만 둘 다 Y=0 이라 같다.

## UCX 콜리전 (FBX 동봉, `UCX_SM_Cart_A_00`~`_08`)

| 번호 | 역할 | 모양 |
|---|---|---|
| 00 | 바닥판 | 박스, 윗면 = 짐칸 바닥(Z −6.0), 두께 8 |
| 01·02 | 측벽 ±Y | 박스, 안쪽 면 Y ±40, 두께 6, 높이 40 |
| 03·04 | 앞뒤 벽 ±X | 박스, 안쪽 면 X −17.85±60, 두께 6, 높이 40 |
| 05·06 | 손잡이 아래·위 | 기울어진 슬래브 2개(컨벡스), 가로봉 포함 |
| 07·08 | 차축+바퀴 | 16각 프리즘(폭 97.2, 반경 18.7, 밑점이 지면) — 바닥 이음매에 덜 걸리라고 박스 대신 |

짐칸 안쪽(±60×±40, 바닥 위 0~40)에는 UCX 정점이 없다(`verify_cart_a_fbx.py` 가 검사). 총 9개, 모두 닫힌 컨벡스.

## 소켓

FBX 의 빈 오브젝트 `SOCKET_Grip`·`SOCKET_Cargo`(메시의 자식, 회전 0) → UE 스태틱 메시 소켓 **`Grip`·`Cargo`**.
gameplay 는 소켓 **위치**만 쓴다(짐칸 상자 축 = 메시 축). 임포트 스크립트가 회전을 0 으로 맞춘다.

## 스크립트

- 제작: `py -3 Tools/Blender/run_headless.py build_cart_a.py` → `cart_a.blend`·`export/SM_Cart_A.fbx`·`export/manifest.json`
- 왕복 검증: `py -3 Tools/Blender/run_headless.py verify_cart_a_fbx.py` → `docs/asset/evidence/027-cart-physics-rebuild.json`
- 미리보기: `py -3 Tools/Blender/run_headless.py preview_cart_a.py --blend ArtSource/item/cart_a/cart_a.blend` → `preview/{neutral,night,flashlight}.png`
- UE 임포트(리드, 에디터 파이썬): `Tools/Unreal/import_cart_a.py` — auto_generate_collision 끔, UCX 1:1, 소켓 위치 검사,
  Collision Complexity 가 UseSimpleAsComplex/ComplexAsSimple 이면 UseDefault 로. 증거 `027-cart-physics-imports.json`.
- `run_headless.py` 는 `blender -b [blend] --python <script>` 로 호출한다(`blender -b <script.py>` 는 Blender 가 .py 를 blend 로 열려다 멈춘다).

## manifest 키 (`export/manifest.json` `assets.cart`)

`deck_height_cm`·`cargo_extent_cm`·`cargo_center_cm`·`grip_cm`·`wheel_radius_cm`·`wheel_centers_cm`·`ucx_count`·`ucx_names`·`sockets`·
`wall_height_cm`·`handle_height_cm`·`ground_z_cm`·`cargo_clear_area_cm`·`anchors_cm`. `runtime_radius_cm` 는 뺐다.

## 제작 실측 (2026-09-13)

167.70×97.20×103.40cm, 6,280 tris / 3,376 vertices / 2재질(프레임·고무) / UV0. Blender FBX 왕복: 치수 오차 <0.0001cm, UCX 9 컨벡스·닫힘,
소켓 2개 위치 일치. 중립/야간/손전등 렌더 갱신. **UE 재임포트·LOD·PIE(B4)는 리드가 한다** — 옛 011 검수 결과(자동 배율 0.59, 적재 슬롯 부착)는
이 메시에 더 이상 해당하지 않는다.

근거: [수레 기획](../../plan/data/item/cart.md), [아이템 계약](README.md), [011 검수](../iterations/011.md)(옛 메시).
