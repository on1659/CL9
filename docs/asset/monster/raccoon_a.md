# Raccoon A — 너구리 전용 모델

## v02 완료 — 재질·표면·모션·실제 게임 검수

- 현재 13개 모션과 원본·FBX·정상1배속 영상: [현재 manifest](../../../ArtSource/monster/raccoon_a/animation_hybrid_v02/manifest.json).
- [5종52클립 납품·검증](../../../ArtSource/monster/concept_batch_1/hybrid_finish_v02/delivery.json) · [검토 화면](../../../ArtSource/monster/concept_batch_1/hybrid_finish_v02/index.html).
- 모든 필수 증거의 실제 파일·해시·통과 값을 확인했다. 사용자 외형 승인을 별도로 주장하지 않는다. 아래 v01 기록과 한계는 이전 이력이다.


## 이전 v01 체크포인트 — 혼합 동물 리그·모션과 엔진 프리뷰

- 종별 리그와 13개 동작을 제작했다: Idle, Walk, Run, LookAround, WakeTelegraph, Catch, HitReact, PushReact, Death, Execute, StalkWalk, Freeze, Retreat.
- [5종 모션 검토 화면](../../../ArtSource/monster/concept_batch_1/hybrid_animation_v01/index.html) · [편집 원본](../../../ArtSource/monster/raccoon_a/animation_hybrid_v01/raccoon_a_Hybrid_Animated.blend) · [동작별 출력·1배속 영상](../../../ArtSource/monster/raccoon_a/animation_hybrid_v01/manifest.json).
- UE 임포트·포즈 왕복, 복사 검토 맵 저장/재열기, 실제 SIE의 네이티브 애니메이션 재생을 확인했다. AI/AnimBP 상태 연결 완료를 뜻하지 않는다.
- 남은 사항: 게임 이동 속도와 재생 속도 보정, 실제 상태 연결, 원본 표면 마감 및 플레이어 접촉 검수. 별도의 사용자 외형 승인으로 기록하지 않는다.

## 이전 체크포인트 — 혼합 동물 정지 모델·UE 배치

- 이번 요청 범위 완료: 실제 정지 3D 외형·FBX·언리얼 복사 맵 배치·저장 후 재열기·실제 엔진 캡처. 사용자 3D 외형 승인은 별도이며 리그/애니메이션/AI 연결은 하지 않았다.
- 혼합 컨셉과 강도 승인 근거: “이정도로해서 배치해봐” → “실제 3D 모델을 만들어 언리얼 맵에 배치”.
- [현재 원본](../../../ArtSource/monster/raccoon_a/model_hybrid_v01/placement/raccoon_a_Hybrid.blend) · [모델 QA](../../../ArtSource/monster/raccoon_a/model_hybrid_v01/placement/qa.json) · [5종 검수 화면](../../../ArtSource/monster/concept_batch_1/hybrid_placement_v01/index.html).
- UE 맵: `/Game/Tests/HybridFiveV01/Lvl_HybridFiveV01`. 메시: `/Game/Monsters/HybridFiveV01/raccoon_a/SM_raccoon_a_Hybrid.SM_raccoon_a_Hybrid`. 초안 외형 높이 0.95 m, 28,555 triangles. 게임 판정 크기를 변경하지 않았다.
- 실제 원본 GLB와 이전 모델을 보존했다. 아래 기존 기록은 과거 버전 이력이다.
- 남은 한계: 최종 표면·노멀·그림자 마감. 검토용 양면 재질과 모델 그림자 투영 비활성 설정을 사용했다. 쥐 몸통의 거친 면, 양 얼굴 경계와 거위 목의 국부 면 정리가 남는다.

```json
{
  "concept_pipeline": {
    "schema_version": 2,
    "asset_id": "raccoon_a",
    "scope": "production",
    "requested_outputs": [
      "finished surface topology",
      "matching CL9 vertex-color materials",
      "Idle",
      "Walk",
      "Run",
      "LookAround",
      "WakeTelegraph",
      "Catch",
      "HitReact",
      "PushReact",
      "Death",
      "Execute",
      "StalkWalk",
      "Freeze",
      "Retreat",
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
      "candidate_version": "model_hybrid_v01/placement",
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
          "evidence": "ArtSource/monster/raccoon_a/animation_hybrid_v02/manifest.json"
        },
        "animation": {
          "status": "pass",
          "evidence": "ArtSource/monster/raccoon_a/animation_hybrid_v02/manifest.json"
        },
        "modelQA": {
          "status": "pass",
          "evidence": "ArtSource/monster/raccoon_a/animation_hybrid_v02/manifest.json"
        }
      },
      "runtime": {
        "level": "actual_host_client_gameplay",
        "gameplay_connected": true,
        "asset_paths": [
          "/Game/Monsters/HybridFiveFinishV02/raccoon_a/Build_48ee7751c7/SK_raccoon_a_HybridV02.SK_raccoon_a_HybridV02",
          "/Game/Monsters/HybridFiveFinishV02/raccoon_a/Build_48ee7751c7/SK_raccoon_a_HybridV02_Skeleton.SK_raccoon_a_HybridV02_Skeleton",
          "/Game/Monsters/HybridFiveFinishV02/raccoon_a/Build_48ee7751c7/Clips_d04e605ab1/AN_Raccoon_HybridV02_Idle.AN_Raccoon_HybridV02_Idle",
          "/Game/Monsters/HybridFiveFinishV02/raccoon_a/Build_48ee7751c7/Clips_fb29579369/AN_Raccoon_HybridV02_Walk.AN_Raccoon_HybridV02_Walk",
          "/Game/Monsters/HybridFiveFinishV02/raccoon_a/Build_48ee7751c7/Clips_38456283af/AN_Raccoon_HybridV02_Run.AN_Raccoon_HybridV02_Run",
          "/Game/Monsters/HybridFiveFinishV02/raccoon_a/Build_48ee7751c7/Clips_7716ec22b2/AN_Raccoon_HybridV02_LookAround.AN_Raccoon_HybridV02_LookAround",
          "/Game/Monsters/HybridFiveFinishV02/raccoon_a/Build_48ee7751c7/Clips_b58b1617ef/AN_Raccoon_HybridV02_WakeTelegraph.AN_Raccoon_HybridV02_WakeTelegraph",
          "/Game/Monsters/HybridFiveFinishV02/raccoon_a/Build_48ee7751c7/Clips_191db593a6/AN_Raccoon_HybridV02_Catch.AN_Raccoon_HybridV02_Catch",
          "/Game/Monsters/HybridFiveFinishV02/raccoon_a/Build_48ee7751c7/Clips_1ab89c60bb/AN_Raccoon_HybridV02_HitReact.AN_Raccoon_HybridV02_HitReact",
          "/Game/Monsters/HybridFiveFinishV02/raccoon_a/Build_48ee7751c7/Clips_58e9efe0a3/AN_Raccoon_HybridV02_PushReact.AN_Raccoon_HybridV02_PushReact",
          "/Game/Monsters/HybridFiveFinishV02/raccoon_a/Build_48ee7751c7/Clips_337acb07aa/AN_Raccoon_HybridV02_Death.AN_Raccoon_HybridV02_Death",
          "/Game/Monsters/HybridFiveFinishV02/raccoon_a/Build_48ee7751c7/Clips_5d04c5b4e8/AN_Raccoon_HybridV02_Execute.AN_Raccoon_HybridV02_Execute",
          "/Game/Monsters/HybridFiveFinishV02/raccoon_a/Build_48ee7751c7/Clips_46ff8d8efc/AN_Raccoon_HybridV02_StalkWalk.AN_Raccoon_HybridV02_StalkWalk",
          "/Game/Monsters/HybridFiveFinishV02/raccoon_a/Build_48ee7751c7/Clips_e944ea06f5/AN_Raccoon_HybridV02_Freeze.AN_Raccoon_HybridV02_Freeze",
          "/Game/Monsters/HybridFiveFinishV02/raccoon_a/Build_48ee7751c7/Clips_f46f1eb9de/AN_Raccoon_HybridV02_Retreat.AN_Raccoon_HybridV02_Retreat"
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
      "source_blender_version": "model_hybrid_v01/placement",
      "candidate_version": "animation_hybrid_v02",
      "motion_plan": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/plan.json",
      "delivery_status": "complete",
      "species_manifest": "ArtSource/monster/raccoon_a/animation_hybrid_v02/manifest.json",
      "previous_delivery": "ArtSource/monster/concept_batch_1/hybrid_animation_v01/delivery.json",
      "current_manifest": "ArtSource/monster/raccoon_a/animation_hybrid_v02/manifest.json",
      "modelQA": {
        "status": "pass",
        "evidence": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/delivery.json"
      }
    },
    "checkpoint": {
      "path": "ArtSource/monster/raccoon_a/animation_hybrid_v02/raccoon_a_Hybrid_Finished.blend",
      "sha256": "e5a5270aaf312526ecfed8c0d06fa50ec380fd60128461037b0340a510ace216"
    },
    "job": null,
    "last_completed": "V02 기존 혼합 외형 마감·정점색 재질·13개 모션·엔진 및 실제 host/client 검수와1배속 영상 완료. 별도 사용자 외형 승인을 뜻하지 않음.",
    "next_action": "새 사용자 요청에 따라 현재 v02 체크포인트에서 재개",
    "blocker": null,
    "previous_requested_outputs": [
      "species-specific rig",
      "Idle",
      "Walk",
      "Run",
      "LookAround",
      "WakeTelegraph",
      "Catch",
      "HitReact",
      "PushReact",
      "Death",
      "Execute",
      "StalkWalk",
      "Freeze",
      "Retreat",
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
              "evidence": "ArtSource/monster/raccoon_a/model_hybrid_v01/placement/qa.json"
            },
            "engine": {
              "status": "native_preview_pass",
              "evidence": "ArtSource/monster/concept_batch_1/hybrid_animation_v01/native_final_qa.json"
            },
            "model_rig": {
              "status": "pass_with_recorded_surface_limitations",
              "evidence": "ArtSource/monster/raccoon_a/animation_hybrid_v01/full_qa.json"
            },
            "animation": {
              "status": "authored_and_video_reviewed",
              "evidence": "ArtSource/monster/raccoon_a/animation_hybrid_v01/manifest.json"
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
              "/Game/Monsters/HybridFiveMotionV01/raccoon_a/Build_9a8a33907c_Normals/SK_raccoon_a_Hybrid.SK_raccoon_a_Hybrid",
              "/Game/Monsters/HybridFiveMotionV01/raccoon_a/Build_9a8a33907c_Normals/Clips_6c6543bd19/AN_Raccoon_HybridV01_Idle.AN_Raccoon_HybridV01_Idle",
              "/Game/Monsters/HybridFiveMotionV01/raccoon_a/Build_9a8a33907c_Normals/Clips_5d5ff0a3b3/AN_Raccoon_HybridV01_Walk.AN_Raccoon_HybridV01_Walk",
              "/Game/Monsters/HybridFiveMotionV01/raccoon_a/Build_9a8a33907c_Normals/Clips_bc2dd1ee82/AN_Raccoon_HybridV01_Run.AN_Raccoon_HybridV01_Run",
              "/Game/Monsters/HybridFiveMotionV01/raccoon_a/Build_9a8a33907c_Normals/Clips_3fd611e997/AN_Raccoon_HybridV01_LookAround.AN_Raccoon_HybridV01_LookAround",
              "/Game/Monsters/HybridFiveMotionV01/raccoon_a/Build_9a8a33907c_Normals/Clips_1a4c92f04b/AN_Raccoon_HybridV01_WakeTelegraph.AN_Raccoon_HybridV01_WakeTelegraph",
              "/Game/Monsters/HybridFiveMotionV01/raccoon_a/Build_9a8a33907c_Normals/Clips_342e3cb5e5/AN_Raccoon_HybridV01_Catch.AN_Raccoon_HybridV01_Catch",
              "/Game/Monsters/HybridFiveMotionV01/raccoon_a/Build_9a8a33907c_Normals/Clips_323f9137e6/AN_Raccoon_HybridV01_HitReact.AN_Raccoon_HybridV01_HitReact",
              "/Game/Monsters/HybridFiveMotionV01/raccoon_a/Build_9a8a33907c_Normals/Clips_8bccf84767/AN_Raccoon_HybridV01_PushReact.AN_Raccoon_HybridV01_PushReact",
              "/Game/Monsters/HybridFiveMotionV01/raccoon_a/Build_9a8a33907c_Normals/Clips_2a1f8ab908/AN_Raccoon_HybridV01_Death.AN_Raccoon_HybridV01_Death",
              "/Game/Monsters/HybridFiveMotionV01/raccoon_a/Build_9a8a33907c_Normals/Clips_fb650299a4/AN_Raccoon_HybridV01_Execute.AN_Raccoon_HybridV01_Execute",
              "/Game/Monsters/HybridFiveMotionV01/raccoon_a/Build_9a8a33907c_Normals/Clips_2e04c0e053/AN_Raccoon_HybridV01_StalkWalk.AN_Raccoon_HybridV01_StalkWalk",
              "/Game/Monsters/HybridFiveMotionV01/raccoon_a/Build_9a8a33907c_Normals/Clips_28e9da41c8/AN_Raccoon_HybridV01_Freeze.AN_Raccoon_HybridV01_Freeze",
              "/Game/Monsters/HybridFiveMotionV01/raccoon_a/Build_9a8a33907c_Normals/Clips_a31a496db7/AN_Raccoon_HybridV01_Retreat.AN_Raccoon_HybridV01_Retreat"
            ],
            "gameplay_connected": false
          },
          "source_blender_version": "model_hybrid_v01/placement",
          "candidate_version": "animation_hybrid_v02",
          "motion_plan": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/plan.json",
          "delivery_status": "full_finish_working",
          "species_manifest": "ArtSource/monster/raccoon_a/animation_hybrid_v01/manifest.json",
          "previous_delivery": "ArtSource/monster/concept_batch_1/hybrid_animation_v01/delivery.json"
        },
        "checkpoint": {
          "path": "ArtSource/monster/raccoon_a/animation_hybrid_v01/raccoon_a_Hybrid_Animated.blend",
          "sha256": "eee4ea4df82acf0da436081875caacdbb639d6b175ab69b78bb0c99532bc9514"
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
  asset_id: raccoon_a
  scope: production
  phase: blender_review
  status: awaiting_user
  updated_at: 2026-10-20T04:56:15+00:00
  blender:
    candidate_version: model_reference_v02
    approved_version: null
  pending_review: {stage: blender, version: model_reference_v02, evidence: ArtSource/monster/raccoon_a/model_reference_v02/corrected_candidate_v12/preview}
  checkpoint: {path: ArtSource/monster/raccoon_a/model_reference_v02/corrected_candidate_v12/raccoon_a.blend, sha256: 47dbc0927f49368ed7286092e05247750cb531c2d2d2bf0ec485a1d1080b344a}
  last_completed: "원래 시안 기반 새 3D 생성·Blender 외형 보정·저장/재열기·실제 4뷰 직접 확인"
  next_action: "v02 실제 외형에 대한 사용자 검토 후 수정 또는 어려운 변형 시험·리그·애니메이션·UE 검수"
```

</details>

## 2026-10-20 v02 실제 Blender 외형 후보

- 새 실제 원본: [raccoon_a.blend](../../../ArtSource/monster/raccoon_a/model_reference_v02/corrected_candidate_v12/raccoon_a.blend), SHA256 `47dbc0927f49368ed7286092e05247750cb531c2d2d2bf0ec485a1d1080b344a`.
- [실제 네 방향 렌더](../../../ArtSource/monster/raccoon_a/model_reference_v02/corrected_candidate_v12/preview/three_quarter.png) · [제작/재열기 기록](../../../ArtSource/monster/raccoon_a/model_reference_v02/corrected_candidate_v12/manifest.json) · [5종 비교 화면](../../../ArtSource/monster/concept_batch_1/model_review_v02/index.html).
- 방식: 기존 6종 4상태 시안의 해당 정상형을 단독 2D 입력으로 정리한 뒤 로컬 TRELLIS.2에서 3D 생성, Blender에서 검사·필요한 오류 보정. `input.png`는 입력 그림이고 `preview/*.png`는 실제 저장 모델의 렌더다. 원래 v01과 생성 원본 GLB를 보존한다.
- 생성 조건: 512 / seed 42 / 12 steps / 목표 50,000 faces / 2,048 texture. 축 +X 정면/+Z 위, 단위 m. 후보 크기는 외형 비교용이며 게임 판정을 변경하지 않았다.
- 실측 메시 기록: `{"mesh_objects": 2, "vertices": 10048, "triangles": 20078, "material_slots": 2, "uv_layers": {"Preserved_Closed_Head_Single_Plume_Feet": ["UVMap"], "Single_Round_Rebuilt_Torso": ["UVMap"]}, "bounds_min_m": [-0.8090711236000061, -0.7991181015968323, 0.006672382354736328], "bounds_max_m": [0.7897235155105591, 0.4441232979297638, 0.993405818939209], "bounds_size_m": [1.5987946391105652, 1.243241399526596, 0.9867334365844727]}`.
- 정면·측면·후면·사선에서 눈마스크, 네 발과 풍성한 줄무늬 꼬리 하나를 확인했다.
- 중복 꼬리와 잘못 생성된 몸통을 제거했다. 원본의 닫힌 머리·발·정상 꼬리 성분을 보존하고 몸통을 단일 둥근 볼륨으로 교체했다. 원본 UV·2048 텍스처로 눈과 무늬의 선명도를 복원했다.
- 뒷다리 위 접힘과 몸통의 교차선, 꼬리 표면의 굴곡, 눈의 U자 흰 테두리가 남는다. 열린 경계는 0이지만 머리에 비매니폴드 에지 1개와 겹친 닫힌 부위가 있어 게임용 리토폴로지가 필요하다.
- 검증 범위: 실제 파일 저장·재열기, 텍스처 포함 여부, 정면·측면·후면·사선 렌더 직접 확인. 이는 사용자 승인이나 변형·게임 성능 합격이 아니다.
- 미검증: 변형용 토폴로지/리그, 행동·이족 광폭화·전환, FBX/UE, LOD와 게임 성능. 기존 소·닭 탈·돼지는 수정하지 않았고, 어미닭·횃대닭은 참고만 유지한다.



## 이력 — 2026-10-20 반려된 정상형 3D 후보 model_round_v01

당시 전체 요청 `production`의 `blender_review`까지 제작했으나 사용자 “저거 다 이상해 다시만들어”로 **반려됐다**. 아래는 비교용 이력이며 현재 승인 대기본이나 v02 완료 체크포인트가 아니다. 종전 게임 연결 이력도 새 후보의 QA로 승계하지 않는다. [완료된 10분 사전 조사와 재사용 판단](../../../ArtSource/monster/research_20261020/README.md) 뒤 별도 버전에서 직접 모델링했다.

- 기준: 최신 [6종 4상태 시안](../../../ArtSource/monster/concept_batch_1/sheet_six_species_four_states_v5.png)을 직접 확인했다. 통통한 공 몸통, 큰 검정 눈마스크와 크림 얼굴, 작고 둥근 삼각귀, 짧은 검정 네 발, 몸만큼 크게 굽은 굵은 줄무늬 꼬리, 순한 큰 눈을 반영했다.
- 외형: 종전 길고 가느다란 몸을 재사용하지 않은 신규 메시다. 몸·머리를 삼각면 큰 덩이로 만들고, 눈가 무늬는 **머리 메시 자체의 재질 영역**으로 통합했다. 초기 얇은 패치의 면 겹침을 제거했으며 떠 보이던 귀뿌리와 내이를 수정했다. 큰 색면 3개(회색·크림·검정), 털 가닥 없음, 얇은 눈 테두리다.
- 실제 편집 파일: [raccoon_a_round_v01.blend](../../../ArtSource/monster/raccoon_a/model_round_v01/raccoon_a_round_v01.blend). SHA256 `4b49ca4a6b5a362b43d74ccbc58bff9d4eba49011f84f805b288e5a890d41191`. [재현 Python](../../../ArtSource/monster/raccoon_a/model_round_v01/build_round.py), [manifest](../../../ArtSource/monster/raccoon_a/model_round_v01/manifest.json), [QA](../../../ArtSource/monster/raccoon_a/model_round_v01/qa.json).
- 실측: **7,798 tris / 3,948 vertices**, 꼬리 포함 바운드 X/Y/Z **254.34×106.00×203.54cm**, min Z=0. +X 정면·+Z 위·1BU=1m. 모델 오브젝트의 위치/회전/스케일 적용을 확인했다. 큰 꼬리의 시각 바운드와 게임 판정 크기는 별개이며 판정 크기를 변경하지 않았다.
- 실제 검사: Blender 5.2.2 별도 background/factory-startup/disable-autoexec, **CPU Cycles 2스레드·20 samples·640×640**, 회색 단색 배경·바닥 없음·중립광. 저장한 `.blend`를 다시 열어 [정면](../../../ArtSource/monster/raccoon_a/model_round_v01/preview/front.png)·[측면](../../../ArtSource/monster/raccoon_a/model_round_v01/preview/side.png)·[후면](../../../ArtSource/monster/raccoon_a/model_round_v01/preview/back.png)·[사선](../../../ArtSource/monster/raccoon_a/model_round_v01/preview/three_quarter.png)을 모두 렌더하고 직접 보았다. 프레임 잘림·꼬리 실루엣·얼굴의 면 겹침 제거·귀뿌리 연결·눈 노출을 확인했다.
- 보존: 종전 `raccoon_a.blend`, `export/`, 기존 모션과 UE 에셋은 수정하지 않았다. 새 후보는 리그/Action 0개인 외형 단계다. 사용자 외형 승인, 직립 변형·웨이트·모션, UV/아틀라스·내보내기, 자기교차/매니폴드 전체 검사, UE 왕복·게임 연결은 **미검증**이다.
- 당시 후속 계획(미실행·v01 반려로 중단): 외형 검토 뒤 사족→직립 전환 때 짧은 앞발이 자연스럽게 내려오는 경로, 공 몸통 유지, 큰 꼬리의 무게중심·바닥 접촉을 가장 어려운 2～3초에서 먼저 시험하는 것이었다. 현재 재개 대상은 상단의 v02 외형 재제작이다.

상태: 게임 연결 검수됨 — 2026-09-11 `MonsterTable.raccoon` → SK/`ABP_Raccoon_A`, Lvl_Flat PIE; 09-15 PushReact; 09-20 저녁 스토커·등 뒤 처형 PIE(C-처형-너구리); **09-21 Execute 2.0초 제작(Run 2 — FBX·manifest 완료, UE 임포트·ABP 교체·PIE 는 리드 대기)**. 클립 9개 + Execute(`Content/Monsters/RaccoonA/`), 재질 슬롯 배정 09-20. 2인 PIE·LOD 미검수 · 작성일: 2026-09-11 · 변경일: 2026-09-21

2026-09-11 · 제작 중. 현재 임시 Fox를 대체할 CL9 신규 모델이다.

Alias=raccoon. 검은 눈가 무늬, 둥근 귀, 넓은 뺨과 짧은 주둥이, 굵은 줄무늬 꼬리로
기존 여우와 구별한다. 네 발 이동이며 잡기에서 머리와 앞발을 내민다. 도둑질 등 미정 AI를 추가하지 않는다.
목표 몸통 폭 약 65cm/귀 포함 높이 약 100cm. 꼬리는 판정 Radius=40cm와 별개인 시각 부위다.
축 +X 앞/+Z 위, root는 발 사이 바닥. 전체 길이/바운드는 제작 후 manifest에 기록한다.

- 독립 종 리그: root/body/chest/neck/head/jaw, 양쪽 귀, 네 다리 각 3뼈, 꼬리 4뼈.
- 몸통과 꼬리의 연속 부위는 인접 뼈 가중치를 섞고, 발가락/얼굴 요소는 해당 뼈를 따른다.
- Vertex Color 1개 재질, UV0 포함. 털은 큰 덩어리와 얼굴·꼬리 색 구역으로 표현한다.
- Idle 2s, Walk 0.8s(140cm/s), Run 0.5s(462cm/s), LookAround 2s,
  HitReact 0.3s, WakeTelegraph 0.8s, Catch 0.8s(2026-09-15 재작업 — 아래 절, 처음 0.6s), PushReact 0.8s(2026-09-15 신설), Death 1.2s.
- 30fps, in-place, root 고정, 판정 Notify 없음. SleepTime=0이므로 Sleep은 현행 실제 미사용.
- 원본 `ArtSource/monster/raccoon_a/raccoon_a.blend`; 메시와 각 클립 FBX, manifest와 미리보기 납품.
- 필수 검수: 가중치/축/바닥·발 접지/루프/꼬리 변형, 중립·야간 식별, UE 리그와 실제 상태 연결/2인 표현.

모델/애니메이션 제작과 최종 UE 합격은 별개로 기록한다.

## 제작 및 Blender 검수 결과 — 2026-09-11

- 귀 내부 중복 형상을 정리하고 외부 면 색으로 대체. 14,776 tris / 7,534 vertices / 24뼈 / 1재질.
- 위 8개 Action, 메시 FBX 1개와 애니메이션 FBX 8개, 편집 원본 저장.
- `export/blender_animation_qa.json`: 전 프레임 평가 메시 min Z 약 0.70cm, root 변위 0,
  4개 루프 첫/끝 뼈 행렬 오차 0. 지지 발 이동 속도 오차 최대 0.0004cm/s 미만.
- `preview/neutral.png`, `night.png`, `walk.png`, `run.png`, `catch.png`, `death.png`,
  `waketelegraph.png`, `lookaround.png` 직접 확인. 눈가/꼬리 무늬와 자세 식별 가능.
- 남은 검수: 이동 영상의 속도 연속성, 손전등, UE 임포트/스켈레톤/재질/LOD/실제 행동/2인 표현.

UE 후속 [004](../iterations/004.md): 메시/8클립 임포트, 크기·재질·스켈레톤·길이 및
전 프레임 발 위치 비교 완료. 최대 위치 차이 <0.007582cm. 실제 행동/2인 PIE/LOD는 미완료.

2026-09-15 반응 클립 재작업(animate_creature_reacts.py, [README](README.md) 표): Catch 0.6→**0.8초** 앞발 휘두르기 — 앞이 들리며(몸 +10cm·+8cm, 피치 −0.40rad) 오른 앞발(0.05~0.30초) 다음 왼 앞발(0.28~0.55초)을 접었다 앞으로 뻗어 내리친다(upper −0.9→+0.4rad, lower +0.8→−0.2rad), 입 0.45; HitReact 0.3초 — 정점(몸 −7cm·머리 −0.30rad 옆으로 0.30) 뒤 복귀; **PushReact 0.8초 신설** — 돼지와 같은 사족 규칙. 바닥 최저점 +0.7cm.

2026-09-21 **Execute 2.0초 신설**(`AN_Raccoon_Execute`, 실행 execute-clips — 너구리의 주 공격 = 등 뒤 처형, [README](README.md) "공격"; 격분 뒤 보통 잡기
`AN_Raccoon_Catch` 는 그대로): 피해자 등 뒤 −120(`ExecuteAnchorDistance`, 텔레포트 고정)·요 0 을 가정한 액터 공간 앵커 표 `Tools/Blender/execute_anchor.py`
를 농부 짝 `AN_Farmer_Executed_Raccoon` 과 공유한다. 0~0.3초 웅크림(몸 −5/−12cm, 피치 +0.08, 코 위 −0.25rad, 귀 세움) → 0.3~0.6 도약(포물선
body **+42/+38cm**, 피치 **−0.45rad** — brief 초안 +50/+45·−0.6 은 코끝이 피해자 머리 위 z 180 으로 나가 줄였다; 앞발 IK 목표 = 피해자 어깨뼈 (−15, ±20, 135),
입 0.45) → **접촉 0.60** → 0.75 유지(입 다물기) → 1.0 착지(피해자가 앞으로 넘어지는 동안 제자리로 내려온다, 착지 눌림 −5cm) → 1.25~1.8 시체 쪽으로 코를 내려
킁킁(목 +0.22·머리 +0.35, 5Hz 잔떨림) → **2.0 = Idle 첫 프레임**(시체 발치에 선 자세 — `SpectateDelay` 까지 유지). 접촉 프레임 검산(manifest
`contact_probe_cm`, 피해자 공간): 코끝 (−10, 0, 152) = 목덜미, 앞발 (−19, ±20, 132) = 어깨뼈(캡슐 표면 −39 안). 뒷발은 도약 중 발 고정 IK 가 닿는
데까지 뻗어 뒤로 끌린다. 바닥 최저점 +0.7cm. FBX 61프레임·30fps, sha256 `32887264…`, 미리보기 `preview/react_execute_f{01,10,19,25,28,34,43,52,61}.png`·
`react_execute_sheet.png`(Execute 는 도약이 프레임을 벗어나지 않게 카메라 배율 1.25). 기존 FBX 는 바이트 그대로(`--clips Execute`). UE:
`/Game/Monsters/RaccoonA/AN_Raccoon_Execute`, `ABP_Raccoon_A` Execute 가지 평가기 클립을 `AN_Raccoon_Catch` → 이것으로(docs/systems/animation.md),
표 `ExecuteDuration` 1.2 → 2.0 은 balance.

## v2 후보 — "귀엽게" 1단계(형태+렌더), 2026-09-29, 사용자 승인 대기

실행 monster-cute-raccoon, [design-concept.md](../design-concept.md) 톤 축·규칙 2. 게임에 연결된 실물(`raccoon_a.blend`·`export/`·UE)은 그대로 두고,
`Tools/Blender/build_raccoon_a_v2.py` 가 원본을 열어 메모리에서만 고친 결과를 `ArtSource/monster/raccoon_a_v2/raccoon_a_v2.blend` 에 쓴다.
- 머리 부위(head·jaw·ear 가중 섬) ×1.15(head 뼈 머리점 기준). 머리 코어 지름 ÷ 몸통 지름: 폭 76.6% → 88.0%, 높이 75.0% → 86.3%(실측, 머리 49×45 → 56.3×51.8cm, 몸통 64×60cm 그대로). 몸통 길이·다리·꼬리는 그대로.
- 주둥이 X ×0.72(뒤끝 기준)·단면 ×1.08 — 끝이 뭉툭해진다. 코는 모양 그대로 따라 들어간다. 전체 길이 211 → 207cm.
- 귀 껍질 Z ×0.86(바닥 기준)·Y ×1.08. 발톱 16개는 끝 고리를 뿌리 반경의 0.8배로 벌리고 뿌리 기준 ×0.7.
- 마스크·꼬리 고리 형태는 그대로. 팔레트만 대비 상향(fur 어둡게, light 더 희게, dark 더 검게, 홍채 호박색, 귀 안쪽 분홍 갈색, 발톱 상아색).
- 눈썹 3단계: 100%(현재) / 50%(+1.8cm 올림, 안쪽 끝 +0.15rad, 두께 ×0.6, 머리 쪽으로 1cm 묻기) / 0%(없음). 오브젝트 기본은 50%이고, 나머지 둘은 메시 데이터 `SK_Raccoon_A_v2_Brow100`·`SK_Raccoon_A_v2_Brow0` 로 보관한다.
- 삼각형 14,776개(0%는 14,328개), 뼈 위치와 액션은 원본 그대로다. 머리를 키웠기 때문에 귀 뼈가 귀 메시보다 낮게 남는데, 이 문제는 2단계(리그·클립)에서 다룬다.
- 렌더: `raccoon_a_v2/preview/neutral.png`, `compare_original_v2.png`(왼쪽 원본, 오른쪽 v2), `compare_brow_100_50_0.png`(왼쪽부터 100%, 50%, 0%).

### v2 눈 표정 3단 — "아픈 눈" 후보, 2026-09-30, 사용자 승인 대기

실행 eyes-pain-raccoon. 게임 상태별 눈: 평소(Idle·순찰) = 눈썹 0%, 추격(Chase) = 눈썹 100%(실물값), 피격(HitReact) = 아픈 눈. 앞의 둘은 위 v2 단계를 그대로 쓰는 비교용 임시값이다.
`Tools/Blender/build_raccoon_a_v2_pain.py` 가 `raccoon_a_v2.blend` 의 눈썹 100% 데이터를 복사해 만들고, 고른 안을 메시 데이터 `SK_Raccoon_A_v2_Pain`(fake user)으로 더했다. Brow100/50/0 과 오브젝트 기본(50%)은 그대로다.
- 실측: 눈썹 섬은 길이 24.6cm, 수평 축이 앞쪽 기준 ±28.6°, 원래 기울기는 거의 수평(−0.027rad)이다. 화난 인상은 기울기보다 눈썹이 마스크 위 가장자리에 낮게 걸친 데서 나온다. 마스크 위 끝은 z 86.2cm 이다.
- 아픈 눈(후보 C 채택):
  - 눈썹: 원래 기울기를 지우고 안쪽 끝이 올라가게 +0.6rad 기울였다. 회전 중심을 바깥 끝에 두어 바깥 끝이 마스크 안으로 내려가지 않는다. 전체를 2cm 올리고 두께는 ×0.6 으로 줄였다. 정점은 머리 중심과의 거리를 원래대로 되돌려 표면에 붙였다. 색은 마스크와 같은 검은색(.012/.013/.015)이라 흰 이마 위에서 선으로 읽힌다. 눈썹 z 는 85.9~95.4cm 이고, 렌더에서 마스크와의 사이에 흰 틈이 남는다.
  - 눈(홍채·동공): 눈 중심보다 홍채 반높이의 25% 아래를 중심으로 세로 ×0.55 눌렀다. 동공은 ×0.8 로 줄였고, 눈을 눈썹 축 방향으로 0.3rad 기울여 안쪽 끝이 올라가게 했다.
- 기각한 후보: A는 흰 눈썹(색 그대로)에 찡그린 눈이다. 흰 이마 위의 흰 눈썹은 음영으로만 보여서 사선이 약하게 읽힌다. B는 C에서 눈 기울기를 뺀 것으로, 눈썹은 걱정하는 모양인데 눈이 수평으로 눌려 "졸린 눈"에 가깝다.
- 알려진 문제: neutral 방향(옆 3/4)에서는 먼 쪽 눈썹의 올라간 안쪽 끝이 머리 윤곽 위로 작은 검은 돌기처럼 보인다. 거위 원본의 먼 쪽 눈썹 돌기와 같은 종류다.
- 렌더: `raccoon_a_v2/preview/compare_pain_candidates.png`(얼굴 쪽 3/4 근접, 왼쪽부터 A, B, C), `compare_three_states.png`(neutral.png 와 같은 카메라 방향·조명, 왼쪽부터 평소 | 추격 | 피격). 자세는 모두 Idle 1프레임이다.


## 2026-10-20 혼합 동물 모델·언리얼 배치

- 현재 요청 범위: production (정지 3D 외형 제작·UE 맵 배치·저장 후 재열기 검수). 리그와 행동 클립은 이번 배치 완료 조건에 추가하지 않는다.
- 사용자 승인 근거: 최신 혼합 동물 그림에 “이정도로해서 배치해봐”, 이어 “실제 3D 모델을 만들어 언리얼 맵에 배치”. 컨셉 강도 승인과 제작·배치 진행 지시이며 아직 보지 않은 3D 결과의 승인은 아니다.
- 컨셉: ArtSource/monster/concept_batch_1/hybrid_placement_v01/approved_concept.png. 작업 버전: model_hybrid_v01. 기존 후보 보존.
- 현재 단계: blender_work. 단독 입력 이미지를 분리하고 로컬 TRELLIS 생성 후 정지 메시를 검수한다.
