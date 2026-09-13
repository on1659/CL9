# 고기 — 날것 / 익음 / 탐

실제 ItemTable meat/None, cooked, burnt의 3행, Radius=30cm를 따른다.
둥글고 두꺼운 고깃덩이와 짧게 돌출된 두 갈래 뼈로 된 독립 신규 소품이다.
날것은 붉은 살과 넓은 지방 무늬, 익음은 황갈색 표면과 짙은 구이 자국,
탐은 검은 표면과 불규칙한 갈색 균열로 구분한다. 외부 이미지/구매 자산을 사용하지 않는다.

- 중심 피벗, +X 뼈 방향, +Z 위. 3상태의 바운드와 원점을 일치시킨다.
- Static Mesh 상태 교체이므로 리그/클립 N/A. 운반 동작은 캐릭터 담당.
- VertexColor 1슬롯/UV0, 외부 텍스처 없음. 원본과 FBX, 재질 값, 미리보기, manifest를 납품한다.
- 목표 원본 크기는 약 60×34×34cm. 실제 수치와 UE SphereRadius로 계산한 최종 치수는 manifest/임포트 증거에 기록한다.
- 게임 구체 충돌과 메시 하단의 차이는 실제 놓기 검수에서 확인한다. 임포트만으로 통과하지 않는다.
- 다음: Blender 제작/중립·야간·손전등 → UE 임포트/바운드/재질 → 실제 요리 상태·운반/2인 PIE.

제작/첫 검수: [008](../iterations/008.md). 원본 실측67.10×34.60×34.00cm,
각2,992 tris/1재질. UE 실표시 약59.76×30.82×30.28cm, 상태 간 동일 배율.
중립·야간·손전등 확인. 구체 충돌 하단과 메시 하단14.859cm 차이로 바닥 표현 미통과.
임시 데이터 연결은 검수 후 복구했으며, 실제 요리/운반과 2인 PIE는 아직 검수하지 않았다.

## 아이콘 (2026-09-13)

퀵슬롯 칸용 흰 실루엣 + 알파 PNG 128×128: `icons/T_Icon_meat.png`. raw(`SM_Meat_Raw_A`) 하나만 렌더하고
cooked·burnt 행도 같은 텍스처를 쓴다(3상태 바운드·실루엣 동일). `Tools/Blender/render_item_icons.py`(Workbench
Flat/Single 흰색, 필름 투명, 직교 3/4 위 시점, 긴 변 88%). RGB 전부 1, 알파만 실루엣. UE는
`Tools/Unreal/import_item_icons.py` → `/Game/UI/Icons/T_Icon_meat`(TextureGroup UI·NoMipmaps·sRGB·NeverStream),
`ItemTable.meat[.sub].Icon` 연결은 리드가 표 행으로 적용한다. 증거: `icons/manifest.json`, `evidence/030-item-icons.json`.
