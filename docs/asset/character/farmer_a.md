# 농부 A — 작업복 풀바디와 1인칭 팔

현재 행 farmer_a의 Manny/SK_Mannequin·ABP_Unarmed·ABP_FP_Copy 계약을 유지한다.
캡슐 반경39cm/반높이90cm·눈높이156cm·메시 Yaw-90도는 변경하지 않는다.
작업복 몸, 고무 장화, 작업 장갑, 짧은 챙 모자와 단순한 얼굴을 새 기하로 제작한다.
참고 마네킹의 얼굴·몸 표면을 복제해서 신규 외형이라고 납품하지 않는다. 기존 뼈/레스트만 재사용한다.

- Metric 1m 원본, 기본 키 약183cm(모자 포함), A포즈 폭은 실제 참조 관절에 맞춘다.
- 목표 풀바디12k tris 안팎, 팔6k 안팎. 실제 손가락5개/손목/팔꿈치 변형을 우선한다.
- 옷 TintColor 영역, 피부, 장화·장갑·띠·얼굴 세부3재질. 큰 작업복 영역으로4슬롯 색을 구분한다.
- 단정한 형태와 둔탁한 비율, 녹색 작업복·낡은 갈색 띠·짙은 장화. 조밀한 장식이나 기존 게임 캐릭터 복제 없음.
- 리그 원본은 실제 UE에서 추출한89뼈. Blender88뼈+root 오브젝트를 FBX root로 내보낸다.
  뼈 이름/부모/레스트와 bone scale1을 [cm 임시 내보내기 왕복](../evidence/024-rig-roundtrip-cm.json)에서 확인했다.
  원본 m 단위는 유지하고 export 복사본만 cm로 바꾼다. 일반 m FBX는 UE root scale100으로 실패했다.
- 몸과 팔은 같은 관절/장갑 원본을 공유하되 별도 메시로 납품한다. 기존 팔 소비처는 몸 리그 Copy Pose 방식이므로
  같은89뼈 계약을 유지하고 시점 가시성을 실제 UE에서 확인한다.

기존 로코모션의 첫 Walk 호환 확인 후 Carry/Use/HitReact/Knockdown/GetUp/Death/Exhausted 등 필요한 상태를 만든다.
팔은 Pickup/Drop/Swap/Throw/Use와 운반물 그립을 검수한다. Notify가 판정·재고를 바꾸지 않는다.
TintColor의 실제 소비/4슬롯 적용은 현재 코드에 연결이 없으므로 BP/재질 연결과 검수 전까지 미완료로 남긴다.
모델/클립/실사용/2인/성능은 제작·검수 진행 중이다.

## 024 제작 결과 (부분 검수)

`ArtSource/character/farmer_a/farmer_a.blend`에 몸/팔/기존 계약의 리그를 저장했다.
몸4100tris/2148정점/3재질, 팔1496tris/784정점/2재질. 몸111.35×43.05×184.4cm A포즈,
팔111.35×40.31×57.12cm. UV0 unwrap/UV1 팔레트/최대2가중치, 5개 손가락을 각각 기존 뼈로 변형한다.
공유18색256×8 PNG는 packed 원본과 `textures/T_Farmer_Palette.png`에 있으며 외부 서비스 의존성 없다.
UE `/Game/CL9Art/Characters/FarmerA/`에 두 SkeletalMesh/3재질/팔레트가 있다.

원본 표면은 새로 제작했고 Epic에서 재사용한 것은 리그 계약과 기존 로코모션이다.
중립·야간·손전등/Walk2포즈, 기존 Skeleton 변경 없음, 실제 CL9 pawn 몸/팔 교체와
Unarmed/FP Copy AnimBP·이동, 팔레트 적용 후 실제 UE 화면의 색을 확인했다.
`TintColor` 입력은 재질에 있고 실제4슬롯 소비 연결은 아직 없다.

## 025 Hold 클립 (2026-09-12)

행의 `HoldPose` 범주별 상체 자세 8클립: 전신 `AN_Farmer_HoldSmall/Flashlight/Large/Idle`(가슴 앞, 팔꿈치 굽힘 —
Idle은 손 내린 자연 자세로 전신 ABP가 쓰지 않는 예비)와 1인칭 팔 `AN_Farmer_FPHoldSmall/Flashlight/Large/Idle`
(눈높이 근처, 게임 팔 FOV 70 안 — FPHoldIdle은 빈손일 때 두 손이 화면 아래 양쪽에 보이는 리썰컴퍼니식 자세,
손목 z 1.39m·앞 0.42m). 두 뼈 IK로 손목을 목표에
두고(`Tools/Blender/farmer_hold_spec.json`: wrist·pole·fingers·curl) 손가락은 국부 Z축(실측)으로 굽힌다.
2초 루프, 숨쉬기 0.012rad, 회전 키만(위치 키 없음) — `Tools/Blender/animate_farmer_hold.py --export`가
`ArtSource/character/farmer_a/export/AN_Farmer_*.fbx`와 manifest `animations`를 쓰고 blend를 저장한다.
UE 임포트는 `Tools/Unreal/import_farmer_hold.py`(레거시 FBX, SK_Mannequin) → `/Game/CL9Art/Characters/FarmerA/Anims`,
[025 증거](../evidence/025-farmer-hold-imports.json). 미리보기 `preview/hold_*_{tp,fp}_{front,fp,hand}.png`
(`_fp_fp.png`는 카메라 설정이 틀려 빈 화면 — 1인칭 판정은 PIE로 했다). ABP 배선과 PIE 검수는 2026-09-12
완료 — [../../systems/animation.md](../../systems/animation.md) "Hold 상체 레이어 배선".
아직 없는 것: 줍기/놓기/던지기 동작, 손가락 접촉(물건 크기별 그립), 걷기 중 팔 흔들림.

다음 필수: 어깨/고관절·손가락의 운반 범위 변형 검수, Carry/Use/피격·넘어짐 등 상태 클립,
1인칭 카메라/FOV·팔 절단면/그립,4슬롯·2인 표현,LOD·성능,영구 데이터 연결.
[024 증거](../iterations/024.md). 첫 몸/팔 외형과 호환 검수이며 전체 캐릭터 완료가 아니다.
