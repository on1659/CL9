# rabbit_a 제작 카드 — 토끼 (네발 규격의 첫 종)

## v02 완료 — 재질·표면·모션·실제 게임 검수

- 현재 11개 모션과 원본·FBX·정상1배속 영상: [현재 manifest](../../../ArtSource/monster/rabbit_a/animation_hybrid_v02/manifest.json).
- [5종52클립 납품·검증](../../../ArtSource/monster/concept_batch_1/hybrid_finish_v02/delivery.json) · [검토 화면](../../../ArtSource/monster/concept_batch_1/hybrid_finish_v02/index.html).
- 모든 필수 증거의 실제 파일·해시·통과 값을 확인했다. 사용자 외형 승인을 별도로 주장하지 않는다. 아래 v01 기록과 한계는 이전 이력이다.


## 이전 v01 체크포인트 — 혼합 동물 리그·모션과 엔진 프리뷰

- 종별 리그와 11개 동작을 제작했다: Idle, Hop, Thump, Charge, Headbutt, Flee, HitReact, PushReact, Death, EarsUpStartle, HopPause.
- [5종 모션 검토 화면](../../../ArtSource/monster/concept_batch_1/hybrid_animation_v01/index.html) · [편집 원본](../../../ArtSource/monster/rabbit_a/animation_hybrid_v01/RabbitHybrid_Animated.blend) · [동작별 출력·1배속 영상](../../../ArtSource/monster/rabbit_a/animation_hybrid_v01/manifest.json).
- UE 임포트·포즈 왕복, 복사 검토 맵 저장/재열기, 실제 SIE의 네이티브 애니메이션 재생을 확인했다. AI/AnimBP 상태 연결 완료를 뜻하지 않는다.
- 남은 사항: 게임 이동 속도와 재생 속도 보정, 실제 상태 연결, 원본 표면 마감 및 플레이어 접촉 검수. 별도의 사용자 외형 승인으로 기록하지 않는다.

## 이전 체크포인트 — 혼합 동물 정지 모델·UE 배치

- 이번 요청 범위 완료: 실제 정지 3D 외형·FBX·언리얼 복사 맵 배치·저장 후 재열기·실제 엔진 캡처. 사용자 3D 외형 승인은 별도이며 리그/애니메이션/AI 연결은 하지 않았다.
- 혼합 컨셉과 강도 승인 근거: “이정도로해서 배치해봐” → “실제 3D 모델을 만들어 언리얼 맵에 배치”.
- [현재 원본](../../../ArtSource/monster/rabbit_a/model_hybrid_v01/placement_corrected/rabbit_a_Hybrid.blend) · [모델 QA](../../../ArtSource/monster/rabbit_a/model_hybrid_v01/placement_corrected/qa.json) · [5종 검수 화면](../../../ArtSource/monster/concept_batch_1/hybrid_placement_v01/index.html).
- UE 맵: `/Game/Tests/HybridFiveV01/Lvl_HybridFiveV01`. 메시: `/Game/Monsters/HybridFiveV01/rabbit_a/SM_rabbit_a_Hybrid.SM_rabbit_a_Hybrid`. 초안 외형 높이 2.10 m, 28,925 triangles. 게임 판정 크기를 변경하지 않았다.
- 실제 원본 GLB와 이전 모델을 보존했다. 아래 기존 기록은 과거 버전 이력이다.
- 남은 한계: 최종 표면·노멀·그림자 마감. 검토용 양면 재질과 모델 그림자 투영 비활성 설정을 사용했다. 쥐 몸통의 거친 면, 양 얼굴 경계와 거위 목의 국부 면 정리가 남는다.

```json
{
  "concept_pipeline": {
    "schema_version": 2,
    "asset_id": "rabbit_a",
    "scope": "production",
    "requested_outputs": [
      "finished surface topology",
      "matching CL9 vertex-color materials",
      "Idle",
      "Hop",
      "Thump",
      "Charge",
      "Headbutt",
      "Flee",
      "HitReact",
      "PushReact",
      "Death",
      "EarsUpStartle",
      "HopPause",
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
      "candidate_version": "model_hybrid_v01/placement_corrected",
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
          "evidence": "ArtSource/monster/rabbit_a/animation_hybrid_v02/manifest.json"
        },
        "animation": {
          "status": "pass",
          "evidence": "ArtSource/monster/rabbit_a/animation_hybrid_v02/manifest.json"
        },
        "modelQA": {
          "status": "pass",
          "evidence": "ArtSource/monster/rabbit_a/animation_hybrid_v02/manifest.json"
        }
      },
      "runtime": {
        "level": "actual_host_client_gameplay",
        "gameplay_connected": true,
        "asset_paths": [
          "/Game/Monsters/HybridFiveFinishV02/rabbit_a/Build_bce17f1675/SK_rabbit_a_HybridV02.SK_rabbit_a_HybridV02",
          "/Game/Monsters/HybridFiveFinishV02/rabbit_a/Build_bce17f1675/SK_rabbit_a_HybridV02_Skeleton.SK_rabbit_a_HybridV02_Skeleton",
          "/Game/Monsters/HybridFiveFinishV02/rabbit_a/Build_bce17f1675/Clips_075cde2487/AN_Rabbit_Idle.AN_Rabbit_Idle",
          "/Game/Monsters/HybridFiveFinishV02/rabbit_a/Build_bce17f1675/Clips_04b80c6956/AN_RabbitV02_Hop.AN_RabbitV02_Hop",
          "/Game/Monsters/HybridFiveFinishV02/rabbit_a/Build_bce17f1675/Clips_461534c788/AN_Rabbit_Thump.AN_Rabbit_Thump",
          "/Game/Monsters/HybridFiveFinishV02/rabbit_a/Build_bce17f1675/Clips_1eaea5244d/AN_RabbitV02_Charge.AN_RabbitV02_Charge",
          "/Game/Monsters/HybridFiveFinishV02/rabbit_a/Build_bce17f1675/Clips_ff4232dafb/AN_RabbitV02_Headbutt.AN_RabbitV02_Headbutt",
          "/Game/Monsters/HybridFiveFinishV02/rabbit_a/Build_bce17f1675/Clips_530323ecf5/AN_RabbitV02_Flee.AN_RabbitV02_Flee",
          "/Game/Monsters/HybridFiveFinishV02/rabbit_a/Build_bce17f1675/Clips_45a2921700/AN_Rabbit_HitReact.AN_Rabbit_HitReact",
          "/Game/Monsters/HybridFiveFinishV02/rabbit_a/Build_bce17f1675/Clips_7f2f155fee/AN_RabbitV02_Death.AN_RabbitV02_Death",
          "/Game/Monsters/HybridFiveFinishV02/rabbit_a/Build_bce17f1675/Clips_86d4377c71/AN_Rabbit_EarsUpStartle.AN_Rabbit_EarsUpStartle",
          "/Game/Monsters/HybridFiveFinishV02/rabbit_a/Build_bce17f1675/Clips_429ec0e2bc/AN_Rabbit_HopPause.AN_Rabbit_HopPause",
          "/Game/Monsters/HybridFiveFinishV02/rabbit_a/Build_bce17f1675/Clips_66fe446f47/AN_Rabbit_PushReact.AN_Rabbit_PushReact"
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
      "source_blender_version": "model_hybrid_v01/placement_corrected",
      "candidate_version": "animation_hybrid_v02",
      "motion_plan": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/plan.json",
      "delivery_status": "complete",
      "species_manifest": "ArtSource/monster/rabbit_a/animation_hybrid_v02/manifest.json",
      "previous_delivery": "ArtSource/monster/concept_batch_1/hybrid_animation_v01/delivery.json",
      "current_manifest": "ArtSource/monster/rabbit_a/animation_hybrid_v02/manifest.json",
      "modelQA": {
        "status": "pass",
        "evidence": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/delivery.json"
      }
    },
    "checkpoint": {
      "path": "ArtSource/monster/rabbit_a/animation_hybrid_v02/RabbitHybrid_Finished.blend",
      "sha256": "931e092169e771c0b5191aa78ced7797785e1089a0552b4cdba7f155fd2cabee"
    },
    "job": null,
    "last_completed": "V02 기존 혼합 외형 마감·정점색 재질·11개 모션·엔진 및 실제 host/client 검수와1배속 영상 완료. 별도 사용자 외형 승인을 뜻하지 않음.",
    "next_action": "새 사용자 요청에 따라 현재 v02 체크포인트에서 재개",
    "blocker": null,
    "previous_requested_outputs": [
      "species-specific rig",
      "Idle",
      "Hop",
      "Thump",
      "Charge",
      "Headbutt",
      "Flee",
      "HitReact",
      "PushReact",
      "Death",
      "EarsUpStartle",
      "HopPause",
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
              "evidence": "ArtSource/monster/rabbit_a/model_hybrid_v01/placement_corrected/qa.json"
            },
            "engine": {
              "status": "native_preview_pass",
              "evidence": "ArtSource/monster/concept_batch_1/hybrid_animation_v01/native_final_qa.json"
            },
            "model_rig": {
              "status": "pass_with_recorded_surface_limitations",
              "evidence": "ArtSource/monster/rabbit_a/animation_hybrid_v01/qa.json"
            },
            "animation": {
              "status": "authored_and_video_reviewed",
              "evidence": "ArtSource/monster/rabbit_a/animation_hybrid_v01/manifest.json"
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
              "/Game/Monsters/HybridFiveMotionV01/rabbit_a/Build_3b165486d1_Normals/SK_rabbit_a_Hybrid.SK_rabbit_a_Hybrid",
              "/Game/Monsters/HybridFiveMotionV01/rabbit_a/Build_3b165486d1_Normals/Clips_39499f1ef7/AN_Rabbit_Idle.AN_Rabbit_Idle",
              "/Game/Monsters/HybridFiveMotionV01/rabbit_a/Build_3b165486d1_Normals/Clips_51b023473a/AN_Rabbit_Hop.AN_Rabbit_Hop",
              "/Game/Monsters/HybridFiveMotionV01/rabbit_a/Build_3b165486d1_Normals/Clips_300270c736/AN_Rabbit_Thump.AN_Rabbit_Thump",
              "/Game/Monsters/HybridFiveMotionV01/rabbit_a/Build_3b165486d1_Normals/Clips_2c48d9884b/AN_Rabbit_Charge.AN_Rabbit_Charge",
              "/Game/Monsters/HybridFiveMotionV01/rabbit_a/Build_3b165486d1_Normals/Clips_0c24f3b62e/AN_Rabbit_Headbutt.AN_Rabbit_Headbutt",
              "/Game/Monsters/HybridFiveMotionV01/rabbit_a/Build_3b165486d1_Normals/Clips_e3f0093249/AN_Rabbit_Flee.AN_Rabbit_Flee",
              "/Game/Monsters/HybridFiveMotionV01/rabbit_a/Build_3b165486d1_Normals/Clips_98d6c0eb8c/AN_Rabbit_HitReact.AN_Rabbit_HitReact",
              "/Game/Monsters/HybridFiveMotionV01/rabbit_a/Build_3b165486d1_Normals/Clips_41a12997aa/AN_Rabbit_Death.AN_Rabbit_Death",
              "/Game/Monsters/HybridFiveMotionV01/rabbit_a/Build_3b165486d1_Normals/Clips_1441c53bce/AN_Rabbit_EarsUpStartle.AN_Rabbit_EarsUpStartle",
              "/Game/Monsters/HybridFiveMotionV01/rabbit_a/Build_3b165486d1_Normals/Clips_49744d1497/AN_Rabbit_HopPause.AN_Rabbit_HopPause",
              "/Game/Monsters/HybridFiveMotionV01/rabbit_a/Build_3b165486d1_Normals/Clips_e42908228c/AN_Rabbit_PushReact.AN_Rabbit_PushReact"
            ],
            "gameplay_connected": false
          },
          "source_blender_version": "model_hybrid_v01/placement_corrected",
          "candidate_version": "animation_hybrid_v02",
          "motion_plan": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/plan.json",
          "delivery_status": "full_finish_working",
          "species_manifest": "ArtSource/monster/rabbit_a/animation_hybrid_v01/manifest.json",
          "previous_delivery": "ArtSource/monster/concept_batch_1/hybrid_animation_v01/delivery.json"
        },
        "checkpoint": {
          "path": "ArtSource/monster/rabbit_a/animation_hybrid_v01/RabbitHybrid_Animated.blend",
          "sha256": "0bef96bcef910fbfa226cbfe03feaa838fd49c0dba23e1d66b6e106f04ed92a6"
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
- 현재 `production / blender_review`, 상태 `awaiting_user`. 토끼 비율을 수정한 `model_rabbit_v03/`가 현재 후보다. v02는 비교용으로 보존한다.
- 기존 도형 조합의 분리된 얼굴/팔다리와 시안 불일치를 바로잡기 위해 원래 시안 기반 입력 → 이미지 기반 3D 생성 → Blender 다각도 보정 경로를 검증한다.
- 양·토끼·생쥐·거위·너구리 5종만 제작. 사용자 답변에 따라 어미닭·횃대닭은 참고로만 보관. 기존 소·닭 탈·돼지 보존.

- v02의 실제 체크포인트와 검수 결과는 아래에 기록한다. 사전 10분 조사는 완료된 기록을 유지한다.

상태: 토끼 비율 수정 외형 검토 대기 (`model_rabbit_v03`, 사용자 미승인)
작성일: 2026-10-19 · 변경일: 2026-10-20

제작 1묶음([production-batch-1.md](C:\Work\CL9\docs\asset\animation\production-batch-1.md)) 0-a 단계의 대상이다. 토끼를 컨셉부터 `/cl9film` 한 편까지 끝까지 보내면서 네발 규격(뼈 이름·계층·피벗·판정 크기 재는 법·FBX 프로파일·ABP 복제 방식)을 확정한다.
2026-10-20 v01의 실제 Blender 후보와 4뷰 저장·재열기 검수는 아래 이력으로 보존한다. 이후 사용자가 v01을 반려했으며 현재 재개 대상은 토끼 비율 수정 v03이다. 새 모델은 사용자 미승인 상태이며, v02 저장·재열기·외형 검수 결과는 아래 현재 체크포인트에 기록한다. 리그·행동형·이족 광폭화·게임 연결은 검증하지 않았다([model-concept.md](C:\Work\CL9\docs\asset\model-concept.md) "승인과 자율 진행").
값 표기: 문서에서 옮긴 값에는 출처를 붙였고, 이 카드가 제안하는 값에는 "추정", 문서에 없는 값에는 "미확인"을 붙였다.

<a id="concept-checkpoint"></a>

## 제작 범위·승인·재개 기록

절차·필드 의미는 [model-concept.md](C:\Work\CL9\docs\asset\model-concept.md) "상태 형식"이 정본이다.

> 아래 이력의 이전 체크포인트는 이번 혼합 동물 배치 기록으로 대체되었습니다.

<details><summary>이전 파이프라인 기록</summary>

```yaml
concept_pipeline:
  schema_version: 2
  asset_id: rabbit_a
  scope: production
  requested_outputs: [] # 상세 후보 목록은 아래 "클립" 표; v02 제작 완료를 뜻하지 않음
  review_policy: concept_and_model
  review_override: null
  phase: blender_review
  status: awaiting_user
  updated_at: 2026-10-20T05:49:13+00:00
  concept:
    candidate_version: null
    approved_version: null
  blender:
    candidate_version: model_rabbit_v03
    approved_version: null
    source_concept_version: sheet_six_species_four_states_v5
  production:
    source_blender_version: null
    candidate_version: null
    manifest: null
    qa:
      motion_probe: {status: not_run, evidence: null}
      model_rig: {status: not_run, evidence: null}
      animation: {status: not_run, evidence: null}
      engine: {status: not_run, evidence: null}
    runtime:
      level: not_tested
      engine: null
      asset_paths: []
      evidence: null
  pending_review: {stage: blender, version: model_rabbit_v03, evidence: ArtSource/monster/rabbit_a/model_rabbit_v03/final_candidate/preview}
  checkpoint: {path: ArtSource/monster/rabbit_a/model_rabbit_v03/final_candidate/rabbit_a.blend, sha256: 2ffad115719f79623b596fcf918bbba913b91ab253232259914b607e05302b93}
  job: null # 실행 중 생성 작업은 root 관리; 이 카드에는 작업 ID 미기록
  last_completed: "토끼의 공 같은 비율 수정·저장 후 재열기·실제 4방향 렌더 직접 확인"
  next_action: "수정된 토끼 외형에 대한 사용자 검토; 승인 후 기존 production 범위의 리그·동작·UE 단계"
  blocker: null
```

</details>

## 2026-10-20 토끼 비율 수정 v03

- 사용자 요청: “좀더 토끼같이해봐 너무 그냥 공같음”. 이전 원문의 공 같은 몸보다 이번 비율 수정 요청을 우선한다.
- 현재 원본: [rabbit_a.blend](../../../ArtSource/monster/rabbit_a/model_rabbit_v03/final_candidate/rabbit_a.blend), SHA256 `2ffad115719f79623b596fcf918bbba913b91ab253232259914b607e05302b93`.
- [실제 네 방향 렌더](../../../ArtSource/monster/rabbit_a/model_rabbit_v03/final_candidate/preview/three_quarter.png) · [제작 기록](../../../ArtSource/monster/rabbit_a/model_rabbit_v03/final_candidate/manifest.json) · [수정 전후 비교](../../../ArtSource/monster/concept_batch_1/model_review_v02/index.html#rabbit_a).
- 사용자의 '너무 그냥 공같음' 피드백에 맞춰 긴 타원형 몸통, 작은 둥근 머리, 좁은 가슴과 큰 후구가 구분되도록 구조를 다시 잡았다.
- 앞다리 두 개가 몸을 지지하고 큰 뒷다리가 접힌 토끼 자세다. 흰 몸·분홍 귀와 코·검은 눈·작은 솜꼬리는 유지했다.
- 실제 네 방향에서 비율과 연결을 확인했다. 하나의 닫힌 메시로 경계·비매니폴드 에지는 0이며, 기존 v02 원본은 보존했다.
- 뒤몸의 약한 면 굴곡과 눈·귀 색 경계의 작은 계단이 남는다. 440,060 tris의 외형 후보이며 게임용 최적화·리그·UE 검수는 아직 진행 전이다.
- 기존 v02 파일은 보존했다. 게임용 토폴로지·LOD·리그·동작·UE는 미검증이며 이 수정은 정상 외형 후보에만 적용한다.



## 이력 — 2026-10-20 v02 실제 Blender 외형 후보

- 새 실제 원본: [rabbit_a.blend](../../../ArtSource/monster/rabbit_a/model_reference_v02/final_candidate/rabbit_a.blend), SHA256 `2416821cb444a50c33f88f69f9df2d3d7ab16004c0b56a3b83570b16483dd8a2`.
- [실제 네 방향 렌더](../../../ArtSource/monster/rabbit_a/model_reference_v02/final_candidate/preview/three_quarter.png) · [제작/재열기 기록](../../../ArtSource/monster/rabbit_a/model_reference_v02/final_candidate/manifest.json) · [5종 비교 화면](../../../ArtSource/monster/concept_batch_1/model_review_v02/index.html).
- 방식: 기존 6종 4상태 시안의 해당 정상형을 단독 2D 입력으로 정리한 뒤 로컬 TRELLIS.2에서 3D 생성, Blender에서 검사·필요한 오류 보정. `input.png`는 입력 그림이고 `preview/*.png`는 실제 저장 모델의 렌더다. 원래 v01과 생성 원본 GLB를 보존한다.
- 생성 조건: 512 / seed 42 / 12 steps / 목표 50,000 faces / 2,048 texture. 축 +X 정면/+Z 위, 단위 m. 후보 크기는 외형 비교용이며 게임 판정을 변경하지 않았다.
- 실측 메시 기록: `{"mesh_objects": 1, "vertices": 278476, "triangles": 556948, "bounds_min_m": [-0.5100194215774536, -0.480681449174881, 0.00040865372284315526], "bounds_max_m": [0.4906347990036011, 0.48068082332611084, 0.9987521767616272], "bounds_size_m": [1.0006542205810547, 0.9613622426986694, 0.9983435273170471], "boundary_edges": 0, "nonmanifold_edges": 0}`.
- 정면·측면·후면·사선에서 둥근 몸, 두 귀, 앞발 두 개와 큰 뒷발, 중앙의 꼬리 하나를 확인했다.
- 찢어진 뒤꿈치를 닫고 귀·발 연결을 다듬었다. 최종 메시의 boundary와 nonmanifold edge는 0이다.
- 볼과 등 일부의 울퉁불퉁함이 남는다. 556,948 tris의 외형 후보이며 게임용 토폴로지와 LOD 정리가 필요하다.
- 검증 범위: 실제 파일 저장·재열기, 텍스처 포함 여부, 정면·측면·후면·사선 렌더 직접 확인. 이는 사용자 승인이나 변형·게임 성능 합격이 아니다.
- 미검증: 변형용 토폴로지/리그, 행동·이족 광폭화·전환, FBX/UE, LOD와 게임 성능. 기존 소·닭 탈·돼지는 수정하지 않았고, 어미닭·횃대닭은 참고만 유지한다.



### 버전 파일 목록

경로는 프로젝트 상대 경로로 기록한다. 아래 v01은 사용자 반려 이력이며 현재 승인 대기본이 아니다.

| 단계 | 버전 | 기반 컨셉 버전 | manifest 경로 | manifest SHA256 |
|---|---|---|---|---|
| 반려된 Blender 정상형 후보(이력) | model_round_v01 | sheet_six_species_four_states_v5 | ArtSource/monster/rabbit_a/model_round_v01/manifest.json | fb49dd227b95542edd156bfd15ef7901d83460ed85eb5846970406f7fc889503 |

이력 체크포인트: `ArtSource/monster/rabbit_a/model_round_v01/rabbit_a.blend`, SHA256 `4aa1cc9af8271364524061731da32759eca6e82f0c867f89ede563cb5df07560`. v02의 현재 체크포인트로 사용하지 않는다.

### 이력 — 2026-10-20 반려된 정상형 외형 후보 v01

- 당시 범위는 **production**, 저장 단계는 `blender_review`였으나 이후 사용자 반려됐다. 아래는 비교 이력이며 v02 제작·검수 완료가 아니다. 기준은 `ArtSource/monster/concept_batch_1/sheet_six_species_four_states_v5.png`와 당시 사용자 토끼 원문이다. 아래의 2026-10-19 기획 치수·리그 표는 초안이며 이 실측값이나 승인으로 대체해서 읽지 않는다.
- 실제 원본: `ArtSource/monster/rabbit_a/model_round_v01/rabbit_a.blend`. 재현 스크립트 `build_model.py`, `manifest.json`, `qa.json`, `preview/{three_quarter,front,side,back}.png`가 같은 폴더에 있다.
- 방식: Sol 에이전트의 Blender 직접 파라메트릭 메시 저작. 조사한 CDmir 원본은 관절 참고에만 사용했으며 이 후보 메시에는 포함하지 않았다. 공 같은 순백 몸·검은 눈·작은 분홍 코·오목한 귀 안쪽과 두께·납작한 큰 뒷발·작은 솜꼬리를 제작했다. 봉제선·털가닥·이빨은 만들지 않았다.
- 실측: **12,212 tris**, 외곽 X/Y/Z **0.859 / 0.684 / 0.995m**(귀·꼬리·발 포함), +X 정면/+Z 위, meter. 이름 있는 메시의 변환은 identity다. 각 부품 nonmanifold edge 0, 외부 이미지·라이브러리 의존성 0을 확인했다.
- 실제 저장 파일을 Blender **5.2.2 LTS**, `--background --factory-startup --disable-autoexec`로 다시 열어 4뷰를 렌더했다. Cycles **CPU 2 threads, 24 samples, 640×640**, 중립광·균일 짙은 회색 배경·바닥 없음. 초기 먼 눈 돌출과 발-몸 분리를 수정하고 4뷰를 직접 확인했다. 에이전트의 자체 검수는 사용자 외형 승인이 아니다.
- 미검증: 겹치는 부품을 가진 외형 후보이며 최종 변형 토폴로지·리그·웨이트·표정 모프·발구르기/박치기 준비·이족 상태/전환·LOD·FBX·UE·게임 연결은 아직 없다. 원본의 4뷰는 정상형 정지 렌더이며 행동 재생 증거가 아니다.
- 당시 후속 계획(미실행·v01 반려로 중단): 외형 검토 뒤 발 접지/귀 접기/이족 전환을 짧게 시험하는 것이었다. 현재 재개 대상은 상단의 v03 토끼 비율 수정이며 새 3D 승인은 없다.

### 사용자 승인·수정 이력

실제 발언과 그 대상 버전만 적는다. v01은 제시 후 반려됐으며 새 모델 승인 기록은 없다.

| 시각(시간대 포함) | 단계·대상 버전 | 결정(OK/NO/수정/철회) | 사용자 발언과 근거 | manifest SHA256 | 다음 행동 |
|---|---|---|---|---|---|
| 2026-10-20(정확한 시각 미기록) | Blender / model_round_v01 | NO | “저거 다 이상해 다시만들어” — 5종 v01 모두 반려 | fb49dd227b95542edd156bfd15ef7901d83460ed85eb5846970406f7fc889503 | model_reference_v02 외형 재제작 |
| 2026-10-20(정확한 시각 미기록) | Blender / model_reference_v02 | 수정 | “좀더 토끼같이해봐 너무 그냥 공같음” — 공 같은 몸에서 토끼다운 머리·몸·다리 비율로 수정 | 기존 v02 manifest 참조 | model_rabbit_v03 외형 수정 |

### 사용자 승인 자리 — 셋

제작 1묶음 2절 0-a의 승인 자리다. 아래 표는 종전 기획 초안의 승인 절차이며 현재 재제작 상태는 상단 체크포인트를 따른다. v01은 제시 후 반려됐고 v02 새 모델 승인은 없다.

| # | 자리 | 무엇을 보고 승인하나 | 지금 상태 |
|---|---|---|---|
| 1 | 컨셉 | 평상시 · 무서운 순간 이미지 한 쌍(아래 프롬프트 절의 후보 가운데 하나)과 톤 기준 넷(결정 #50) | 대기 — 이미지 없음 |
| 2 | 짧은 동작 | Headbutt 0.4초 + 플레이어 Tumble 2~3초 구간의 1배속 영상(아래 "짧은 동작 시험") | 대기 — 컨셉·Blender 외형 검토 뒤 |
| 3 | 게임 연결 | `MonsterTable` 행 `rabbit`·`ABP_Rabbit_A` 연결 뒤 `/cl9film` 한 편(평시·예고·공격·죽음) | 대기 — 표 행은 gameplay 몫 |

## 용도와 모습

- **분류 / Alias / SubAlias:** monster / `rabbit` / 미확인(SubAlias 규칙은 몬스터 표가 정한다).
- **사용할 장소·플레이 상황:** 실내의 몬스터 층(`Tier` `Monster`, 게이지 없음)이고, 문 밖으로 따라나가지 않는다(새유형 5절 칸 9, 결정 #35·#37). 평시에는 풀을 뜯거나 배회하다가 사람을 보면 귀를 세우고 멈칫 뛰기로 다가오고, 0.5초 웅크린 뒤 들이받는다. 큰 소리(타격·딸랑이·딱총·큰 목소리)를 들으면 귀가 서고 14초 동안 달아난다. 판 중간 등장은 통합 4의 B83 추천("토끼를 판 중간 등장에서 빼고 판 시작 종으로 둔다")을 따르면 없다.
- **기획 및 시스템 문서:** [monster-newtypes.md](C:\Work\CL9\docs\plan\draft\monster-newtypes.md) 5절(종 카드), [art-plan.md](C:\Work\CL9\docs\plan\draft\art-plan.md) 4.21·5.1·5.2, [story-draft.md](C:\Work\CL9\docs\plan\draft\story-draft.md) 4.5(탈 없는 동물 — 추천 가)·6절(톤), [decisions.md](C:\Work\CL9\docs\decisions.md) #44(큰 목소리로도 쫓긴다)·#48(10종 묶음)·#50(톤 기준 넷)·#52(탈 없는 종에는 사연·쪽지가 없다).
- **기존 메시·리그를 재사용하는지:** 하지 않는다. 새 모델·새 스켈레톤이다. 기존 네발 리그(`pig_a`·`raccoon_a`·`sheep_a`)의 뼈 이름 방식만 이어 쓰는 안을 아래에 적었다. 클립 재사용은 없다.
- **형태·재질·색:** 큰 귀가 몸 높이의 절반쯤 되는 둥근 몸에 흰색과 회색의 넓은 면을 칠한다(art-plan 4.21). 겁 많은 큰 눈과 큰 뒷발(뒷발 구르기)이 특징이다. 색 수를 제한하고 넓은 색면으로 칠하며 채도를 높인다([design-concept.md](C:\Work\CL9\docs\asset\design-concept.md) 소재 불가지 규칙 2). 흰색·회색은 채도가 낮은 색이므로, 귀 안쪽과 코에 채도 높은 분홍 한 가지를 강조색으로 두는 것을 기본으로 한다(추정). 털은 한 가닥씩이 아니라 큰 덩어리로 만든다([common.md](C:\Work\CL9\docs\asset\common.md)). Vertex Color 단일 재질과 Flat Shading은 기존 몬스터와 같다.
- **기준 이미지:** 없음. 아래 프롬프트로 만든 후보 가운데 사용자가 고른 파일의 경로·SHA256을 여기에 적는다.
- **생성 도구:** 이미지는 ChatGPT(사용자가 2026-10-13에 정한 길) 또는 Tripo 이미지 생성(무료 할당)으로 만든다 — [tools/tripo.md](C:\Work\CL9\docs\asset\tools\tripo.md). 3D 초안은 직접 모델링과 이미지 → 3D(Tripo·TRELLIS, [tools/trellis2.md](C:\Work\CL9\docs\asset\tools\trellis2.md)) 가운데 3D 단계에서 고른다(미확인).
- **리썰/R.E.P.O.에서 참고한 특징:** "위협은 알아볼 수 있는 체형과 큰 예고 동작"([reference-direction.md](C:\Work\CL9\docs\asset\reference-direction.md))을 따라, 멈칫 뛰기와 웅크림이 손전등 밖 실루엣으로도 읽혀야 한다. "결과는 웃기게"는 들이받힌 플레이어가 200cm 날아가 나뒹구는 장면에 적용한다.
- **이번 제작 범위 / 연결이 필요한 기능:** `production`이다. 모델·네발 규격 리그·11클립·FBX·UE 왕복·표 행·ABP·촬영까지 간다. 표 행 `rabbit`의 새 열(`NoiseReaction` `Flee`, `FleeNoiseRadius`, `CatchFlingDistance` 등)과 상태 기계 변경(멈칫 뛰기·소리로 달아남)은 gameplay 몫의 **연결 필요**다.

## 동물·탈 구조와 짧은 시험

- **탈:** 해당 없음. 토끼는 탈 없는 동물이다(story-draft 4.5 추천 가). 평상 모델에 실밥·이음매를 넣지 않는다(톤 기준 ④는 탈 있는 종에만 걸린다 — 통합 4 2절 25번).
- **유지할 외형·비율과 종별 차별점:** 귀는 몸 높이의 절반쯤, 몸은 둥글고 뒷발은 크다. 닭(두 발·날개)·돼지(낮고 긴 몸)·너구리와 어둠 속 실루엣을 가르는 핵심은 **서 있는 두 귀**다. 같은 귀가 Ears Up Startle에서는 확 서고, 무서운 순간에는 등 쪽으로 눕는다.
- **본체 관절/이동 유형:** 네발로 뛴다(앞발로 착지한 뒤 뒷발이 앞발 앞으로 넘어오는 토끼식 보행). 웅크리면 뒷발 전체(발목~발가락)가 바닥에 붙는다.
- **발 지지 위치:** 앞발 두 개와 뒷발 발바닥 전체다. 웅크림·Thump 때 뒷발 뒤꿈치의 접지가 바뀌므로 `hind_l_foot`·`hind_l_toe` 두 뼈로 접지를 제어한다(아래 뼈 표).
- **가장 어려운 2~3초:** Headbutt + 플레이어 Tumble이다(아래 "짧은 동작 시험").
- **1배속·중립광·측면 확인과 수치 검사:** 아래 시험 절의 검사 항목을 쓴다. 단위는 cm이고, 허용치는 첫 시험 전에 정해 여기에 적는다(미확인).
- **짧은 엔진 왕복:** 몸은 스켈레탈 하나이고, 탈이 없으므로 모프·천은 필요 없다. 표정(겁 많은 눈 ↔ 노려보는 눈)을 모프로 할지 뼈로 할지는 미확인이다 — 닭·돼지 v2의 "상태별 눈 3단"도 방식이 아직 정해지지 않았다.
- **시험 결론·남은 문제:** 아직 없다.

## 치수와 기술 계약

| 항목 | 값 및 상태 |
|---|---|
| 원본 크기 X×Y×Z (cm) | **미확인(첫 블록아웃에서 정한다).** 시작점은 귀를 세운 최고점 약 90cm, 몸통 높이 약 60cm, 길이 약 95cm, 폭 약 65cm다(모두 추정). 키 90cm는 art-plan 4.21의 추정값이고, 귀를 포함한 높이인지는 문서에 없다. 이 카드는 "귀 포함 90"을 제안한다 — "큰 귀가 몸 높이의 절반쯤"(몸 60 + 귀 30)과 "사람 허리까지"가 근거다(추정) |
| 런타임 크기·자동 배율 | 몬스터에는 Radius 자동 배율이 없다. 실제 cm로 만들고 액터 Scale은 1이다([monster/README.md](C:\Work\CL9\docs\asset\monster\README.md) "모델과 크기") — 준수 |
| 판정 반경/캡슐·출처 | `Radius` 35(추정 — art-plan 4.21·9.3 C2의 제안값, 정본은 `MonsterTable`). `CatchRadius` 84, `PounceRadius` 300, `PounceLunge` 250, `PounceWindup` 0.5초, `PounceMoveTime` 0.3초(새유형 5절 칸 9 제안값). **판정 크기 재는 법(규격 확정 항목):** 닭·돼지처럼 몸통(귀·꼬리 제외) 가로 단면의 반폭을 실측해 `Radius` 근처에 두고, 전체 외형을 판정 크기로 쓰지 않는다(chicken_a 2026-09-11 결과, pig.md `Radius` 45 = 에셋 구 반경 실측). 실측값은 manifest에 남긴다 — 초안 |
| 피벗·정면·기준 포즈 | 피벗은 **네 발 사이 바닥 Z=0**이고 `root`가 그 자리에 있다. UE +X 정면, `MeshYawOffset` 0을 목표로 한다. 기준 포즈는 네 발로 선 Idle 자세(귀 세움)를 제안한다(추정 — 레스트 포즈를 웅크림으로 할지 선 자세로 할지는 첫 리그에서 정한다) — 초안 |
| 충돌 소유자·메시 충돌 필요 여부 | 게임 충돌은 몬스터 액터가 소유한다. 메시 충돌은 필요 없다. Physics Asset은 Death 래그돌을 쓸 때만 만든다(선택) — 준수 |
| 원본·FBX·UE 목적 경로 | 원본 `ArtSource/monster/rabbit_a/rabbit_a.blend`, FBX `ArtSource/monster/rabbit_a/export/`(`SK_Rabbit_A.fbx`, `AN_Rabbit_<클립>.fbx`, `manifest.json`), UE `/Game/Monsters/RabbitA/`(`SK_Rabbit_A`, `SKEL_Rabbit`, `AN_Rabbit_*`, `ABP_Rabbit_A`) — 기존 명명([common.md](C:\Work\CL9\docs\asset\common.md))을 따른 초안 |
| 삼각형·재질 슬롯·텍스처・LOD | 5,000~15,000 tris(동물 한 마리 목표, common.md)이고, 실제 목표는 기존 몬스터와 같은 약 1만 tris다(design-concept 규칙 3). Vertex Color 재질 1슬롯, 텍스처 없음이 기본이다(기존 5종과 같다). LOD1 약 50%, LOD2 약 25%에서 귀·눈 실루엣을 지킨다. 한 판에 동시 최대 2마리다(`SpawnMaxCount` 2 — 새유형 칸 9 추정) — 초안 |
| Skeleton·뼈 수·최대 영향 뼈 수 | 새 `SKEL_Rabbit`, 변형 뼈 **26개 제안**(아래 표). 정점당 영향 뼈 최대 4(common.md 목표), 가중치 합 1, 미할당 정점 0 — 초안 |
| 부착 기준·소켓/마커 소비처 | 머리 앞 접촉점(Headbutt가 플레이어 배에 닿는 자리 — 짧은 시험의 검산 지점, 소켓 이름은 미확인). 그 밖의 부착은 없다. 소리 원점(쿵쿵 `ThumpSoundRadius`)을 액터 위치로 둘지 뒷발 소켓으로 둘지는 미확인이다 |

### 네발 규격 — 뼈 이름·계층 제안

토끼가 처음 쓰고 **눈먼 개·여우가 이어 쓰는 규격**이다. 하나의 스켈레톤 에셋을 셋이 같이 쓰는 것이 아니라 이름·계층·축 규칙을 같이 쓰고, 레스트 포즈와 뼈 길이는 종마다 따로 둔다(art-plan 9.2 B3 추천 "규격 공유"). 클립은 리타깃해 시작점으로만 쓴다.

**이름 방식 — 후보 둘.**

| 후보 | 모양 | 얻는 것 | 비용 |
|---|---|---|---|
| **가. 기존 네발 방식 — 추천** | `ear_l_01`, `front_l_upper`, `hind_l_foot`(좌우 표시가 부위 이름 바로 뒤) | 지금 게임에 연결된 네발 셋(`pig_a`·`raccoon_a`·`sheep_a` — `C:\Work\CL9\Tools\Blender\build_pig_a.py`·`build_raccoon_a.py`의 `front_l_upper`·`ear_l`)과 모양이 같아 빌드·반응 클립 스크립트의 이름 규칙이 한 갈래로 남는다 | 좌우 표시가 이름 끝에 오지 않는다 |
| 나. 접미사 방식 | `ear_01_l`, `front_upper_l`, `hind_foot_l`(좌우 표시가 맨 끝) | 이름 끝만 보고 좌우를 바꿀 수 있다. UE 미러 표의 기본 짝 찾기와 맞는지는 미확인이다 | 기존 네발 셋과 이름 규칙이 둘로 갈린다 |

추천은 **가**다. [common.md](C:\Work\CL9\docs\asset\common.md)는 "좌우 뼈는 `_l`/`_r`"만 정하고 위치는 정하지 않았으며 기존 리그 이름은 바꾸지 않으므로, 새 규격을 기존 쪽에 맞추는 편이 갈래가 적다. 아래 표는 가의 이름으로 적었다.

**계층 — 변형 뼈 26개(제안).** Blender에서 +X 정면·+Z 위로 두고, 다리 뼈는 아래로, 귀 뼈는 위로 향하게 한다(기존 리그와 같은 관행 — 추정). 컨트롤·IK 뼈는 `RIG_CONTROLS`에 두고 내보내지 않는다.

| # | 뼈 | 부모 | 하는 일 | 토끼 | 눈먼 개·여우가 이어 쓸 때 |
|---|---|---|---|---|---|
| 1 | `root` | — | 바닥 원점(네 발 사이 Z=0). 이동 클립에서 고정한다 | 그대로 | 그대로 |
| 2 | `body` | root | 골반·엉덩이. 몸 위치·높이의 기준이다 | 웅크림·뛰기의 무게 중심 | 그대로 |
| 3 | `spine` | body | 등 가운데. 몸을 둥글게 말거나 편다 | 뛰기의 등 굽힘, 낮게 까는 자세 | 개의 흔들기(Shake Fling), 여우의 낮은 자세 |
| 4 | `chest` | spine | 가슴·앞다리 뿌리 | 앞발 착지의 받침 | 그대로 |
| 5 | `neck` | chest | 목 | 짧게 둔다. 들이받기 직전에 머리를 숙인다 | 개의 물고 흔드는 목(art-plan 4.18 리그 요구) |
| 6 | `head` | neck | 머리 | 들이받는 머리 | 그대로 |
| 7 | `jaw` | head | 아래턱 | 오물거림(Idle의 풀 뜯기) | 개 Lunge Bite, 여우 Pounce Grab·Drag Run(문 채 끌기)에 필수 |
| 8~9 | `ear_l_01` · `ear_l_02` | head · ear_l_01 | 귀 뿌리 · 귀 끝 | 큰 귀 두 뼈씩(art-plan 4.21 리그 요구) — 서기·눕기·끝 처짐 | 개 Ear Turn(귀를 소리 쪽으로 돌림 — 4.18 리그 요구), 여우 귀 |
| 10~11 | `ear_r_01` · `ear_r_02` | head · ear_r_01 | 오른쪽, 같은 일 | | |
| 12~14 | `front_l_upper` · `front_l_lower` · `front_l_paw` | chest · 앞 뼈 · 앞 뼈 | 앞다리 위 · 아래 · 발 | 짧은 앞발 | 그대로(길이만 종마다 다르다) |
| 15~17 | `front_r_upper` · `front_r_lower` · `front_r_paw` | chest · 앞 뼈 · 앞 뼈 | 오른쪽, 같은 일 | | |
| 18~21 | `hind_l_upper` · `hind_l_lower` · `hind_l_foot` · `hind_l_toe` | body · 앞 뼈 · 앞 뼈 · 앞 뼈 | 허벅지 · 정강이 · 긴 발(발목~발가락 뿌리) · 발가락 | **뒷발 구르기의 핵심.** 웅크림에서는 foot 전체가 바닥에 닿고, Thump에서는 foot을 들었다가 내려친다 | 개·여우에서는 foot이 짧은 발허리, toe가 발가락이 된다 — 같은 네 마디라 접지 제어가 같아진다 |
| 22~25 | `hind_r_upper` · `hind_r_lower` · `hind_r_foot` · `hind_r_toe` | body · 앞 뼈 · 앞 뼈 · 앞 뼈 | 오른쪽, 같은 일 | | |
| 26 | `tail_01` | body | 꼬리 | 짧은 솜꼬리 하나 | 종마다 `tail_02`…를 더한다(여우는 "꼬리가 몸만 하다" — 4.19, 뼈 수는 여우 카드 몫). 규격은 두 자리 번호 `tail_NN`이다(pig_a `tail_01`~`04`와 같다) |

- 기존 `pig_a` 24뼈와의 차이는 `spine` 하나, 귀 두 번째 마디 둘, 뒷발 넷째 마디(`hind_*_toe`) 둘을 더하고 꼬리를 하나로 줄인 것이다. 앞발은 `pig_a`와 같은 세 마디다.
- 눈꺼풀·눈썹(겁 많은 눈 ↔ 노려보는 눈)은 뼈를 더하지 않고 모프나 정점 이동으로 두는 안이 있다(추정). 방식은 미확인이다.
- 이 표는 **제안**이다. 첫 리그에서 극단 포즈(웅크림·Thump·Death)를 시험한 뒤 바뀌면 이 표를 고치고, 눈먼 개·여우 카드가 고친 표를 이어 받는다.

## 클립 / 상태별 변형

공통 규칙은 30fps, 제자리 이동(in-place), Root Motion 없음, 루프의 첫·끝 포즈 일치, 로코모션은 현재 속도 / 기준 속도로 재생하는 것이다([common.md](C:\Work\CL9\docs\asset\common.md)). 클립 하나당 Action 하나이고, 애니메이션은 판정을 만들지 않는다. 길이의 출처는 [art-plan.md](C:\Work\CL9\docs\plan\draft\art-plan.md) 2.2 토끼 줄과 새유형 5절 칸 8·9·12다.

| 이름 | 길이(초) | fps | 루프 | 루트 이동 | 트리거(상태) | 플레이어 짝 |
|---|---|---|---|---|---|---|
| `AN_Rabbit_Idle` | 2.0(추정) | 30 | 예 | 없음, 발 고정 | 평시(풀 뜯기·배회 중 정지). 코 킁킁 예고음과 맞춘다 | 없음 |
| `AN_Rabbit_Hop` | 미확인 — 뛰기 한 번(150cm)을 한 주기로 제안(추정) | 30 | 예 | 없음, 기준 속도 180cm/s(추정 — `ChaseSpeed` 평균값) | 다가옴(멈칫 뛰기), 배회 | 없음 |
| `AN_Rabbit_Thump` | 0.5 | 30 | 아니오 | 없음 | 후보 둘 — ① 들이받기 예고(웅크림 0.5초 = `PounceWindup` 0.5)를 이 클립이 겸한다 ② 다가옴 중 소리 이벤트에만 쓰고 예고는 따로 둔다. 추천은 ①이다(길이가 같고, 컨셉의 무서운 순간 "뒷발로 쿵쿵 구르며 몸을 낮게 까는" 자세와 같다). 쓰는 상태는 미확인 | 없음 |
| `AN_Rabbit_Charge` | 미확인(루프) | 30 | 예 | 없음, 기준 속도 미확인 | 미확인 — 덮침이 0.3초 동안 250cm(약 833cm/s)라면 루프보다 원샷이 맞고, 루프로 둔다면 쓰는 상태가 문서에 없다 | 없음 |
| `AN_Rabbit_Headbutt` | 0.4 | 30 | 아니오 | 없음 — 덮침 이동은 코드가 한다(`PounceMoveTime` 0.3, `PounceLunge` 250) | 들이받기(이 종의 주 공격 — monster/README "공격"). 접촉 프레임은 짧은 시험에서 정한다 | **3인칭 Tumble**(1초 — art-plan 2.3) + 날림 200cm·넘어짐 1.5초(`CatchFlingDistance`, 새유형 칸 4). 1인칭은 "화면이 위로 튀고 짐이 흩어진다"(칸 12) — 카메라 표현인지 클립인지 미확인 |
| `AN_Rabbit_Flee` | 미확인(루프) | 30 | 예 | 없음, 기준 속도 650cm/s(`RetreatSpeed` 제안값) | 달아남 14초(`FleeTime`) | 없음 |
| `AN_Rabbit_HitReact` | 0.3 | 30 | 아니오 | 없음 | 경직 = `HitStunMs` 300. 맞는 순간 확 튀고(첫 0.05초) 나머지 동안 돌아온다(README 규칙). 끝나면 달아남으로 간다 | 없음(플레이어 쪽은 기존 Swing) |
| `AN_Rabbit_Death` | 미확인(끝 유지) — 1.2(추정, 닭·돼지와 같은 길이) | 30 | 아니오 | 없음 | HP 0. 끝 포즈를 유지하고 바닥을 뚫지 않는다 | 없음 |
| `AN_Rabbit_EarsUpStartle` | 0.3 | 30 | 아니오 | 없음 | 달아남 진입(600cm 안의 타격·딸랑이·딱총·큰 목소리) → Flee | 없음 |
| `AN_Rabbit_HopPause` | 0.6 | 30 | 아니오 | 없음 | 다가옴의 멈춤(뛰기 150cm마다 0.6초) | 없음 |
| `AN_Rabbit_HoleOut` | 0.5(추정) | 30 | 아니오 | 없음 | 판 중간 벽 구멍에서 나오기. **B83 추천대로 토끼가 판 중간 등장에서 빠지면 만들지 않는다**(통합 4 2절 8번). 구멍 높이 0~40cm에는 키 약 90cm가 들어가지 않는다 | 없음 |

**클립 표에 걸린 문제(연결 필요).**

- **멈칫 뛰기와 발 미끄러짐.** 새유형 칸 8은 "표현은 애니, 판정은 평균 속도로 둔다"고 적었다. 액터가 180cm/s로 고르게 움직이는데 애니가 0.6초 멈추면 그동안 약 108cm를 미끄러진다. 후보는 셋이다 — ① 액터 속도를 뛰기·멈춤 박자에 맞춰 바꾼다(gameplay 변경) ② Hop 한 주기 안에 멈춤을 넣고 액터 속도 곡선을 애니와 맞춘다 ③ 멈춤 없이 고른 뛰기만 쓰고 Hop Pause는 실제로 멈출 때만 쓴다(예고가 약해진다). 이 카드는 고르지 않는다 — gameplay와 함께 정할 일이다.
- **Headbutt 0.4초와 Catch 휴지 0.8초.** README는 잡는 종의 공격 클립 길이를 코드의 Catch 휴지 0.8초에 맞춘다. 토끼 Headbutt는 0.4초로 적혀 있어, 나머지 0.4초를 무엇이 채우는지 미확인이다.
- **PushReact.** 기존 5종에는 프라이팬 밀침 경직 클립(0.8초)이 있지만 토끼 11클립에는 없다. 토끼에게 밀침이 걸리는지 미확인이다.
- **Tumble 1초와 넘어짐 1.5초.** 플레이어 Tumble 1초가 넘어짐 1.5초 안의 어디까지인지(뒤의 0.5초를 Down 유지로 채우는지) 미확인이다.

## 짧은 동작 시험 — Headbutt 0.4초 + 플레이어 Tumble (2~3초)

제작 1묶음 0-a의 승인 자리 2이고, 네발 규격의 첫 검증이다. 외형 검토를 마친 초안과 최소 리그로 이 구간만 먼저 만든다([workflow.md](C:\Work\CL9\docs\asset\workflow.md) "어려운 약 2~3초부터").

| 구간(초, 추정) | 토끼 | 플레이어 |
|---|---|---|
| 0.0~0.5 | Thump(웅크림 예고) — 몸을 낮게 깔고 귀를 등 쪽으로 눕히며 뒷발로 바닥을 친다 | 서 있음(또는 걷기) |
| 0.5~0.8 | Headbutt 앞부분 — 덮침(코드는 0.3초 동안 250cm를 움직인다. 시험에서는 앵커로 거리를 고정한다) | 그대로 |
| 0.8~0.9 | Headbutt 접촉 — 머리가 플레이어 배에 닿는다(새유형 칸 12 "머리로 배를 받아") | Tumble 시작 — 뒤로 날아간다 |
| 0.9~1.9 | Headbutt 끝 → 착지 자세 | Tumble 1초 — 과장되게 구른다 |
| 1.9~2.5 | Idle 첫 프레임으로 돌아온다(재장전) | Down 유지 또는 GetUp 시작(미확인) |

- **2인 정렬:** 두 리그를 각자 blend에서 저작하고, 처형 클립처럼 액터 공간 앵커 표(`C:\Work\CL9\Tools\Blender\execute_anchor.py` 방식 — 원점 = 피해자 캡슐 중심 바닥, +X = 피해자 전방)로 접촉 시각과 거리를 맞추는 안을 제안한다(추정). Headbutt는 처형이 아니므로 표에 새 항목을 둘지 시험용 앵커만 둘지는 미확인이다.
- **볼 것:** 웅크림에서 뒷발 네 마디가 접혀도 발이 바닥을 뚫지 않는지, 귀 두 마디가 뒤늦게 따라오는지, 머리 접촉점이 플레이어 배 높이에 오는지 본다(토끼 머리 높이와 플레이어 배 높이 모두 미확인 — 새 플레이어 외형은 성인형 비율이 아니다). 날림이 웃기게 읽히는지도 본다(통합 4 2절 25번은 "결과 장면이 웃긴가"를 짧은 동작 시험의 기준으로 옮겼다).
- **수치 검사:** 바닥 최저점(cm), root 이동(cm), 접촉 프레임의 머리 앞점을 피해자 공간 좌표로 남긴다(처형 클립의 `contact_probe_cm`과 같은 방식). 허용치는 시험 전에 정한다(미확인).
- **엔진 왕복:** 이 구간을 UE로 작게 왕복한다. 동물 클립은 **Interchange로만** 넣는다(레거시 FBX 경로로 넣으면 요가 −90° 돈다 — monster/README 2026-09-21). 1m 큐브 = 100cm, +X 정면, `MeshYawOffset` 0을 확인한다.
- **납품:** 1배속 MP4(정면·측면), 접촉 근접 영상, 우측 상단 표시(제작 방식·버전·1x·Blender/인게임 구분 — workflow.md 5절).

## 내보내기와 검수 기록

- Blender 버전 / exporter 이름·버전 / 축·단위·베이크 옵션: 2026-10-20 Blender 5.2.2 LTS, +X 정면/+Z 위, meter. 이번 외형 후보는 exporter/베이크 미실행.
- UE 버전 / importer / Skeleton 지정 / 임포트 옵션:
- 중립광·어두운 조명·와이어·클립 미리보기 경로:
- 몸–탈 / 탈 자체 / 손 집기 / 발 접지 / 외형 / 전환 QA를 구분한 결과: (몸–탈·탈 자체·손 집기는 해당 없음)
- 1배속 및 비교 영상 묶음, 우측 상단 제작 방식·버전·속도·렌더/인게임 표시:
- 엔진 임포트·프리뷰·실제 게임 상태 연결의 구분과 실행/로그/재열기 증거:
- 동작 영상의 실제 재생/디코딩·화면 검수 결과, 사용자에게 제시한 영상·시각·피드백:
- 크기·원점·전방·충돌·리그·루프 확인 결과:
- 게임 시나리오 / 2인 PIE / 성능 측정 결과:
- 미실행 항목과 이유: 반려된 v01은 재열기/4뷰/메시·의존성 검사까지만 수행했다. v02의 실제 모델·재열기·4뷰 완료는 아직 기록하지 않았으며 리그·모션·FBX·UE·게임 검수도 미실행이다.
- 알려진 문제·다음 수정: 외형은 사용자 미승인이고, 부품 겹침이 있는 정지 후보라 변형 토폴로지·접지/전환 시험이 필요하다.
- 재개 정보: 현재 대상은 상단 `model_reference_v02` 제작 계획과 체크포인트다. v01 원본·명령·manifest/QA는 반려 이력으로만 보존하며 v02 완료 증거로 승계하지 않는다. 생성 실행 상태는 root가 관리한다.

**규격 확정 때 이 카드에 남길 것**(art-plan 5.1 목록): 네발 뼈 이름·계층과 `root` 규칙, 피벗과 `MeshYawOffset`, 판정 크기를 메시에서 재는 법, 클립 명명과 길이·fps·루프 검사, FBX 프로파일, manifest의 치수·축 검사 항목, UE 폴더와 ABP 복제 방식(`ABP_Chicken` 복제 → `SKEL_Rabbit`), 표 행을 잇는 순서, `/cl9film` 촬영 대본(평시·예고·공격·죽음 한 편).

## 미확인·연결 필요 목록 (19건)

1. 키 90cm가 귀를 포함한 높이인지(이 카드는 귀 포함을 제안한다).
2. SubAlias.
3. 레스트 포즈(선 자세 / 웅크림).
4. 머리 앞 접촉 소켓의 이름과 필요 여부, 뒷발 소리 소켓의 필요 여부.
5. 표정(눈) 변화 방식 — 모프 / 뼈.
6. Hop 한 주기의 길이와 기준 속도, 멈칫 뛰기 발 미끄러짐의 해법(gameplay와 함께 정한다).
7. Thump가 들이받기 예고 클립을 겸하는지.
8. Charge를 쓰는 상태와 기준 속도.
9. Headbutt 0.4초와 코드 Catch 휴지 0.8초의 관계.
10. Flee 루프의 길이.
11. Death의 길이.
12. PushReact가 필요한지.
13. Tumble 1초와 넘어짐 1.5초의 관계, 1인칭 들이받힘이 클립인지 카메라 표현인지.
14. Hole Out을 만들지(B83).
15. 토끼 머리 높이와 플레이어 배 높이(접촉 높이).
16. Headbutt 2인 정렬을 앵커 표의 새 항목으로 둘지.
17. 짧은 시험·수치 검사의 허용치.
18. 3D 초안 방식(직접 모델링 / 이미지 → 3D).
19. UE 미러 표가 이름 방식 나와 맞는지(이름 방식 가를 고르면 해당 없음).

## 컨셉 이미지 프롬프트

후보 넷 × (평상시 · 무서운 순간) 한 쌍씩이다. 프롬프트는 이미지 모델용 영어이고, 아래에 한국어 요지 한 줄을 붙였다. 이미지는 아직 만들지 않았다.

**토끼는 탈이 없는 종이라 "정체가 드러나는 순간"이 따로 없다.** 톤 문장 5("탈이 없는 몬스터는 처음부터 끝까지 귀엽게 크게 만든다 — 무서움은 모습이 아니라 예고와 규칙에서 나온다", story-draft 6.1)를 따라, 무서운 쪽 이미지는 **같은 모델·같은 색**에서 자세와 눈만 바꾼 장면(뒷발 구르기 웅크림, 멈칫 응시)으로 만든다. 새 부위·이빨·상처·실밥은 넣지 않는다.

톤 기준 넷(결정 #50)을 이 종에 적용하는 법은 다음과 같다. ① 평시 귀여움 — 평상시 이미지에서 "귀엽다"나 "웃기다"가 나와야 한다. ② 무서움은 그 순간에만 — 무서운 기운은 무서운 쪽 이미지의 자세와 눈에만 있다. ③ 같은 몸 — 탈이 없으므로 "뒤틀림" 대신 같은 몸이 같은 동작(구르기·들이받기)을 위협적으로 하는 모습으로 읽는다. ④ 이음매 한 곳 — 해당 없다(탈 없는 종, 통합 4 2절 25번).

**공통 꼬리말** — 모든 프롬프트 끝의 `+ 공통 꼬리말` 자리에 이 문장을 그대로 붙인다:

```text
stylized low-poly game creature, chunky rounded shapes, flat-shaded large color areas, limited palette of three or four colors, one saturated accent color, cute and comedic, no realistic fur strands, no blood, no gore, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

### 후보 A — 솜뭉치

평상시:

```text
A giant timid rabbit as tall as a person's waist, its body almost a perfect round ball of pure white, two huge upright ears about half the body height with bright hot-pink insides, a small hot-pink nose, very large glossy black eyes that look shy and nervous, oversized flat hind feet tucked under the round body, a tiny white cotton tail, sitting calmly with its nose twitching, + 공통 꼬리말
```

요지: 흰 공 같은 몸에 몸 높이 절반의 큰 귀(안쪽 진분홍)와 겁먹은 큰 눈을 한, 사람 허리 높이의 토끼가 얌전히 앉아 있다.

무서운 순간:

```text
The same giant white rabbit with exactly the same body, colors and proportions, now pressed low and flat to the ground like a coiled spring, both huge ears pinned straight back along its spine, one oversized hind foot raised high mid-stomp about to slam the floor, eyes wide and staring straight ahead with tiny pinpoint pupils, head lowered ready to headbutt, threatening pose only, no teeth, no wounds, no seams, + 공통 꼬리말
```

요지: 같은 흰 토끼가 몸을 납작하게 깔고 귀를 등에 붙인 채 뒷발 하나를 높이 들어 내려치려 하고, 작아진 눈동자로 정면을 노려본다.

톤 기준: ① 공 모양 몸과 큰 눈이 귀엽다. ② 무서움은 납작한 자세와 작은 눈동자에만 있다. ③ 같은 모델·같은 색이고 새 부위가 없으며, 뒷발 구르기(Thump)를 그대로 위협으로 쓴다. ④ 해당 없음.

### 후보 B — 흰 장화

평상시:

```text
A giant gentle rabbit as tall as a person's waist, a soft pear-shaped body in dusty blue-gray, a bright white belly and comically huge bright white hind feet that look like big boots, tall stiff ears with saturated coral-pink insides, a round white muzzle, big dark worried eyes, standing on all fours and sniffing the air, + 공통 꼬리말
```

요지: 푸른빛 회색의 배 모양 몸에 흰 배와 장화처럼 큰 흰 뒷발, 산호색 귀 안쪽을 한 순한 큰 토끼가 네 발로 서서 냄새를 맡는다.

무서운 순간:

```text
The same blue-gray rabbit with the same white boots and the same proportions, crouched very low with its huge white hind boots planted wide behind it, both tall ears swung forward to point at the viewer like two horns, head tucked down for a charge, front paws pulled in, eyes narrowed into a hard stare, the whole body tense and leaning forward, threatening pose only, no teeth, no wounds, no seams, + 공통 꼬리말
```

요지: 같은 토끼가 흰 장화 같은 뒷발을 넓게 버티고 웅크린 채 두 귀를 뿔처럼 앞으로 겨누고, 머리를 숙여 들이받을 준비를 한다.

톤 기준: ① 큰 흰 장화 발이 우스꽝스럽다. ② 무서움은 앞으로 겨눈 귀와 굳은 눈에만 있다. ③ 같은 몸이 들이받기(Headbutt) 직전의 자세를 취한다. ④ 해당 없음.

### 후보 C — 짝귀

평상시:

```text
A giant clumsy rabbit as tall as a person's waist, a round cream-white body with one large charcoal-gray patch on its back like a saddle, one ear standing straight up and the other ear flopped over one eye in a silly way, a hot-pink nose, round black eyes, chubby cheeks, sitting with a small tuft of grass in its mouth, + 공통 꼬리말
```

요지: 등에 짙은 회색 안장 무늬가 있는 크림색 둥근 토끼가 한쪽 귀는 세우고 한쪽 귀는 눈 위로 접은 채 풀을 물고 있다.

무서운 순간:

```text
The same cream-white rabbit with the same charcoal saddle patch, startled: both ears snapped bolt upright and rigid at the same height, the flopped ear now perfectly straight, the body frozen mid-hop with front paws lifted off the ground, staring directly at the viewer with wide unblinking eyes, a sudden total stillness, threatening pose only, no teeth, no wounds, no seams, + 공통 꼬리말
```

요지: 같은 토끼의 두 귀가 접혀 있던 쪽까지 꼿꼿이 서고, 앞발을 든 채 뛰다 멈춰 눈을 깜빡이지 않고 정면을 본다.

톤 기준: ① 짝귀가 우스꽝스럽다. ② 무서움은 두 귀가 같이 서는 순간과 멈춘 응시에만 있다. ③ 같은 몸이 Ears Up Startle·Hop Pause를 하고, 접힌 귀가 펴지는 변화가 실루엣으로 읽힌다. ④ 해당 없음.

### 후보 D — 큰 발

평상시:

```text
A giant shy rabbit as tall as a person's waist, a bottom-heavy round body in light gray with darker gray ear tips and darker gray feet, enormous long hind feet longer than its head, ears with saturated pink insides, a small white muzzle, sleepy half-closed bashful eyes, lying loaf-shaped with its big feet sticking out in front, + 공통 꼬리말
```

요지: 밝은 회색에 귀 끝과 발이 짙은 회색인, 아래가 무거운 토끼가 머리보다 긴 뒷발을 앞으로 내놓고 식빵처럼 엎드려 졸린 눈을 하고 있다.

무서운 순간:

```text
The same light gray rabbit with the same dark gray feet and ear tips, body arched low and long, ears flat back, both enormous hind feet lifted together and frozen just before slamming the floor, eyes fully open and fixed on the viewer, a heavy stomping pose that suggests a loud thump, threatening pose only, no teeth, no wounds, no seams, + 공통 꼬리말
```

요지: 같은 회색 토끼가 몸을 길고 낮게 휘고 두 큰 뒷발을 함께 들어 바닥을 내려치기 직전에 멈춘 채, 크게 뜬 눈으로 정면을 본다.

톤 기준: ① 식빵 자세와 졸린 눈, 큰 발이 귀엽다. ② 무서움은 두 발 구르기 직전의 자세에만 있다. ③ 같은 몸이 뒷발 구르기를 과장하고, 쿵쿵 소리 예고(`ThumpSoundRadius`)와 그림이 맞는다. ④ 해당 없음.

### 이미지 입력 조건 ([tools/tripo.md](C:\Work\CL9\docs\asset\tools\tripo.md) "입력 이미지 조건" 원문)

단일 오브젝트, 3/4 정면, 화면 중앙, 균일한 짙은 회색 배경, 바닥면 없음, 바닥 그림자 없음, 글자 없음. 이 조건을 프롬프트 끝에 그대로 적으면 Nano Banana가 지켰다.
스타일 지시(로우폴리·평면 셰이딩)는 이미지 모델이 반만 듣는다 — 형태와 색만 맞으면 된다. 톤은 Blender 재단이 맡는다.

## 2026-10-20 외형 방향 재검토 — 괴팍한 동물·수인형 토끼

- 이번 범위: `concept_only / concept_review`, 새 시안은 사용자 미승인. 기존 Blender 후보와 제작 이력은 보존한다.
- 최신 사용자 방향: “괴물있는애들은 귀엽게하고, 괴물아닌애들은 괴팍하고 좀 무섭게”, 사람 크기와 수인형 토끼도 제안. 이전의 모든 동물을 끝까지 귀엽게 만드는 기준보다 이 최신 피드백을 우선한다.
- 토끼: 사람 키의 이족 수인형 탐색안. 긴 팔, 구부정한 몸통, 큰 뒷발, 기울어진 귀와 의심스러운 시선으로 위협을 표현한다. 숨은 귀신·탈 파열은 없다. 구체적인 키와 게임 판정은 미확정.
- [양·토끼·생쥐·거위·너구리 5종 비교 시안](../../../ArtSource/monster/concept_batch_1/eccentric_concept_v01/sheet.png): 위 평상시, 아래 행동. 내장 image_gen 생성 그림이며 실제 모델 렌더가 아니다. 부정확했던 사람 크기 표시는 최종 그림에서 제거했다.
- 다음 재개: 이 외형 방향에 대한 사용자 피드백부터 반영한다. 기존 네발 v03을 승인된 최종 컨셉으로 간주하거나 이 시안만으로 모델·리그 제작을 시작하지 않는다.


## 2026-10-20 혼합 동물 모델·언리얼 배치

- 현재 요청 범위: production (정지 3D 외형 제작·UE 맵 배치·저장 후 재열기 검수). 리그와 행동 클립은 이번 배치 완료 조건에 추가하지 않는다.
- 사용자 승인 근거: 최신 혼합 동물 그림에 “이정도로해서 배치해봐”, 이어 “실제 3D 모델을 만들어 언리얼 맵에 배치”. 컨셉 강도 승인과 제작·배치 진행 지시이며 아직 보지 않은 3D 결과의 승인은 아니다.
- 컨셉: ArtSource/monster/concept_batch_1/hybrid_placement_v01/approved_concept.png. 작업 버전: model_hybrid_v01. 기존 후보 보존.
- 현재 단계: blender_work. 단독 입력 이미지를 분리하고 로컬 TRELLIS 생성 후 정지 메시를 검수한다.
