# 프라이팬 / 냄비

상태: 게임 연결 검수됨 — 2026-09-11 `frying_pan` 행 → `SM_FryingPan_A`(PIE 줍기·놓기·던지기), `pot` 행 신설 → `SM_Pot_A`(`cl9.additem pot` PIE); 아이콘 09-14, 그립 행 031. 냄비 장비(상점) 행은 가격 미정으로 없음 · 작성일: 2026-09-11 · 변경일: 2026-09-21

프라이팬은 실제 item `frying_pan/None`, Hand, Radius30cm, 물리 시뮬레이션 비활성 행을 따른다.
현재 조리는 조리대가 담당한다. 냄비는 장비 기획에 있지만 실제 행이 없으므로 신규 게임 기능을 추가하지 않는다.

- 프라이팬: 지름35cm의 얕은 검은 팬, 긴 손잡이와 실제로 뚫린 걸이 구멍. 목표 전체 길이60cm.
- 냄비: 지름36cm/높이26cm의 열린 에나멜 냄비, 양쪽 고리 손잡이. 뚜껑/새 애니메이션은 현재 요구하지 않는다.
- 중심 피벗. 팬 그릇은 +X, 손잡이는 -X, 입구는 +Z. 냄비도 입구 +Z.
- 표면/손잡이 최대2재질, VertexColor/UV0. 실제 두께가 있는 닫힌 메시 표면과 빈 내부를 만든다.
- 정적 소품이므로 리그/클립 N/A. 사용/운반 동작은 캐릭터에서 담당한다.
- 원본 `ArtSource/item/cookware/`, 원본/개별 FBX/manifest/중립·야간·손전등·내부 검수 납품.
- 팬은 UE 실제 행에 임시 연결하여 줍기/놓기/던지기를 확인한다. 자동 배율과 바닥 차이도 기록한다.
  냄비는 크기/재질/LOD의 에셋 검수와 실제 행 미구현을 구분한다.

근거: [장비 기획](../../plan/data/equipment.md), [아이템 계약](README.md).

제작/검수: [010](../iterations/010.md). 팬1,120 tris, 냄비1,804 tris/각2재질.
원본/FBX/중립·야간·손전등과 UE 크기/재질/LOD3 생성 확인.
팬의 실제 1월드 줍기·놓기·던지기 상태/부착 통과. 바닥 간격26.022cm는 미통과로 유지한다.
그립/2인/LOD 시각/성능과 냄비 데이터 연결은 미완료. 팬의 임시 데이터 연결은 원복했다.

## 아이콘 (2026-09-13)

퀵슬롯 칸용 흰 실루엣 + 알파 PNG 128×128: `icons/T_Icon_frying_pan.png`(`SM_FryingPan_A`), `icons/T_Icon_pot.png`(`SM_Pot_A`).
`Tools/Blender/render_item_icons.py`(Workbench Flat/Single 흰색, 필름 투명, 정면에서 −Y 45°·위 35° 직교 3/4 시점 — 팬은
그릇과 손잡이가 대각선으로, 냄비는 양쪽 고리가 보인다, 긴 변 88%). RGB 전부 1, 알파만 실루엣. UE는
`Tools/Unreal/import_item_icons.py` → `/Game/UI/Icons/T_Icon_frying_pan`·`T_Icon_pot`(TextureGroup UI·NoMipmaps·sRGB·NeverStream),
`ItemTable.frying_pan.Icon`·`ItemTable.pot.Icon` 연결은 리드가 표 행으로 적용한다. 증거: `icons/manifest.json`, `evidence/030-item-icons.json`.
