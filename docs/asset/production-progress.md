# 전체 에셋 제작 진행

실행 지침: [asset-production-loop](../goal/asset-production-loop.md)

## 현재 재개 지점

- **[별도 세션 2026-09-11 저녁] CL9Art 미사용 에셋 일괄 연결 완료 — 되돌리지 말 것.** `docs/goal/wire-cl9art-assets.md`.
  맵 조각(벽 500/125·울타리·문짝·바닥·천장·마당·디딤판)은 `PieceTable`/`SegKindTable`에, 소품(둥지·상자·포대·
  여물통·작업등·작업대·표식 4·보·기둥)은 `ModuleTable.PropSets`에, 시설 5종은 BP 메시 + 두 레벨 인스턴스에,
  푸드트럭은 두 레벨의 `FoodTruck_Body/Steps` 액터(임시 위치)에, 농부 A는 `farmer_a` 행에, 냄비·작업등은 새 아이템
  행에 연결했다. catalog.md `UE 연결` 컬럼과 decisions.md 2026-09-11 항목이 정본. 남은 미연결: `SM_Henhouse_Wall_250_A`
  (소비처 없음), Stretch100 변형 3종(불필요), 테스트 에셋.

- **[별도 세션 2026-09-11] 아이템 7행 UE 영구 연결 완료 — 되돌리지 말 것.** `ItemTable`의 meat(raw/cooked/burnt)·
  cart·flashlight·headlamp·frying_pan 행 `Mesh`를 `CL9Art/Items/…` 메시로 바꿨다(자동 배율: 고기 0.89 → 60×31×30cm,
  수레 0.60 → 91×69×63cm, 손전등 0.88, 헤드램프 0.75, 프라이팬 0.99 → 60×35×8cm). Lvl_Flat 아이템 줄 PIE로 확인.
  egg_raw는 이미 연결돼 있었고 나머지 egg 상태·pot·standing_lamp는 메시/행이 없어 그대로다.
- **[별도 세션 2026-09-11] 동물 5종 UE 영구 연결 완료 — 재개 시 되돌리거나 다시 만들지 말 것.**
  Claude 세션이 `MonsterTable`의 chicken/goose/raccoon/pig/sheep 행을 `CL9Art/Monsters/<종>A/`의
  SK·스켈레톤과 신규 `Content/Monsters/ABP_<종>_A`(ABP_Chicken 복제 → 타깃 스켈레톤 교체 → 시퀀스
  플레이어 8개 교체, 기준 속도 = manifest `reference_speed_cm_s`)로 연결하고 `MeshYawOffset`을 0으로 바꿨다.
  Lvl_Flat PIE에서 5종 스폰·접지·걷기, 닭 HitReact/Death 알파, 양 추격→잡기 확인. 기록: `docs/decisions.md`
  2026-09-11, `docs/systems/animation.md` 보유 에셋 표. 이 행들의 남은 검수는 2인 PIE·LOD·Investigate LookAround
  연결뿐이며 "임시 검수 후 복구" 절차는 더 이상 적용하지 않는다. 참고: 09-10 Dreamscape 팩 축소로 팩 애님
  시퀀스가 디스크에서 사라져 옛 ABP_Chicken/Pig/Sheep은 빈 플레이어 상태다(참조 없음, 삭제 보류).

- 진행 중 [024](iterations/024.md): 농부 A 신규 몸4100tris/팔1496tris·기존 Manny89뼈, FBX2개/UV0+UV1.
  `ArtSource/character/farmer_a/farmer_a.blend`, UE `CL9Art/Characters/FarmerA`. 전체45필수 행 최종 완료0개 유지.
  원본 m 단위, cm export 복사본/root 이름 유지로 UE root scale1. 모든 뼈 참조/기존 Walk 비교 통과.
  1월드 실제 CL9 pawn 몸/팔·Unarmed/FP Copy AnimBP·이동 통과. 신규 상태 동작은 아직 없다.
  실제 렌더에서 VC가 하얗게 보여 공유18색256×8 팔레트/UV1로 변경, 최종 실제 UE 색 확인.
  legacy FBX+원래 노멀 임포트, 기존 Skeleton 참조 해시 불변/Interchange cvar1 복구.
  build→fit_farmer_a_silhouette→bake_farmer_palette→export_farmer_a 순서. 기존 fit/bake 재실행 금지.
  다음 Carry/Use/피격 등 클립·1인칭 손/아이템/FOV 검수. 어깨/고관절 전체 운반 범위·슬롯/2인/LOD·영구 연결 전.
  farmer_b 몸/팔도 미제작. `_reference`의 Epic 메시/Walk는 참고본이며 신규 모델 완료로 세지 않는다.
  검수4액터 정리/원래29액터 보존, UE Lvl_Flat/PIE 종료, Blender CL9_FARMER_A. 실행 핸들 없음.

- 최신 [023](iterations/023.md): 푸드트럭 본체/별도 계단 제작·UE 개별 파츠 검수.
  `ArtSource/map/food_truck/`, 본체5128tris/630×353.5×325.1cm/UCX24, 계단224tris/120×114×64cm/UCX4.
  각 UV0/3렌더/LOD3, 본체4·계단2상수 재질. 후면 조리대/계단 호환 화면은 복사본 검수다.
  뒤 차축42cm/조리대 앵커30cm 이동으로 덮개·조리대6cm 간격 확보. 각 fit/cleanup은 manifest guard 적용, 재실행 금지.
  1월드 캡슐 출입/통로/벽·천장/4단 지지 통과. 실제 CharacterMovement 입력으로 상승→통로→하강/지면 복귀.
  검수 뒤 비충돌 램프/그릴 중복 면만 수정, 최종 FBX와 UE 해시/바운드/UCX/UV/manifold 일치.
  영구 연결/게임 조명·시점/운반/2인/LOD·성능 미완료. 전체45필수 행 최종 완료0개 유지.
  검수4액터 정리/원래29액터 보존, UE Lvl_Flat/PIE 종료, Blender CL9_FOOD_TRUCK. 실행 핸들 없음.
  다음 farmer_a 몸/팔: 현재 Manny/FPP 리그와 데이터 참조를 추출→원본 호환 확인→외형 제작. 전체 맵 배치 제외.

- 최신 [022](iterations/022.md): 트럭 패드/패널 원본·FBX2개/각3렌더/UV0/각3재질/UCX1/UE LOD3.
  `ArtSource/map/truck_parts/`, UE `CL9Art/Maps/TruckParts`, 패드784tris/400×400×20cm, 패널868tris/15×80×120cm.
  1월드 실제 BP 회복 밖 미회복/이탈 초기화/5초 후 완전 회복, 패널 RPC→로컬 Shop 열림/닫힘 확인.
  UIManager 조회는 ObjectIterator에서 PIE GameInstance 소유자를 선택한다. 미지원 라이브러리 경로 재사용 금지.
  영구 연결/기존 배율·피벗 보정/경사·경계 이동/2인/LOD·성능 미완료. 게임 구매 없음.
  검수4액터 정리/원래29액터 보존, UE Lvl_Flat/PIE 종료, Blender `CL9_TRUCK_PANEL`. 실행 핸들 없음.
  다음은 푸드트럭 외형 모델 제작. 전체 맵 배치/이동 차량 기능 제외, 모델 우선 유지.

- 최신 [021](iterations/021.md): 납품 창구 원본/FBX/3렌더/UV0/3재질/UCX6/UE LOD3.
  `ArtSource/map/delivery_counter/`, UE `CL9Art/Maps/DeliveryCounter`, 1,552 tris/100cm 큐브/바닥 피벗.
  실제 Counter BP1월드 PIE 계란 줍기→납품→빈손 판매/수입 증가·중복 없음, 몸통·상판 충돌 확인.
  Stock protected 조회 대신 공개 포커스·프롬프트/액터 제거/자금 결과로 검수. 내부 배열/주문·수레 미검수.
  영구 연결/기존 인스턴스 피벗/2인/LOD·성능 미완료. 원래 BP/레벨/데이터 변경 없음.
  검수4액터 정리/원래29액터 보존, UE Lvl_Flat/PIE 종료, Blender `CL9_DELIVERY_COUNTER`. 실행 핸들 없음.
  다음은 트럭 패드/조작 패널 소비 BP 확인→개별 파츠 제작. 전체 맵 배치 제외/모델 우선 유지.

- 최신 [020](iterations/020.md): 조리대 원본/FBX/3렌더/UV0/3재질/UCX8/UE LOD3.
  `ArtSource/map/kitchen/`, UE `CL9Art/Maps/Kitchen`, 2,608 tris/120×300×90cm/+X 정면/바닥 피벗.
  `handle_fit=true` 반복 적용 금지. 실제 배치 Yaw90/배율1에서 기존300×120 작업 면적 복원.
  HeatZone이 세계 AABB를 다시 회전하는 결함 발견: 검수 인스턴스 Absolute Rotation=true/세계 회전0 보정.
  최종1월드 실제 Kitchen BP의 줍기/놓기/가열 중 제거/다시 놓고 익음→탐과 상판/열 영역·구체 지지 통과.
  원래 BP 수정 없음: 영구 연결 때 HeatZone 회전/기존 배율·피벗 보정 필요. 2인/LOD·성능 미완료.
  검수4액터 정리/원래29액터 보존, UE Lvl_Flat/PIE 종료, Blender `CL9_KITCHEN`. 실행 핸들 없음.
  다음은 납품 창구 BP 치수 조회→개별 모델 제작. 전체 맵 배치 제외/모델 우선 유지.

- 최신 [019](iterations/019.md): 반납 항아리 원본/FBX/3렌더/UV0/3재질/UCX17/UE LOD3.
  `ArtSource/map/return_jar/`, UE `CL9Art/Maps/ReturnJar`, 1,980 tris/높이100cm/바닥 피벗.
  1월드 실제 Jar BP에서 계란 줍기→반납/기록 증가·제거, 입구 비차단/몸통 차단/구체 내부 지지 확인.
  검수 도구 미지원 호출 수정 후 같은 PIE에서 마무리. 최종 FBX/UE 해시 일치.
  영구 연결/기존 원통 인스턴스 위치 보정/수레/2인/LOD·성능 미완료.
  검수4액터 정리/원래29액터 보존, UE Lvl_Flat/PIE 종료, Blender `CL9_RETURN_JAR`.
  실행 핸들 없음. 다음은 조리대 BP 치수 조회→상판과 HeatZone 바운드 계약에 맞춰 제작.

- 최신 [018](iterations/018.md): 마당 바닥/표식4종, 원본5개/FBX6개/각3렌더/UE LOD3.
  `ArtSource/map/henhouse_yard/`, UE `CL9Art/Maps/HenhouseYard`. 바닥1,180 tris/UCX1.
  1월드 PIE 바닥 이음매<0.0001cm/구체 정착/표식4종 비충돌 확인. 흙면·충돌면 차이0.5cm.
  표식 사료/조리 `icon_layer_fit=true` 재실행 금지. 실제 역할/색/2인/LOD 검수 미완료.
  검수7액터 정리, UE Lvl_Flat/PIE 종료, Blender `CL9_WAYFINDING_EXIT`. 실행 핸들 없음.
  다음은 반납 항아리 소비 BP 확인 → 개별 모델 제작. 전체 맵 조립 제외 유지.

- 최신 [017](iterations/017.md): 짧은 벽250/125·디딤판3종, FBX5개/3렌더/UV0/UCX1/UE LOD3.
  `ArtSource/map/henhouse_access_parts/`, UE `CL9Art/Maps/HenhouseAccess`, 792/528/220 tris.
  1월드 PIE에서 기존500cm벽과 이음매<0.000004cm, 디딤판 방향별 복원/충돌 확인.
  벽 `intersection_fit/top_fit=true`, 디딤판 `marker_fit=true` 재실행 금지. 원본/UE 최신 해시 일치.
  실제 역할 선택/걷기·운반/2인/LOD 미완료. 검수6액터 정리, 데이터 변경 없음.
  UE Lvl_Flat/PIE 종료, Blender `CL9_HENHOUSE_TREAD`, 실행 핸들 없음. MCP 후속 조회 성공.
  다음은 마당 바닥/방·출구 표식. 전체 맵 조립 제외/모델 제작 우선 유지.

- 최신 [016](iterations/016.md): 보/기둥/천장3종 + Stretch100 천장 FBX 변형, 원본/3렌더/UV0/UCX1/UE LOD3.
  `ArtSource/map/henhouse_structure/`, UE `CL9Art/Maps/HenhouseStructure`, 508/596/792 tris.
  1월드 PIE에서 보 이음매0, 천장 복원500×500×30cm/이음매<0.0001cm, 회전·충돌·아래 공간 확인.
  실제 역할 연결/2인/LOD 시각·성능 미완료. 검수6액터 정리/원래 목록 확인, 데이터 변경 없음.
  UE Lvl_Flat/PIE 종료, Blender `CL9_HENHOUSE_CEILING`, 실행 핸들 없음.
  다음은 짧은 벽/계단 디딤판/마당 바닥 소비 계약 확인 → 개별 파츠 제작. 전체 맵 배치 제외 유지.

- 최신 [015](iterations/015.md): 작업대 원본/FBX/3렌더/UV0/2재질/UCX12/UE LOD3.
  `ArtSource/map/henhouse_workbench/`, UE `CL9Art/Maps/HenhouseWorkbench`, 180×75×90cm/1,212 tris.
  1월드 PIE에서 검수 구체가 상판에 정착, 몸체/선반 차단 및 중앙 빈 공간 비차단 확인.
  실제 CL9 아이템/모듈 연결/2인/LOD 시각·성능 미완료. 검수3액터 정리/원래 목록 확인.
  UE Lvl_Flat/PIE 종료, Blender `CL9_HENHOUSE_WORKBENCH`, 실행 핸들 없음.
  다음은 보/기둥·천장 재사용 구조 파츠. 전체 맵 배치 제외/모델 제작 우선 유지.

- 최신 [014](iterations/014.md): 문짝 원본/FBX/3렌더/UV0/2재질/UCX1/UE LOD3.
  `ArtSource/map/henhouse_door/`, UE `CL9Art/Maps/HenhouseDoor`, 131.9873×16×256.4243cm/1,120 tris.
  힌지 왼쪽 아래 피벗, 1월드 PIE 닫힘/수동90도 참고 자세에서 캡슐 차단·통과 검사.
  현재 FlapIn 회전 동작 없음/높이630cm 비균일 배율 문제로 실제 연결·애니메이션 표현 미완료.
  검수2액터 정리, UE Lvl_Flat/PIE 종료. Blender `CL9_HENHOUSE_DOOR`, 실행 핸들 없음.
  다음은 양계장 작업대 소품. 전체 맵 배치 제외/모델 제작 우선 유지.

- 최신 [013](iterations/013.md): 울타리 개별 파츠 원본/FBX/3렌더/UV0/2재질/UCX1/UE LOD3.
  `ArtSource/map/henhouse_fence/`, UE `CL9Art/Maps/HenhouseFence`, 250×24×288cm/1,128 tris.
  1월드 PIE에서 두 장500cm/틈0cm, 90·270도 회전과 line/capsule 충돌 검사 통과.
  실제 걷기/운반/2인/LOD 시각·성능/테마 연결 미완료. 검수4액터 제거 후 원래 목록 확인.
  UE Lvl_Flat/PIE 종료, Blender `CL9_HENHOUSE_FENCE`, 실행 핸들 없음.
  다음은 양계장 문짝의 실제 폭/힌지 계약 확인 → 개별 메시 제작. 전체 맵 배치 제외 유지.

- 최신 [012](iterations/012.md): 스탠드 작업등 원본/FBX/앵커/UV0/2재질/4렌더와 UE 임포트/LOD3.
  `ArtSource/item/standing_lamp_a/`, UE `CL9Art/Items/StandingLampA`, 1,552 tris.
  Off/On 재질 파라미터0/4 확인. 실제 데이터 행이 없어 운반/조명/충돌/2인/성능은 미완료.
  Blender `CL9_STANDING_LAMP_A`, 발광0 복구, 실행 핸들 없음. 다음은 양계장 울타리 파츠.

- 최신 [011](iterations/011.md): 수레 원본/FBX/UV0/2재질/앵커와 최종3렌더, UE 임포트/LOD3.
  `ArtSource/item/cart_a/`, UE `CL9Art/Items/CartA`, 5,532 tris.
  1월드 적재/꺼내기 성공. 초기 지면 침투를 PIE에서 보정한 뒤 견인 연결·해제/수평 응답 확인.
  적재 바닥65.948cm 차이, 구체·메시 하단28.651cm 차이, 견인 중 Z 상승은 미통과.
  테스트 cart 행은 원복, PIE 종료. Blender `CL9_CART_A`, 실행 핸들 없음. 원본을 재생성하지 않는다.
- 다음 제작: standing_lamp 모델. 전체 맵 조립 제외/모델 제작 우선, 위 UE 의존성은 별도 미완료 유지.

- 최신 [010](iterations/010.md): 프라이팬/냄비 신규 원본/FBX/UV0/2재질/최종 3렌더, UE 임포트/LOD3.
  `ArtSource/item/cookware/`, UE `CL9Art/Items/Cookware`, 1,120/1,804 tris.
  실제 1월드 팬 줍기→놓기→재획득→던지기 상태/부착 통과. 바닥 표현 간격26.022cm 미통과.
  냄비는 실제 행/기능 미구현. 그립/2인/LOD 시각·성능은 미완료. 테스트 팬 행은 원복, PIE 종료.
  Blender `CL9_COOKWARE`, 실행 핸들 없음. `handle_fit_revision=1`, `inner_wall_fit=true` 재실행 금지.
- 다음 제작: 수레 모델. 정적 소품 비교는 `preview_static_pair.py`, 임포트/임시 행 연결은 매개변수 공용화.

- 최신 [009](iterations/009.md): 손전등/헬멧형 헤드램프 원본/FBX/2재질/UV0/미리보기와 UE 임포트.
  `ArtSource/item/portable_lights/`, UE `CL9Art/Items/PortableLights`. 1,528/2,844 tris, UE LOD3 생성.
  헬멧 곡면 장식·안쪽 브래킷 수정 후 최종 4렌더 직접 확인. `trim_fit_revision=2`를 다시 실행하지 않는다.
  실제 1월드 Lvl_Flat 줍기/착용/F 우선순위·토글 상태/부착 통과. 빔/시점 간섭/정전/2인/바닥/LOD는 미완료.
  테스트용 두 행 복구, PIE 종료. Blender `CL9_PORTABLE_LIGHTS`, 실행 핸들 없음.
- 다음 제작: 프라이팬/냄비 모델. 헬멧과 손전등은 재생성하지 않고 남은 UE 게이트를 이어간다.

- 최신 [008](iterations/008.md): 고기 3상태 원본/FBX/중립·야간·손전등 제작 및 UE 임포트/LOD3 생성.
  `ArtSource/item/meat_states/`, UE `CL9Art/Items/MeatStates`. 각2,992 tris/1재질, 3상태 바운드 동일.
  1월드 Lvl_Flat 실제 메시/배율 확인. 구체 충돌 하단과 메시 하단14.859cm 차이로 바닥 QA 미통과.
  테스트용 meat 행 변경은 원래 메시로 복구. PIE 종료, 맵 저장/조립 변경 없음. 실행 핸들 없음.
- [007](iterations/007.md): 양의 중립·야간·주요 자세 렌더 직접 확인까지 갱신.
  다음 제작: 손전등/헤드램프 모델. 아이템 바닥 소비 경로, 실제 운반·요리/2인 PIE는 미완료로 유지.
  Blender 현재 `CL9_MEAT_KIT`. 양/고기 원본을 재생성하지 않는다.

- 최신 체크포인트: 양 원본/24뼈/8클립과 UE 임포트 증거가 저장되어 있다.
  `ArtSource/monster/sheep_a/`, `evidence/007-sheep-imports.json`, `evidence/007-sheep-pose-comparison.json`.
  중립 렌더 직접 확인. 야간/달리기/잡기/사망 렌더는 저장되어 있으나 이번 재개에서 직접 확인하지 않았다.
  실제 AnimBP/2인 PIE·LOD/영상 검수는 미완료. 양을 재생성하지 말고 남은 검수와 카탈로그 갱신부터 재개한다.
- 사용자 토큰 절약 지시: 전체 맵 배치 제외를 유지하고 재사용 파츠 제작과 최소 UE 호환 검수에 집중한다.
  아래 '현재/다음'은 각 반복 당시 이력이며 이 체크포인트가 우선한다.

- 최신 [006](iterations/006.md): 돼지 모델/24뼈/8클립 제작과 UE 임포트 확인.
  `ArtSource/monster/pig_a/`, UE `CL9Art/Monsters/PigA`. 8,404 tris/1재질.
  중립·야간·주요 자세, Blender 바닥/루프/접지, UE 크기·길이·발 위치 검수.
- Blender 현재 `CL9_PIG_A`, Idle 프레임1. 실행 핸들 없음. 다음은 양 신규 모델/리그/클립.
  네 종 실제 AnimBP/2인 PIE·LOD/영상/손전등의 미완료 게이트는 유지한다.
- 네 발 애니메이션/감사 솔버는 종 이름을 매개변수로 받도록 공용화. 기본은 기존 Raccoon,
  Pig wrapper는 자기 원본/Action만 처리한다. 기존 너구리 원본/FBX는 다시 만들지 않았다.

- 최신 [005](iterations/005.md): 거위 원본/메시/20뼈 리그/10클립과 UE 임포트 완료.
  `ArtSource/monster/goose_a/`, UE `CL9Art/Monsters/GooseA`. 9,516 tris, 1재질.
  수면 눈 감기 수정, Blender 바닥/루프/지지 속도 감사 및 UE 전 프레임 발 위치 <0.000055cm 확인.
- Blender 현재 `CL9_GOOSE_A`, Idle 1프레임. 실행 중 렌더/임포트 핸들 없음.
  원본/Action은 있으므로 재생성하지 않는다. 거위 AlertWindup만 60fps/0.55초임에 주의.
- 다음 제작: 돼지 모델/리그/클립. 거위 이동 영상의 낮은 몸통/다리 가림 확인과 손전등,
  세 종 실제 AnimBP/2인 PIE·LOD는 미완료로 유지한다. 임포트만으로 최종 완료 처리하지 않는다.

- 최신 반복 [004](iterations/004.md): 닭·너구리 메시 2개와 각 8클립 UE 임포트 완료.
  `CL9Art/Monsters/ChickenA`, `RaccoonA`. 크기/재생 길이/스켈레톤/색 입력 확인.
  전 프레임 발 위치 차이 닭 <0.000046cm, 너구리 <0.007582cm. 실제 AnimBP/2인 PIE는 미완료.
- 다음 제작: 거위 전용 모델/리그·울음/수면 포함 클립. 모델 제작 우선 지시를 유지한다.
  이미 확인한 임포트를 반복하지 않는다. 원본 변경 시 해시 비교 후 재임포트한다.
- 현재 UE `Lvl_Flat`, PIE false. QA 임시 닭 액터는 제거하고 원래 레벨로 복귀. 실행 핸들 없음.
- 사용자 최종 범위: 전체 맵 배치·조립 제외. 재사용 파츠와 최소 UE 연결·충돌 검수 유지.
  필수 제작 45개/완료 0개, 별도 맵 작업 참고 17개. 아래 임포트 미실행 기록은 과거 이력이다.

- 2026-09-11 최신 검수: 너구리 신규 모델/24뼈 리그와 8개 Action/개별 FBX 저장. 귀 겹침을 정리하여 **14,776 tris / 7,534 vertices / 1재질 / 최대 2개 비영 가중치**. 원본 `ArtSource/monster/raccoon_a/raccoon_a.blend`.
- `audit_raccoon_a.py`의 전 프레임 실제 변형 결과: 8클립 바닥 관통 없음(min Z 약 0.70cm), root 변위 0, 4개 루프 경계 오차 0, 지지 발 속도 오차 <0.0004cm/s. `export/blender_animation_qa.json`에 샘플/FBX 해시 저장.
- 중립·야간 및 걷기/달리기/잡기/사망/각성/둘러보기 렌더 직접 확인. 동영상·손전등 검수 및 UE 리그/클립 왕복은 아직 미완료. 렌더 실행 핸들 없음.
- 다음 행동: 너구리 이동 동영상/손전등 검수 → 닭 대표 스켈레탈 FBX의 UE 왕복으로 축·크기·리그 계약 확인. 기존 씬/Action이 있으므로 재생성하지 않는다.
- 최신 사용자 선호: **UE 테스트는 Unreal MCP를 사용**. Native `list_toolsets`, `describe_toolset`, `IsPIERunning`, `get_current_level` 호출 성공. 마지막 실제 상태 `/Game/Maps/Lvl_Flat`, PIE false, UE 5.8.1, 미저장 맵/콘텐츠 0. AssetTools에는 FBX 임포트 도구가 없으므로 임포트만 기존 에디터 Python 경로가 필요하며, 검수에는 MCP를 사용한다. 이번 턴에는 임포트/맵 수정하지 않았다.
- 사용자 역할 정정: **맵에 들어가는 개별 파츠 제작이 이 작업의 역할이고, 파츠 배치·맵 조립은 언리얼에서 한다.** Blender 조립 샘플은 파츠 호환/분위기 검토용이다. 맵 생성기나 방 배치 설계 작업으로 범위를 넓히지 않는다. 파츠의 UE 사용 가능성 검수는 유지한다.

- 최신 사용자 지시: **모델 제작 우선**. UE 입력 문제에 매달리지 말고 전체 종/아이템 모델·리그·클립을 진행한다. 최종 UE 검수 기준은 유지한다.
- 현재 반복 [002](iterations/002.md): 계란 5상태 모델/FBX/중립·야간 미리보기 제작. 닭 전용 모델(10,196 tris, 17뼈, 1재질)과 8클립 FBX 제작. 닭 잡기/사망 바닥 관통 및 뜬 죽음 자세 수정, Blender 프레임 전수 바닥/루트/루프/지지 속도 감사 통과. UE 검수는 별도 미완료.
- Blender 활성 씬 `CL9_RACCOON_A`. 개별 원본은 각 `ArtSource` 에셋 폴더에 libraries.write로 저장한다. 기존 양계장 원본을 덮어쓰지 않는다.
- 닭 감사/수정 후 렌더 확인 완료. 걷기 24프레임/달리기 15프레임 512×512 MP4 저장·디코더 메타데이터 확인. 실행 중 렌더/테스트 핸들 없음.
- 다음 제작 행동: 너구리 전용 모델/리그/클립으로 진행. UE를 사용자가 플레이 중이면 독립 모델 제작을 우선하고, 가능한 시점에 닭/계란 FBX의 축·크기·리그 왕복을 검수한다. 새 형태를 대량 생산하기 전에 이 미완료 UE 게이트를 처리한다.
- 사용자 UE 단서: Lvl_Flat 첫 실행 조작 불가, F8 탈출/복귀 후 정상. 입력 포커스/possess 경로 후보로 기록했으며 원인 확정/코드 수정은 하지 않았다.
- 증거 주의: `001-pie-initial.json`은 재개 조회로 빈 worlds가 저장되어 초기 증거로 사용할 수 없다. 앞으로 조회 출력은 `Saved/asset-pie-current.json`, 실제 테스트 결과는 시각이 포함된 evidence 파일로 분리한다.

### 이전 샘플 왕복 이력

- 2026-09-11: 전체 제작 goal **active**. 이전 응답의 세션 안내는 제작 진전이 없었으며, 이번 실행에서 실제 파일/프로세스/MCP 상태를 재검증하고 제작·검수를 재개했다.
- 2026-09-10: 사용자 요청으로 분류별 UE5 PIE 테스트·2인 검수·실패 수정/재임포트 절차를 필수 게이트로 구체화했다. 실제 UE 테스트는 아직 실행하지 않았다.
- 기존 작업: [양계장 첫 샘플](map/henhouse_sample_01.md), 8개 FBX와 원본/미리보기 존재.
- 최초 [catalog](catalog.md): **62개 작업 단위 / 완료 0개**. 실제 데이터 행 + 명시된 기획 + 조립 검수 단위. 누락 발견 시 추가한다.
- 이전 반복: [001](iterations/001.md), 양계장 샘플의 첫 UE 왕복. 샘플 8종과 1m 기준 큐브 임포트 완료, 실사용 검수 미완료.
- UE 5.8.1 실제 실행 확인. `/Game/Maps/Lvl_Flat`, PIE 중 아님, 에디터 미저장 맵/콘텐츠 0개 확인 후 시작.
- UE MCP `http://127.0.0.1:8000/mcp` 응답 확인. `Tools/Unreal/mcp_client.py`로 정식 toolset 발견/호출.
- FBX 자동 임포트를 위해 에디터 Python Remote Execution 활성화. 기존 localhost(127.0.0.1)/TTL 0 유지, 에디터 재시작 없음.
- Blender MCP 현재 원본: `ArtSource/map/henhouse_sample_01/henhouse_sample_01.blend`, `CL9_HH01_STAGE`.
  기존 파일은 보존. 별도 `ArtSource/validation/unit_axis/`에 기준 큐브 원본과 FBX 생성.
- 최신 체크포인트: 샘플 8종+1m 큐브 임포트 완료. 치수 오차 <0.003cm, UCX 개수 일치 확인 (`evidence/001-ue-import.json`). 상자 바닥 피벗 수정, 벽/둥지 3단계 LOD 구성. 전체 QA 완료를 의미하지 않는다.
- 검수 맵 `/Game/CL9Art/Tests/Lvl_AssetQA01` 저장. 실제 ItemTable의 날달걀 행만 새 메시로 연결했다. 원래 PIE 설정은 `evidence/001-original-pie-settings.json`에 저장했다.
- 2인 listen PIE 실행: 양쪽에서 날달걀 메시/1.05 배율과 바닥 접촉 확인. 마지막 관찰 당시 PIE 실행 중이므로 재개 시 실제 상태부터 확인한다.
- 운반 테스트 **실패/미통과**: `evidence/001-pie-interactions-retry.json`의 집기 후 attachment=false. 초기 테스트 파일의 observed는 통과 판정이 아니다. 플레이어 시작 위치에 따른 집기 거리와 주문 선택 UI 상태를 진단해야 한다.
- UE 재검수 때 행동: 실제 PIE 월드/플레이어·달걀 위치 확인 → 집기 범위 안에 테스트 시작 위치 설정 → 실제 집기/수납/투척 및 서버·클라이언트 동기화 검증. 결과를 확인하기 전에 합격으로 처리하지 않는다.
- 기존 동물 ABP에서 애니메이션/스켈레톤 누락에 따른 PIE 컴파일 오류 발견. 별도 수정·검수 필요. 전체 완료 수는 계속 0이다.
- 한도/중단 후에는 이 문서와 `iterations/001.md`, 최신 evidence를 읽고 이어간다. PIE가 실행 중이면 에셋 임포트/저장 전에 종료한다. 타임아웃만으로 작업을 중복 실행하지 않는다.

## 실행 중 유지할 기록

현재 반복 번호·선택 ID·현재 단계·Blender 파일/씬·UE 경로·완료 증거·차단 사유·다음 명령.
한 항목이 끝날 때마다 갱신하고, 상세 검수는 개별 카드와 `iterations/NNN.md`에 둔다.
