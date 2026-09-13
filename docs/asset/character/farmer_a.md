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
UE `/Game/Characters/FarmerA/`에 두 SkeletalMesh/3재질/팔레트가 있다.

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
UE 임포트는 `Tools/Unreal/import_farmer_hold.py`(레거시 FBX, SK_Mannequin) → `/Game/Characters/FarmerA/Anims`,
[025 증거](../evidence/025-farmer-hold-imports.json). 미리보기 `preview/hold_*_{tp,fp}_{front,fp,hand}.png`
(`_fp_fp.png`는 카메라 설정이 틀려 빈 화면 — 1인칭 판정은 PIE로 했다). ABP 배선과 PIE 검수는 2026-09-12
완료 — [../../systems/animation.md](../../systems/animation.md) "Hold 상체 레이어 배선".
아직 없는 것: 줍기/놓기/던지기 동작, 손가락 접촉(물건 크기별 그립), 걷기 중 팔 흔들림.

다음 필수: 어깨/고관절·손가락의 운반 범위 변형 검수, Carry/Use/피격·넘어짐 등 상태 클립,
1인칭 카메라/FOV·팔 절단면/그립,4슬롯·2인 표현,LOD·성능,영구 데이터 연결.
[024 증거](../iterations/024.md). 첫 몸/팔 외형과 호환 검수이며 전체 캐릭터 완료가 아니다.

## 026 휘두르기 클립 (2026-09-13)

좌클 `Swing`의 표현 2클립: 전신 `AN_Farmer_Swing`·1인칭 팔 `AN_Farmer_FPSwing` — Small 들기 자세(시작=끝)에서 오른손을
치켜들었다가(0.12s) 정면 아래로 내려치고(0.25s) 되돌아오는 0.45초 원샷(30fps 15프레임, 키 자세 사이 선형 보간).
`Tools/Blender/animate_farmer_swing.py`가 `farmer_pose.py`(hold 스크립트에서 떼어낸 두 뼈 IK·손가락·FBX 헬퍼)로 만들고
`--export`로 FBX·manifest(`swing_keys`)·blend를 쓴다. UE 임포트 `Tools/Unreal/import_farmer_swing.py` →
`/Game/Characters/FarmerA/Anims`(2026-09-12 폴더 이동 뒤 경로), [026 증거](../evidence/026-farmer-swing-imports.json).
미리보기 `preview/swing_{tp,fp}_f{1,5,9}_front.png`. ABP 배선은 [../../systems/animation.md](../../systems/animation.md) "휘두르기 가지".
알려진 것(2026-09-13 밤 확인): `export_action`의 스테이징 장면이 24fps 였다. 30fps 로 저작한 15프레임이 24fps 로 구워져 UE 에
**0.583초**로 들어갔다(ABP 문턱 0.58 은 그 길이에 맞춘 값). 028 에서 스테이징 fps 를 저작 장면과 같게 고쳤으므로 이 스크립트를
다시 `--export` 하면 0.467초(14/30)가 되고 ABP 문턱·`SwingElapsed` 오프셋을 같이 바꿔야 한다 — 재수출은 하지 않았다.

## 028 공격 대기·먹기 클립 + 그립 역산 (2026-09-13 밤, use-hint-anim)

- **공격 대기** `AN_Farmer_SwingWindup`(전신)·`AN_Farmer_FPSwingWindup`(1인칭 팔): 좌클을 누르고 있는 동안 도는 **1.0초 루프**
  (30fps 31프레임, 첫·끝 프레임 동일 — 손 위치 차 0.0cm). 자세는 휘두르기 `KEYS` t=0.12 의 치켜든 자세 그대로이고 손목 z 가
  ±1cm 사인, 전신은 spine_03 ±0.006rad 로 숨쉰다. 뗀 순간 ABP 가 `AN_Farmer_Swing` 을 치켜든 자세(저작 t=0.12 — **지금
  임포트된 0.583초 클립에서는 0.15초**)부터 이어 틀면 이음새가 없다. `Tools/Blender/animate_farmer_windup.py`
  (`animate_farmer_swing.py` 의 `KEYS`·`SPINE` 을 import — 그 스크립트는 본문을 `main()` 아래로 옮겼다).
- **먹기** `AN_Farmer_Eat`·`AN_Farmer_FPEat`: 우클 길게(Dish 먹기)의 표현. Small 들기 자세(`farmer_hold_spec.json` Small)에서
  오른손이 입 앞으로 갔다가(0→0.25초) 머물고(0.25~0.35초) 돌아오는(→0.6초) **0.6초 원샷**(19프레임, 선형 보간). 시작·끝 =
  HoldSmall 이라 레이어 이음새가 없다. 전신은 손목이 머리 뼈 앞 24cm·입 높이(1.57m)에서 손가락이 얼굴을 향하고, 1인칭은 손이
  화면 아래 오른쪽에서 올라와 눈높이 바로 아래 27cm 앞(렌즈 앞)에 선다. `Tools/Blender/animate_farmer_eat.py`(키는 manifest
  `eat_keys`).
- 공통: `--export` 가 FBX·manifest(`animations` 4항목: 길이·fps·loop·sha256)·blend 를 쓴다. `farmer_pose.py` 에 1인칭 미리보기
  `render_fp`(팔 메시만 그린다 — 025 의 `_fp_fp.png` 가 빈 원인은 몸 메시의 턱·가슴이 손을 가린 것)와 "액션이 붙은 채 렌더하면
  미리보기 장면 프레임으로 리그가 평가되는" 문제의 수정(`preview.frame_set`)을 넣었다. 검증 `Tools/Blender/verify_farmer_clip_fbx.py`
  (FBX 재읽기: 30fps·1.0초/0.6초·루프 이음새 0). 미리보기 `preview/windup_{tp,fp}_f{1,8,16}_*.png`·`eat_{tp,fp}_f{1,5,9}_*.png`.
  UE 임포트 `Tools/Unreal/import_farmer_clips.py`(manifest 기반 공용 — 이름 목록 인자/`CL9_FARMER_CLIPS`, 기본 이번 4클립, sha 검사,
  길이 불일치 경고, 증거 `ue_import` 절 추가). [028 증거](../evidence/028-farmer-windup-eat.json). ABP 배선은
  [../../systems/animation.md](../../systems/animation.md) "공격 대기·먹기 가지".
- **그립 역산** `Tools/Blender/measure_item_grip.py`: 고기 3상태·프라이팬의 `GripOffset`/`GripRotation` 후보를 계산한다
  (Blender 리그에는 `HandGrip_R` 소켓이 없어 기존 Large 행 실측으로 소켓 자세를 되살리고 손전등 행으로 검산 — 오차 4.9°).
  손잡이(고기 뼈 +X 21cm·팬 손잡이 −X 18cm) 끝에서 4cm 안쪽이 손바닥 점, 몸통은 정면에서 15° 위로. 값·대안(너클 축 정합)은
  [028-item-grip](../evidence/028-item-grip.json) 과 실행 보고서. 표에 넣고 PIE A5 로 판정하는 것은 리드.
- 리그 실측 `Tools/Blender/probe_farmer_rig.py`(fps·뼈 위치·손 축, `-- --fpcam` 이면 1인칭 카메라 시험 렌더).

## 029 1인칭 빈손 팔 클립 + 손전등 자세 재조정 (2026-09-13 밤, fp-arms-flashlight)

사용자 "리썰처럼 최초 화면에선 안 보이다가, 뛸 때 손을 흔들면서 뛰는 애니메이션으로 손을 자연스럽게 보여주자. 플래시를 들면 손을
조금 뻗어서 플래시가 보이게". 1인칭 팔 클립만 만들었다 — 전신(남이 보는 것)은 템플릿 로코모션 그대로다.

- **화면 모델**: 카메라 눈높이 1.56m, `FirstPersonFieldOfView` 70(가로)·16:9 → 세로 반각 21.5°. `FirstPersonScale` 0.6 은 카메라 원점
  기준 축소라 각도(화면 위치)는 그대로다. 손목이 카메라에서 θ° 아래면 화면 세로 `0.5 + 0.5·tanθ/tan21.5°`. 미리보기
  `farmer_pose.render_fp`(4:3·10° 내려다봄, 아래 반각 37.7°)는 세로로 더 너그러워서 미리보기가 비면 게임에서도 빈다.
- **`AN_Farmer_FPArmsDown`** — 빈손 서기·걷기의 기본(`AN_Farmer_FPHoldIdle` 대체). 두 팔을 거의 편 채(어깨~손목 0.50m, 팔꿈치 ~130°)
  몸 옆·살짝 앞으로 늘어뜨린 자세, 손목 [±0.26, −0.16, 0.97]m = 카메라 75° 아래 — 피치 0 에서 프레임 밖, 38° 넘게 내려다봐야 손이
  들어온다. 2.0초 루프, 손목 z ±0.5cm 사인 숨쉬기, 손바닥은 허벅지 쪽. 미리보기 `fparmsdown_f{1,16}_fp.png` 빈 프레임.
- **`AN_Farmer_FPRun`** — 달리기 팔 흔들기 **0.6초 루프**(19프레임, 첫=끝), manifest `reference_speed_cm_s: 654`(RunSpeed) —
  ABP 가 `PlayRate = Speed / 654` 로 튼다. 손목이 어깨를 축으로 반지름 0.35m 호 위를 θ −25°(뒤)↔92°(앞·위) 사인 왕복, 왼팔은
  반대 위상. 앞 극점 손목 [−0.20, −0.32, 1.44] = 카메라 20° 아래(화면 세로 0.96, 아래 가장자리) — 주먹이 그 위 0.72~0.95, 가로
  0.80 에서 오른쪽 아래로 올라왔다 내려간다(왼 주먹은 왼쪽 아래). 뒤 극점은 카메라 뒤. 주먹 curl 1.1rad·엄지 0.6, 손바닥
  안쪽·아래(카메라엔 손등·엄지). 미리보기 `fprun_f{1,5,10,14}_fp.png`(f1·f10 빈 프레임, f5 오른 주먹, f14 왼 주먹).
- 두 클립 `Tools/Blender/animate_farmer_fparms.py`(키는 manifest `fparms_keys`). 손바닥 방향은 `set_palms` 가 실측한다 —
  가운뎃손가락을 굽혔을 때 둘째 마디가 움직이는 쪽을 손바닥으로 보고 손 뼈를 손가락 축 둘레로 돌린다(`farmer_pose.aim` 의
  roll=0 은 최소 회전이라 손이 아래를 향하면 손바닥이 제멋대로였고, 왼손은 뼈 축이 거울상이라 국부 축 부호를 믿을 수 없다).
- **손전등 자세** `farmer_hold_spec.json` `Flashlight.fp` 손목 [−0.15, −0.42, 1.42] → **[−0.13, −0.50, 1.45]**(앞 +8cm·안쪽 2cm·위 3cm,
  어깨~손목 0.53m, 팔꿈치 ~148°): 카메라 18.4° 아래(화면 세로 0.92, 가장자리) → 12.4°(0.78). `animate_farmer_hold.py -- --export
  --only Flashlight` 로 `AN_Farmer_FPHoldFlashlight`·`AN_Farmer_HoldFlashlight`(전신은 값 변경 없음 — 같은 스크립트가 쌍으로 뽑아
  sha 만 바뀜)만 재출력. hold 스크립트에 넣은 것: `--only <범주>` 필터(다른 범주 FBX·manifest 항목 보존 — FBX 에 저장 시각이 박혀
  같은 값이라도 sha 가 바뀌므로), 스테이징 fps 를 저작 장면과 맞춤(025 의 24fps → 30fps, UE 길이 2.5초 → 2.0초, 루프라 무해), 앞
  클립의 액션을 떼고 나서 자세를 주는 수정(액션이 붙은 채 렌더하면 tp 자세가 fp 미리보기·결과 수치에 섞였다), 1인칭 미리보기를
  `farmer_pose.render_fp` 로 교체(025 의 빈 `_fp_fp.png` 해결).
- 검증 `verify_farmer_clip_fbx.py`(기본 목록 = 이번 4클립): 전부 30fps, 2.0/0.6/2.0/2.0초, 루프 이음새 0, sha 일치. UE 임포트
  `Tools/Unreal/import_farmer_clips.py`(기본 목록·증거 경로를 029 로) → `/Game/Characters/FarmerA/Anims`.
  [029 증거](../evidence/029-farmer-fparms.json). ABP 배선은 [../../systems/animation.md](../../systems/animation.md)
  "1인칭 팔 표시 규칙".
- 하지 않은 것: 전신 달리기 팔 클립, 다른 Hold 범주 재출력, 걷기 전용 팔 클립(걷기는 FPArmsDown — 기본값 19).
