# pig_hollow 제작 카드 — 빈 돼지 (탈 터진 돼지 괴물 본체)

<!-- combat-doff-20261020 -->
## 2026-10-20 현재 — 요청 7동작 Unreal 프리뷰 납품

- 사용자 요청: “돼지 탈벗는거랑 다람쥐 거미 공격 피격 죽음 모션도 다 만들어서 언리얼에서 보여줘”. 이번 요청의 **7동작 제작/UE 프리뷰·영상은 납품**, `runtime.level: preview`, `job: null`. 기존 UE 제외 범위를 이 지시로 확장했다. 게임 AI/AnimBP/피해 판정 연결이나 본체 외형 최종 승인으로 확대하지 않는다.
- **현재 맵/시퀀스:** `/Game/Monsters/CombatDoff20261020/QA_CombatDoff`, `LS_CombatDoff`. 돼지 10초 → 다람쥐 공격/피격/죽음 → 거미 공격/피격/죽음의 30초·1x 자동 반복. ActorScale=1, 저장 후 다른 맵을 거쳐 재열기했고 실제 PIE에서 카메라 전환·관절 이동·죽음 끝 정지를 확인했다. 관람용 샷 가시성은 해당 동물의 구간 밖에만 적용하며, 탈 벗기 중 본체/탈을 숨기거나 교체하지 않는다.
- **영상:** [전체 UE 30초·1x](../../../ArtSource/monster/combat_doff_20261020/video/CombatDoff_UE_1x.mp4), [돼지 UE 10초](../../../ArtSource/monster/combat_doff_20261020/video/Pig_Doff_UE_1x.mp4), [돼지 측면 10초](../../../ArtSource/monster/combat_doff_20261020/video/Pig_Doff_Side_UE_1x.mp4), [실제 인코딩 영상 표본](../../../ArtSource/monster/combat_doff_20261020/engine_storyboard.jpg). 1280×720/30fps, 우상단 방식·review v03·1x·UE 프리뷰 표시. 프레임별 엔진 캡처를 원래 속도로 조립한 영상이며 실시간 FPS 측정 영상이 아니다. UE 영상 9개와 Blender 반응 영상 6개 전 프레임 디코드·출력 프레임 수 검사 통과, 실제 인코딩 영상 12시점 시각 표본 확인. 연속 동작 전체 시청/모든 충돌 합격으로 확대하지 않는다.
- **편집/내보내기:** `ArtSource/monster/combat_doff_20261020/Pig/Pig_Doff_Tri_UE.blend`, `export/GC_Pig_Doff_Tri.abc`는 원래 v09의 평가된 같은 탈+본체 동작이다. F200 해제/F211 접지·탈 정지/F214 X 눈 유지. 돼지 얼굴·귀·꼬리·네 발굽은 같은 탈에 남는다. 이미지 9개 패킹, 외부 캐시 의존 없음. [파일/해시·전체 납품 목록](../../../ArtSource/monster/combat_doff_20261020/delivery_manifest.json).
- **독립 본체 클립:** `/Game/Monsters/CombatDoff20261020/Pig/NativeScaleV03/AN_PigBody_Doff`, 10초. 원시 FBX `export/AN_PigBody_Doff.fbx`는 Interchange `import_uniform_scale=0.01`로 가져와 기존 PigBody 소비 리그와 맞췄다. 9시점×18뼈 위치 최대 오차 0.0001604cm, 기존 소비 리그 대비 스케일 비 오차 0.000001193. 소비 리그의 검증된 FBX 단위 루트 스케일은100이며, 이를 Blender 로컬1과 직접 비교하지 않는다. 루트10000이던 최초 잘못된 임포트는 비채택이고 경로 위 NativeScaleV03이 현재다. [실측](../../../ArtSource/monster/combat_doff_20261020/native_pig_body_qa.json).
- **수정/재개:** n-gon 캐시 임포트 실패는 출력 사본 삼각화로, 안감 UV 겹침은 두께를 뺀 바깥 표면 베이크로, 재질 생성 중 텍스처 참조 크래시는 텍스처 선임포트 후 새 재질 생성으로 해결했다. 맵 복제의 잔류 World 오류는 새 전용 맵 생성으로 바꿨다. 촬영 자동 노출과 관람용 기본 Pawn 노출도 수정했다. 실패 캐시/프레임/로그는 별도 경로에 보존하며 현재 파일로 사용하지 않는다. 도구 `Tools/Blender/*pig*20261020.py`, `Tools/Unreal/*combat*20261020.py`, `probe_pig_native_scale_20261020.py`.
- **남은 한계:** 기존 v09 본체 후면 판형 조형, 초기 팔 회전/수납·통과·연속 충돌 문제는 이번 엔진 이관에서 재조형하지 않았다. 본체 외형 승인/전체 천·신체 충돌 합격이나 최종 게임용 탈 리그 완성으로 표시하지 않는다. 현재 요청한 모션 프리뷰는 준비됐으며 다음 작업은 사용자 시각 피드백에 따른 이 항목 보정 또는 별도 게임 연결 범위다. 승인 돼지 v02·원본 v09·기존 무료 이동 원본 해시는 유지했다.
<!-- /combat-doff-20261020 -->

상태: 요청 7동작 UE 프리뷰·정상 속도 영상 납품 / 본체 외형·전체 충돌 QA 한계 유지 (2026-10-20)
작성일: 2026-10-19 · 변경일: 2026-10-20

제작 1묶음([production-batch-1.md](C:\Work\CL9\docs\asset\animation\production-batch-1.md)) 1-3 단계의 신규 모델이다. 결정 #49("집 주인 다섯 전부 괴물(빈 것) 제작")로 돼지에도 빈 것 모델이 든다. 돼지가 광폭화 문턱을 넘으면 2.0초 동안 탈이 터지고(Burst), 그 뒤로는 같은 행 `pig`의 빈 것 열로 움직인다(결정 #42).
**추가 사용자 결정(2026-10-19, 조율 세션이 전달):** 기존 돼지 `pig_a`도 새 규격(톤 기준 넷)에 맞춰 **재제작**한다. 그래서 아래 리그 절에 "돼지·빈 돼지를 새 리그 한 벌로 함께 설계"하는 후보를 두었다. 이 결정이 [decisions.md](C:\Work\CL9\docs\decisions.md)에 기록됐는지는 미확인이다.
2026-10-19 사용자 요청 “돼지는 괴물 있는 애니까 첫 두 번째 시안을 기준으로 모델 만들기 시작해”로 제작을 시작했다. 현재 기준은 아래 승인 concept_doff_v02다. 추가 광폭화 v2~v4는 돼지 제작에서 제외한다. 실제 돼지 Blender 외형 model_doff_v02는 사용자 수락했으며 본체 외형·동작은 아직 승인 전이다. 이 카드가 두 상태 제작의 현재 정본이다.
값 표기: 문서에서 옮긴 값에는 출처를 붙였고, 이 카드가 제안하는 값에는 "추정", 문서에 없는 값에는 "미확인"을 붙였다.

<a id="concept-checkpoint"></a>

## 제작 범위·승인·재개 기록

절차·필드 의미는 [model-concept.md](C:\Work\CL9\docs\asset\model-concept.md) "상태 형식"이 정본이다.

```yaml
concept_pipeline:
  schema_version: 2
  asset_id: pig_hollow
  scope: production
  requested_outputs: [separate_editable_costume_and_body, doff_animation, multiview_and_motion_previews, dependencies_and_qa, unreal_sequence_preview] # 2026-10-20 사용자 후속 요청으로 UE 프리뷰 추가
  review_policy: concept_and_model
  review_override: null
  phase: engine_verify
  status: working
  updated_at: '2026-10-20T01:26:50+09:00'
  concept:
    candidate_version: concept_doff_v02
    approved_version: concept_doff_v02
  blender:
    candidate_version: model_doff_v02
    approved_version: model_doff_v02 # 제시한 돼지 외형에 적용. 미제시 본체/리그 승인으로 확대하지 않음
    source_concept_version: concept_doff_v02
    manifest: ArtSource/monster/pig_hollow/model_doff_v02/manifest.json
    qa: ArtSource/monster/pig_hollow/model_doff_v02/reopen_qa.json
  production:
    source_blender_version: model_doff_v02
    candidate_version: motion_preview_v09 / combat_doff_20261020 UE review v03
    manifest: ArtSource/monster/combat_doff_20261020/delivery_manifest.json
    qa:
      motion_probe: {status: requested_release_drop_probe_pass, evidence: ArtSource/monster/pig_hollow/animation_doff_v01/video_qa_drop_probe_v09.json}
      model_rig: {status: partial_with_failures, evidence: ArtSource/monster/pig_hollow/animation_doff_v01/motion_qa_v07.json}
      animation: {status: requested_release_drop_complete_with_inherited_failures, evidence: ArtSource/monster/pig_hollow/animation_doff_v01/drop_qa_v09.json}
      engine: {status: preview_verified, evidence: ArtSource/monster/combat_doff_20261020/delivery_manifest.json}
    runtime:
      level: preview
      engine: Unreal Engine 5.8.3
      asset_paths: [/Game/Monsters/CombatDoff20261020/Pig, /Game/Monsters/CombatDoff20261020/LS_CombatDoff, /Game/Monsters/CombatDoff20261020/QA_CombatDoff]
      evidence: ArtSource/monster/combat_doff_20261020/engine_reopen.json
  pending_review: null
  checkpoint: {path: ArtSource/monster/combat_doff_20261020/Pig/Pig_Doff_Tri_UE.blend, sha256: 8a705b1a1ea795a6bc0cbaeb9c214244b08a332f2eb42b91e31ae9a43dc30090}
  job: null
  last_completed: 요청한 7동작 Unreal 프리뷰와 영상15개 납품. 실제 PIE 자동 재생·저장 재열기·정상 속도 영상 전체 디코드/프레임 수 검사 및 뼈 위치/소비 리그 배율 검사 통과
  next_action: 현재 모션 프리뷰 검토. 기존 v09 본체 후면 조형·초기 회전/수납·연속 충돌 한계는 유지하며 별도 보정 또는 실제 게임 연결 요청에서 계속
  blocker: null
```

### 버전 파일 목록

경로는 프로젝트 상대 경로로 기록한다. model_pair_v01~v03은 입이 커지는 옛 구조이며 현재 승인된 방향은 concept_doff_v02의 별도 본체·탈 벗기 구조다.

| 단계 | 버전 | 기반 컨셉 버전 | manifest 경로 | manifest SHA256 |
|---|---|---|---|---|
| Blender 외형 | model_pair_v03 | sheet_six_species_v1 돼지 1·2행 | ArtSource/monster/pig_hollow/model_pair_v03/manifest.json | E0C83BA2EAB5AF198A8209E33BA732D5405AF06C46DA8E26B71E82F9AFD8847C |
| 승인 컨셉·새 세션 시작문 | concept_doff_v02 | 돼지 탈 + 작은 머리·거대한 배·긴 팔 본체 | ArtSource/monster/pig_hollow/concept_doff_v02/manifest.json | 81EF30BDCB363D6BBBFF152C0873495C25F456B14C5A3AA0766953ECC2978DF2 |
| 실제 별도 탈·본체 외형 후보 | model_doff_v01 | concept_doff_v02 | ArtSource/monster/pig_hollow/model_doff_v01/manifest.json | a0ed211de6c60e51548dfb1c03d15817dd719ae00588a37e1701978627edaaf7 |
| 돼지 탈 외형 수정 후보 — 표면 재구성 | model_doff_v02 | concept_doff_v02 원본 돼지 크롭 | ArtSource/monster/pig_hollow/model_doff_v02/manifest.json | f498af1b0023444e16c102403a5703820cda2839f66aa8b0888f31b8e262291e |
| 실제 탈 벗기 동작 시험본 | animation_doff_v01 / motion_preview_v07 | concept_doff_v02 + 사용자 수락 돼지 model_doff_v02 | ArtSource/monster/pig_hollow/animation_doff_v01/manifest_v07.json | 2a6063ef324b760cf1fbf5dc481bf3b1dd9d3ecdde2c5a000cb2e81c1a1522c6 |
| 마지막 X 눈·바닥 접촉 수정 | animation_doff_v01 / motion_preview_v08 | motion_preview_v07, 마지막 두 수정만 반영 | ArtSource/monster/pig_hollow/animation_doff_v01/manifest_v08.json | 3eee47c8289abecfb1015a6f6e63b2c0270a320e3bb300ba32cb7d6995d8c040 |
| 벗은 즉시 탈 낙하·바닥 정지 수정 | animation_doff_v01 / motion_preview_v09 | motion_preview_v08, 손 해제·탈 낙하/정지 타이밍 수정 | ArtSource/monster/pig_hollow/animation_doff_v01/manifest_v09.json | 4833742a2e7ca95843601ca733012d8221f6c7a8ea696438d4593ea8f7ef326d |

### 사용자 승인·수정 이력

실제 발언과 그 대상 버전만 적는다. 컨셉을 제작 기준으로 수락한 것과 Blender 모델 승인을 구분한다.

| 시각(시간대 포함) | 단계·대상 버전 | 결정(OK/NO/수정/철회) | 사용자 발언과 근거 | manifest SHA256 | 다음 행동 |
|---|---|---|---|---|---|
| 2026-10-19 +09:00 (발언 시각 미기록) | 돼지 평상시·정체 컨셉 | 제작 시작 수락 | “돼지는 괴물 있는 애니까 첫 두 번째 시안을 기준으로 모델 만들기 시작해” — 최초 12칸 1열 두 행 | 컨셉 PNG SHA256 39666e43bb425ae6a6b9e4dea7bb4c2511b42a2e2058625c1258f07f5dc64166 | 실제 3D 초안 제작 및 제시 |
| 2026-10-19 +09:00 (09:14 기록) | concept_doff_v02 | OK / 새 세션 제작 | “조아 이대로 만들자 새세션에서 저거주고만들자 여기서는 다른동물 이어서만들고 가능해?” | 81EF30BDCB363D6BBBFF152C0873495C25F456B14C5A3AA0766953ECC2978DF2 | 승인 컨셉 유지, 새 세션에서 실제 모델과 탈 벗기 동작 시작 |
| 2026-10-19 +09:00 (10:53 기록) | model_doff_v01 돼지 탈 외형 | NO / 수정 | “돼지가 그림이랑 너무다른데?” — 승인 그림 대비 세로 체형·긴 다리·얇고 뜬 귀·작은 코·둥근 발굽·얼굴을 타고 내려오는 봉제선·플라스틱 표면으로 달라진 것을 확인 | a0ed211de6c60e51548dfb1c03d15817dd719ae00588a37e1701978627edaaf7 | 컨셉 승인 유지, v01 보존/비채택. 수치 미세조정 대신 승인 그림 기반으로 외형 접근 변경 |
| 2026-10-19 +09:00 (12:08 기록) | model_doff_v02 돼지 외형 및 영상 제작 | OK / 영상 진행 | “ㅇㅇ 이대로영상만들어봐” — 직전 승인 그림/수정 3D 비교의 v02 돼지에 적용. 본체 미제시 결과의 사용자 승인을 주장하지 않음 | f498af1b0023444e16c102403a5703820cda2839f66aa8b0888f31b8e262291e | 돼지 외형 유지, 승인 컨셉 본체·탈 구조와 2~3초 시험부터 실제 벗기 영상까지 진행 |
| 2026-10-19 +09:00 (13:12 기록) | v07 영상의 마지막 표정·접지 | 수정 / 두 항목 한정 | “마지막에 돼지얼굴이 x되는 그런거만해주고 땅에 닿게해줘” — 마지막 양쪽 X 눈과 빈 탈/발바닥 접촉 보정. 본체 전체 외형·기존 결함의 승인으로 확대하지 않음 | 변경 전 manifest_v07: 2a6063ef324b760cf1fbf5dc481bf3b1dd9d3ecdde2c5a000cb2e81c1a1522c6 | v08 마지막 구간만 수정·재렌더·납품, 다른 제작 범위 추가 없음 |
| 2026-10-19 +09:00 (13:25 기록) | v08 탈 낙하·정지 타이밍 | 수정 | “돼지가죽이 벗으면서 나올때 자기혼자 스르르 떨어지게해줘 벗고난다음에는 그냥 떨어져야돼 바로” | 변경 전 manifest_v08: 3eee47c8289abecfb1015a6f6e63b2c0270a320e3bb300ba32cb7d6995d8c040 | 벗고 손을 놓은 즉시 낙하, 바닥 접촉 뒤 이동 종료. X 눈·접지 유지한 v09 납품 |

### 현재 실제 산출물과 한계

**motion_preview_v09 — 즉시 낙하·정지 수정 납품 (2026-10-19 13:25 +09:00):**

- [수정 사선10초·1배속 영상](../../../ArtSource/monster/pig_hollow/animation_doff_v01/video/pig_doff_full_main_v09_1x.mp4), [수정 측면 영상](../../../ArtSource/monster/pig_hollow/animation_doff_v01/video/pig_doff_full_side_v09_1x.mp4), [편집 Blender](../../../ArtSource/monster/pig_hollow/animation_doff_v01/pig_doff_motion_preview_v09.blend). 기존10초/30fps/1152×864 유지. 마지막 손 지지 F200 뒤 바로 F201에 낙하, F211에 접지해 약0.367초 안에 내려앉고 멈춘다. X 눈은 접지 직후 F214 완성이다.
- [2.53초 사선 시험](../../../ArtSource/monster/pig_hollow/animation_doff_v01/video/pig_doff_drop_probe_main_v09_1x.mp4) 및 [측면 시험](../../../ArtSource/monster/pig_hollow/animation_doff_v01/video/pig_doff_drop_probe_side_v09_1x.mp4) F180–255를 먼저 확인했다. [실제 디코드 낙하/접지 비교](../../../ArtSource/monster/pig_hollow/animation_doff_v01/preview_v09/decoded_drop_sequence.jpg)의 F195/200/203/206/209/211/214/230을 사선·측면으로 검사했다. 전체 영상600프레임과 짧은 영상152프레임 모두 디코드 오류0. 중립광·정상 속도이며 인게임 영상이 아니다.
- [실제 재열기 검사](../../../ArtSource/monster/pig_hollow/animation_doff_v01/drop_qa_v09.json): 수정한 손 집기 구간 최대 오차0.000001061m(허용15mm), F211–300 탈의 평가 메시 전체 정점 최대 이동0m. 접지 후 루트 이동·처짐 흔들림·느린 미끄러짐이 없다. 빈 탈/발바닥 접지±5mm 기준 통과. 기존과 같은 탈·부착 얼굴/귀/꼬리/발굽이며 본체 숨김/교체나 뼈 스케일 보정 없음. 직접 저작한 가속 낙하 곡선/형태 키이며 천 솔버 계산 합격을 주장하지 않는다.
- [납품 manifest](../../../ArtSource/monster/pig_hollow/animation_doff_v01/manifest_v09.json), [짧은 렌더 재사용 근거](../../../ArtSource/monster/pig_hollow/animation_doff_v01/probe_reuse_v09.json). 기존 F1–179는 v08 렌더, F180–255는 같은 v09/카메라/해상도/렌더러의 검증된 짧은 시험 렌더를 재사용하고 F256–300만 추가 출력했다. 모든 이미지 패킹, 필수 외부 의존/물리 캐시 없음. 이전 승인본/기존 pig_a·다른 동물 원본은 보존했다. 현재 실행 작업 없음.
- 재현: `drop_pig_costume_v09.py` → `audit_pig_drop_v09.py` → `render_pig_doff_video_v03.py --version v09 --phase drop_probe --start 180 --end 255 --view main/side` → 짧은 MP4 인코드/검사 → `reuse_pig_drop_probe_v09.py` → 같은 렌더러 `--version v09 --phase full --view main/side` → `package_pig_doff_video_v04.py --phase full --version v09` → `package_pig_drop_v09.py`. 이번 수정 범위만 완료했으며 아래 v07의 다른 메시/수납/회전 결함은 그대로 기록한다.

**v09 실패 수정 이력:** 원인은 벗은 뒤에도 이어지던 기존 탈 루트 이동(F250까지)과 늦은 처짐(F270까지), 늦은 손 회수였다. 첫 시험의 손 오프셋 최대36.43cm 실패는 손 pose 회전에 rest 방향을 이중 적용한 탓이어서 pose×inverse(rest)의 변형 회전으로 수정했다. 실패 파일 `pig_doff_motion_preview_v09_failed_grip_offset.blend`와 `drop_qa_v09_failed_offset.json`은 비채택 증거다. 로그 `drop_changes_v09.log`, `drop_qa_v09.log`, `render_drop_probe_main/side_v09.log`에 실제 실행을 보존한다.

**motion_preview_v08 — 사용자 요청 마지막 두 수정 완료 (2026-10-19 13:12 +09:00):**

- [수정 사선 영상](../../../ArtSource/monster/pig_hollow/animation_doff_v01/video/pig_doff_full_main_v08_1x.mp4), [수정 측면 영상](../../../ArtSource/monster/pig_hollow/animation_doff_v01/video/pig_doff_full_side_v08_1x.mp4), [수정 Blender 원본](../../../ArtSource/monster/pig_hollow/animation_doff_v01/pig_doff_motion_preview_v08.blend), [마지막 얼굴 정면](../../../ArtSource/monster/pig_hollow/animation_doff_v01/preview_v08/end_face_front.png), [마지막 사선](../../../ArtSource/monster/pig_hollow/animation_doff_v01/preview_v08/end_main.png). 기존과 동일한10초·30fps·1배속·1152×864이며 v08/Blender 구분 표시. 양쪽 영상300프레임씩 전체 디코드 오류0.
- F230–240에 동그란 눈을 양쪽 검은 X자로 전환한다. 원래 탈 표면의 편집 가능한 `M_Costume_End_XEyes` 및 `CTRL_End_XEyes`, `End_EyeClear/Cross` 속성을 사용했다. 표면에 붙은 같은 눈 영역이며 떠 있는 판·별도 돼지 메시 교체가 아니다. 마지막 얼굴 정면과 실제 디코드 F230/235/240/250/270/300 사선·측면 [비교 시트](../../../ArtSource/monster/pig_hollow/animation_doff_v01/preview_v08/decoded_end_transition.jpg)를 확인했다.
- F230–250 `End_Ground_Contact` 국소 키로 빈 탈 밑면/네 발굽과 본체 발바닥/발가락을 바닥에 맞추고, 바닥을 뚫던 지지 손 아래 면도 보정했다. 마지막 실제 평가 메시 최저점: 빈 탈0.003m, 양 발바닥 및6개 발가락 각각 약0.003m. 바닥±5mm 기준 통과. 전신 위치/뼈 길이/배율/리그 동작은 바꾸지 않았다. 마지막 접지 보정이며 전체 구간의 체중 이동·발 이탈 합격은 아니다.
- v07의 F1–230은 매 프레임 신규 제어값이0임을 확인하여 양쪽230장씩 재사용, 바뀐 F231–300만 각70장 렌더했다. 손 집기 종료 F200 이전의 동작·접촉 QA는 영향을 받지 않는다. [수정 범위/해시](../../../ArtSource/monster/pig_hollow/animation_doff_v01/end_changes_v08.json), [재열기·접지·변경 전 구간 검사](../../../ArtSource/monster/pig_hollow/animation_doff_v01/end_qa_v08.json), [납품 manifest](../../../ArtSource/monster/pig_hollow/animation_doff_v01/manifest_v08.json). 파일 이미지 전부 패킹, 추가 필수 외부 의존 없음. 승인 v02 및 v07 원본은 보존했다.
- 수정 중 실패/해결: 새 형태 키가 현재 열림·처짐 혼합을 복사해 F300 외형이 이중 변형됐다. `from_mix=False` 및 Basis 좌표에서 수직 보정량만 추가하도록 수정했으며 실패 파일 `pig_doff_motion_preview_v08_failed_mix.blend`와 `preview_v08/*failed_mix.png`는 비채택 증거다. 한쪽 눈의 피부 UV 샘플이 원본 베이크의 검은 부분을 읽던 문제는 검증한 분홍 볼 샘플로 교체했다. 재현은 `finish_pig_xeyes_ground_v08.py` → `fix_pig_end_cheek_sample_v08.py` → `audit_pig_end_v08.py` → `render_pig_doff_video_v03.py --phase full --view main/side --version v08` → `package_pig_doff_video_v04.py --phase full --version v08` → `package_pig_end_v08.py`다.
- 이번 한정 수정은 완료했으나, 아래 v07의 팔 회전·닫힌 수납 노출·후면 조형·천 신장 등은 사용자의 “그런거만” 범위에 따라 그대로 두었다. 사용자 취향 승인/전체 제작 합격으로 표시하지 않는다. 현재 실행 중 제작 작업 없음.

**motion_preview_v07 납품 (2026-10-19 12:54 +09:00):**

- [사선 10초·1배속 MP4](../../../ArtSource/monster/pig_hollow/animation_doff_v01/video/pig_doff_full_main_v07_1x.mp4), [측면 10초·1배속 MP4](../../../ArtSource/monster/pig_hollow/animation_doff_v01/video/pig_doff_full_side_v07_1x.mp4). 각각1152×864 / 30fps / F1–300, 영상 내 우상단에 실제 방식·v07·1x·Blender 구분 표시. ffmpeg 전체 디코드 오류0, 실제 디코드300장씩 확인. 사선/측면 각12프레임 디코드 비교 시트로 시각 표본 검사했으며 연속 충돌 전수 합격을 주장하지 않는다.
- [편집 Blender](../../../ArtSource/monster/pig_hollow/animation_doff_v01/pig_doff_motion_preview_v07.blend), [8뷰](../../../ArtSource/monster/pig_hollow/animation_doff_v01/preview_v06/multiview_v07.jpg), [파일/해시 목록](../../../ArtSource/monster/pig_hollow/animation_doff_v01/manifest_v07.json), [실제 재열기·집기점·관절·접지·연속성 검사](../../../ArtSource/monster/pig_hollow/animation_doff_v01/motion_qa_v07.json). 메시/웨이트/본체 전용 리그/키 액션/탈 등 개구부와 빈 탈 형태 키 편집 가능. 파일 이미지 전부 패킹, 필수 외부 이미지/물리 캐시 없음. 원시 GLB·재구성/베이크 중간 파일·재현 스크립트도 보존했다.
- v07은 F46–58 팔꿈치 굽힘 평면 이동만 완화했다. F44 전완의 큰 회전은 남아 최대73.53°/프레임, F57 관절 머리 이동 최대10.31cm/프레임으로 **연속성 합격이 아니다**. 팔 회전은 다음 수정 우선순위다. 본체 최대 수납 보정31.06cm·잔여 본체 노출·후면 판 같은 생성 흔적·탈 신장·발 디딤/완전 이탈 역시 미해결이다. 사용자 미검토 본체나 이 시험 영상을 완성 승인으로 표시하지 않는다.
- v07 실제 손 표면 대 탈 집기점: 오른손 F62–185, 왼손 F62–200 최대0.000001788m, 1.5cm 초과0건. 고정 뼈 길이·배율과 유한 행렬 검사도 통과했다. 앞서 통과한 내용만 유지하며 손가락 전체 감싸기/관통·천 자기 교차·연속 몸–탈 충돌은 합격이 아니다.
- 수정이 없는 프레임은 재렌더하지 않았다. v06→v07 팔 world matrix를 매 프레임 비교(허용2e-6)하여 사선/측면286장씩 재사용하고 나머지14장씩 렌더했다(`elbow_smoothing_v07.json`). F1/F300이 동일하여 v06 정면/측면/후면/사선8뷰도 재사용했고 `views.json`에 원래 렌더 출처를 유지한다. 짧은2.53초 실제 시험 영상은 v04이며 F90–165가 이후 수정에 영향받지 않는다. 현재 제작/렌더 프로세스는 종료했다.

**motion_preview_v06 실제 동작 검사:** v04/v05 실제 표면 감사에서 접촉 오차 최대21.22cm, 17건(F185 양손, F186–200 왼손)을 확인했다. 초반 수납 보정보다 큰 원인은 리토폴로지한 탈 메시의 부모가 `null`인 반면 손 목표만 `Costume_ROOT` 이동을 따라간 점이었다(`contact_transform_debug.json`). v06에서 실제 탈 표면을 이 루트에 연결했다. F1–180 루트가 항등변환인 것을 매 프레임 확인하여 기존 사선 렌더180장을 재사용하고 변한 후반부터 출력했다(`root_repair_v06.json`).

- 재열기 후 F1–300 관절 행렬 유한성·뼈 길이 검사: 길이 오차 최대0.000000391m, scale 최대오차0.000000596. 숨김 애니메이션 없음. 기존 승인 돼지 v02 원본 SHA256은 그대로 `b2edc00722851ea2233937c205e153714427a73f64e4c689f211fc5e93d7ad65`다.
- 실제 본체 정점 대 실제 탈 테두리 정점으로 오른손 F62–185, 왼손 F62–200 전 프레임 검사. 최대거리0.000001185m, 1.5cm 기준 실패0건. **두 개 집기점만의 접촉 통과**이며 모든 손가락의 감싸기·관통 합격은 아니다.
- F215–300 양발 메시 최저점은 바닥에서1.282–1.285cm 높아 접지 미달이다. F1 잔여 본체 노출/수납, F90–165 후면의 생성 조형 잔여, F166–230 손 주변 탈의 과도한 신장, 후반 다리 빼기·체중 이동, 천 자기 교차/연속 몸–탈 충돌은 미해결이다. 목/눈의 표현과 본체 후면 외형도 최종 승인 전이다. 이번 영상/Blender는 완성 승인이 아닌 **10초·30fps·1배속 동작 시험본**이다.

**animation_doff_v01 동작 시험 진행 (2026-10-19 12:38 +09:00):** 승인 돼지 v02 원본을 보존하고, 별도 파생 `pig_doff_animation_fit_v04.blend`에서 실제 등 봉제선 개구부/안감, 본체 전용 리그와 300프레임·30fps 탈 벗기 키를 제작했다. 본체는 승인 시안 마지막 칸에서 돼지를 마스킹한 입력으로 로컬 TRELLIS.2 1회(512/42/12/100k/2K, 144.85초) 생성했다. 재생성 없이 복셀 리토폴로지·색 베이크·누락 뒷발 직접 조형·본체 웨이트를 수정했다. 보이지 않던 등/발은 추정이며 사용자 본체 외형 승인이 아니다.

- 반복 실패와 변경: `pose_001/062/140/215/270.png`의 종이처럼 늘어진 피부는 큰 구멍을 막은 다각형·분절 웨이트가 원인이어서 복셀 정리와 bone heat로 변경(`heat_*.png`). 첫 프레임이 재열기마다 뒤틀린 문제는 갱신 전 부모 변환 사용이 원인이어서 목표 부모 행렬로 local basis를 직접 계산(`motion3_*.png`). 모두 `animation_doff_v01/probe_stills/`에 실패 증거를 보존한다.
- 닫힌 상태 수납은 관절 접기만으로 실패했다. v04는 본체에 `Compact_ClothPressure` 국소 보정 키를 쓰고 등 개구부가 열릴 때 해제한다. 뼈 길이/오브젝트 축소·본체 숨김/교체는 사용하지 않았다. 다만 본체 최대 보정 31.06cm이며 **체적 보존 물리 검증은 아니다**. F1에서 일부 정점의 표면 밖 잔여 최대 2.14cm가 수치상 남아 전체 수납 합격으로 처리하지 않는다(`compact_fit_v04.json`).
- F62–185/200의 분석적 손목 도달 오차는 0이나 손 표면·손가락 전체 접촉 검수와 구별한다. 실제 손 표면 거리·발 접지·관통·천 자기 겹침은 별도 검사 중이다. F140/270 정지 영상에서 손 주변 탈 신장, 남은 후면 조형 흔적, 끝부분 발 빼기/탈 이격에 보정 여지가 있다. 정상 속도 영상은 이 상태를 숨기지 않는 **시험본**으로 납품하며 전체 제작 완료/물리 합격을 선언하지 않는다.

**model_doff_v02 돼지 외형 수정 결과 (2026-10-19 11:01 +09:00):**

- [편집 원본](../../../ArtSource/monster/pig_hollow/model_doff_v02/pig_costume_appearance_v02.blend), 12,456,096 bytes. [승인 원본 / 실제 수정 3D 비교](../../../ArtSource/monster/pig_hollow/model_doff_v02/preview/approved_vs_v02.png), [반려 v01 / 수정 v02 비교](../../../ArtSource/monster/pig_hollow/model_doff_v02/preview/v01_vs_v02.png), [정면·측면·후면·사선](../../../ArtSource/monster/pig_hollow/model_doff_v02/preview/pig_four_views_v02.png). 모두 실제 Blender 렌더이며 새 컨셉 이미지가 아니다.
- 로컬 TRELLIS.2 1회(512, seed42, steps12, 목표100k, 텍스처2048), 실제 생성 시간 182.58초. 원시 GLB SHA256 `b97e00af56886973b2059b73ab2848f7b2a3279ab42faec14ee55049843b8bcd`, 원시 GLB·latents·raw_mesh·생성 로그는 `model_doff_v02/trellis_costume/`에 보존한다. 재생성하지 않고 이 결과에서 작업한다.
- Blender 5.2.2 LTS에서 +X 정면으로 방향 정리·균일 크기 조정·재질/참조 이미지 패킹. 생성기가 후면에 만든 중복 꼬리 2개(안팎 총 4개 분리 성분)를 제거하고 몸에 연결된 한 꼬리는 유지했다. 프레이밍을 실제 메시 바운드에 맞춰 코·발굽·꼬리가 잘리지 않게 수정했다. 생성 원본은 보존했다.
- 저장 후 재열기·유한 좌표·패킹 이미지 확인: [reopen_qa.json](../../../ArtSource/monster/pig_hollow/model_doff_v02/reopen_qa.json). 표면 메시 **45,311 정점 / 92,998 tris**, 치수 약 **195.59×159.48×135.00cm**. 높이135cm는 프리뷰 가정이며 게임 판정/기존 pig_a 크기를 바꾸지 않았다. 컬렉션 `COSTUME_SURFACE`, 루트 `Costume_ROOT`.
- 짧은 다리·낮은 배·두꺼운 접힌 귀·큰 코·뒤쪽 봉제선은 v01보다 원본에 가까워졌다. **사용자 외형 승인이나 원본 일치 합격은 아직 없다.** 코 중앙의 생성 윤곽, 눈 주변의 텍스처 경계, 표면 디테일 손실, 일부 봉제선/끈의 생성 흔적, 보이지 않던 후면 추정은 추가 보정 항목이다. 외형 충실도 검토와 파일 QA를 구분한다.
- 이번 수정 결과는 **돼지 외부 표면 초안**이다. 속 빈 등 개구부·안감·소매 내부·부착물 분리/유지 구조·변형용 리토폴로지는 아직 옮기지 않았다. 반려된 v01의 외형/변형 키를 새 기준으로 자동 승계하지 않는다. 본체 v01 역시 승인되지 않았으며 승인 컨셉의 본체 외형 확인은 계속 필요하다. 리그·접촉/수납 시험·전체 탈 벗기·1배속 영상은 미제작/미검증, UE는 요청 범위 제외다.
- 실제 명령/설정은 manifest와 생성 로그에 있다. 입력 준비 `Tools/Blender/prepare_pig_doff_v02_reference.py`, 임포트·후면 정리·재열기·렌더 `review_pig_doff_model_v02.py`, 비교/해시 `package_pig_doff_model_v02.py`. 수정 시 새 파생 체크포인트를 사용하고, 이미 생성/검수한 GLB와 렌더를 이유 없이 반복하지 않는다. 현재 작업 프로세스는 모두 종료했다.

**2026-10-19 v01 반려 후 수정 작업:** 사용자 “돼지가 그림이랑 너무다른데?”를 돼지 탈 외형 반려로 기록한다. v01의 기본 구조 검사 통과는 외형 충실도 통과가 아니다. 승인 그림의 짧은 다리에 낮게 앉은 둥근 몸, 두툼하게 아래로 접힌 귀, 넓고 둔한 코, 짧고 넓은 분할 발굽, 깊게 파묻힌 작은 검은 눈, 등 뒤쪽 봉제선, 무광의 분홍 표면을 회복한다. 새 디자인·컨셉 재승인 없이 승인 v02 좌상단 돼지만 `(130,53,635,450)`으로 무손실 크롭하여 로컬 TRELLIS.2에 입력한다. 크롭 SHA256 `40ce4fbbc532b3eddf201a0e3fd6040798452bb095c539eab29ef35337f8309d`. [전처리 근거](../../../ArtSource/monster/pig_hollow/model_doff_v02/input/crop.json). 로컬 설치·캐시·GPU 여유를 확인하고 기존 검증 프로필 512/42/12/100k/2K 1회 실행. 생성 모델은 외형 기준/작업 사본이며 속 빈 탈·리그·애니메이션 완성으로 취급하지 않는다. v01 본체 역시 승인된 적 없으며 이 수정이 본체 승인으로 확대되지 않는다.

**model_doff_v01 반려된 실제 외형 초안 이력 (2026-10-19 10:34 +09:00):**

- 편집 원본: [pig_costume_body_v01.blend](../../../ArtSource/monster/pig_hollow/model_doff_v01/pig_costume_body_v01.blend), 9,587,999 bytes. Blender **5.2.2 LTS**, 직접 메시 저작·복셀 융합 조형·등 개구부 형태 키. 승인 이미지는 파일에 패킹했고 외부 텍스처/물리 캐시 의존 없음. 새 이미지 생성·유료 생성 없음.
- 실제 저장본을 다시 열어 정면·측면·후면·사선을 탈과 본체 각각 렌더했다. [8뷰 시트](../../../ArtSource/monster/pig_hollow/model_doff_v01/preview/appearance_review_sheet.png), [나란한 모델](../../../ArtSource/monster/pig_hollow/model_doff_v01/preview/review_pair.png), [본체와 빈 탈 정지 연구](../../../ArtSource/monster/pig_hollow/model_doff_v01/preview/review_empty.png). 원본 PNG 12개 + 라벨 파생 PNG 3개이며 동작 영상이 아니다.
- 컬렉션 `COSTUME` / `BODY` / `BODY_DETAILS` 및 별도 `Costume_ROOT` / `Body_ROOT`. 얼굴·귀·꼬리·네 발굽은 모두 같은 `Costume_ROOT` 소속이다. 속 빈 셸에 어두운 안감·16mm 두께가 있고 `Dorsal_Open` / `Empty_Drape`는 **정지 구조 연구**다. 입 열림 키나 턱 힌지가 없다. 각 키는 단독으로 0→1 사용하며 애니메이션 검증된 조합이 아니다.
- 본체는 가면 없는 작은 머리·깊은 눈·세로 목 틈·중앙 골이 있는 큰 배·긴 팔·다섯 손가락·짧은 뒷다리를 직접 조형했다. 실제 본체 주 메시 연결 성분 **1개**, 유한 좌표, 귀 2개·꼬리 1개·분할 발굽 8조각, 동일 탈 루트, 패킹 참조, 미지정 재질 없음의 기본 검사 7개 통과. [재열기 검사](../../../ArtSource/monster/pig_hollow/model_doff_v01/reopen_qa.json), [치수·복잡도](../../../ArtSource/monster/pig_hollow/model_doff_v01/model_qa.json). 이 검사는 접촉·관통·수납 QA를 대체하지 않는다.
- 임시 외형 치수 X×Y×Z: 닫힌 탈 약 **193.38×129.88×151.47cm**, 본체 약 **161.80×252.76×222.10cm**(손까지 포함). Unit Scale 1, +X 정면, +Z 위. 게임 판정/액터 배율은 변경하지 않았다.
- 평가 메시: 탈 약 **221,312 tris**, 본체 약 **497,408 tris**. 현재는 외형 조형본으로 게임용 최종 예산·변형 토폴로지 미달/미완료이며, 외형 확인과 짧은 구조 시험 뒤 리토폴로지·리그를 진행한다. 원본을 최종 게임 메시로 표시하지 않는다.
- 자체 보정: 빈 탈의 무한대에 가까운 가장자리 늘어짐을 개구부 가중치 범위 제한으로 제거; 팔 단면의 급회전은 연속 프레임 전달로 수정; 손목을 연속 조형; 목의 미지정 재질과 Boolean 8정점 잔여 조각 제거. 최종 저장본으로 관련 렌더/검사 갱신.
- **남은 한계:** 빈 탈 형태 키는 소매 내부의 닫힌 끝면이 분홍 면으로 보이는 부분이 있고 최종 빈 주머니/소매 연결 토폴로지가 아니다. 수납 자세·배 통과·본체/탈/손가락 교차·천 자기 교차·손 집기·발 지지·자연스러운 처짐은 **미검증**이다. 본체 발 최저점 약 −0.083cm는 조형본 수치이며 접지 합격으로 쓰지 않는다. 등·보이지 않는 면은 승인 시안에 근거한 추정 조형이다. 목 틈의 재질 경계도 최종 토폴로지 단계에서 정리할 수 있다.
- 리그·전체 탈 벗기 클립·1배속 영상은 아직 제작하지 않았다. 사용자의 명시 순서와 스킬의 실제 Blender 외형 검토 조건 때문에 현재 **전체 제작 미완료 / 외형 확인 대기**다. 컨셉 재승인을 요청하지 않는다. 외형 확인 뒤 짧은 기술 시험에서 계속하며 이후 기술 단계별 재승인은 요구하지 않는다. UE/게임 연결은 이번 범위 제외.
- 재개는 위 체크포인트를 열어 진행한다. 완료된 렌더를 다시 만들 필요 없다. 재현 스크립트는 `Tools/Blender/build_pig_doff_model_v01.py` → `finish_pig_doff_model_v01.py` → `audit_pig_doff_model_v01.py` → `package_pig_doff_model_v01.py`이며, 현재 후보를 덮어쓰지 말고 새 파생 버전으로 경로를 바꾼다. 생성 로그 `ArtSource/monster/pig_hollow/model_doff_v01_build.log`, 보정/검사 로그는 해당 버전의 `finish.log` / `audit.log`. 다른 동물 및 기존 pig_a 원본은 편집하지 않았다.

**2026-10-19 현재 세션 제작 재개:** 사용자 명시 요청으로 `concept_doff_v02`를 그대로 제작한다. 컨셉 재승인·새 디자인 생성 없음. 본체는 가면 없는 작은 머리·깊은 눈·거대한 배·긴 가는 팔·짧은 뒷다리·세로 목 틈을 유지한다. 등 개구부가 열리는 분홍 돼지 탈은 얼굴·귀·꼬리·네 발굽이 붙은 같은 빈 탈로 남는다. `model_pair_v01~v03` 및 `concept_doff_v01`은 새 본체의 기반으로 쓰지 않는다. 이번 범위는 편집 원본·필요 의존 파일·실제 모델 외형 검토·짧은 동작 시험·전체 탈 벗기·정상 속도 영상·QA이며 UE/게임 구현은 제외한다. 실제 Blender 외형 승인은 아직 없으므로 정지 모델과 열린 탈 구조를 먼저 제시한다. 연결된 Blender/실행 중 Blender 프로세스는 없었으며 전용 background Blender 5.2를 사용한다. 기존 pig_a·닭·소 파일은 편집하지 않는다. 아래 옛 계획은 이 현재 지시보다 우선하지 않는다.

**컨셉 승인 및 이전 세션 인계 이력:** `concept_doff_v02`는 2026-10-19 사용자 “조아 이대로 만들자”로 승인됐다. [당시 시작문](../../../ArtSource/monster/pig_hollow/concept_doff_v02/start_new_session.txt)을 받아 현재 세션에서 실제 모델 제작을 시작했다. 당시 “다른 동물 컨셉 계속”은 이전 세션에 한정되며, 현재 세션은 돼지 제작만 한다. 실제 Blender 모델 승인은 아직 없다.

**승인 본체 v02:** 사용자 “괴물다른디자인”에 따라 v01의 가면·근육형 어깨를 제거하고 작은 머리, 깊은 눈, 부푼 배, 가늘고 긴 팔, 짧은 뒷다리와 세로로 갈라진 목의 먹보형을 제안했고 위 이력으로 승인됐다. 돼지 탈 및 등으로 벗어 나오는 구조는 유지. [현재 시안](../../../ArtSource/monster/pig_hollow/concept_doff_v02/pig_costume_emergence_v02.png), [프롬프트](../../../ArtSource/monster/pig_hollow/concept_doff_v02/prompt.txt). 내장 이미지 도구 1회. SHA256 `016DCF6C1A1A119578DB4A068C4E4D3A6460F83A9049CC32B9C5FE01EACB365E`. 현재 세션에서는 이미지 생성 없이 이 승인본으로 직접 3D 저작한다.

**2026-10-19 후속 지시가 아래 옛 입 열림 구조보다 우선한다.** 사용자: “돼지랑 돼지괴물 그려서 닭처럼 괴물이 벗고 나오는 거 만들어”. 돼지 겉모습은 닫힌 정상 얼굴을 유지하고, 등 봉제 개구부로 별도 본체가 나온 뒤 빈 돼지 탈이 남는 구조로 변경한다. `model_pair_v03`은 새 구조의 완성 모델이 아니다. 아래 기존 계획에서 입 하나만 커진다는 부분은 과거 방향으로 읽는다.

- 새 [4단계 컨셉](../../../ArtSource/monster/pig_hollow/concept_doff_v01/pig_costume_emergence_v01.png): 평상시 / 등 벌어짐 / 본체 탈출 중 / 본체와 벗은 탈. 내장 이미지 도구 1회, [실제 프롬프트](../../../ArtSource/monster/pig_hollow/concept_doff_v01/prompt.txt).
- 후보 본체: 닭 계열의 어두운 피부·밝은 가면, 넓고 무거운 어깨와 몸통, 긴 팔과 접힌 뒷다리. 돼지 전용 실루엣 제안이며 사용자 승인 전. 컨셉 이미지 SHA256 `9F4070FACFE3A381DC4028DB75C8B516A72B37A315BC55C839D3A890DE634642`.
- 이번 요청으로 탈 벗기 동작 제작까지 범위 확대. 새 컨셉 검토→실제 3D 외형 검토→손 접촉·통과 2~3초 시험→전체 벗기 동작 순서. 엔진 연결 범위는 별도 확정 전이며 스틸 이미지를 3D/애니메이션 완료로 간주하지 않는다.

**아래는 이전 입 열림 초안의 증거다.**

- 직접 저작한 [편집 원본](../../../ArtSource/monster/pig_hollow/model_pair_v03/pig_normal_hollow_v03.blend)과 [비교 렌더](../../../ArtSource/monster/pig_hollow/model_pair_v03/preview/normal_hollow_comparison.png). 기준 이미지를 파일에 패킹했다. 실제 `.blend`에서 렌더했으며 이미지 생성 모델 출력이 아니다.
- `CTRL_Pig_State`의 `Open` 값 0은 평상시, 1은 정체. 양쪽 콧구멍은 뚫린 메시이고 입 안은 연속된 검은 주머니 메시. 기존 게임의 `pig_a.blend`·FBX·UE 에셋은 수정하지 않았다.
- 초안 32,675 정점 / 62,754 삼각형(모디파이어 전), 84개 메시. 현재는 외형 초안이라 게임 예산 5~15k보다 큼. 평상시 약 169×126×148cm, 정체 약 150×126×237cm로 임시 조형 크기이며 기존 게임 판정 크기로 확정한 값이 아니다.
- 저장 후 재열기, `Open` 0/.25/.5/.75/1의 유한 좌표와 다리/발굽 불변 확인: [reopen_qa.json](../../../ArtSource/monster/pig_hollow/model_pair_v03/reopen_qa.json). 중간값은 외형 키의 선형 보간이며 자연스러운 Burst 동작 검증이 아니다.
- 정면·측면·후면·3/4를 상태별 렌더했다. 입 뒤 테두리가 내부에 보이던 결함은 v03의 연속 안감으로 보정. 아직 힌지형 입 열림/옆면 주름·귀 두께·표면 질감을 외형 피드백에 맞춰 다듬을 수 있다.
- 리그·게임용 리토폴로지·UV/텍스처 베이크·애니메이션·엔진 임포트·게임 QA는 미실행. 기존 아래 장기 제작 계획의 `production` 표현이 이번 요청 범위를 자동 확대하지 않는다.
- 재현: `Tools/Blender/build_pig_hollow_pair_v01.py`(현재 v03 출력), `Tools/Blender/review_pig_hollow_pair_v03.py`. 완료한 렌더를 재개 시 다시 만들 필요는 없다.

### 현재 검토 단계

이번 요청의 명시 순서와 `concept_and_model`을 적용한다. 이미 승인된 컨셉은 다시 묻지 않으며, 실제 외형 확인 뒤 기술 단계의 재승인을 요구하지 않는다.

| # | 자리 | 무엇을 보고 승인하나 | 지금 상태 |
|---|---|---|---|
| 1 | 컨셉 | concept_doff_v02 | 사용자 승인 완료, 유지 |
| 2 | 실제 Blender 외형 | v01 반려 후 model_doff_v02 돼지 외형 수정 비교·다각도 렌더. 본체는 아직 승인 없음 | 사용자 “ㅇㅇ 이대로영상만들어봐”로 돼지 v02 수락·영상 지시. 미제시 본체 승인을 주장하지 않음 |
| 3 | 짧은 시험·전체 동작 | 손 집기·배 통과·지지·탈 변형의 2~3초 시험 후 전체 벗기 | v04 2.53초 및 v07 전체10초 사선/측면1배속 영상 납품·전체 디코드 완료. 집기점·고정 길이 검사 통과. 팔 회전·수납·신장·발 접지/탈출 실패가 남아 최종 제작 미완료 |
| 4 | 게임 연결 | UE 임포트·게임 상태 연결 | 이번 요청 범위 제외 |

## 과거 입 열림 설계 기록 — 현재 제작 지시로 사용하지 않음

아래 용도·리그 후보·클립 목록·Burst 시간·프롬프트는 옛 입 열림 해석의 보존 기록이다. 현재 모델·리그·탈 벗기 제작 기준은 위 `concept_pipeline`과 승인 `concept_doff_v02`이며, 아래 입 뼈·5개 클립·UE 작업을 이번 범위에 추가하지 않는다.

## 용도와 모습

- **분류 / Alias / SubAlias:** monster / `pig` / 빈 것은 같은 행의 빈 것 열로 다룬다(결정 #42). 별도 SubAlias가 드는지는 미확인이다.
- **사용할 장소·플레이 상황:** 돼지 농장 실내. 광폭화 사건(돼지가 먹고 있는 것을 도로 줍기 +2, 먹이통 안의 것 집기 +1, 문턱 3~8)이 쌓이면 터진다. 빈 돼지는 **느린 건 그대로**(396cm/s)이고, 입이 커져 잡는 반경이 100 → 140cm로 늘며, 빈손도 냄새(1200cm)로 쫓고 미끼를 무시한다. HP 12~14, 경직 없음, **문 밖으로 따라나오지 않는다** — 빈 돼지의 규칙은 "걸어라, 멈추지 마라, 막다른 길로 들어가지 마라"다. 잡히면 처형 즉사다(주인 6.3·결정 함 #26).
- **기획 및 시스템 문서:** [monster-owners.md](C:\Work\CL9\docs\plan\draft\monster-owners.md) 6절(특히 6.3 광폭화·빈 것 칸), [art-plan.md](C:\Work\CL9\docs\plan\draft\art-plan.md) 4.5·2.2·5.2 G2, [motion-list-2026-10-18.md](C:\Work\CL9\docs\review\motion-list-2026-10-18.md) 돼지 줄, [story-draft.md](C:\Work\CL9\docs\plan\draft\story-draft.md) 6.3 돼지 행, [motion-plan.md](C:\Work\CL9\docs\asset\motion-plan.md) 공격 논의안(돼지 "벌어진 입으로 붙잡아 삼키기" — 사용자 미승인 초안), [pig_a.md](C:\Work\CL9\docs\asset\monster\pig_a.md).
- **기존 메시·리그를 재사용하는지:** 아래 "기존 pig_a 24뼈와의 관계"에 후보 둘을 적었다.
- **형태·재질·색:** 입이 몸보다 커진 분홍 덩어리이고, 입 안이 깊고 어둡다. 몸·짧은 다리·귀·꼬리는 평상시 돼지와 같아야 한다(톤 기준 ③). 색은 평상시 돼지의 분홍을 그대로 쓰고, 입 안을 거의 검은 한 색 면으로 둔다(추정). 색 수 제한·넓은 색면·높은 채도([design-concept.md](C:\Work\CL9\docs\asset\design-concept.md) 규칙 2)를 지킨다. 피·살·이빨 줄 같은 사실적 공포 장식은 쓰지 않는다(story-draft 6.1 문장 2 — 무서운 것은 탈의 안쪽이고, 실밥이 벌어지고 몸의 비율이 틀어지는 몇 초다).
- **기준 이미지:** 없음. 평상시 쪽 비교 자료는 지금 있는 렌더다 — 실물 `C:\Work\CL9\ArtSource\monster\pig_a\preview\neutral.png`(게임 연결 검수됨), 귀여움 1단계 `C:\Work\CL9\ArtSource\monster\pig_a_v2\preview\v2_neutral.png`·`compare_neutral.png`(사용자 승인 대기). `pig_a`가 재제작되므로 이 둘은 임시 비교 자료다.
- **생성 도구:** 이미지는 ChatGPT 또는 Tripo 이미지 생성 — [tools/tripo.md](C:\Work\CL9\docs\asset\tools\tripo.md). 3D 초안은 평상시 돼지와 정점 대응이 맞아야 Burst가 이어지므로, 이미지 → 3D 생성보다 **평상시 돼지 메시에서 직접 변형해 만드는 길**이 맞을 가능성이 높다(추정).
- **리썰/R.E.P.O.에서 참고한 특징:** "위협은 알아볼 수 있는 체형과 큰 예고 동작"([reference-direction.md](C:\Work\CL9\docs\asset\reference-direction.md)) — 어둠 속에서 몸보다 큰 입의 실루엣 하나로 "터진 돼지"가 읽혀야 한다.
- **이번 제작 범위 / 연결이 필요한 기능:** `production`이다. 빈 돼지 메시·리그(아래 후보)·클립 5(Burst·Hollow Idle·Run·Execute·Death)·FBX·UE 왕복·돼지 영상에 더하기까지 간다. 광폭화 게이지·조짐 단계·빈 것 열·Burst 동안의 상태는 gameplay 몫의 **연결 필요**다. 조짐 3단계의 "목의 실밥이 벌어지고 입꼬리가 귀까지 찢어져 보인다"는 재질 파라미터(주인 6.3)이고, 실밥 벌어짐 재질은 asset-todo의 이펙트 항목이다.

## 기존 pig_a 24뼈와의 관계 — 후보 둘과 추천

`pig_a` 24뼈는 `root`, `body`, `chest`, `neck`, `head`, `jaw`, `ear_l`·`ear_r`, 네 다리 각 세 마디(`front_l_upper`·`_lower`·`_paw` 등), `tail_01`~`04`다(`C:\Work\CL9\Tools\Blender\build_pig_a.py`). 턱 뼈 하나로는 "몸보다 커지는 입"이 안 된다(art-plan 4.5 리그 요구).

| 후보 | 무엇을 하나 | 얻는 것 | 비용·위험 |
|---|---|---|---|
| ① 기존 24뼈 유지 + 턱·입 뼈 추가 | 지금 `SKEL_Pig`의 24뼈를 그대로 두고, 빈 돼지 메시에 입 뼈를 더한다(아래 "더할 뼈" 표). 같은 스켈레톤 에셋이 두 메시를 받는다 | 지금 게임에 연결된 10클립·Execute·플레이어 짝 `AN_Farmer_Executed_Pig`·처형 앵커 표가 그대로 남는다. Burst 첫 프레임에서 평상시 메시를 빈 돼지 메시로 바꿔도 포즈가 같아 이음이 보이지 않는다(추정) | **`pig_a`가 재제작되므로 이 안의 이점이 줄어든다.** 새 외형의 비율·관절 위치가 바뀌면 기존 클립의 레스트 포즈가 맞지 않아 어차피 다시 굽게 된다(추정 — 새 `pig_a`의 비율은 아직 없다). UE 스켈레톤에 뼈를 더해도 기존 클립이 그대로 도는지는 CL9에서 확인한 적이 없다(art-plan 5.2 G2 "뼈를 더하면 공유가 깨진다(미확인)") |
| ② **돼지·빈 돼지를 새 리그 한 벌로 함께 설계 — 추천** | `pig_a` 재제작과 빈 돼지를 한 리그로 같이 설계한다. 입 뼈를 처음부터 넣고, 평상시 돼지에서는 그 뼈를 레스트 포즈에 둔다. 이름·계층은 네발 규격([rabbit_a.md](C:\Work\CL9\docs\asset\monster\rabbit_a.md) "네발 규격")을 따르고, 돼지 몫의 입 뼈만 더한다 | 평상시 ↔ 빈 돼지의 Burst가 한 스켈레톤 안에서 이어진다. 재제작 비율에 맞춘 레스트 포즈로 클립을 한 번만 굽는다. 네발 규격과 이름이 같아져 토끼·눈먼 개·여우와 스크립트가 한 갈래가 된다 | 기존 10클립·Execute·플레이어 짝을 새 리그로 다시 만들어야 한다(처형 앵커 표의 값은 재사용할 수 있다 — 아래 Hollow Execute). 재제작 `pig_a`의 컨셉 승인을 먼저 기다린다. 토끼의 네발 규격 확정(0-a) 뒤가 안전하다 |

**추천은 ②다.** 사용자가 `pig_a` 재제작을 정했으므로 ①의 큰 이점(기존 클립 그대로)이 새 외형의 비율 변화로 대부분 사라질 가능성이 높고(추정), 빈 돼지의 핵심 장면인 Burst는 두 메시가 한 리그·한 레스트 포즈를 쓸 때 가장 깨끗하게 이어진다. 단, **재제작 `pig_a`가 기존 24뼈의 계층과 레스트 포즈를 그대로 유지하기로 정해지면 ①이 낫다** — 그때는 클립·앵커·플레이어 짝이 모두 살아남는다. 빈 돼지만 따로 리그를 두는 셋째 길은 Burst가 두 스켈레톤을 넘나들어야 해서 적지 않았다.

**더할 뼈(①·② 공통, 제안).** 컨셉 후보에 따라 다르다.

| 뼈 | 부모 | 하는 일 | 쓰는 후보 |
|---|---|---|---|
| `lip_upper` | head | 윗입술. 머리가 뒤로 젖혀질 때 입 윗선을 끌어올린다 | A·B·C·D |
| `lip_lower` | jaw | 아랫입술. 아래 입 테두리를 펼친다 | A·B·C·D |
| `mouth_corner_l` · `mouth_corner_r` | head | 입꼬리. 실밥을 따라 귀 뿌리까지 찢어지는 선의 끝점 | A·C·D |
| `jaw_02` | jaw | 아래턱 둘째 마디. 턱이 바닥까지 내려가는 길이를 고정 길이 뼈로 낸다 | C |
| `head_flap` | neck | 얼굴 앞부분 전체가 자루 뚜껑처럼 등 쪽으로 젖혀지는 판 | B |

- 뼈 스케일로 입을 늘려 커 보이게 하지 않는다(workflow.md "편의상 뼈 스케일로 길이를 늘려 … 통과시키지 않는다"). 입이 몸보다 커지는 부피는 **빈 돼지 메시 자체의 형태**로 만들고, Burst 동안의 열림은 위 뼈와 모프(또는 둘 중 하나)로 낸다. 모프와 뼈 가운데 무엇을 주로 쓸지는 짧은 시험에서 정한다(미확인).
- 입 안의 깊은 어둠은 형태(깊게 판 입 안 면)와 단색 짙은 재질로 낸다. 자체 발광은 쓰지 않는다(monster/README "자체 발광 눈은 기본에 넣지 않는다").

## 동물·탈 구조와 짧은 시험

- **유지할 외형·비율과 종별 차별점:** 평상시 돼지의 낮고 긴 분홍 몸·짧은 다리·큰 코·접힌 귀·말린 꼬리를 그대로 둔다. 바뀌는 것은 **입 하나**다 — "먹는 입이 몸보다 커진다"(story-draft 6.3 같은 동작의 뒤틀림). 닭(세로로 서며 날개 자리에서 긴 팔)·소(등이 갈라지고 뿔이 길어짐)와 연출이 겹치지 않는다.
- **본체 관절/이동 유형:** 네발. 빈 돼지는 입이 앞으로 무거워 머리를 낮게 끄는 걸음이 된다(추정). 속도는 평상시와 같다.
- **탈 구조:** 돼지의 탈은 "몸 전체"다(art-plan 4.5 연결 위치). 탈을 벗어 바닥에 떨어뜨리는 구조가 아니라, **같은 가죽이 입에서부터 찢어져 벌어진 채 그대로 붙어 있는 구조**를 제안한다(추정 — 닭의 천 탈 탈착과 달리 런타임 천 탈착이 필요 없다). 빈 돼지 메시는 평상시 메시와 정점 대응을 맞춰 Burst 첫 프레임에서 바꾼다(추정).
- **평상 모델의 이음매 한 곳(톤 기준 ④):** 평상시 돼지에 Burst가 열리는 자리의 실밥이 한 곳 보여야 한다(story.md "평상시 모델에 실밥이 보여야 한다"). 자리는 컨셉 후보마다 다르다(아래 프롬프트 절). **이 실밥은 재제작 `pig_a`에 넣어야 하는 것**이라, 이 카드는 위치만 제안하고 `pig_a` 카드가 받는다.
- **가장 어려운 2~3초:** Burst 2.0초(아래 "짧은 동작 시험").
- **시험 결론·남은 문제:** 아직 없다.

## 치수와 기술 계약

| 항목 | 값 및 상태 |
|---|---|
| 원본 크기 X×Y×Z (cm) | **미확인.** 평상시 `pig_a` 실측은 187.23×82.00×109.90cm(pig_a.md "제작 결과")이고 재제작 뒤에는 달라질 수 있다. 빈 돼지는 입을 벌린 자세에서 입 높이가 몸 높이(약 110cm)를 넘는다 — 시작점은 최고점 약 140~150cm(추정, 정면에서 입이 몸 실루엣보다 커 보이는 정도) |
| 런타임 크기·자동 배율 | 실제 cm로 만들고 액터 Scale은 1이다(monster/README) |
| 판정 반경/캡슐·출처 | `Radius` 45(같은 행 — 빈 것 열에 `Radius` 변경이 없다, 주인 6.3). 빈 돼지 `CatchRadius` 140 / `PounceLunge` 110 / `PounceRadius` 186 / `PounceWindup` 0.8(주인 6.3 B15 제안값). 몸보다 큰 입이 캡슐 45cm 밖으로 나가므로, 좁은 통로·문틀에서 시각 메시가 벽을 크게 뚫는지 검수해야 한다(monster/README "검수") |
| 피벗·정면·기준 포즈 | 네 발 사이 바닥 Z=0, UE +X 정면. `MeshYawOffset`은 카탈로그(0)와 pig.md 설명(−90)이 어긋나 미확인이다(art-plan 7절 13번) — 후보 ②로 새 리그를 만들면 0을 목표로 한다. 기준 포즈는 리그 후보에 따른다 |
| 충돌 소유자·메시 충돌 | 몬스터 액터가 소유한다. 메시 충돌은 필요 없다 |
| 원본·FBX·UE 목적 경로 | 원본 `ArtSource/monster/pig_hollow/pig_hollow.blend`(후보 ②면 재제작 `pig_a`와 한 blend에 둘 수도 있다 — 미확인), FBX `ArtSource/monster/pig_hollow/export/`(`SK_Pig_Hollow.fbx`, `AN_Pig_Burst.fbx`, `AN_Pig_Hollow<클립>.fbx`, `manifest.json`), UE `/Game/Monsters/PigA/`(같은 종 폴더 — 추정) — 초안 |
| 삼각형·재질 슬롯·텍스처·LOD | 5,000~15,000 tris, 목표 약 1만(평상시 `pig_a` 8,404). 입 안 깊은 면이 더해진다. Vertex Color 1슬롯 + 실밥 벌어짐 재질 파라미터(조짐 3단계 — 슬롯을 늘릴지 같은 재질의 파라미터로 둘지 미확인). LOD는 입 실루엣을 지킨다 — 초안 |
| Skeleton·뼈 수·최대 영향 뼈 수 | 후보 ①: `SKEL_Pig` 24 + 입 뼈 4~6 = 28~30. 후보 ②: 네발 규격 26 + 입 뼈 4~6 = 30~32(추정 — 돼지 꼬리가 4마디면 +3). 정점당 영향 뼈 최대 4(common.md), 입 둘레는 4가 필요할 가능성이 높다(추정) |
| 부착 기준·소켓/마커 소비처 | 입 안 기준점(Hollow Execute 접촉 검산 지점 — 소켓 이름 미확인). 탈 터지는 소리(감쇠 2100, 주인 6절 ⑫ 추정)의 원점은 액터 위치로 충분하다(추정) |

## 클립 / 상태별 변형

공통 규칙은 30fps, 제자리 이동, Root Motion 없음, 루프 첫·끝 포즈 일치다([common.md](C:\Work\CL9\docs\asset\common.md)). 길이의 출처는 art-plan 2.2 돼지 줄과 주인 6절이다. 평상시 돼지의 신규 클립(Eat·Sniff·Heavy Walk·Mouth Open Idle)은 이 카드가 아니라 `pig_a` 카드의 몫이다. 다만 **Mouth Open Idle(조짐)의 입 모양은 Burst 첫 프레임과 이어져야** 하므로 같이 설계한다.

| 이름 | 길이(초) | fps | 루프 | 루트 이동 | 트리거(상태) | 플레이어 짝 |
|---|---|---|---|---|---|---|
| `AN_Pig_Burst` | 2.0 | 30 | 아니오 | 없음 | 터짐(게이지 ≥ 문턱, `BurstTime` 2.0). 끝 프레임 = Hollow Idle 첫 프레임 | 없음 |
| `AN_Pig_HollowIdle` | 2.0(추정) | 30 | 예 | 없음, 발 고정 | 빈 돼지 대기. 벌어진 입이 숨 쉬듯 조금씩 여닫힌다(추정) | 없음 |
| `AN_Pig_HollowRun` | 미확인(추정 0.6) | 30 | 예 | 없음, 기준 속도 396cm/s(빈 것 `ChaseSpeed` = 평상시) | 빈 돼지 추격. 문 밖으로는 나가지 않으므로 문 밖 속도는 쓰지 않는다. 걷기 클립이 따로 필요한지 미확인 | 없음 |
| `AN_Pig_HollowExecute` | 2.0(평상시 `ExecuteDuration`과 같다고 읽었다 — 빈 것 열에 따로 없다, 미확인) | 30 | 아니오 | 없음 | 빈 돼지 처형(2인). 첫 0.3초는 카메라 블렌드 중 예비동작, 끝 자세 = 시체 옆에 선 자세(monster/README Execute 규칙) | 아래 "Hollow Execute" |
| `AN_Pig_HollowDeath` | 미확인(끝 유지) — 1.2(추정) | 30 | 아니오 | 없음 | HP 0(12~14, 사실상 처치 불가 — 주인 6.3). 끝 포즈 유지 | 없음 |

### Hollow Execute — 기존 처형 앵커 표를 재사용하나

지금의 앵커 표(`C:\Work\CL9\Tools\Blender\execute_anchor.py` `ANCHOR['Pig']`)는 몬스터를 피해자 정면 **+100cm**(`CatchRadius` 100, 잡힌 자리에서 순간 이동 없음), 요 180, 코끝 94cm, 앞 추력 15cm 이내, **접촉 0.50초**, 구간(예비 0~0.3 · 박치기 0.3~0.5 · 복귀 0.5~0.9 · 털기 1.0~1.6)으로 두고, 플레이어 짝 `AN_Farmer_Executed_Pig`(brace 0.40 · hit 0.50 · react 0.62 · land 1.10 · down 1.40, 끝 자세 DOWN)와 공유한다. 빈 돼지는 `CatchRadius`가 140이라 이 거리 가정이 맞지 않는다.

| 후보 | 무엇을 하나 | 얻는 것 | 비용 |
|---|---|---|---|
| 가. 기존 `Pig` 항목을 그대로 쓴다 | 몬스터 +100, 접촉 0.5초. 몸짓만 큰 입으로 바꾼다 | 플레이어 짝 클립을 그대로 쓴다 | 실제 잡힘 거리는 0~140cm인데 클립이 100을 가정한다. 입 앞점이 피해자 공간에서 어디에 오는지 다시 재야 한다 |
| 나. **새 항목 `PigHollow`를 더한다 — 추천** | 몬스터 +140, 요 180. **접촉 시각 0.50초와 플레이어 쪽 시간표는 `Pig`와 같게** 두고, 입 앞점·추력 상한만 새로 잰다 | 같은 접촉 시각을 지키면 `AN_Farmer_Executed_Pig`를 그대로 쓸 수 있다(추정). 잡힌 자리에서 순간 이동이 없다는 규칙도 지킨다 | 앵커 표에 항목 하나가 는다. 새 리그(리그 후보 ②)라면 몬스터 쪽 클립은 어차피 새로 만든다 |

추천은 **나**다. 거리 가정이 판정과 맞아야 접촉 검산(`contact_probe_cm`)이 뜻을 가진다. 동작은 두 갈래가 있다 — 지금 돼지처럼 **박치기**를 큰 입으로 하는 것과, 공격 논의안(motion-plan.md)의 **"벌어진 입으로 붙잡아 삼키기"**다. 삼키기는 끝 자세 "시체 옆에 선 자세"와 플레이어 짝의 끝 자세 DOWN(시체가 남는다)과 부딪칠 수 있어, 삼키는 척 물었다가 뱉어 쓰러뜨리는 모양이 필요한지 미확인이다. 이 카드는 동작을 고르지 않는다 — 처형 장면을 짧게 하는 판 루프 4.4 안과 함께 정할 일이다.

## 짧은 동작 시험 — Burst 2.0초

빈 돼지에서 가장 어려운 구간이다. 외형 검토를 마친 초안(평상시·빈 돼지 두 메시)과 최소 리그로 이 2초만 먼저 만든다([workflow.md](C:\Work\CL9\docs\asset\workflow.md) "어려운 약 2~3초부터"). 닭 Burst 1.8초(0-b)와 연출이 겹치지 않아야 한다.

| 구간(초, 추정) | 무엇 |
|---|---|
| 0.0~0.4 | 조짐 끝(Mouth Open Idle·씹는 입)에서 시작. 몸이 한 번 부르르 떤다. 평상 모델의 이음매(실밥)가 재질로 벌어지기 시작한다 |
| 0.4~0.9 | 실밥을 따라 입꼬리가 귀 쪽으로 찢어진다(`mouth_corner_*`, 실밥 벌어짐 재질). 머리가 뒤로 젖혀지며 윗입술이 들린다 |
| 0.9~1.5 | 입이 벌어지며 커진다. 약 1.3초에 정면 실루엣에서 입이 몸보다 커지는 순간이 온다(추정 — 이 장면이 컨셉 이미지의 그 순간이다). 입 안의 깊은 어둠이 보인다 |
| 1.5~2.0 | 커진 입의 무게로 앞이 내려앉고, 네 발을 다시 디딘다. 끝 프레임 = Hollow Idle 첫 프레임 |

- **볼 것:** 평상시 → 빈 돼지 메시 교체가 보이지 않는가(교체 프레임·정점 대응), 입이 커질 때 몸·다리·귀·꼬리가 평상시 외형을 지키는가(톤 기준 ③), 입 둘레 웨이트가 찢어지는 선에서 튀지 않는가, 입 안 면이 뒤집히거나 바깥 면을 뚫지 않는가, 몸보다 큰 입이 바닥을 뚫지 않는가(바닥 최저점), 1배속에서 2초가 읽히는가.
- **수치 검사:** 바닥 최저점(cm), root 이동(cm), 입 높이·폭의 프레임별 실측(몸 높이와의 비), 교체 프레임 전후 정점 위치 차(cm). 허용치는 시험 전에 정한다(미확인).
- **엔진 왕복:** 메시 교체(또는 한 메시의 모프)·추가 뼈·실밥 재질 파라미터가 UE에서 같은 2초로 재생되는지 작게 왕복한다. Blender의 모디파이어·드라이버가 FBX로 그대로 전달된다고 가정하지 않는다(workflow.md). 동물 클립은 **Interchange로만** 넣는다.
- **납품:** 1배속 MP4(정면·측면·3/4), 입 근접 영상, 평상시와 끝 프레임의 나란한 비교 시트, 우측 상단 표시(workflow.md 5절).

## 내보내기와 검수 기록

- Blender 버전 / exporter 이름·버전 / 축·단위·베이크 옵션:
- UE 버전 / importer / Skeleton 지정 / 임포트 옵션:
- 중립광·어두운 조명·와이어·클립 미리보기 경로:
- 몸–탈 / 탈 자체 / 손 집기 / 발 접지 / 외형 / 전환 QA를 구분한 결과: (탈 자체 = 찢어진 가죽의 자기 교차·뒤집힘, 전환 = Burst 메시 교체)
- 1배속 및 비교 영상 묶음, 우측 상단 제작 방식·버전·속도·렌더/인게임 표시:
- 엔진 임포트·프리뷰·실제 게임 상태 연결의 구분과 실행/로그/재열기 증거:
- 동작 영상의 실제 재생/디코딩·화면 검수 결과, 사용자에게 제시한 영상·시각·피드백:
- 크기·원점·전방·충돌·리그·루프 확인 결과:
- 게임 시나리오 / 2인 PIE / 성능 측정 결과:
- 미실행 항목과 이유: 전부 미실행이다 — 이 카드는 문서 단계(`phase: brief`)다.
- 알려진 문제·다음 수정:
- 재개 정보: 마지막 실제 완료 단계·편집 원본·다음 실행 명령·실행 중 프로세스 유무:

## 미확인·연결 필요 목록 (18건)

1. `pig_a` 재제작 결정이 decisions.md에 기록됐는지.
2. 빈 것에 별도 SubAlias가 드는지.
3. 재제작 `pig_a`가 기존 24뼈의 계층·레스트 포즈를 유지하는지(리그 후보 ①·②를 가른다).
4. UE 스켈레톤에 뼈를 더해도 기존 클립이 그대로 도는지(후보 ①).
5. 빈 돼지 원본 크기와 입 최고점.
6. `MeshYawOffset`(카탈로그 0 / pig.md −90).
7. 원본을 `pig_a`와 한 blend에 둘지.
8. 실밥 벌어짐을 재질 슬롯으로 둘지 파라미터로 둘지.
9. 입 기준점 소켓 이름.
10. 입 열림을 모프로 할지 뼈로 할지.
11. Hollow Run 한 주기 길이, 빈 돼지 걷기 클립이 따로 드는지.
12. 빈 돼지 `ExecuteDuration`(평상시 2.0과 같은지).
13. Hollow Execute의 동작(박치기 / 삼키기)과 삼키기일 때 시체가 남는 모양.
14. `PigHollow` 앵커의 입 앞점·추력 상한.
15. Hollow Death 길이.
16. Burst 메시 교체 방식(교체 / 한 메시의 모프).
17. Burst·수치 검사의 허용치.
18. Burst 동안의 게임 상태(무적인지, 움직이는지 — gameplay 몫).

## 컨셉 이미지 프롬프트

빈 돼지는 **"정체" 쪽 이미지만** 만든다. 평상시 쪽은 돼지 본체의 몫이고, 승인 때는 정체 이미지를 평상시 돼지와 나란히 놓고 본다. 후보는 넷이고, 프롬프트는 이미지 모델용 영어, 아래에 한국어 요지 한 줄을 붙였다. 이미지는 아직 만들지 않았다.

**나란히 볼 평상시 참고:** 지금은 `C:\Work\CL9\ArtSource\monster\pig_a\preview\neutral.png`(실물)과 `C:\Work\CL9\ArtSource\monster\pig_a_v2\preview\v2_neutral.png`(귀여움 1단계, 승인 대기)다. `pig_a`가 재제작되므로, 새 돼지 컨셉이 나오면 그것과 다시 나란히 본다. 각 후보의 "평상 모델 이음매" 자리는 새 돼지 컨셉에 그대로 넣어야 한 쌍이 맞는다.

톤 기준 넷(결정 #50)은 다음과 같이 본다. ① 평시 귀여움 — 평상시 돼지 이미지가 맡는다(이 카드 밖). ② 무서움은 정체 순간에만 — 이 이미지에만 무서움이 있다. 피·살·이빨 줄 없이 입 안의 깊은 어둠과 비율이 틀어진 몸으로 무섭게 한다. ③ 같은 몸의 뒤틀림 — "먹는 입이 몸보다 커진다"(story-draft 6.3). 몸·다리·귀·꼬리·분홍은 평상시와 같아야 하고, 나란히 놓았을 때 "아, 저게 그거였구나"가 나와야 한다. ④ 평상 모델에 이음매 한 곳 — 후보마다 자리를 적었다.

**공통 꼬리말** — `+ 공통 꼬리말` 자리에 붙인다:

```text
stylized low-poly game monster, chunky rounded shapes, flat-shaded large color areas, limited palette of saturated bubblegum pink plus near-black for the mouth interior, no realistic flesh, no blood, no gore, no rows of teeth, no tongue detail, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

### 후보 A — 귀까지 찢어진 입

```text
The same chubby saturated-pink low-poly pig with the same long low body, short stubby legs, big round snout, floppy folded ears and curly tail, but its mouth corners have torn open along a stitched seam line all the way back to the roots of its ears, the whole upper head tilted back so the open mouth seen from the front is wider and taller than the entire body, the inside of the mouth is a deep pitch-black hollow with no visible bottom, a few loose pink thread ends hanging at the torn corners, the body skin slightly slack as if empty inside, + 공통 꼬리말
```

요지: 같은 분홍 돼지의 입꼬리가 실밥을 따라 귀 뿌리까지 찢어지고, 머리가 뒤로 젖혀져 정면에서 입이 몸 전체보다 크며, 입 안은 바닥이 안 보이는 검은 구멍이다.

톤 기준: ② 무서움은 바닥 없는 입 안과 몸보다 큰 입에만 있다. ③ 먹는 입이 그대로 커졌고 나머지는 같은 돼지다. ④ **평상 모델 이음매: 양쪽 입꼬리에서 귀 뿌리까지 이어지는 가는 실밥 한 줄**(주인 6.3 조짐 3단계 "입꼬리가 귀까지 찢어져 보인다"와 같은 자리).

### 후보 B — 자루 입

```text
The same chubby saturated-pink low-poly pig with the same short legs, hind body and curly tail, but the whole front half of its head has opened like the mouth of a drawstring sack: the snout and face are folded back over its shoulders as one loose pink flap, revealing a huge round opening larger than the pig's body, the opening is a deep dark tunnel going far back inside the empty pink skin, loose pink thread loops along the rim of the opening, + 공통 꼬리말
```

요지: 같은 분홍 돼지의 머리 앞쪽 전체가 자루 주둥이처럼 열려 얼굴이 어깨 위로 젖혀지고, 몸보다 큰 둥근 구멍이 빈 가죽 깊숙이 어두운 굴처럼 이어진다.

톤 기준: ② 무서움은 빈 가죽 속으로 이어지는 굴에만 있다. ③ 몸·다리·꼬리는 같은 돼지이고 머리가 입으로 뒤집혔다 — 다만 얼굴이 뒤로 접혀 "그 돼지"로 읽히는 정도가 넷 가운데 가장 약할 수 있다(추정). ④ **평상 모델 이음매: 목둘레를 한 바퀴 도는 실밥**(주인 6.3 조짐 3단계 "목의 실밥이 벌어지고"와 같은 자리).

### 후보 C — 먹이통 턱

```text
The same chubby saturated-pink low-poly pig with the same long low body, short legs, floppy ears and curly tail, but its lower jaw has unhinged and dropped all the way down to the floor like a wide scoop or a feeding trough, the jaw sliding forward along the ground ahead of the body, the open mouth taller than the pig itself, the inside a deep dark hollow, the big round snout still on top, the torn seams under the chin showing loose pink threads, + 공통 꼬리말
```

요지: 같은 분홍 돼지의 아래턱이 빠져 먹이통 같은 넓은 삽처럼 바닥까지 내려가 앞으로 끌리고, 입이 돼지 키보다 높으며 안은 깊고 어둡다.

톤 기준: ② 무서움은 바닥을 쓸며 다가오는 턱과 어두운 입 안에만 있다. ③ 먹던 입이 먹이통이 됐다 — 돼지 농장의 먹이통(`Trough`)과 그림이 이어진다. 큰 코가 위에 남아 "그 돼지"로 읽힌다. ④ **평상 모델 이음매: 턱 아래를 가로지르는 실밥**.

### 후보 D — 풍선 입

```text
The same chubby saturated-pink low-poly pig, but its mouth has ballooned outward into a huge rounded pink dome-shaped maw much bigger than the rest of the body, the rim of the mouth stretched thin and glossy, the pig's normal small body, short stubby legs, floppy ears and curly tail dangling comically under and behind the giant mouth, the inside of the dome a deep dark hole, a ring of stitch marks stretched wide around the mouth rim, + 공통 꼬리말
```

요지: 같은 분홍 돼지의 입이 몸보다 훨씬 큰 둥근 풍선처럼 부풀고, 원래의 작은 몸·짧은 다리·귀·꼬리가 그 아래·뒤에 매달려 있으며, 둥근 입 안은 깊고 어둡다.

톤 기준: ② 무서움은 몸보다 큰 입과 그 속 어둠에 있지만, 작은 몸이 매달린 모양은 우스운 쪽으로 기울 수 있다(추정 — 넷 가운데 가장 덜 무서울 수 있다). ③ 입만 커지고 몸은 그대로라 같은 돼지로 가장 잘 읽힌다. ④ **평상 모델 이음매: 주둥이(코와 입) 둘레를 도는 실밥 한 바퀴**.

### 이미지 입력 조건 ([tools/tripo.md](C:\Work\CL9\docs\asset\tools\tripo.md) "입력 이미지 조건" 원문)

단일 오브젝트, 3/4 정면, 화면 중앙, 균일한 짙은 회색 배경, 바닥면 없음, 바닥 그림자 없음, 글자 없음. 이 조건을 프롬프트 끝에 그대로 적으면 Nano Banana가 지켰다.
스타일 지시(로우폴리·평면 셰이딩)는 이미지 모델이 반만 듣는다 — 형태와 색만 맞으면 된다. 톤은 Blender 재단이 맡는다.


<!-- free-motion-probe-20261019 -->
### 무료 모션 적용 시험 — 2026-10-19 영상 납품

- 사용자 요청: “ㅇㅇ 그럼 무료부터 받아서 돼지랑 다람쥐 거미에 적용해보고 영상으로 보여줘”. 범위는 기존 모델을 이용한 Blender 무료 모션 시험이다. 이전 탈 벗기·즉시 낙하·다람쥐 변신의 원본과 승인 이력은 보존한다.
- **영상:** [12초·사선/측면·30fps·1배속 비교](../../../ArtSource/monster/free_motion_probe_20261019/review/free_motion_pig_squirrel_spider_1x.mp4). 돼지 본체 대기 3초 → 돼지 저자세 이동 3초 → 다람쥐 이동 3초 → 거미 보행 3초. 실제 Blender 모델 렌더이며 출처·버전·1x·Blender 표시. 4개 개별 MP4와 비교 MP4 전체 디코드 통과, 실제 인코딩 프레임의 처음/중간/끝 화면 확인. [디코드 표본](../../../ArtSource/monster/free_motion_probe_20261019/review/encoded_motion_review.jpg).
- **무료 자료:** 기존 공식 Mixamo Running Crawl / Mutant Breathing Idle FBX 재사용. 제작자 공식 Quaternius Spider.blend 및 CC0 License.txt 신규 다운로드. `vendor/` 원본·라이선스·해시와 `download_manifest.json`, `source_motion_report.json` 보존. 추가 결제·애드온 설치 없음.
- **실제 적용:** 돼지 대기는 몸통·팔·고개의 원본 회전 변화, 저자세 이동은 네 끝점 경로를 사용했다. 다람쥐는 기존 짧은 발 제어에 모션 경로·몸 이동·작은 상하 움직임을 이식했다. 거미는 원본 8개 발 제어 경로를 현행 8다리×4관절에 고정 길이 FABRIK로 적용하고 실제 갈고리/발끝 메시 높이를 보정했다. Mixamo 30fps, 거미 원본 24fps를 시간 변경 없이 30fps로 표본화했다.
- **편집 파일:** `ArtSource/monster/free_motion_probe_20261019/{pig_idle,pig,squirrel,spider}_free_motion_v01.blend`. 시험 구간은 돼지 대기 F451–540, 돼지 이동 F301–390, 다람쥐/거미 F201–290. 파일에는 각120프레임 키가 있으며 영상은 앞90프레임씩 제시한다. 원본 메시·바인드 불변, 돼지 기존 F1–300 및 찬장 주인 F1–166 전체 뼈/제어/모프 비교 통과. 원본 파일 해시도 불변. `preservation_qa.json`에 범위와 수치 기록.
- **수정 이력:** 거미 첫 시험의 큰 주기 회전/발끝 침범은 거리 진폭·이전 자세 기반 풀이와 실제 발끝 높이 보정으로 수정했다. 최종 표본 최저 높이 약3.77mm, 프레임당 관절 회전 최대14.04도. 다람쥐 시험 파일에서 새 키 추가로 기존 희소 꼬리 모프가 구간 사이에 보간되던 현상은 기존 F1–166을 동일 포즈/모프 키로 복원했고, 새 시험 구간 차이0 확인. 실패 렌더/로그는 작업 폴더에 남겼다.
- **제한/판정:** 돼지 저자세 이동의 왼팔 완전 접지는 실패이므로 영상에도 표시했고 채택 완료로 올리지 않는다. 다람쥐는 기존 발/탈 제어를 이용한 제한적 시험이며 새 완전 동물 리그가 아니다. 거미도 전체 발 미끄러짐·자기 충돌·새 탈착·성장 연결 합격은 아니다. 기존 돼지 후면 메시 결함 등은 이번 모션 시험에서 재조형하지 않았다. Unreal·추가 유료 구매·기존 게임 에셋 수정 없음.
- [납품 manifest](../../../ArtSource/monster/free_motion_probe_20261019/delivery_manifest.json). `phase: motion_probe`, `status: review_ready`, `job: null`. 다음 단계는 시험본 시각 판단에 따라 유효한 모션만 선택해 접지·전환을 보강하는 것이다. 도구는 `Tools/Blender/*free_motion*20261019.py`와 `repair_squirrel_probe_rotation_modes_20261019.py`에 보존했다.
<!-- /free-motion-probe-20261019 -->


<!-- free-motion-unreal-20261019 -->
### 무료 모션 Unreal 에셋 준비 완료 — 2026-10-19 V02

- 사용자 요청: “언리얼에 사용할수잇게 만들어줘”. **직전 무료 모션 시험의 돼지 본체·다람쥐 닫힌 탈·거미 본체와 4클립**을 UE5.8.3 native 에셋으로 준비했다. 기존 탈 벗기/성장 시퀀스의 UE 구현이나 기존 몬스터 AI 교체로 확대하지 않았다.
- **현재 콘텐츠:** `/Game/Monsters/FreeMotion20261019/V02`. PigBody/Squirrel/Spider의 SkeletalMesh·Skeleton 3세트, BC/N/R 2K atlas·1재질, 모델별 LOD0/1/2, 애니메이션 4개, 재생용 BP 4개, `QA_FreeMotion` 맵 포함. 이전 V01은 최초 임포트/화면 실패 증거로 보존하며 V02가 현재 납품이다.
- **바로 배치할 BP:** `PigBody/BP_PigBody_Idle`, `PigBody/BP_PigBody_Crawl`, `Squirrel/BP_Squirrel_Scurry`, `Spider/BP_Spider_Walk`. Native SkeletalMeshActor 상속, SingleNode Animation, PlayRate=1·Looping/Playing=true. 테스트 맵과 ActorScale=1로 검증했다. 기존 게임 액터·테이블·AnimBP·충돌/AI 판정은 수정하지 않았다. 배치용 BP의 충돌은 꺼져 있다.
- **클립:** `AN_PigBody_Idle` 4초, `AN_PigBody_Crawl` 2초, `AN_Squirrel_Scurry` 2초, `AN_Spider_Walk` 2.5초. 제자리 모션이며 루프 닫힘 포즈를 명시적으로 추가했다. Root travel은 다람쥐에서 제거했다. BlendShape/천/GN 계산은 이번 닫힌 상태 메시의 기준 형태로만 고정했으며 새 탈착 런타임을 제공하는 것이 아니다.
- **메시/텍스처:** LOD0 삼각형 돼지50,000·다람쥐30,000·거미35,000; LOD1 약50%, LOD2 약25%. Blender 전용 재질을 BC/Normal/Roughness에 베이크, UE tangent normal Green 반전 설정. 기존 원본 메시/클립 파일은 해시 불변. 출력 복사본의 병합·UV atlas·삼각화·LOD 감소는 원본 재조형이나 외형 승인으로 표시하지 않는다.
- **보정/실패 해결:** 돼지 왼팔의 고정 월드-X 목표를 어깨 상대 목표로 바꾸고 골반을25cm 낮춰 네 지지를 풀이했다. 실제 손·발 표면 목표 높이 최대 오차1.384cm, 오른손/양발은1mm 이내. 다람쥐의 뒤집힌 닫힌 부품736면은 V02 내보내기 사본에서 법선을 재계산했고, 3모델의 닫힌 연결 성분 역방향 면0 확인. 초기 에디터 스크린샷 타임아웃은 실제 Simulate In Editor의 SceneCapture2D로 촬영 경로를 바꿔 해결했다.
- **엔진 QA:** Interchange 임포트 유지, 뼈 위치 표본 최대 오차 약0.07195cm(0.72mm), 클립 길이 일치, 3 LOD 임포트. 맵 저장 후 다른 레벨을 거쳐 재열기하여 메시/클립 참조·자동 반복/배속 유지 확인. Simulate In Editor에서 수동 포즈/시간 변경 없이 약5.7초 자동 재생 관찰: 각 클립1~3회 루프 전환, 실제 소켓 이동(머리/손/앞발/거미발) 확인. 원본 본체/탈·pig_a·다른 동물 에셋은 보존했다.
- **실제 엔진 화면:** [V02 화면](../../../ArtSource/monster/free_motion_probe_20261019/unreal_v02/review/unreal_models_review_v02.jpg). 기본 LOD 4장과 최저 LOD 3장을 SceneCapture2D로 촬영·확인했다. Blender 렌더/실시간 FPS 영상으로 표시하지 않는다. [납품 파일·사용 위치·QA](../../../ArtSource/monster/free_motion_probe_20261019/unreal_v02/delivery_manifest.json).
- **검수 한계:** 기존 돼지 후면 잔여 조형과 본체 외형 미승인 이력은 남아 있다. 다람쥐는 현재의 제한적인 발/탈 리그이다. 새 전체 탈 벗기/성장 연결, 전신 자기 충돌, 다수 몬스터 성능, 기존 AI/AnimBP 전환·멀티플레이 게임 검수는 이번 준비 범위가 아니다.
- **재개 파일:** `ArtSource/monster/free_motion_probe_20261019/unreal_v02/{PigBody,Squirrel,Spider}/`의 `*_UE.blend`, `export/` FBX·텍스처와 `export_manifest.json`. 도구 `Tools/Blender/{fix_pig_crawl_for_ue,export_free_motion_ue,audit_free_motion_normals}_20261019.py`, `Tools/Unreal/*free_motion*20261019.py`. PigBody/Spider는 검증한 V01 내보내기 재사용, Squirrel은 V02 법선/베이크 재실행이다.
- `phase: engine_verify`, `status: complete_for_requested_asset_preparation`, `job: null`. 에디터는 최초 `/Game/Maps/Lvl_Flat` 복원·PIE 종료·dirty map0·이번 공유 임대/PIE 잠금 해제. 콘텐츠 브라우저에는 현재 BP들을 표시했다. 다음 작업은 사용자가 요청하는 실제 소비 액터/게임 상태 연결 범위에서 시작한다.
<!-- /free-motion-unreal-20261019 -->
