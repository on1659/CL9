# 농부 A — 작업복 풀바디와 1인칭 팔

상태: 게임 연결 검수됨 — 2026-09-11 `CharacterTable.farmer_a` `SkeletalMesh`/`ArmsMesh` → `SK_Farmer_A_Body`/`_Arms`; 클립 전신 12·1인칭 12 + `ABP_FP_View`(09-14) 임포트, 032 PIE(넘어짐·던지기 수치); 09-21 처형당함 전신 2클립 제작(034 절 — UE 임포트·ABP 는 리드 대기). 4슬롯 `TintColor`·2인·LOD 미검수 · 작성일: 2026-09-11 · 변경일: 2026-09-21

현재 행 farmer_a의 Manny/SK_Mannequin·ABP_Unarmed 계약을 유지한다. 1인칭 팔 ABP 는 `Content/Characters/FarmerA/Anims/ABP_FP_View`
(2026-09-13 밤 신설 — 카메라 자식 뷰모델, 031 절; 옛 `ABP_FP_Copy`·`CtrlRig_FPWarp` 는 2026-09-14 삭제).
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
- 몸과 팔은 같은 관절/장갑 원본을 공유하되 별도 메시로 납품한다. 팔은 2026-09-13 부터 카메라 자식 뷰모델(`ABP_FP_View`, 031 절)이라
  몸 포즈를 복사하지 않지만 같은89뼈 계약을 유지하고 시점 가시성을 실제 UE에서 확인한다(024 당시는 Copy Pose 방식이었다).

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

## 031 1인칭 뷰모델 — 카메라 로컬 클립 8종 재저작 + 전신 Swing 30fps 재수출 (2026-09-13 저녁, fp-viewmodel)

사용자 "애니메이션이 부자연스러워 — 억지로 팔을 들고 있는 모습", "위아래를 볼 때 손이 움직이는 거랑 시야가 이상해 — 리썰 조작감으로".
답: 리썰 방식으로 전환 — 팔 메시가 **카메라 컴포넌트의 자식**(상대 (0,0,−EyeHeight)·yaw −90, gameplay)이 되고 몸 포즈 복사·FPWarp 는
폐기, 1인칭 클립은 전부 다시 만들었다. 이름은 그대로 덮어썼다.

- **저작 규약(카메라 로컬)** — `Tools/Blender/farmer_pose.py` 머리말이 정본. 클립 공간 = 리그 root 원점, 눈 (0,0,1.56m)에서 −Y(Blender)/
  +X(UE)를 보는 공간. 피치·요는 카메라가 통째로 돌리므로 **피치 0 고정 프레이밍**으로만 저작한다. 몸 포즈 복사·척추 레이어가 없으니
  **클립 하나가 전 뼈를 소유**한다(root·pelvis·spine identity, `export_action` 이 전 뼈를 굽는다). 숨쉬기는 척추가 아니라 손목 z 사인
  (`farmer_pose.breathe`, Hold ±0.5cm). manifest `animations[*]` 에 `space: "camera"`·`eye_height_m: 1.56`·`layer: "full rig (camera-local
  view model, spine identity)"`. 손바닥 방향은 스펙의 `palm`(월드 방향)으로 실측 정렬한다(`align_palm` — 029 의 `set_palms` 를 공용화).
- **미리보기·화면 모델** — `render_fp(tag, dir, pitch_deg=0)` 는 게임과 같은 카메라(1280×720·가로 FOV 70 → 세로 반각 21.5°·눈 1.56m·
  근접 5cm/0.6 = 8.3cm)로 팔 메시만 그린다(옛 4:3·10° 내려다봄 폐기). `fp_screen_uv(point) -> (u, v) | None`(왼쪽 위 0,0, 프레임 밖·카메라
  뒤면 None)이 카메라 모델을 코드로 갖고 있어 아래 수치는 전부 실측이다(`fp_pose_report`: 손목·너클·손가락 끝 uv, 팔꿈치 각, 손목 꺾임).
  FirstPersonScale 0.6 은 카메라 원점 기준 닮음 변환이라 화면상 위치·크기가 그대로다.
- **Hold 3범주** `farmer_hold_spec.json` `fp`(`tp` 무변경, `Idle` 항목 삭제) — 리썰식 느슨한 자세: 팔꿈치 ~90°, 아래팔이 우하단 구석에서
  안쪽으로 비스듬히 올라오고 주먹만 화면에 든다. 손등·엄지가 카메라를 향한다(1차 미리보기에서 손바닥이 보여 `palm` 을 안쪽·살짝 아래로 돌림).
  주먹 위치는 brief 의 u 0.72~0.80 이 아니라 리드가 전달한 사용자 피드백(기준 캡처 `preview/my/1.png` — 우하단 구석 u 0.86~0.92·v 0.85~0.95,
  Large 0.28/0.72·v ~0.9)을 따랐다.
  · `AN_Farmer_FPHoldSmall`: 손목 [−0.24, −0.36, 1.415]m(눈 앞 36cm·아래 14.5cm·오른쪽 24cm — 프레임 바로 밖 구석), 주먹이 구석에서 안으로
    들어와 너클 (0.83, 0.83)·손가락 끝 (0.79, 0.96)·주먹 중심 ≈ (0.90, 0.92), curl 1.1. brief 초안의 "아래 25~30cm" 는 깊이 40cm 에서 32° 아래 =
    프레임 밖이라 화면 uv 를 기준으로 잡았다.
  · `AN_Farmer_FPHoldFlashlight`: 같은 높이·구석 [−0.24, −0.35, 1.415](너클 (0.81, 0.87)), 꽉 쥔 주먹(curl 1.4), 팔 안 뻗음(029 "뻗기" 폐기).
    렌즈 정면은 표 행이 맞춘다(아래 그립).
  · `AN_Farmer_FPHoldLarge`: 두 손목 [±0.15, −0.36, 1.44](30cm 간격), 손목 (0.20, 0.92)/(0.80, 0.92)·너클 (0.33, 0.89)/(0.67, 0.89) — 주먹 중심
    ≈ 0.28/0.72·v 0.9, 두 팔꿈치 90°.
  `animate_farmer_hold.py -- --export --fp-only` — 전신 FBX 는 sha 까지 그대로. 스크립트의 중복 IK 함수는 `farmer_pose` 공용으로 통일.
- **`AN_Farmer_FPArmsDown`** 029 설계 그대로 재수출(카메라 자식이라 피치와 무관하게 항상 프레임 밖). **`AN_Farmer_FPRun`** 호 설계 유지·
  `reference_speed_cm_s` 654 — 게임과 같은 카메라로 다시 보니 앞 극점 주먹이 우하단 모서리에 걸쳐 잘려(u 0.76~0.94·v 0.73~0.96) 앞 극점
  92→100°·앞 손목 바깥 거리 0.20→0.14m 로 조정, 이제 주먹이 u 0.65~0.82·v 0.58~0.77 에 온전히 든다.
- **`AN_Farmer_FPSwing`·`AN_Farmer_Swing`** 0.467초(30fps 15프레임) 원샷. fp 키는 시작·끝을 `Small.fp` 에서 읽고(Small 이 바뀌면 자동
  재키잉) 치켜든 자세(t=0.12, 손목 [−0.17, −0.34, 1.57], 주먹 우상단 u 0.75~0.86·v 0.22~0.46)·내려친 자세(t=0.25, 손목 [−0.05, −0.40, 1.42],
  화면 아래 중앙 가장자리)는 스크립트 상수. 전신은 026 키 그대로를 30fps 로 재수출(옛 0.583초 폐기) → 두 ABP 모두 대기→휘두르기 오프셋
  0.12·평가 범위 0.347. **`AN_Farmer_FPSwingWindup`** 1.0초 루프 = 위 치켜든 자세 + 손목 ±1cm. **`AN_Farmer_FPEat`** 0.6초 원샷 = 새 Small →
  입 앞(손목 [−0.05, −0.27, 1.49], 렌즈 앞 27cm 중앙, 너클 (0.59, 0.48)) → Small. windup·eat 은 `--fp-only`(전신 무변경).
- **일괄 수출** `py -3 Tools/Blender/export_farmer_fp_clips.py` — hold·fparms·swing·windup·eat·verify 를 순서대로(모두 manifest·blend 를 쓰므로
  병렬 금지). 검증 `verify_farmer_clip_fbx.py`(기본 9클립): 전부 30fps, 길이 일치, 루프 이음새 0, sha 일치. UE 임포트
  `Tools/Unreal/import_farmer_clips.py`(기본 목록 = 9클립, 증거 `ue_import` 절) → `/Game/Characters/FarmerA/Anims`.
  [031 증거](../evidence/031-farmer-fp-viewmodel.json). ABP 는 리드의 `ABP_FP_View` — [../../systems/animation.md](../../systems/animation.md)
  "1인칭 팔".
- **그립 검산** `measure_item_grip.py`(→ [031-item-grip](../evidence/031-item-grip.json)): 소켓 보정(hand_r 대비 소켓 자세, 리그 상수)은
  028 당시 옛 1인칭 자세를 스크립트에 박아 되살리고(손전등 옛 행 검산 3.4°) 새 자세에 상수만 적용한다. 결과 — 손전등: 옛 행이면 새 자세에서
  렌즈가 30.9° 빗나가므로 `GripRotation (−17.8, 136.8, 69.3) → (−29.8, 105.5, 34.6)`; 두 손 물건(냄비·작업등): 손 간격 44→30cm 라
  `GripOffset (12.3, −5.4, 17.3) → (11.5, 1.6, 6.7)`·`GripRotation (1.6, 81.6, 51.8) → (4.9, 112.4, 26.8)`; 고기·팬은 `forward`(몸통 정면 15° 위)/
  `knuckle`(너클 축) 두 후보 — 표 적용·PIE A5 판정은 리드.
- 미리보기(게이트 15장): `hold_{Small,Flashlight,Large}_fp_fp.png`·`fparmsdown_f1_fp.png`·`fprun_f{1,5,10,14}_fp.png`·`windup_fp_f1_fp.png`·
  `swing_fp_f{1,5,9}_fp.png`·`eat_fp_f{1,5,9}_fp.png`(+ `_front`·`_hand` 보조).
- 하지 않은 것: manifest `animations` 의 HoldIdle/FPHoldIdle 항목·Idle FBX·미리보기 삭제(리드 판단), 전신 Hold·SwingWindup·Eat 재수출.

## 032 휘두르기 재작업 + 넘어짐·던지기 클립 (2026-09-15, 애니메이션 보강)

사용자 "피격·공격 애님이 부족하다, 무기 공격이 어색하다". 분석: 휘두르기는 키 4개 직선 보간에 치켜든 자세가 옆으로 벌어진 팔(웨이터 팔), 타격이 허리 옆,
몸통 정지가 원인. 넘어짐·죽음·던지기 클립은 0개였다(코드는 넘어지면 이동 불가 + 카메라 하강만). 만든 것(전부 30fps, `Tools/Blender/farmer_pose.py` 헬퍼):

- **휘두르기 재작업** `animate_farmer_swing.py` — 키 6개(HoldSmall → 치켜듦 0.12 → 타격 직전 0.18 → 타격 0.25 → 팔로스루 0.31 → HoldSmall 0.467)에
  구간별 완급(치켜듦까지 inout·타격까지 in·타격 linear·뒤 out3·복귀 inout). 전신은 손목이 오른쪽 어깨 위·뒤(−0.30, 0.10, 1.80)로 올라갔다가
  몸 앞(−0.15, −0.50, 1.05)으로 뻗고, 몸통이 오른쪽으로 비틀며 젖혔다가(어깨 12.5cm 뒤) 왼쪽으로 비틀며 숙인다(10.8cm 앞) — 척추 3마디 분배, 머리는
  반대로 돌려 정면 유지, 왼팔은 뒤로 흔든다. 시작·끝은 `farmer_hold_spec.json Small.tp` 그대로(옛 026 키는 HoldSmall 과 11cm 어긋나 가지 전환 때 튀었다).
  팔 길이(0.55m)를 넘는 중간 프레임은 어깨 기준 0.54m 로 클램프(`clamp_reach`). 1인칭은 치켜든 주먹이 우상단(너클 (0.86, 0.17)) → 화면 중앙 오른쪽을
  지나(f7 너클 (0.66, 0.50)) → 아래 중앙 가장자리(타격, 손목 v 0.98) → 왼쪽 아래로 빠졌다가 바닥 가장자리를 따라 Small 로 돌아온다. **ABP 상수
  (오프셋 0.12·범위 0.347) 불변**. 공격 대기(`animate_farmer_windup.py`)는 새 치켜든 자세로 전신·1인칭 둘 다 재수출.
  축 실측(`_measure_axes`): Manny 척추 뼈 국부 x = 비틀기, y = 옆 기울기(+ 왼쪽), z = 앞뒤(+ 뒤로) — 옛 스크립트의 `spine_03 [0.04,0,0]` "앞으로 기울기"는
  실제로는 비틀기였다.
- **넘어짐 3+2클립** `animate_farmer_knockdown.py` — 전신 `AN_Farmer_Knockdown`(0.5초: 맞아 젖혀짐 0.12 → 중력처럼 가속해 등으로 넘어짐 0.45 → 착지),
  `AN_Farmer_Down`(2초 루프: 등을 대고 누움, 골반 z 0.12, 왼 무릎 세움, 팔 벌림, 숨쉬기 척추 ±0.03), `AN_Farmer_GetUp`(0.6초: 상체 일으킴 0.3 → 쭈그림 0.48 →
  기준 자세). 전신 골반 위치 키 포함(`key_current_pose(locations=('pelvis',))` 신설 인자) — 로코메션·발 IK 를 통째로 대체하는 가지에만 쓴다. 바닥 관통은 몸
  메시 최저점으로 골반을 올려 잡는다(최저점 +0.5cm). 1인칭 `AN_Farmer_FPKnockdown`(0.5초: 두 손이 얼굴 앞으로 확 올라왔다 빠짐)·`AN_Farmer_FPGetUp`
  (0.6초: 두 손바닥이 아래에서 밀어 올리듯). 카메라 하강은 이미 C++ Tick 이 한다(DownedZ, 6/s 보간).
- **던지기 2+2클립** `animate_farmer_throw.py` — 전신 `AN_Farmer_ThrowWindup`(1초 루프, 오른손을 어깨 뒤로 당김·왼손 앞으로 조준·손목 ±1cm)·
  `AN_Farmer_Throw`(0.333초 = 11프레임: 앞·위로 뻗어 놓고(0.15) 팔로스루 뒤 **기준 자세(identity)로 끝난다** — 던지면 손이 비어 HoldPose=None 이라
  레이어 가중치가 빠질 때 튀지 않게). 1인칭 `AN_Farmer_FPThrowWindup`(주먹이 오른쪽 가장자리)·`AN_Farmer_FPThrow`(화면 중앙 위로 뻗으며 손이 펴지고 아래로).
- 검증 `verify_farmer_clip_fbx.py -- <13클립>`: 전부 30fps·길이 일치·sha 일치, 루프 이음새 ≤0.01cm. 미리보기 시트(`Tools/Blender/contact_sheet.py` 신설):
  `preview/swing_{tp,fp}_strip.png`·`swing_{tp,fp}.gif`·`{knockdown,down,getup,throwwindup,throw}_sheet.png`·`fp{knockdown,getup,throwwindup,throw}_sheet.png`.
  전신 미리보기 카메라는 `render_front(cam_location=, look_at=)` 인자로 오른쪽 앞(휘두르는 팔 쪽)에서 찍는다.
- **함정(2026-09-15 PIE 실측)**: `farmer_pose.export_action` 은 리그를 100배(cm) 스테이징 사본에 얹어 굽는데, 뼈 **위치** 키(넘어짐의 pelvis)는 m 단위
  국부 이동이라 그대로 얹으면 100배 작게(78cm → 0.78cm) 나갔다 — 회전만 쓰던 기존 클립엔 없던 문제. 위치 커브만 100배로 환산한 임시 액션으로 굽도록
  고쳤고(회전만인 클립은 sha 불변), 재수출 뒤 PIE 에서 골반이 84 → 18cm 로 내려갔다 일어나는 것을 확인했다(`docs/asset/evidence/032-anim-pie.json`,
  `Tools/Unreal/qa_anim_pass_pie.py` — 넘어짐·던지기 대기/던지기·몬스터 경직 변수 전달을 수치로 잰다; 자동 PIE 가 3fps 라 0.3초 경직·팬 밀침의 육안 확인은 사용자 몫).
- UE 임포트(`Tools/Unreal/import_farmer_clips_032.py`, 13클립, 증거 [032-farmer-anim-pass.json](../evidence/032-farmer-anim-pass.json); PIE 시각 확인
  [032-anim-visual.json](../evidence/032-anim-visual.json)·`032-anim-pie-down.png`·`032-anim-pie-visual.png`)·ABP 가지·C++ 복제값은
  [../../systems/animation.md](../../systems/animation.md) "넘어짐 가지"·"던지기 가지" — 같은 태스크의 ④ 단계.

## 034 처형당함 클립 — 종별 2인 클립의 농부 짝 (2026-09-21, execute-clips)

사용자 "몬스터 제작 목록에 공격 모션 — 처형과 공격은 같은 모션". 처형 종(돼지·너구리)의 `AN_<종>_Execute` 와 짝이 되는 전신 2클립, 각 2.0초 = 행
`ExecuteDuration`(초안 — `Tools/Blender/execute_anchor.py` `DURATION`, balance 가 바꾸면 그 값만 고쳐 다시 굽는다). `animate_farmer_executed.py`
(`animate_farmer_knockdown.py` 의 `apply_full`·`lerp_params`·`STAND/HIT/LAND/DOWN` 을 import, 30fps, 골반 위치 키 포함, 바닥 관통은 몸 메시 최저점으로 보정
+0.5cm). 두 리그는 각자 자기 blend 에서 저작하고 **액터 공간 앵커 표 `execute_anchor.py`**(원점 = 피해자 캡슐 중심 바닥, +X 피해자 전방 = 리그 −Y,
몬스터 자리·접촉 시각·구간)만 공유한다(goal D7). 피해자 요는 처형 진입 때 서버가 맞춰 주므로 "정면"·"등 뒤"를 그대로 가정했다(gameplay 인터페이스).

- **`AN_Farmer_Executed_Pig`** — 정면(+100) 박치기. 0~0.40 선 채(카메라 블렌드 0.3 동안 피해자는 못 본다) → 0.50 두 손을 가슴 앞으로 반쯤 올려 막으려
  함(BRACE 신설: 손목 (∓0.28, −0.40, 1.15), 척추 +0.05, 무릎 0.10) → **접촉 0.50** → 0.62 척추 뒤로 젖혀지고 팔이 앞으로 튐(032 의 HIT) → 뒤로(−X 로 머리)
  넘어져 1.10 LAND → 1.40 DOWN → 2.0 까지 DOWN 유지. **끝 = `AN_Farmer_Down` 첫 프레임과 같은 자세**(같은 DOWN 키·숨쉬기 0 — 보통 죽음 시체와 같은
  그림). 완급 linear·out·out·in(중력)·out·linear. 끝 프레임 실측: 골반 (0, −2.3, 17.7)cm, 머리 y +64(리그 +Y = 액터 −X).
- **`AN_Farmer_Executed_Raccoon`** — 등 뒤(−120) 덮침. 0~0.55 선 채 → **접촉 0.60** → 0.72 척추 앞으로 −0.40rad, 팔 뒤·옆(손목 (∓0.45, +0.25, 1.05)),
  무릎 0.45 꺾여 골반 0.80(HIT_FRONT 신설) → 1.00 앞으로 엎어지는 중(FALL_FRONT: 골반 피치 −1.0·z 0.45, 팔 앞·아래) → 1.25 **PRONE 신설** → 2.0 까지 유지.
  PRONE = 골반 피치 −π/2(엎드림), 얼굴 옆으로(head_yaw 0.80, 머리 +0.15·척추 +0.10 으로 얼굴이 바닥에 박히지 않게), 오른팔 옆(−0.55, −0.60, 0.05)·왼팔
  앞(0.30, −0.90, 0.05), 다리 거의 펴짐(무릎 0.05/0.15), 골반 z 초안 0.12 → 바닥 보정 19.5cm. 끝 프레임 실측: 머리 y −68.5(액터 +X — "앞으로 머리"), z 27.
- 농부 골반 XY 는 두 클립 모두 0(`apply_full` 관례 — 규칙 3, 데스캠이 캡슐 중심을 본다). 마지막 프레임은 부활까지 유지된다 — `ABP_Unarmed` Executed 가지
  Run 2 사양(`IsExecuted` 단독 조건 + `ExecutedByPig` 로 두 클립 중 하나, `ExplicitTime = ExecuteElapsed`)은 [../../systems/animation.md](../../systems/animation.md).
- 산출: `export/AN_Farmer_Executed_Pig.fbx`(sha256 `1312e420…`)·`AN_Farmer_Executed_Raccoon.fbx`(`c1eb2954…`), 61프레임·30fps, `verify_farmer_clip_fbx.py`
  fps 30·길이 2.0·sha 일치·75뼈. manifest `animations` 2항목(`end_pose`·`execute_anchor` 포함) + `executed_keys`(brace·hit_front·fall_front·prone), blend 재저장.
  미리보기 `preview/executed_{pig,raccoon}_f{01,10,16|19,22|25,28,34,43,52,61}_front.png` + `executed_{pig,raccoon}_sheet.png`(카메라 (−2.9, −2.7, 1.9) →
  (0, 0, 0.8) — 선 머리와 누운 몸이 다 들어온다).
- UE 임포트 `Tools/Unreal/import_farmer_clips_034.py`(2클립, 증거 [034-farmer-executed.json](../evidence/034-farmer-executed.json) — 리드가 채운다),
  에셋 `/Game/Characters/FarmerA/Anims/AN_Farmer_Executed_{Pig,Raccoon}`. 몬스터 짝은 [../monster/pig_a.md](../monster/pig_a.md)·[../monster/raccoon_a.md](../monster/raccoon_a.md).

## v2 귀여움 1단계 — 얼굴·눈·채도 (2026-09-30, farmer-cute-eyes, 사용자 승인 전)

[../design-concept.md](../design-concept.md) 의 "귀여움은 비율이 아니라 형태·색·표정에서만" 을 따른 첫 재작업이다. 실물 `farmer_a.blend`·`export/`·UE 는
건드리지 않았고, 결과는 `ArtSource/character/farmer_a_v2/` 에 따로 있다. 모든 수치는 렌더 옵션이며 확정은 사용자가 한다.

- **스크립트** `Tools/Blender/cute_farmer_a_v2.py`(실물 blend 를 열어 읽기만 하고 `save_as_mainfile(copy=True)` 로 v2 에 쓴다). 얼굴 구조 실측은
  `Tools/Blender/probe_farmer_face.py`(읽기 전용)로 했다. 실물 얼굴은 머리 뼈 가중치 1인 섬들로 되어 있다: 얼굴 로프트(96정점, 초타원 지수 .85), 귀 2, 검은 상자 눈 2(1.9×1.3cm),
  갈색 상자 눈썹 2, 코·콧수염·아랫입술 상자.
- **비율 불변**: 몸 치수가 111.35×43.05×184.4cm 로 실물과 같다. 모자·장갑·장화 섬과 리그·클립도 그대로다. 몸은 4,100 → 4,588 tris, 2,148 → 2,404 정점이다.
  재질 슬롯 3개(Cloth·Skin·Detail)는 순서까지 그대로다.
- **얼굴**: 로프트 지수를 .85 에서 1.0(타원)으로 바꿨고, 턱 끝 링은 (5.2, 4.4) → (5.8, 4.8)cm 로 넓혔다. 코는 60% 구형화한 뒤 얼굴 쪽 뒷면을 기준으로
  (.9, .8, .85)배 줄였다. 콧수염은 크기는 그대로 두고 45% 구형화했다.
- **눈**: 옛 눈·눈썹 상자를 지우고 흰자(반지름 1.55×1.85cm)·동공(1.18×1.48cm)·하이라이트(0.36cm, 두 눈 모두 +X 위)를 저폴리 타원체로 붙였다.
  위치는 (±3.6, 171.6cm)로, 원본(±3.2, 172.3)보다 조금 벌리고 낮췄다. 눈썹은 짧은 타원체(1.35×0.36cm)로 174.6cm 에 두었다. 볼 홍조(1.2×0.75cm, ±5.7, 168.8)를
  새로 더했다. 위치는 얼굴 표면에 레이캐스트해 잡고, 방향은 표면 법선에 맞춘다.
- **눈 2단**(셰이프 키): `Basis` 는 평소 표정이다. 피격 후보는 둘이다 — `Eye_Hurt_A` 는 흰자와 동공을 세로 0.5배로 찡그리고 안쪽을 0.12rad 올린다. `Eye_Hurt_B` 는 흰자를 세로 0.8배로 줄이고
  동공을 0.55배로 줄이며 하이라이트를 숨긴다. 두 후보 모두 눈썹 안쪽 끝을 0.38rad 올리고, 눈썹을 0.3cm 위·0.15cm 안쪽으로 옮긴다. 셰이프 키는 FBX 에서 모프 타깃으로 나간다.
  HitReact 와 어떻게 이을지는 2단계에서 정한다.
- **채도**(선형 RGB): 작업복 기본 틴트(Cloth 재질 곱)를 (.13, .20, .08) → (.07, .25, .04)로 바꿨다. 소매 끝·모자 띠·주머니 입구 녹색에는 휘도를 유지한 채 채도 1.5배를,
  장화·장갑·띠·버클 등 나머지 유채색에는 1.2배를 줬다. 피부는 1.15배로 (.48, .30, .19) → (.50, .30, .17)이 됐다. 몸·팔 정점색을 같은 규격(256×8, 32칸)의
  `farmer_a_v2/textures/T_Farmer_Palette_v2.png`(20색)로 다시 굽고 UV1 을 다시 이었다. 전체 색 대응은 `farmer_a_v2/measure.json` 에 있다.
- **렌더**(레스트 포즈): `preview/compare_full.png` 는 실물 | v2 전신이다. `neutral.png` 와 같은 무대(카메라 (2.1, −3.1, 1.8)·정사영 3.05·Key/Fill)에서 실물도 다시 찍었다.
  `preview/compare_eyes.png` 는 실물 | 평소 | 피격 A | 피격 B 얼굴 근접이다(85mm 원근 + 얼굴 보조광). 개별 컷은 `preview/full_*.png`·`face_*.png` 이다.
- **v2 blend**: `SK_Farmer_A_Body` 가 v2 메시이고, 실물 메시는 `REF_Original_SK_Farmer_A_Body` 로 숨겨 보관했다. `SK_Farmer_A_Arms` 는 형태는 그대로 두고 색만 바꿨다.
  리그는 레스트 포즈다.
- 다음(2단계, 승인 후): 선택한 옵션 확정, FBX 수출(모프 타깃 포함), UE 재임포트, 틴트 기본값·4슬롯 `TintColor` 적용 확인, 피격 눈 연결.
