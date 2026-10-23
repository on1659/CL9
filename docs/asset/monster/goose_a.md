# Goose A — 길목의 경보꾼

## v02 완료 — 재질·표면·모션·실제 게임 검수

- 현재 12개 모션과 원본·FBX·정상1배속 영상: [현재 manifest](../../../ArtSource/monster/goose_a/animation_hybrid_v02/manifest.json).
- [5종52클립 납품·검증](../../../ArtSource/monster/concept_batch_1/hybrid_finish_v02/delivery.json) · [검토 화면](../../../ArtSource/monster/concept_batch_1/hybrid_finish_v02/index.html).
- 모든 필수 증거의 실제 파일·해시·통과 값을 확인했다. 사용자 외형 승인을 별도로 주장하지 않는다. 아래 v01 기록과 한계는 이전 이력이다.


## 이전 v01 체크포인트 — 혼합 동물 리그·모션과 엔진 프리뷰

- 종별 리그와 11개 동작을 제작했다: Idle, Walk, Run, LookAround, WakeTelegraph, HitReact, PushReact, Death, Sleep, AlertWindup, Honk.
- [5종 모션 검토 화면](../../../ArtSource/monster/concept_batch_1/hybrid_animation_v01/index.html) · [편집 원본](../../../ArtSource/monster/goose_a/animation_hybrid_v01/goose_a_Hybrid_Animated.blend) · [동작별 출력·1배속 영상](../../../ArtSource/monster/goose_a/animation_hybrid_v01/manifest.json).
- UE 임포트·포즈 왕복, 복사 검토 맵 저장/재열기, 실제 SIE의 네이티브 애니메이션 재생을 확인했다. AI/AnimBP 상태 연결 완료를 뜻하지 않는다.
- 남은 사항: 게임 이동 속도와 재생 속도 보정, 실제 상태 연결, 원본 표면 마감 및 플레이어 접촉 검수. 별도의 사용자 외형 승인으로 기록하지 않는다.

## 이전 체크포인트 — 혼합 동물 정지 모델·UE 배치

- 이번 요청 범위 완료: 실제 정지 3D 외형·FBX·언리얼 복사 맵 배치·저장 후 재열기·실제 엔진 캡처. 사용자 3D 외형 승인은 별도이며 리그/애니메이션/AI 연결은 하지 않았다.
- 혼합 컨셉과 강도 승인 근거: “이정도로해서 배치해봐” → “실제 3D 모델을 만들어 언리얼 맵에 배치”.
- [현재 원본](../../../ArtSource/monster/goose_a/model_hybrid_v01/placement_unified/goose_a_Hybrid.blend) · [모델 QA](../../../ArtSource/monster/goose_a/model_hybrid_v01/placement_unified/qa.json) · [5종 검수 화면](../../../ArtSource/monster/concept_batch_1/hybrid_placement_v01/index.html).
- UE 맵: `/Game/Tests/HybridFiveV01/Lvl_HybridFiveV01`. 메시: `/Game/Monsters/HybridFiveV01/goose_a/SM_goose_a_Hybrid.SM_goose_a_Hybrid`. 초안 외형 높이 1.55 m, 45,000 triangles. 게임 판정 크기를 변경하지 않았다.
- 실제 원본 GLB와 이전 모델을 보존했다. 아래 기존 기록은 과거 버전 이력이다.
- 남은 한계: 최종 표면·노멀·그림자 마감. 검토용 양면 재질과 모델 그림자 투영 비활성 설정을 사용했다. 쥐 몸통의 거친 면, 양 얼굴 경계와 거위 목의 국부 면 정리가 남는다.

```json
{
  "concept_pipeline": {
    "schema_version": 2,
    "asset_id": "goose_a",
    "scope": "production",
    "requested_outputs": [
      "finished surface topology",
      "matching CL9 vertex-color materials",
      "Idle",
      "Walk",
      "Run",
      "LookAround",
      "WakeTelegraph",
      "HitReact",
      "PushReact",
      "Death",
      "Sleep",
      "AlertWindup",
      "Honk",
      "HonkMove",
      "speed matched gait",
      "actual state and player contact integration",
      "host/client gameplay verification",
      "normal-speed videos"
    ],
    "phase": "complete",
    "status": "complete",
    "updated_at": "2026-10-20T17:34:19.663843+00:00",
    "review_policy": "user_directed",
    "review_override": "2026-10-21 user: 제대로 다해줘. 재질도 맞추고. Finish the displayed five including remaining materials, topology/contact, real speed-matched gaits, state integration and actual gameplay verification.",
    "concept": {
      "approved_version": "hybrid_placement_v01/approved_concept.png"
    },
    "blender": {
      "candidate_version": "model_hybrid_v01/placement_unified",
      "approved_version": null
    },
    "production": {
      "manifest": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/delivery.json",
      "qa": {
        "engine_import": {
          "status": "pass",
          "evidence": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/engine_import.json"
        },
        "mesh_assets": {
          "status": "pass",
          "evidence": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/mesh_assets_qa.json"
        },
        "palette": {
          "status": "pass",
          "evidence": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/palette_parity_qa.json"
        },
        "native_tests": {
          "status": "pass",
          "evidence": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/runtime_tests_qa.json"
        },
        "runtime_profiles": {
          "status": "pass",
          "evidence": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/runtime_profiles.json"
        },
        "gameplay_video": {
          "status": "pass",
          "evidence": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/gameplay_video_qa.json"
        },
        "primary_runtime": {
          "status": "pass",
          "evidence": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/runtime_playback_qa.json"
        },
        "paired_runtime": {
          "status": "pass",
          "evidence": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/paired_runtime_qa.json"
        },
        "surface_visual": {
          "status": "pass",
          "evidence": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/surface_visual_qa.json"
        },
        "contact_visual": {
          "status": "pass",
          "evidence": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/contact_visual_qa.json"
        },
        "gameplay_visual": {
          "status": "pass",
          "evidence": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/gameplay_visual_qa.json"
        },
        "saved_reopened": {
          "status": "pass",
          "evidence": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/saved_reopened_qa.json"
        },
        "model_rig": {
          "status": "pass",
          "evidence": "ArtSource/monster/goose_a/animation_hybrid_v02/manifest.json"
        },
        "animation": {
          "status": "pass",
          "evidence": "ArtSource/monster/goose_a/animation_hybrid_v02/manifest.json"
        },
        "modelQA": {
          "status": "pass",
          "evidence": "ArtSource/monster/goose_a/animation_hybrid_v02/manifest.json"
        }
      },
      "runtime": {
        "level": "actual_host_client_gameplay",
        "gameplay_connected": true,
        "asset_paths": [
          "/Game/Monsters/HybridFiveFinishV02/goose_a/Build_420a41b411/SK_goose_a_HybridV02.SK_goose_a_HybridV02",
          "/Game/Monsters/HybridFiveFinishV02/goose_a/Build_420a41b411/SK_goose_a_HybridV02_Skeleton.SK_goose_a_HybridV02_Skeleton",
          "/Game/Monsters/HybridFiveFinishV02/goose_a/Build_420a41b411/Clips_f79d50beb6/AN_goose_a_Hybrid_Idle.AN_goose_a_Hybrid_Idle",
          "/Game/Monsters/HybridFiveFinishV02/goose_a/Build_420a41b411/Clips_ed1f4db08a/AN_goose_a_Hybrid_Walk_004.AN_goose_a_Hybrid_Walk_004",
          "/Game/Monsters/HybridFiveFinishV02/goose_a/Build_420a41b411/Clips_914021934c/AN_goose_a_Hybrid_Run_003.AN_goose_a_Hybrid_Run_003",
          "/Game/Monsters/HybridFiveFinishV02/goose_a/Build_420a41b411/Clips_7e0698bcd8/AN_goose_a_Hybrid_LookAround.AN_goose_a_Hybrid_LookAround",
          "/Game/Monsters/HybridFiveFinishV02/goose_a/Build_420a41b411/Clips_a94ba6f387/AN_goose_a_Hybrid_HitReact.AN_goose_a_Hybrid_HitReact",
          "/Game/Monsters/HybridFiveFinishV02/goose_a/Build_420a41b411/Clips_680b6a426f/AN_goose_a_Hybrid_WakeTelegraph.AN_goose_a_Hybrid_WakeTelegraph",
          "/Game/Monsters/HybridFiveFinishV02/goose_a/Build_420a41b411/Clips_b73df98868/AN_goose_a_Hybrid_Death_003.AN_goose_a_Hybrid_Death_003",
          "/Game/Monsters/HybridFiveFinishV02/goose_a/Build_420a41b411/Clips_5630ba6213/AN_goose_a_Hybrid_Sleep.AN_goose_a_Hybrid_Sleep",
          "/Game/Monsters/HybridFiveFinishV02/goose_a/Build_420a41b411/Clips_27a53d5045/AN_goose_a_Hybrid_AlertWindup.AN_goose_a_Hybrid_AlertWindup",
          "/Game/Monsters/HybridFiveFinishV02/goose_a/Build_420a41b411/Clips_da7867fc0e/AN_goose_a_Hybrid_Honk.AN_goose_a_Hybrid_Honk",
          "/Game/Monsters/HybridFiveFinishV02/goose_a/Build_420a41b411/Clips_b9901a2a5c/AN_goose_a_Hybrid_PushReact.AN_goose_a_Hybrid_PushReact",
          "/Game/Monsters/HybridFiveFinishV02/goose_a/Build_420a41b411/Clips_6cb345ed40/AN_goose_a_Hybrid_HonkMove_002.AN_goose_a_Hybrid_HonkMove_002"
        ],
        "evidence": {
          "engine_import": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/engine_import.json",
          "mesh_assets": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/mesh_assets_qa.json",
          "palette": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/palette_parity_qa.json",
          "native_tests": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/runtime_tests_qa.json",
          "runtime_profiles": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/runtime_profiles.json",
          "gameplay_video": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/gameplay_video_qa.json",
          "primary_runtime": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/runtime_playback_qa.json",
          "paired_runtime": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/paired_runtime_qa.json",
          "surface_visual": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/surface_visual_qa.json",
          "contact_visual": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/contact_visual_qa.json",
          "gameplay_visual": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/gameplay_visual_qa.json",
          "saved_reopened": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/saved_reopened_qa.json"
        }
      },
      "source_blender_version": "model_hybrid_v01/placement_unified",
      "candidate_version": "animation_hybrid_v02",
      "motion_plan": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/plan.json",
      "delivery_status": "complete",
      "species_manifest": "ArtSource/monster/goose_a/animation_hybrid_v02/manifest.json",
      "previous_delivery": "ArtSource/monster/concept_batch_1/hybrid_animation_v01/delivery.json",
      "current_manifest": "ArtSource/monster/goose_a/animation_hybrid_v02/manifest.json",
      "modelQA": {
        "status": "pass",
        "evidence": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/delivery.json"
      }
    },
    "checkpoint": {
      "path": "ArtSource/monster/goose_a/animation_hybrid_v02/goose_a_Hybrid_Finished_v02.blend",
      "sha256": "ce1e04cf9bad99b383cd1af83e9a74a0ffe905671b4de1cfa54657094cd211a3"
    },
    "job": null,
    "last_completed": "V02 기존 혼합 외형 마감·정점색 재질·12개 모션·엔진 및 실제 host/client 검수와1배속 영상 완료. 별도 사용자 외형 승인을 뜻하지 않음.",
    "next_action": "새 사용자 요청에 따라 현재 v02 체크포인트에서 재개",
    "blocker": null,
    "previous_requested_outputs": [
      "species-specific rig",
      "Idle",
      "Walk",
      "Run",
      "LookAround",
      "WakeTelegraph",
      "HitReact",
      "PushReact",
      "Death",
      "Sleep",
      "AlertWindup",
      "Honk",
      "separate animation FBX",
      "normal-speed motion videos",
      "Unreal import and playback verification"
    ],
    "delivery_history": [
      {
        "captured_at": "2026-10-20T17:34:19.663843+00:00",
        "phase": "model_rig",
        "status": "working",
        "production": {
          "manifest": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/delivery.json",
          "qa": {
            "static_model": {
              "status": "pass",
              "evidence": "ArtSource/monster/goose_a/model_hybrid_v01/placement_unified/qa.json"
            },
            "engine": {
              "status": "native_preview_pass",
              "evidence": "ArtSource/monster/concept_batch_1/hybrid_animation_v01/native_final_qa.json"
            },
            "model_rig": {
              "status": "pass_with_recorded_surface_limitations",
              "evidence": "ArtSource/monster/goose_a/animation_hybrid_v01/qa.json"
            },
            "animation": {
              "status": "authored_and_video_reviewed",
              "evidence": "ArtSource/monster/goose_a/animation_hybrid_v01/manifest.json"
            },
            "engine_visual": {
              "status": "sampled_review_with_documented_limitations",
              "evidence": [
                "ArtSource/monster/concept_batch_1/hybrid_animation_v01/engine_visual_rabbit.json",
                "ArtSource/monster/concept_batch_1/hybrid_animation_v01/engine_visual_sheep_goose.json",
                "ArtSource/monster/concept_batch_1/hybrid_animation_v01/engine_visual_rat_raccoon.json"
              ]
            },
            "gameplay": {
              "status": "not_connected",
              "limitations": [
                "Rabbit and rat bespoke states/controllers are not implemented. Author/import/replay their clips; do not claim gameplay integration or implement new game rules silently.",
                "Rabbit Headbutt .4s and generic Catch .8s differ. Preserve authored timing and explicitly map any preview hold without changing gameplay values.",
                "Rabbit HopPause must play only at actual stationary phase; constant 180cm/s motion during .6s pause would visibly slide 108cm.",
                "Goose AlertWindup/Honk and raccoon Freeze/Retreat have internal state branches without all required public animation getters.",
                "Raccoon Execute requires checking current hybrid contact points against the existing victim anchor; prior model's contact QA cannot transfer."
              ]
            }
          },
          "runtime": {
            "level": "native_animation_preview",
            "engine": "Unreal Engine 5.8",
            "map": "/Game/Tests/HybridFiveMotionV01/Lvl_HybridMotion_final",
            "asset_paths": [
              "/Game/Monsters/HybridFiveMotionV01/goose_a/Build_b396efc2bf_Normals/SK_goose_a_Hybrid.SK_goose_a_Hybrid",
              "/Game/Monsters/HybridFiveMotionV01/goose_a/Build_b396efc2bf_Normals/Clips_c34e281e03/AN_goose_a_Hybrid_Idle.AN_goose_a_Hybrid_Idle",
              "/Game/Monsters/HybridFiveMotionV01/goose_a/Build_b396efc2bf_Normals/Clips_59add2a485/AN_goose_a_Hybrid_Walk.AN_goose_a_Hybrid_Walk",
              "/Game/Monsters/HybridFiveMotionV01/goose_a/Build_b396efc2bf_Normals/Clips_0ab032b2bf/AN_goose_a_Hybrid_Run.AN_goose_a_Hybrid_Run",
              "/Game/Monsters/HybridFiveMotionV01/goose_a/Build_b396efc2bf_Normals/Clips_bbfe596b83/AN_goose_a_Hybrid_LookAround.AN_goose_a_Hybrid_LookAround",
              "/Game/Monsters/HybridFiveMotionV01/goose_a/Build_b396efc2bf_Normals/Clips_089b32dc1e/AN_goose_a_Hybrid_HitReact.AN_goose_a_Hybrid_HitReact",
              "/Game/Monsters/HybridFiveMotionV01/goose_a/Build_b396efc2bf_Normals/Clips_59acfa4738/AN_goose_a_Hybrid_WakeTelegraph.AN_goose_a_Hybrid_WakeTelegraph",
              "/Game/Monsters/HybridFiveMotionV01/goose_a/Build_b396efc2bf_Normals/Clips_0250575d13/AN_goose_a_Hybrid_Death.AN_goose_a_Hybrid_Death",
              "/Game/Monsters/HybridFiveMotionV01/goose_a/Build_b396efc2bf_Normals/Clips_f4d0402b95/AN_goose_a_Hybrid_Sleep.AN_goose_a_Hybrid_Sleep",
              "/Game/Monsters/HybridFiveMotionV01/goose_a/Build_b396efc2bf_Normals/Clips_6d8fdadaaa/AN_goose_a_Hybrid_AlertWindup.AN_goose_a_Hybrid_AlertWindup",
              "/Game/Monsters/HybridFiveMotionV01/goose_a/Build_b396efc2bf_Normals/Clips_84cb688633/AN_goose_a_Hybrid_Honk.AN_goose_a_Hybrid_Honk",
              "/Game/Monsters/HybridFiveMotionV01/goose_a/Build_b396efc2bf_Normals/Clips_ffd1cd6c86/AN_goose_a_Hybrid_PushReact.AN_goose_a_Hybrid_PushReact"
            ],
            "gameplay_connected": false
          },
          "source_blender_version": "model_hybrid_v01/placement_unified",
          "candidate_version": "animation_hybrid_v02",
          "motion_plan": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/plan.json",
          "delivery_status": "full_finish_working",
          "species_manifest": "ArtSource/monster/goose_a/animation_hybrid_v01/manifest.json",
          "previous_delivery": "ArtSource/monster/concept_batch_1/hybrid_animation_v01/delivery.json"
        },
        "checkpoint": {
          "path": "ArtSource/monster/goose_a/animation_hybrid_v01/goose_a_Hybrid_Animated.blend",
          "sha256": "3b743af7a8c0b118e9f08a2a3ad2ce009bf9679c340e1e2252b8b25f5af769d8"
        },
        "last_completed": "V01 rigs/51 clips and native preview retained; current game material/behavior definitions inspected. Full-finish revision in progress."
      }
    ],
    "appearance_approval": "not_claimed"
  }
}
```


## 2026-10-20 v01 반려 및 v02 재제작

- 사용자: “저거 다 이상해 다시만들어”. `model_round_v01` 5종 모두 반려, 원본은 비교용으로 보존한다.
- 현재 `production / blender_review`, 상태 `awaiting_user`. 새 후보는 `model_reference_v02/`. v01은 현재 승인 대기본이 아니다.
- 기존 도형 조합의 분리된 얼굴/팔다리와 시안 불일치를 바로잡기 위해 원래 시안 기반 입력 → 이미지 기반 3D 생성 → Blender 다각도 보정 경로를 검증한다.
- 양·토끼·생쥐·거위·너구리 5종만 제작. 사용자 답변에 따라 어미닭·횃대닭은 참고로만 보관. 기존 소·닭 탈·돼지 보존.

- v02의 실제 체크포인트와 검수 결과는 아래에 기록한다. 사전 10분 조사는 완료된 기록을 유지한다.

> 아래 이력의 이전 체크포인트는 이번 혼합 동물 배치 기록으로 대체되었습니다.

<details><summary>이전 파이프라인 기록</summary>

```yaml
concept_pipeline:
  schema_version: 2
  asset_id: goose_a
  scope: production
  phase: blender_review
  status: awaiting_user
  updated_at: 2026-10-20T04:56:15+00:00
  blender:
    candidate_version: model_reference_v02
    approved_version: null
  pending_review: {stage: blender, version: model_reference_v02, evidence: ArtSource/monster/goose_a/model_reference_v02/final_closed_candidate/preview}
  checkpoint: {path: ArtSource/monster/goose_a/model_reference_v02/final_closed_candidate/goose_a.blend, sha256: de7a9326cdc29803b25bfe830287dee9c94aff48b18c0051bd4045b715bd856e}
  last_completed: "원래 시안 기반 새 3D 생성·Blender 외형 보정·저장/재열기·실제 4뷰 직접 확인"
  next_action: "v02 실제 외형에 대한 사용자 검토 후 수정 또는 어려운 변형 시험·리그·애니메이션·UE 검수"
```

</details>

## 2026-10-20 v02 실제 Blender 외형 후보

- 새 실제 원본: [goose_a.blend](../../../ArtSource/monster/goose_a/model_reference_v02/final_closed_candidate/goose_a.blend), SHA256 `de7a9326cdc29803b25bfe830287dee9c94aff48b18c0051bd4045b715bd856e`.
- [실제 네 방향 렌더](../../../ArtSource/monster/goose_a/model_reference_v02/final_closed_candidate/preview/three_quarter.png) · [제작/재열기 기록](../../../ArtSource/monster/goose_a/model_reference_v02/final_closed_candidate/manifest.json) · [5종 비교 화면](../../../ArtSource/monster/concept_batch_1/model_review_v02/index.html).
- 방식: 기존 6종 4상태 시안의 해당 정상형을 단독 2D 입력으로 정리한 뒤 로컬 TRELLIS.2에서 3D 생성, Blender에서 검사·필요한 오류 보정. `input.png`는 입력 그림이고 `preview/*.png`는 실제 저장 모델의 렌더다. 원래 v01과 생성 원본 GLB를 보존한다.
- 생성 조건: 512 / seed 43 / 12 steps / 목표 50,000 faces / 2,048 texture. 축 +X 정면/+Z 위, 단위 m. 후보 크기는 외형 비교용이며 게임 판정을 변경하지 않았다.
- 실측 메시 기록: `{"mesh_objects": 1, "vertices": 163273, "triangles": 326550, "material_slots": 1, "uv_layers": {"Goose_Clean_Sleeping_Normal": ["UVMap"]}, "bounds_min_m": [-0.7877485752105713, -0.8204062581062317, 0.0], "bounds_max_m": [0.7910647988319397, 0.8201357126235962, 1.3277812004089355], "bounds_size_m": [1.578813374042511, 1.6405419707298279, 1.3277812004089355]}`.
- 정면·측면·후면·사선에서 둥근 몸, 접힌 목과 감긴 눈, 넓은 날개와 두 발을 확인했다.
- 등의 찢김이 심한 첫 생성본을 제외하고 seed 43으로 다시 생성했다. 날개 밑 잘못된 돌기 네 개와 뒷면의 열린 틈을 제거했다.
- 부리·눈 색 경계의 작은 계단과 잔표면이 남는다. 326,550 tris의 닫힌 외형 후보이며 게임용 토폴로지와 LOD 정리가 필요하다.
- 검증 범위: 실제 파일 저장·재열기, 텍스처 포함 여부, 정면·측면·후면·사선 렌더 직접 확인. 이는 사용자 승인이나 변형·게임 성능 합격이 아니다.
- 미검증: 변형용 토폴로지/리그, 행동·이족 광폭화·전환, FBX/UE, LOD와 게임 성능. 기존 소·닭 탈·돼지는 수정하지 않았고, 어미닭·횃대닭은 참고만 유지한다.



## 이력 — 2026-10-20 반려된 둥근 정상형 후보 model_round_v01

당시 `scope: production` · `phase: blender_review`의 외형 후보였으나 사용자 “저거 다 이상해 다시만들어”로 **반려됐다**. 현재 승인 대기본이나 v02 완료 체크포인트가 아니다. [조사 안내](../../../ArtSource/monster/research_20261020/README.md)와 [리그·경로 조사](../../../ArtSource/monster/research_20261020/rig_workflow.md)는 완료된 사전 조사이며 아래 파일·수치는 비교 이력으로 보존한다.

- 기준: `ArtSource/monster/concept_batch_1/sheet_six_species_four_states_v5.png` 정상형. 통통한 흰 몸, 앞에서 올라 등(-X)으로 접힌 둥근 목과 뒤를 향한 작은 머리·닫힌 주황 부리, 감은 눈, 큰 덩이 깃, 넓적 물갈퀴.
- 새 원본: [goose_round_v01.blend](../../../ArtSource/monster/goose_a/model_round_v01/goose_round_v01.blend). 재현 스크립트·manifest·qa.json·[QA](../../../ArtSource/monster/goose_a/model_round_v01/qa.md)는 같은 폴더.
- 네 방향: [대표 뒤 사선](../../../ArtSource/monster/goose_a/model_round_v01/preview/three_quarter.png), [앞](../../../ArtSource/monster/goose_a/model_round_v01/preview/front.png), [옆](../../../ArtSource/monster/goose_a/model_round_v01/preview/side.png), [뒤](../../../ArtSource/monster/goose_a/model_round_v01/preview/back.png). Blender 5.2.2 background CPU Cycles, 2 threads/20 samples/640px, 저장 후 재열기 렌더.
- 목은 연속 quad tube로 저작하고 접합을 부드럽게 정리했다. 초기 부리 가림·잘못된 수면 방향·목 안쪽 구멍·손가락 같은 발은 직접 렌더를 보고 수정했다. 최종 수치 정본은 후보 폴더 `qa.json`.
- 후보 수치: 24 mesh objects, 6,676 vertices, **13,256 tris**, 전체 bounds **1.355483×1.133796×1.324738m**(X×Y×Z), 최저 Z=0. 같은 v01에서 초기 110,976 tris를 부위별 decimate하여 10~15k 예산에 맞췄고, 큰 흰 덩이에 은은한 면이 읽히게 했다. 개별 메시 nonmanifold/zero-area 0이며 메시 간 교차·변형·게임 성능 합격은 아니다.
- 기존 아래 게임 연결본/원본/11개 클립은 보존했다. 반려된 v01은 armature·애니메이션·FBX·UE 임포트/게임 검수 **미실행** 이력이다. v01의 승인 대기를 이어가지 않으며 상단의 v02 외형 재제작부터 진행한다.

## 기존 게임 연결본

상태: 게임 연결 검수됨 — 2026-09-11 `MonsterTable.goose` → SK/`ABP_Goose_A`(Catch 슬롯 = Honk, Sleep 슬롯 = `AN_Goose_Sleep`), Lvl_Flat PIE 스폰·접지·걷기; 09-15 PushReact. 클립 11개(`Content/Monsters/GooseA/`). `AlertWindup` 은 소비 상태 없음, 2인 PIE·LOD 미검수 · 작성일: 2026-09-11 · 변경일: 2026-09-21

2026-09-11 · 신규 제작. 닭 임시 메시를 교체할 독립 외형과 종별 리그.

- 긴 S자 목, 작은 머리와 넓적한 주황 부리, 크고 낮은 흰 몸통, 넓은 물갈퀴.
  목을 세우는 예고와 앞으로 뻗으며 우는 동작이 멀리서 읽혀야 한다.
- 목표 높이 약 170cm, 몸통 폭 약 70cm. +X 정면/+Z 위, root 바닥 원점, Blender 단위 1m.
- root/body, 목 3뼈, head/jaw/tail, 좌우 날개 2뼈와 다리 3뼈씩. 목은 연속 메시/혼합 가중치.
- VertexColor 1재질, UV0. 흰 깃털/회색 날개 경계/주황 부리·발/검은 눈을 큰 색 구역으로 분리.
- Idle 2s, Walk 0.8s, Run 0.5s, LookAround 2s, HitReact 0.3s, WakeTelegraph 0.8s,
  Death 1.2s, Sleep 2s 루프, AlertWindup 0.55s, Honk 0.6s 표현 클립.
  기본 30fps. AlertWindup만 60fps/33프레임 간격으로 정확히 0.55초를 보존하고 UE 길이를 확인한다.
- Catch 없음: 현재 거위 CatchRadius=0. 울음은 판정 Notify를 발생시키지 않는다.
- 원본 `ArtSource/monster/goose_a/goose_a.blend`, 개별 SK/AN FBX 및 manifest/중립·야간 미리보기.
- 검수: 목 과신장·꺾임·부리 개폐·날개 접힘·발 접지·루프, UE 크기/축/클립과 실제 상태 연결.

## 실제 제작 결과

[005](../iterations/005.md): 166.21×79.80×167.30cm, 9,516 tris, 4,840 vertices,
20변형뼈(눈 감기 2뼈 추가), 1재질. 위 10클립/FBX/편집 원본 저장.
걷기126cm/s, 달리기528cm/s. 전 프레임 바닥 관통 없음, root 고정, 5루프 경계 일치.
UE 메시/10클립/재질 임포트와 길이 확인. AlertWindup 0.55초 보존.
UE 발 위치와 Blender 차이 <0.000055cm. 실제 AnimBP·2인 PIE/LOD와 이동 영상·손전등은 미완료.

## v2 귀여움 1단계 (2026-09-29, 승인 대기)

[design-concept.md](../design-concept.md)의 "과장되되 귀엽게"에 맞춘 형태와 렌더 실험이다. 실물 `goose_a.blend`·`export/`·UE 에셋은 그대로 두고,
별도 원본 `ArtSource/monster/goose_a_v2/goose_a_v2.blend`(씬 `CL9_GOOSE_A_V2`)에 담았다. 스크립트는 `Tools/Blender/cute_goose_a_v2.py`
(`py -3 Tools/Blender/run_headless.py cute_goose_a_v2.py [-- --final A]`)이다. 원본 메시의 정점을 옮기기만 해서 위상은 그대로다(4,840 정점, 9,516 tris).
리그는 아직 새 비율에 맞추지 않았고 렌더는 모두 레스트 포즈다.

- **레버 비교** `preview/compare_lever.png`(원본 / A / B). A는 보이는 목 길이를 17.5% 줄이고(48.3→39.9cm) 머리와 부리를 한 덩어리로 ×1.35 키웠다(전고 168.5→167.3cm).
  B는 목 길이를 유지한 채 머리를 ×1.35 키우고 목 끝 반지름을 ×1.25(8.6→10.8cm) 늘렸다(전고 175.3cm). **후보는 A**다. 목이 머리 크기에 맞게
  짧아져서 머리·목·몸이 한 덩어리의 둥근 S로 읽히고, 거위다운 긴 목의 인상도 남는다. B는 긴 막대 위에 큰 머리가 얹힌 모양이라 머리가 더 위태로워 보이고, 키도 7cm 커진다.
- **부리**: 부리 뿌리를 기준으로 길이 ×0.88, 두께 ×1.35. 두 단 구조와 콧구멍 위치 관계는 유지된다.
- **눈썹 다이얼** `preview/compare_brow.png`(100 / 40 / 0%). 40%는 쐐기 각도를 +0.13rad에서 +0.01rad로 줄여 앞쪽이 거의 수평이 되게 하고, 눈썹을 1.2cm 올리며, 두께를 원본의 0.76배로 줄였다. 0%는 눈썹을 없앤다.
- **채도**(선형 RGB): 주황 (.70,.24,.022)→(.95,.23,.005), 부리 경계 (.24,.072,.010)→(.34,.055,.003),
  흰색 (.72,.70,.62)→(.78,.74,.60), 크림 (.88,.84,.69)→(.92,.85,.64). 회색과 검정은 그대로 두었다.
- 최종 후보 `preview/neutral_v2.png`(A, 눈썹 40%), 원본과 나란히 놓은 비교는 `preview/compare_final.png`이다. 숫자 확정은 사용자 승인 뒤에 한다.
- 원본 렌더에서 머리 뒤 위쪽의 작은 돌기는 깃 술이 아니라 **먼 쪽 눈썹(Brow_l)이 머리 옆 실루엣 밖으로 삐져나온 것**이다(섬 감사: 머리 반폭 12.6cm, 눈썹 바깥 끝 14.4cm). 따라서 눈썹을 0%로 하면 이 돌기도 함께 사라진다.

### 상태별 눈 3단 — 아픈 눈 (2026-09-30, 승인 대기)

게임 상태에 따라 눈 표정을 셋으로 나눈다. 비교용 임시값은 레버 A 기준으로 **평소(Idle/순찰) = 눈썹 0%**, **추격(Chase) = 눈썹 100%(실물 쐐기)**,
**피격(HitReact) = 아픈 눈**이다. 평소 눈썹은 0%와 40% 중 0%를 골랐다. 40%도 눈 위를 반쯤 덮어 약간 뚱한 인상이 남고, 0%는 눈 전체가 드러나 가장 순하게 보인다.
스크립트는 `Tools/Blender/pain_eyes_goose_a_v2.py`(`py -3 Tools/Blender/run_headless.py pain_eyes_goose_a_v2.py --blend ArtSource/monster/goose_a_v2/goose_a_v2.blend -- --pick P3`)이다.
`goose_a_v2.blend`의 기존 오브젝트(레버 A/B, 눈썹 단계, `SK_Goose_A`)는 그대로 두었고, 후보 `VAR_Pain_P1`·`VAR_Pain_P2`와 고른 `VAR_BrowPain`(P3)을 숨긴 채 덧붙였다.
씬 사용자 속성 `cl9_brow_presets`에 세 상태의 파라미터가 JSON으로 들어 있다. 이번에도 위상은 그대로이고 렌더는 레스트 포즈다.

- **아픈 눈의 원리**: 화난 눈(앞쪽, 즉 부리 쪽 안쪽 끝이 내려간 쐐기, Y축 +0.13rad)과 반대로 안쪽 끝을 올린다. 눈썹을 −0.70rad까지 돌린 뒤 머리 표면에 붙인다.
  붙이는 방식은 이렇다. 눈썹 중심의 표면 법선 하나를 기준으로, 뒤·가운데·앞 세 구역의 "표면 위 최저 높이"를 지나는 2차 곡선만큼 끌어내리고(모양을 유지한 채 매끈하게 휜다),
  법선 방향 두께는 0.6배로 줄인다. 원본 쐐기는 표면에서 6cm나 솟아 있었는데, 이것이 먼 쪽 눈썹이 머리 실루엣 밖으로 삐져나온 원인이었다.
- **후보** `preview/compare_pain_candidates.png`(위 = 기본 카메라 방향 근접, 아래 = 정옆 근접. 맨 왼쪽은 추격 100% 참고):
  P1은 −0.45rad(가벼운 걱정), P2는 −0.70rad(강한 걱정), P3는 P2에 눈 세로 0.72배(아래 끝 고정, 찡그림)를 더했다.
  **P3를 골랐다.** P1은 앞뒤 높이 차가 +0.8cm에 그쳐 거의 수평으로 읽히고, P2는 기울기는 읽히지만 눈이 여전히 동그랗게 커서 "놀람"에 가깝다.
  P3는 올라간 안쪽 끝과 가늘어진 눈이 함께 "아픔"으로 읽히고, 평소의 크고 둥근 눈과도 가장 뚜렷하게 구별된다.
- **실루엣 점검**(기본 카메라 투영에서 먼 쪽 눈썹이 머리+목 볼록 껍질 밖으로 나간 최대 거리): 추격 100% 2.04cm, 기존 40% 3.56cm, P1 1.01cm, P2 1.14cm, **P3 0.69cm**, 평소 0% 0cm.
  아픈 눈은 추격보다 덜 삐져나온다. 표면 위 최대 높이는 추격 6.0cm에서 P3 2.5cm로 줄었다. 0cm까지 낮추면 눈썹이 눈에 겹쳐 안쪽 끝이 올라간 모양이 읽히지 않는다.
- **3단 비교** `preview/compare_three_states.png`(위 = `neutral_v2.png`와 같은 카메라·조명의 전신, 아래 = 같은 방향 머리 근접).
  근접에서는 세 상태가 뚜렷이 구별되지만, 전신 거리에서 아픈 눈은 흰 눈썹이 흰 머리 위에 얇게 붙어 있어 약하게 읽힌다. 전신에서 피격을 전달하는 것은 HitReact 클립의 몸짓이 주로 맡게 된다.
- 눈 세로 0.72배는 메시를 직접 줄여 만든 형태다. 2단계(리그·클립)에서는 기존 눈 감기 2뼈로 같은 모양을 내야 한다.

2026-09-15 반응 클립 재작업(animate_creature_reacts.py, [README](README.md) 표): HitReact 0.3초 — 목이 S 자로 움츠러들며(neck −0.45/+0.25/−0.35rad) 부리 0.45·날개 0.8, 머리 두 번 흔들림 뒤 복귀; **PushReact 0.8초 신설** — 젖혀져 붕 뜸(몸 −16cm·+10cm, 피치 −0.5rad, 목 젖힘) → 착지 → 발 허우적(±10cm) → 복귀. Catch 는 잡지 않는 종이라 만들지 않는다(Honk 그대로). 바닥 최저점 +1.2cm.


## 2026-10-20 혼합 동물 모델·언리얼 배치

- 현재 요청 범위: production (정지 3D 외형 제작·UE 맵 배치·저장 후 재열기 검수). 리그와 행동 클립은 이번 배치 완료 조건에 추가하지 않는다.
- 사용자 승인 근거: 최신 혼합 동물 그림에 “이정도로해서 배치해봐”, 이어 “실제 3D 모델을 만들어 언리얼 맵에 배치”. 컨셉 강도 승인과 제작·배치 진행 지시이며 아직 보지 않은 3D 결과의 승인은 아니다.
- 컨셉: ArtSource/monster/concept_batch_1/hybrid_placement_v01/approved_concept.png. 작업 버전: model_hybrid_v01. 기존 후보 보존.
- 현재 단계: blender_work. 단독 입력 이미지를 분리하고 로컬 TRELLIS 생성 후 정지 메시를 검수한다.
