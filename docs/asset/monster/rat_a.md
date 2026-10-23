# rat_a 제작 카드 — 쥐 (떼 하나를 몬스터 하나로, 작은 설치류 규격의 첫 종)

## v02 완료 — 재질·표면·모션·실제 게임 검수

- 현재 7개 모션과 원본·FBX·정상1배속 영상: [현재 manifest](../../../ArtSource/monster/rat_a/animation_hybrid_v02/manifest.json).
- [5종52클립 납품·검증](../../../ArtSource/monster/concept_batch_1/hybrid_finish_v02/delivery.json) · [검토 화면](../../../ArtSource/monster/concept_batch_1/hybrid_finish_v02/index.html).
- 모든 필수 증거의 실제 파일·해시·통과 값을 확인했다. 사용자 외형 승인을 별도로 주장하지 않는다. 아래 v01 기록과 한계는 이전 이력이다.


## 이전 v01 체크포인트 — 혼합 동물 리그·모션과 엔진 프리뷰

- 종별 리그와 7개 동작을 제작했다: Run, HoleEmerge, HoleEnter, SplitLeft, SplitRight, Peek, Scatter.
- [5종 모션 검토 화면](../../../ArtSource/monster/concept_batch_1/hybrid_animation_v01/index.html) · [편집 원본](../../../ArtSource/monster/rat_a/animation_hybrid_v01/rat_a_Hybrid_Animated.blend) · [동작별 출력·1배속 영상](../../../ArtSource/monster/rat_a/animation_hybrid_v01/manifest.json).
- UE 임포트·포즈 왕복, 복사 검토 맵 저장/재열기, 실제 SIE의 네이티브 애니메이션 재생을 확인했다. AI/AnimBP 상태 연결 완료를 뜻하지 않는다.
- 남은 사항: 게임 이동 속도와 재생 속도 보정, 실제 상태 연결, 원본 표면 마감 및 플레이어 접촉 검수. 별도의 사용자 외형 승인으로 기록하지 않는다.

## 이전 체크포인트 — 혼합 동물 정지 모델·UE 배치

- 이번 요청 범위 완료: 실제 정지 3D 외형·FBX·언리얼 복사 맵 배치·저장 후 재열기·실제 엔진 캡처. 사용자 3D 외형 승인은 별도이며 리그/애니메이션/AI 연결은 하지 않았다.
- 혼합 컨셉과 강도 승인 근거: “이정도로해서 배치해봐” → “실제 3D 모델을 만들어 언리얼 맵에 배치”.
- [현재 원본](../../../ArtSource/monster/rat_a/model_hybrid_v01/placement/rat_a_Hybrid.blend) · [모델 QA](../../../ArtSource/monster/rat_a/model_hybrid_v01/placement/qa.json) · [5종 검수 화면](../../../ArtSource/monster/concept_batch_1/hybrid_placement_v01/index.html).
- UE 맵: `/Game/Tests/HybridFiveV01/Lvl_HybridFiveV01`. 메시: `/Game/Monsters/HybridFiveV01/rat_a/SM_rat_a_Hybrid.SM_rat_a_Hybrid`. 초안 외형 높이 0.28 m, 27,096 triangles. 게임 판정 크기를 변경하지 않았다.
- 실제 원본 GLB와 이전 모델을 보존했다. 아래 기존 기록은 과거 버전 이력이다.
- 남은 한계: 최종 표면·노멀·그림자 마감. 검토용 양면 재질과 모델 그림자 투영 비활성 설정을 사용했다. 쥐 몸통의 거친 면, 양 얼굴 경계와 거위 목의 국부 면 정리가 남는다.

```json
{
  "concept_pipeline": {
    "schema_version": 2,
    "asset_id": "rat_a",
    "scope": "production",
    "requested_outputs": [
      "finished surface topology",
      "matching CL9 vertex-color materials",
      "Run",
      "HoleEmerge",
      "HoleEnter",
      "SplitLeft",
      "SplitRight",
      "Peek",
      "Scatter",
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
          "evidence": "ArtSource/monster/rat_a/animation_hybrid_v02/manifest.json"
        },
        "animation": {
          "status": "pass",
          "evidence": "ArtSource/monster/rat_a/animation_hybrid_v02/manifest.json"
        },
        "modelQA": {
          "status": "pass",
          "evidence": "ArtSource/monster/rat_a/animation_hybrid_v02/manifest.json"
        }
      },
      "runtime": {
        "level": "actual_host_client_gameplay",
        "gameplay_connected": true,
        "asset_paths": [
          "/Game/Monsters/HybridFiveFinishV02/rat_a/Build_d12033f491/SK_rat_a_HybridV02.SK_rat_a_HybridV02",
          "/Game/Monsters/HybridFiveFinishV02/rat_a/Build_d12033f491/SK_rat_a_HybridV02_Skeleton.SK_rat_a_HybridV02_Skeleton",
          "/Game/Monsters/HybridFiveFinishV02/rat_a/Build_d12033f491/Clips_6be4b4da13/AN_Rat_HybridV02_Run.AN_Rat_HybridV02_Run",
          "/Game/Monsters/HybridFiveFinishV02/rat_a/Build_d12033f491/Clips_57e0479018/AN_Rat_HybridV02_HoleEmerge.AN_Rat_HybridV02_HoleEmerge",
          "/Game/Monsters/HybridFiveFinishV02/rat_a/Build_d12033f491/Clips_d70f8fe701/AN_Rat_HybridV02_HoleEnter.AN_Rat_HybridV02_HoleEnter",
          "/Game/Monsters/HybridFiveFinishV02/rat_a/Build_d12033f491/Clips_5cbe6cae86/AN_Rat_HybridV02_SplitLeft.AN_Rat_HybridV02_SplitLeft",
          "/Game/Monsters/HybridFiveFinishV02/rat_a/Build_d12033f491/Clips_37001685f0/AN_Rat_HybridV02_SplitRight.AN_Rat_HybridV02_SplitRight",
          "/Game/Monsters/HybridFiveFinishV02/rat_a/Build_d12033f491/Clips_40a44e26c4/AN_Rat_HybridV02_Peek.AN_Rat_HybridV02_Peek",
          "/Game/Monsters/HybridFiveFinishV02/rat_a/Build_d12033f491/Clips_2c1cce0d5d/AN_Rat_HybridV02_Scatter.AN_Rat_HybridV02_Scatter"
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
      "species_manifest": "ArtSource/monster/rat_a/animation_hybrid_v02/manifest.json",
      "previous_delivery": "ArtSource/monster/concept_batch_1/hybrid_animation_v01/delivery.json",
      "current_manifest": "ArtSource/monster/rat_a/animation_hybrid_v02/manifest.json",
      "modelQA": {
        "status": "pass",
        "evidence": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/delivery.json"
      }
    },
    "checkpoint": {
      "path": "ArtSource/monster/rat_a/animation_hybrid_v02/rat_a_Hybrid_Finished.blend",
      "sha256": "0f2ed7ae5c4c495740fcfae4462dfac7423fe19938f88594e5862db09407263f"
    },
    "job": null,
    "last_completed": "V02 기존 혼합 외형 마감·정점색 재질·7개 모션·엔진 및 실제 host/client 검수와1배속 영상 완료. 별도 사용자 외형 승인을 뜻하지 않음.",
    "next_action": "새 사용자 요청에 따라 현재 v02 체크포인트에서 재개",
    "blocker": null,
    "previous_requested_outputs": [
      "species-specific rig",
      "Run",
      "HoleEmerge",
      "HoleEnter",
      "SplitLeft",
      "SplitRight",
      "Peek",
      "Scatter",
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
              "evidence": "ArtSource/monster/rat_a/model_hybrid_v01/placement/qa.json"
            },
            "engine": {
              "status": "native_preview_pass",
              "evidence": "ArtSource/monster/concept_batch_1/hybrid_animation_v01/native_final_qa.json"
            },
            "model_rig": {
              "status": "pass_with_recorded_surface_limitations",
              "evidence": "ArtSource/monster/rat_a/animation_hybrid_v01/full_qa.json"
            },
            "animation": {
              "status": "authored_and_video_reviewed",
              "evidence": "ArtSource/monster/rat_a/animation_hybrid_v01/manifest.json"
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
              "/Game/Monsters/HybridFiveMotionV01/rat_a/Build_633a08d0a5_Normals/SK_rat_a_Hybrid.SK_rat_a_Hybrid",
              "/Game/Monsters/HybridFiveMotionV01/rat_a/Build_633a08d0a5_Normals/Clips_afa39c90d1/AN_Rat_HybridV01_Run.AN_Rat_HybridV01_Run",
              "/Game/Monsters/HybridFiveMotionV01/rat_a/Build_633a08d0a5_Normals/Clips_e54f0770a9/AN_Rat_HybridV01_HoleEmerge.AN_Rat_HybridV01_HoleEmerge",
              "/Game/Monsters/HybridFiveMotionV01/rat_a/Build_633a08d0a5_Normals/Clips_b1592853d8/AN_Rat_HybridV01_HoleEnter.AN_Rat_HybridV01_HoleEnter",
              "/Game/Monsters/HybridFiveMotionV01/rat_a/Build_633a08d0a5_Normals/Clips_a002fc6752/AN_Rat_HybridV01_SplitLeft.AN_Rat_HybridV01_SplitLeft",
              "/Game/Monsters/HybridFiveMotionV01/rat_a/Build_633a08d0a5_Normals/Clips_87b3432766/AN_Rat_HybridV01_SplitRight.AN_Rat_HybridV01_SplitRight",
              "/Game/Monsters/HybridFiveMotionV01/rat_a/Build_633a08d0a5_Normals/Clips_38d286929a/AN_Rat_HybridV01_Peek.AN_Rat_HybridV01_Peek",
              "/Game/Monsters/HybridFiveMotionV01/rat_a/Build_633a08d0a5_Normals/Clips_b8855cfc34/AN_Rat_HybridV01_Scatter.AN_Rat_HybridV01_Scatter"
            ],
            "gameplay_connected": false
          },
          "source_blender_version": "model_hybrid_v01/placement",
          "candidate_version": "animation_hybrid_v02",
          "motion_plan": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/plan.json",
          "delivery_status": "full_finish_working",
          "species_manifest": "ArtSource/monster/rat_a/animation_hybrid_v01/manifest.json",
          "previous_delivery": "ArtSource/monster/concept_batch_1/hybrid_animation_v01/delivery.json"
        },
        "checkpoint": {
          "path": "ArtSource/monster/rat_a/animation_hybrid_v01/rat_a_Hybrid_Animated.blend",
          "sha256": "58384298c1676af2cbc30339714d49fcf9553a7189700c286017b24af9b67033"
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

상태: 새 정상형 Blender 외형 후보 검토 대기 (`model_reference_v02`, 사용자 미승인)
작성일: 2026-10-19 · 변경일: 2026-10-20

제작 1묶음([production-batch-1.md](C:\Work\CL9\docs\asset\animation\production-batch-1.md)) 1-5 단계의 대상이다. 규칙은 결정 #45로 확정된 **(가) 쥐 떼 몰이 — "벽 속 긁는 소리가 나면 구멍 앞 길에서 비켜라"**다.
2026-10-20 v01 개체 한 마리의 실제 Blender 후보와 4뷰 저장·재열기 검수는 아래 이력으로 보존한다. 이후 사용자가 v01을 반려했으며 현재 재개 대상은 v02다. 새 모델은 사용자 미승인 상태이며, v02 저장·재열기·외형 검수 결과는 아래 현재 체크포인트에 기록한다. 리그·질주·이족 광폭화·떼·게임 연결은 검증하지 않았다([model-concept.md](C:\Work\CL9\docs\asset\model-concept.md) "승인과 자율 진행").
값 표기: 문서에서 옮긴 값에는 출처를 붙였고, 이 카드가 제안하는 값에는 "추정", 문서에 없는 값에는 "미확인"을 붙였다. **떼를 화면에 그리는 방식은 기술 검토 전이다.**

<a id="concept-checkpoint"></a>

## 제작 범위·승인·재개 기록

절차·필드 의미는 [model-concept.md](C:\Work\CL9\docs\asset\model-concept.md) "상태 형식"이 정본이다.

> 아래 이력의 이전 체크포인트는 이번 혼합 동물 배치 기록으로 대체되었습니다.

<details><summary>이전 파이프라인 기록</summary>

```yaml
concept_pipeline:
  schema_version: 2
  asset_id: rat_a
  scope: production
  requested_outputs: [] # 상세 후보 목록은 아래 "클립" 표; v02 제작 완료를 뜻하지 않음
  review_policy: concept_and_model
  review_override: null
  phase: blender_review
  status: awaiting_user
  updated_at: 2026-10-20T04:56:15+00:00
  concept:
    candidate_version: null
    approved_version: null
  blender:
    candidate_version: model_reference_v02
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
  pending_review: {stage: blender, version: model_reference_v02, evidence: ArtSource/monster/rat_a/model_reference_v02/final_candidate/preview}
  checkpoint: {path: ArtSource/monster/rat_a/model_reference_v02/final_candidate/rat_a.blend, sha256: 15bee8f1b3ec49f566005673b8dd04e3f26bcdc569e0f4534762c5fd7e46ce9d}
  job: null # 실행 중 생성 작업은 root 관리; 이 카드에는 작업 ID 미기록
  last_completed: "원래 시안 기반 새 3D 생성·Blender 외형 보정·저장/재열기·실제 4뷰 직접 확인"
  next_action: "v02 실제 외형에 대한 사용자 검토 후 수정 또는 어려운 변형 시험·리그·애니메이션·UE 검수"
  blocker: null # 떼 그리는 방식이 기술 검토 전 — 개체 메시·클립 제작은 막히지 않는다
```

</details>

## 2026-10-20 v02 실제 Blender 외형 후보

- 새 실제 원본: [rat_a.blend](../../../ArtSource/monster/rat_a/model_reference_v02/final_candidate/rat_a.blend), SHA256 `15bee8f1b3ec49f566005673b8dd04e3f26bcdc569e0f4534762c5fd7e46ce9d`.
- [실제 네 방향 렌더](../../../ArtSource/monster/rat_a/model_reference_v02/final_candidate/preview/three_quarter.png) · [제작/재열기 기록](../../../ArtSource/monster/rat_a/model_reference_v02/final_candidate/manifest.json) · [5종 비교 화면](../../../ArtSource/monster/concept_batch_1/model_review_v02/index.html).
- 방식: 기존 6종 4상태 시안의 해당 정상형을 단독 2D 입력으로 정리한 뒤 로컬 TRELLIS.2에서 3D 생성, Blender에서 검사·필요한 오류 보정. `input.png`는 입력 그림이고 `preview/*.png`는 실제 저장 모델의 렌더다. 원래 v01과 생성 원본 GLB를 보존한다.
- 생성 조건: 512 / seed 42 / 12 steps / 목표 50,000 faces / 2,048 texture. 축 +X 정면/+Z 위, 단위 m. 후보 크기는 외형 비교용이며 게임 판정을 변경하지 않았다.
- 실측 메시 기록: `{"mesh_objects": 2, "vertices": 19696, "triangles": 45740, "material_slots": 2, "uv_layers": {"geometry_0": ["UVMap"], "Rat_SingleThinSpiralTail": []}, "bounds_min_m": [-0.15803106129169464, -0.1691596955060959, 0.0], "bounds_max_m": [0.17415043711662292, 0.16898661851882935, 0.2199999988079071], "bounds_size_m": [0.33218149840831757, 0.33814631402492523, 0.2199999988079071]}`.
- 정면·측면·후면·사선에서 큰 귀, 네 발, 얇은 나선 꼬리 하나를 확인했다.
- 중복 생성된 꼬리와 중앙 잔해를 제거하고 뒤몸을 닫았다. 재질을 회색·분홍·크림·검정으로 정리해 뒤몸 색 이음선을 없앴다.
- 귀 안쪽의 깊은 접힘과 뒤몸 연결 부위의 표면 변화가 남는다. 리그 변형과 게임 성능은 아직 검증하지 않았다.
- 검증 범위: 실제 파일 저장·재열기, 텍스처 포함 여부, 정면·측면·후면·사선 렌더 직접 확인. 이는 사용자 승인이나 변형·게임 성능 합격이 아니다.
- 미검증: 변형용 토폴로지/리그, 행동·이족 광폭화·전환, FBX/UE, LOD와 게임 성능. 기존 소·닭 탈·돼지는 수정하지 않았고, 어미닭·횃대닭은 참고만 유지한다.



### 버전 파일 목록

경로는 프로젝트 상대 경로로 기록한다. 아래 v01은 사용자 반려 이력이며 현재 승인 대기본이 아니다.

| 단계 | 버전 | 기반 컨셉 버전 | manifest 경로 | manifest SHA256 |
|---|---|---|---|---|
| 반려된 Blender 정상형 개체 후보(이력) | model_round_v01 | sheet_six_species_four_states_v5 | ArtSource/monster/rat_a/model_round_v01/manifest.json | ecb1857bf81248e4b50775b338114692db03436c774630f6c95ff461349ba5df |

이력 체크포인트: `ArtSource/monster/rat_a/model_round_v01/rat_a.blend`, SHA256 `8bf0afe394247b252a313bf5b8ce834bbdf6e93efbfd3b67ec2e2f1fb96a1e26`. v02의 현재 체크포인트로 사용하지 않는다.

### 이력 — 2026-10-20 반려된 정상형 개체 외형 후보 v01

- 당시 범위는 **production**, 저장 단계는 `blender_review`였으나 이후 사용자 반려됐다. 아래는 비교 이력이며 v02 제작·검수 완료가 아니다. 기준은 `ArtSource/monster/concept_batch_1/sheet_six_species_four_states_v5.png`와 당시 사용자 생쥐 원문이다. 아래의 2026-10-19 치수·10뼈 규격·떼 예산은 초안이며 이 실측이나 승인으로 대체해서 읽지 않는다.
- 실제 원본: `ArtSource/monster/rat_a/model_round_v01/rat_a.blend`. 재현 스크립트 `build_model.py`, `manifest.json`, `qa.json`, `preview/{three_quarter,front,side,back}.png`가 같은 폴더에 있다.
- 방식: Sol 에이전트의 Blender 직접 파라메트릭 메시 저작. 외부 설치류 메시·리그는 포함하지 않았다. 회색 공몸·오목한 분홍 원귀·얕은 검은 눈·작은 분홍 코/발·가는 나선 꼬리를 만들었다. 봉제선·털가닥은 없다.
- 실측: **10,048 tris**, 외곽 X/Y/Z **0.367712 / 0.236 / 0.218939m**(긴 나선 꼬리·귀 포함), +X 정면/+Z 위, meter. 모든 메시 변환 identity, 각 부품 nonmanifold edge 0, 외부 이미지·라이브러리 의존성 0을 확인했다.
- 실제 저장 파일을 Blender **5.2.2 LTS**, `--background --factory-startup --disable-autoexec`로 다시 열어 4뷰를 렌더했다. Cycles **CPU 2 threads, 24 samples, 640×640**, 중립광·균일 짙은 회색 배경·바닥 없음. 초기 발-몸 분리·먼 눈 돌출·길쭉한 귀 비율을 수정하고 4뷰를 직접 확인했다. 귀는 정면에서 원형이며 꼬리 나선은 측면/사선에서 읽힌다. 에이전트 자체 검수는 사용자 외형 승인이 아니다.
- **떼용 최적화 전:** 이 외형 후보는 기존 300–800tris 초안 예산을 초과한다. 최종 단순화·LOD·인스턴싱/정점 애니메이션 등 떼 소비 방식을 승인 후 검증해야 한다. 이 후보를 그대로 수십 마리 배치해 성능이 검증됐다고 쓰지 않는다.
- 미검증: 겹치는 부품을 가진 외형 후보이며 최종 변형 토폴로지·리그·웨이트·표정·질주·이족 상태/전환·떼·FBX·UE·게임 연결은 아직 없다. 개체 한 마리만 만들었고 떼 렌더는 만들지 않았다.
- 당시 후속 계획(미실행·v01 반려로 중단): 외형 검토 뒤 질주 접지·이족 전환을 짧게 시험하는 것이었다. 현재 재개 대상은 상단의 v02 외형 재제작이며 새 3D 승인은 없다.

### 사용자 승인·수정 이력

실제 발언과 그 대상 버전만 적는다. v01은 제시 후 반려됐으며 새 모델 승인 기록은 없다.

| 시각(시간대 포함) | 단계·대상 버전 | 결정(OK/NO/수정/철회) | 사용자 발언과 근거 | manifest SHA256 | 다음 행동 |
|---|---|---|---|---|---|
| 2026-10-20(정확한 시각 미기록) | Blender / model_round_v01 | NO | “저거 다 이상해 다시만들어” — 5종 v01 모두 반려 | ecb1857bf81248e4b50775b338114692db03436c774630f6c95ff461349ba5df | model_reference_v02 외형 재제작 |

### 사용자 승인 자리 — 셋

제작 1묶음 2절 1-5의 승인 자리다. 아래 표는 종전 기획 초안의 승인 절차이며 현재 재제작 상태는 상단 체크포인트를 따른다. v01은 제시 후 반려됐고 v02 새 모델 승인은 없다.

| # | 자리 | 무엇을 보고 승인하나 | 지금 상태 |
|---|---|---|---|
| 1 | 컨셉 | 개체 한 마리(평상시) · 떼가 쏟아지는 순간 이미지 한 쌍(아래 프롬프트 절)과 톤 기준 넷(결정 #50) | 대기 — 이미지 없음 |
| 2 | 짧은 동작 | 구멍에서 쏟아져 질주로 이어지고 길에 선 플레이어가 넘어지는 2~3초(아래 "짧은 동작 시험") | 대기 — 떼 방식의 기술 검토 뒤 |
| 3 | 게임 연결 | `MonsterTable` 행 `rat` 연결 뒤 `/cl9film` 한 편(예고 10초 → 쏟아짐 → 질주 → 넘어짐 → 갈라짐) | 대기 — 표 행·떼 이동은 gameplay 몫 |

## 전제 — 떼 하나 = 몬스터 하나

- **게임에서는 하나다.** `MonsterTable` 새 행 `rat` 하나가 떼 전체다. 떼는 처치되지 않고, 위치·상태(구멍 속 → 나오기 예고 → 쏟아짐 → 질주 → 갈라짐/흩어짐 → 구멍 속)를 몬스터 하나로 복제한다. 각 머신이 그 값으로 떼를 그린다(떠돌이 5절 칸 1·13 — 새 복제 없음은 추정).
- **화면에서는 여럿이다.** 화면에는 **개체 메시**(쥐 한 마리, 스켈레탈) 여러 마리와 **떼 이펙트**(흐름의 모양·구멍 분출·갈라짐)가 함께 그려진다. 개체 수는 미확인이다.
- **아트가 만드는 것:** 개체 메시 한 벌, 작은 설치류 규격 리그, 클립 4, 떼 이펙트의 모양 기준. 떼 방식이 무엇으로 정해지든 클립의 원본은 Blender의 스켈레탈 Action으로 만든다(정점 애니메이션으로 굽는 길을 고르더라도 같은 Action에서 굽는다 — 추정). 그래서 개체 메시와 클립 제작은 기술 검토를 기다리지 않고 시작할 수 있다.

## 용도와 모습

- **분류 / Alias / SubAlias:** monster / `rat` / 미확인.
- **사용할 장소·플레이 상황:** 실내 몬스터 층이고, 문 밖으로 따라나가지 않는다(결정 #35·#37). 판 중간 등장의 공통 길(결정 #43)을 쓴다 — 진짜 구멍 하나 + 가짜 구멍 둘에서 같은 벽 속 긁는 소리가 10초 동안 커진 뒤(`EmergeTellTime`), 진짜 구멍에서 떼가 0.5초 동안 쏟아져 같은 구역의 가장 가까운 다른 구멍까지 600cm/s로 바닥을 달린다. 길(폭 100cm, 초안)에 선 사람은 피해 없이 넘어진다(0~2개가 발밑에 떨어진다). 떼의 머리에 빛을 0.3초 비추면 그 사람 둘레 150cm를 비켜 간다. 딱총을 쏘면 0.6초 동안 흩어져 가장 가까운 구멍으로 돌아간다(떠돌이 5절 칸 4·8·9).
- **기획 및 시스템 문서:** [monster-roamers.md](C:\Work\CL9\docs\plan\draft\monster-roamers.md) 5절(추천안 가 — 결정 #45로 확정), [art-plan.md](C:\Work\CL9\docs\plan\draft\art-plan.md) 4.12·2.2·2.4(쥐 떼 이펙트)·5.2 G6, [story-draft.md](C:\Work\CL9\docs\plan\draft\story-draft.md) 1.2 12행(사연 없음)·6.1 문장 5, [decisions.md](C:\Work\CL9\docs\decisions.md) #41·#43·#45·#50·#52.
- **기존 메시·리그를 재사용하는지:** 하지 않는다. 새 모델·새 스켈레톤이다.
- **형태·재질·색:** 작고 둥근 회색 쥐에 귀와 꼬리만 과장해, 떼가 물결처럼 읽히게 한다. 겉은 생쥐 인형 무리다(art-plan 4.12). 색 수를 제한하고 넓은 색면으로 칠하며 채도를 높인다([design-concept.md](C:\Work\CL9\docs\asset\design-concept.md) 규칙 2). 회색 몸에 귀 안쪽·꼬리·코를 채도 높은 분홍 한 가지로 칠하는 것을 기본으로 한다(추정). **인형처럼 보이게 하되 바느질·실밥·단추 눈은 넣지 않는다** — 쥐는 탈 없는 종이라 이음매가 없어야 한다(story-draft 6.2 ③, 결정 #52).
- **기준 이미지:** 없음. 고른 후보의 경로·SHA256을 여기에 적는다.
- **생성 도구:** 이미지는 ChatGPT 또는 Tripo 이미지 생성 — [tools/tripo.md](C:\Work\CL9\docs\asset\tools\tripo.md). 3D 입력으로 쓸 수 있는 것은 **개체 한 마리 이미지뿐**이다. 떼 이미지는 여러 오브젝트라 3D 입력 조건을 지킬 수 없으므로 컨셉 검토 전용이다.
- **리썰/R.E.P.O.에서 참고한 특징:** "어둠은 사물 일부를 숨기고, 손전등은 발견할 정보를 제한적으로 드러낸다"([reference-direction.md](C:\Work\CL9\docs\asset\reference-direction.md)) — 헤드램프에 떼가 두 갈래로 갈라지는 순간이 이 종의 극적 탈출 장면이다. "결과는 웃기게"는 쓸려 넘어져 엉덩방아를 찧는 장면에 적용한다.
- **이번 제작 범위 / 연결이 필요한 기능:** `production`이다. 개체 모델·리그·클립 4·떼 이펙트·FBX·UE 왕복·표 행·촬영까지 간다. 떼 액터(구멍 사이 경로 이동·폭 100 판정·갈라짐), 판 중간 등장의 공통 길, 표 행 `rat`은 gameplay 몫의 **연결 필요**다. 구멍 사이 바닥의 긁힌 줄(단서)은 맵 몫이다.

## 동물·탈 구조와 짧은 시험

- **탈:** 해당 없음. 탈 없는 동물이고 실밥·이음매가 없다.
- **유지할 외형·비율과 종별 차별점:** 큰 귀(머리만 한 둥근 귀)와 긴 꼬리. 같은 벽 구멍을 쓰는 닭(133cm)·두꺼비와 크기부터 다르고, 떼일 때 귀가 줄지어 물결처럼 읽히는 것이 핵심이다.
- **본체 관절/이동 유형:** 네발 질주(작은 설치류의 등을 굽혔다 펴는 뛰기). 개체마다 뼈를 적게 둔다.
- **가장 어려운 2~3초:** 쏟아짐 0.5초 + 질주 + 길에 선 플레이어가 쓸려 넘어지는 접촉(아래 "짧은 동작 시험"). 떼 방식이 정해진 뒤에 한다.
- **짧은 엔진 왕복:** 개체 메시 한 마리와 Run 한 클립을 먼저 왕복한다. 떼 그리기의 시험은 기술 검토가 정한 방식으로 한다.
- **시험 결론·남은 문제:** 아직 없다.

## 치수와 기술 계약

| 항목 | 값 및 상태 |
|---|---|
| 원본 크기 X×Y×Z (cm) | **미확인(첫 블록아웃에서 정한다).** 개체 몸길이 약 20cm(art-plan 4.12 추정, 꼬리 제외로 읽었다 — 꼬리 포함 여부는 문서에 없다), 꼬리 길이 몸길이의 1.5~2배, 귀를 포함한 키 약 10~12cm, 폭 약 8cm(모두 추정) |
| 떼 크기 | 떼 폭 100cm(떠돌이 5절 칸 9 초안 — 복도 폭과 맞춰 다시 본다), 떼 길이·밀도·마리 수는 미확인 |
| 런타임 크기·자동 배율 | 몬스터에는 자동 배율이 없다. 실제 cm로 만들고 개체 Scale은 1이다. 떼 안에서 개체 크기를 조금씩 흔들지(예: 0.9~1.1배)는 떼 방식에 달려 있다(미확인) |
| 판정 반경/캡슐·출처 | 개체에는 판정이 없다. 판정은 떼 액터의 폭 100cm(넘어뜨림), 갈라짐 둘레 150cm, 빛 반응 600cm·0.3초(`LightReactRange`·`LightReactTime`)이고 모두 떠돌이 5절 칸 9의 제안값이다. 떼 `Radius` 열이 필요한지는 미확인 |
| 피벗·정면·기준 포즈 | 개체 메시는 **네 발 사이 바닥 Z=0**, UE +X 정면, `MeshYawOffset` 0 목표. 기준 포즈는 네 발로 선 자세(추정). **떼 액터의 기준점은 미확인(기술 검토)** — 떼 머리·떼 가운데·구멍 자리 가운데 하나다 |
| 구멍 연결 | 벽 구멍 `WallHole`(쥐구멍 아치 — 닭 출입 구멍 `CoopHole`과 모양을 가른다), **높이 0~40cm**(구조 8.2·맵 3.3). 개체 키 약 12cm(추정)라 Hole Emerge에서 몸을 세워도 40cm 안에 든다. 구멍 폭은 미확인(맵 몫 — 맵 3.3은 "손바닥만 한 아치"). 좁은 구멍에서 폭 100cm 떼가 나오므로, 떼 이펙트는 구멍에서 좁게 분출한 뒤 바닥에서 부채꼴로 퍼져야 한다(추정) |
| 충돌 소유자·메시 충돌 | 충돌·판정은 떼 액터가 소유한다. 개체 메시에는 충돌이 없다 |
| 원본·FBX·UE 목적 경로 | 원본 `ArtSource/monster/rat_a/rat_a.blend`, FBX `ArtSource/monster/rat_a/export/`(`SK_Rat_A.fbx`, `AN_Rat_<클립>.fbx`, `manifest.json`), UE `/Game/Monsters/RatA/`(`SK_Rat_A`, `SKEL_Rat`, `AN_Rat_*`). 떼 이펙트 에셋의 이름·종류(Niagara 시스템 등)는 떼 방식에 달려 있다(미확인) — 초안 |
| 삼각형·재질 슬롯·텍스처·LOD | 개체 하나 300~800 tris(추정). common.md의 "동물 한 마리 5,000~15,000"은 여러 마리가 함께 보이는 떼에 맞지 않아 따로 둔다. 한 화면 최대 마리 수와 합계 예산은 기술 검토에서 실측으로 정한다(미확인). Vertex Color 1슬롯, 텍스처 없음. LOD는 귀·꼬리 실루엣을 지키며 2단계 이하(추정) |
| Skeleton·뼈 수·최대 영향 뼈 수 | 새 `SKEL_Rat`, 변형 뼈 **10개 제안**(아래 표). 정점당 영향 뼈는 최대 2(추정 — 떼로 많이 그리므로 기존 돼지 목표 2와 같게 낮춘다) |
| 부착 기준·소켓/마커 소비처 | 개체에는 부착이 없다. 떼 질주 소리(`RatSwarmRun`)의 원점을 떼 머리에 둘지 떼 가운데에 둘지는 떼 액터 쪽에서 정한다(미확인) |

### 작은 설치류 규격 — 뼈 이름·계층 제안

쥐가 처음 쓰고 두더지·다람쥐가 이어 쓰는 규격(art-plan 5.2 G6)이다. 네발 규격([rabbit_a.md](C:\Work\CL9\docs\asset\monster\rabbit_a.md))과 같은 이름 방식(기존 네발 방식 — `front_l`처럼 좌우 표시가 부위 바로 뒤)을 쓰되, 떼로 많이 그려지므로 **다리를 한 뼈씩**으로 줄인다. 두더지는 큰 앞발, 다람쥐는 큰 꼬리 체인을 더한다.

| # | 뼈 | 부모 | 하는 일 | 쥐 | 두더지·다람쥐가 이어 쓸 때 |
|---|---|---|---|---|---|
| 1 | `root` | — | 바닥 원점, 이동 클립에서 고정 | 그대로 | 그대로 |
| 2 | `body` | root | 몸 전체(엉덩이~가슴). 질주의 등 굽힘은 이 뼈와 head 사이에서 낸다 | 하나로 둔다 | 다람쥐는 `chest`를 더할 수 있다(뼈 수는 종 카드 몫) |
| 3 | `head` | body | 머리. 귀는 머리에 붙은 단단한 부분으로 두고 뼈를 주지 않는다 | 큰 귀가 머리와 같이 움직인다 | 다람쥐는 `jaw`(세는 입), 두더지는 코 뼈를 더할 수 있다 |
| 4~6 | `tail_01` · `tail_02` · `tail_03` | body · 앞 뼈 · 앞 뼈 | 꼬리 세 마디. 떼 위로 꼬리가 휘날리는 물결 | 과장한 긴 꼬리 | 다람쥐는 큰 꼬리 체인으로 늘린다(`tail_NN`) |
| 7 | `front_l` | body | 왼 앞다리 하나 | 다리 한 뼈 | 두더지는 `front_l_upper`·`_lower`·`_paw`로 쪼갠다(네발 규격의 이름) |
| 8 | `front_r` | body | 오른 앞다리 | | |
| 9 | `hind_l` | body | 왼 뒷다리 하나 | | 다람쥐 뒷다리는 네발 규격 이름으로 쪼갤 수 있다 |
| 10 | `hind_r` | body | 오른 뒷다리 | | |

- 귀를 따로 움직이고 싶으면 `ear_l`·`ear_r`(head 자식) 두 개를 더해 12뼈로 늘리는 안이 있다(추정). 떼에서 한 마리의 귀가 따로 움직이는 것이 멀리서 보이는지 의문이라, 기본은 뼈 없음이다.
- 정점 애니메이션으로 굽는 방식이 선택되면 뼈 수는 실행 비용에 들지 않지만, 굽기 전 원본 리그로는 그대로 쓴다(추정).

## 클립 / 상태별 변형

공통 규칙은 30fps, 제자리 이동(in-place), Root Motion 없음, 루프 첫·끝 포즈 일치다([common.md](C:\Work\CL9\docs\asset\common.md)). 길이의 출처는 [art-plan.md](C:\Work\CL9\docs\plan\draft\art-plan.md) 2.2 쥐 줄과 떠돌이 5절 칸 8·12다. 떼 안의 개체가 같은 클립을 같은 박자로 틀면 기계적으로 보이므로, 시작 위상을 개체마다 흩는 것을 전제로 루프를 만든다(추정 — 흩는 방법은 떼 방식 몫).

| 이름 | 길이(초) | fps | 루프 | 루트 이동 | 트리거(상태) | 플레이어 짝 |
|---|---|---|---|---|---|---|
| `AN_Rat_Run` | 미확인 — 한 주기 약 0.3초(추정) | 30 | 예 | 없음, 기준 속도 600cm/s(`ChaseSpeed` 질주 제안값) | 질주(진짜 구멍 → 가장 가까운 다른 구멍) | 길에 선 사람은 **Knockdown**(있음 — 떠돌이 5절 칸 12) |
| `AN_Rat_HoleEmerge` | 0.5 | 30 | 아니오 | 없음 — 구멍 밖으로 나오는 이동을 클립에 넣을지 떼 이펙트가 맡을지는 미확인 | 쏟아짐(예고 끝). 구멍에서 튀어나와 바닥에 내려앉고 Run 첫 프레임으로 끝난다(추정) | 없음 |
| `AN_Rat_HoleEnter` | 0.4 | 30 | 아니오 | 없음 | 질주 끝(도착 구멍)과 흩어짐 끝. 몸을 낮춰 구멍으로 뛰어든다 | 없음 |
| `AN_Rat_Split` | 미확인(짧은 원샷 — 0.3~0.5초 추정) | 30 | 아니오 | 없음 | 갈라짐(떼의 머리에 빛 0.3초). 옆으로 꺾어 빛을 비켜 가는 동작. 왼쪽·오른쪽을 미러 한 벌로 쓸지 둘로 만들지는 미확인 | 빛을 든 사람은 넘어지지 않는다 |
| 떼 이펙트 | — | — | — | — | 예고 끝 0.5초의 "쥐 머리들이 비친다"(떠돌이 칸 2), 구멍 분출, 폭 100cm 흐름, 갈라짐(둘레 150cm의 빈 원), 흩어짐 0.6초(딱총) | — |

**클립 표에 걸린 문제.**

- **흩어짐 0.6초(딱총).** 상태는 있지만 클립 목록에는 없다. Split 또는 Run에 방향 전환을 섞어 대신할지, 새 클립이 필요한지 미확인이다.
- **예고 끝의 머리 비침 0.5초.** Hole Emerge의 앞부분으로 덮을지, 구멍 안쪽에서 쓰는 짧은 클립(예: Peek)이 따로 필요한지 미확인이다.
- **Death·Idle.** 떼는 처치되지 않고(칸 9 `HpMin`/`HpMax` 불사) 구멍 속에서는 몸이 없으므로 둘 다 필요 없다고 읽었다(추정). 결정은 아니다.

## 떼 그리는 방식 — 기술 검토 전

**고르지 않았다.** 후보를 적어 두고, 고르는 것은 성능 측정이 들어간 기술 검토의 몫으로 남긴다. 측정 없이 아트가 고르면 추측이다.

| 후보 | 모양 | 아트 쪽에서 보는 점 |
|---|---|---|
| ① 개체마다 스켈레탈 메시 | 쥐 여러 마리를 각각 스켈레탈 메시로 그리고 애니메이션 업데이트를 줄인다 | 클립을 그대로 쓴다. 마리 수가 많으면 비용이 크다는 것이 일반적인 우려다(이 머신에서 실측 없음) |
| ② 정점 애니메이션으로 굽기 | 스켈레탈 클립을 정점 애니메이션 텍스처로 구워 정적 메시를 인스턴스나 Niagara 메시 파티클로 그린다 | Blender Action에서 굽는 단계가 하나 더 든다. 굽는 도구와 UE 머티리얼이 CL9에서 검증되지 않았다 |
| ③ 섞기 | 플레이어 가까이의 앞줄 몇 마리만 ①, 나머지는 ② | 두 경로를 다 만든다. 둘 사이 외형이 같아 보여야 한다 |

어느 후보든 개체 메시·뼈·클립의 원본은 같다. 그래서 1-5 단계의 컨셉 → 개체 모델 → 리그 → 클립은 기술 검토와 나란히 진행할 수 있다(추정).

## 짧은 동작 시험 — 쏟아짐 + 질주 + 넘어짐 (2~3초)

떼 방식이 정해진 뒤에 한다.

| 구간(초, 추정) | 떼 | 플레이어 |
|---|---|---|
| 0.0~0.5 | 예고 끝 — 구멍 안쪽에서 쥐 머리들이 비친다 | 길 위에서 줍고 있다 |
| 0.5~1.0 | 쏟아짐(Hole Emerge 0.5초) — 좁은 구멍에서 분출해 폭 100cm로 퍼진다 | 그대로 |
| 1.0~2.0 | 질주 600cm/s — 앞머리가 플레이어의 다리를 쓸고 지나간다 | Knockdown(엉덩방아) |
| 2.0~2.5 | 떼 꼬리가 지나가고 앞머리가 다음 구멍으로 들어간다(Hole Enter) | 넘어진 채 |

- **볼 것:** 구멍(높이 0~40cm, 폭 미확인)에서 나오는 동안 개체가 벽을 뚫지 않는지, 떼가 한 덩어리 물결로 읽히는지(귀가 줄지어 보이는지), 같은 박자로 움직이는 개체가 눈에 띄지 않는지, 쓸려 넘어지는 장면이 웃기게 읽히는지(통합 4 2절 25번).
- **엔진 왕복:** 동물 클립은 **Interchange로만** 넣는다(monster/README 2026-09-21).

## 내보내기와 검수 기록

- Blender 버전 / exporter 이름·버전 / 축·단위·베이크 옵션: 2026-10-20 Blender 5.2.2 LTS, +X 정면/+Z 위, meter. 이번 외형 후보는 exporter/베이크 미실행.
- UE 버전 / importer / Skeleton 지정 / 임포트 옵션:
- 중립광·어두운 조명·와이어·클립 미리보기 경로:
- 몸–탈 / 탈 자체 / 손 집기 / 발 접지 / 외형 / 전환 QA를 구분한 결과: (몸–탈·탈 자체·손 집기는 해당 없음)
- 1배속 및 비교 영상 묶음, 우측 상단 제작 방식·버전·속도·렌더/인게임 표시:
- 엔진 임포트·프리뷰·실제 게임 상태 연결의 구분과 실행/로그/재열기 증거:
- 동작 영상의 실제 재생/디코딩·화면 검수 결과, 사용자에게 제시한 영상·시각·피드백:
- 크기·원점·전방·충돌·리그·루프 확인 결과:
- 게임 시나리오 / 2인 PIE / 성능 측정 결과(떼 마리 수별 프레임 비용 포함):
- 미실행 항목과 이유: 반려된 v01은 재열기/4뷰/메시·의존성 검사까지만 수행했다. v02의 실제 모델·재열기·4뷰 완료는 아직 기록하지 않았으며 리그·모션·떼·FBX·UE·게임 검수도 미실행이다.
- 알려진 문제·다음 수정: 10,048tris는 떼용 초안 예산 초과다. 외형 검토 후 단순화·리그·질주/이족 전환·떼 소비 방식 검증이 필요하다.
- 재개 정보: 현재 대상은 상단 `model_reference_v02` 제작 계획과 체크포인트다. v01 원본·명령·manifest/QA는 반려 이력으로만 보존하며 v02 완료 증거로 승계하지 않는다. 생성 실행 상태는 root가 관리한다.

## 미확인·연결 필요 목록 (17건)

1. 몸길이 20cm가 꼬리를 포함하는지.
2. SubAlias.
3. 떼의 길이·밀도·한 화면 최대 마리 수와 합계 삼각형 예산.
4. 떼 액터의 기준점.
5. 떼 `Radius` 열이 필요한지.
6. 개체 크기 흔들기를 할지.
7. 구멍 폭(맵 몫).
8. 떼 이펙트 에셋의 종류·이름.
9. 질주 소리 원점(떼 머리 / 가운데).
10. Run 한 주기 길이.
11. Hole Emerge의 구멍 밖 이동을 클립이 맡을지 이펙트가 맡을지.
12. Split 길이와 좌우를 미러로 쓸지.
13. 흩어짐 0.6초에 새 클립이 드는지.
14. 예고 끝 머리 비침에 별도 클립이 드는지.
15. 떼 그리는 방식(기술 검토).
16. 개체 시작 위상을 흩는 방법(떼 방식 몫).
17. 새 복제가 없다는 읽기(떠돌이 칸 13 "없음(추정)")가 맞는지.

## 컨셉 이미지 프롬프트

후보 셋 × (평상시 · 떼가 쏟아지는 순간) 한 쌍씩이다. 프롬프트는 이미지 모델용 영어이고, 아래에 한국어 요지 한 줄을 붙였다. 이미지는 아직 만들지 않았다.

**쥐는 탈이 없는 종이라 "정체"가 없다.** 톤 문장 5를 따라, 무서운 쪽 이미지는 같은 개체가 **여럿이 한꺼번에 쏟아지는 장면**으로 만든다 — 무서움은 모습이 아니라 수와 기세에서 나온다. 한 마리의 생김새는 평상시와 같아야 하고, 이빨·핏발·상처를 더하지 않는다. 떼 이미지는 여러 오브젝트라 **3D 입력 조건을 지킬 수 없으므로 컨셉 검토 전용**이다. 3D 입력에는 평상시 한 마리 이미지만 쓴다.

톤 기준 넷(결정 #50)을 이 종에 적용하는 법은 다음과 같다. ① 평시 귀여움 — 한 마리가 생쥐 인형처럼 귀엽다. ② 무서움은 그 순간에만 — 떼가 쏟아지는 장면에만 기세가 있다. ③ 같은 몸 — 떼 속 개체가 평상시 한 마리와 똑같이 생겼다(뒤틀림 대신 "수"). ④ 이음매 한 곳 — 해당 없다(탈 없는 종). 인형처럼 그리되 바느질 자국이 생기지 않게 프롬프트에 `no stitches, no seams`를 넣는다.

**공통 꼬리말 — 평상시(3D 입력용)** — `+ 꼬리말 A` 자리에 붙인다:

```text
stylized low-poly game creature, chunky rounded shapes, flat-shaded large color areas, limited palette of three colors, one saturated accent color, cute toy-like look with no stitches and no seams, no realistic fur strands, no blood, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

**공통 꼬리말 — 떼(컨셉 검토 전용)** — `+ 꼬리말 B` 자리에 붙인다:

```text
stylized low-poly game creatures, chunky rounded shapes, flat-shaded large color areas, limited palette, every mouse identical to the others, no stitches, no seams, no blood, no gore, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

### 후보 A — 펠트 공

평상시:

```text
A tiny toy-like mouse about 20 cm long, a round soft gray body shaped like a ball, enormous round ears as big as its head with bright hot-pink insides, a long thin hot-pink tail twice its body length curling into a little spiral at the tip, tiny hot-pink feet and nose, big shiny black eyes, standing on all fours with a cheerful curious look, + 꼬리말 A
```

요지: 공처럼 둥근 회색 몸에 머리만 한 둥근 귀(안쪽 진분홍)와 끝이 말린 긴 분홍 꼬리를 한 20cm 생쥐 인형이 호기심 어린 얼굴로 서 있다.

떼가 쏟아지는 순간:

```text
A dense rushing stream of dozens of the same tiny round gray mice with huge pink-lined ears and long pink tails, pouring forward in one single wave toward the viewer, packed shoulder to shoulder, the rows of big round ears forming a rippling surface like water, pink tails whipping above the flow, all the eyes catching the light as tiny white highlights, overwhelming momentum, + 꼬리말 B
```

요지: 같은 둥근 회색 생쥐 수십 마리가 한 물결로 쏟아지고, 줄지은 둥근 귀가 물결 면처럼 일렁이며 분홍 꼬리가 그 위로 휘날린다.

톤 기준: ① 공 같은 몸과 큰 귀가 귀엽다. ② 무서움은 수십 마리가 한꺼번에 몰려오는 장면에만 있다. ③ 떼 속 한 마리가 평상시와 같다. ④ 해당 없음.

### 후보 B — 물방울

평상시:

```text
A tiny toy-like mouse about 20 cm long, a teardrop-shaped body in slate blue-gray with a pale gray belly, two very large ears shaped like round satellite dishes with saturated magenta insides, a long thin magenta tail, a pointed little magenta nose, round black eyes, sitting up on its hind legs with its front paws held together, + 꼬리말 A
```

요지: 물방울 모양의 푸른 회색 몸에 접시처럼 큰 귀(안쪽 자홍)와 긴 자홍 꼬리를 한 생쥐가 앞발을 모으고 뒷발로 앉아 있다.

떼가 쏟아지는 순간:

```text
A tall cresting wave made of dozens of the same slate blue-gray mice with dish-shaped magenta-lined ears, surging toward the viewer, the front row rearing up with ears flared wide, the mice behind climbing over each other so the wave rises higher than a person's knee, thin magenta tails trailing behind like spray, + 꼬리말 B
```

요지: 같은 푸른 회색 생쥐 수십 마리가 서로 타고 넘어 무릎 높이의 파도처럼 솟구치고, 앞줄이 귀를 활짝 편 채 일어선다.

톤 기준: ① 앞발을 모은 앉은 자세와 접시 귀가 귀엽다. ② 무서움은 떼가 파도처럼 솟는 장면에만 있다. ③ 떼 속 한 마리가 평상시와 같다. ④ 해당 없음.

### 후보 C — 고리 꼬리

평상시:

```text
A tiny toy-like mouse about 20 cm long, a plump warm gray body with a white belly, large round ears with white rims and saturated pink insides, a long tail with three bold white rings that is easy to read from far away, a pink nose, round black eyes, mid-step with one front paw lifted, + 꼬리말 A
```

요지: 따뜻한 회색에 흰 배, 흰 테두리 큰 귀, 흰 고리 셋이 또렷한 긴 꼬리를 한 통통한 생쥐가 앞발 하나를 든 채 걷는다.

떼가 쏟아지는 순간:

```text
Dozens of the same plump warm gray mice with white-ringed tails bursting out of a small dark round arch-shaped hole outline and fanning out across the frame, a narrow jet of mice at the hole spreading into a wide river, the white tail rings flickering everywhere and making the whole flow read as fast motion, + 꼬리말 B
```

요지: 같은 생쥐 수십 마리가 작은 둥근 아치 구멍에서 가늘게 분출해 넓은 강처럼 부채꼴로 퍼지고, 흰 꼬리 고리가 사방에서 깜빡여 빠르기가 읽힌다.

톤 기준: ① 통통한 몸과 고리 꼬리가 귀엽다. ② 무서움은 구멍에서 쏟아지는 장면에만 있다. ③ 떼 속 한 마리가 평상시와 같고, 구멍에서 좁게 나와 퍼지는 모양이 실제 구멍(0~40cm)과 떼 폭(100cm)의 관계를 미리 보여 준다. ④ 해당 없음.

### 이미지 입력 조건 ([tools/tripo.md](C:\Work\CL9\docs\asset\tools\tripo.md) "입력 이미지 조건" 원문)

단일 오브젝트, 3/4 정면, 화면 중앙, 균일한 짙은 회색 배경, 바닥면 없음, 바닥 그림자 없음, 글자 없음. 이 조건을 프롬프트 끝에 그대로 적으면 Nano Banana가 지켰다.
스타일 지시(로우폴리·평면 셰이딩)는 이미지 모델이 반만 듣는다 — 형태와 색만 맞으면 된다. 톤은 Blender 재단이 맡는다.


## 2026-10-20 혼합 동물 모델·언리얼 배치

- 현재 요청 범위: production (정지 3D 외형 제작·UE 맵 배치·저장 후 재열기 검수). 리그와 행동 클립은 이번 배치 완료 조건에 추가하지 않는다.
- 사용자 승인 근거: 최신 혼합 동물 그림에 “이정도로해서 배치해봐”, 이어 “실제 3D 모델을 만들어 언리얼 맵에 배치”. 컨셉 강도 승인과 제작·배치 진행 지시이며 아직 보지 않은 3D 결과의 승인은 아니다.
- 컨셉: ArtSource/monster/concept_batch_1/hybrid_placement_v01/approved_concept.png. 작업 버전: model_hybrid_v01. 기존 후보 보존.
- 현재 단계: blender_work. 단독 입력 이미지를 분리하고 로컬 TRELLIS 생성 후 정지 메시를 검수한다.
