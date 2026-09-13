# 삼각 받침 작업등

장비 기획의 `standing_lamp`: 놓으면 주변을 밝히는 운반 장비. 현재 실제 아이템/장비 행은 없다.
양계장 작업용 삼각 받침, 조절식 기둥, 보호봉으로 감싼 확산등과 상단 운반 손잡이로 제작한다.

- 목표 높이180cm/바닥 폭85cm, +X 조작부, 바운드 중심 피벗. 실제 값은 manifest를 따른다.
- 2재질: 버텍스 색 본체와 불투명 확산등. `LightStrength` 파라미터0=꺼짐,4=켜짐 미리보기.
  실제 광원 범위/밝기는 게임 데이터 미정이며 이 미리보기 값을 게임 수치로 확정하지 않는다.
- Static Mesh, 리그/클립 N/A. 펼침/접힘 기능은 기획에 없으므로 추가하지 않는다.
- 발바닥/광원 중심/운반 손잡이를 REF Empty와 manifest 앵커로 제공한다.
- 원본/FBX/UV0/재질 값/꺼짐·켜짐/중립·야간·손전등을 납품하고 UE 크기·재질·LOD 검수.
- `ArtSource/item/standing_lamp_a/`. 실제 소비처가 없어 배율/운반/정전/2인 검수는 미연결로 구분한다.

근거: [장비](../../plan/data/equipment.md), [손전등과 보조 조명](../../plan/data/item/flashlight.md).

제작 결과: 86.95×75.60×179.60cm, 1,552 tris/836 vertices, UV0/2재질.
원본·FBX·앵커·4렌더와 UE 메시/Off·On 인스턴스 저장. [012 검수](../iterations/012.md).
실제 장비 연결/운반/광원/충돌/2인/LOD 시각·성능은 미완료다.

## 아이콘 (2026-09-13)

퀵슬롯 칸용 흰 실루엣 + 알파 PNG 128×128: `icons/T_Icon_standing_lamp.png`(`SM_StandingLamp_A`). `Tools/Blender/render_item_icons.py`
(Workbench Flat/Single 흰색, 필름 투명, 정면에서 −Y 45°·위 35° 직교 3/4 시점, 긴 변 88% — 키가 커서 세로로 꽉 차고 삼각
받침이 아래에 보인다). RGB 전부 1, 알파만 실루엣. UE는 `Tools/Unreal/import_item_icons.py` → `/Game/UI/Icons/T_Icon_standing_lamp`
(TextureGroup UI·NoMipmaps·sRGB·NeverStream), `ItemTable.standing_lamp.Icon` 연결은 리드가 표 행으로 적용한다.
증거: `icons/manifest.json`, `evidence/030-item-icons.json`.
