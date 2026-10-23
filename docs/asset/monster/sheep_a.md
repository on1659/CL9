# Sheep A — 밝은 털과 짙은 얼굴

## v02 완료 — 재질·표면·모션·실제 게임 검수

- 현재 9개 모션과 원본·FBX·정상1배속 영상: [현재 manifest](../../../ArtSource/monster/sheep_a/animation_hybrid_v02/manifest.json).
- [5종52클립 납품·검증](../../../ArtSource/monster/concept_batch_1/hybrid_finish_v02/delivery.json) · [검토 화면](../../../ArtSource/monster/concept_batch_1/hybrid_finish_v02/index.html).
- 모든 필수 증거의 실제 파일·해시·통과 값을 확인했다. 사용자 외형 승인을 별도로 주장하지 않는다. 아래 v01 기록과 한계는 이전 이력이다.


## 이전 v01 체크포인트 — 혼합 동물 리그·모션과 엔진 프리뷰

- 종별 리그와 9개 동작을 제작했다: Idle, Walk, Run, LookAround, WakeTelegraph, Catch, HitReact, PushReact, Death.
- [5종 모션 검토 화면](../../../ArtSource/monster/concept_batch_1/hybrid_animation_v01/index.html) · [편집 원본](../../../ArtSource/monster/sheep_a/animation_hybrid_v01/sheep_a_Hybrid_Animated.blend) · [동작별 출력·1배속 영상](../../../ArtSource/monster/sheep_a/animation_hybrid_v01/manifest.json).
- UE 임포트·포즈 왕복, 복사 검토 맵 저장/재열기, 실제 SIE의 네이티브 애니메이션 재생을 확인했다. AI/AnimBP 상태 연결 완료를 뜻하지 않는다.
- 남은 사항: 게임 이동 속도와 재생 속도 보정, 실제 상태 연결, 원본 표면 마감 및 플레이어 접촉 검수. 별도의 사용자 외형 승인으로 기록하지 않는다.

## 이전 체크포인트 — 혼합 동물 정지 모델·UE 배치

- 이번 요청 범위 완료: 실제 정지 3D 외형·FBX·언리얼 복사 맵 배치·저장 후 재열기·실제 엔진 캡처. 사용자 3D 외형 승인은 별도이며 리그/애니메이션/AI 연결은 하지 않았다.
- 혼합 컨셉과 강도 승인 근거: “이정도로해서 배치해봐” → “실제 3D 모델을 만들어 언리얼 맵에 배치”.
- [현재 원본](../../../ArtSource/monster/sheep_a/model_hybrid_v01/placement_corrected/sheep_a_Hybrid.blend) · [모델 QA](../../../ArtSource/monster/sheep_a/model_hybrid_v01/placement_corrected/qa.json) · [5종 검수 화면](../../../ArtSource/monster/concept_batch_1/hybrid_placement_v01/index.html).
- UE 맵: `/Game/Tests/HybridFiveV01/Lvl_HybridFiveV01`. 메시: `/Game/Monsters/HybridFiveV01/sheep_a/SM_sheep_a_Hybrid.SM_sheep_a_Hybrid`. 초안 외형 높이 1.10 m, 27,831 triangles. 게임 판정 크기를 변경하지 않았다.
- 실제 원본 GLB와 이전 모델을 보존했다. 아래 기존 기록은 과거 버전 이력이다.
- 남은 한계: 최종 표면·노멀·그림자 마감. 검토용 양면 재질과 모델 그림자 투영 비활성 설정을 사용했다. 쥐 몸통의 거친 면, 양 얼굴 경계와 거위 목의 국부 면 정리가 남는다.

```json
{
  "concept_pipeline": {
    "schema_version": 2,
    "asset_id": "sheep_a",
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
          "evidence": "ArtSource/monster/sheep_a/animation_hybrid_v02/manifest.json"
        },
        "animation": {
          "status": "pass",
          "evidence": "ArtSource/monster/sheep_a/animation_hybrid_v02/manifest.json"
        },
        "modelQA": {
          "status": "pass",
          "evidence": "ArtSource/monster/sheep_a/animation_hybrid_v02/manifest.json"
        }
      },
      "runtime": {
        "level": "actual_host_client_gameplay",
        "gameplay_connected": true,
        "asset_paths": [
          "/Game/Monsters/HybridFiveFinishV02/sheep_a/Build_de735120a7/SK_sheep_a_HybridV02.SK_sheep_a_HybridV02",
          "/Game/Monsters/HybridFiveFinishV02/sheep_a/Build_de735120a7/SK_sheep_a_HybridV02_Skeleton.SK_sheep_a_HybridV02_Skeleton",
          "/Game/Monsters/HybridFiveFinishV02/sheep_a/Build_de735120a7/Clips_fa37950028/AN_sheep_a_Hybrid_Idle.AN_sheep_a_Hybrid_Idle",
          "/Game/Monsters/HybridFiveFinishV02/sheep_a/Build_de735120a7/Clips_ed4885ab7b/AN_sheep_a_Hybrid_Walk_002.AN_sheep_a_Hybrid_Walk_002",
          "/Game/Monsters/HybridFiveFinishV02/sheep_a/Build_de735120a7/Clips_358a6575c8/AN_sheep_a_Hybrid_Run_001.AN_sheep_a_Hybrid_Run_001",
          "/Game/Monsters/HybridFiveFinishV02/sheep_a/Build_de735120a7/Clips_942f6e7453/AN_sheep_a_Hybrid_LookAround.AN_sheep_a_Hybrid_LookAround",
          "/Game/Monsters/HybridFiveFinishV02/sheep_a/Build_de735120a7/Clips_12893325d7/AN_sheep_a_Hybrid_HitReact.AN_sheep_a_Hybrid_HitReact",
          "/Game/Monsters/HybridFiveFinishV02/sheep_a/Build_de735120a7/Clips_c477d4dc8d/AN_sheep_a_Hybrid_WakeTelegraph.AN_sheep_a_Hybrid_WakeTelegraph",
          "/Game/Monsters/HybridFiveFinishV02/sheep_a/Build_de735120a7/Clips_9dea2f5cdc/AN_sheep_a_Hybrid_Catch_001.AN_sheep_a_Hybrid_Catch_001",
          "/Game/Monsters/HybridFiveFinishV02/sheep_a/Build_de735120a7/Clips_f188571105/AN_sheep_a_Hybrid_PushReact.AN_sheep_a_Hybrid_PushReact",
          "/Game/Monsters/HybridFiveFinishV02/sheep_a/Build_de735120a7/Clips_12b4aa0d06/AN_sheep_a_Hybrid_Death_001.AN_sheep_a_Hybrid_Death_001"
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
      "species_manifest": "ArtSource/monster/sheep_a/animation_hybrid_v02/manifest.json",
      "previous_delivery": "ArtSource/monster/concept_batch_1/hybrid_animation_v01/delivery.json",
      "current_manifest": "ArtSource/monster/sheep_a/animation_hybrid_v02/manifest.json",
      "modelQA": {
        "status": "pass",
        "evidence": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/delivery.json"
      }
    },
    "checkpoint": {
      "path": "ArtSource/monster/sheep_a/animation_hybrid_v02/sheep_a_Hybrid_Finished_v02.blend",
      "sha256": "a194948cd5b4e8b21af52d9ea4bc7943529e7907468d21fc8537f7eddbc4042f"
    },
    "job": null,
    "last_completed": "V02 기존 혼합 외형 마감·정점색 재질·9개 모션·엔진 및 실제 host/client 검수와1배속 영상 완료. 별도 사용자 외형 승인을 뜻하지 않음.",
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
              "evidence": "ArtSource/monster/sheep_a/model_hybrid_v01/placement_corrected/qa.json"
            },
            "engine": {
              "status": "native_preview_pass",
              "evidence": "ArtSource/monster/concept_batch_1/hybrid_animation_v01/native_final_qa.json"
            },
            "model_rig": {
              "status": "pass_with_recorded_surface_limitations",
              "evidence": "ArtSource/monster/sheep_a/animation_hybrid_v01/qa.json"
            },
            "animation": {
              "status": "authored_and_video_reviewed",
              "evidence": "ArtSource/monster/sheep_a/animation_hybrid_v01/manifest.json"
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
              "/Game/Monsters/HybridFiveMotionV01/sheep_a/Build_a046a1d54e_Normals/SK_sheep_a_Hybrid.SK_sheep_a_Hybrid",
              "/Game/Monsters/HybridFiveMotionV01/sheep_a/Build_a046a1d54e_Normals/Clips_869a0cab2f/AN_sheep_a_Hybrid_Idle.AN_sheep_a_Hybrid_Idle",
              "/Game/Monsters/HybridFiveMotionV01/sheep_a/Build_a046a1d54e_Normals/Clips_b23031a940/AN_sheep_a_Hybrid_Walk.AN_sheep_a_Hybrid_Walk",
              "/Game/Monsters/HybridFiveMotionV01/sheep_a/Build_a046a1d54e_Normals/Clips_e7a1d330d3/AN_sheep_a_Hybrid_Run.AN_sheep_a_Hybrid_Run",
              "/Game/Monsters/HybridFiveMotionV01/sheep_a/Build_a046a1d54e_Normals/Clips_7b58a657b1/AN_sheep_a_Hybrid_LookAround.AN_sheep_a_Hybrid_LookAround",
              "/Game/Monsters/HybridFiveMotionV01/sheep_a/Build_a046a1d54e_Normals/Clips_0fc73dc31b/AN_sheep_a_Hybrid_HitReact.AN_sheep_a_Hybrid_HitReact",
              "/Game/Monsters/HybridFiveMotionV01/sheep_a/Build_a046a1d54e_Normals/Clips_737ab79ff1/AN_sheep_a_Hybrid_WakeTelegraph.AN_sheep_a_Hybrid_WakeTelegraph",
              "/Game/Monsters/HybridFiveMotionV01/sheep_a/Build_a046a1d54e_Normals/Clips_a7b181100f/AN_sheep_a_Hybrid_Catch.AN_sheep_a_Hybrid_Catch",
              "/Game/Monsters/HybridFiveMotionV01/sheep_a/Build_a046a1d54e_Normals/Clips_bc9d005973/AN_sheep_a_Hybrid_PushReact.AN_sheep_a_Hybrid_PushReact",
              "/Game/Monsters/HybridFiveMotionV01/sheep_a/Build_a046a1d54e_Normals/Clips_19b7f82702/AN_sheep_a_Hybrid_Death.AN_sheep_a_Hybrid_Death"
            ],
            "gameplay_connected": false
          },
          "source_blender_version": "model_hybrid_v01/placement_corrected",
          "candidate_version": "animation_hybrid_v02",
          "motion_plan": "ArtSource/monster/concept_batch_1/hybrid_finish_v02/plan.json",
          "delivery_status": "full_finish_working",
          "species_manifest": "ArtSource/monster/sheep_a/animation_hybrid_v01/manifest.json",
          "previous_delivery": "ArtSource/monster/concept_batch_1/hybrid_animation_v01/delivery.json"
        },
        "checkpoint": {
          "path": "ArtSource/monster/sheep_a/animation_hybrid_v01/sheep_a_Hybrid_Animated.blend",
          "sha256": "69d37af7102f8b7593b517d2c2da51724aeab969586d3307be85c14e298907ba"
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
  asset_id: sheep_a
  scope: production
  phase: blender_review
  status: awaiting_user
  updated_at: 2026-10-20T04:56:15+00:00
  blender:
    candidate_version: model_reference_v02
    approved_version: null
  pending_review: {stage: blender, version: model_reference_v02, evidence: ArtSource/monster/sheep_a/model_reference_v02/final_candidate/preview}
  checkpoint: {path: ArtSource/monster/sheep_a/model_reference_v02/final_candidate/sheep_a.blend, sha256: d7dc6df48cb62524477953558aacacc39043dad0645d53513bd2e92dc35b11e1}
  last_completed: "원래 시안 기반 새 3D 생성·Blender 외형 보정·저장/재열기·실제 4뷰 직접 확인"
  next_action: "v02 실제 외형에 대한 사용자 검토 후 수정 또는 어려운 변형 시험·리그·애니메이션·UE 검수"
```

</details>

## 2026-10-20 v02 실제 Blender 외형 후보

- 새 실제 원본: [sheep_a.blend](../../../ArtSource/monster/sheep_a/model_reference_v02/final_candidate/sheep_a.blend), SHA256 `d7dc6df48cb62524477953558aacacc39043dad0645d53513bd2e92dc35b11e1`.
- [실제 네 방향 렌더](../../../ArtSource/monster/sheep_a/model_reference_v02/final_candidate/preview/three_quarter.png) · [제작/재열기 기록](../../../ArtSource/monster/sheep_a/model_reference_v02/final_candidate/manifest.json) · [5종 비교 화면](../../../ArtSource/monster/concept_batch_1/model_review_v02/index.html).
- 방식: 기존 6종 4상태 시안의 해당 정상형을 단독 2D 입력으로 정리한 뒤 로컬 TRELLIS.2에서 3D 생성, Blender에서 검사·필요한 오류 보정. `input.png`는 입력 그림이고 `preview/*.png`는 실제 저장 모델의 렌더다. 원래 v01과 생성 원본 GLB를 보존한다.
- 생성 조건: 512 / seed 42 / 12 steps / 목표 50,000 faces / 2,048 texture. 축 +X 정면/+Z 위, 단위 m. 후보 크기는 외형 비교용이며 게임 판정을 변경하지 않았다.
- 실측 메시 기록: `{"mesh_objects": 8, "vertices": 43768, "triangles": 53997, "material_slots": 8, "uv_layers": {"geometry_0": ["UVMap"], "Probe_Flush_Cornea_0": [], "Probe_Flush_Cornea_1": [], "Sheep_Tiny_Catchlight_0": [], "Sheep_Tiny_Catchlight_1": [], "Sheep_Thin_Red_Neck_Collar": [], "Sheep_Red_Collar_UnderChin_Continuation": [], "Sheep_Bell_Suspension": ["UVMap"]}, "bounds_min_m": [-1.1410340070724487, -0.7685763239860535, 0.0], "bounds_max_m": [1.1410340070724487, 0.768576443195343, 1.5832868814468384], "bounds_size_m": [2.2820680141448975, 1.5371527671813965, 1.5832868814468384]}`.
- 실제 네 방향에서 몸·낮은 얼굴·처진 귀·네 발·방울의 연결을 확인했다. 머리 위 부유 잔해 6개를 제거했다.
- 눈에 얕은 곡면과 작은 흰 반사점을 넣고, 얇은 빨간 목줄·방울 연결부를 복원했다.
- 작은 턱밑 목줄 꺾임, 생성 표면의 비대칭과 간략화된 코·입이 남은 외형 후보다. 사용자 승인과 리그·게임 검수는 별도다.
- 검증 범위: 실제 파일 저장·재열기, 텍스처 포함 여부, 정면·측면·후면·사선 렌더 직접 확인. 이는 사용자 승인이나 변형·게임 성능 합격이 아니다.
- 미검증: 변형용 토폴로지/리그, 행동·이족 광폭화·전환, FBX/UE, LOD와 게임 성능. 기존 소·닭 탈·돼지는 수정하지 않았고, 어미닭·횃대닭은 참고만 유지한다.



## 이력 — 2026-10-20 반려된 정상형 3D 후보 model_round_v01

당시 전체 요청 `production`의 `blender_review`까지 제작했으나 사용자 “저거 다 이상해 다시만들어”로 **반려됐다**. 아래는 비교용 이력이며 현재 승인 대기본이나 v02 완료 체크포인트가 아니다. 종전 게임 연결 이력도 새 후보의 QA로 승계하지 않는다. [완료된 10분 사전 조사와 재사용 판단](../../../ArtSource/monster/research_20261020/README.md) 뒤 별도 버전에서 직접 모델링했다.

- 기준: 최신 [6종 4상태 시안](../../../ArtSource/monster/concept_batch_1/sheet_six_species_four_states_v5.png)과 [양 4상태 v01](../../../ArtSource/monster/sheep_a/concept_biped_v01/sheep_four_states_v01.png)을 직접 확인했다. 낮은 작은 검정 얼굴, 처진 귀, 큰 순한 검은 눈, 짧은 갈라진 발굽, 빨간 목줄·황동 종을 반영했다.
- 외형: 공 같은 몸에 주 구름 14개·중간 구름 8개를 융합한 단일 털 몸통. 별개 구슬 100개를 겹치거나 털 가닥을 만들지 않았다. 큰 면과 제한 팔레트를 유지하고 눈의 흰 테두리를 줄였다. 시각 검토에서 들려 보이던 귀를 내리고 종의 연결부를 추가했다.
- 실제 편집 파일: [sheep_a_round_v01.blend](../../../ArtSource/monster/sheep_a/model_round_v01/sheep_a_round_v01.blend). SHA256 `dc2ea16f7f00a7adc22764a60d5c4c6df0ca7d5c0d2f5da38ef8b1c49c72cc8e`. [재현 Python](../../../ArtSource/monster/sheep_a/model_round_v01/build_round.py), [manifest](../../../ArtSource/monster/sheep_a/model_round_v01/manifest.json), [QA](../../../ArtSource/monster/sheep_a/model_round_v01/qa.json).
- 실측: **11,190 tris / 5,667 vertices**, 바운드 X/Y/Z **187.85×141.28×166.56cm**, min Z=0. +X 정면·+Z 위·1BU=1m. 모델 오브젝트의 위치/회전/스케일 적용을 확인했다. 이 치수는 후보 실측이며 게임 판정 크기를 변경한 것이 아니다.
- 실제 검사: Blender 5.2.2 별도 background/factory-startup/disable-autoexec, **CPU Cycles 2스레드·20 samples·640×640**, 회색 단색 배경·바닥 없음·중립광. 저장한 `.blend`를 다시 열어 [정면](../../../ArtSource/monster/sheep_a/model_round_v01/preview/front.png)·[측면](../../../ArtSource/monster/sheep_a/model_round_v01/preview/side.png)·[후면](../../../ArtSource/monster/sheep_a/model_round_v01/preview/back.png)·[사선](../../../ArtSource/monster/sheep_a/model_round_v01/preview/three_quarter.png)을 모두 렌더하고 직접 보았다. 프레임 잘림·귀 방향·눈 테두리·구름 실루엣·종 연결부를 확인했다.
- 보존: 종전 `sheep_a.blend`, `export/`, 기존 모션과 UE 에셋은 수정하지 않았다. 새 후보는 리그/Action 0개인 외형 단계다. 사용자 외형 승인, 직립 변형·웨이트·모션, UV/아틀라스·내보내기, 자기교차/매니폴드 전체 검사, UE 왕복·게임 연결은 **미검증**이다.
- 당시 후속 계획(미실행·v01 반려로 중단): 외형 검토 뒤 양 자신의 사족→직립 전환에서 앞발이 발굽으로 유지되는지, 몸통의 구름 볼륨과 무게중심이 유지되는지 가장 어려운 2～3초를 먼저 시험하는 것이었다. 현재 재개 대상은 상단의 v02 외형 재제작이다.

## 2026-10-19 새 외형 컨셉 — 이족보행 광폭화

- 당시 요청 범위: `concept_only`. 사용자 “여기서는 다른동물 이어서” 이후 “양부터 이어서” 선택. 돼지 탈 괴물의 실제 제작은 별도 새 세션으로 넘겼다. 현재 범위는 상단의 `production`이다.
- [4상태 후보 v01](../../../ArtSource/monster/sheep_a/concept_biped_v01/sheep_four_states_v01.png): 좌상 평상시, 우상 경계, 좌하 분노, 우하 이족보행 광폭화. 흰 양털·검은 얼굴/다리·빨간 목줄과 황동 방울을 유지한다. 광폭화는 양 자신이 두 발로 일어서며 앞발은 사람 손이 아닌 갈라진 발굽이다.
- 근거: 후속 `story-draft.md`에서 양은 탈 없는 몬스터 층이다. 이번 사용자 지시 “괴물 없는 애들은 광폭화 때 이족보행”을 외형에 반영했다. 별도 내부 괴물·탈 벗기·실밥은 없다. 기존 기획의 게이지 없음과 새로운 외형 제안을 구분하며, 게임 규칙·구현을 완료했다고 표시하지 않는다.
- 내장 `image_gen` 1회 생성. [사용 프롬프트](../../../ArtSource/monster/sheep_a/concept_biped_v01/prompt.txt). 참조는 `ArtSource/monster/concept_batch_1/sheet_six_species_four_states_v5.png`의 양 열.
- 4칸·양의 색/부속 유지·광폭화의 두 뒷발과 늘어진 발굽 앞다리 확인. 사용자 외형 승인 전 후보이고 새 모델/리그/애니메이션은 시작하지 않았다. 다음 행동은 이 후보의 사용자 피드백 반영이다.
- 아래 기존 제작·검수 이력은 종전 게임 모델의 기록이며 이번 컨셉의 기술 검수로 승계하지 않는다.

상태: 게임 연결 검수됨 — 2026-09-11 `MonsterTable.sheep` → SK/`ABP_Sheep_A`, Lvl_Flat PIE 추격→잡기; 09-15 PushReact. 클립 9개(`Content/Monsters/SheepA/`), 재질 슬롯 배정 09-20. 2인 PIE·LOD 미검수 · 작성일: 2026-09-11 · 변경일: 2026-09-21

2026-09-11 · 현재 sheep 테스트 종의 구매 팩 임시 메시를 교체하는 독립 신규 모델.
큰 털 덩어리와 긴 짙은 얼굴, 가로 눈동자, 옆으로 뻗은 귀, 짧은 꼬리와 갈라진 발굽.
게임 역할/밸런스는 기존 표를 유지한다. 뿔 등 새로운 공격 기능을 추가하지 않는다.

- +X 정면/+Z 위, root 바닥, 단위1m. 몸통/가슴 혼합 가중치, 머리/턱/귀와 네 다리, 꼬리 리그.
- 1개 VertexColor 재질/UV0, 최대 2가중치. 원본 `ArtSource/monster/sheep_a/sheep_a.blend`.
- Idle2s, Walk0.8s(126cm/s), Run0.5s(396cm/s), LookAround2s,
  HitReact0.3s, WakeTelegraph0.8s, Catch0.8s(2026-09-15 재작업 — 아래 절, 처음 0.6s), PushReact0.8s(2026-09-15 신설), Death1.2s. 30fps/in-place, 판정 Notify 없음.
- SleepTime=0이므로 수면 클립 미사용. 필요한 상태는 실제 데이터/AnimBP 연결에서 다시 대조한다.
- 납품: SK/개별 AN FBX, 원본, manifest, 중립/야간과 주요 자세. 바닥/루프/발·털 경계 변형,
  UE 크기/축/리그/클립/실제 행동/2인 표현을 검수한다.

제작 결과: 164.90×90.16×116.40cm, 14,776 tris/24뼈/1재질. 8클립 FBX 저장.
중립·야간·달리기·잡기·사망 렌더 확인, Blender 수치 감사 및 UE 크기/길이/발 위치 검수.
[007 기록](../iterations/007.md). 실제 AnimBP/AI·2인 PIE·LOD/성능은 아직 미완료.

2026-09-29 "귀엽게" 1단계 시안(형태+렌더만, 게임 미연결): `ArtSource/monster/sheep_a_v2/`(blend·`preview/compare_*.png`·`measurements.json`),
스크립트 `Tools/Blender/build_sheep_a_v2.py`. 얼굴 ×1.20/1.28/1.35(목 중심 기준), 주둥이 길이 ×0.72로 짧고 둥글게, 귀 길이 ×0.88,
눈썹 현재/완화 2단계, 털 뭉치 ×1.12 참고안, 채도 팔레트(털 밝은 크림, 얼굴 더 짙은 회색, 귀 안쪽 분홍). 원본 형상·클립·FBX는 그대로다.
안 확정 — 사용자 승인 후 리그·클립 재작업(2단계)으로 간다.

2026-09-30 눈 표정 3단 시안(같은 스크립트·blend, 게임 미연결): 평소(Idle/순찰) 눈썹 t=0.5 · 추격(Chase) t=1.0 · 피격(HitReact) **아픈 눈**.
아픈 눈 프리셋은 스크립트의 `PAIN` 표 — 눈썹을 눈 위로 2cm 당기고 안쪽(콧등 쪽) 끝을 0.55rad 들어 바깥 끝이 눈꼬리를 누르게 하며,
눈·눈동자 세로를 ×0.55로 찡그린다(후보 B, 삼각형 수 변화 없음). 후보 A(눈썹만 들기)는 걱정하는 눈, C(거의 감음)는 조는 눈으로 읽혀 B를 골랐다.
렌더: `preview/compare_pain_candidates.png`(근접, 화난 눈 대비), `preview/compare_three_states.png`(neutral.png 카메라). 비교용 얼굴 배율 1.28.

2026-09-15 반응 클립 재작업(animate_creature_reacts.py, [README](README.md) 표): 돼지와 같은 사족 규칙 — Catch 0.6→**0.8초** 박치기(코 앞으로 +20cm, 머리 좌우 흔들기), HitReact 0.3초(정점 뒤 복귀), **PushReact 0.8초 신설**(뒤로 붕 → 착지 → 허우적 → 복귀). 바닥 최저점 +1.2cm.


## 2026-10-20 혼합 동물 모델·언리얼 배치

- 현재 요청 범위: production (정지 3D 외형 제작·UE 맵 배치·저장 후 재열기 검수). 리그와 행동 클립은 이번 배치 완료 조건에 추가하지 않는다.
- 사용자 승인 근거: 최신 혼합 동물 그림에 “이정도로해서 배치해봐”, 이어 “실제 3D 모델을 만들어 언리얼 맵에 배치”. 컨셉 강도 승인과 제작·배치 진행 지시이며 아직 보지 않은 3D 결과의 승인은 아니다.
- 컨셉: ArtSource/monster/concept_batch_1/hybrid_placement_v01/approved_concept.png. 작업 버전: model_hybrid_v01. 기존 후보 보존.
- 현재 단계: blender_work. 단독 입력 이미지를 분리하고 로컬 TRELLIS 생성 후 정지 메시를 검수한다.
