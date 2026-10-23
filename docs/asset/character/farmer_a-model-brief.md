# farmer_a 새 외형 제작 브리프

## 2026-10-22 앉기 v04 — 계란 형태 유지/다리 접기 모션 수정 완료

- 사용자 피드백: v03의 압축된 몸통이 너무 납작하므로 계란 모양은 유지하고 다리를 접는다. v03 압축 방식은 `superseded_squashed_body`로 최신 카드에서 제외했다. 이번 범위는 기존7종 모션 수정과 저장된 계란 시험 연결 교체다. 새 실제 플레이 실행은 아래처럼 구분한다.
- 현재 납품: `ArtSource/character/farmer_a/egg_crouch_v04/`의 `EggCook_Crouch_v04.blend`, `manifest.json`, `delivery.json`, `export/AN_Egg_Crouch*.fbx`, `index.html`. 원본은 동일한 `cart_grip_v02/farmer_cart_grip_v02.blend`, RIG_RoundCook_Body 88뼈/50메시. 앉기 시작0.2초/일어서기0.3초/앉은대기/전후좌우보행 총7종142프레임.
- 포즈: torso chain 압축·scale·몸 기울임을 제거했다. 골반14.5cm 하강/뒤로3.5cm, 발앞4.5cm/좌우1.5cm, 무릎 굽힘 약33.7→89.6도. 발목/무릎 IK는 원래 다리 길이 안에 있다. 몸 높이는160→145.5cm로 실제 짧은 다리가 접히는 범위만 낮춘다. 기존v03의51cm하강 수치는 이 버전에 적용하지 않는다.
- 형태 검사: 몸통·얼굴·모자·앞치마28개 보호 메시의 모든 평가 정점을 전체142프레임에서 비교. 골반 이동 성분을 뺀 비강체 변형 최대0.000023cm, 배–바닥 여유최소2.93cm (`rigid_egg_qa.json`). 원본 정점/토폴로지/가중치/재질/레스트 계층 동일 (`source_integrity_qa.json`). 전체142프레임88관절 FBX 재열기·루프·접지·도달범위 검사 통과.
- 시각 검수: Start/F/L/End 짧은 시험 후, 최종 정면·측면·사선 전체 프레임 순서 시트7개 검토. CPU Cycles 렌더21개 정상속도MP4와 `preview/egg_crouch_motion_review_1x.mp4` 전체 디코드 통과. 실제 Blender 렌더 비교 `preview/egg_stand_crouch_comparison.png`. 프레임 시트 검토를 연속 영상 시청으로 표현하지 않는다.
- 엔진: `/Game/Characters/FarmerA/EggCrouchV04/Build_<hash>/`에7클립, 기존 계란 메시/전용 Skeleton에서 전체 압축 포즈 비교 통과 (`engine_import.json`). GripV02 원본은 보존하고 새 `ABP_EggCrouchV04_Body_Fold`를 생성·컴파일·정확한 패키지만 저장했다. 카트 IK 유지, native IsCrouched의 Idle/WalkF 및0.2/0.3초 포즈 블렌드를 계승한다. `round_cook_trial` 행의 AnimBP만 새 클래스로 교체·저장했다 (`runtime_connection.json`). 시작/끝 클립 자체를 재생하는 상태머신이나 다른방향 분기를 새로 완료한 것은 아니다.
- **실제 플레이 범위:** 기존 카메라/입력 코드는 변경하지 않았다. v03에서 확인했던 카메라156→90cm 처리는 유지하며, v04에서는 다른 작업 PIE를 중단/재시작/변경하지 않았다. 저장 변경 전후 활성 AnimInstance 클래스·PIE 상태·에디터 월드 동일을 검사했다. 현재 v04의 새 실제 플레이/2인/낮은 틈 통과는 미검수다. 기존 앉은 캡슐 전체높이110cm는 수정하지 않았고, 압축하지 않은 시각 높이145.5cm이므로 낮은 공간의 메시–충돌 여유는 별도 검수/설계가 필요하다. v03 실제 플레이 영상을 v04 증거로 복사하지 않았다.
- 페이지: 리소스 갤러리 최신 항목을 계란형 유지v04로 교체, 구v03에 대체 링크 고지. `viewer_qa.json`: HTTP43파일/7클립/21검색항목/JS문법/영상Range206 통과. 새 페이지는 Blender 모션과 저장된 엔진 연결 범위를 표시한다.
- 재개: `animate_egg_crouch_v02.py`에 `CL9_EGG_CROUCH_VERSION=egg_crouch_v04`, 공통render/QA/encode에는 `CL9_ANIM_PACK_ROOT=ArtSource/character/farmer_a/egg_crouch_v04`. `verify_egg_shape_v04.py`, `Tools/Assets/finalize_egg_crouch_v04.py`. 엔진 `egg_crouch_v04.py`는 `Saved/PlayerCrouch/egg_v04_stage.txt`의 import/connect/setup/test 한 단계를 실행한다. 최신 saved-row 연결 스크립트 `Saved/PlayerCrouch/egg_v04_connect_row.py`는 활성 PIE를 보존한다. 렌더 로그 `Saved/PlayerCrouch/egg_v04_final_render.log` 완료, 본인 임대·잠금·대기열 해제, 실행 중 본인 작업 없음.

## 2026-10-22 앉기 v03 — 몸통 압축 피드백으로 대체됨

- 사용자 피드백: v02는 단순 고개 숙이기로 보였고, 몸이 아래로 빠르게 내려가며 시야도 낮아지는 앉기가 필요하다. v02는 `superseded_head_bow` 이력이며 현재 결과는 `egg_crouch_v03`이다. 계란 외형 자체는 같은 승인 원본을 사용한다.
- 저작: `cart_grip_v02/farmer_cart_grip_v02.blend`에서 전용88리그/50메시를 유지했다. 골반9.5cm하강 + torso chain의 위치 압축50%, pitch2도. 뼈 scale=1, rest/가중치/정점/재질 참조 동일. 모자상단 약160→109cm, 발은 접지, 배–바닥 여유 유지. Start0.2초/End0.3초와 대기/전후좌우 보행 총7종/30fps142프레임. `EggCook_Crouch_v03.blend`, `export/AN_Egg_Crouch*.fbx`.
- 검수: 짧은 Start/F/L/End 시험에서 구조 확인 후 전체142프레임·88뼈 FBX 왕복/원본 무결성 검사, 정면·측면·사선 전체 프레임 시트 검토. CPU Cycles 원본 재질 영상21개 및 이어보기 전체 디코드 통과. `source_integrity_qa.json`, `fbx_qa.json`, `preview/motion_qa.json`, `preview/video_qa.json`. 프레임 시트와 연속 영상 시청을 구분한다.
- 엔진: `/Game/Characters/FarmerA/EggCrouchV03/Build_<hash>/` 7클립, 기존 계란 GripV02 메시/전용 Skeleton에서 전체 압축 포즈 비교 통과(`engine_import.json`). 원래 GripV02 AnimBP는 보존하고 `ABP_EggCrouchV03_Body_Low`를 복제하여 native IsCrouched로 새 낮은 포즈를 선택한다. 기존 수레 IK/Event 실행은 유지. `round_cook_trial` 행만 새 ABP로 연결, 기본 farmer_a 외형/AnimBP는 교체하지 않았다.
- 실제 시야/입력: 기존 C++ Crouch/UnCrouch/캡슐/눈높이를 재사용했다. 앉을 때만 `CL9Feel::CrouchCameraInterp=18`로 빠르게 낮추고 넘어짐/사망의6은 유지한다. IA_Crouch(Boolean), IMC_Gameplay의 LeftControl 매핑, 기본 캐릭터와 계란 BP의 CrouchAction 연결. 기존 매핑은 보존하며 UE5.8의 DefaultKeyMappings에서 저장값을 확인했다(폐기된 Mappings 배열은 검사에 사용하지 않음). 빌드 브로커 성공 로그 `build-20261022-113922.log`.
- PIE: 저장된 `QA_EggCrouch`에서 Enhanced Input action 주입이 실제 Started/Completed 바인딩을 통과하도록 확인했다. 657개 실시간 표본에서 카메라156→90cm, 캡슐 반높이90→55cm, head하강약52cm, 시선pitch0도 유지. 카메라는 native crouch 시작 후 약0.15초에 목표2cm 이내에 도달했다. 두 번 앉기/입력 해제 복귀·짧은 전진 확인. `ingame_qa.json`, `native_crouch_qa.json`. 실제 OS 키보드를 눌렀다는 주장이 아닌 Enhanced Input 봇 입력이며 LeftControl 매핑은 별도 저장 검증이다.
- 영상/페이지: `preview/egg_crouch_ingame_1x.mp4` 11초 실제 PIE 실시간 캡처. 시험 위치 이동/몬스터 제거/봇 입력 고지. 3인칭과1인칭의 서기–앉기–복귀 대표6장 시각 확인, 전체MP4디코드 통과. 21개 Blender 영상·FBX·원본 링크는 `index.html`, 리소스 갤러리 최신 카드는 v03 실제 PIE 영상으로 교체. `viewer_qa.json` HTTP42링크/JS/Range206 통과.
- **검증 범위:** 계란 시험 캐릭터 연결이다. 현재 게임 AnimBP의 앉은 보행 분기는 Idle/WalkF이며 후진·횡이동 클립은 납품했으나 방향 분기 연결은 별도다. 진입/이탈은0.2/0.3초 포즈 블렌드이며 별도 Start/End 시퀀스를 재생하는 상태머신은 아니다. 전속도 보행 접지/2인 복제/장애물 아래 일어서기 추가 검수는 미실행. 이미 존재하는 native 앉기 기능 전체를 이번에 새로 구현했다고 보고하지 않는다.
- 종료: 원래 Lvl_Flat 및 플레이 설정 복원, 본인 PIE 종료·잠금/임대 해제, 실행 중 작업 없음. 재개는 `animate_egg_crouch_v02.py`에 `CL9_EGG_CROUCH_VERSION=egg_crouch_v03`, 공통 렌더/QA/인코딩에는 `CL9_ANIM_PACK_ROOT=ArtSource/character/farmer_a/egg_crouch_v03`. 엔진 import/connect/setup/test/run 도구는 `Tools/Unreal/*egg_crouch_v03.py`, 납품은 `Tools/Assets/finalize_egg_crouch_v03.py`.

## 2026-10-22 계란형 요리사 앉기 v02 — 고개 숙이기 피드백으로 대체됨

- 사용자 정정과 현재 기준: 플레이어는 채택된 계란형 요리사다. 옛 농부/Manny로 만든 `crouch_v01`은 대상 선택 오류로 최신 목록에서 제외했고 이력 파일은 보존했다. 앞으로 이 요청의 현재 원본/검수는 `egg_crouch_v02`를 사용한다.
- 편집 입력: 아래 10-17 소매 수정까지 포함한 `cart_grip_v02/farmer_cart_grip_v02.blend`, 전용 `RIG_RoundCook_Body`, 메시50개. 원본 SHA256과 모든 메시 정점/토폴로지/가중치/재질 참조·뼈대 레스트/부모 서명이 납품본과 동일하다(`source_integrity_qa.json`). 몸·눈·모자·앞치마를 한 덩어리로 기울이고 실제 짧은 다리 길이 안에서 디딤을 저작했다. 사람형 클립을 그대로 붙인 결과가 아니다.
- 납품: `ArtSource/character/farmer_a/egg_crouch_v02/`의 `EggCook_Crouch_v02.blend`, `manifest.json`, `delivery.json`, `index.html`, `export/AN_Egg_Crouch*.fbx`. 앉기 시작·앉은 대기·전후좌우 이동·일어나기 7종, 30fps 총163프레임. 대기는 미세한 무게 이동, 보행은 양발 교대/이중 지지, 기준15.625cm/s 제자리 클립이다.
- 검사: 전체163프레임88관절 FBX 재열기, 고정 관절 길이/발 접지/루프, 배–바닥 여유, 좌우 발 교차를 검사했다. 초기 척추 굽힘에서 생긴 앞치마 관통을 계란 몸 전체의 동일 회전으로 고쳤고 전체 보폭을 실제 다리 길이 안으로 줄였다. `fbx_qa.json`, `preview/motion_qa.json`.
- 엔진: 기존 계란 `/Game/Characters/FarmerA/RoundCookTrialV01/Body/SK_RoundCook_Body_Skeleton`에 Interchange로7종 임포트·저장. `/Game/Characters/FarmerA/EggCrouchV02/Build_<source hash>/`에서 기존 계란 메시의 압축 포즈를 전체 프레임/88관절 비교했다(`engine_import.json`). 옛 농부 Skeleton으로 검증한 것이 아니다.
- 화면/영상: 정면·측면·사선 CPU Cycles 렌더, 전체 프레임 순서 시트 확인. 정상속도21개 MP4와 이어보기 `preview/egg_crouch_motion_review_1x.mp4`는 제작 방식·버전·1배속 표시 및 전체 디코드 통과(`preview/video_qa.json`). 프레임 시트 검토와 연속 동영상 시청을 구분한다. 비교 이미지 `preview/egg_stand_crouch_comparison.png`. 리소스 갤러리 최신 항목과 검색을 계란 결과로 교체했다.
- **실제 조작은 미연결:** 이번 범위는 계란 캐릭터 모션 재제작과 UE 임포트다. 앉기 입력/캡슐/카메라/이동 속도/AnimBP 상태·실제 2인 플레이는 미구현/미검수로 남긴다. 기존 게임 데이터/실행 중 PIE/승인 원본을 변경하지 않았다. 임포트용 에디터 임대는 해제했다.
- 재개: `Tools/Blender/animate_egg_crouch_v02.py`(`--only <clip>`), `verify_egg_crouch_v02.py`, `verify_egg_source_integrity.py`, `render_egg_crouch_v02.py`. 엔진 `Tools/Unreal/import_egg_crouch_v02.py`. 검수/인코딩/페이지/완료 기록은 `Tools/Assets/*egg_crouch_v02.py`. 최종 로그 `Saved/PlayerCrouch/egg_final_render.log`. 실행 중 작업 없음.

## 2026-10-17 수레 손 접촉 수정 v02

사용자가 v01의 손이 수레를 잡지 않는 문제를 지적하여 실제 손잡이 추종으로 수정했다. `scope: production`의 기존 시험 맵 개선이며 원래 farmer_a 기본 캐릭터 교체는 하지 않았다.

- 저장된 시험 맵은 동일한 `RoundCookTrialV01/QA_CartTrial`. `round_cook_trial` 행을 `ABP_GripV02_Body/Arms`, `GripV02/Body/SK_GripV02_Body`, `GripV02/Arms/SK_GripV02_Arms`로 연결했다. 저장 에셋으로 PIE를 다시 시작해 확인했다.
- 실제 수레 루트 메시 `Grip` 소켓의 월드 위치·forward·right에서 양손 목표를 매 프레임 계산한다. 손목 오프셋은 손잡이에서 미는 사람 쪽7cm, 좌우30cm. `TwoBoneIK`로 전신/1인칭 손목을 추종한다. 전신 어깨를 앞쪽44cm·좌우35cm·높이96cm로 자세 보정하고 팔꿈치를 안쪽으로 정리했다. 전신 손목은 하박과 상대 회전을 유지하고, 그 옵션이 되돌리는 손목 위치를 마지막 월드 위치 보정으로 목표에 다시 고정한다. 팔 리치 보정에는 최대2배 길이 보정을 허용하므로 승인된 기본 대기 외형과 밀기 자세를 구분한다.
- 기존 소매는 단일 뼈를 따르는 분리 조각이라 IK 길이 보정 시 장갑과 틈이 생겼다. 연결된 상박/하박 튜브와 양끝 뼈 사이의 연속 스킨 웨이트로 교체했다. 원본은 보존하고 `ArtSource/character/farmer_a/cart_grip_v02/farmer_cart_grip_v02.blend`, Body.fbx, Arms.fbx로 저장했다. 몸통·얼굴·다리 외형은 유지했다.
- 전신과 1인칭 팔 리그의 좌우가 반대인 점을 실측하여 FP의 양손 목표/팔꿈치 방향을 반대로 연결했다. 시험 BP의 FP 렌더 타입을 None으로 바꿔 전용 FOV/scale 보정으로 월드 수레와 손의 화면 투영이 어긋나는 문제를 없앴다. 카메라 FOV/캡슐/이동/수레 물리 수치는 바꾸지 않았다.
- 상태는 HoldPose enum 스위치의 None + CanSwitchSlot=false일 때만 활성화한다. Large 물건과 혼동하던 v01 조건을 수정했다. 조준 중 수레 참조를 저장하고 미는 동안에는 다른 조준 대상으로 덮어쓰지 않는다. 소유 플레이어의 Focus 기반이므로 네트워크 원격 캐릭터 지원은 별도 구현/검수 대상이다.
- 전신/FP 새 메시의 기존 스켈레톤 재사용 확인. 재질 참조가 재시작 후 WorldGrid로 돌아가는 저장 문제도 강제 저장으로 수정했다. 제작 스크립트: `fix_farmer_grip_sleeves_v02.py`, `import_farmer_grip_v02.py`, `connect_farmer_grip_v02.py`, `attach_farmer_grip_ik_v02.py`, `persist_farmer_fp_render.py`, `align_farmer_grip_wrists.py`.
- 영상: `cart_grip_v02/cart_grip_review.mp4`, 16초 / 20fps / 1280×820. 실제 PIE 창 실시간 캡처, 봇 이동/시선 및 실제 E 함수. 시험 배치·보조 조명·몬스터 제거는 자막 고지. 최종 입력은 `frames_contact/`, `record_start_contact.json`, `ingame_qa_contact.json`; 중간 frames/ 및 frames_final/는 최종 손목 위치 보정 전이다.
- 최종 검수: 전진·회전·정지·1인칭·E 놓기. 65개 표본에서 전신/FP 손목 목표 오차 <0.1cm, 잡기 유지와 놓은 뒤 PushAlpha=0 확인 (`contact_qa.json`). 영상 전체 ffmpeg 디코딩 성공, 측면/1인칭 대표 프레임 시각 검토. PIE 종료·한국어 및 원래 Lvl_Flat 복원·본인 잠금 해제 완료. 로그의 손목과 목표 위치 일치는 IK 추종 검사이며 손가락 삼각형 접촉/관통 전체 합격을 뜻하지 않는다. 단순 장갑의 감싸는 세부 모양, 급회전/좁은 통로/멀티플레이는 추가 검수 범위다. v01의 약50cm 허공 밀기 결함과 FP 교차/소매 단절은 이번 수정 대상으로 처리했다.

## 2026-10-17 실제 수레 인게임 적용 시험 — 접촉 불합격, 시험 맵 제공

사용자 요청대로 우리 실제 수레와 새 둥근 캐릭터를 Unreal PIE에 적용하여 확인했다. 기본 캐릭터 교체나 최종 밀기 완성이 아닌 플레이 가능한 별도 시험이다.

- 맵 `/Game/Characters/FarmerA/RoundCookTrialV01/QA_CartTrial`, 캐릭터 행 `round_cook_trial`, `BP_RoundCookTrial`, `GM_RoundCookTrial`. 기존 farmer_a 행과 캡슐·속도·CartMult·수레 물리는 유지했다. 원래 Lvl_Flat의 게임 모드 및 시험 중 추가한 수레는 원상 복구했고 시험 맵에만 추가 수레를 저장했다.
- 실제 `/Game/Items/BP_CL9Cart`, `SM_Cart_A`: 폭97.2cm, 손잡이 지상 약100cm. 목표 거리 캡슐39 + GripDistance60 = 중심에서99cm. 현 밀기 모션 손목은 앞49cm·높이74cm 부근이므로 손잡이 접촉 불합격. 단순 팔 벌어짐보다 앞뒤 도달 거리와 높이가 먼저 해결되어야 한다.
- 전신 AnimBP의 대기/걷기/밀기 블렌드 및 실제 E 잡기·놓기, 이동 입력으로 수레 전진·회전 확인. 전신 PushAlpha가 잡은 중1, 놓은 뒤0으로 바뀌는 로그 보존. **현재 시험에서는 CanSwitchSlot=false를 밀기 조건으로 사용하므로 Large 물건 운반과 구분하지 못한다. 빈손 수레 시험 전용이며 기본 게임 적용 금지.** 시도했던 GetPullingCart BlueprintPure 소스 변경은 되돌렸다. C++ 재빌드 없음.
- 팔 AnimBP는 기존 별도 팔 루프만 적용. 1인칭 손잡이 IK와 밀기 전용 팔 자세 미구현. 손이 화면 하단에 보이지만 손잡이를 잡지 않는다. 바닥 줍기 연결·멀티플레이·좁은 통로 전체 QA 미실행.
- 초기 EventGraph DSL의 enum 비교 타입 오류로 연결이 생성되지 않은 것을 런타임 PushAlpha=0 및 노드 수2로 검출했다. 외부 MCP 오류를 확인하고 시험 조건으로 수정, 노드 수16 및 실행 변수 확인. VertexColor의 잘못된 RGB 출력명도 기본 출력으로 수정하여 실제 색 복원. 저장된 수정 에셋으로 PIE 재시작 확인.
- 납품 영상 `ArtSource/character/farmer_a/cart_ingame_v01/cart_ingame_review.mp4`, 16초 / 1280×820 / 20fps. 실제 창 실시간 캡처 타임스탬프에 맞춰 1배속 합성. 봇 이동/시선 + 실제 E 함수. 시험 배치·카메라·보조 조명·몬스터 제거를 자막 고지. 원래 몬스터와 UI가 가린 초기 촬영은 납품에서 제외.
- 최종 촬영 입력 `frames4/`, `record_start4.json`, 로그 `ingame_qa4.json`, 합성 `Tools/Film/package_farmer_cart_trial.py`. 이전 frames/frames2/frames3는 중간 실패 또는 검토본이다. 샘플 FBX·Blender 작업은 같은 폴더 export/와 farmer_cart_ingame_v01.blend.
- 다음 작업: 수레의 실제 손잡이에 맞춘 접촉 설계(짧은 팔의 도달 거리 문제를 승인 비율과 함께 해결), 팔꿈치 폭 축소 및 양손 IK, 정확한 수레 상태 노출/블렌드, 실제 수레 전진·급회전·좁은 길·1인칭 재검수. 이번 결과를 조작감 합격 또는 완성 애니메이션으로 표시하지 않는다.

## 2026-10-17 VanillaLoop 무료 샘플 적용 시험 — 현재 작업

- 사용자 요청: 무료 샘플을 받아 기존 부드러운 캐릭터에 줍기·밀기를 적용하고 영상으로 확인. `scope: production` 중 **두 동작의 Blender 리타기팅 시험**이며 UE 게임 연결 요청으로 확대하지 않음. 사용자 EULA 수락 승인 후 Fab 다운로드 완료.
- 출처: [VanillaLoop Free Sample Animation Set](https://www.fab.com/listings/5b737d10-b4cd-4294-8f6b-757b9088039d). Fab에서 제공한 `freesampleanimationset.unitypackage` 3,405,500 bytes 안의 원본 FBX 사용. Unreal 라이브러리 다운로드 대신 공개 다운로드 포맷에 포함된 FBX를 추출했으며 Unity 프로젝트를 만들지 않음.
- 작업 폴더: `ArtSource/character/farmer_a/free_sample_probe_v01/`. 원본 패키지·추출 FBX는 `source/`, 편집 납품본은 `farmer_pickup_push_v01.blend`. 기존 soft_round_v01 원본은 덮어쓰지 않음.
- 줍기: `A_ItemPickup_fromIdle_RH_100cm.fbx`, 25fps / 50 재생 프레임 / 2.0초. **선반 높이 물건 집기**이며 바닥 줍기가 아님. 16프레임에서 시험 소품 접촉·부착.
- 밀기: `A_Push_Heavy_01_Loop.fbx`, 25fps / 55 재생 프레임 / 2.2초. 원본 root-motion 루프 1회와 시험용 카트.
- 적용: 원본 몸·다리 회전 리타기팅, 몸통 회전 완화, 골반 이동 축소, 프레임별 발 높이 보정. 짧은 팔에는 원본 줍기 타이밍과 경로를 바탕으로 두 관절 해석 보정 및 밀기 손 목표 적용. **기성 모션의 무보정 호환 결과가 아니며**, 손가락 관절은 기존 분리 장갑 구조를 유지.
- 짧은 접촉 시험에서 옆으로 빠지는 팔을 수정. 첫 소품은 장갑에 깊이 파고들어 폐기하고, 실제 평가된 손바닥 앞면을 기준으로 위치를 다시 맞춤. 구버전 root-level PNG와 `farmer_sample_probe.blend`, `farmer_sample_corrected.blend`는 중간 시험이며 납품본이 아님.
- 렌더 진행: `Tools/Blender/render_farmer_sample.py`, 로그 `render_final.log`. **최종 줍기 고정 그립은 `Tools/Blender/fix_farmer_sample_grip.py`, 로그 `grip_final.log`이며 해당 스크립트가 납품 blend를 마지막 저장함.** 사선·측면 640×480 CPU Cycles, 영상 조립 `Tools/Film/package_farmer_sample.py`, 로그 `package_final.log`. 최종 입력은 `pickup_fixed_*_frames/frame_*.png`와 `push_*_frames/frame_*.png`. 최종 영상은 `preview/pickup_1x.mp4`, `push_1x.mp4`, `pickup_push_review_1x.mp4` 예정.
- 검수: 저장 후 재열기 및 전체 105프레임 발 최저점 약 +0.2cm 확인. 집은 뒤 16–50프레임 물건 구 표면과 장갑 최근접 정점의 부호 거리 약 -0.464cm로 일정. 의도된 그립 접촉이며 삼각형 교차 전체 검사가 아님. 손바닥만 기준으로 삼으면 손가락과 겹치는 결함이 있어 오른손 장갑 전체 앞쪽 지지면으로 보정했고, 매 프레임 지지점을 재선정할 때 미끄러지는 문제는 접촉 시점의 hand_r 로컬 위치를 고정해서 해결. 접촉·시작·끝 사선/측면 스틸 확인. 전체 영상 디코딩은 렌더 완료 후 갱신. UE 임포트·AnimBP 연결·1인칭·멀티플레이 미검수.
- **완료:** 두 동작의 Blender 적용 시험·사선/측면 영상 납품 완료. 위 예정 경로의 MP4 3개 생성. 1280×568 / 25fps / 줍기 50프레임 2초 / 밀기 55프레임 2.2초 / 이어보기 105프레임 4.2초, 전체 디코딩 성공·drop/dup 0. 디코딩한 시작/접촉/끝 이미지 및 타이밍 시트 확인. `video_qa.json`, `final_geometry_qa.json`, `source_manifest.json` 보존. 기존 soft_round_v01 SHA256 동일. 손가락은 단순 장갑이며 밀기 바와 그립의 겹침이 있으므로 정밀 손가락 접촉/게임 최종 리그 합격으로 간주하지 않음. 현재 단계 `blender_motion_trial_delivered`; 실행 중 작업 없음.


## 2026-10-17 기존 둥근 요리사 표면 수정 — 현재 작업

### 실제 모델 수정 및 애니메이션 조사

사용자가 soft_round_v01 컨셉을 보고 “좋아 이 느낌으로 실제 모델도 부드럽게 수정해줘”라고 승인했다. `scope: model_only`, 컨셉 검토 완료. 아래 이미지 제안 단계보다 이 실행 기록이 최신이다.

- 실제 수정본: `ArtSource/character/farmer_a/soft_round_v01/farmer_a_soft_round_v01.blend`.
- 제작 스크립트: `Tools/Blender/soften_farmer_round_v01.py`; 검증: `Tools/Blender/verify_farmer_soft_round_v01.py`.
- 기존 rigged_walk 파일에서 이어 작업했다. 면별 색 차이 제거, smooth shading, 부위별 subdivision과 bevel, 무광 재질 적용. Bevel은 리그 변형 전에 평가하여 동작 중 토폴로지 변화 방지.
- 기존 88뼈 전신/팔 리그와 걷기 액션을 포함한다. 기존 승인 원본과 리깅 원본의 SHA256 보존 여부는 `soft_round_v01/model_qa.json`에 기록.
- 실제 Blender 렌더: `soft_round_v01/preview/`의 front/side/back/threequarter, fp_pan, walk_001/016/031. AI 이미지가 아니다. CPU Cycles로 렌더하여 공유 Unreal GPU 촬영과 분리했다.
- 외형용 평가 메시 약 44,848 tris, 약 깊이102.08 × 폭162.00 × 높이160.02cm. 원본 대비 subdivision 수축 약 1cm 이하. 게임 납품 예산/LOD 최적화는 별도이며 이번 수정본은 UE 미적용.
- 남은 사항: 장갑은 기존 분리 파츠 구조를 유지하며 손가락 접촉·최종 변형 토폴로지는 미검증. 전신 걷기 대표 3프레임은 시각 확인했지만 전체 동작 영상 검수와 UE 검수를 대체하지 않음.
- 최종 검증: 저장 후 재열기 성공, 기존 원본 2개의 해시 동일. 전신·FP 46프레임의 뼈 포즈와 원본 정점 가중치 서명 동일. 전신 루프 끝점 최대 차이 0.02171cm, 걷기 전체 부츠 최저점 +0.12149cm. 다각도 시트 `preview/model_review.png` 확인. 현재 `phase: blender_review`, 수정본 사용자 외형 검토 대기이며 게임 연결 완료 아님.

애니메이션 조사(2026-10-17, 로컬 실물 + 아래 공식 배포 페이지):

| 후보 | 확인한 것 | 이 캐릭터에 적용할 방법 |
|---|---|---|
| 기존 둥근 캐릭터 Walk | `blockout/rigged_walk`에 전신/FP 1.5초 30fps 걷기와 리그 존재 | 최우선 재사용. 이번 모델에도 같은 리그/액션 보존 |
| 기존 농부 26클립 | `Content/Characters/FarmerA/Anims/AN_Farmer_*`: FP 12개, 전신14개 | HoldSmall/Large/Flashlight, Swing, Throw, Eat, Knockdown/Down/GetUp 우선. 파일 존재와 새 체형 적용 완료는 다름 |
| 기존 마네킹 이동 | `Content/Characters/Mannequins/Anims/Unarmed`: Idle, 방향별 Walk/Jog, Jump/Fall/Land | 짧은 다리 보폭·골반 흔들림 조정. 새 레스트가 달라 직접 호환 아님 |
| [Mixamo](https://helpx.adobe.com/creative-cloud/faq/mixamo-faq.html) | Adobe ID로 무료, 상업 게임 사용 가능; 휴머노이드 전용이며 과장된 비율은 자동 리깅 실패 가능 | 기존 리그로 리타기팅하는 외부 후보. 걷기·달리기·겁먹은 대기 계열을 탐색하되 이번에 개별 클립 다운로드/적용하지 않음 |
| [Pick Up & Carry Animation Pack](https://www.fab.com/listings/e102d1e1-5ac5-4fb8-afe7-ddbe5bb16693) | 배포 설명상 65개, 무거운/중간/작은 물건의 집기·운반·내려놓기, 바닥/탁자/선반 변형 | 게임의 재료 집기·납품에 맞는 외부 후보. 짧은 팔과 몸통 간섭을 보정해야 함 |
| [Carrying Animation Bundle 1](https://www.fab.com/listings/22505e65-4df2-4aa2-8541-5612185a4049) | Manny UE5 기반 3개 운반 루프: 앞/머리 위/어깨, FBX | 앞에 안고 걷기 후보. 전용 집기·놓기 세트는 아님 |

추천: 기존 걷기 → 재료 안고 걷기 → 던지기/팬 휘두르기 → 넘어짐/일어남 순서로 짧게 체형 보정. 귀여운 뒤뚱거림·허둥대는 달리기는 이 체형에 맞춘 직접 키 보정이 필요하다. 외부 팩 가격·계정 소유권은 확인하지 않았고 구매·업로드·다운로드하지 않았다.

### 직전 이미지 제안 기록
### 2026-10-17 Fab·Mixamo 게임용 후보 추가 조사

**최신 사용자 정정:** 직접 썰고 볶는 게임이 아니므로 아래 Cooking/Chef 추천은 철회하고 조사 이력으로만 보존한다. 최신 우선순위는 운반·수레 밀기·이동·넘어짐이다.

후속 선정(공식 목록 확인, 적용·시연 전체 검수는 미실행):
- [VanillaLoop Free Sample Animation Set](https://www.fab.com/listings/5b737d10-b4cd-4294-8f6b-757b9088039d): 무료. 이동/Item Pickup/Push/Lift/Pull/DBNO 등 팩의 일부 샘플. 우리 체형 리타기팅 비교용 첫 후보.
- Pick Up & Carry: 재료 줍기·들고 걷기·탁자에 놓기. [제작자 시연](https://youtu.be/YoMmaY9EGXk).
- [Push Set](https://www.fab.com/listings/15f36675-213d-4f55-8ebf-cf481b242876): 허리/엉덩이 높이 및 어깨/머리 높이의 밀기 시작·전진 루프·정지·막힘. UE5 Mannequin, in-place/root-motion, FBX·IK 뼈 명시. 수레 손잡이에 맞출 후보이며 전용 수레 동작 검증은 아님.
- [Flashlight Set](https://www.fab.com/listings/efdc3a60-796c-4305-9205-8fc7bcfe489a): 손전등 장착·켜기/끄기·조준 Pitch/Yaw. 전신 후보이며 현재 1인칭 팔 카메라에 별도 조정 필요.
- Stylized Animations vol. 1: 걷기·겁먹기·낙하의 코미디 후보. 체형 적합성 미검증.
- 보류: [Loot Anim Set](https://www.fab.com/listings/ddfd5a02-98a0-4ab1-9834-f471e9ab65a4)은 상자·시체 수색 비중이 커 즉시 줍기에는 과할 수 있음. [DBNO Set](https://www.fab.com/listings/558ddaf4-13c5-4c29-bd1a-f122ce15c201)은 부상·기기·회복까지 96개 고유 동작으로 현재 넘어짐만 보강하기에는 범위가 큼. 기존 Mixamo Getting Up부터 시험.

사용자는 기존 게임 클립 목록보다 이전처럼 외부 기성 애니메이션을 찾아달라고 정정했다. 아래는 공식 상품 설명/목록 기준 추천이며 실제 시연 전체 시청·캐릭터 리타기팅 통과를 뜻하지 않는다. 구매·추가 다운로드는 미실행.

- **요리:** [Cooking/Chef Pack 1](https://www.fab.com/listings/784ce653-0957-43f2-8c8a-ef9ef3fdbdcd), 17개. 볼 섞기, 채소/고기 썰기, 팬 조리, 양념 등. [공식 시연](https://youtu.be/NQb8Sv-BdD4). 푸드트럭 조리 표현 후보이며 기존 자동 조리를 새 수동 조작으로 바꾸는 제안은 아님.
- **과장된 이동·공포 코미디:** [Stylized Animations vol. 1](https://www.fab.com/listings/d7ac3924-69c9-450b-9248-8bca96d233f2), 18트랙. Fast paced walk, Scared, Shortfall/Longfall, 4방향 걷기 등. [공식 시연](https://youtu.be/Nmcab_PRonE). SK_Mannequin 기반이며 큰 머리/가는 사지 체형을 고려했다고 명시. 우리 넓은 몸통/짧은 다리는 별도 보정 필요.
- **공포 반응:** [Scared - MoCap Animation Pack](https://www.fab.com/listings/740052ca-99f9-42c9-9f78-d20c800ace49), 60개로 설명. 겁먹은 대기·살금살금 이동·달리기·숨기, 루프/전환과 root-motion/in-place. 긴 연기는 잘라 쓰고 플레이 조작을 불필요하게 잠그지 않는 방향.
- **수레 밀기 보강:** [Push and Pull Animation Pack](https://www.fab.com/listings/bcddac3e-275c-43f3-b59c-2f4114e3df40), 48개. 밀기 시작/대기/전진/좌우/종료/걸림. 수레 전용 완성 시스템으로 확인한 것은 아니며 손잡이 IK·접촉 보정 필요.
- **줍기·운반·놓기:** 위 Pick Up & Carry 65개 팩. 실제 재료 회수/납품 연결에 가장 직접적인 후보.
- **이미 확보한 Mixamo:** `ArtSource/monster/storage_owner_disguise/mixamo_free_library_20261016/manifest.json`의 모션65개 모두 실제 파일 존재 확인. 플레이어 우선 시험 후보는 Picking Up, Picking Up Object, Lifting Object, Dropping, Getting Up From Back/Knockdown/Stomach. 괴물용으로 받았던 팩이며 전부 플레이어에게 맞는 것은 아님.

적용 제안 순서: 이미 받은 Mixamo 줍기/기상 짧은 시험 → 필요 시 Pick Up & Carry → Cooking/Chef → 코미디 이동/공포 반응. 가격·라이선스 등급·UE5.8 직접 호환은 이 조사에서 확정하지 않음. 새 캐릭터는 1인칭 팔과 동료 화면 전신을 각각 검수해야 함.

### 직전 이미지 제안 상세


사용자가 기존 컨셉과 v3 모델을 확인하고 “맞는데 너무 각짐”이라고 피드백했다.
직전 사람형 A/B/C 제안은 채택되지 않았으며 현재 작업 기준이 아니다. 기존 둥근 요리사의 비율·색·의상을 이어간다.

- 현재 단계: `concept_review`; 기존 외형의 각진 면과 윤곽을 부드럽게 하는 수정 후보 제시. 실제 모델 수정은 미실행.
- [부드러운 표면 시안](../../../ArtSource/character/farmer_a/soft_round_v01/concept.png), [프롬프트](../../../ArtSource/character/farmer_a/soft_round_v01/prompt.txt).
- 내장 image_gen으로 기존 `blockout/preview/body_contact.png`를 편집한 2D 이미지다. Blender 렌더/리그 검수 증거가 아니다.
- 몸·모자·장갑의 삼각면을 줄이고 둥근 윤곽과 부드러운 음영을 제안했다. 이미지의 신발 광택·둥근 밑창은 모델 확정사항이 아니며 실제 수정에서는 무광·발 접지 유지가 필요하다.
- 기존 v3 원본·리그·검증된 걷기는 보존. 다음 모델 작업은 별도 사본에서 실루엣 유지, 곡면 음영/윤곽 개선, 장갑 연결과 앞치마 천 형태를 확인한다.

## 2026-10-17 사람 캐릭터 재디자인 — 미채택 제안 이력

사용자 요청: “사람캐릭터를 우리 게임에 맞게 다시 만들자.” 이번 외형은 사람형으로 전환한다.
아래 비인간형 v3의 승인·리깅 시험은 과거 이력으로 보존하며 새 후보의 승인으로 사용하지 않는다.
기존 모델·리그·UE 연결은 이번 컨셉 작업에서 변경하지 않았다.

- 범위: `model_only`로 시작하는 외형 재제작. 현재 `concept_review`, `awaiting_user`, `review_policy: concept_and_model`. 리깅·전체 애니메이션·게임 교체 범위는 아직 확정하지 않음.
- 후보: [사람형 A/B/C 컨셉 시트](../../../ArtSource/character/farmer_a/human_redesign_v01/human_concepts_abc.png).
- 제작: 내장 image_gen, 신규 이미지 생성. [전체 프롬프트](../../../ArtSource/character/farmer_a/human_redesign_v01/prompt.txt). 실제 Blender 모델 렌더가 아님.
- 제안: A 듬직한 푸드트럭 직원 / B 길쭉하고 겁 많은 직원 / C 작고 넓은 운반 담당. 성인 사람 얼굴, 단순한 면, 큰 노란 장갑, 짧은 앞치마와 작업화로 게임 역할을 드러낸다. 추천은 A이며 사용자 선택은 미확정.
- 이미지 검토: 세 사람형의 실루엣·얼굴·의복과 A 앞/뒤 보조 뷰가 표시됨. 손 확대는 시각 제안이며 실제 손가락 구조·그립 검증은 아님. 성별·피부색·머리·최종 팔레트는 확정하지 않음.
- 다음: 사용자 외형 선택/수정 → 선택한 후보의 실제 Blender 초안과 전신·1인칭 팔 검토. 기존 캡슐·눈높이·리그 호환은 별도 검증하며 자동 변경하지 않음.

---

## 이전 비인간형 제작 이력

상태: v3 외형 사용자 채택 / 별도 전신·1인칭 팔 리깅 및 기존 Walk 리타기팅 시험 / Blender 렌더·가중치·루프 검수됨 / UE 연결 미실행
작성일: 2026-10-11 · 검토일: 2026-10-11
대상: 플레이어 `farmer_a`의 전신과 별도 1인칭 팔

## 2026-10-11 컨셉 대조 기준 보완 — 다음 수정 시 먼저 읽기

사용자가 전달한 피드백에 따라 [이미지별 형태 기준과 대조 절차](farmer_a-visual-reference.md)를 추가했다.
R1·R2는 전신, R3는 1인칭 손·팔의 **형태 기준**이며 색·분위기 참고만으로 사용하지 않는다.
아래 v1~v3는 제작 이력이다. v3 채택 및 걷기 시험 기록은 보존하지만 그 기록만으로 컨셉 일치를 보증하지 않는다.
특히 '단일 달걀형'과 '낮고 넓은 반죽'은 후보 제작에서 선택한 형태이며 이미지에서 확정된 해부 구조가 아니다.
정확한 머리·몸 분리, 손가락 수, 가려진 형태는 미확정으로 표시하고 제안과 관찰을 구분한다.
다음 외형 수정은 기존 원본을 보존한 별도 후보에서 수행한다. 이번 문서 보완에서는 모델·리그·UE를 수정하지 않았다.

아래 '첫 제작 세션'과 초기 산출물 표는 최초 작업 당시 범위다. 현재 존재하는 v3를 없는 것으로 취급하거나
초기 스크립트를 바로 재실행하지 말고, 최신 [세션 시작문](farmer_a-model-brief-astra-prompt.md)을 따른다.

## 2026-10-11 v3 채택 후 리깅·기존 Walk 시험

사용자가 v3 외형을 채택하고 기존 동작 하나의 변형 확인을 요청했다. 다른 리소스는 삭제하지 않는다.
승인된 blockout 원본을 보존하고 `blockout/rigged_walk/`에 별도 산출물을 만들었다.

- 원본: `ArtSource/character/farmer_a/blockout/rigged_walk/farmer_a_round_cook_rigged_walk.blend`
- 재실행: `Tools/Blender/rig_farmer_round_cook_walk.py` (Blender 실행, `--no-render` 지원)
- 재생 씬: `ROUNDCOOK_WALK_BODY`, `ROUNDCOOK_WALK_FP`. 타임라인1~45, 30fps, 1.5초.
- 실제 게임에서 추출해 보관하던 `ArtSource/character/_reference/existing_walk.fbx` 사용.
  출처는 `/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd`.
  SHA256 `ae3ca5375c99d6ca0e09d5acb9e76e9da8f87441062e7a851563eef260859869` 일치.
- 전신·별도 팔에 각각88뼈 리그를 만들고 이름·부모 계층을 복사하되 짧은 팔다리에 맞춰 레스트 위치를 조정했다.
  몸은 높이별 최대2가중치, 팔꿈치는2가중치 전이, 장갑·신발은 해당 손·발 뼈로 변형한다.
  손가락 세부 관절 변형은 이번 Walk 시험에서 확인하지 않았다.
- 기존 클립의 월드 회전 변화량을 새 레스트에 옮겨 별도 액션으로 베이크했다.
  골반 이동22%, 발 회전35%, 1인칭 팔 변화18%로 조정하고 1인칭 척추는 고정했다.
  프레임별 발 최저점으로 골반 높이를 보정했다. 사람이 걷는 큰 발 회전은 첫 렌더 확인 후 줄였다.
- 검수: 전신·팔 가중치 누락0, 정규화 오류0, 최대2영향.
  전신 루프 양 끝의 최대 정점 차0.02177cm, 팔0.00359cm.
  전신의 최대 인접 프레임 정점 이동8.843cm(장갑), 팔3.379cm.
  실제 EEVEE 전신·팔6단계 렌더를 직접 열어 확인했고 각각45프레임 MP4를 저장했다.
- `preview/body_walk.mp4`, `fp_walk.mp4`, `body_sheet.png`, `fp_sheet.png`.
  검사 수치는 `walk_rig_qa.json`, 기존 원본34파일 해시 동일 확인은 `preservation_check.json`.
  기존 farmer 원본·기존 FBX 클립·승인된 v3 원본·소품 원본은 변경하지 않았다.

**Blender 리깅·걷기 변형 시험 완료 / UE 임포트·실제 게임 이동 검수 미실행.**
새 레스트는 기존 Manny 레스트와 다르므로 이름이 같다고 기존 Skeleton에 바로 덮어 임포트하지 않는다.
게임 연결 시 별도 Skeleton·IK Retargeter 등으로 왕복 검수해야 한다. 현재 동작은 제자리 걷기이며
게임 이동 속도에 따른 발 미끄러짐, 동작 블렌드, 큰 관절 범위, 그립·손가락·충돌 간섭은 다음 검수다.
이번 작업에서 카메라·캡슐·CharacterTable·기존 UE 에셋 연결은 변경하지 않았다.

## 현재 후보 — 낮고 넓은 반죽 요리사 v3

2026-10-11 사용자 요청에 따라 계란으로 읽히는 실루엣을 줄였다. 몸통 높이를 줄이고,
윗면을 완만하게 누르며 아래 절반을 넓힌 부드러운 반죽 덩어리 형태다.
작은 걱정 표정, 세이지 색, 노란 장갑, 짧은 갈색 신발과 요리 모자는 참고 이미지에서 이어간다.
실제 계란 종족이라는 설정을 확정한 것은 아니다.

- 현재 원본·스크립트 경로는 아래 산출물 표와 같다. v2 원본·스크립트·렌더·실측·QA는
  `ArtSource/character/farmer_a/blockout/archive/round_cook_v2/`에 먼저 보존했다.
- 모자 포함 높이175→**161cm**, 팔 포함 폭**162.727cm**, 깊이**102.554cm**.
  몸통만 폭130.33×깊이92×높이110cm. 신발 간격을 넓히고 앞치마의 촘촘한 얼룩과 면별 색 차이를 줄였다.
- 전신48파츠 / 기본4,816tris / 의복 두께 포함6,592tris / 2재질.
  별도 팔은18파츠 / 2,856tris / 2재질이며 기존4개 정적 포즈를 유지했다.
- EEVEE 전신4각도·팔4포즈·크기 비교를 다시 렌더하고 전신·팔 모아보기를 직접 열어 확인했다.
  저장한 .blend를 다시 열어 전신과 팔4씬이 남아 있음을 확인했다.
- 재생성 전후 소유 오브젝트407→407, 전신 퇴화면0, 닫힌 전신 파츠 음수 부피0.
  바닥 최저Z=0, +X 정면·+Z 위·미터·바닥 원점 유지.
- **파일 생성 완료 / Blender 정적 시각 검수 완료 / UE 적용 미실행.**
  게임 카메라·충돌은 변경하지 않았다. 팔 포함 폭은 여전히 캡슐보다 넓으며,
  실제 동작·벽 간섭·게임 시점 검수와 리깅·UV·LOD는 후속 작업이다.

## 이전 후보 — 참고 이미지에 맞춘 v2 보존 이력

2026-10-11 사용자 피드백: 첫 Blender 후보가 다른 채팅에서 만든 이미지와 너무 다름.
「공포 요리 게임 콘셉트 잡기」 채팅의 생성 결과를 찾아 실제 이미지를 열어 확인하고 전신을 다시 제작했다.
이 절과 아래 v1 기록은 이전 후보 이력이며 **현재 v3 모델의 치수·디자인으로 읽지 않는다.**

참고한 이미지:

- [달걀 회수 장면 — 연두색 요리사](../../../output/concepts/2026-10-10-local-game-loop-45s-v1/03-eggraid-reference.png)
- [전투 장면 — 둥근 요리사 2명](../../../output/concepts/2026-10-10-combat-mockup-v1/combat-scene-mockup.png)
- [1인칭 장갑·소매](../../../output/concepts/2026-10-10-combat-mockup-v1/combat-scene-mockup-fps.png)

현재 원본은 기존 목표 경로 `ArtSource/character/farmer_a/blockout/farmer_a_round_cook_candidate.blend`이고,
제작 스크립트도 `Tools/Blender/build_farmer_round_cook_candidate.py`에 갱신했다.
수정 전 v1 원본·제작 스크립트·실측·검수·모든 렌더는
`ArtSource/character/farmer_a/blockout/archive/round_cook_v1/`에 보존했다.

| 항목 | 당시 v2 — 현재 디자인 지시 아님 |
|---|---|
| 큰 형태 | 당시 제작 해석: 머리·몸을 분리하지 않은 단일 달걀형 크리처, 아주 짧은 발·팔. 이미지로 구조가 확정됐다는 뜻은 아님 |
| 얼굴 | 작은 검은 동공·흰자·걱정하는 눈썹·작은 입, 코·주둥이·이빨 제거 |
| 색·의상 | 연두/세이지 몸, 노란 장갑, 흰 커프, 허리 앞치마, 뒤쪽 매듭, 짙은 갈색 신발 |
| 표면 | 몸은 삼각면과 제한된 면별 색 변화, 앞치마는 작은 때 자국. 최종 텍스처 베이크는 아님 |
| 폭×깊이×높이 | **162.727×96.699×175cm** (팔·장갑·모자·뒤 매듭 포함) |
| 몸의 주 덩어리 | 약120×88×124cm. 신장보다 둥근 실루엣이 우선 |
| 전신 파츠·삼각형·재질 | 48파츠 / 기본4,816tris / 의복 두께 평가 후 **6,592tris** / 2재질 |
| 팔 | 한 쌍18파츠 / 2,856tris / 2재질. 이전 4포즈 프레이밍을 유지하고 장갑·소매 색을 일치시킴 |
| 원점·축 | 바닥중앙(0,0,0), 최저 Z=0, +X 정면·+Z 위·미터 |
| 카메라·충돌 | 게임 설정 변경 없음. FP 비교는 가로70°·16:9·눈높이156cm·피치0° |

전신4장과 1인칭4장을 실제 EEVEE로 다시 렌더했고 `preview/body_contact.png`와
`preview/fp_contact.png`도 갱신했다. 첫 수정 렌더가 여전히 세로로 길어 몸을 넓고 짧게 다듬었고,
앞치마의 큰 삼각 얼룩은 작은 때 자국으로 줄였다. 눈썹을 걱정하는 방향으로 바꾸고 발바닥 접점을 평평하게 정리했다.
수정 렌더를 직접 열어 외형을 확인했다. 원본을 저장 후 다시 열어 전신·팔 후보가 남아 있음을 확인했다.
재실행 후보 오브젝트 수407→407로 중복이 없었고, 퇴화면0·닫힌 파츠 음수 부피0이다.

**제작됨 / Blender 정적 시각 검수됨 / UE 임포트·게임 연결 미실행.**
표정·그립은 정적 시안이며 리그·웨이트·동작·최종UV·LOD는 다음 단계다.
팔까지 포함한 폭은 캡슐보다 넓고, 유지한 카메라 눈높이는 캐릭터 얼굴보다 높다.
둥근 외형을 채택한 뒤 문틀·벽·시점·기존 동작 호환을 실제 게임에서 검수해야 한다.
외형에 맞추려고 판정·카메라를 자동 변경하지 않았다.

## 목표와 이번 작업의 끝

동물이 주인인 장소에서 재료를 훔쳐 푸드트럭에서 요리해 파는 협동 추출 게임의 플레이어를 새로 디자인한다.
사용자가 원하는 방향은 **동글동글한 비인간형 캐릭터, 저폴리, B급 공포 코미디, 1인칭 플레이**다.
성인형 사람 비율을 유지할 필요는 없다.

이번 제작 세션의 목표는 **직접 열어 수정할 수 있는 Blender 시안과 실제 모델에서 뽑은 비교 화면**이다.
문서를 다시 정리하거나 OBJ 파일을 생성하는 것만으로 이 단계를 완료하지 않는다.
리깅·UE 교체·전체 애니메이션 제작은 외형 시안 이후 단계다.

## 최초 브리프의 사용자 요구와 디자인 제안 — 이력

| 구분 | 내용 |
|---|---|
| 사용자 요구 | 작은 둥근 인상, 저폴리, 동물 몰래 재료를 가져오는 상황, B급 공포 코미디, 1인칭 |
| 이번 브리프의 제안 | 요리사 크리처, 큰 머리와 둥근 몸통, 짧은 팔다리, 장갑, 앞치마 |
| 아직 확정되지 않음 | 얼굴, 요리사 모자 채택, 색 조합, 최종 신장, 손가락 형태, 신체 비율 |
| 참고하는 방식 | 리썰의 제한된 시야와 긴장, R.E.P.O.의 과장된 형태와 동료 행동의 코미디 |
| 캐릭터 고유성 | 요리·재료 운반이라는 CL9 소재로 만들고 참고 게임 캐릭터 외형을 그대로 복제하지 않는다 |

시작안은 큰 덩어리 3~4개만으로 읽히는 실루엣이다. 얼굴은 어두운 공간에서 식별할 수 있을 정도로 단순화한다.
앞치마·장갑 등 역할을 알려주는 요소를 우선하고 작은 장식은 줄인다.
저폴리는 형태를 의도적으로 단순화하는 방향이다. 모든 면에 Flat Shading을 강제하지 않는다.
B급 감성은 엉뚱한 비율과 허둥대는 자세로 표현하며, 관통·뒤집힌 노멀 같은 제작 오류로 대신하지 않는다.

## 최초 검토 당시 OBJ의 실제 상태 — 이력

검토일에 OBJ/MTL 텍스트를 파싱한 결과다. Blender 렌더나 UE 실행 결과는 아니다.

| 항목 | 확인 결과 |
|---|---|
| 메시 | `ArtSource/character/farmer_a/blockout/farmer_a_round_cook_blockout.obj` |
| 재질 | 같은 폴더의 `farmer_a_round_cook_blockout.mtl` |
| 구성 | OBJ `o` 항목 21개, 정점 668개, 삼각형 1,252개 |
| 재질 | 사용·정의된 재질 이름 8개가 대응함. 실제 임포트 색상은 미검수 |
| 좌표 범위 | X −0.83776~0.83776 / Y −0.48750~0.40895 / Z −0.03000~2.83000 |
| 바운드 X×Y×Z | 1.67552 × 0.89645 × 2.86000 OBJ 좌표 단위 |
| 단위 | OBJ에는 물리 단위가 보장되지 않음. 제작 주석은 미터를 의도함 |
| 미터로 읽을 경우 | 폭 167.552cm / 깊이 89.645cm / 높이 286cm. 최저점은 바닥보다 3cm 아래 |
| UV / 저장 노멀 | `vt` 0개 / `vn` 0개. 노멀은 임포터에서 계산·검수해야 함 |
| 인덱스 | 모든 면이 삼각형이며 정점 인덱스 범위가 유효함 |
| 형상 방향 | 생성 코드의 의도는 −Y 정면, +Z 위. 임포트 후 확인 필요 |
| 리그·동작 | 없음. 독립된 1인칭 팔 납품본도 없음 |
| 최초 검토 당시 blockout 폴더 | OBJ·MTL 두 파일만 있었음. 이후 v1~v3 및 걷기 시험 산출물은 위 이력 참조 |

**현재 OBJ는 크기·원점이 정리되지 않은 형태 스케치다.** 286cm를 목표 신장으로 사용하지 않는다.
단순 부품을 조합한 메시이므로 관절 변형, 손가락 그립, 파츠 관통, 외형 품질은 검증되지 않았다.
외형이 목표에 맞지 않으면 이 OBJ를 유지하려고 애쓰지 말고 별도 후보를 새로 만든다.

## 기존 게임과 맞춰야 하는 조건

아래 값은 [기존 캐릭터 카드](farmer_a.md)와 [플레이어 규격](README.md)의 문서 기준이다.
이번 검토에서 실행 중인 게임이나 최신 코드 값을 다시 검증한 것은 아니다. 실제 연결 시 소비 코드·데이터와 대조한다.

- 전신과 1인칭 팔은 이미 별도 메시다. 전신은 동료에게 보이고, 팔은 카메라 자식 뷰모델이다.
  새 외형도 전신과 1인칭 팔을 각각 설계·검수한다. 팔 분리의 필요 여부를 처음부터 다시 결정하지 않는다.
- 기존 팔은 `ABP_FP_View`를 사용한다. 문서 기준 1인칭 FOV는 70°, 눈높이는 156cm다.
  Blender 비교 카메라에서는 화면비와 가로/세로 FOV 해석을 명시한다.
- 캡슐은 반경 39cm·반높이 90cm이며 전신 원점은 바닥 중앙이다.
  외형을 맞추기 위해 판정 크기나 눈높이를 자동으로 변경하지 않는다.
- 기존 모델의 약 183cm는 비교 기준이다. **성인형 신체 비율을 강제하는 조건이 아니다.**
  새 시안의 크기와 신체 비율을 각각 기록하고, 캡슐·눈높이 가이드를 함께 보여준다.
- 기존 Manny 계열 리그는 UE 기준 89뼈 계약이다. Blender에서는 88개 bone과 root 오브젝트로 표현된 이력이 있다.
  짧은 팔다리와 기존 레스트 포즈가 잘 맞는다고 가정하지 않는다. 관절 위치·웨이트·동작 시험 뒤 호환 여부를 판단한다.
- 공통 신규 모델 규격은 +X 정면이지만 기존 farmer 리그는 별도 축 계약을 따른다.
  공통 규칙만 보고 기존 리그나 완성 애니메이션을 90° 회전시키지 않는다.
- 플레이어 색 구분 영역을 확보한다. 현재 임시 8재질을 최종 슬롯 수로 확정하지 않고,
  [공통 규격](../common.md)의 예산과 실제 셰이더 구조에 맞춘다.

## 첫 제작 세션에서 할 일 — 최초 지시 이력

1. 작업 지침과 아래 근거 문서를 읽고 기존 Blender 파일·씬 상태를 확인한다.
2. 기존 OBJ와 현재 게임 캐릭터를 크기 가이드 옆에 놓고, 별도 후보에서 크기·바닥 원점·실루엣을 정리한다.
3. 큰 머리·짧은 팔다리·장갑·앞치마를 갖춘 전신 후보를 만든다.
   모자와 얼굴은 아직 디자인 제안이며 현재 임시 OBJ를 확정 디자인으로 취급하지 않는다.
4. 별도 1인칭 팔 시안을 만들고 빈손, 작은 재료, 손전등, 프라이팬을 든 구도를 만든다.
   가능하면 기존 소품을 사용하고, 없으면 크기가 명시된 대용물을 쓴다.
   정적 포즈 검수라면 애니메이션 검수로 보고하지 않는다.
5. 정면·측면·3/4 중립광과 어두운 손전등 조명 화면을 저장하고 직접 확인해 형태를 수정한다.
6. 아래 산출물을 저장하고 브리프에 실제 경로·치수·실행 결과를 기록한다.

손전등은 손과 렌즈 위치에서 광원이 읽히게 한다. 헤드램프는 시점 근처에서 전방을 비추는 방향으로 제안한다.
광원을 보여주기 위해 화면 위에서 아래로 떨어지는 별도 스포트라이트를 넣지는 않는다.
이것은 화면 연출 요구이며 새 조명 시스템 구현 완료를 뜻하지 않는다.

## 첫 단계 산출물과 완료 기준 — 최초 지시 이력

아래 경로는 **생성할 목표 경로**이며 현재 존재한다고 가정하지 않는다.

| 산출물 | 목표 |
|---|---|
| Blender 원본 | `ArtSource/character/farmer_a/blockout/farmer_a_round_cook_candidate.blend` |
| 재실행 가능한 제작 스크립트 | `Tools/Blender/build_farmer_round_cook_candidate.py` — 자신의 후보만 다루며 중복 생성 방지 |
| 전신 미리보기 | 같은 blockout 폴더의 `preview/body_front.png`, `body_side.png`, `body_threequarter.png`, `body_dark.png` |
| 1인칭 미리보기 | `preview/fp_empty.png`, `fp_small.png`, `fp_flashlight.png`, `fp_pan.png` |
| 검수 기록 | 이 문서에 실제 치수·파츠 수·삼각형·재질·카메라 조건·문제점 기록 |

- 원본을 다시 열었을 때 전신과 팔 후보가 남아 있고 수정 가능하다.
- 중립광에서도 형태가 읽히며, 발 접점·의복·손의 큰 관통 문제가 정리되어 있다.
- 1인칭에서 팔 절단면이 노출되지 않고 핵심 소품과 조준/상호작용 영역이 구분된다.
- 실제 모델 렌더를 직접 확인하고, 남은 문제를 기록했다.
- Blender가 연결되지 않았거나 렌더를 확인하지 못했다면 해당 단계는 미완료다.
  지난 세션의 연결 실패를 현재 상태로 단정하지 말고 다시 확인한다.

리깅과 게임 연결은 다음 단계다. 먼저 기존 동작 하나로 변형·축·크기를 검증한 뒤 전체 동작으로 넓힌다.
제작됨 → Blender 검수됨 → UE 임포트 검수됨 → 게임 연결 검수됨을 따로 기록한다.

## 2026-10-11 첫 제작 세션 결과 — v1 보존 이력

**이번 단계: 검토 가능한 모델 시안 제작·Blender 정적 시각 검수 완료.** 최종 외형 채택,
Manny 리그 호환, 애니메이션, UE 임포트 및 게임 연결은 완료하지 않았다.
처음 연결 확인은 실패했으나 사용자가 MCP를 켠 후 재시도하여 Blender **5.2.2 LTS**에 연결했다.
EEVEE 실제 렌더를 `look`으로 열어 확인하고 수정했다. 이미지 생성 AI나 그림 목업을 검수 근거로 쓰지 않았다.

### 원본과 보존

- [수정 가능한 Blender 후보](../../../ArtSource/character/farmer_a/blockout/farmer_a_round_cook_candidate.blend)
- [재실행 제작 스크립트](../../../Tools/Blender/build_farmer_round_cook_candidate.py)
- [실측 JSON](../../../ArtSource/character/farmer_a/blockout/candidate_metrics.json),
  [메시 검사 기록](../../../ArtSource/character/farmer_a/blockout/candidate_qa.json)
- 기존 `farmer_a.blend`·OBJ·MTL·소품 원본 및 UE 에셋은 변경하지 않았다.
  연결 당시 미저장 기본 씬(큐브·카메라·조명)은 후보 파일의 `Scene`에 남아 있다.
  작업 전 별도 사본은 `Saved/BlenderMCP/before_round_cook_20261011_032918.blend`이다.
- 후보만 소유 태그로 정리·재생성한다. 재실행 전후 후보 오브젝트 **413개 → 413개**, 후보 씬 **7개 → 7개**로 중복 없음 확인.
  이 수에는 조명·카메라·비교 가이드·기존 메시 복사본도 포함되며 전신의 메시 파츠 수와 다르다.
- 저장한 후보 `.blend`를 실제로 다시 열었다. 전신·어두운 전신·팔 4포즈·크기 비교 씬과 보존 씬이 남아 있음을 MCP로 확인했다.

### 디자인·치수·메시

큰 반죽 머리, 배가 볼록한 청록 작업복, 짧고 벌어진 다리, 둥근 요리 모자,
세 손가락+엄지 장갑, 넓은 앞치마를 가진 요리사 크리처 후보다.
작은 붉은 코·긴장한 눈썹·한 개의 이빨로 허둥대는 인상을 제안한다.
`design-concept.md`의 과거 성인 비율 조건보다 이번 사용자의 비인간형 비율 요구를 우선했다.
외형·모자·표정·최종 신장은 아직 채택 결정이 아니다.

| 항목 | 실제 후보 값 |
|---|---|
| 단위·축 | Metric / Unit Scale 1 / 1BU=1m / +X 정면·+Z 위 |
| 전신 바닥 원점 | `RC_Body_FloorRoot` = (0,0,0), 전신 최저 Z=0 |
| 전신 폭×깊이×높이 | **135.4×82.6×185.0cm** (폭은 양팔·장갑 포함, 높이는 모자 포함) |
| 머리 덩어리 | 약85×70×77cm, 목이 드러나지 않는 약2.4등신 인상 |
| 전신 | 메시 파츠50 / 기본8,372tris / 의복 두께 평가 후 **8,560tris** / 재질3종 |
| 별도 팔 한 쌍 | 포즈별 메시18파츠 / **2,856tris** / 재질2종 |
| 팔+작은 재료 | 19파츠 / 3,296tris / 재질3종(소품 포함) |
| 팔+손전등 | 19파츠 / 4,368tris / 재질4종(소품 포함) |
| 팔+프라이팬 | 19파츠 / 3,976tris / 재질4종(소품 포함) |
| 색 | 정점 `CookColor`와 Skin·ClothTint·Detail 재질. 청록 옷은 플레이어 구분색 제안 영역 |
| 예산 | 시안은 정밀 손·관절을 생략하여 공통 최종 메시 목표보다 적다. 성능·LOD 검수 결과는 아님 |

팔은 전신에서 단순히 잘라낸 사본이 아니라 시점 가시성용 별도 후보이며,
전신과 같은 장갑·커프·옷 색을 사용한다. 4개 포즈는 독립된 편집 가능한 정적 메시다.
스킨·변형 뼈·웨이트·최종 UV·LOD는 없다. 기존 89뼈 계약과 호환된다고 주장하지 않는다.

### 실제 미리보기·카메라

모든 개별 렌더는 `ArtSource/character/farmer_a/blockout/preview/`의 1280×720 PNG다.

| 화면 | 파일 |
|---|---|
| 중립광 정면·측면·3/4 | `body_front.png`, `body_side.png`, `body_threequarter.png` |
| 어두운 휴대 손전등 조명 | `body_dark.png` |
| 빈손·작은 재료·손전등·프라이팬 | `fp_empty.png`, `fp_small.png`, `fp_flashlight.png`, `fp_pan.png` |
| 기존 실제 전신 / 새 후보 / 원래 크기 OBJ + 치수 가이드 | `size_compare.png` |
| 전신·팔 2×2 모아보기 | `body_contact.png`, `fp_contact.png` |

- 전신 비교 카메라는 직교이며 가로 폭4m, 16:9에서 세로2.25m로 전신을 포함한다.
- 팔 카메라는 **가로 FOV70° / 세로 약43° / 16:9 / 눈높이156cm / 피치0° / near8.3cm**다.
  8.3cm는 기존 근접5cm와 뷰모델 배율0.6의 문서상 비교값이다. UE 설정은 수정하지 않았다.
- 기존 `egg_states.blend`의 `SM_Egg_Raw_A`(최장13cm), `portable_lights.blend`의
  `SM_Flashlight_A`(최장26cm), `cookware.blend`의 `SM_FryingPan_A`(최장48cm)를 복사했다.
  **시안 비교용 크기로 복사본을 정규화한 값이며 현재 게임 소품 실측값이 아니다.**
- 손전등 스포트는 오른손 렌즈 앞에서 같은 전방 축을 비춘다. 손전등 화면에 위에서 떨어지는 보조 스포트는 없다.
  전신 어두운 화면도 관찰자 손 높이의 전방 손전등을 사용했다. 헤드램프는 이번 4포즈에 포함하지 않았다.
- 크기 비교는 기존 몸의 정적 원본 형상과 원래 286cm OBJ를 나란히 보여준다.
  OBJ 임포터의 축 변환을 복사본에서 보정했고, 원본 OBJ의 크기·바닥 아래3cm 상태는 비교용으로 유지했다.
  새 후보에는 반경39cm·전체높이180cm 캡슐과 눈높이156cm 가이드를 별도로 표시한다.

### 직접 본 뒤 수정한 내용과 남은 문제

- 첫 전신 렌더에서 모자·발이 잘렸다. 직교 카메라 폭을2.55m→4m로 수정하여 정면·측면·3/4 전체가 들어오게 했다.
- 둥근 주머니가 버튼처럼 보이고 평면 수정본은 앞치마가 뚫고 나왔다.
  배 곡면을 따르는 주머니와 상단 봉제선으로 바꾸고 노멀 방향을 정리했다.
- 바지에서 붉은 장화 커프가 톱니처럼 드러났다. 커프 반경을 줄이고 장화와 같은 짙은 색으로 정리했다.
- 1인칭 오른손 소품이 화면 왼쪽에 나타나던 좌우 배치를 바로잡았다.
- 손전등 복사본의 퇴화면8개를 발견해 복사본만 정리했다. 마지막 메시 검사에서 퇴화면0,
  닫힌 파츠의 음수 부피0, 제작 메시의 회전·배율 위반0을 확인했다.
- 최종 렌더에서 발·모자·앞치마·손의 큰 시각 오류가 정리됨을 확인했다.
  팔 절단면은 화면에 드러나지 않고 소품과 화면 중앙 조준 영역이 분리된다.
  **정적 외형/그립 검수만 수행했으며 애니메이션 검수로 간주하지 않는다.**
- 남은 제약: 팔 포함 폭135.4cm는 캡슐 지름78cm보다 넓고, 머리·배도 일부 넘는다.
  게임 문틀·벽·협동 운반 간섭은 미검수다. 외형 때문에 충돌·눈높이를 바꾸지 않았다.
- 손가락은 둥근 덩어리를 겹친 그립 제안이다. 관절 토폴로지·정밀 접촉·피치 변화·큰 짐·던지기·넘어짐은 다음 단계다.
  팔4포즈는 형상 리뷰용이며 리깅/동작 납품본이 아니다.
- TintColor 런타임 소비·4인 색상·게임 조명·성능은 미검수다. 다음 단계는 외형 채택 후 기존 동작1개로 리그·축·변형 왕복 검증이다.

## 근거와 개발 세션

- [게임 개요](../../plan/overview.md)
- [에셋 제작 규격](../README.md), [공통 규격](../common.md), [제작 절차](../workflow.md)
- [리썰·R.E.P.O. 적용 기준](../reference-direction.md)
- [기존 플레이어 규격](README.md), [기존 farmer_a 카드](farmer_a.md)
- [개발 세션 시작문](farmer_a-model-brief-astra-prompt.md)

기존 아트 문서의 성인 비율은 이번 새 외형 시안의 강제 조건으로 사용하지 않는다.
사용자의 비율 변경 의도를 이 후보에 적용하고, 실제 채택과 게임 반영 시 관련 정본을 함께 갱신한다.
