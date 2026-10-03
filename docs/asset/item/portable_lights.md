# 손전등 / 헬멧형 헤드램프

상태: 게임 연결 검수됨 — 2026-09-11 `flashlight`·`headlamp` 행 → `SM_Flashlight_A`·`SM_Headlamp_A`(`BP_CL9FlashItem`), PIE 줍기·착용·F 토글·렌즈 앞 광원; 아이콘 09-14, `GripRotation` 031. 정전·2인·LOD 미검수 · 작성일: 2026-09-11 · 변경일: 2026-09-21

실제 item flashlight/None, headlamp/None, Radius12cm. 정면은 광원과 같은 +X다.
손전등은 황색 공업용 몸체/고무 손잡이/넓은 렌즈, 헤드램프는 기획대로 헬멧에 붙은 램프로 만든다.
헬멧 아래는 실제로 열린 구조이며, 테두리/고정 밴드/전면 램프를 큰 형태로 구분한다.

- 중심 피벗, 적용된 회전/배율, Static Mesh. 스위치와 렌즈는 정적이고 리그/클립 N/A.
- 표면 VertexColor 1재질 + 렌즈 1재질. 렌즈 기본 발광0. 실제 빛 토글은 기존 SpotLight가 담당한다.
- 손전등 목표 길이24cm/머리 지름9cm. 헬멧 목표 길이29cm/폭25cm/높이13cm.
  실측/Radius 자동 배율은 manifest와 UE 증거를 우선한다.
- 헤드램프의 현재 부착은 카메라 상대(8,0,10)cm. 머리/카메라 간섭과 타인 시점은 실제 착용 때 검수한다.
- 실제 SpotLight는 메시 원점에 붙는다. 렌즈 원점과의 거리/자가 그림자는 UE에서 확인할 의존성이다.
- 납품: `.blend`, 두 FBX, UV0, 재질 값, 중립/야간/손전등 비교, manifest.
- 원본 경로: `ArtSource/item/portable_lights/`. UE 임포트 뒤 실제 줍기/착용/F/정전·2인 검수 필요.

근거: [헤드램프 기획](../../plan/data/item/headlamp.md), [아이템 계약](README.md).

제작 결과와 근거는 [009](../iterations/009.md). 손전등1,528 tris, 헬멧2,844 tris/각2슬롯.
최종 cm/자동 배율은 해당 기록과 manifest에 있다. 중립/야간/손전등/헬멧 내부 확인.
실제 1월드 줍기/착용/조명 토글 상태와 부착은 통과했다. 화면 간섭/빔/정전/2인/바닥/LOD는 미완료.
검수 뒤 실제 두 행은 원래 메시로 복구했다. 원본을 재생성하지 않는다.

## 아이콘 (2026-09-13)

퀵슬롯 칸용 흰 실루엣 + 알파 PNG 128×128: `icons/T_Icon_flashlight.png`(`SM_Flashlight_A`), `icons/T_Icon_headlamp.png`(`SM_Headlamp_A`).
`Tools/Blender/render_item_icons.py`(Workbench Flat/Single 흰색, 필름 투명, 정면(+X 렌즈)에서 −Y 45°·위 35° 직교 3/4 시점,
긴 변 88%). 헤드램프는 헬멧 돔 위에 램프가 튀어나온 실루엣이다. RGB 전부 1, 알파만 실루엣. UE는
`Tools/Unreal/import_item_icons.py` → `/Game/UI/Icons/T_Icon_flashlight`·`T_Icon_headlamp`(TextureGroup UI·NoMipmaps·sRGB·NeverStream),
`ItemTable.flashlight.Icon`·`ItemTable.headlamp.Icon` 연결은 리드가 표 행으로 적용한다. 증거: `icons/manifest.json`, `evidence/030-item-icons.json`.
