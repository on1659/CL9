# 닭 탈 괴물 — TRELLIS 닭 생성부터 탈 벗기·재착용까지

## 2026-10-17 현재 — 구매 Forward01 기반 짧은 덮침 v06

- 기존 직접 제작 공격 v04는 사용자 연출 거절. 구매 Crawling Ghoul 7공격/2점프 비교 후 **AttackForward01**을 새 본체에 적용했다. 원본 도약·골반/팔 변화를 리타깃하고, 긴 팔의 손 목표를 전방으로 보정하며 이족 착지와 기존 대기 복귀를 저작했다. 기존 체형·바인드/손가락 리그·배치 크기 0.5 유지.
- `/Game/Monsters/MotionReview20261017/AttackPackV01/ABP_chicken_Attack`, 기존 chicken/revealed_attack 행에서 소비. 예고 0.6s, 짧은 이동 0.42s/75cm, 빗나감 복귀 0.72s. 기존 공유 서버 판정/AI 재사용, 저작한 관절 폴 커브로 발 IK가 공격 자세를 보존한다.
- [실제 게임 1배속 약 3.1초](../../../ArtSource/monster/motion_library_20261017/attack_pack_v06/chicken/review/chicken_attack_flat_1x.mp4), [중립광 Blender 1배속](../../../ArtSource/monster/motion_library_20261017/attack_pack_v06/chicken/review/chicken_attack_neutral_1x.mp4), [갤러리](../../../ArtSource/model-gallery.html#latest-motion-tests). 예고→도약/양손 도달→빗나감 착지·복귀→재추격 확인. 실제 게임 녹화의 다른 AI 격리/첫 위치 초기화·평지 서버 검수 한계는 [현재 제작 카드](disguise-trio-next-production.md) 상단에 기록했다.
- 정점/바인드 동일, 오프라인 바닥 최저 +0.600cm, 본 UE 왕복 오차 0.00015cm 미만, 고정 발목 목표 오차 0.001cm 미만. 맵 저장 재열기/전체 디코드/브라우저 1배속 재생 확인. 원본·증거 `ArtSource/monster/motion_library_20261017/attack_pack_v06/chicken/`. `status: owned_pounce_delivered_review_pending`, `job: null`. 기존 옷 탈착은 이번에 수정하지 않았다.


## 2026-10-17 현재 — 새 본체 실제 AI·짧은 덮침

- 사용자 승인: 기존 체형 유지, 짧은 덮침의 예고→공격→빗나감 복귀를 실제 Lvl_Flat에서 검수. `status: native_pounce_delivered_review_pending`, `job: null`.
- 기존 v07 40뼈 본체/바인드, 배치 크기 0.5 유지. 새 native `ChickenMonster_AI` (1500,450,35.5), 독립 MonsterDef `/Game/Data/Monsters/ChickenRevealedAttack` (chicken/revealed_attack). 기존 닭 프리뷰·탈착 시퀀스/일반 ChickenA 행은 보존.
- `/Game/Monsters/MotionReview20261017/AttackV01/ABP_chicken_Attack`: 무료 Mixamo→Blender 이동본을 제자리 클립으로 분리, 실제 속도 재생률·두 발 접촉 커브/월드 IK. 공격은 Blender 직접 저작한 웅크림·고개·팔·짧은 공중 이동·착지 복귀. 서버 상태 시간으로 평가하며 이동/판정은 공유 C++ AI가 수행.
- 검수 초안 예고 0.6s, 이동 0.28s/75cm, 빗나감 복귀 0.65s, CatchRadius 65cm. 실제 PIE의 플레이어 옆 회피로 miss→복귀→추격과 이후 hit 확인. 정지 구간 이동 0cm, 출발 후 Yaw 변화 0°, 고정 발목 목표 오차 0.001cm 미만. 평지 서버 검수 범위이며 전체 충돌/멀티/전투 밸런스 합격으로 확대하지 않는다.
- [실제 게임 1배속 약 12초](../../../ArtSource/monster/motion_library_20261017/attack_v04/chicken/review/chicken_attack_flat_1x.mp4), [갤러리](../../../ArtSource/model-gallery.html#latest-motion-tests). 검수 시작 위치만 같은 맵의 열린 바닥으로 초기화, 다른 AI는 해당 PIE에서 격리. 이후 플레이어 입력·카메라만 제어. 워터마크/전체 디코드/브라우저 원속도 재생·맵 저장 재열기 확인.
- 소와 공통 코드/증거·재개 정보는 [현재 제작 카드](disguise-trio-next-production.md#2026-10-17-현재--소-어깨-돌진닭-짧은-덮침-연결검수) 상단, `ArtSource/monster/motion_library_20261017/attack_v04/`. 이번 공격은 닭 v02 통과본 재사용; 소 수정 때문에 닭을 재베이크하지 않았다. 기존 옷 탈착의 품질 승인 상태를 바꾸지 않는다.


## 2026-10-17 현재 — 소·닭 이동 테스트 레벨 v07 납품

- 사용자 요청: 기존 체형 유지, 대기→걷기→달리기→방향 전환을 테스트 레벨에 적용하고 원속도 영상과 갤러리에 등록. 기존 탈착/손 집기 원본은 수정하지 않았다.
- 현재 닭 원본 `ArtSource/monster/motion_library_20261017/runtime_v07/chicken/chicken_locomotion_runtime.blend`; 검증한 v03 닭 작업을 복사하여 재사용했다. 무료 Mixamo Creature NPC의 breathing idle/walking/run/left turn 45를 기존 40뼈에 회전 차이로 리타깃, 전환 보간·발별 IK·디딤 XY 고정을 적용했다. 본체 정점/뼈 바인드 및 ContactBackV2 원본 SHA-256 동일. 431프레임·30fps, 최저 메시 Z 약 0.600cm, 고정 발목 XY 오차 최대 약 0.00138cm. 손가락/발가락 표면 마찰이나 전체 자기 충돌 합격으로 확대하지 않는다.
- UE 5.8.3 Interchange로 새 SkeletalMesh/AnimSequence 임포트. 테스트 레벨 `/Game/Monsters/MotionReview20261017/V07/QA_CowChicken_Locomotion`, 액터 `MotionReview_chicken`. Native AnimSequence 재생이며 기존 닭 GeometryCache/게임 에셋은 보존했다. Blender→UE 대표 뼈 위치 오차 최대 약 0.00040cm, 저장 후 다른 맵을 거쳐 재열어 설정 유지 확인.
- [닭 실제 PIE 1배속 영상 · 15.1초](../../../ArtSource/monster/motion_library_20261017/runtime_v07/chicken/review/chicken_idle_walk_run_turn_ue_1x.mp4), 1280×816·30fps. 원속도 실시간 녹화, native playback 시작 시 한 번 초기화하고 Python은 관찰/카메라 추적만 수행. MP4 전체 디코드·9개 시간 표본 확인. [갤러리](../../../ArtSource/model-gallery.html) 상단 소/닭 영상 플레이어와 제작 방식/1배속/UE 워터마크, 브라우저 로드·재생 확인.
- 소와 공유한 증거는 `ArtSource/monster/motion_library_20261017/runtime_v07/{engine_import,cold_reopen,test_level,video_manifest,delivery_manifest}.json`. `status: test_level_delivered_quality_review`, `job: null`. 새 모델의 AI/AnimBP/내비·전투·멀티 연결 완료는 아니다. 전체 자연스러움에 대한 사용자 승인은 미확인.

## 2026-10-17 전체 몬스터 모션 전환 · 닭 무료 Mixamo 걷기 시험

- 최신 사용자 범위: 전체 현행 몬스터 모션을 기존/다운로드 애니메이션으로 제작. 기존 탈착·원본 체형·집기 구간은 보존한다. 전체 목록은 [motion-plan](../motion-plan.md)의 2026-10-17 절 참조.
- 이 문서의 정본 `animation_locomotion_contact_back_v2/chicken_disguise_contact_back.blend`를 읽고 새 `ArtSource/monster/motion_library_20261017/chicken_mixamo_probe/chicken_mixamo_walk_probe.blend`에 무료 Mixamo `Creature NPC Pack/mutant walking.fbx`를 리타깃했다. 원래 30fps·시간 배율 1, 90프레임. 40뼈와 메시 원본 정점 동일. 기본 회전 차이를 옮긴 뒤 발별 2관절 IK와 디딤 중 XY 고정을 적용했다. 원본·탈착 구간의 손 집기는 수정하지 않았다.
- 최초 FK/루트 높이 보정만 쓴 시험의 미끄러짐을 확인해 발별 보정으로 교체했다. 이전 Blender 자동 백업/렌더는 보존. 전 프레임 최저 Z 약 2.50cm. 관통 전체·손 접촉·자기 충돌·게임 상태 검수 합격을 뜻하지 않는다. 큰 팔의 몸 옆 이동과 무게감은 추가 시각 평가 대상이다.
- [사선·측면 3초·1배속 영상](../../../ArtSource/monster/motion_library_20261017/chicken_mixamo_probe/review/chicken_mixamo_probe_1x.mp4). 실제 제작 방식·Blender·속도 워터마크, MP4 전체 디코드 통과와 디코드 표본 확인. 편집 원본·90프레임 QA·리그 전용 FBX는 같은 폴더. `status: locomotion_probe_review`, `job: null`. 새 클립 UE 임포트/AI 연결 미실행. 기존 닭 GeometryCache·게임 에셋과 Content/Peb 수정 없음.

작성: 2026-10-11 · 최신 인계 정리: **2026-10-16**(§22; 머리말은 2026-10-12 시점 — 2026-10-16 정정). 프로젝트 루트: `C:/Work/CL9`.
이 문서는 `chicken_disguise`의 **현재 제작 요구·입력 선택·이어하기 순서의 단일 정본**이다.
특정 AI 모델(Astra 등)에 의존하지 않는다. 실제 제작을 요청받은 후 아래 순서와 검수 증거를 따라 진행한다.
아래 시작 상태 표는 제작 전 인계 스냅샷이다. 이후 **2026-10-12 실제 제작**에서 TRELLIS 닭 생성과 새 Blender 리그·벗기/재착용 영상까지 만들었다.

(2026-10-16 정정: 현재 정본은 §22 의 `animation_locomotion_contact_back_v2/chicken_disguise_contact_back.blend` 다. 아래 `animation_trellis_v2` 는 2026-10-12 시점의 결과다.) 2026-10-12 시점 둥근 복원 결과(v2): [Blender 원본](../../../ArtSource/monster/chicken_disguise/animation_trellis_v2/chicken_disguise_round_restored.blend),
[전체 30초](../../../ArtSource/monster/chicken_disguise/animation_trellis_v2/preview/off_on_review.mp4),
[벗기 11초](../../../ArtSource/monster/chicken_disguise/animation_trellis_v2/preview/take_off.mp4),
[재착용 19초](../../../ArtSource/monster/chicken_disguise/animation_trellis_v2/preview/put_on.mp4),
[측면 30초](../../../ArtSource/monster/chicken_disguise/animation_trellis_v2/preview/side_contact_review.mp4).
[새 QA 기록](../../../ArtSource/monster/chicken_disguise/animation_trellis_v2/qa.md)과 [manifest](../../../ArtSource/monster/chicken_disguise/animation_trellis_v2/manifest.json)이 그 시점(2026-10-12) 수정본의 정본이다.
각진 v1 원본과 영상은 반려 이력으로 보존한다. v2는 실제 TRELLIS 곡면으로 돌아왔지만 내부 간섭·천 자기 겹침은 남아 있어 품질 최종 합격이 아니다.

이전 v1 이력: [Blender 원본](../../../ArtSource/monster/chicken_disguise/animation_trellis_v1/chicken_disguise_trellis_animation.blend), [전체 영상](../../../ArtSource/monster/chicken_disguise/animation_trellis_v1/preview/off_on_review.mp4), [당시 QA](../../../ArtSource/monster/chicken_disguise/animation_trellis_v1/qa.md).

별도 Higgsfield Blender 플러그인 시험: [생성 영상](../../../ArtSource/monster/chicken_disguise/animation_higgsfield_pickup_v1/preview/higgsfield_chicken_pickup.mp4), [Blender 시험 파일](../../../ArtSource/monster/chicken_disguise/animation_higgsfield_pickup_v1/chicken_disguise_higgsfield_pickup.blend), [manifest](../../../ArtSource/monster/chicken_disguise/animation_higgsfield_pickup_v1/manifest.json). 플러그인 생성·리타게팅·렌더·전체 디코딩은 통과했지만, 몬스터가 탈을 잡아 올리는 의미 동작은 실패했다. 몬스터가 탈에서 멀어지며 뛰어오르고 탈은 바닥에 남는다. 기존 잔액에서 0.1크레딧이 차감됐고 이번 실행의 추가 결제는 0원(잔액 10 → 9.9)이다. 계정의 크레딧 출처·요금제가 확인되지 않아 0.1크레딧의 원화 환산은 확정할 수 없다. Higgsfield는 생성 전 비용을 버튼에 표시하고 크레딧 팩 가격은 결제 시점에 표시한다고 안내한다([크레딧 기준](https://higgsfield.ai/creator-hub/help-center/credits/how-credits-work), [크레딧 팩 가격](https://higgsfield.ai/creator-hub/help-center/credits/how-credit-packs-work)). 이 영상은 승인된 애니메이션이 아닌 플러그인 시험 결과다.

**이전 v1의 반려 당시 품질 기록:** 재착용 중 얼굴/목/팔/손가락-탈 교차, 닫힌 내부 수납 간섭, 낙하 탈의 약 18mm 바닥 간격이 남았다.
[실제 QA와 실패 프레임](../../../ArtSource/monster/chicken_disguise/animation_trellis_v1/qa.md),
[manifest](../../../ArtSource/monster/chicken_disguise/animation_trellis_v1/manifest.json)이 현재 작업 상태의 정본이다.
당시 v1 편집 체크포인트는 animation_trellis_v1/checkpoints/G_grasp_clearance.blend다. 현재 이어갈 파일은 animation_trellis_v2/chicken_disguise_round_restored.blend다. TRELLIS 기존 결과는 새 생성으로 덮지 않는다. Higgsfield 시험은 animation_higgsfield_pickup_v1/에서 한 번 실행해 실제 모션·영상 검수를 마쳤다. 0.1크레딧이 사용됐고 픽업 의미 검수는 실패했다. 추가 유료 생성은 하지 않는다. 열린 사용자 Blender 씬과 기존 결과를 보존했고 UE 연결은 수행하지 않았다.

## 2026-10-12 둥근 외형 복원과 새 애니메이션 v2

사용자 “ㅇㅇ네모난 닭이말이되냐 돌려”에 따라 직전 제시한 blender-round-source-v5 둥근 외형 복귀를 승인받았다. 닭의 실제 곡면·UV·큰 날개·볏·부리·발을 복원하고 무대의 기존 약 3m 높이에 맞춰 원시 닭 전체를 고정 균일 1.12배로 배치했다. 본체 수납을 위해 닭 하부를 비균일하게 다시 키우지 않았다.

새 폴더 animation_trellis_v2에서 본체 고정 길이 접힘, 별도 중앙 안감, 맞닿는 뒤 개구부, 실제 천 정점 경로의 손목·손가락 집기, 오른손 지지 F480 유지, 마지막 손 복귀 뒤 F860–895 닫힘을 수정했다. 900개 정수 프레임의 evaluated mesh 검사에서 손끝 패드–외피 최근접 거리 최대 약 1.32cm, 본체 최저 2.70mm, 바닥 탈 최저 약 4mm, 뼈 길이 최대 상대 오차 약 1.90e-6이다. Scale/visibility 애니메이션은 없다.

본체–외피 BVH 겹침 후보와 내부 수납·천 자기 겹침, 뒤/아래 본체 노출, 일정 두께·장력·질량 기반 체중 이동이 남았다. 실제 천 시뮬레이션 합격이나 관통 0을 주장하지 않는다. 상세 실패 부위·검사 범위·반려한 수정 시험은 v2 QA에 기록했다. 기존 Blender 원본과 승인 v5 외형 해시가 그대로임을 확인했다.

전체·측면 900프레임, 벗기 330프레임, 재착용 570프레임, 측면 0.5배속 1,800프레임을 실제 전체 디코딩했다. 모두 1920×1080/30fps다. 최종 MP4 포즈 시트에서 둥근 시작/종료와 남은 복귀 궤적 급변을 확인했다.

현재 Blender SHA256: dd2d166df996f70ef20f667c683c53d29682e2ba0d543dd4a135db7b3655ad02. 이 파일과 새 영상은 수정본 납품이며 사용자의 전체 애니메이션 최종 승인을 대신하지 않는다.

## 2026-10-12 외형 반려와 둥근 원형 재검토

사용자 피드백: **“?? 닭이 왜이렇게 네모나게된거임?”** — 직전 `animation_trellis_v1` 영상의 각진 닭 외형은 반려됐다.
원인은 TRELLIS 원형을 절차적 외피로 재구성한 뒤, 수납용으로 뒤쪽 30%, 하부 X 최대 70%/Y 28%와 앞쪽 여유를 추가하면서 승인 이미지의 둥근 비율을 잃은 것이다. 생성 원본 자체는 둥글다. 기존 영상·원본은 이력으로 보존하며 승인본으로 취급하지 않는다.

```yaml
concept_pipeline:
  phase: handoff
  status: working
  concept:
    approved_version: approved_chicken_disguise_20261011
  blender:
    candidate_version: blender-round-source-v5
    approved_version: blender-round-source-v5
  checkpoint:
    path: ArtSource/monster/chicken_disguise/animation_trellis_v1/shape_revision_v5/chicken_round_source_review.blend
    sha256: 64b09e30508e83e9d13b6c1297e91ca10013b182f9a7a26d0094cab1b073ae5c
  pending_review: null
  approval: 사용자 “ㅇㅇ네모난 닭이말이되냐 돌려” — 제시한 둥근 v5 외형으로 복귀 승인
  job: null
  last_completed: 둥근 표면·고정 길이 수납·실제 천 집기 복원, 새 Blender 저장, 900프레임 수치 검수, 영상 5개 전체 디코딩과 실제 MP4 포즈 시트 검수
  next_action: 남은 내부 수납 간섭·천 겹침·마감·복귀 궤적과 체중 이동을 수정; v2 원본 및 영상/QA에서 이어가기
```

[둥근 원형 사선 렌더](../../../ArtSource/monster/chicken_disguise/animation_trellis_v1/shape_revision_v5/preview/three_quarter.png) · [Blender 외형 검토본](../../../ArtSource/monster/chicken_disguise/animation_trellis_v1/shape_revision_v5/chicken_round_source_review.blend) · [manifest](../../../ArtSource/monster/chicken_disguise/animation_trellis_v1/shape_revision_v5/manifest.json).
Manifest SHA256: `62607d33c7c12ed316eb8a03fddc963bd638568367026743807e0985f8fc3594`. 정확한 생성 표면/텍스처를 유지한 **외형 검토본**이며 새 속 빈 탈·리그·애니메이션 완성본이 아니다.
`shape_revision_v2`는 절차적 외피 복귀만으로 원형을 회복하지 못한 시험, `v3/v4`는 원시 얇은 장식에 Solidify를 적용해 표면 오류가 생긴 내부 시험으로 채택하지 않는다. 사용자 Blender와 v1 영상은 변경하지 않았다.

## 0. 다른 AI가 처음 읽을 것

### 게임에서 이 장면이 필요한 이유

CL9은 동물 주인이 지키는 장소에서 재료를 훔쳐 푸드트럭에서 요리하는 협동 공포 게임이다.
이 캐릭터는 평상시 **크림색 봉제 닭 탈 안에 긴 사지의 본체가 접혀 있는 괴물**이다.
광폭화하면 본체가 위로 빠져나오고 탈은 아래로 떨어진다. 진정하면 바닥의 **같은 탈**을 직접
집어 펼쳐 몸을 접어 넣고 다시 닭으로 위장한다. 사용자가 말한 “닭가죽”은 이 봉제 탈이다.
진정 조건·AI·광폭화 수치는 이 모델 제작에서 새로 정하지 않는다.

### 읽는 순서와 문서 역할

1. [에셋 안내](../README.md) → [공통 규격](../common.md) → [디자인 방향](../design-concept.md).
2. [제작·검수 과정](../workflow.md)과 [TRELLIS 실행법](../tools/trellis2.md).
3. 이 문서 전체와 아래 기준 이미지를 **실제로 열어** 확인.
4. 옛 시안의 실패를 조사할 때만 [전신 모델 기록](chicken_disguise-fullbody-model.md),
   [이전 애니메이션 기록](chicken_disguise-natural-production.md), [초기 동작 시안 이력](chicken_disguise-animatic.md)을 읽는다.

공통 좌표·명명·내보내기 규칙은 위 공통 문서가 관리한다. 본 문서는 이 캐릭터의 외형·탈 구조·연기·검수 요구를 관리한다.
과거 문서의 “애니메이션 중단”은 당시 이력이다. 이번 인계의 목표는 **새 외형으로 닭 생성부터 벗기·재착용까지 이어갈 수 있게 하는 것**이다.
기존 결과의 외형 채택을 임의로 확정하거나 옛 영상의 통과 수치를 새 메시의 검수로 옮기지 않는다.

### 0-A. 시작 시점의 실제 상태

2026-10-12 디스크 확인 기준. 다음 작업자는 실행 전 다시 확인한다.

| 대상 | 실제 상태 | 다음 작업 |
|---|---|---|
| TRELLIS 환경 | 이 컴퓨터에서 본체 생성 성공. Windows low-VRAM 포크와 로컬 패치 사용 | 재설치부터 하지 말고 도구 문서의 환경 확인 |
| TRELLIS 본체 | `trellis2_v1/monster/model.glb`, Blender 원본·4방향 렌더 있음. 95,152 triangles, 1 material, 리그 없음 | 변형용 메시·얼굴·손발 정리 |
| TRELLIS 닭 | `trellis2_v1/chicken/model.glb` **없음** | 아래 닭 단독 이미지로 새 생성 |
| 옆 세션 외형 모델 | `model_fullbody_v1/{chicken,monster}/`, 정지 모델. 닭 134,942 / 본체 70,356 triangles | 외형/편집 부품 비교용. 그대로 최종 리그라고 하지 않음 |
| 이전 자연 동작 시안 | `animation_natural_v1/`에 모델·리그·30초 영상 있음. 이후 사용자가 외형 반려 | 접촉·타이밍·검사 방식만 참고. 옛 외형·완료 판정 자동 승계 금지 |
| TRELLIS 후속 애니메이션 | animation_trellis_v1/에 Blender 원본과 벗기·재착용·전체·측면 영상이 있음. E_authored_QA_pending; C/D/E 미검수, F/G 미실행 | 아래 실제 QA·manifest에서 결함과 다음 단계 확인 |
| Higgsfield 집기 시험 | 별도 animation_higgsfield_pickup_v1/에서 플러그인 1.5.57로 1회 생성 완료, 19본 리타게팅·4.5초 MP4 저장. 0.1크레딧 사용, 잔액 9.9, 이번 실행 추가 결제 0원 | 영상은 디코딩 통과했지만 탈이 들리지 않아 픽업 동작 불합격. 추가 유료 생성은 새 사용자 요청 전까지 보류 |

표의 상대 경로 기준은 `ArtSource/monster/chicken_disguise/`이다.
현재 본체는 긴 팔·굽은 등·웅크린 자세가 옆 세션 모델보다 기준 그림에 가까워 **후속 작업의 시작 후보**로 둔다.
이는 비교 검수의 판단이며 사용자의 최종 외형 승인이나 게임 사용 승인이 아니다.
[동일 조건 비교](../../../ArtSource/monster/chicken_disguise/trellis2_v1/comparison_with_neighbor/comparison.jpg)와
[실행/외형 검수 기록](../../../ArtSource/monster/chicken_disguise/trellis2_v1/monster/blender_qa.json)을 확인한다.

본체의 알려진 문제:
- 얼굴 텍스처에 의도하지 않은 동심원 무늬가 있다. 작은 점 눈·가는 세로 틈을 유지하며 고친다.
- 손가락·발 형태와 지면 접촉을 다듬어야 한다. 포즈를 이미 갖고 있는 정지 메시라 자동 웨이트 결과를 그대로 믿지 않는다.
- 뒤쪽은 단일 이미지에서 추정됐다. 원본에 없는 정교한 해부학이라고 주장하지 않는다.
- 생성 모델은 조밀한 삼각 메시다. 관절 변형과 손으로 천 잡기에 적합한 토폴로지를 별도로 확보해야 한다.

### 0-B. 입력 이미지와 재사용 파일

| 용도 | 프로젝트 루트 기준 실제 파일 |
|---|---|
| 최상위 외형·상태 기준 | `ArtSource/monster/chicken_disguise/reference/approved_chicken_disguise_20261011.png` |
| 닭 TRELLIS 입력 | `ArtSource/monster/chicken_disguise/reference/fullbody_redraw_20261011_v1/chicken_fullbody.png` |
| 본체 생성에 쓴 입력 | `ArtSource/monster/chicken_disguise/reference/fullbody_redraw_20261011_v1/monster_fullbody.png` |
| 재사용할 본체 원시 결과 | `ArtSource/monster/chicken_disguise/trellis2_v1/monster/model.glb` |
| 본체 검토용 Blender | `ArtSource/monster/chicken_disguise/trellis2_v1/monster/chicken_disguise_trellis.blend` |
| 본체 생성 설정/입력 해시 | `ArtSource/monster/chicken_disguise/trellis2_v1/monster/generation.json` |
| 비교용 닭 모델 | `ArtSource/monster/chicken_disguise/model_fullbody_v1/chicken/chicken_fullbody.blend` |
| 비교용 본체 모델 | `ArtSource/monster/chicken_disguise/model_fullbody_v1/monster/monster_fullbody.blend` |

입력 SHA256:
- 닭: `27a1678135b59a05d820070ce412842d7232ae8c8fc0e59d71a66792ae53255a`.
- 본체: `ba6b6bd530e7967fbabd39ec4f014d0414df505eca6bd6061c0615a3df15ede9`.

전신 단독 이미지는 원래 채택 콘셉트에서 파생한 **제작용 후보**다. 별도의 최종 승인으로 기록하지 않는다.
이미지 간 차이는 원래 채택한 외형과 현재 동작 요구에 맞춰 설명한다.
이번 목표는 부드러운 봉제 닭과 어두운 각진 본체의 대비다. 과거 공통 문서의 flat shading·vertex color만을
기계적으로 적용해 채택 이미지의 봉제 외형을 없애지 않는다. 고밀도 초안과 게임용 예산 준수 여부는 별도 기록한다.

### 0-C. 다음 세션의 첫 실제 작업 — TRELLIS 닭 생성

공통 환경·오류·중단 재개 규칙은 [도구 문서](../tools/trellis2.md)가 정본이다.
아래는 **이 닭의 첫 생성용 구체 명령**이다. 폴더가 비어 있지 않으면 종료하며,
이미 성공한 결과가 있으면 먼저 검수한다. 조건을 바꾸는 새 후보는 `trellis2_v2/chicken` 등 새 폴더를 쓴다.

PowerShell, 프로젝트와 설치가 이 컴퓨터에 그대로 있는 경우:

```powershell
$trellisRoot = 'C:/Work/Tools/TRELLIS2'
$trellisPython = "$trellisRoot/.venv/Scripts/python.exe"
$chickenImage = 'C:/Work/CL9/ArtSource/monster/chicken_disguise/reference/fullbody_redraw_20261011_v1/chicken_fullbody.png'
$chickenOutput = 'C:/Work/CL9/ArtSource/monster/chicken_disguise/trellis2_v1/chicken'

# 입력 존재와 해시, 빈 출력 폴더를 검사하고 준비한다. 실패 시 기존 결과를 건드리지 않는다.
& $trellisPython -c "import pathlib,sys,hashlib; s=pathlib.Path(sys.argv[1]); p=pathlib.Path(sys.argv[2]); assert s.is_file(), 'input missing'; assert not p.exists() or (p.is_dir() and not any(p.iterdir())), 'output is not empty: inspect existing result'; p.mkdir(parents=True,exist_ok=True); print(hashlib.sha256(s.read_bytes()).hexdigest())" $chickenImage $chickenOutput
if ($LASTEXITCODE -ne 0) { throw '입력/출력 확인 실패. 생성하지 않습니다.' }

& $trellisPython -u "$trellisRoot/cl9_generate.py" `
  --image $chickenImage --output $chickenOutput `
  --resolution 512 --seed 42 --steps 12 --faces 100000 --texture 2048 `
  *> "$chickenOutput/generate.log"
if ($LASTEXITCODE -ne 0) { throw '닭 생성 실패. 로그와 중간 파일을 보존하고 도구 문서의 재개 절차를 확인하세요.' }
```

이 닭 명령은 현재 runner의 인자를 확인해 작성했으며 **닭 생성 자체는 아직 실행하지 않았다**.
`run-cl9-monster.ps1`은 본체 입력이 고정되어 있으므로 닭 생성에 사용하지 않는다.
`cl9_preview_blender.py`도 본체 이름·높이 3.9m가 고정된 프리뷰이므로 닭에 그대로 쓰지 않는다.

생성 직후 확인:
1. `input_preprocessed.png`에서 볏·꼬리·두 날개·두 발이 모두 남았는지 확인한다.
2. 원시 GLB를 별도 Blender로 읽고 정면·옆·뒤·3/4를 렌더한다. 정지 그림을 3D 결과로 대신하지 않는다.
3. 둥근 세로 몸통, 작은 검은 눈, 붉은 볏/육수, 노란 부리/발, 큰 날개, 봉제선의 인상이 유지되는지 비교한다.
4. 생성 과정에서 붙은 발가락·날개, 비어 있는 면, 텍스처 이음새, 불필요한 그림자/바닥 조각을 기록·정리한다.
5. 외형 후보를 보존한 뒤 **속 빈 탈로 재구성**한다. 생성된 닭을 단순히 위아래로 늘여 본체를 숨기지 않는다.

### 0-D. 단계별 저장과 이어하기

| 단계 | 수행할 일 | 다음으로 넘어갈 증거 |
|---|---|---|
| A. 입력·상태 확인 | 위 이미지/본체와 이전 기록을 읽고 새 출력 위치를 확정 | 입력 해시·환경·파일 현황을 manifest에 기록 |
| B. 닭 생성·외형 보정 | TRELLIS 닭 생성, 본체 얼굴/손발 정리 | 닭/본체 4방향 실제 렌더, 원본과 차이 기록 |
| C. 탈 구조·체적 | 닭 내부·입구·두께·장식 연결, 본체 접힌 자세 설계 | 측면/단면에서 고정 길이 사지가 탈 안에 들어가는 증거 |
| D. 리그 시험 | 본체 IK/손가락·의상 변형·공간 전환 구성 | 손 접지, 천 집기, 들어 올리기, 낙하, 마지막 팔 넣기 짧은 시험 |
| E. 주요 동작 | 아래 타이밍 표에 맞춰 별도 벗기/입기 연기 | 고정 3/4와 측면 저해상도 전체 영상 |
| F. 수정·천 움직임 | 실패 프레임 수정, 속도·체중 이동·처짐·잔움직임 보강 | 접촉/관통 수치와 1배속·0.5배속 시각 검수 |
| G. 납품·재열기 | Blender 원본 재열기, 최종 렌더·MP4 재읽기 | 아래 산출물과 실제 QA·미해결 항목 |

작업 원본은 `animation_trellis_v1/checkpoints/`에 단계별로 보존한다.
같은 작업을 재개할 때는 manifest의 마지막 실제 완료 단계와 해당 파일을 먼저 읽는다.
GPU 생성·렌더를 중복 실행하거나 완료된 단계부터 무조건 재생성하지 않는다.
통상적인 도구 선택·리그 수정·자체 검수는 제작 요청 범위에서 계속 진행한다.
**닭 이미지 한 장이나 정지 모델만 납품하고 요청된 애니메이션을 끝냈다고 하지 않는다.**

## 1. 목표와 현재 범위

첨부한 컨셉아트의 닭 탈 괴물로 다음 동작을 자연스럽게 제작한다.

1. 평상시 귀여운 닭 탈 안에 본체가 접혀 있다.
2. 광폭화하면서 봉제선이 아래에서 위로 벌어지고 본체가 위로 빠져나온다.
3. 탈이 본체와 완전히 분리되어 바닥에 떨어지고 천처럼 가라앉는다.
4. 진정하면 괴물이 바닥의 **그 탈**을 직접 집어 펼친다.
5. 몸과 팔다리를 차례로 접어 넣고 탈을 여며 다시 닭이 된다.

이번 납품은 수정 가능한 Blender 원본과 리그·애니메이션, 실제 렌더 영상이다.
기존 16초 초안의 파츠 위치 보간을 조금 다듬는 수준에서 끝내지 않는다.
손·천 접촉과 체중 이동을 만들 수 있도록 필요한 모델·리그 재제작을 포함한다.
게임의 광폭화 게이지, AI, 전투 수치, UE 임포트·멀티플레이 연결은 이번 범위 밖이다.
영상에서는 진정 계기를 타임라인 이벤트로 가정한다. 게임 기능 구현이라고 보고하지 않는다.

## 2. 외형의 최상위 기준 — 반드시 이미지를 직접 열어 볼 것

![사용자가 최종 선택한 외형](../../../ArtSource/monster/chicken_disguise/reference/approved_chicken_disguise_20261011.png)

- 정본 파일: `C:/Work/CL9/ArtSource/monster/chicken_disguise/reference/approved_chicken_disguise_20261011.png`
- SHA256: `83ecc108441a821b43605f1ef988c9d46f333a2d7bfa41807913d3077d213da1`
- 사용자의 최신 결정: **“이거로 가자. 얘로 자연스럽게 옷 입고 벗는 애니메이션을 만들고 싶다.”**
- 이 그림은 분위기·색 참고에 그치지 않는 **외형과 비율의 기준**이다.
- 이미지에서 확인되지 않는 등·내부 구조·리그·정확한 치수는 제작 가정으로 기록한다.

### 맞춰야 하는 형태

| 부위 | 형태 기준 |
|---|---|
| 평상시 닭 | 둥글고 통통한 크림색 봉제 탈. 작은 검은 눈, 붉은 볏·육수, 노란 부리와 발. 날개는 큰 부드러운 덩어리 |
| 봉제선 | 옆·등을 타고 이어지는 굵고 드문 실땀. 평상시부터 보여 변신을 예고 |
| 본체 얼굴 | **길고 좁은 밝은 타원형**, 아주 작은 어두운 눈 두 개, 아래로 짧게 이어지는 가는 세로 틈. 첨부 이미지의 단순한 얼굴을 유지 |
| 본체 몸 | 어둡고 마른 몸통, 솟은 어깨와 굽은 등. 매끈한 막대 조립품 대신 어깨·팔꿈치·손목이 연결되는 형태 |
| 팔다리 | 몸에 비해 매우 긴 팔, 크게 굽은 무릎, 바닥을 짚는 길고 벌어진 손가락. 손이 체중을 받는 자세 |
| 스타일 | 큰 면과 실루엣이 읽히는 로우폴리 계열. 탈은 부드럽고 본체는 뼈대와 관절이 도드라진다. 촘촘한 삼각 면이나 털로 복잡하게 만들지 않음 |
| 상대 크기 | 옆의 작은 요리사보다 훨씬 큰 본체. 같은 카메라·바닥 위에 비교 캐릭터를 두고 비율을 대조. 그림의 원근을 그대로 실측값으로 오해하지 않음 |

**이전 대화의 얼굴을 더 무섭게 바꾸라는 요청보다 이번 이미지 선택이 우선한다.**
`animatic_v3`의 긴 이빨 입, `animatic_v4`의 튀어나온 주둥이·크게 벌어진 턱은 채택하지 않는다.
첨부 얼굴이 가면처럼 보인다는 이유로 다시 해부학적 입·송곳니·뿔을 추가하지 않는다.
공포감은 본체의 실루엣, 정지와 급격한 움직임의 대비, 작은 시선 이동으로 만든다.

### 그림과 동작 요구가 다른 부분

그림의 본체에는 닭 탈이 허리에 걸려 있다. 이것은 **벗는 도중의 순간**으로 사용한다.
완료 자세에서는 반드시 탈이 땅에 떨어져 있고 본체와 분리되어야 한다.
최신 사용자 동작 요구가 그림의 정지 자세보다 우선한다.

## 3. 기존 파일의 용도

| 경로 | 재사용할 수 있는 것 / 주의 |
|---|---|
| `docs/asset/monster/chicken_disguise-animatic.md` | 과거 타이밍과 실패·한계 기록. 현재 외형 정본은 이 문서의 첨부 이미지 |
| `ArtSource/monster/chicken_disguise/animatic_v2/chicken_disguise_animatic_v2.blend` | 벗기·낙하·회수·재착용 순서, 무대·카메라 참고. 16초 오브젝트 애니메이션 초안으로 최종 리그 아님 |
| `Tools/Blender/build_chicken_disguise_off_on.py` | 이전 초안 생성 코드. 그대로 다시 돌려 자연스러운 애니메이션을 완성했다고 하지 않음 |
| `animatic_v1` ~ `animatic_v4` | 이력 보존. 원본을 덮거나 기존 얼굴을 자동 채택하지 않음 |

본체의 막대형 팔다리, 두 개의 단단한 탈 껍질, 전체 Scale로 납작하게 만든 천은 교체 대상이다.
이미지 생성 결과를 실제 3D 모델 렌더 또는 애니메이션 검수 결과로 대신하지 않는다.

## 4. 동작을 만들 수 있는 모델과 리그부터

### 생성 메시를 변형용 모델로 준비

- 원시 GLB는 보존하고 별도 작업 메시를 만든다. 현재 본체 GLB는 Blender 비교에서 정면이 -Y였다.
  실제로 다시 확인한 뒤 공통 규격 +X 정면으로 맞추며, 이 경우 Z축 +90°가 출발값이다.
  본체 프리뷰의 3.9m와 옆 세션 닭 약 2.65m는 **외형 가정**이다. 리깅 전에 같은 씬에서 비교하고
  내부 접힘 시험으로 상대 크기를 정한다. 런타임 캡슐·판정 크기를 바꾸지 않는다.
- 생성된 웅크림을 그대로 rest pose로 쓸지, 외형을 유지하며 더 펴진 작업 포즈로 정리할지 결정·기록한다.
  어느 쪽이든 완전 접힘/완전 탈출/바닥 지지 자세에서 무릎·팔꿈치·겨드랑이·사타구니를 시험한다.
- 리토폴로지 또는 국소 재구성으로 어깨·팔꿈치·손목·손가락·골반·무릎의 접힘을 확보한다.
  Decimate만 한 메시를 변형 합격으로 처리하지 않는다. 원시 고밀도 표면의 색·질감은 정리 메시로 베이크한다.
- 얼굴 동심원은 의도한 디자인이 아니다. 원래 밝은 타원·점 눈·세로 틈에 맞춰 재도색/재질·UV를 정리한다.
  옆 세션 얼굴 부품을 참고/재사용하면 출처와 변경점을 기록하고 비율·접합을 실제 렌더로 확인한다.
- 몸·얼굴·손가락·장식의 검사 대상을 manifest에 명시한다. 바깥 몸통 한 개만 검사하고 전체 관통이 없다고 하지 않는다.

### 본체

- 단일 바닥 `root` 아래 골반·척추·가슴·목·머리, 쇄골·상하완·손목, 허벅지·종아리·발을 제어한다.
- 손은 천을 집고 놓는 손가락 제어가 필요하다. 주먹 전체를 옷에 갖다 대는 것으로 접촉을 대신하지 않는다.
- 손·발 IK와 pole 제어, 필요 구간 IK/FK 전환을 둔다. 어깨·골반의 체중 이동을 별도로 만든다.
- 관절 주변 메시가 구부러질 수 있게 연결하고 스킨 웨이트를 검수한다.
- 본체가 작아졌다 커지는 전역 Scale 변신을 쓰지 않는다. 긴 사지는 관절로 접고 몸통은 숙여 숨긴다.
- 제작 전 내부 접힘 포즈를 측면/단면으로 확인한다. 탈에 들어가지 않으면 관절 배치와 탈 내부 여유를 조정하되, 첨부 실루엣과의 차이를 기록한다.

### TRELLIS 닭을 실제 벗을 수 있는 탈로 바꾸기

- TRELLIS 결과의 외피를 형태 기준으로 삼아 내부 공간과 열리는 등/옆 봉제선을 만든다.
  생성물이 닫힌 솔리드처럼 보여도 속 빈 옷과 같은 구조라고 가정하지 않는다.
- 고정 토폴로지로 봉제선 양쪽 경계를 준비하고 붙어 있던 가장자리를 제어해 벌리는 방식을 기본안으로 삼는다.
  연출용 실땀은 벌어짐에 맞춰 풀리되 탈의 주 메시·정체성은 유지한다. 프레임마다 임의 remesh로 UV/웨이트를 깨지 않는다.
- 외형에 보이는 닭 발은 탈에 연결된 부품으로 유지한다. 본체의 발·손 출입 경로와 착용 시 위치를 단면에서 설명한다.
  장식이 본체 발로 갑자기 바뀌거나 낙하 중 사라지지 않게 한다.
- 두께는 모델 크기에 맞는 제작 가정으로 기록한다. 입구 단면·안감·접힌 부분이 뒤집히거나 벌어져도
  Two Sided 재질로 구조 오류를 가리지 않는다.
- 몸통·머리/볏·날개·밑단의 무게감이 다르게 보이도록 제어 영역을 나눈다. 어느 뼈/형태 키/캐시가
  큰 형태와 잔움직임을 담당하는지 문서화하고 과도한 장식 늘어남을 검수한다.

### 닭 탈 리그와 움직임

- 몸통·머리·날개가 이어진 **속이 빈 봉제 옷**으로 만든다. 두 개의 단단한 달걀 껍질처럼 열리지 않는다.
- 부리·볏·발·눈 등의 장식은 탈에 붙어 함께 움직인다. 내부가 비면 닭 머리도 무게에 따라 처진다.
- 바닥에 떨어져도 회수할 수 있는 큰 한 벌 형태를 유지한다. 장식과 주요 천을 조각조각 소실시키지 않는다.
- 열리는 봉제선, 손으로 잡는 두 지점, 바닥에 먼저 닿는 부분을 이름 붙여 관리한다.
- 탈 몸통·입구·머리·날개·밑단을 제어하는 리그 또는 이에 준하는 변형 구조를 둔다.
- 기본 큰 움직임과 잡는 위치는 리그/키프레임으로 제어하고, 자유로운 밑단·실밥·천의 잔움직임은 시뮬레이션이나 보조 뼈로 더한다.
- 실제 Cloth 사용 여부 자체가 합격 기준은 아니다. 무게·접촉·주름·낙하가 보이고 재현 가능한지가 기준이다.

### 같은 옷이 몸 → 바닥 → 손 → 몸으로 이동해야 한다

- 몸에 착용 중인 공간, 월드 바닥, 손에 붙는 공간 사이의 제약 전환에서 월드 위치·회전이 튀지 않게 한다.
- 잡고 있는 동안 손가락과 천의 잡는 지점은 붙어 있어야 한다. 손이 먼저 닿고 잡은 다음에 옷이 움직인다.
- 떨어뜨린 탈의 위치·방향을 유지한다. 재착용 때 처음 위치로 순간이동시키지 않는다.
- 리그를 분리한다면 본체와 탈 Action을 같은 프레임 범위·기준 원점·이벤트 표로 동기화한다.
- 베이크된 캐시·제약·드라이버 의존성을 기록한다. FBX로 내보내지 않은 천 효과를 엔진용 애니메이션이라고 부르지 않는다.

## 5. 연기와 타이밍

아래 초는 **약 30초 감상용 프리뷰의 제작 시작값**이다. 30fps, 1–330프레임 벗기 / 331–900프레임 진정·입기를 출발점으로 둔다.
기존 16초에 억지로 맞추지 않는다.
무게와 접촉을 위해 조절할 수 있으며 실제 프레임은 제작 카드에 기록한다.
공격 예고 시간이나 게임의 광폭화 수치를 정한 것이 아니다.

### 벗기 — 약 0~11초

| 구간 | 동작과 검수 초점 |
|---|---|
| 0~2초 | 닭이 호흡하다 멈춘다. 내부의 작은 압력 변화와 봉제선 당김. 몸 전체를 같은 파형으로 흔들지 않음 |
| 2~3.5초 | 아래쪽 실땀부터 위쪽으로 차례로 풀리며 상단 입구가 위·바깥으로 젖혀진다. 압력이 먼저, 천의 반응은 조금 뒤 |
| 3.5~6초 | 본체가 머리와 한쪽 어깨를 먼저 내밀고 한 손으로 바닥을 짚는다. 이어 반대 어깨·손이 빠져나옴. 양쪽이 기계처럼 동시에 펼쳐지지 않음 |
| 6~8초 | 손과 발로 무게를 지탱하면서 등을 펴고 다리를 차례로 뺀다. 탈이 잠시 허리에 걸렸다가 풀려 아래로 내려옴 |
| 8~9.5초 | 마지막 걸린 부분이 빠지면 천이 중력으로 낙하. 한쪽 가장자리 → 몸통 천 → 닭 머리 순서처럼 무게 차이가 드러남. 바닥 접촉 후 작은 잔움직임 |
| 9.5~11초 | 본체가 낮게 웅크려 정지한 뒤 고개만 천천히 들어 플레이어를 본다. 바닥의 탈과 본체가 함께 보이는 완료 자세 |

“위로 찢어짐”은 천 전체가 공중으로 발사되는 뜻이 아니다. 봉제선이 아래→위로 열리고 본체가 위로 빠져나온 뒤 **옷은 아래로 떨어진다**.

### 진정과 재착용 — 약 11~30초

| 구간 | 동작과 검수 초점 |
|---|---|
| 11~14초 | 광폭 상태 유지 후 진정 계기. 호흡·어깨 긴장이 누그러지고 머리가 바닥의 옷을 찾음 |
| 14~17초 | 한 손/두 발로 몸을 지탱하면서 다른 손을 내려 옷 가장자리를 집는다. 손가락이 닫힌 뒤 잡은 부분부터 들림 |
| 17~20초 | 반대 손이 다른 가장자리를 집고 입구를 벌린다. 천 아래쪽은 늦게 따라 올라오며 닭 머리와 볏이 처져 흔들림 |
| 20~24초 | 골반을 낮추고 발과 무릎을 하나씩 안으로 넣는다. 몸통을 접어 넣는 동안 한 손은 입구를 잡고 다른 손은 균형/옷 당김을 담당 |
| 24~27초 | 한 팔을 먼저 안으로 넣고, 남은 손으로 탈의 윗부분을 끌어올린다. 머리와 마지막 팔이 차례로 숨음. 본체를 통째로 아래로 밀어 넣지 않음 |
| 27~29초 | 봉제선 양쪽을 당겨 여미고 남은 실밥과 천이 정리됨. 손이 사라진 이후 마법처럼 옷이 복구되는 연출을 피함 |
| 29~30초 | 작은 닭 자세로 무게가 내려앉고 호흡으로 연결. 마지막 닭의 크기·비율은 시작과 일치 |

입기는 **벗기 역재생이 아닌 별도 연기**로 제작한다. 입을 때는 옷을 찾고, 잡고, 펼치고, 균형을 잡는 과정이 추가된다.
지지 손을 떼는 순간에는 이미 다른 손/발로 무게를 옮겨야 한다.

### 찢어진 봉제선을 다시 닫는 방식 — 제작 가정

사용자가 확정한 것은 “다시 입는다”이며 봉합 장치는 미정이다.
이번 제작 기본안은 **기존 봉제선 안쪽에 연결된 여밈끈**이다.
겉의 성긴 실밥이 끊어져도 천과 안쪽 끈은 이어져 있고, 마지막에 남은 손이 끈을 당겨 입구를 모은다.
손이 숨기 전까지 여밈 동작이 보이도록 입구 위치와 접는 순서를 설계한다.
끊어진 실밥은 남겨 둔다. 새 바느질·원상복구가 필요하다는 설정을 임의로 확정하지 않는다.
이 가정으로 우선 제작하되, 불가능한 관통이나 순간 복구로 해결하지 말고 카드에 가정과 결과를 명시한다.

## 6. 작업 순서

1. §0-A~D로 실제 상태와 입력을 확인하고 TRELLIS 닭 생성부터 진행한다. 정본 이미지를 직접 보고 기존 원본/열린 Blender 상태를 확인한다.
2. 새 작업 경로에 외형·크기 가이드, 접힌 본체와 탈 내부 공간을 만들고 정면·측면·3/4로 자체 대조한다.
3. 본체와 탈 리그를 만든다. 손으로 잡기, 몸 지탱하기, 옷 낙하의 짧은 시험을 먼저 확인한다.
4. 주요 포즈를 만든다: 닭 → 전조 → 상단 벌어짐 → 한 손 접지 → 탈 이탈 → 바닥 안착 → 회수 → 펼치기 → 진입 → 마지막 팔 → 여밈 → 닭.
5. 큰 동작과 접촉을 먼저 통과시키고, 움직임의 가감속·몸통 지연·관절의 호를 다듬는다.
6. 천·볏·날개·실밥의 잔움직임을 더한다. 필요 시 캐시를 베이크하고 재오픈하여 같은 결과인지 확인한다.
7. 아래 검수 영상을 실제로 렌더하고 읽어 본다. 실패 구간을 수정한 뒤 최종 MP4와 원본을 납품한다.

단계별 중간 파일을 저장하고 계속 진행한다. 계획이나 모델 한 장을 보여주고 요청된 애니메이션 제작을 끝내지 않는다.
통상적인 리그·제작 방식 선택은 스스로 진행한다. 최종 결과를 만들 수 없는 실제 장애가 있으면 해결 시도와 남은 문제를 구체적으로 기록한다.

## 7. 자연스러움 합격 기준

- [ ] 첨부의 밝고 긴 단순 얼굴과 마른 긴 본체가 유지된다. v3/v4의 이빨 얼굴이 섞이지 않는다.
- [ ] 접힌 본체가 탈 내부에 들어간다. 전역 축소, 팔다리 소실, 갑작스러운 Scale 변화로 숨기지 않는다.
- [ ] 봉제선이 위로 열리는 방향과 본체가 위로 빠져나오는 과정이 보인다.
- [ ] 마지막 천이 본체에서 풀린 다음 떨어진다. 탈이 공중에서 먼저 납작해지지 않는다.
- [ ] 바닥에 닿은 천이 퍼지고 멈춘다. 통째로 떠 있거나 바닥을 크게 관통하지 않는다.
- [ ] 완료 자세에서 옷은 땅에 있고 본체와 분리되어 있다. 다시 입을 때 그 위치에서 회수한다.
- [ ] 손이 닿기 전 옷이 따라오지 않는다. 잡는 동안 천이 손을 관통하거나 떨어져 움직이지 않는다.
- [ ] 손·발의 지지 구간에 미끄러짐이 없다. 팔꿈치·무릎이 순간 뒤집히지 않는다.
- [ ] 큰 중심 이동 전에 골반·어깨가 준비하고, 지지점을 바꾸며 실제로 무게를 옮긴다.
- [ ] 입구를 잡는 손과 몸을 넣는 순서가 성립한다. 두 손을 먼저 숨긴 뒤 옷이 저절로 올라가지 않는다.
- [ ] 마지막 손이 숨기 전에 여밈을 마칠 수 있다. 닫히는 천이 얼굴·팔을 관통하지 않는다.
- [ ] 옷의 큰 접힘과 처짐이 있고 장식 무게가 읽힌다. 단단한 파츠 회전/전체 z축 압축만으로 납품하지 않는다.
- [ ] 1배속에서 동작 목적이 읽히고, 0.5배속/프레임 단위로도 제약 전환의 튐이 없다.
- [ ] 정면 3/4와 측면 둘 다 통과한다. 카메라 컷·암전·모션블러로 관통을 숨기지 않는다.
- [ ] 첫 닭 상태와 마지막 닭 상태의 형태·위치 연결이 자연스럽다. 끊어진 실밥은 의도한 차이로 기록한다.

손-천 접촉, 지지 손·발, 낙하 안착, 마지막 팔이 들어가는 구간은 프레임 범위와 실패/통과 근거를 QA에 남긴다.
가능하면 접촉점 거리와 바닥 침범량도 측정한다. 측정 허용치는 모델 크기를 보고 정해 기록하며 임의 숫자를 사용자 확정값으로 적지 않는다.
렌더 프로세스 성공만으로 위 체크를 완료 처리하지 않는다.

### 검수 파일에 남길 수치와 해석

| 검사 | 남길 증거 | 주의 |
|---|---|---|
| 손-천 접촉 | 좌우 잡기 구간·손가락 패드/천 표면 거리·최대값/발생 프레임 | IK 목표점이나 본 끝 거리만으로 실제 접촉을 인증하지 않음 |
| 지지점 | 손바닥/발바닥 world 위치의 지지 구간 변위와 영상 | 손목만 고정되고 손가락이 미끄러지는 경우도 확인 |
| 바닥 침범 | 바닥의 실제 z, 메시 최저점·최대 침범 프레임 | 무대와 기준 바닥을 달리 놓아 관통을 숨기지 않음 |
| 내부 수납 | 단면/측면·본 길이·전체 scale curve·본체/탈 간섭 | 본체를 축소하거나 render visibility로 숨기지 않음 |
| 메시/웨이트 | 미할당 정점, 가중치 합/영향 본 수, 열린 경계의 의도 | 생성 결과의 조밀함이 변형 품질을 보장하지 않음 |
| 공간 전환 | 몸→월드→손→몸 전환 전후 변위·회전, 전후 연속 프레임 | constraint on/off 프레임의 pop 확인 |
| 충돌 | 검사 메시 목록·간격·예외·최대 실패 프레임 | 손가락/얼굴/장식/자기충돌 누락을 명시 |
| 영상 | 파일별 fps·크기·길이·프레임 수·전체 디코딩 결과 | 실제 MP4에서 주요 접촉 프레임을 다시 열어 확인 |

허용 오차는 실측 크기에 맞춰 선정한 **제작 기준**으로 기록한다. 옛 파일의 “교차 0”을 복사하지 않는다.
수치 검사와 시각 검수는 각각 결과를 남긴다. 검사에서 제외된 면/부품이 있으면 QA에 명시한다.
`pass / fail / not_run`을 구분하고, 렌더 성공만으로 자연스러움 항목을 pass로 바꾸지 않는다.

## 8. 결과물과 경로

TRELLIS 제작 결과는 ArtSource/monster/chicken_disguise/animation_trellis_v1/에 이미 있다. 원본 Blender 파일과 MP4들은 유지하고 자동 덮어쓰지 않는다. 품질은 최종 합격이 아니며, 정확한 실패 항목은 해당 qa.md와 manifest.json을 따른다.

Higgsfield 집기 시험은 별도 ArtSource/monster/chicken_disguise/animation_higgsfield_pickup_v1/에 있다. 이 폴더는 기존 TRELLIS Blender 파일의 별도 복사본으로 시작했고, 플러그인 생성 모션과 리타게팅은 보존되어 있다. `preview/higgsfield_chicken_pickup.mp4`는 1280×720·20fps·4.5초이며 전체 디코딩은 통과했지만, 몬스터가 탈을 들어 올리지 않아 픽업 애니메이션으로는 불합격이다. 실제 비용·작업 ID·검수 내용은 폴더의 `manifest.json`과 위 링크를 따른다.
이미 작업이 있다면 상태를 읽고 이어서 진행하며 자동 덮어쓰기하지 않는다.

- `chicken_disguise_trellis_animation.blend`: 수정 가능한 모델·리그·Action·검수 카메라. 텍스처/캐시 누락 없음.
- `checkpoints/`: 단계별 편집 원본. 기존 파일을 자동 재생성으로 덮지 않는다.
- `textures/` 및 필요 시 `cache/`: 패킹하지 못한 텍스처·베이크된 천 의존성. 원본을 다시 열어 누락을 확인한다.
- `preview/take_off.mp4`: 벗기 전체. 시작과 완료 자세 유지 포함.
- `preview/put_on.mp4`: 회수부터 재착용까지. 별도 동작.
- `preview/off_on_review.mp4`: 약 25~35초 전체 프리뷰. 기본 목표 30초, 실제 길이 기록.
- `preview/side_contact_review.mp4`: 측면 고정 카메라로 접촉과 내부 진입을 검수할 수 있는 영상.
- `preview/pose_sheet.png`: 주요 포즈의 실제 Blender 렌더.
- `manifest.json`: 기준 이미지 경로/해시, 크기, 뼈·메시 목록, Action/프레임 범위, fps, Blender 버전, 캐시/외부 의존성, 실제 검증 결과.
- `qa.md`: 통과 항목, 실패 프레임과 수정, 남은 한계, 실행하지 않은 검수.
- `contact_audit.json`, `mesh_qa.json`, `video_readback.json`: 새 모델에 대해 실제 실행한 수치·영상 검사. 미실행이면 미실행으로 기록한다.
- Higgsfield 집기 시험은 완료 상태다. `animation_higgsfield_pickup_v1/manifest.json`에 입력 프롬프트·해시·생성 ID·크레딧·렌더·검수 결과를 기록한다. 별도 영상은 전체 디코딩을 통과했지만 탈이 들리지 않아 동작은 불합격이다. 이미 허용된 1회 생성은 사용했으므로, 추가 크레딧이 드는 생성은 사용자의 새 요청 전에는 하지 않는다. 무비용 수동 보정은 가능하나 AI 원본과 구분해 저장한다.

manifest에는 단계별 상태·입력 해시·생성 설정·원시/작업 메시 구분·리그와 Action 이름·단위/축·가정 치수·
제약/드라이버/캐시 의존성·접촉 이벤트 프레임·영상별 실제 길이·실행 명령·다음 작업을 기록한다.
옛 manifest/QA 숫자를 새 결과에 복사하지 않는다.

기본 30fps. 먼저 저해상도로 검수하고 최종은 1920×1080 H.264 MP4를 목표로 한다.
렌더 비용으로 해상도를 낮춰야 하면 실제 해상도를 명시한다. 완성되지 않은 540p 초안을 최종 1080p라고 하지 않는다.
최종 MP4를 다시 열어 길이·프레임·해상도를 확인하고, 인코딩된 영상에서 접촉/전환 구간을 추출해 직접 확인한다.
무음으로 납품해도 된다. 효과음·공포 조명 추가보다 형태와 접촉을 우선한다.

이 세션에서 FBX/UE 연결까지 임의로 확대하지 않는다. 엔진용 뼈 베이크와 천 변형 전달이 남았으면 명확히 적는다.
나중에 내보내기 쉽도록 본체/탈의 Action·기준 좌표·이벤트는 정리한다.
`SeamRelease`, `SuitDetached`, `SuitGrounded`, `GripL/R`, `BodyInside`, `SuitClosed` 등은 제작용 마커이며 구현된 게임 이벤트가 아니다.

## 9. 환경·보존·참조 문서

- 기존 사용자 Blender 파일과 `animatic_v1`~`v4`, `animation_natural_v1`, `model_fullbody_v1`, TRELLIS 원시 결과를 보존한다. 실행 중인 다른 작업의 씬을 초기화하지 않는다.
- 별도 background Blender 프로세스나 작업 전용 파일을 쓴다. 이전 확인 경로는 `C:/Program Files/Blender Foundation/Blender 5.2/blender.exe`; 새 세션에서 존재·버전을 다시 확인한다.
- 좌표·단위·root·리깅 전 변환 적용은 [공통 규격](../common.md)을 따른다. 생성 GLB의 방향을 재확인한다.
- Blender 실행/렌더 결과만 보고하고, UE나 다른 게임 런타임을 실행하지 않았다면 그 검증을 주장하지 않는다.
- 읽을 문서: `docs/asset/README.md`, `common.md`, `workflow.md`, `monster/README.md`, `chicken_disguise-animatic.md`.
- 플랫폼 연결이 필요해지면 현재 프로젝트 파일과 규격을 재확인한다. 이번 독립 Blender 제작에 기존 게임 데이터나 플랫폼을 임의로 바꾸는 작업은 포함하지 않는다.

**완료 보고는 사용자가 바로 볼 수 있는 영상, Blender 원본, 실제 검수 결과와 남은 한계를 중심으로 한다.**

## 10. 인계·실행 시작문

현재 상태를 이어받는 세션은 먼저 이 문서와 연결된 에셋 규격·TRELLIS 도구 지침, animation_trellis_v1/manifest.json 및 qa.md를 확인한다. 기존 TRELLIS 생성·리깅·영상은 이미 존재하며 자동으로 다시 만들거나 덮어쓰지 않는다.

Higgsfield 1회 시험은 완료됐다. 연결된 플러그인 1.5.57로 `ardy-core` 모션을 한 번 생성했고, 19개 본에 리타게팅해 별도 Blender 파일과 4.5초 영상을 저장했다. 예상·실제 비용은 0.1크레딧, 잔액은 10에서 9.9로 바뀌었다. 작업 ID·프롬프트·해시·영상 검수는 `animation_higgsfield_pickup_v1/manifest.json`에 있다.

전체 영상 디코딩은 통과했지만 시각 검수에서 몬스터가 탈을 들어 올리지 못하고 멀어지는 실패가 확인됐다. 이 결과를 픽업 애니메이션 승인본으로 사용하지 않는다. 이미 1회 생성을 사용했으므로 추가 크레딧 생성은 사용자 요청 전까지 보류한다. 이어갈 때는 기존 시험 파일과 결과를 보존하고, 무비용 수동 접촉 보정 또는 별도 생성 요청을 결정한다. 사용자의 Blender 씬은 이 작업에서 변경·저장하지 않았고 UE 검증도 수행하지 않았다.


## 11. 외부 라이브러리 없는 수동 재착용 검수본 — 2026-10-12

사용자의 “다른 라이브러리 쓰지 말고 직접 작업” 요청으로, 이 채팅에서 만든 `model_fullbody_v1`의 닭·몬스터를 사용해 별도 리그와 동작을 제작했다. 외부 모델/모션 라이브러리, TRELLIS 또는 Higgsfield 모션은 이 버전에 사용하지 않았다. 기존 다른 제작 폴더는 보존했다.

범위는 바닥에 놓인 닭 탈을 직접 집어 다시 입는 30초 동작이다. **현재는 동작 검수본이며 자연스러운 최종본으로는 불합격**이다. 900프레임 전체 검사에서 71프레임에 본체–탈 본면 교차가 남았고, 측면 영상에서 천의 처짐이 부족하다. 본 길이·스케일/가시성 채널·발바닥 지지 변위·MP4 전체 디코딩은 별도로 측정했다.

- [수동 Blender 원본](../../../ArtSource/monster/chicken_disguise/animation_handmade_pickup_v1/chicken_handmade_pickup.blend)
- [30초 재착용 영상 · 1080p/30fps](../../../ArtSource/monster/chicken_disguise/animation_handmade_pickup_v1/preview/pickup_put_on.mp4)
- [30초 측면 영상 · 540p/30fps](../../../ArtSource/monster/chicken_disguise/animation_handmade_pickup_v1/preview/side_contact_review.mp4)
- [실측 검수와 남은 문제](../../../ArtSource/monster/chicken_disguise/animation_handmade_pickup_v1/qa.md)
- [파일·출처·단계 기록](../../../ArtSource/monster/chicken_disguise/animation_handmade_pickup_v1/manifest.json)

다음 수정 대상은 손·팔·발목이 개구부를 통과하는 경로와 천의 처짐이다. 보정 키를 늘리는 것만으로 합격 처리하지 않는다. 현재 검수본을 자동 재생성으로 덮지 않고 별도 후속 버전에서 수정한다. FBX/Unreal 적용은 수행하지 않았다.


## 12. 재착용 후 둥근 닭으로 작아지는 후보 — 2026-10-12

기존 TRELLIS 재착용 영상은 큰 탈 모양에서 끝나 사용자가 요청한 “입은 뒤 작고 둥근 귀여운 닭”이 되지 않았다. 원본 animation_trellis_v1 및 v5 모델 초안은 보존하고, 별도 후보 ArtSource/monster/chicken_disguise/animation_trellis_v1/round_chicken_finish_v1/에서 재착용 엔딩을 수정했다.

재착용 후반 scene frame 850~877에서 기존 착용 형태를 바닥 기준으로 약 0.52배까지 줄인다. frame 878부터 TRELLIS v5 둥근 닭 후보 메시를 보이고, 작은 바운스로 안착한다. 최종 메시 크기는 약 1.325×1.164×1.404m, scale 0.53이다. 기존 동작과 둥근 새 메시가 다른 토폴로지라 연속 메시 모프가 아니라 축소 후 메시 전환이다.

- [둥근 닭 엔딩 재착용 영상 · 19초/1080p/30fps](../../../ArtSource/monster/chicken_disguise/animation_trellis_v1/round_chicken_finish_v1/preview/put_on_round_finish.mp4)
- [벗기부터 둥근 닭 엔딩까지 · 30초/1080p/30fps](../../../ArtSource/monster/chicken_disguise/animation_trellis_v1/round_chicken_finish_v1/preview/off_on_round_finish.mp4)
- [최종 프레임](../../../ArtSource/monster/chicken_disguise/animation_trellis_v1/round_chicken_finish_v1/preview/transition_0900.png)
- [수정 가능한 Blender 원본](../../../ArtSource/monster/chicken_disguise/animation_trellis_v1/round_chicken_finish_v1/chicken_disguise_round_finish.blend)
- [이번 후보 QA](../../../ArtSource/monster/chicken_disguise/animation_trellis_v1/round_chicken_finish_v1/qa.md) · [manifest](../../../ArtSource/monster/chicken_disguise/animation_trellis_v1/round_chicken_finish_v1/manifest.json)

두 영상은 전체 디코딩과 저장된 Blender 씬 재오픈 검사를 통과했다. 사용자는 이 후보를 Lvl_Flat에 프리뷰 배치하는 것을 승인했다. 승인 범위는 정지 메시 프리뷰이며, 게임용 최종 모델 승인이나 Blender 애니메이션의 Unreal 재생 승인은 아니다. frame 850 이전의 본체-탈 및 손가락-탈 교차 문제는 그대로 남는다. 이번 작업에 Higgsfield 추가 생성/크레딧 사용은 없다. Unreal 배치 상세는 §13을 본다.


## 13. Unreal Lvl_Flat 정지 메시 프리뷰 — 2026-10-12

사용자 요청에 따라 둥근 닭의 최종 메시만 FBX로 내보내 `/Game/Monsters/ChickenA/RoundFinishPreview/SM_Chicken_RoundFinish_Candidate`에 Static Mesh로 임포트하고 `/Game/Maps/Lvl_Flat`에 배치했다. 기존 `SK_Chicken_A`와 `BP_CL9Monster`는 교체하지 않았다. Blender의 착용 애니메이션은 영상 프리뷰로 유지되며, 레벨 안에서는 재생되지 않는다.

- [언리얼 임포트용 FBX](../../../ArtSource/monster/chicken_disguise/animation_trellis_v1/round_chicken_finish_v1/unreal_preview/SM_Chicken_RoundFinish_Candidate.fbx)
- [Base Color 텍스처](../../../ArtSource/monster/chicken_disguise/animation_trellis_v1/round_chicken_finish_v1/unreal_preview/T_CHICKEN_RoundFinish_BaseColor.png)
- [Metallic/Roughness 텍스처](../../../ArtSource/monster/chicken_disguise/animation_trellis_v1/round_chicken_finish_v1/unreal_preview/T_CHICKEN_RoundFinish_MRS.png)
- Unreal asset: `/Game/Monsters/ChickenA/RoundFinishPreview/SM_Chicken_RoundFinish_Candidate`
- Level actor label: `PREVIEW_Chicken_RoundFinish_Candidate`
- Transform: location `(1500, 0, -1.5) cm`, yaw `180°`, scale `(1,1,1)`. Collision is disabled for this preview actor.
- Imported bounds: about `132.46 × 116.40 × 140.44 cm`.

`Lvl_Flat` was saved and read back: the actor references the imported Static Mesh and the map is clean. The editor viewport was opened on this map and framed on the chicken from a front three-quarter angle. This verifies level placement and display only; it does not verify a playable in-level put-on sequence, rigging, or game-ready optimization.


## 14. 외부 라이브러리 없는 벗기·재착용 v2 전체 출력 — 2026-10-12

손·팔·발목 관통과 천 처짐을 모두 수정해 벗기와 입기를 완성하라는 후속 요청으로 `animation_handmade_off_on_v2`에서 직접 모델·40본 리그·900프레임 동작·천 변형을 제작했다. 이 버전은 이 채팅의 `model_fullbody_v1`을 사용하며 외부 모델/모션 라이브러리·TRELLIS·Higgsfield 모션을 사용하지 않았다. 기존 제작 폴더와 사용자의 열린 Blender 씬은 보존했다.

**파일 출력은 완료했으나, 사용자가 요청한 자연스럽고 관통 없는 완성 품질은 달성하지 못했다. 작업 품질 상태는 미완료/불합격이다.** 최종 저장본 900프레임에서 몸–옷 교차 542프레임, 변경되지 않은 최종 본체의 비인접 자기 교차 236프레임, 실제 검지·엄지 피부–천 거리 15mm 초과 13/802 손-프레임 표본이 남았다. 국소 최근접 법선 기준 깊이의 최대 검출값은 프레임 853에서 약 0.246m이며, 최대 침범 부피 측정값은 아니다. 지지 손은 실제 피부 기준으로 바닥에 닿지 않는 구간이 있고, 입기 중 하단 천의 늘어짐과 팔/봉제선 간섭이 실제 인코딩 영상에서도 확인됐다. 렌더 성공을 품질 합격으로 바꾸지 않았다.

- [편집 가능한 Blender 원본](../../../ArtSource/monster/chicken_disguise/animation_handmade_off_on_v2/chicken_handmade_off_on.blend)
- [전체 30초 · 1080p/30fps](../../../ArtSource/monster/chicken_disguise/animation_handmade_off_on_v2/preview/off_on_full.mp4)
- [벗기 12초 · 1080p/30fps](../../../ArtSource/monster/chicken_disguise/animation_handmade_off_on_v2/preview/take_off.mp4)
- [바닥 회수·입기 18초 · 1080p/30fps](../../../ArtSource/monster/chicken_disguise/animation_handmade_off_on_v2/preview/put_on.mp4)
- [측면 30초 · 540p/30fps](../../../ArtSource/monster/chicken_disguise/animation_handmade_off_on_v2/preview/side_contact_review.mp4)
- [실제 검수 결과](../../../ArtSource/monster/chicken_disguise/animation_handmade_off_on_v2/qa.md) · [파일/출처/의존성 기록](../../../ArtSource/monster/chicken_disguise/animation_handmade_off_on_v2/manifest.json)

네 영상은 각각 900/360/540/900프레임 전체 디코딩을 통과했다. 분리 영상의 11.97초·30.08fps 시간 정보 오류는 명시적 30fps CFR 인코딩으로 수정했고 12/18초로 재검사했다. `Unknown process id 27123`는 이미 종료된 로그 세션 조회 오류였으며 Blender 저장은 정상 종료 로그로 확인했다. 별개로 발 부분의 프레임 간 1.824m 튐을 발견해 안정된 다리 경로로 다시 만들고 천의 시간 방향 변형을 필터링했다. 발 형태 키 정점의 최대 프레임 간 이동은 약 0.0763m로 줄었지만, 자연스러움 합격을 뜻하지 않는다.

원본은 기준 이미지를 패킹했고 외부 PC2 없이 재생된다. 천은 같은 토폴로지의 900개 절대 형태 키와 `(frame-1)*10` 드라이버로 편집하며, 몸의 포즈를 바꾸면 자동 재시뮬레이션되지는 않는다. 본체 전역 스케일 변화나 가시성 전환으로 숨기지 않았다. 손·발 실제 피부의 바닥 접촉, 키 시간·가중치·의존성, 영상에서 추출한 3/4·측면 프레임 검수와 예외는 각 QA JSON에 남겼다. 별도 팔꿈치 우회 후보는 같은 180프레임에서 자기 교차가 49→61로 증가해 채택하지 않았다.

현재 방식의 보정만으로는 옷 입구와 팔·다리 진입 경로, 지지 손 접지, 하단 천 구조가 함께 성립하지 않는다. 해당 실패 프레임의 의복 구조와 동작 경로를 다시 맞춰야 하며, 이 출력본을 최종 승인본으로 사용하지 않는다. FBX/Unreal 검증은 수행하지 않았다.


## 15. 이번 세션 괴물 원형 + 옆 세션 진입 동작의 Unreal 연결 — 2026-10-12

사용자 요청: 괴물은 model_fullbody_v1/monster 원형을 유지하고, 세션 01a12a8e-3c7b-75f0-8b1d-fa5680c10462의 실제 들어가는 동작으로 Lvl_Flat에서 입고 벗기를 반복한다. 직전 두 Static Mesh의 위치·Scale 보간은 이 요구의 완성본이 아니다.

**연결과 인게임 반복 재생 검증 완료, 자연스러움 최종 승인 대기.** 이번 세션 괴물의 원형 메시를 옆 세션의 40본 리그에 연결했다. 새 컨셉이나 옆 세션의 간소화된 괴물 본체로 교체하지 않았다. 외부 모션 라이브러리/Higgsfield 새 생성은 없으며 이번 연결 작업의 신규 서비스 크레딧은 0이다.

- 원형: model_fullbody_v1/monster/monster_fullbody.blend. 동작/천: animation_handmade_off_on_v2/chicken_handmade_off_on.blend. 두 원본 파일 수정 없음.
- [통합 Blender 편집 원본](../../../ArtSource/monster/chicken_disguise/animation_session_ingame_v1/chicken_session_original_motion.blend)
- [이번 괴물의 들어가는 18초 Blender 영상](../../../ArtSource/monster/chicken_disguise/animation_session_ingame_v1/preview/put_on_original_monster.mp4)
- [Lvl_Flat 입기·벗기 반복 34초 인게임 영상](../../../ArtSource/monster/chicken_disguise/animation_session_ingame_v1/preview/LV_Flat_ActualEnterRemoveLoop.mp4)
- [출처·설정 manifest](../../../ArtSource/monster/chicken_disguise/animation_session_ingame_v1/manifest.json), [검수와 남은 문제](../../../ArtSource/monster/chicken_disguise/animation_session_ingame_v1/qa.md)
- [독립 원형/본 검사](../../../ArtSource/monster/chicken_disguise/animation_session_ingame_v1/qa/independent_mesh_motion_readback.json), [실시간 서버·클라이언트 표본](../../../ArtSource/monster/chicken_disguise/animation_session_ingame_v1/qa/pie_samples.json), [게임 영상 디코딩](../../../ArtSource/monster/chicken_disguise/animation_session_ingame_v1/qa/unreal_video_readback.json)

독립 Blender 재읽기에서 13개 원형 부품/35,228 정점의 원형 좌표 오차 0m와 면 구성 일치를 확인했다. 16개 표본 프레임의 본 Scale 오차는 0이다. 원래 전체 동작은 벗기 F1–360 + 재착용 F361–900이며, 재착용은 역재생이 아닌 별도 저작 동작이다.

### 엔진에서 열고 확인할 위치

- 레벨: /Game/Maps/Lvl_Flat
- 캐시: /Game/Monsters/ChickenA/SessionCurrent/ActualMotion/GC_ChickenDisguise_SessionOriginal
- 시퀀스: /Game/Monsters/ChickenA/SessionCurrent/ActualMotion/LS_ChickenDisguise_AuthoredMotionLoop
- 변형 메시 액터: PREVIEW_SessionMonster_ActualEnterLoop, 위치 (1500,0,0)cm, 회전 pitch=0/yaw=180/roll=0, 배치 Scale (0.5,0.5,0.5) 고정
- 자동 재생 액터: SEQ_ChickenDisguise_ActualMotion_Auto. auto_play=true, loop_count=-1. Play 시 진입 18초 → 벗기 12초 → 다음 진입 순서로 반복한다.
- 검수 카메라: PREVIEW_SessionMotion_Camera, 위치 (1200,-250,180)cm, pitch=-15/yaw=45, FOV65. 게임의 시작 연출이 카메라를 먼저 만들 수 있으므로, 그 이후 PIE의 각 로컬 컨트롤러 ViewTarget을 이 카메라로 전환한다. 일반 플레이 카메라를 영구 교체하지 않았다.

기존 정적 프리뷰 두 개는 보존하되 숨기고, 기존 SEQ_ChickenDisguise_InGameLoop_Auto의 auto_play를 껐다. 새 캐시는 실제 몸/천 변형을 보존하는 GeometryCache 프리뷰이며, 메시 전체의 Scale이나 가시성 변경으로 본체를 숨기지 않는다. 편집 가능한 스켈레탈 리그와 동작은 Blender 원본에 남는다. 캐시는 약 1.53GB로 최적화되지 않았으며 몬스터 AI·전투·AnimBP 연결 완료로 취급하지 않는다. Unreal은 10개 기본 색 재질로 연결했고 Blender 절차적 디테일은 베이크하지 않았다.

### 재현 시 놓치기 쉬운 설정

1. 최종 Alembic은 export/GC_ChickenDisguise_SessionOriginal_UE.abc다. 얼굴의 n-gon만 삼각분할하고 기존 quad는 유지했다. 실패한 중간 Alembic 두 개는 삭제했고 최종 임포트 출처는 유지했다.
2. GeometryCacheComponent의 running=true와 manual_tick=true가 모두 필요하다. manual_tick만 켜면 시간은 바뀌어도 변형이 멈출 수 있다.
3. 시작 오프셋은 12초다. 시퀀스 표시 속도는 30fps지만 first_loop_start_frame_offset은 24,000Hz tick 단위이므로 **288,000 ticks**를 넣는다. 360을 넣으면 0.015초가 되어 벗기부터 시작한다.
4. Section.modify()를 먼저 호출한 뒤 params를 바꿔 컴파일된 시퀀스가 이전 값을 재사용하지 않게 한다. Rotator는 pitch/yaw/roll 키워드 인자를 사용한다.
5. 패키지와 현재 레벨만 저장한다. 공유 에디터를 다시 띄우거나 Save All로 다른 작업을 저장하지 않는다. 기존 출력은 확인 없이 덮어쓰지 않는다.

34초 MP4는 실제 PIE 게임 창을 프레임마다 캡처한 1280×720/30fps/1,020프레임 영상이다. 무관한 라운드 전환을 막기 위해 월드 시간은 정지하고 시퀀스를 프레임별 평가했다. 자동 재생 자체는 별도의 라이브 PIE 서버·클라이언트 표본에서 반복 시간 변화와 고정 배치 Scale을 확인했다. 프레임 캡처 영상을 실시간 성능 측정으로 사용하지 않는다. 전체 디코딩과 9개 주요 시점 시각 검수를 통과했다. 검수 후 PIE는 종료했으며 자동 재생 설정은 레벨에 저장돼 있다.

천 늘어짐, 팔/여밈 간섭, 검은 틈, 최종 발목 노출은 남아 있다. 이번 원형 메시의 관통 전수 검사는 실행하지 않았으며 옆 세션의 실패 수치를 새 모델 검사 결과로 복사하지 않았다. 다음 품질 작업은 같은 Blender 통합 원본에서 천 여밈/팔·발 간섭을 수정한 뒤 재렌더하고, 사용자의 자연스러움 승인을 받는 것이다.

재현 스크립트: Tools/Blender/{integrate_session_chicken_motion,export_session_chicken_cache,render_session_chicken_motion,audit_session_chicken_motion,readback_session_chicken_motion}.py와 Tools/Unreal/{import_session_chicken_motion,place_session_chicken_motion,capture_session_chicken_motion,readback_session_chicken_capture}.py. 캡처 스크립트의 WINDOW_REF는 PIE 재시작 후 새 Slate Snapshot에서 확인해 갱신한다.


## 16. 애니메이션 보정 도구 실사용 평가 — 2026-10-12

§15 통합 원본의 430–490 구간을 별도 animation_tool_trials_20261012 폴더에서 시험했다([시험 기록](../tools/animation-tool-trials-20261012.md)). 기본 Cloth는 탈 형태 유지 실패, PPF는 초기 탈 교차로 계산 거부(별도 깨끗한 천 제어시험은 CUDA 실행 성공), Ragdoll 전신·팔 시험은 집기 접촉 유지 실패다. HiPhy 체험판은 동봉 약관의 상업 제품 개발·연구 금지 때문에 모델에 적용하지 않았다. 유료 구매나 원본 대체는 하지 않았다. 상세 재현·수치·한계는 docs/asset/tools/animation-tool-trials-20261012.md와 출력 qa.md에 기록했다. 우선 실제 손가락 집기점을 정하고 교차 없는 시뮬레이션용 탈을 만든 뒤 무료 PPF를 재평가한다. 전신 900프레임 품질 승인으로 취급하지 않는다.


## 17. 천·진입 경로·둥근 닭 마무리 — 2026-10-13

실제 Blender 3D 원본을 보정해 UE GeometryCache로 재생한 검수용 수정본이다. 동영상만 제작한 결과가 아니다. 기존 §15의 원본/캐시와 §16·§18의 별도 시험을 보존하고 animation_session_ingame_v2에 저장했다. 중간 관통이 모두 해결됐다는 최종 승인은 아니다.

[실제 Lvl_Flat 인게임 영상](../../../ArtSource/monster/chicken_disguise/animation_session_ingame_v2/preview/LV_Flat_PolishedEnterRemoveLoop.mp4), [Blender 원본](../../../ArtSource/monster/chicken_disguise/animation_session_ingame_v2/chicken_session_polished.blend), [QA](../../../ArtSource/monster/chicken_disguise/animation_session_ingame_v2/qa.md), [manifest](../../../ArtSource/monster/chicken_disguise/animation_session_ingame_v2/manifest.json).

원형 13부품/35,228정점의 좌표·면 구성과 원본 SHA256을 유지했다. 40본 고정 길이 IK로 팔·다리를 접고 넓힌 목 입구를 거쳐 들어가도록 수정했다. 천을 동일 토폴로지로 재단하고 목·등 여밈 및 장식 부착을 보정했다. 본체 축소/가시성 전환으로 숨기지 않는다. 벗기 F1–360(12초), 입기 F361–960(20초), 30fps/32초다.

완전 착용 F935–960의 26프레임은 몸–천 교차와 닫힌 설계 외피 밖 정점 모두 0이며 앞·옆·뒤에서 둥근 닭만 보인다. 전체 960프레임 중 445프레임에는 중간 몸–천 교차가 검출된다. 손가락·팔 전환과 발–옷 접합부의 가려진 접촉/보이는 관통을 모두 분류한 결과가 아니므로 관통 0으로 기록하지 않는다. 본체 자기 교차와 손 스킨–천 거리 전수 검사는 별도 미완료다. §18 PPF 시험의 수치를 이 버전에 전용하지 않는다.

UE 에셋은 /Game/Monsters/ChickenA/SessionCurrent/ActualMotionPolished/ 아래 GC_ChickenDisguise_Polished와 LS_ChickenDisguise_PolishedLoop다. Lvl_Flat의 PREVIEW_SessionMonster_ActualEnterLoop 및 SEQ_ChickenDisguise_ActualMotion_Auto가 입기부터 시작해 32초를 무한 반복한다. 캐시 패키지 약 1.44GB/9개 단순 재질의 검수용 변형 캐시이며 실전 SkeletalMesh/AnimBP·AI·네트워크 동기화·성능 최적화 완료가 아니다.

인게임 MP4는 36초/1280×720/30fps/1,080프레임 전부 디코딩 통과했다. 실제 PIE 창을 프레임 이동하며 캡처했으며 실시간 성능 측정 영상이 아니다. 별도 35초 자동 재생 표본에서 캐시 변형과 32초 루프 복귀를 확인했다. 영상 근거는 qa/unreal_video_readback.json, 자동 재생은 qa/pie_samples.json이다.

사용자 바닥 문의 후 로그에 10-13 00:34경 바닥 및 프리뷰 액터 삭제가 있었음을 확인했다. 자동 저장본의 Floor_0만 복사해 원래 바닥 크기·위치·재질·충돌을 복구했고 맵 전체는 되돌리지 않았다. 실제 PIE 네 지점 바닥 충돌 및 캐릭터 접지를 확인했다. 프리뷰도 원래 방향으로 재연결 후 8초 동안 재생/메시 변형/지면 위 높이를 재검증했다. 복구 근거는 v2 qa/floor_restore*.json 및 qa/preview_restore.json·restore_runtime_verified.json이다. 맵 저장 완료, 검수 PIE 종료.

이번 수정은 기존 수작업 동작을 Blender에서 로컬 보정했으며 신규 Higgsfield 생성/유료 호출/외부 서비스 생성 크레딧은 0이다. 로컬 계산 비용은 측정하지 않았다. 후속 제작은 중간 손 집기·입구 통과의 보이는 관통 개선 및 최종 게임용 리그/천 방식 결정이다. docs/model을 새로 만들지 않는다.


## 18. 손 집기와 PPF 재시험 — 2026-10-12

사용자 “탈 겹치는거랑 손으로 잡는거 먼저 고치고 PPF로 다시” 요청에 따라 §15 원본의 F430–490 집어 올리기 구간을 별도 animation_ppf_grip_v4에서 다시 시험했다. §17의 전체 시퀀스 작업을 덮어쓰지 않는다.

[PPF 실제 2초 영상](../../../ArtSource/monster/chicken_disguise/animation_ppf_grip_v4/preview/ppf_grip_lift.mp4), [원본 왼쪽/PPF 오른쪽 비교](../../../ArtSource/monster/chicken_disguise/animation_ppf_grip_v4/preview/original_vs_ppf_grip.mp4), [Blender 시험 원본](../../../ArtSource/monster/chicken_disguise/animation_ppf_grip_v4/chicken_ppf_grip.blend), [QA](../../../ArtSource/monster/chicken_disguise/animation_ppf_grip_v4/qa.md), [manifest](../../../ArtSource/monster/chicken_disguise/animation_ppf_grip_v4/manifest.json).

손목·팔 고정 길이 IK, 실제 손 스킨 기반 엄지/검지 집기, 나머지 손가락 굽힘과 잡은 천 패치 회전을 수정했다. 교차 없는 693정점 열린 천 표면으로 PPF CUDA 계산이 약 110.16초에 완료됐다. 저장된 후보 재읽기를 포함한 61프레임 검사에서 천–몸과 비인접 천 자기 교차 0, 핀 최대 오차 0.0737mm, 손 스킨–핀 패치 중심 표면 최근접 거리 6.85–9.22mm를 확인했다. 영상 전체 디코딩과 실제 프레임/양손 확대 렌더를 열어 확인했다. 13개 원형 메시/35,228정점과 40본 리그를 보존했고 원본 SHA256은 그대로다.

**집기/깨끗한 천의 물리 시험은 통과했지만 닭 탈 최종 외형은 불합격이다.** 기존 9,415정점 탈을 그대로 보정한 것이 아니며 시험용 열린 천으로 교체해 둥근 봉제 볼륨을 잃었다. 날개/볏/한쪽 발–천 교차도 표본에서 남았다. 장식 밀어내기 v5는 부속이 최대 0.81m 떨어져 반려했다. 이 후보를 최종 착용 애니메이션이나 원본 대체로 사용하지 않는다.

다음 제작에는 승인된 둥근 외형을 유지하는 교차 없는 열린 3D 시뮬레이션 외피와 봉제 부속 결합이 필요하다. PPF 단독 자동 보정으로 해결됐다고 기록하지 않는다. 이번 작업의 유료 결제·외부 모델 업로드·열린 Blender/UE 수정은 없다.

## 19. 둥근 탈 유지·팔다리 관통 보정·3배속 PIE 영상 — 2026-10-13

§17의 둥근 탈을 유지한 별도 animation_round_ingame_3x_v1 후보를 만들어 Lvl_Flat의 기존 변형 프리뷰에 적용했다. 30개 EXPORT 메시의 원형 좌표·면 구성 해시는 기준 v2와 같고 40본 Scale은 1이다. 고정 길이 IK로 팔다리 경로와 손톱 네 구간을 보정했다. 원형 몸체 13개 메시와 두께 18mm 탈의 삼각형 교차는 전체 321프레임에서 0이다. 기준 원본 네 파일의 SHA256은 그대로다.

**부분 개선 및 실제 게임 적용·촬영 완료이며, 전체 겹침 해결이나 최종 품질 합격은 아니다.** 탈 자체 중간 표면의 비인접 교차는 170/321프레임에 남는다(봉제 여밈·하단 캡 포함, 접힌 천의 실제 교차도 존재). 바닥 층 분리·삼각형 분리·손가락 접촉 시험은 개선하지 못해 채택하지 않았다. 정밀한 집기 접촉도 남는다. 평평한 PPF 시험 외피는 사용하지 않았다.

- [실제 Lvl_Flat 3배속 영상 · 12.67초/720p/30fps](../../../ArtSource/monster/chicken_disguise/animation_round_ingame_3x_v1/preview/LV_Flat_RoundChicken_3x.mp4)
- [완전 착용 인게임 프레임](../../../ArtSource/monster/chicken_disguise/animation_round_ingame_3x_v1/preview/round_closed_ingame.png)
- [편집 가능한 Blender 원본](../../../ArtSource/monster/chicken_disguise/animation_round_ingame_3x_v1/chicken_round_3x.blend)
- [QA와 남은 문제](../../../ArtSource/monster/chicken_disguise/animation_round_ingame_3x_v1/qa.md) · [manifest](../../../ArtSource/monster/chicken_disguise/animation_round_ingame_3x_v1/manifest.json)

캐시: /Game/Monsters/ChickenA/SessionCurrent/ActualMotionRound3x/GC_ChickenDisguise_Round3xFinal. 시퀀스: 같은 폴더 LS_ChickenDisguise_Round3xLoop. 기존 프리뷰·자동 재생 액터에 연결하고 레벨을 저장했다. 원본 32초는 10.6667초로 3배 리타이밍됐고 입기 6.6667초→벗기 4초로 반복한다. 속도가 캐시에 반영돼 UE PlayRate는 1, 시작 오프셋 96,000tick(4초), 배치 Scale (0.5,0.5,0.5) 고정이다. 이전 v2 캐시와 시퀀스는 보존했다.

MCP StartPIE/StopPIE, 자동 재생 시간·반복 표본, 저장된 씬 재읽기, 전체 380프레임 디코딩과 9개 시점 검수를 완료했다. 영상은 실제 PIE 창을 월드 시간 정지 상태로 프레임별 캡처했다. 실시간 성능 측정은 아니다. 6.5초에 완전 착용된 둥근 닭을 확인했다. GeometryCache 프리뷰이며 몬스터 AI·전투·AnimBP용 최종 에셋은 아니다. UE 기본 재질 9개를 사용했고 Blender 절차적 디테일은 베이크하지 않았다. PIE 종료와 잠금 해제 완료. 신규 서비스 크레딧/유료 결제/외부 업로드는 0이다.


## 20. 완전히 가려진 뒤 작아지는 마무리 추가 — 2026-10-13

사용자 “숨겨지고나서 작아지는건 왜없음?” 정정에 따라 §19의 크기 고정 Scale 0.5에서 빠졌던 마지막 단계를 추가했다. 최신 기준은 animation_round_shrink_v1이다. §19의 둥근 외피/팔다리 경로/3배속 캐시와 기존 원본은 보존한다.

입기 6.6667초 → 몸이 완전히 가려진 큰 닭 → 1초간 60%로 축소 → 작은 닭 1초 유지 → 벗기 전 1초간 원래 크기로 복원 → 벗기 4초, 총 13.6667초/410프레임 반복이다. 축소 중 몸을 가시성 전환으로 삭제하지 않는다. 닫힌 탈과 내부 몸 전체에 균일 루트 배율을 적용하는 사용자 요청의 변신 연출이다. 원형 메시 비례와 본 Scale 1은 보존한다.

- [실제 인게임 영상 · 15.67초/720p/30fps, 6.67초부터 축소](../../../ArtSource/monster/chicken_disguise/animation_round_shrink_v1/preview/LV_Flat_ChickenShrinkLoop.mp4)
- [Blender 저작 원본](../../../ArtSource/monster/chicken_disguise/animation_round_shrink_v1/chicken_round_shrink.blend)
- [QA와 단계별 시간](../../../ArtSource/monster/chicken_disguise/animation_round_shrink_v1/qa.md) · [manifest](../../../ArtSource/monster/chicken_disguise/animation_round_shrink_v1/manifest.json)

UE는 ActualMotionRound3x의 기존 GC_ChickenDisguise_Round3xFinal 캐시를 재사용하고 새 LS_ChickenDisguise_Round3xShrinkLoop로 연결했다. 입기·닫힌 포즈 유지·벗기의 3구간과 Transform Scale XYZ 키를 쓴다. 닫힌 포즈 구간은 F200–290, 축소 F200–230, 작은 닭 유지 F230–260, 복원 F260–290이다. 배율은 0.5 → 0.3 → 0.5이며 입기 도중 노출된 괴물을 작게 만들지 않는다. Lvl_Flat의 기존 프리뷰/자동 반복 액터에 적용하고 레벨 및 새 시퀀스를 저장했다. 바닥은 유지했다.

Blender 원본은 기존 3배속 파일의 30개 메시 좌표/면 해시를 유지하고 EXPORT 루트 CTRL_ChickenDisguise_SizeFinish에 1 → 0.6 → 1 키를 넣었다. 1–411프레임/30fps, 입기는 F121부터다. 완전 착용 후 축소·복원의 7개 시점에서 몸–탈 삼각형 교차 0/본 Scale 오차 0, 균일 축소 정점 오차 최대 약 0.000000656m다.

실제 MCP StartPIE 자동 재생 16초에서 크기 변화와 410프레임 루프 복귀를 확인했다. 별도 F200/215/230/245/260/275/290 검증에서 Scale 0.5/0.4/0.3/0.3/0.3/0.4/0.5가 확인됐고, 이 구간 캐시는 마지막 완전 착용 상태였다. 영상 470프레임 전체 디코딩 및 9개 시점 시각 검수 완료, 실제 게임 창의 프레임 이동 캡처이며 실시간 성능 측정은 아니다. 검수 PIE 종료.

기존 접힌 천 자체의 겹침과 정밀 집기 문제는 별도로 남는다. GeometryCache 프리뷰이며 새 AnimBP/몬스터 AI/전투 통합을 만들었다는 의미는 아니다. 신규 유료 호출/외부 생성 크레딧/업로드는 0, 로컬 계산 비용은 미측정이다.


## 21. 원래 괴물과 작은 닭의 걸음 추가 — 2026-10-13

사용자 “이대로 몬스터와 다 움직인 좀 만들자”에 따라 승인된 괴물/둥근 닭 형상에 두 모습의 왕복 걸음을 붙였다. 최신 인게임 프리뷰 기준은 **animation_locomotion_v1**이다. §20의 완전히 가려진 뒤 축소는 유지하며, 원본 파일 SHA256과 메시 30개의 rest 좌표/면 해시를 보존한다.

- [두 걸음 실제 게임 영상 · 7.6초/720p/30fps](../../../ArtSource/monster/chicken_disguise/animation_locomotion_v1/preview/LV_Flat_ChickenMonster_Walks.mp4) — 앞 4초 닭, 뒤 괴물.
- [수정 가능한 Blender 원본](../../../ArtSource/monster/chicken_disguise/animation_locomotion_v1/chicken_monster_locomotion.blend)
- [시간표·검수·재개 방법](../../../ArtSource/monster/chicken_disguise/animation_locomotion_v1/qa.md) · [manifest](../../../ArtSource/monster/chicken_disguise/animation_locomotion_v1/manifest.json)

전체 반복은 입기 6.6667초 → 완전 착용 후 축소 1초 → 작은 닭 걸음 4초 → 원래 크기로 복원 1초 → 벗기 4초 → 괴물 걸음 3.2초 → 정리 자세 약 0.37초, 총 20.2333초/UE 607프레임이다. Blender 원본은 F1–608/30fps다. 영상은 새 걸음만 잘라 가까이 촬영했으며, Lvl_Flat에는 전체 반복이 저장되어 있다.

괴물은 기존 40본 리그의 두 관절 IK로 발 교대, 팔 반대 흔들림, 골반 움직임을 만든다. 닭은 발 아래쪽의 변형과 작은 몸통 흔들림으로 뒤뚱거린다. 한 방향 이동 거리는 괴물 151.2cm, 닭 48cm이고 돌아와 탈 위치와 다시 연결된다. 기준 속도는 괴물 126cm/s, 작은 닭 30cm/s다. 본 Scale 1, 팔다리 길이 유지, 검사한 걸음 시점의 몸–외피 교차 0을 확인했다.

UE 새 에셋은 /Game/Monsters/ChickenA/SessionCurrent/LocomotionV1의 GC_ChickenDisguise_Locomotion, LS_ChickenDisguise_LocomotionLoop이다. 기존 PREVIEW_SessionMonster_ActualEnterLoop/SEQ_ChickenDisguise_ActualMotion_Auto를 재사용하고 자동 재생·무한 반복을 저장했다. 바닥은 보존하고 국소 프리뷰 조명을 보강했다. 실제 MCP PIE 23초에서 루프 복귀, 별도 10개 시점에서 축소/걸음/복원 상태를 확인했으며 액터 Hidden은 false였다. 영상 228프레임 전체 디코딩과 12개 시점 시각 검수 완료. 게임 창의 프레임 이동 캡처이며 실시간 FPS 측정은 아니다.

재개는 [configure_chicken_locomotion_preview.py](../../../Tools/Unreal/configure_chicken_locomotion_preview.py)로 기존 캐시와 시퀀스를 다시 연결한다. 영상 촬영 때만 플레이어/HUD와 무관한 기존 동물들을 PIE 안에서 숨겼고 종료 후 복원된다. 이 시안의 괴물을 가시성 토글로 숨긴 것은 아니다.

현재 범위는 **게임 월드 GeometryCache/LevelSequence 프리뷰**다. chicken_a의 기존 AI/AnimBP/MonsterTable/멀티 상태를 새 모델로 교체하지 않았다. AI 연결 시 제자리 Idle/Walk/Run을 분리하고 실제 속도 ÷ 기준 속도로 재생해야 한다. 원샷 입고 벗기는 상태 진입 서버 시각에서 파생시키고, 애니메이션이 판정을 바꾸게 만들지 않는다. 기존 접힌 천 자체 겹침·정밀 손가락 집기 및 첫 걸음의 방향 전환/발 디딤 완급은 후속 다듬기 대상이다.

신규 유료 서비스 호출·생성 크레딧·외부 업로드 0. 로컬 전력/장비 비용은 미측정. 검수 PIE 종료. 기존 버전과 문서는 보존했고 docs/model은 만들지 않았다.

## 22. 현재 인게임 기준 탈착·등 형상 수정본 납품 — 2026-10-16

범위는 `production`이다. §21의 현재 인게임 프리뷰를 기준으로 닭의 주요 외형·완전히 가려진 뒤 축소·닭/괴물 걷기를 유지하고, 양방향 집기·팔/발 통과·천 처짐과 낙하·등 능선을 직접 수정했다. 기존 프로젝트 모델/40본 리그를 사용했고 외부 모델·모션 라이브러리를 추가하지 않았다. 새 출력의 사용자 품질 승인을 추정하지 않는다.

- [실제 UE PIE 전체 30초](../../../ArtSource/monster/chicken_disguise/animation_locomotion_contact_back_v2/preview/LV_Flat_ChickenContactBackV2.mp4)
- [입기·축소 9.667초](../../../ArtSource/monster/chicken_disguise/animation_locomotion_contact_back_v2/preview/LV_Flat_Chicken_Don_Shrink.mp4) · [벗기·낙하 6초](../../../ArtSource/monster/chicken_disguise/animation_locomotion_contact_back_v2/preview/LV_Flat_Chicken_Doff.mp4)
- [편집 가능한 Blender 정본](../../../ArtSource/monster/chicken_disguise/animation_locomotion_contact_back_v2/chicken_disguise_contact_back.blend) · [전체 최종 Alembic](../../../ArtSource/monster/chicken_disguise/animation_locomotion_contact_back_v2/export/GC_ChickenDisguise_Final.abc)
- [중립광 전체](../../../ArtSource/monster/chicken_disguise/animation_locomotion_contact_back_v2/preview/chicken_disguise_don_doff_1x.mp4) · [측면](../../../ArtSource/monster/chicken_disguise/animation_locomotion_contact_back_v2/preview/chicken_disguise_don_doff_side_1x.mp4)
- [닭 5컷 동작 도해](../../../ArtSource/monster/chicken_disguise/animation_locomotion_contact_back_v2/storyboard/chicken_doff_five_steps_v1.png) — ImageGen 설명 그림이며 실제 3D 검수 증거와 구분한다.
- [검수·범위·재개 기록](../../../ArtSource/monster/chicken_disguise/animation_locomotion_contact_back_v2/qa.md) · [manifest](../../../ArtSource/monster/chicken_disguise/animation_locomotion_contact_back_v2/manifest.json)

정본은 `chicken_disguise_contact_back.blend` 하나다. 같은 폴더의 `v2*`, `complete`, `routed`, `delivery*` 후보는 최종 납품본으로 선택하지 않는다. 원본 LocomotionV1 SHA256 `3d7c1e6cce33067696cc3456e2b65c87ed9940bf9dcfbeeb9dd559e33f8a15ef`는 그대로다. 모든 파일 이미지를 패킹했고 링크된 Blender 라이브러리는 없다.

실제 두께를 포함한 몸/손톱–탈 교차 0/728프레임, 노출 구간 손·팔–상아색 얼굴/머리 교차 0/341프레임을 확인했다. 집기는 실제 엄지/검지 피부를 기준으로 저작하고 314표본을 측정했다. 손을 놓은 뒤 빈 탈이 내려앉는 포즈를 새로 만들었으며 바닥 정지 선택 포즈의 천 자기 교차는 0이다. 등은 기존 피부 4,293정점의 넓은 능선/골로 보정했다. 내부 다리/부츠 공간은 수정했고 외부 닭 발바닥·축소 루트·기존 괴물 걷기는 수치 비교로 보존했다.

저장된 소비 경로는 **게임 월드 GeometryCache/LevelSequence 프리뷰**다. `/Game/Monsters/ChickenA/SessionCurrent/ContactBackV2/LS_Chicken_ContactBackV2_Final.LS_Chicken_ContactBackV2_Final`를 기존 `PREVIEW_SessionMonster_ActualEnterLoop`/`SEQ_ChickenDisguise_ActualMotion_Auto`에 연결했다. 전체 기본 캐시와 F458–484 보정 캐시를 3구간으로 연결하며 수정 구간 밖 데이터 동등성을 해시로 확인했다. 단일 최종 Alembic도 별도 납품한다. AI/AnimBP/MonsterTable/전투/네트워크 상태 교체를 완료했다는 뜻은 아니다.

레벨 저장 후 실제 재열기, 실제 MCP StartPIE/StopPIE, 27초 자동 재생·반복·보정 캐시 전환, 900프레임 촬영 및 전체 디코딩을 확인했다. 영상은 실제 PIE 창의 시간 정지/30fps 프레임 이동 캡처이며 실시간 성능 측정은 아니다. 원래 `Lvl_Henhouse`로 복원했고 dirty map/content/PIE 없음, 임시 FPS 제한 복원과 이 세션 잠금 해제를 완료했다.

**잔존:** 잡힌 옷깃·변형 중 접힘·뒤 봉합부의 천 자기 교차와 완전히 가려진 몸의 접힌 자세 자기 겹침은 일부 남아 있다. 표면 교차 0을 모든 천 자기 충돌/모든 소수 프레임/전신 자기 겹침의 해결로 확대하지 않는다. 자세한 원자료와 제작·복구 과정은 위 QA/manifest에 보존한다.
