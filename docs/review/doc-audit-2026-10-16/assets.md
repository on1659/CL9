# 에셋 문서·실물 감사 보고서 (읽기 전용, 2026-10-16)

대상 저장소는 `C:\Work\CL9` 이다. 파일은 하나도 수정하지 않았다. 세 개의 대형 폴더(`C:\Work\CL9\ArtSource\monster\storage_owner_disguise`, `C:\Work\CL9\ArtSource\monster\chicken_disguise`, `C:\Work\CL9\ArtSource\map\henhouse_indoor`)는 상위 폴더 이름만 `ls` 로 보았고, 그 안의 개별 파일은 문서가 직접 가리키는 경로만 존재 여부를 확인했다. 이 폴더들과 `C:\Work\CL9\ArtSource\review`, `C:\Work\CL9\output` 는 모두 git 에 추적되지 않는 상태다(`git ls-files` 결과 0개, 상태 `??`).

판정 기준은 "파일 존재"뿐이다. uasset 은 열어 보지 않았고, 데이터 표(`C:\Work\CL9\Content\Data\ItemTable.uasset`, `PieceTable.uasset`, `SegKindTable.uasset`, `ModuleTable.uasset`)는 에셋 이름 문자열이 들어 있는지만 `grep` 으로 확인했다. 따라서 "표에 연결됨"은 문자열 존재까지만 확인한 것이다.

## 0. 먼저 읽을 요약

1. 카탈로그 `C:\Work\CL9\docs\asset\catalog.md` 의 원본 경로(ArtSource) 컬럼은 모든 행에서 폴더가 실재하고, 해당 폴더의 `export` 아래 FBX 와 `manifest.json` 도 있다. 끊어진 경로는 없다. 문서 링크(`catalog.md`, `production-progress.md` 의 md 링크)도 깨진 것이 없다.
2. 카탈로그에서 실물과 어긋난 행은 다섯 군데다. 계란 cooked·burnt(UE 임포트가 이미 됨), 우리 울타리 `pen_fence`(UE 임포트·표 행이 이미 있음), 돼지·너구리 클립 수(Execute 누락), 농부 전신 클립 수, 그리고 `chicken_disguise` 행이 가리키는 "최신" 버전(문서 안에서 이미 더 새 버전이 나옴)이다.
3. `C:\Work\CL9\docs\asset\production-progress.md` 는 2026-09-21 이후 기록이 없다. 같은 문서 11~12줄은 "계란은 raw 만 임포트됨"이라고 적지만 실물에는 cooked·burnt 가 2026-10-09 에 임포트돼 있다. 또 이 문서와 카탈로그 어디에도 `storage_owner_disguise`, `henhouse_indoor`, `food_truck_b`, `exterior_ground`, `henhouse_roof` 가 나오지 않는다(카탈로그 14행의 `chicken_disguise` 한 줄만 예외).
4. `storage_owner_disguise` 는 "저장고 주인" 소 탈 몬스터이며, 정의 문서는 `C:\Work\CL9\docs\asset\monster\disguise-trio-next-production.md` 160줄 이하다. 문서가 가리키는 현재 후보는 `runtime_walk_v20` 이고, 2026-10-16 의 Mixamo 두 시험 폴더가 그 뒤를 잇는 "사용자 검토 대기" 상태다. 같은 문서 안에 이미 낡은 "현재" 표기가 여러 개 남아 있다.
5. `chicken_disguise` 의 현재 버전은 인계서 22절(652줄)이 말하는 `animation_locomotion_contact_back_v2` 다. 인계서 머리말(8~12줄)은 아직 `animation_trellis_v2` 를 "현재"라고 쓰고, 카탈로그는 `animation_locomotion_v1` 을 "최신"이라고 쓴다. 둘 다 낡았다.
6. `henhouse_indoor` 는 실내 계사 소품 세트다. UE 임포트는 끝났다(`C:\Work\CL9\Content\Maps\Henhouse\Indoor` 에 SM 90개, 확인용 레벨 `Lvl_HenhouseIndoor_Review.umap` 있음). 게임 표 연결은 `ModuleTable.uasset` 에 Indoor 메시 7개 경로가 들어 있는 것으로 보아 일부 진행된 듯하나, 문서(`henhouse_indoor_usage.md`, `PRODUCTION_LOG.md`)는 "PropSets 연결 없음"으로 멈춰 있다.
7. 어느 md 에도 링크되지 않는 docs 내 이미지는 9장이다(`C:\Work\CL9\docs\asset\evidence` 7장, `C:\Work\CL9\docs\reference\image\player` 2장). 상세는 4절.

## 1. 표 1 — 카탈로그 행 대 실제

범례. "FBX/manifest"는 카탈로그의 원본 경로 폴더 아래 export 의 FBX 와 `manifest.json` 존재 여부다. "Content"는 `C:\Work\CL9\Content` 아래 uasset 존재 여부다. 판정은 "일치", "문서 낡음"(실물이 더 앞섬), "문서 누락"(실물은 있으나 문서에 행이 없음)으로 쓴다.

### 1-1. 아이템

| 카탈로그 행(줄) | 문서 상태 | 실제 | 판정 |
|---|---|---|---|
| egg_raw (55) | 모델·FBX 있음, UE "샘플 메시 연결", QA UE 미완료 | `C:\Work\CL9\ArtSource\item\egg_states\export\SM_Egg_Raw_A.fbx` 와 `manifest.json` 있음. `C:\Work\CL9\Content\Items\Egg\SM_Egg_Raw_A.uasset` 있음. ItemTable 에 이름 문자열 있음 | 일치 |
| egg_cooked (56) | 계란후라이로 재제작, UE "임포트·Mesh 열 기록(미실행)" | FBX·manifest 있음. `C:\Work\CL9\Content\Items\Egg\SM_Egg_Cooked_A.uasset` 과 `M_CL9_Egg_Fried.uasset` 이 2026-10-09 19:38 에 존재. ItemTable 에 이름 문자열 있음 | 문서 낡음. 카드 `C:\Work\CL9\docs\asset\item\egg_states.md` 3~5줄은 "첫 판 임포트됨, 둘째 판 재임포트 대기"로 맞게 적었다 |
| egg_burnt (57) | 위와 같음 | `C:\Work\CL9\Content\Items\Egg\SM_Egg_Burnt_A.uasset` 있음(2026-10-09). 작업 트리의 `SM_Egg_Burnt_A.fbx`·`SM_Egg_Cooked_A.fbx` 는 git 상 수정 상태이므로 둘째 판이 UE 에 다시 들어갔는지는 확인하지 않았다 | 문서 낡음 |
| egg_cracked (58) | 제작/FBX 있음, UE 미연결 | `SM_Egg_Cracked_A.fbx` 있음. Content 에는 없음. ItemTable 에도 없음 | 일치 |
| egg_broken (59) | 위와 같음 | `SM_Egg_Broken_A.fbx` 있음. Content·ItemTable 에 없음 | 일치 |
| meat_raw / meat_cooked / meat_burnt (60~62) | FBX 있음, 2,992 tris, UE 영구 연결 완료 | `C:\Work\CL9\ArtSource\item\meat_states\export` 에 FBX 3개와 manifest. `C:\Work\CL9\Content\Items\MeatStates` 에 `SM_Meat_Raw_A`·`SM_Meat_Cooked_A`·`SM_Meat_Burnt_A` 와 정점색 재질 3개 | 일치 |
| cart (63) | 재제작 6,280 tris, `SM_Cart_A` 연결 | `C:\Work\CL9\ArtSource\item\cart_a\export\SM_Cart_A.fbx`·manifest 있음. `C:\Work\CL9\Content\Items\CartA\SM_Cart_A.uasset`, `Items\BP_CL9Cart.uasset` 있음 | 일치 |
| flashlight / headlamp (64, 65) | FBX 있음, 연결 완료 | `C:\Work\CL9\ArtSource\item\portable_lights\export` 에 FBX 2개·manifest. `C:\Work\CL9\Content\Items\PortableLights` 에 `SM_Flashlight_A`·`SM_Headlamp_A` | 일치 |
| frying_pan / pot (66, 67) | FBX 있음, 연결 완료 | `C:\Work\CL9\ArtSource\item\cookware\export` 에 FBX 2개·manifest. `C:\Work\CL9\Content\Items\Cookware` 에 `SM_FryingPan_A`·`SM_Pot_A` | 일치 |
| standing_lamp (68) | FBX 1,552 tris, 연결 완료, `BP_CL9LampItem` | `C:\Work\CL9\ArtSource\item\standing_lamp_a\export\SM_StandingLamp_A.fbx`·manifest 있음. `C:\Work\CL9\Content\Items\StandingLampA\SM_StandingLamp_A.uasset`, `Items\BP_CL9LampItem.uasset` 있음 | 일치 |
| 퀵슬롯 아이콘 7종(31~36줄 설명) | `T_Icon_*` 7개 임포트 완료 | `C:\Work\CL9\Content\UI\Icons` 에 egg·meat·frying_pan·pot·flashlight·headlamp·standing_lamp 와 mic 가 있다. 각 아이템 폴더 `icons\manifest.json` 도 있다 | 일치 |

### 1-2. 몬스터·캐릭터

| 카탈로그 행(줄) | 문서 상태 | 실제 | 판정 |
|---|---|---|---|
| chicken_a (69) | 클립 8+PushReact+PounceWindup(=10), 연결 완료 | `C:\Work\CL9\ArtSource\monster\chicken_a\export` 에 SK 1 + AN FBX 10 + manifest. `C:\Work\CL9\Content\Monsters\ChickenA` 에 AN 10개·SK 2개. `Content\Monsters\ABP_Chicken_A.uasset` 있음 | 일치 |
| goose_a (70) | 클립 10+PushReact | export 에 AN 11개·SK·manifest. `Content\Monsters\GooseA` 에 AN 11개. `ABP_Goose_A.uasset` 있음 | 일치 |
| raccoon_a (71) | "9개" | export 에 AN 10개(`AN_Raccoon_Execute` 포함). `Content\Monsters\RaccoonA` 에 AN 10개. `ABP_Raccoon_A.uasset` 있음 | 문서 낡음(Execute 가 2026-09-21 에 추가됨. `C:\Work\CL9\docs\asset\monster\README.md` 는 Execute 를 적고 있음) |
| pig_a (72) | "9개" | export 에 AN 10개(`AN_Pig_Execute` 포함). `Content\Monsters\PigA` 에 AN 10개. `ABP_Pig_A.uasset` 있음 | 문서 낡음 |
| sheep_a (73) | "9개" | export 에 AN 9개. `Content\Monsters\SheepA` 에 AN 9개. `ABP_Sheep_A.uasset` 있음 | 일치 |
| farmer_a_body (74) | 4100 tris, 연결 완료, "전신 AN 12개 실재" | `C:\Work\CL9\ArtSource\character\farmer_a\export` 에 `SK_Farmer_A_Body.fbx`, AN FBX 25개, manifest(루트에도 `manifest.json`). `C:\Work\CL9\Content\Characters\FarmerA\SK_Farmer_A_Body.uasset` 있음. Anims 폴더에는 전신 AN 14개(Executed 2개 포함)와 1인칭 AN 12개, `ABP_FP_View` 가 있다 | 부분 낡음(클립 수). 별도로 `C:\Work\CL9\docs\asset\character\farmer_a-model-brief.md` 3줄은 "둥근 요리사 v3 외형 채택, UE 연결 미실행"이라 적지만 카탈로그 행에는 v3 언급이 없다. 실물 `SK_Farmer_A_Body.uasset` 의 수정 시각은 2026-10-04 로 v3 blend(2026-10-11)보다 앞선다 |
| farmer_a_arms (75) | 1496 tris, 연결 완료 | `SK_Farmer_A_Arms.fbx` 있음. `Content\Characters\FarmerA\SK_Farmer_A_Arms.uasset` 있음 | 일치 |
| farmer_b_body / farmer_b_arms (76, 77) | 미제작, 원본 "—" | `C:\Work\CL9\ArtSource\character` 에 `farmer_b` 폴더 없음. Content 에도 없음 | 일치 |

### 1-3. 맵 조각·시설

| 카탈로그 행(줄) | 문서 상태 | 실제 | 판정 |
|---|---|---|---|
| henhouse_wall_500_a (78) | 샘플, 연결 완료 | `C:\Work\CL9\ArtSource\map\henhouse_sample_01\export\SM_Henhouse_Wall_500_A.fbx`·manifest 있음. `C:\Work\CL9\Content\Maps\Henhouse\Props\SM_Henhouse_Wall_500_A.uasset` 있음. PieceTable 에 이름 문자열 있음 | 일치 |
| henhouse_wall_half (79) | 792·528 tris, 소비처 없음 | `C:\Work\CL9\ArtSource\map\henhouse_access_parts\wall250\export\SM_Henhouse_Wall_250_A.fbx`·manifest 있음. `C:\Work\CL9\Content\Maps\Henhouse\Access\Wall250\SM_Henhouse_Wall_250_A.uasset`, `Wall125` 도 있음 | 일치 |
| henhouse_fence (80) | 연결 완료 | `C:\Work\CL9\ArtSource\map\henhouse_fence\export\SM_Henhouse_Fence_250_A.fbx`·manifest. `Content\Maps\Henhouse\Fence\SM_Henhouse_Fence_250_A.uasset` | 일치 |
| **pen_fence (81)** | **"UE 미연결 — 임포트 스크립트 실행 대기"**, QA "UE 임포트·이음매·충돌 미실행" | `C:\Work\CL9\ArtSource\map\pen_fence\export\SM_Pen_Fence_250_A.fbx`·manifest 있음. **`C:\Work\CL9\Content\Maps\Henhouse\PenFence\SM_Pen_Fence_250_A.uasset` 있음**. PieceTable 에 `SM_Pen_Fence_250_A` 와 `penFence`, SegKindTable 에 `penFence` 문자열 있음. `C:\Work\CL9\docs\decisions.md` 1579줄도 "UE 임포트 완료"라고 적음 | 문서 낡음 |
| henhouse_door (82) | 연결 완료 | `C:\Work\CL9\ArtSource\map\henhouse_door\export\SM_Henhouse_Door_A.fbx`·manifest. `Content\Maps\Henhouse\Door\SM_Henhouse_Door_A.uasset` | 일치 |
| henhouse_floor (83) | 샘플, 연결 완료 | `henhouse_sample_01\export\SM_Henhouse_Floor_500_A.fbx`. `Content\Maps\Henhouse\Props\SM_Henhouse_Floor_500_A.uasset`. PieceTable 에 문자열 있음 | 일치 |
| henhouse_ceiling (84) | 연결 완료, Stretch100 불필요 | `C:\Work\CL9\ArtSource\map\henhouse_structure\ceiling\export` 에 `SM_Henhouse_Ceiling_500_A.fbx`·`Stretch100_A.fbx`·manifest. `Content\Maps\Henhouse\Structure\Ceiling` 에 두 SM 모두 있음 | 일치 |
| henhouse_stair (85) | 연결 완료 | `C:\Work\CL9\ArtSource\map\henhouse_access_parts\tread\export` 에 FBX 3개·manifest. `Content\Maps\Henhouse\Access\Tread` 에 SM 3개 있음 | 일치 |
| henhouse_nest / crate / feed_sack / work_lamp / trough (86~90) | 샘플, 연결 완료 | `henhouse_sample_01\export` 에 `SM_Henhouse_Nest_ThreeBay_A`·`Crate_A`·`FeedSack_A`·`WorkLamp_A`·`FeedTrough_A` FBX 와 manifest. `Content\Maps\Henhouse\Props` 에 같은 이름 SM 5개 | 일치 |
| henhouse_beam (91) | 보·기둥, 연결 완료 | `henhouse_structure\beam\export\SM_Henhouse_Beam_500_A.fbx`, `pillar\export\SM_Henhouse_Pillar_400_A.fbx`, 각 manifest. `Content\Maps\Henhouse\Structure\Beam`, `Pillar` 에 SM 있음 | 일치 |
| henhouse_sign (92) | 표식 4종, 연결 완료 | `C:\Work\CL9\ArtSource\map\henhouse_yard\sign_{coop,exit,feed,kitchen}\export` 에 FBX·manifest 각 1. `Content\Maps\Henhouse\Yard\Sign{Coop,Exit,Feed,Kitchen}` 에 SM 있음 | 일치 |
| henhouse_ground (93) | 연결 완료 | `henhouse_yard\ground\export` 에 FBX 2개·manifest. `Content\Maps\Henhouse\Yard\Ground` 에 SM 2개 | 일치 |
| henhouse_workbench (94) | 연결 완료 | `C:\Work\CL9\ArtSource\map\henhouse_workbench\export\SM_Henhouse_Workbench_A.fbx`·manifest. `Content\Maps\Henhouse\Workbench\SM_Henhouse_Workbench_A.uasset` | 일치 |
| food_truck (95) | 본체·계단 FBX 2개, 연결 완료 | `C:\Work\CL9\ArtSource\map\food_truck\body\export`·`steps\export` 에 FBX·manifest 각 1. `Content\Maps\FoodTruck\Body\SM_FoodTruck_A.uasset`, `Steps\SM_FoodTruck_Steps_A.uasset` | 일치 |
| truck_pad / truck_panel (96, 97) | 연결 완료 | `C:\Work\CL9\ArtSource\map\truck_parts\{pad,panel}\export` 에 FBX·manifest. `C:\Work\CL9\Content\Items\TruckPad\SM_TruckPad_A`, `TruckPanel\SM_TruckPanel_A`, BP 2개 | 일치 |
| kitchen (98) | 연결 완료 | `C:\Work\CL9\ArtSource\map\kitchen\export\SM_Kitchen_A.fbx`·manifest(조리 효과·장식 하위 폴더에도 manifest). `Content\Items\Kitchen\SM_Kitchen_A.uasset`, `Items\BP_CL9Kitchen.uasset` | 일치 |
| delivery_counter (99) | 연결 완료 | `C:\Work\CL9\ArtSource\map\delivery_counter\export\SM_DeliveryCounter_A.fbx`·manifest. `Content\Items\DeliveryCounter\SM_DeliveryCounter_A.uasset` | 일치 |
| return_jar (100) | 연결 완료 | `C:\Work\CL9\ArtSource\map\return_jar\export\SM_ReturnJar_A.fbx`·manifest. `Content\Items\ReturnJar\SM_ReturnJar_A.uasset` | 일치 |
| campfire (101) | 연결 완료 2026-09-17 | `C:\Work\CL9\ArtSource\map\campfire\export\SM_Campfire_A.fbx`·manifest. `Content\Maps\Henhouse\Props\SM_Campfire_A.uasset` | 일치 |
| night_sky (102) | 재질, 원본 폴더 없음 | `C:\Work\CL9\Content\Maps\Sky\M_CL9_NightSky.uasset` 있음. ArtSource 원본은 문서도 "없음"으로 적음 | 일치 |
| module_yard_gate ~ module_stair_up 17행 (103~119) | 미연결, 원본 "—" | 대응 ArtSource 폴더 없음. 소품·조각 연결은 위 행들이 담당. ModuleTable 은 에셋이 아니라 데이터이므로 uasset 대조 대상 아님 | 일치. 단 `module_pen`(108)의 "Lvl_Flat 만 연결"은 pen_fence 가 이미 임포트된 것과 같이 읽어야 한다 |
| voice_attenuation / voice_speaker_attenuation (120, 121) | 원본 파일 없음, 리드 실행·verify OK | `C:\Work\CL9\Content\Sounds\Master\SA_CL9_Voice.uasset`, `SA_CL9_VoiceSpeaker.uasset` 있음. `T_Icon_mic.uasset` 도 있음 | 일치 |

### 1-4. 카탈로그에 행이 없는 실물(문서 누락)

| 대상 | 실제 | 문서 위치 |
|---|---|---|
| 푸드트럭 B(본체·계단·출발 레버·전원 스위치) | `C:\Work\CL9\ArtSource\map\food_truck_b` 5개 하위 폴더에 FBX·manifest. `C:\Work\CL9\Content\Maps\FoodTruck\B` 에 SM 5개 | 카탈로그·진행 문서에 없음. `C:\Work\CL9\docs\asset\map\food_truck.md` 110줄, 증거 `036-food-truck-b-imports.json` 에 있음 |
| 바깥 지면 | `C:\Work\CL9\ArtSource\map\exterior_ground\export\SM_CL9_ExteriorGround_100_A.fbx`·manifest. `C:\Work\CL9\Content\Maps\Outdoor\SM_CL9_ExteriorGround_100_A.uasset` | 폴더 경로는 어느 문서에도 없고, `C:\Work\CL9\docs\systems\stage.md` 128·133줄이 메시 이름만 언급 |
| 양계장 지붕 | `C:\Work\CL9\ArtSource\map\henhouse_roof\export` 에 FBX 2개·manifest. `C:\Work\CL9\Content\Maps\Henhouse\Structure\Roof` 에 SM 2개 | `C:\Work\CL9\docs\asset\map\henhouse_roof.md` 카드만 있음(카탈로그 행 없음) |
| 양계장 실내 소품 세트 | 5절 참조 | `C:\Work\CL9\docs\asset\map\henhouse_indoor_*.md` 6개 |
| 닭 탈 괴물, 소 탈 몬스터 | 5·6절 참조 | 닭은 카탈로그 14행이 한 줄로 가리킴. 소는 카탈로그·monster README 에 없음 |

### 1-5. `production-progress.md` 의 주장 점검

| 위치 | 문서 내용 | 실제 | 판정 |
|---|---|---|---|
| 7~18줄 "다음 재개 지점 (2026-09-21)" | 계란 cooked·burnt·cracked·broken 은 FBX 만 있고 `Content/Items/Egg/` 에는 raw 만 있다 | cooked·burnt 는 2026-10-09 에 임포트됨. cracked·broken 만 미임포트 | 문서 낡음 |
| 같은 절 | "나머지 카드는 전부 UE 임포트·게임 연결까지 끝났다" | pen_fence 는 2026-09-28 이후 임포트, 문서 작성 시점에는 아직 아니었음(당시엔 맞는 문장) | 시점이 지남 |
| 134~163줄 반복 기록 색인 001~024 | 24개 iterations 문서 | `C:\Work\CL9\docs\asset\iterations` 에 `001.md`~`024.md` 24개 있음. 링크 깨짐 없음 | 일치 |
| 문서 전체 | 2026-09-21 이후 기록 없음 | 카탈로그는 2026-10-12·13 항목까지 갱신. 진행 문서는 2026-10-04 마지막 커밋 | 문서 낡음 |
| 경로 존재 점검 | 문서 안 `ArtSource/...` 경로 전수 | 실제로 없는 경로는 아래 3개뿐이었다. `C:\Work\CL9\ArtSource\monster\storage_owner_disguise\animation_natural_v1`, `...\pantry_owner_disguise\animation_natural_v1`, `...\hive_owner_disguise\animation_natural_v1`. 모두 `disguise-trio-next-production.md` 의 후속 계획 경로이며, 저장고 폴더의 실제 이름은 `animation_natural_v01`~`v14`, 찬장·벌집 폴더는 만들어진 적이 없다. 나머지 불일치는 한글 조사가 경로에 붙은 파싱 잡음이다 | 계획 경로 3건만 해당 |

## 2. 표 2 — ArtSource·output 이미지 폴더의 문서상 지위

이미지(png/jpg)를 가진 폴더를 대상으로 했다. 지위는 문서가 직접 적은 표현만 따랐다. 문서가 지위를 말하지 않으면 "언급 없음" 또는 "경로만 언급"으로 썼다.

### 2-1. 현재 연결된 자산의 원본 폴더(현재 정본)

| 폴더 | 지위 | 근거 | 비고 |
|---|---|---|---|
| `C:\Work\CL9\ArtSource\item\cart_a\preview`, `cookware\preview·icons`, `egg_states\preview·icons`, `meat_states\preview·icons`, `portable_lights\preview·icons`, `standing_lamp_a\preview·icons` | 현재 정본(게임 연결 행의 원본) | `C:\Work\CL9\docs\asset\catalog.md` 55~68줄, `item\*.md` 카드 | 계란 cooked·burnt 는 둘째 판이 Blender 검수만 됨(`item\egg_states.md` 3~5줄) |
| `C:\Work\CL9\ArtSource\map\{campfire,delivery_counter,henhouse_access_parts,henhouse_door,henhouse_fence,henhouse_sample_01,henhouse_structure,henhouse_workbench,henhouse_yard,kitchen,pen_fence,return_jar,truck_parts,food_truck}` 의 preview·하위 폴더 | 현재 정본 | `catalog.md` 78~101줄, `docs\asset\map\*.md` | `pen_fence` 는 UE 임포트 완료인데 카탈로그 81줄은 미연결로 적음 |
| `C:\Work\CL9\ArtSource\character\farmer_a\preview` (181장) | 현재 정본(게임 연결된 농부 A) | `catalog.md` 74·75줄, `docs\testing\pie-scenarios.md` 102줄이 `preview\*_fp.png` 를 가리킴 | |
| `C:\Work\CL9\ArtSource\character\farmer_a\blockout` (47장) | 현재 채택 외형(v3 둥근 요리사)의 원본. 게임 미연결 | `C:\Work\CL9\docs\asset\character\farmer_a-model-brief.md` 3줄("v3 외형 사용자 채택 … UE 연결 미실행"), 24·58·83·144·204줄 | 하위 `archive`, `rigged_walk`, `farmer_a_round_cook_candidate.blend`. 카탈로그에는 반영 없음 |
| `C:\Work\CL9\ArtSource\monster\chicken_a\preview` (52장), `goose_a\preview`, `pig_a\preview`, `raccoon_a\preview`, `sheep_a\preview` | 현재 정본(게임 연결 5종) | `catalog.md` 69~73줄, `monster\*_a.md` | `chicken_a\preview` 하위 이름은 README 가 `react_*_sheet.png` 로만 언급 |
| `C:\Work\CL9\ArtSource\ui\icons` | 현재 정본(HUD 마이크 아이콘) | `catalog.md` 37줄, `docs\asset\README.md` 76·130줄 | |
| `C:\Work\CL9\ArtSource\map\food_truck_b\*` | 현재 정본(임포트됨) | `docs\asset\map\food_truck.md` 110줄, `docs\goal\round-start-truck-intro.md` 132줄, `docs\plan\data\rules.md` 155줄 | 카탈로그에는 행 없음 |
| `C:\Work\CL9\ArtSource\map\henhouse_roof\preview` | 현재 정본(임포트됨) | `docs\asset\map\henhouse_roof.md` 31·41줄 | 카탈로그에는 행 없음 |
| `C:\Work\CL9\ArtSource\map\exterior_ground\preview` | 언급 없음(폴더 경로 기준). 실물은 임포트돼 사용 중 | `docs\systems\stage.md` 128·133줄은 UE 메시 이름만 언급 | |

### 2-2. 과거 시안·미승인·보존용

| 폴더 | 지위 | 근거 | 비고 |
|---|---|---|---|
| `C:\Work\CL9\ArtSource\character\farmer_a_v2` (8장+텍스처 1장) | 과거 시안 — "귀여움 1단계, 사용자 승인 전", 게임 미연결 | `docs\asset\character\farmer_a.md` 234~259줄 | 이후 2026-10-11 에 v3(blockout) 채택으로 방향이 바뀜 |
| `C:\Work\CL9\ArtSource\monster\chicken_a\v2` (21장) | 과거 시안 — "귀엽게 톤 1단계, 사용자 승인 대기" | `docs\asset\monster\chicken_a.md` 64~70줄 | 실물·export·Content 는 건드리지 않았다고 명시 |
| `C:\Work\CL9\ArtSource\monster\goose_a_v2` (12장), `pig_a_v2` (10장), `raccoon_a_v2` (5장), `sheep_a_v2` (25장) | 과거 시안 — 형태+렌더만, 게임 미연결 | `goose_a.md` 30·48줄, `pig_a.md` 39줄, `raccoon_a.md` 53줄, `sheep_a.md` 21줄 | `C:\Work\CL9\docs\asset\design-concept.md` 는 기존 몬스터 선정을 철회하고 재논의 중이라 적음 |
| `C:\Work\CL9\ArtSource\monster\disguise_trio_preview_v1` (reference 1장 + hive·pantry·storage 각 8장) | 과거 시안(비교용 보존). 단 `reference\owners_reference.png` 는 "사용자 첨부 외형" 기준 이미지로 현재도 쓰임 | `docs\asset\monster\disguise-trio-next-production.md` 7·9줄(reference), 421줄(체크섬 기준), 138줄("V1은 비교용으로 보존") | |
| `C:\Work\CL9\ArtSource\monster\disguise_trio_preview_v2` (각 7장) | 현재 형태 기준 프리뷰(세 종의 본체 구분). 본 제작 전 단계, 사용자 채택 전 | 같은 문서 24줄, 128~147줄("본체의 형상 기준은 아래 V2 프리뷰") | 찬장(pantry)·벌집(hive) 주인은 후속 본 제작 폴더가 없다 |
| `C:\Work\CL9\ArtSource\character\_reference` | 이미지 없음(fbx 5개, json·blend). 참고본 | `farmer_a-model-brief.md` 27줄(기존 Walk 추출본), `production-progress.md` 의 "참고본이며 신규 모델 완료로 세지 않는다" | |

### 2-3. 실험·언급 없음

| 폴더 | 지위 | 근거 | 비고 |
|---|---|---|---|
| `C:\Work\CL9\ArtSource\review\chicken_methods_20261013` (`_review` 에 jpg·png 10장, 상위에 mp4 5개) | 언급 없음 | docs 전체 Grep 결과 0건 | 파일 이름으로는 닭 탈 제작 방식 5종(Blender 직접·TRELLIS·Higgsfield·보정 게임 적용 3배속·닭 괴물 걷기) 비교 영상이다. 이름만으로 판단한 것이며 문서가 정의하지 않는다 |
| `C:\Work\CL9\ArtSource\character\farmer_a\textures` (1장) | 언급 없음 | Grep 0건 | 팔레트 텍스처로 보이나 확인하지 않음 |
| `C:\Work\CL9\ArtSource\map\henhouse_roof`·`exterior_ground` | 2-1 참조 | | |

### 2-4. `output\concepts` 5개 폴더

| 폴더 | 지위 | 근거 | 비고 |
|---|---|---|---|
| `C:\Work\CL9\output\concepts\2026-10-10-local-game-loop-45s-v1` (png·jpg 16장 + mp4·json) | 현재 정본의 일부 — 농부 A 전신 형태 기준 R1(`03-eggraid-reference.png`) | `docs\asset\character\farmer_a-model-brief.md` 79줄, `farmer_a-visual-reference.md` 13줄 | 나머지 파일은 문서 링크 없음. Tools 의 json 에도 경로가 남아 있음 |
| `C:\Work\CL9\output\concepts\2026-10-10-combat-mockup-v1` (8장 + md·py) | 현재 정본의 일부 — R2(`combat-scene-mockup.png`), R3(`combat-scene-mockup-fps.png`) | `farmer_a-model-brief.md` 80·81줄, `farmer_a-visual-reference.md` 14·15줄. 문서가 "R1·R2는 전신 형태 기준, R3는 1인칭 손·팔 형태 기준이며 색·분위기 참고로 격하하지 않는다"고 못박음 | 스토리보드 md·png 와 `inspect_current_game_reference.py` 는 문서가 링크하지 않음 |
| `C:\Work\CL9\output\concepts\2026-10-10-local-film-30s-v1` (42장 + mp4·json) | 언급 없음 | docs 전체 Grep 0건 | `.claude\runs\*\baseline.json` 에 미추적(`??`) 파일 목록으로만 등장 |
| `C:\Work\CL9\output\concepts\2026-10-10-round-cooks-video` (4장 + blend·py·wav) | 언급 없음 | 같음 | |
| `C:\Work\CL9\output\concepts\2026-10-10-b-horror` (3장) | 언급 없음 | 같음 | |

### 2-5. 대형 폴더 세 곳

| 폴더 | 지위 | 근거 | 비고 |
|---|---|---|---|
| `C:\Work\CL9\ArtSource\monster\storage_owner_disguise` | 3.1절 | 3.1절 | |
| `C:\Work\CL9\ArtSource\monster\chicken_disguise` | 3.2절 | 3.2절 | |
| `C:\Work\CL9\ArtSource\map\henhouse_indoor` | 3.3절 | 3.3절 | |

## 3. 대형 폴더 세 곳의 상세

### 3.1 `storage_owner_disguise` — "저장고 주인" 소 탈 몬스터

정의 문서는 `C:\Work\CL9\docs\asset\monster\disguise-trio-next-production.md` 다. 그 문서 43줄이 "저장고 주인 — 무거운 본체", 160줄이 "2026-10-13 소 탈 몬스터 신규 전체 제작 요청", 176줄이 `asset_id: storage_owner_disguise` 를 적는다. 사용자 승인 사항은 외형 컨셉 v03(195~197줄), Blender 외형 v02(201~209줄, 1184줄 "실제 Blender v02 외형 승인")다. 그 밖에 이 폴더를 언급하는 문서는 `C:\Work\CL9\docs\asset\tools\monster-animation-sources-20261016.md`(60·62·91줄) 하나뿐이다. `decisions.md`, 카탈로그, 진행 문서, `monster\README.md` 에는 소가 없다. `Content\Monsters\StorageOwnerDisguise` 에는 실제 UE 에셋이 있다.

폴더 상위에는 하위 폴더와 로그 파일 합쳐 206개 항목이 있다. 폴더 이름은 `animation_natural_v01~v14`, `blender_v01~v02`, `centered_cycle_v01~v20`, `centered_peel_v01~v02`, `concept_v01~v03`, `meshy_*`(centered_cycle v09~17, chicken_style v01~08, full_cycle v01~19, grasp_cycle_v18, textured_runtime_v19, compare_v01), `mixamo_*`, `motion_coupled_skin_v01~04`, `motion_probe_*`, `motion_sketch_v01~02`, `runtime_walk_v20`, `v19_motion_patch_v01~03` 등이고 나머지는 `.log` 85개다.

**문서가 가리키는 현재 버전**

1. 현재 테스트 레벨 후보는 `C:\Work\CL9\ArtSource\monster\storage_owner_disguise\runtime_walk_v20`. 문서 근거는 `disguise-trio-next-production.md` 1339~1350줄("현재 테스트 레벨 — RuntimeWalkV20"), 320~337줄의 YAML(`test_level_candidate`, `pending_review: runtime-walk-v20`, 상태 "사용자 시각 검토 대기"). 확인한 파일은 모두 있다: `cow_runtime_walk_v20.blend`, `delivery_manifest.json`, `review\cow_v20_doff_roam_same_skin_don_roam_1x.mp4`. UE 쪽은 `C:\Work\CL9\Content\Monsters\StorageOwnerDisguise\RuntimeWalkV20` 에 `BP_CowWalkV20`, `ABP_CowBodyV20`, `ABP_CowCostumeV20`, `QA_CowWalkV20.umap`, Body·BodySupported·Costume 하위 폴더가 있다(2026-10-16 19:03~19:16 생성).
2. 그 뒤 2026-10-16 에 추가된 시험 둘이 "사용자 검토 대기"다. `mixamo_motion_probe_v01`(1352~1366줄)과 `mixamo_getup_probe_v02`(1368~1381줄). 각 `review\*.mp4`, `.blend` 가 있다. `mixamo_free_library_20261016` 은 무료 모션 65개 보관본이며 문서가 "전부 적용 대상으로 취급하지 않는다"고 적는다.
3. 외형 정본은 `concept_v03\cow_disguise_concept_v03.png` 와 `blender_v02\approved_appearance_v02.blend`. 둘 다 있다.
4. 이미지 이력으로 문서가 링크하는 것은 `concept_v01`·`concept_v02`·`concept_v03` 의 png, `fit_rule_review_v01\size_options_review.png`, `motion_sketch_v01`·`motion_sketch_v02` 의 png, `meshy_grasp_cycle_v18\approved_grip_storyboard.png`, `blender_v02\preview\*.png` 다. 모두 있다.

**문서 안의 낡은 "현재" 표기(같은 문서 내부 불일치)**

- 162줄: "현재 작업: meshy-textured-runtime-v19, 텍스처·언리얼 연결·실제 PIE 촬영 제작 중". 이후 1339줄 이하에서 v20 이 대체했다.
- 609줄 `current_work` 와 1065줄 `current_directory` 는 아직 `meshy_full_cycle_v18` 을 가리킨다.
- 1223~1231줄 "소 v13 납품"(`animation_natural_v13`, Content `RuntimeV03`)은 바로 아래 1233~1236줄에서 사용자가 반려했고 "현재 최종 합격이 아니다"라고 쓴다.
- 64줄의 후속 경로 `animation_natural_v1/` 은 실제 이름 `animation_natural_v01` 과 다르다.
- 실제 UE 에셋 폴더는 `ProductionV01`, `RuntimeV02`, `RuntimeV03`, `SkinProbeV09`, `MeshyReviewV01`, `MeshyRuntimeV19`, `RuntimeWalkV20` 일곱 개다. 문서가 직접 설명하는 것은 이 중 일부다.

**문서에 한 번도 이름이 나오지 않는 하위 폴더(정리 후보를 판단할 재료)**: `animation_natural_v01·02·03·04·06·07·08·09·10·11·12·14`, `blender_v01`, `centered_cycle_v01~06·08·09·11·12·14~19`, `centered_peel_v01`, `meshy_centered_cycle_v09~16`, `meshy_chicken_style_v01~07`, `meshy_full_cycle_v01~15·17·19`, `motion_coupled_skin_v01·03`, `motion_probe_skin_v01~11`, `motion_probe_v01·04~15·17`, `v19_motion_patch_v01`. 위치 조사에서는 폴더 이름만 보았고 내용은 열지 않았다. 이 폴더들이 이미지를 갖는지는 대형 폴더 규칙에 따라 훑지 않았으므로 확인하지 않았다.

### 3.2 `chicken_disguise` — 닭 탈 괴물

현재 인계서는 `C:\Work\CL9\docs\asset\monster\chicken_disguise-animation-handoff.md`(671줄)다. 이 문서의 22절(652~671줄, 2026-10-16)이 가장 새롭고, 현재 정본 폴더는 `C:\Work\CL9\ArtSource\monster\chicken_disguise\animation_locomotion_contact_back_v2` 다. 문서가 "정본은 `chicken_disguise_contact_back.blend` 하나다. 같은 폴더의 `v2*`, `complete`, `routed`, `delivery*` 후보는 최종 납품본으로 선택하지 않는다"고 적었고, 실제로 그 폴더에는 같은 종류의 blend 가 십여 개 있다. 확인한 파일은 모두 있다: `chicken_disguise_contact_back.blend`, `qa.md`, `manifest.json`, `preview\LV_Flat_ChickenContactBackV2.mp4`, `export\GC_ChickenDisguise_Final.abc`, `storyboard\chicken_doff_five_steps_v1.png`. UE 쪽도 있다: `C:\Work\CL9\Content\Monsters\ChickenA\SessionCurrent\ContactBackV2` 에 `LS_Chicken_ContactBackV2_Final`, `GC_Chicken_ContactBackV2`, `GC_Chicken_FaceClearanceV2`, `LS_Chicken_ContactBackV2`.

**버전별 지위**

| 폴더 | 지위 | 근거 |
|---|---|---|
| `reference` (`approved_chicken_disguise_20261011.png`, `fullbody_redraw_20261011_v1`) | 현재 정본(외형 기준 이미지) | `docs\decisions.md` 1782줄, `chicken_disguise-animatic.md` 5줄, `chicken_disguise-natural-production.md` 11줄 |
| `animation_locomotion_contact_back_v2` | 현재 정본(2026-10-16) | 인계서 22절 652줄 |
| `animation_locomotion_v1` | 직전 기준. 22절이 "원본 SHA256 그대로"라며 보존 | 인계서 21절 632줄. 카탈로그 14줄은 이것을 "최신"이라 적음 |
| `animation_round_shrink_v1`, `animation_round_ingame_3x_v1`, `animation_session_ingame_v1`·`v2` | 중간 단계. 인계서 12~20절이 각자 설명 | 인계서 603~621·527~570줄 |
| `animation_trellis_v2` | 인계서 머리말(8~12줄)은 "현재"라 적지만 뒤 절들이 대체한 중간 버전 | 인계서 25줄 |
| `animation_trellis_v1` | 반려 이력(각진 외형) | 인계서 16·20줄, 37줄 |
| `animation_handmade_off_on_v2`, `animation_handmade_pickup_v1`, `animation_ppf_grip_v4` | 실험·시험 | 인계서 461·506·589줄 |
| `animation_higgsfield_pickup_v1`, `animation_tool_trials_20261012` | 외부 도구 시험 | 인계서 18·406줄, `docs\asset\tools\animation-tool-trials-20261012.md` 7·12·45줄 |
| `animation_natural_v1` | 과거 이력(외형 반려) | `chicken_disguise-natural-production.md` 3~11줄 |
| `animatic_v1`~`v4` | 과거 초기 시안. v3·v4 의 이빨 얼굴은 채택하지 않음 | `chicken_disguise-animatic.md` 5~9·15~18줄 |
| `model_fullbody_v1`, `trellis2_v1` | 2026-10-11 정지 외형·TRELLIS 생성 이력 | `chicken_disguise-fullbody-model.md` 1~3줄, 인계서 104~124줄 |
| `animation_hybrid_v1`, `animation_ppf_grip_v1`·`v2`·`v3`·`v5` | 문서에 경로로 언급되지 않음 | docs 전체 Grep 0건 |

**어긋난 서술**: 카탈로그 14줄과 `monster\README.md` 12~13줄의 "독립 시안, 게임 연결 미연결"은 맞다. 다만 카탈로그가 링크하는 `§21` 앵커는 실제로 존재하지만 가리키는 버전이 한 단계 낡았다. 또 `Content\Monsters\ChickenA\SessionCurrent` 와 `RoundFinishPreview` 는 기존 `chicken_a` 폴더 안에 놓여 있어서 "기존 chicken_a 런타임과 구분된다"는 README 의 문구와 위치가 다르다(내용은 GeometryCache·LevelSequence 프리뷰이며 기존 `ABP_Chicken_A` 와 별개).

### 3.3 `henhouse_indoor` — 양계장 실내 소품 세트

정의 문서는 `C:\Work\CL9\docs\asset\map\henhouse_indoor_objects.md`(목록·톤·규칙, 2026-10-13), `henhouse_indoor_prompts.md`(이미지 프롬프트 정본), `henhouse_indoor_gpt_prompt.md`(복사용 합본), `henhouse_indoor_handoff.md`·`henhouse_indoor_handoff_bundle.md`(ChatGPT 이관 브리프, 번들은 생성물), `henhouse_indoor_usage.md`(UE 임포트·레벨 배치 절차, 2026-10-14)다. `docs\asset\map\README.md` 17~21줄이 이 목록을 "아래 양계장 카드들을 대체"한다고 적는다.

폴더 상위 항목은 99개다(제작 ID 폴더 약 90개와 `_overview`, `_template`, `README.md`, `PRODUCTION_LOG.md`, `IMAGE_CATALOG.json`, `PACK_COVERAGE.json`, `production_contract.json` 등). `_overview` 에는 `assembly_v01~v05`, `concept`, `delivery_file_audit.json`, `shell_kit_v01·v02.json` 이 있다. 이 폴더들은 문서가 "현재 제작 정본"으로 가리키며, `PRODUCTION_LOG.md` 맨 위(2026-10-14)가 제작 상태 정본이다. 유일한 미납품 ID 는 `egg_tray_stack` 이고 Content 에도 없다.

**UE 임포트 대응**: `C:\Work\CL9\Content\Maps\Henhouse\Indoor` 에 uasset 91개(SM 90개 + 공용 재질 `Materials\M_CL9_Henhouse_VertexColor`)가 있다. 문서의 "90 FBX"와 같다. 확인용 레벨 `C:\Work\CL9\Content\Maps\Henhouse\Lvl_HenhouseIndoor_Review.umap`(2026-10-15) 도 있다. 구매 팩 25개 ID 의 재배치 위치는 문서가 말한 `Content\Maps\Henhouse\Props` 와 일치한다.

**게임 연결**: 문서(`PRODUCTION_LOG.md`, `usage.md` 269줄, `look-rework.md` 64줄)는 "PropSets·런타임 맵 연결 없음, 신규 껍데기의 PieceTable·SegKind 연결은 별도"라고 적는다. 그런데 `Content\Data\ModuleTable.uasset`(git 상 수정 상태)에는 `Henhouse/Indoor/basket`, `egg_box_stack`, `feathers`, `poster`(A·B), `stain`(A·B) 일곱 개 메시 경로 문자열이 들어 있다. 문서가 갱신되지 않았거나 작업 중인 변경이다. 표 내용은 열어 보지 않았다.

**카탈로그 반영**: 없음. 카탈로그와 진행 문서 어디에도 `henhouse_indoor` 가 없다. 그래서 90개 SM 은 "필수 45개" 분모에 포함되지 않은 채 UE 에 들어가 있다.

## 4. 어떤 md 도 링크하지 않는 docs 내 이미지

대상은 `C:\Work\CL9\docs` 아래 png·jpg·jpeg 전체(13장)이며, `docs`, `.claude`, `Tools` 의 md 와 루트 md 의 링크·`<img>` 를 풀어 대조했다. `C:\Work\CL9\docs\asset\iterations` 에는 이미지가 없다.

링크되지 않는 이미지는 아래 9장이다.

| 이미지 | 다른 방식의 언급 |
|---|---|
| `C:\Work\CL9\docs\asset\evidence\001-pie-modal.png` | 없음 |
| `C:\Work\CL9\docs\asset\evidence\001-pie-view-0.png` | 없음 |
| `C:\Work\CL9\docs\asset\evidence\001-pie-view-1.png` | 없음 |
| `C:\Work\CL9\docs\asset\evidence\001-ue-editor.png` | 없음 |
| `C:\Work\CL9\docs\asset\evidence\024-farmer-idle.png` | 없음(스크립트 `Tools\Unreal\qa_farmer_pie.py` 24줄이 이 이름으로 저장함) |
| `C:\Work\CL9\docs\asset\evidence\032-anim-pie-down.png` | `docs\asset\character\farmer_a.md` 206줄이 이름을 백틱으로만 적음(링크 아님) |
| `C:\Work\CL9\docs\asset\evidence\032-anim-pie-visual.png` | 위와 같음 |
| `C:\Work\CL9\docs\reference\image\player\lethal_hud.jpeg` | `docs\systems\ui.md` 76줄, `docs\testing\pie-scenarios.md` 915줄이 경로를 백틱으로 적음(링크 아님) |
| `C:\Work\CL9\docs\reference\image\player\repo_hud.jpg` | 없음(저장소의 md·py·json 어디에도 이름이 없음) |

링크된 나머지 4장은 `docs\asset\evidence\024-farmer-lit.png`, `024-farmer-palette.png`, `docs\reference\image\design\1.png`, `2.png` 다. `design\1.png` 는 `docs\asset\design-concept.md` 가 "채택하지 않는다"고 적은 복셀 레퍼런스이고, `2.png` 는 채택한 삼각면 로우폴리 기준이다.

## 5. 정리 판단에 쓸 수 있는 관찰(추측 없음)

- 문서가 "현재"라고 가리키는 폴더는 닭 `animation_locomotion_contact_back_v2`, 소 `runtime_walk_v20`(과 10-16 의 Mixamo 두 시험), 양계장 실내 `henhouse_indoor` 전체, 농부 `blockout`(v3), 그리고 이미 게임에 연결된 `item`·`map`·`monster\*_a`·`character\farmer_a` 다.
- 문서가 "과거·미승인 시안"으로 적은 이미지 폴더는 `farmer_a_v2`, `chicken_a\v2`, `goose_a_v2`, `pig_a_v2`, `raccoon_a_v2`, `sheep_a_v2`, `disguise_trio_preview_v1`(reference 제외)이다.
- 문서에 이름이 없는 폴더는 `ArtSource\review\chicken_methods_20261013`, `ArtSource\character\farmer_a\textures`, `output\concepts` 의 세 폴더(`b-horror`, `local-film-30s-v1`, `round-cooks-video`)이며, `ArtSource\monster\storage_owner_disguise` 와 `chicken_disguise` 안에도 문서가 부르지 않는 하위 폴더가 다수 있다(3.1·3.2절 목록).
- 위 폴더 중 어떤 것도 git 에 추적되지 않으므로 삭제하면 되돌릴 수 없다.
