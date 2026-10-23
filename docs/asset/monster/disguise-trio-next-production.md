# 다음 제작 — 저장고·찬장·벌집 주인의 탈 변신

<!-- owners-remaining-20261020 -->
## 2026-10-20 현재 — 찬장 변신·벌집 군집 탈착의 Native UE 검수본

- 사용자 지시: “이제 나머지 알아서 만들고 언리얼에 올려”. `scope: production`, `review_policy: user_directed`. 이 지시는 추가 외형 승인 대기 없이 제작을 이어가도록 허용하며, 새 결과를 사용자가 이미 봤거나 승인했다고 기록하지 않는다.
- 최신 소의 기존 Unreal 모델·AI/이동/공격과 다람쥐·거미의 기존 모델·이동/공격 출력은 재사용한다. 10-11의 간단한 v2 프리뷰로 되돌리지 않는다. 사용자 반려 후 중단한 소 Cloth Lab 실험은 채택하지 않는다.
- 이번 남은 대상: 찬장 주인 animation_v02의 탈 변신을 엔진에서 재생할 수 있도록 준비, 벌집 주인의 모델·종별 리그·탈의/회수와 재착용·비행 동작 및 Unreal 에셋/배치 검수. 벌집은 현재 story.md의 여러 마리로 흩어짐을 반영한다. 기존 한 마리의 집게턱·큰 배·막날개 방향을 작은 군집 개체에 적용한다.
- 산출물: `ArtSource/monster/owners_remaining_20261020/` 및 에셋별 새 파생 폴더, 편집 원본·FBX·manifest·정상 속도 검수 영상. 엔진은 별도 에셋 폴더/검수 맵으로 준비한다. 신규 군집 AI·등반 AI·전투 밸런스 구현까지 임의 확대하지 않는다.
- 현재 `status: native_model_animation_preview_delivered`, `runtime.level: preview`. SkeletalMesh·Skeleton·AnimSequence·재생용 Blueprint와 별도 맵을 저장/재열기/PIE 재생까지 확인했다. 신규 군집/등반 AI·전투 판정 연결이나 최종 성능 검수 완료를 뜻하지 않는다. 공유 에디터 잠금은 해제했다.
- **현재 원본:** `Pantry/v02/Pantry_{Doff,WornIdle,RevealedIdle}.blend`, `Hive/v03/HiveSwarm_authoring.blend`. 찬장은 기존 animation_v02의 같은 탈·변신/성장 경로와 native Spider 본체를 이어 썼다. 본체 스켈레탈+탈 모프의 두 재질 슬롯이며 기존 원본 두 파일의 SHA256 보존을 검사한다. 원본에서 의도한 .2→1 성장도 보존했다.
- **벌집:** Blender 직접 모델링·고정 길이 종별 리그. 벌 탈 안의 곤충4마리, 6다리·막날개·집게턱으로 구분했다. 봉제부 열림→분리 비행→같은 탈 접지→두 개체가 천 표면 집기→회수→재수납/여밈. 입기는 별도 접근/잡기 경로이며 역재생·메시 숨김 교체가 아니다. 탈은 세 구간의 직접 제어 굽힘으로 천 솔버 결과가 아니며, 현재 4개체가 한 스켈레탈 에셋에 들어 있다.
- **클립:** 찬장 Doff 5.5초, WornIdle/RevealedIdle 각2초. 벌 Doff 5초, SwarmIdle 2초, Don 7초, FullCycle 14초. 모두30fps/1배속. 짧은 ContactProbe도 보존했다. 벌의 Doff 끝→비행 루프→Don 시작 및 주기 끝→처음 본 행렬 경계 오차0. 찬장 재착용은 기존 원본에 없어 이번 변환에 포함하지 않았다.
- **UE 경로:** `/Game/Monsters/OwnersRemaining20261020/Pantry/V02/`, `/Game/Monsters/OwnersRemaining20261020/Hive/V03/`. 각각 SK_Pantry/SK_Hive, AN_* 동작, BP_* 재생용 액터. 지도 `/Game/Monsters/OwnersRemaining20261020/QA_OwnersRemaining`에 찬장 변신과 벌 전체 주기가 배치돼 있다. GeometryCache가 아닌 native 애니메이션이다.
- **QA:** Interchange 대표 본 위치 오차 최대 약0.00037cm, 찬장 모프 활성값 오차 약2e-8. 벌 천 표면 집기236표본 IK 목표 오차0, 천 최저점 표본+18mm, 본 배율 유지. 닫힌 시작/끝은 타원체 외곽 안으로 수납됨을 확인했다. 맵 재열기 후 실제 PIE 자동 재생 약14초에서 찬장 끝 자세 유지·벌 반복·관절 이동 확인. 이후 별도 사선/측면1,174프레임 캡처. 수치 표본을 연속 몸–천/개체 간 충돌이나 자연스러움 합격으로 확대하지 않는다.
- **수정 이력:** 초기 찬장 파생본의 SceneRoot 방향 오류는 Pantry/v02에서 수정했다. 벌의 날개 수납과 가상 집기점은 Hive/v03에서 표면 접촉으로 수정했다. 사용하지 않는 원시 UV 채널을 제거해 UE 줄무늬/얼굴/본체 색도 수정했다. 이전 파생본은 현재 맵에서 사용하지 않는다. 최종 벌 모델 FBX는 `Hive/v03/export/Hive_Mesh.fbx`; 동작별 FBX도 최종 UV로 갱신했다.
- **다음 품질 작업:** 벌 탈의 자연스러운 주름/처짐, 개체 간 및 몸–천 연속 충돌은 추가 검수가 필요하다. 찬장 약7.6만 삼각형/63개 내보내기 모프는 검수용이며 LOD·모프 성능 예산은 별도 작업이다. 사용자 외형 승인이나 최종 게임 AI 완성본으로 기록하지 않는다.
- **납품:** [UE 전체·1x](../../../ArtSource/monster/owners_remaining_20261020/video/OwnersRemaining_UE_1x.mp4), [UE 사선+측면](../../../ArtSource/monster/owners_remaining_20261020/video/OwnersRemaining_UE_MainSide_1x.mp4), [벌 Blender 사선+측면](../../../ArtSource/monster/owners_remaining_20261020/video/Hive_FullCycle_Blender_1x.mp4), [파일/해시·QA·한계](../../../ArtSource/monster/owners_remaining_20261020/delivery_manifest.json). 동작별 영상도 같은 video 폴더에 있다. UE 영상은30fps/1x 프레임별 엔진 캡처로 실시간 성능 영상이 아니며, native_playback.json이 별도의 실제 자동 재생 증거다.
<!-- /owners-remaining-20261020 -->

<!-- combat-clips-20261020 -->
## 2026-10-20 현재 — 다람쥐·거미 공격/피격/죽음 UE 프리뷰 납품

- 사용자 명시 요청의 다람쥐 닫힌 탈과 거미 본체 각각 `Attack / HitReact / Death` 6클립 제작/UE 프리뷰·영상 납품. `status: motion_preview_delivered_review_ready`, `runtime.level: preview`, `job: null`. 기존 외형·리그·메시·바인드·텍스처와 소/닭 원본을 보존했고 게임 AI/피해 판정을 교체하지 않았다.
- 기반: `ArtSource/monster/free_motion_probe_20261019/unreal_v02/{Squirrel,Spider}/*_UE.blend`. 출력 `ArtSource/monster/combat_doff_20261020/{Squirrel,Spider}/`. 새 클립은 Blender 직접 저작이며 기존 무료 이동 모션을 공격으로 이름만 바꾸지 않는다.
- 다람쥐: 움츠림→짧은 전방 도약/덮침→착지·복귀, 뒤로 움찔 후 회복, 옆으로 쓰러져 정지. 거미: 앞다리 한 쌍을 들고 전방 내려치기(나머지 다리 IK 지지), 몸통 움찔, 몸을 낮추고 다리 말아 죽음 정지. 2초/1초/3초, 30fps, 죽음은 역재생 복귀하지 않는다.
- **현재 편집본:** `Squirrel/{Squirrel_Attack,Squirrel_HitReact,Squirrel_Death}.blend`, `Spider/v03/{Spider_Attack,Spider_HitReact,Spider_Death}.blend`와 각 `export/` FBX. 모두 필요한 이미지 패킹/재열기 통과. 다람쥐/거미 메시·기준 리그 원본 파일 해시는 유지했다. 거미 초기의 최대148°/프레임 뒤집힘은 이전 해결 자세를 잇는 IK와 회전 전파, 걷기 중간 자세 대신 고정 지지 기준 자세로 수정했다. 현재 최대22.905°/프레임이며 수치 통과를 사용자 연출 승인으로 쓰지 않는다.
- **QA:** 각 클립 전 프레임 실제 평가 메시 최저점 약+3mm, 유한 좌표·뼈 배율 유지. 죽음 마지막1.5초 정점 이동 다람쥐0/거미0.00000150m. Native AnimSequence 6개 길이2/1/3초 및 원본 뼈 위치 표본 최대 오차0.09036cm. 저장한 맵 재열기와 약31초 실제 PIE 자동 재생에서 카메라/관절 이동, 죽음 끝 자세 유지, 샷당 한 동물 가시성 확인. 연속 다리 자기 충돌/공격 판정/멀티플레이 게임 검수 합격은 아니다.
- **UE 현재 에셋:** `/Game/Monsters/CombatDoff20261020/Squirrel/` 및 `Spider/V03/`의 `AN_*_Attack`, `AN_*_HitReact`, `AN_*_Death`와 같은 이름 `BP_*`. Blueprint는 기존 종별 Skeleton/3 LOD 메시를 참조하는 재생용 SkeletalMeshActor다. 죽음 BP는 반복하지 않고 끝에서 멈춘다. 맵 `QA_CombatDoff`의 30초 자동 시퀀스가 돼지 탈 벗기와 6클립을 순서대로 보여준다. 이전 거미 V01/V02는 비채택 이력이다.
- **영상/납품:** [전체 UE 30초·1x](../../../ArtSource/monster/combat_doff_20261020/video/CombatDoff_UE_1x.mp4), [다람쥐 공격](../../../ArtSource/monster/combat_doff_20261020/video/Squirrel_Attack_UE_1x.mp4), [거미 공격](../../../ArtSource/monster/combat_doff_20261020/video/Spider_Attack_UE_1x.mp4), [파일/해시·동작별 영상·QA](../../../ArtSource/monster/combat_doff_20261020/delivery_manifest.json). 피격·죽음 및 Blender 영상도 같은 `video/` 폴더. UE 영상은 1280×720/30fps/1x 프레임별 PIE 캡처이며 실시간 성능 영상이 아니다. 새 영상15개 전체 디코드/프레임 수 통과, 인코딩 영상12표본 시각 확인.
- 재현/재개 `Tools/Blender/{build_combat_clips,review_combat,package_combat_doff,manifest_combat_doff}_20261020.py`, `Tools/Unreal/{import_combat,stage_combat_doff,finalize_combat_scene,combat_shot_visibility,capture_combat_doff}_20261020.py`. 현재 모션 프리뷰 준비 완료. 다음 작업은 사용자 피드백 또는 실제 소비 액터/AnimBP의 별도 연결 요청에서 이어간다.
<!-- /combat-clips-20261020 -->

## 2026-10-17 현재 — 구매 공격 모션 비교·적용 v06

- 범위 `production`: 이미 산 Crawling Ghoul 공격부터 비교, 소 어깨 돌진·닭 짧은 덮침 적용, 기존 체형 유지, 예고→공격→빗나감 복귀의 짧은 1배속 영상/갤러리. 추가 구매 없음. `status: owned_attack_motion_delivered_review_pending`, `job: null`. 기존 `attack_v04`는 사용자 **공격 연출 거절**로 보존하며 자연스러움 승인으로 취급하지 않는다.
- **구매 원본 비교:** `attack_pack_v01/sources/manifest.json`, `source_pose_comparison.png`: 전진 공격 4개·제자리 3개·점프 2개를 실제 원본 FBX/NPZ 5시점으로 비교했다. 전진01/04는 큰 도약, 02/03은 한 팔 공격, 제자리03은 낮은 지지와 몸통 비틀림. **소 `AttackInPlace03`의 몸통·어깨/고개 변화 + 직접 보정한 돌진·앞발 지지**, **닭 `AttackForward01`의 골반·도약·팔 경로 + 직접 보정한 전방 도달·이족 착지**를 선택했다. 완성 소 어깨 돌진이 팩에 그대로 들어 있다는 뜻은 아니다.
- **현재 편집·출력:** `ArtSource/monster/motion_library_20261017/attack_pack_v06/{cow,chicken}/`의 `*_attack_review.blend`(이동 포함), `*_attack_game.blend`(제자리), FBX, qa.json, anim_event.dsl. 통과한 v07 정점·바인드·이동 클립을 재사용했다. 소 크기 1/닭 0.5 유지. 최초 직접 이식은 소 머리·닭 긴 손 바닥 침범으로 실패. 소 몸통 회전을 체형에 맞춰 제한하고 오른 앞팔 폴을 수정, 닭 손은 원본 움직임을 전방 도달 목표에 매핑하고 이족 착지로 보정했다. 시작/끝은 기존 대기로 블렌드한 뒤 발 IK를 풀어 발이 뜨지 않게 했다. v06 메시 최저 소/닭 약 +0.600cm, 도달 초과 0, 발 목표 오차 0.001cm 미만. 기존 중간 실패 후보는 보존.
- **실제 소비:** `/Game/Monsters/MotionReview20261017/AttackPackV01/ABP_cow_Attack`, `ABP_chicken_Attack`, 각 AnimSequence. 애니메이션만 기존 스켈레톤에 Interchange 임포트. 대표 본 왕복 오차 소 0.00051cm/닭 0.00015cm 미만. 공격에는 저작된 팔꿈치/무릎 방향을 Pole_X/Y/Z 커브로 전달해, 고정 발 IK가 기본 폴로 공격 자세를 덮어쓰지 않게 했다. 소는 기존 Cow 행/배치 유지, 닭은 기존 chicken/revealed_attack 행/배치 유지. 새 ABP와 닭 이동 0.42s/복귀 0.72s만 갱신; 소 0.85s 예고/0.55s 돌진/1.0s 복귀 유지. 기존 공유 서버 이동·판정 코드는 변경하지 않아 추가 C++ 빌드 불필요.
- **영상:** [소 실제 PIE 1배속 약 3.8초](../../../ArtSource/monster/motion_library_20261017/attack_pack_v06/cow/review/cow_attack_flat_1x.mp4), [닭 실제 PIE 1배속 약 3.1초](../../../ArtSource/monster/motion_library_20261017/attack_pack_v06/chicken/review/chicken_attack_flat_1x.mp4). 두 영상 모두 예고→공격→옆 회피로 빗나감→자세 복귀→추격 재개. 제작 방식/버전/1배속/PIE 워터마크, 실제 녹화 시각 유지. [소 중립광 1배속](../../../ArtSource/monster/motion_library_20261017/attack_pack_v06/cow/review/cow_attack_neutral_1x.mp4), [닭 중립광 1배속](../../../ArtSource/monster/motion_library_20261017/attack_pack_v06/chicken/review/chicken_attack_neutral_1x.mp4)은 Blender CPU 렌더로 명시. 별도 CPU 렌더는 다른 세션의 GPU PIE 촬영과 겹치지 않도록 진행했다.
- **검수와 한계:** 실제 Lvl_Flat 공유 AI의 miss→복귀→Chase 및 이후 hit를 12초 표본으로 확인하고, 최종 영상은 첫 miss 구간만 추렸다. 예고/빗나감 복귀 중 틱 이동 0cm, 출발 후 Yaw 변화 0°, 고정 발목 목표 오차 0.001cm 미만. 실제 틱 최대 공격 변위 소 6.67cm/닭 2.98cm. 평지 서버/관찰 카메라·플레이어 봇 검수이며 경사·멀티·전신 충돌/전투 밸런스 확정이 아니다. 검수 시작 위치를 열린 바닥 (2400,0)으로 초기화하고 다른 AI/예약 스폰은 해당 PIE에서만 격리. 이후 대상 몬스터의 이동/상태는 직접 제어하지 않았다. 초기 녹화는 준비 중 다른 AI에게 플레이어가 사망해 결과창이 가렸으므로 채택하지 않았고, 스테이징 격리 후 다시 촬영했다.
- **저장·납품:** Lvl_Flat 저장→다른 맵→재열기에서 새 ABP/행·크기와 기존 닭 프리뷰 유지 확인. 기존 탈착·Content/Peb·다른 세션의 더티 Farmer ABP는 수정/저장하지 않음. [갤러리](../../../ArtSource/model-gallery.html#latest-motion-tests) 상단 두 신규 영상과 중립광/구매 원본 비교 링크; 이전 직접 제작 영상은 '연출 거절' 비교 기록으로 표시. MP4 전체 디코드·시간별 시트·브라우저 1배속 재생 확인. `attack_pack_v06/{engine_import,anim_graph,placement,cold_reopen}.json`, `probe_{cow,chicken}_pack_v02.json`, 각 `review/delivery.json`. 변경 전 행/맵 `*_before_pack.*` 보존, 에디터 언어 ko 복원. 새 원본 생성·긴 탈착 재작업으로 확대하지 않았다.
- 다음: 사용자 영상 검토 후 소의 어깨 충격/닭의 전방 도달·착지 중 필요한 구간만 수정. 기술 QA를 사용자 자연스러움 승인으로 대신하지 않는다.


## 2026-10-17 이전 직접 제작 시안 — 공격 연출 거절·기술 연결 기록

- 사용자 승인 `production`: 소 어깨 돌진, 닭 짧은 덮침. 기존 체형 유지, **예고→공격→빗나간 뒤 복귀→재추격**를 Lvl_Flat과 1배속 영상/갤러리에 연결했다. `status: superseded_visual_rejected`, `job: null`. 취향 승인은 사용자 영상 검토 대기.
- 소 `CowMonster_AI`는 기존 (2200,0,64.5), Mesh Scale 1 유지. 닭 새 본체는 `ChickenMonster_AI` (1500,450,35.5), Mesh Scale 0.5로 추가했다. 기존 `PREVIEW_SessionMonster_ActualEnterLoop` (1500,0,0), Scale 0.5와 탈착 시퀀스/원본, v07 이동, Content/Peb는 보존했다.
- 소비 경로 `/Game/Monsters/MotionReview20261017/AttackV01/ABP_cow_Attack`, `ABP_chicken_Attack`: 제자리 걷기/달리기/회전, 실제 속도 재생률, 접촉 커브와 월드 발 고정 IK. 공격은 Blender 직접 저작, **복제된 서버 상태 경과 시간**으로 SequenceEvaluator 재생. 애님 노티파이/Python이 판정을 실행하지 않는다. 닭 이동은 기존 무료 Mixamo→Blender 보정본, 소 이동은 구매 Ghoul→Blender 보정본 재사용.
- 공유 C++ Pounce에 행 선택형 `PounceMoveTime`/`PounceRecoveryTime`와 `GetPounceTiming()` 추가. 기본 0은 기존 즉시 덮침 유지. 양수인 행만 연속 이동·출발 방향 고정·매 스텝 벽 스윕/명중 검사, 빗나간 뒤 정지 회복. 명중은 기존 HostCatch/피해/쿨다운으로 한 번 처리한다. 브로커 #1 정식 빌드 성공 (`.claude/runs/build-broker/logs/build-20261017-142317.log`).
- `/Game/Data/Monsters/Cow`: 예고 **0.85s**, 이동 **0.55s/220cm**, 빗나감 복귀 **1.0s**, CatchRadius 100cm. 닭은 기존 MonsterTable을 바꾸지 않고 독립 행 `/Game/Data/Monsters/ChickenRevealedAttack`, Alias chicken/SubAlias revealed_attack: **0.6s**, **0.28s/75cm**, **0.65s**, CatchRadius 65cm. 닭 새 본체의 기존 배치 크기 0.5를 유지했다. 수치는 이번 검수 초안이며 전체 전투 밸런스 확정이 아니다.
- 편집/출력 정본 `ArtSource/monster/motion_library_20261017/attack_v04/{cow,chicken}/`: `*_attack_review.blend`(이동 포함), `*_attack_game.blend`(제자리), FBX, qa.json, anim_event.dsl. 소 v01 팔꿈치 바닥 침범을 IK 폴/지지 자세로 수정, v03의 잔여 최저점은 머리 정점임을 확인해 고개 피치를 수정했다. 소 v04 최저 메시 +0.600cm, 닭은 통과한 v02 재사용. 정점·바인드 동일, 도달 초과 0, UE 왕복 대표 본 오차 소 0.00069cm/닭 0.00015cm 미만.
- 실제 서버 PIE에서 양쪽 **miss→제자리 복귀→재추격 및 이후 hit** 확인. 예고/빗나감 복귀 중 틱 이동 0cm, 출발 후 공격 Yaw 변화 0°. 연속 이동 최대 틱 변위 소 6.76cm/닭 4.49cm, 고정 발목 목표 오차 0.001cm 미만. 평지·서버 단독 검수이며 경사/멀티/발바닥 전체 마찰·플레이어와의 전신 관통/벽 충돌의 별도 시연까지 합격한 것은 아니다.
- **1배속 영상 약 12초씩:** [소 어깨 돌진](../../../ArtSource/monster/motion_library_20261017/attack_v04/cow/review/cow_attack_flat_1x.mp4), [닭 짧은 덮침](../../../ArtSource/monster/motion_library_20261017/attack_v04/chicken/review/chicken_attack_flat_1x.mp4). 실제 PIE 실시간 녹화, 기록 시각 유지, 제작 방식/버전/1배속/검수 카메라 워터마크. 검수 때만 다른 AI를 격리하고 첫 위치를 같은 맵의 열린 바닥 (2400,0)으로 초기화했다. 이후 플레이어 접근·회피 입력과 관찰 카메라만 제어했고 몬스터 이동/상태는 실제 공유 AI가 처리했다. 배회 후 울타리가 가리는 초기 촬영본도 보존했다.
- [갤러리 최신 모션](../../../ArtSource/model-gallery.html#latest-motion-tests) 상단 두 영상. 전체 디코드 통과, 시간별 프레임/갤러리 브라우저 재생 1배속 확인. 맵 저장→다른 맵→재열기에서 두 행 등록/ABP 컴파일/배치 크기/기존 닭 유지 확인. 증거 `attack_v04/{engine_import,anim_graph,placement,cold_reopen}.json`, `probe_cow_miss_v05.json`, `probe_chicken_miss_v04.json`, 각 `review/delivery.json`. 원래 맵·Cow/CL9DefList 백업은 같은 폴더의 `*_before_attack.*`. 에디터 언어 ko 복원.
- 다음 작업은 영상 취향 검토와 필요 시 무게감/팔 경로 보강. 옷 탈착 재작업과 다른 5종 신규 공격 제작은 이번에 확대하지 않았다.


## 2026-10-17 소 실제 AI · Lvl_Flat 연결 v01

- 사용자 범위 `production`: 기존 소 체형/걷기·달리기·회전 유지, 배회→플레이어 발견→추격 연결과 Lvl_Flat 실플레이 확인. 옷 탈착은 이전 중단 상태를 유지한다.
- **저장된 실제 배치:** `/Game/Maps/Lvl_Flat`의 `CowMonster_AI`, 기존 닭 옆 (2200,0,64.5), 크기 1. 기존 자체 배치한 대기 전용 SkeletalMeshActor만 공유 `BP_CL9Monster`로 교체했다. 기존 닭·다른 몬스터·소 v19/v20/원본/Content/Peb는 보존했다.
- 신규 행 `/Game/Data/Monsters/Cow`: 레지스트리가 지원하는 독립 MonsterDef. 기존 MonsterTable이 이미 수정된 상태여서 덮어 저장하지 않고 행 하나를 별도 에셋으로 등록했다. `CL9DefList` 재스캔 저장, 행 Validate 통과. 이번 이동 시험의 배회 55.66, 추격 230.35cm/s, 회전 65deg/s는 **검수용 초안**이다. CatchRadius=0; 소의 공격·격분·스토리 트리거 미구현.
- `/Game/Monsters/MotionReview20261017/AI_V01/ABP_Cow_Locomotion`: v07 대기/걷기/달리기/회전을 제자리 클립으로 분리, 실제 GetGroundSpeed에 따라 재생 속도·전환을 계산한다. 생성 경로 이동을 중복 재생하지 않는다. 네 발 접촉 커브와 월드 위치 고정 TwoBoneIK, 도달 범위 이탈 시 해제, 본체/바인드/뼈 길이 유지. 포즈 공간 자동 변환 가정이 실패한 시도는 로그로 보존하고 명시적 LocalToComponent/ComponentToLocal 노드로 해결했다.
- **실제 검수:** 맵 저장→다른 맵→재열기 후 소 Alias/행/등록/기존 닭 위치 유지 확인. 실제 공유 AI의 배회·감지 게이지·추격 전환, 걷기/달리기/방향 전환을 서버 PIE에서 23초 관찰했다. 1377표본, 고정된 발목의 연속 XY 이동 및 목표 오차 모두 0.001cm 미만. 경사·계단·클라이언트 멀티·발바닥 전체 마찰·전신/플레이어 관통까지 합격한 수치는 아니다. 기존 공유 몬스터는 플레이어/아이템과 겹침을 허용하며 이번에 그 충돌 규칙은 바꾸지 않았다.
- [실제 AI 1배속 영상 · 약 22초](../../../ArtSource/monster/motion_library_20261017/ai_runtime_v01/review/cow_native_ai_flat_fixed_1x.mp4). Meshy 6 몸체(CC BY 4.0) / 구매 Ghoul + Blender 리타깃 / UE AnimBP·발 고정 IK, 버전·1배속·PIE·플레이어 봇·검수 카메라 워터마크. 무음이며 녹화 타임스탬프 유지. 소의 위치나 AI 상태를 Python으로 움직인 촬영이 아니다. 플레이어만 시작 위치 초기화 후 AddMovementInput으로 이동; 카메라는 관찰용.
- `ArtSource/model-gallery.html#latest-motion-tests` 상단에 신규 AI 영상 추가. 증거/재개 정본 `ArtSource/monster/motion_library_20261017/ai_runtime_v01/{placement,cold_reopen,graph_connection_verified,final_runtime,delivery}.json`; 백업 `Lvl_Flat_before_ai.umap`, `CL9DefList_before_ai.uasset`. `status: native_cow_ai_delivered_review_pending`, `job: null`.
- **공격 논의안:** [motion-plan.md](../motion-plan.md#2026-10-17-공격-컨셉-논의안). 소 어깨·몸통 돌진, 닭 짧은 덮침, 돼지 삼키기 등 현재 7종을 구분했다. 신규 공격은 사용자 미승인 초안이며 판정/피해량을 구현하지 않았다.


## 2026-10-17 현재 — 소·닭 이동 테스트 레벨 v07 납품

### 추가 요청 완료 — Lvl_Flat 기존 닭 옆 실제 배치

- 사용자 요청대로 `/Game/Maps/Lvl_Flat`의 기존 `PREVIEW_SessionMonster_ActualEnterLoop`(1500,0,0) 오른쪽에 `CowMonster_V07_NextToChicken`을 추가했다. 위치 **(2200,0,-0.5)cm**, Yaw 180°, Scale (1,1,1). v07 소 메시를 그대로 사용하며 제자리 대기 루프를 재생한다. AI/내비·전투 연결이나 기존 닭 교체로 확대하지 않았다.
- 대기 애니메이션 `/Game/Monsters/MotionReview20261017/V07/FlatPlacement/AN_Cow_Idle_Flat`는 v07 첫 60프레임을 명시적 본 키로 복사했다. 원본 구간과 대표 본 위치 오차 최대 약 0.00014cm. 맵 저장 완료, 모든 기존 액터의 위치/회전/크기 유지 확인. 원래 맵은 `runtime_v07/flat_placement/Lvl_Flat_before_cow.umap`에 보존했다. 기존에 더티였던 LookTable/MI 에셋은 저장하지 않았다.
- 실제 `UEDPIE_0_Lvl_Flat`에서 소·기존 닭의 동시 존재, 소 메시 가시성/원래 크기/대기 재생 진행을 확인했다. [실제 게임 배치 확인 화면](../../../ArtSource/monster/motion_library_20261017/runtime_v07/flat_placement/pie_proof_clear/f00010.jpg). 처음 시점은 벽에 가려져 보여주는 위치를 반대편으로 옮겨 재확인했다. 플레이어 이동은 PIE 확인용이며 맵의 PlayerStart/기존 카메라는 수정하지 않았다. 사용자 확인을 위해 PIE는 두 모델을 바라보는 위치로 실행해 두었다.
- 증거 `runtime_v07/flat_placement/{placement,pie_verification,view_debug}.json`. `status: flat_placement_delivered_idle_only`, `job: null`. 소 탈착과 기존 닭 시퀀스, Content/Peb는 보존.

- 최신 사용자 요청은 **소·닭부터 테스트 레벨에서 대기→걷기→달리기→방향 전환, 기존 체형 유지, 발 뜸/미끄러짐 보정, 1배속 영상과 model-gallery 등록**이다. 옷 탈착 작업은 이전 거절 상태를 그대로 보존하며 재개하지 않았다.
- 소 편집 원본 `ArtSource/monster/motion_library_20261017/runtime_v07/cow/cow_locomotion_runtime.blend`; 450프레임·30fps. 구매한 Ghoul Idle/CrawlForward/CrawlFastForward를 리타깃하고 소의 넓은 체형에 맞춰 회전 시 재디딤·접촉 고정을 저작했다. 마지막 회전 프레임 되감기, 과도한 지지 유지, 도달 불가능한 발 목표를 수정했다. v01–06 실패 원본/QA는 보존. 최종은 다리 길이를 유지하는 해석적 2관절 풀이이며 자동 IK의 끌림을 그대로 합격시키지 않았다.
- 본체 정점/53뼈 바인드/원본 v20 SHA-256 동일. 전 프레임 메시 최저 Z 약 0.600cm, 고정된 발목 XY 오차 최대 약 0.00012cm. 재디딤/회전 중 이동하는 발과 손가락 표면 마찰까지 0이라고 주장하지 않는다. 프레임별 값은 `cow/qa.json`.
- UE 5.8.3 Interchange 임포트 완료. 테스트 레벨 `/Game/Monsters/MotionReview20261017/V07/QA_CowChicken_Locomotion`, 소 Native SkeletalMeshActor `MotionReview_cow`. **AnimSequence 실제 재생이며 GeometryCache/프레임 이동 촬영이 아니다.** 저장→다른 맵→재열기에서 두 모델/재생 설정 유지 확인. Blender와 UE 대표 프레임 뼈 위치 최대 오차 약 0.00646cm. 기존 v20/닭 원본·게임 에셋·Content/Peb 보존.
- [소 실제 PIE 1배속 영상 · 15.7초](../../../ArtSource/monster/motion_library_20261017/runtime_v07/cow/review/cow_idle_walk_run_turn_ue_1x.mp4). 1280×816·30fps 출력, 녹화 타임스탬프 유지. 대기·걷기·달리기·회전·복귀 대기를 런타임 로그로 확인. 촬영 시작 시 위치 한 번 초기화 후 native playback, Python은 관찰/카메라 추적만 수행. 어두운 첫 촬영은 `rejected_take01`, 조명 조정 과정은 `pie_frames_take02`, 최종은 `pie_frames_take03`에 보존. 치트/빨리 감기 없음; 테스트 레벨 자동 시연으로 표기.
- [model-gallery.html](../../../ArtSource/model-gallery.html) 상단 최신 모션에 소·닭 플레이어 추가, 브라우저에서 두 영상 로드/재생 확인. 제작 방식/속도/UE 구분 워터마크, MP4 전체 디코드·9개 시간 표본 확인. `runtime_v07/{engine_import,cold_reopen,test_level,video_manifest,delivery_manifest}.json`이 증거. `status: test_level_delivered_quality_review`, `job: null`.
- **한계:** 몬스터 AI·내비게이션·전투·멀티 연결, 옷 탈착, 모든 천/신체 자기 충돌은 이번 범위가 아니다. 전체 자연스러움에 대한 사용자 승인은 아직 받지 않았다.

2026-10-11 사용자 요청: **현재 진행 중인 닭 제작은 그대로 두고**, 나머지 몬스터의 다음 제작 지침과 변신 영상 프리뷰를 준비한다.

> **2026-10-16 현재 지점(문서 감사 표기)** — 소(`storage_owner_disguise`)의 현재 테스트 레벨 후보는 아래 "현재 테스트 레벨 — RuntimeWalkV20" 절의 `runtime_walk_v20` 이고, 그 뒤 Mixamo 시험 둘(`mixamo_motion_probe_v01`·`mixamo_getup_probe_v02`)이 사용자 검토 대기다. 본문 중간의 "현재 작업: meshy-textured-runtime-v19", YAML/JSON 블록의 `current_work`·`current_directory: …/meshy_full_cycle_v18` 는 그 시점의 기록이라 지우지 않고 둔다. 찬장·벌집 폴더는 아직 만들어진 적이 없다(`animation_natural_v1/` 계획 경로는 실제 이름 `animation_natural_v01` 과 다르다).

## 기준과 이번 범위

![사용자 첨부 외형](../../../ArtSource/monster/disguise_trio_preview_v1/reference/owners_reference.png)

원본 보관: `ArtSource/monster/disguise_trio_preview_v1/reference/owners_reference.png`.
첨부의 **평상시 저장고·찬장·벌집 주인**을 겉모습의 형태 기준으로 삼는다.
이미지 하단의 단순 거대화·화난 동물 모습은 새로운 본체 외형의 확정안이 아니다.
앞서 닭에서 선택한 “봉제 탈 속에 숨겨진 괴물” 규칙을 나머지 세 종에 적용해 제안한다.

이번 산출물은 **각 10초 변신 동작/형태 프리뷰와 다음 제작 지침**이다.
세 본체의 외형·동작 차이는 새 제안이며 사용자 채택 전이다.
재착용·최종 스킨 리그·천 시뮬레이션·FBX·게임 AI·UE 연결까지 완성한 작업이 아니다.
후속 자연스러운 애니메이션 제작은 아래 종별 기준을 확정한 뒤 이 문서로 이어간다.

## 작업 격리 — 닭 제작 유지

- `ArtSource/monster/chicken_disguise/animation_natural_v1/` 및 닭 제작 스크립트를 수정하지 않는다.
- 공유 Blender MCP의 라이브 씬, 열린 사용자 파일, 다른 세션의 Blender 프로세스를 조작·종료하지 않는다.
- 각 종은 별도 background Blender와 전용 폴더에서 작업한다. 렌더는 한 작업 안에서 순차 처리한다.
- 프리뷰 폴더: `ArtSource/monster/disguise_trio_preview_v2/{storage,pantry,hive}/`.
- 후속 본 제작은 해당 프리뷰를 덮지 않고 아래 별도 폴더에 만든다.
- 현재 프로젝트의 몬스터 데이터 행·스켈레톤·맵을 새 후보에 임의로 연결하지 않는다.

## V2 수정 — 본체 구조부터 구분

| 주인 | 평상시 외형 | 숨겨진 본체 제안 | 변신에서 가장 먼저 읽힐 것 |
|---|---|---|---|
| 저장고 | 캐러멜 털 덩어리, 큰 옆뿔, 크림색 얼굴과 앞머리, 짧은 굽 | 낮고 두꺼운 수평 몸통, 굵은 앞다리와 넓은 발, 가로로 벌어지는 큰 턱을 가진 사족형 | 둔한 압력 → 한쪽 앞발로 무게를 받음 → 낮고 넓은 몸통이 나옴 |
| 찬장 | 남청색 작은 몸, 큰 둥근 꼬리, 밝은 얼굴, 뾰족한 귀 | 납작하고 긴 분절 몸통, 여덟 다리, 쐐기형 머리와 여섯 작은 눈을 가진 등반형 | 꼬리 밑/등 틈 → 앞다리부터 바깥을 잡음 → 나머지 다리가 시차를 두고 펼쳐짐 |
| 벌집 | 둥근 노란 몸과 갈색 줄, 짧은 더듬이·발, 흰 날개 | 큰 분절 배, 좁은 허리, 검은 겹눈과 집게턱, 여섯 다리와 막날개를 가진 비행형 | 날갯짓 정지 → 등 봉제선 열림 → 여섯 다리와 본체 날개가 차례로 펼쳐짐 |

사용자 피드백: 이전 프리뷰는 세 종 모두 흰 타원 얼굴과 긴 팔다리여서 너무 비슷했다. **그 공통 본체 설계를 폐기한다. 흰 타원 얼굴과 긴 인간형 팔 구조는 닭에만 남긴다.**
세 종은 머리, 몸통 방향, 다리 수와 굵기, 지지 방식이 각각 다르다. 공통점은 봉제 탈·어두운 안쪽 재질·봉제선이 열리고 같은 옷을 다시 입는 규칙이다.
실루엣만 보아도 저장고=낮고 무거운 네발짐승, 찬장=가로로 긴 여덟 다리, 벌집=공중에 매달린 배와 날개로 구별되어야 한다. 색이나 장식만 다르게 하여 같은 몸을 반복하지 않는다.
뿔은 저장고 탈의 장식이고 큰 꼬리는 찬장 탈의 일부다. 본체에 그대로 복사해 단순히 커진 동물로 만들지 않는다.
벌의 하얀 날개는 탈에 남고 본체의 어두운 날개가 별도로 펼쳐진다.
피·장기보다 봉제 탈의 부드러움과 안쪽 관절·실루엣의 대비로 공포감을 만든다.

## 1. 저장고 주인 — 무거운 본체

### 외형 대조

- 몸통은 털 한 가닥씩이 아니라 여러 큰 캐러멜 덩어리로 구성한다.
- 옆으로 뻗고 끝이 위로 말린 큰 뿔, 눈 위로 내려오는 두꺼운 밝은 앞머리, 짧고 단단한 굽을 유지한다.
- 본체는 지면과 평행한 두꺼운 몸통과 낮은 중심을 가진다. 넓은 어깨와 큰 앞발, 더 짧은 뒷다리로 체중을 받친다.
- 얼굴은 어깨 앞쪽의 넓은 주둥이와 가로로 벌어지는 턱이다. 두꺼운 눈썹 아래 작은 눈이 있고, 치아는 입 안에서만 드러난다. 밝은 타원판이나 사람 얼굴을 붙이지 않는다.
- 앞다리는 닭보다 짧고 굵게, 앞발은 넓게 만든다. 평상시 탈의 뿔을 본체에 복사하지 않는다.
- 공포 동작: 낮은 자세로 멈춰 있다가 턱을 열고 짧고 무거운 돌진. 공격 구현은 후속 범위다.

### 변신

1. 몸이 낮아지고 털과 봉제선이 안쪽 압력으로 당겨진다.
2. 아래에서 위로 열린 등·목 뒤 틈으로 한쪽 어깨와 굵은 앞다리가 나온다.
3. 한 앞발을 바닥에 붙이고 체중을 옮긴 다음 반대 앞발과 등을 펼친다.
4. 본체가 위로 빠져나오며 남은 다리를 순서대로 뺀다.
5. 뿔 달린 머리와 털옷은 함께 떨어진다. 무거운 뿔 쪽이 먼저 기울어지고 두꺼운 털은 늦게 가라앉는다.
6. 탈을 벗어난 위치에서 네 발로 낮게 멈춘다. 등을 사람처럼 세우지 않고 넓은 턱을 천천히 플레이어 쪽으로 돌린다.

후속 재착용: 뒷다리와 한 앞발로 지지하고 자유로운 앞발로 탈의 입구를 끌어 정렬한다. 필요한 순간만 앞발 둘을 사용하며 체중 이동을 보여준다. 뿔이 걸리지 않게 다리·몸통·한쪽 어깨 순서로 들어가고 마지막 앞발을 회수한다.
후속 경로: `ArtSource/monster/storage_owner_disguise/animation_natural_v1/`.

## 2. 찬장 주인 — 낮고 빠른 등반형

### 2026-10-19 현재 — animation_v02 수정 모델·변신 영상 제시

- **후속 실행 승인/수정본:** 사용자가 비교 설명을 본 뒤 “저대로 영상만들어줘”라고 요청했다. `animation_v02`에서 채워진 꼬리, 두꺼운 다리/겹친 앞몸통 갑각·큰 배, 낮춘 탈출 경로와 고정 카메라를 저작했다. 기존 파일과 다른 종·사용자 GUI를 보존했다. 사용자의 수정 방향 진행 지시를 실제 v02 외형/모션의 최종 취향 승인으로 표시하지 않는다.
- **현재 영상/원본:** [수정 영상 8.533초·30fps·1x](../../../ArtSource/monster/pantry_owner_disguise/animation_v02/review/pantry_transform_v02_1x.mp4), [사선·측면 전체 1x 비교](../../../ArtSource/monster/pantry_owner_disguise/animation_v02/review/pantry_full_pair_1x.mp4), [편집 가능한 .blend](../../../ArtSource/monster/pantry_owner_disguise/animation_v02/pantry_squirrel_spider_transform_v02.blend), [다각도 모델](../../../ArtSource/monster/pantry_owner_disguise/animation_v02/review/model_multiview_v02.jpg), [주요 자세](../../../ArtSource/monster/pantry_owner_disguise/animation_v02/review/transformation_storyboard_v02.jpg). 본 동작은 166프레임·5.533초이고 제시 영상에는 시작 1초/끝 2초 정지를 추가했다. 본 동작의 재생 속도를 바꾸지 않았다. 탈과 거미를 같은 바닥/축척으로 렌더했다. 우측 상단에 실제 Blender 직접 제작/v02/1x/Blender를 표시했다.
- **v02 검증 범위:** [전체 정수 프레임 QA](../../../ArtSource/monster/pantry_owner_disguise/animation_v02/motion_qa_all.json), [재열기·제어·동일성](../../../ArtSource/monster/pantry_owner_disguise/animation_v02/scene_qa.json), [연결 천 중심선 길이](../../../ArtSource/monster/pantry_owner_disguise/animation_v02/tail_seam_length_qa.json). 1–166프레임 몸–탈 표면 교차 0쌍, 지지 갈고리 표면 정점 오차 최대 0.00000211m, 로컬 다리 길이 오차 최대 0.00000106m, 프레임당 관절 회전 최대 16.314도. 본체 36뼈/탈 15뼈, 8다리·6눈, 탈 루트 Scale=1, 본체 0.2→1 단조 성장, 숨김/교체 키 없음. 꼬리의 연결 천은 접힌 1.3m 중심선이 펼쳐지는 형태 키이며 166자세 길이 최대 편차 0.00000010m다. 바닥 접힘은 형태 키/뼈/Geometry Nodes 직접 보정이며 천 솔버가 아니다. 모든 MP4 전체 디코드와 원본 PNG 332장 디코드, 실제 인코딩된 처음/중간/끝 및 위험 자세 시각 표본을 확인했다. 전체 연속 시청, 서브프레임/부피 포함/천 자기 충돌/표면 신장 전체 합격, 시안 완전 일치 또는 사용자 취향 승인으로 확대하지 않는다.
- **실패와 재개:** 초기 꼬리의 강체 회전이 성장하는 배와 겹쳤고, 천 힌지의 바닥 아래 회전 때문에 탈 전체를 들어 올리는 기존 보정이 얼굴 접촉을 일으켰다. 꼬리를 먼저 옆으로 이동한 뒤 눕히고 연결 천이 접힌 길이를 펴도록 바꿨다. 천 표면만 바닥에서 접히게 하고 얼굴·발의 전체 상승을 제거했다. 앞다리는 구부린 채 개구부로 통과하며 후반 다리 전개 시간을 늘렸다. 실패 원본/QA/렌더 로그는 버전 폴더에 보존했다. [납품 manifest](../../../ArtSource/monster/pantry_owner_disguise/animation_v02/delivery_manifest.json)에 실제 파일/해시를 기록했다. 현재 실행 작업 없음. 재착용/등반 AI/공격/Unreal 연결은 제외 범위를 유지한다.

#### animation_v01 재검토와 보존 이력
- **수정 요청의 근거:** animation_v01 제시 후 “뭔기느낌이달라 다시봐”. 승인 concept_v02, 실제 최종 다각도 렌더와 주요 동작 자세를 직접 재대조했다. [시안/실제 비교](../../../ArtSource/monster/pantry_owner_disguise/review_v02/concept_vs_animation_v01.png). 당시 재검토는 비교 자료/문서 갱신까지였고 v01 원본은 보존했다. 외형/동작 품질 수정 필요로 기존 완료 판정을 철회한 뒤 위 v02 영상 요청으로 이어졌다. v01 기술 QA는 아래 명시한 검사 범위의 이력으로 유지한다.
- **차이와 수정 우선순위:** (1) 채워진 풍성한 꼬리와 짧고 둥근 탈이 큰 C자 고리/긴 몸으로 단순화됐다. (2) 무겁고 두꺼운 불규칙 갑각·넓은 앞몸통·큰 배 대신 가는 막대 다리, 매끈한 타원 판, 규칙적인 구형 배가 기계적인 인상을 만든다. (3) f38–64의 앞다리 수직 전개/본체 상승 때문에 낮게 기어 나오며 커지는 동작이 약해졌다. (4) 빈 탈이 부드럽게 무너지는 천보다 단단한 그릇으로 남고 앞발 옆에서 잘 보이지 않는다. 우선 새 파생 외형 후보에서 꼬리/몸통/갑각 실루엣을 맞춘 뒤, 낮은 앞다리 접지·전방 탈출과 천 접힘을 어려운 2~3초 구간에서 재시험한다. 외형 변경 후보는 실제 3D로 제시한 뒤 전체 클립으로 확장한다.
- **보존한 사용자 외형 승인 이력:** 실제 model_v01 자료 제시 뒤 “ㅇㅇㅇ해봐”로 후속 제작 승인. 승인 원본은 보존하고 `motion_probe_v01`에서 2.933초 핵심 구조를 확인한 뒤 `animation_v01`로 납품했다. 이후 기술 단계에 별도 재승인을 요구하지 않았다. 이 이력은 최신 피드백에 대한 수정 필요 판정을 대신하지 않는다.
- **보존한 v01 편집 원본:** [pantry_squirrel_spider_transform_v01.blend](../../../ArtSource/monster/pantry_owner_disguise/animation_v01/pantry_squirrel_spider_transform_v01.blend). 본체 36뼈(8다리×4 + root/앞몸통/배/수납 머리), 탈 15뼈(8개 천 힌지 + root/머리/꼬리/네 발), 전체 357개 메시의 스키닝·편집용 UV. 본체 루트의 균일 Scale과 FK 뼈, 탈 루트/FK 뼈/처짐 모프를 직접 편집할 수 있다. 큰 경로·접촉점은 저작 스크립트와 베이크된 FK 키로 보존했다. 자유 천 시뮬레이션이나 자동 모션 결과로 표시하지 않는다.
- **영상 납품:** [전체 사선·측면 1배속](../../../ArtSource/monster/pantry_owner_disguise/animation_v01/review/pantry_full_pair_1x.mp4), [전체 사선 단독](../../../ArtSource/monster/pantry_owner_disguise/animation_v01/review/pantry_full_three_quarter_1x.mp4). 정상 → 봉제선 개방 → 앞다리 통과/지지 → 나머지 다리 전개와 동시 성장 → 같은 탈의 독립 낙하 → 낮게 안정. **166프레임·30fps·5.533초·속도 1x**, 1600×696 비교 / 800×696 단독. [핵심 30–117프레임·2.933초 시험](../../../ArtSource/monster/pantry_owner_disguise/motion_probe_v01/review/pantry_core_pair_1x.mp4)과 원본/실패 원인·검사 로그도 보존했다. 실제 제작 방식/버전/Blender/1x 표시는 영상 우측 상단에 있다.
- **최종 렌더:** [정면·측면·후면·사선](../../../ArtSource/monster/pantry_owner_disguise/animation_v01/review/final_model_multiview.png), [연속 동작 주요 자세](../../../ArtSource/monster/pantry_owner_disguise/animation_v01/review/transformation_storyboard.jpg). 최종 .blend에서 렌더했다. 거대 본체/빈 탈은 각 화면 안에서 같은 바닥·같은 축척이며 카메라 확대와 실제 본체 성장 제어를 구분한다.
- **최종 QA:** [전체 정수 프레임 관통/접지/연속성](../../../ArtSource/monster/pantry_owner_disguise/animation_v01/motion_qa_all.json), [재열기·스키닝·UV·보존](../../../ArtSource/monster/pantry_owner_disguise/animation_v01/scene_qa.json), [리그 정리 전후 비교](../../../ArtSource/monster/pantry_owner_disguise/animation_v01/rig_qa.json). 1–166프레임 몸–탈 삼각형 표면 교차 0쌍, 실제 지지 갈고리 정점의 접점 최대 오차 **0.00000195m**, 로컬 다리 뼈 길이 최대 오차 0.000001m 미만, 뼈 최대 프레임 회전 **15.752도**(짧은 전개 상한 16도), 도달 불가 목표 0개. 정리 전후 Scale/천 힌지 행렬 차이 0. 탈 Scale=1, 본체 0.2→1 단조 성장, 같은 본체/같은 탈 유지, 숨김/교체 애니메이션 없음, 외부 이미지/캐시 의존 없음. 전체 MP4 디코드와 원본 PNG 332장 디코드, 실제 인코딩된 처음/중간/끝 및 위험 자세 시각 표본을 확인했다. 영상 전체 연속 시청 또는 서브프레임·부피 포함·천 자기 충돌 인증으로 확대하지 않는다.
- **실패 원인/해소:** 닫힌 등 윗점을 좌우에 함께 묶어 생긴 통로 막힘은 봉제선 정점의 좌우 웨이트를 분리했다. 움직이는 목표와 접힌 자세를 매 프레임 섞을 때 생긴 회전 경로 전환은 고정된 통과 자세와 뼈의 공간 회전 보간으로 바꿨다. 앞다리는 먼저 위로 통과한 뒤 바깥 접지로 전환하고, 꼬리는 같은 탈의 부착 관절에서 뒤쪽으로 기울여 다리와 분리했다. 마지막 3개 위험 프레임은 몸의 초기 상승 경로를 수정한 뒤 전체 프레임 재검사했다. 샘플만 통과한 중간본과 잘못 복제된 카메라 표본은 최종 검수 영상으로 사용하지 않는다.
- **납품 이력/현재 재개:** Blender 모델·리깅·단일 변신·다각도/1배속 영상·QA 파일은 제작·납품했으나 **최신 재검토에서 외형/동작 품질 수정 필요**다. [기존 납품 manifest](../../../ArtSource/monster/pantry_owner_disguise/animation_v01/delivery_manifest.json)는 당시 파일·해시·제작 방식·검사 범위의 불변 이력으로 보존한다. 게임 LOD/성능 예산·재착용·등반 AI·공격·Unreal 연결은 별도 범위다. 실제 수정 후보는 현재 편집 원본의 새 파생 버전으로 만든다. `job: null`.
- **현재 범위 `production`:** 사용자가 승인 concept_v02를 기반으로 Blender 모델·리깅·단일 탈 벗기/성장 애니메이션, 다각도 렌더·중립광 사선/측면 1배속 영상·QA·재개 기록을 요청했다. 실제 모델 외형 확인 → 어려운 2~3초 시험 → 최종 리그/전체 동작의 순서를 유지한다. 재착용·등반 AI·공격·Unreal 연결은 이번 범위에서 제외한다.
- **보존한 외형 승인본 model_v01:** [초안 원본](../../../ArtSource/monster/pantry_owner_disguise/model_v01/pantry_squirrel_spider_model_v01.blend), [3상태 개요](../../../ArtSource/monster/pantry_owner_disguise/model_v01/preview/appearance_review_overview.png), [다각도](../../../ArtSource/monster/pantry_owner_disguise/model_v01/preview/appearance_review_sheet.png), [접힌 본체/8다리 구조](../../../ArtSource/monster/pantry_owner_disguise/model_v01/preview/folded_structure_review.png). 별도 background Blender 5.2.2 LTS에서 직접 메시를 저작했다. 이미지 생성·유료 3D 서비스는 사용하지 않았다.
- **승인 외형:** 남청색/크림 얼굴, 작은 귀·발, 밝은 끝의 큰 말린 꼬리를 가진 탈. 별도의 거미는 낮고 넓은 앞몸통·둥근 단일 배·낮은 쐐기 머리·양쪽 세 눈·짧은 촉지·네 쌍의 긴 다리·짧은 갈고리다. 배의 표면 판은 장갑 무늬이며 긴 지네 몸통이 아니다. 탈에는 실제 열린 경계·7mm 두께/안감·봉제선과 열림/처짐 형태 키를 만들었다. 탈 얼굴·귀·발·꼬리는 같은 탈 루트에 남는다.
- **크기/제어:** 작업 단위 1BU=1m, +X 정면/+Z 위. 거미 전체 다리 폭 / 탈 길이(꼬리 제외) **4.965배**. 동일 바닥·동일 축척의 거대 본체/빈 탈 렌더에 적용했다. 게임 치수는 미확정이다. 탈 루트 Scale=1, 접힌 본체 루트 0.20, 거대 본체 루트 1.00으로 독립 제어한다. 초안의 세 정지 연구 컬렉션을 교체하는 방식은 사용하지 않았으며, 실제 animation_v01은 하나의 본체와 하나의 탈을 연속 제어한다.
- **외형 초안의 예비 관절 구조(이력):** 각 다리 4개 뼈, 좌우 4개씩 총 8체인; 당시 본체 35뼈. 동일한 8다리를 관절로 접었다. 당시에는 정지 외형용 스키닝까지만 확인했으며 최종 작업에서는 수납 머리 뼈·별도 탈 리그와 접촉 경로를 보강했다.
- **실제 검사:** 저장 파일 재열기, 승인 이미지 내장/외부 이미지 의존 없음, 정상 탈/빈 탈의 65개 기본 메시·토폴로지 동일, 눈 6개, 8다리 체계, PNG 19장 전체 디코드 및 다각도/구조 시트 시각 확인. 최종 거대 본체/빈 탈의 **한 정지 자세**에서 평가 메시 삼각형 표면 교차 0쌍, 빈 탈 최저 Z=0.012m. 이 검사는 내부 포함·자기 충돌·모든 동작 프레임의 관통/접지 합격이 아니다. 꼬리 안쪽 찝힘은 곡률/체적을 정리했고, 넓어진 꼬리와 앞다리의 비교 배치 교차는 빈 탈 위치를 옮겨 해소했다.
- **외형 초안 제시 당시의 미완료 항목(해소 이력):** 모델 검토·핵심 시험·최종 리그·전체 변신/영상·동작 QA는 위 animation_v01 납품으로 이어졌다. 고밀도 Blender 모델을 게임 메시 예산/LOD 합격으로 취급하지 않는다.
- **보존:** concept_v01/v02와 닭·소·돼지 승인본, 사용자 Blender GUI, Unreal 콘텐츠는 변경하지 않았다. 버전 폴더 안 원본/렌더/로그와 [manifest](../../../ArtSource/monster/pantry_owner_disguise/model_v01/manifest.json), [재열기·정지 QA](../../../ArtSource/monster/pantry_owner_disguise/model_v01/reopen_qa.json)를 보존했다. 재현/검사는 `Tools/Blender/{build,render,audit,finish,package}_pantry_spider_model_v01.py`를 사용한다. 빌드 반복은 필요 없고 현재 .blend에서 재개한다.

```yaml
concept_pipeline:
  schema_version: 2
  scope: production
  phase: delivery
  status: awaiting_user
  review_policy: concept_and_model
  review_override: null
  requested_outputs:
    - editable_costume_spider_blend
    - final_rig
    - one_continuous_doff_unfold_grow_animation
    - multiview_model_renders
    - neutral_three_quarter_and_side_1x_videos
    - qa_and_resume
  concept:
    candidate_version: concept_v02
    approved_version: concept_v02
    approval_evidence: '2026-10-19 사용자가 v02를 보고 “이대로해서 만들어보자” 승인; 본 요청에서 재확인'
  blender:
    source_concept_version: concept_v02
    candidate_version: animation_v02
    approved_version: model_v01
    approval_evidence: '실제 3D 개요/다각도 제시 후 사용자가 “ㅇㅇㅇ해봐”로 이 외형의 후속 진행 승인'
    manifest: ArtSource/monster/pantry_owner_disguise/animation_v02/delivery_manifest.json
  production:
    source_blender_version: model_v01
    candidate_version: animation_v02
    manifest: ArtSource/monster/pantry_owner_disguise/animation_v02/delivery_manifest.json
    qa:
      static_source: pass
      motion_probe: pass
      scene_reopen_and_control_identity: pass
      continuous_transformation: pass
      motion_contact_and_collision: pass
      one_x_video: pass
      concept_fidelity_user_review: not_run
      low_crawl_and_soft_costume_user_review: not_run
    runtime: {level: not_tested, scope: excluded_by_user}
    contact_qa_scope: integer_frame_surface_intersections_and_planted_claw_vertices_1_to_166
  pending_review:
    stage: blender_and_motion
    version: animation_v02
    presented_at: '2026-10-19T13:29:01+09:00'
    evidence: ArtSource/monster/pantry_owner_disguise/animation_v02/review/pantry_transform_v02_1x.mp4
  checkpoint:
    path: ArtSource/monster/pantry_owner_disguise/animation_v02/pantry_squirrel_spider_transform_v02.blend
    sha256: 1b80f32bc33d655f4e74dfae41fcbf3210d4620ba4828e1e4b2023e2983a4ac9
  job: null
  last_completed: 'v02 외형/연속 변신 수정, 핵심 2.933초 시험·전체 5.533초 비교·8.533초 제시 영상, 166 정수 프레임 QA·재열기·332 PNG/MP4 디코드·manifest 납품'
  next_action: '실제 v02 모델/모션 사용자 피드백을 받으면 이 체크포인트의 새 파생 버전에서 해당 부위부터 수정. 재착용/AI/공격/Unreal은 범위 제외'
  blocker: null
```

재개: 이 절과 `animation_v02/delivery_manifest.json`, `motion_qa_all.json`, `scene_qa.json`, 실제 제시 영상을 읽고 위 체크포인트 해시를 확인한다. concept_v02/model_v01 승인 이력과 v01 납품본은 보존하며 v02 외형/모션을 승인됐다고 추정하지 않는다. 승인 컨셉을 다시 생성하지 않는다. 새 수정은 현재 .blend의 파생 버전에서 필요한 구간부터 검사한다. 수치 QA 통과를 시안 재현/자연스러움 전체 합격으로 확대하지 않는다. 같은 본체/8다리/같은 탈과 탈 독립 Scale=1, 탈출 중 동시 성장 계약을 유지한다. 메시 숨김/교체로 통과를 대신하지 않는다. 현재 실행 작업은 없다.

### 2026-10-19 컨셉 승인 이력 — 거대 거미 v02·제작 시작문

- 사용자 수정: “좀더 큰 거미”, “껍질벗고나와서 커지는거까지가 한스탭”. 이어 v02를 보고 “이대로해서 만들어보자 돼지처럼 프롬포트작성해줘”라고 컨셉 승인 및 시작문 작성을 요청했다. 당시 실행은 이미지 보존·승인 기록·시작문 작성까지였으며 이후 실제 제작 상태는 위 animation_v01 납품 절이 정본이다. 컨셉 승인은 유지하고 실제 Blender 외형 승인은 구분한다.
- **최신 승인 시안:** [거대 거미 v02](../../../ArtSource/monster/pantry_owner_disguise/concept_v02/squirrel_costume_monster_v02.png). 귀여운 남청색/크림 다람쥐 탈 유지, 거대하고 둥근 배와 넓고 낮은 앞몸통, 긴 다리 네 쌍. 아래 v01의 긴 분절 몸통 제안보다 이 승인이 우선한다. 작은 빈 탈이 본체 앞발 옆에 같은 축척으로 남아 크기 차이를 드러낸다.
- **변신 계약:** 등·꼬리 뿌리 개구부에서 나오는 동안 다리를 순차적으로 펼치고 본체와 배가 커지는 **한 번의 연속 동작**. 완전 탈출 후 별도 성장 단계를 붙이지 않는다. 탈은 확대하지 않는다. 이번 본체 성장은 사용자가 의도한 종별 연출이며 관통을 숨기기 위한 임의 크기 변경 허가가 아니다. 다리 폭 약 몸길이 5배(꼬리 제외)는 이미지 프롬프트의 참고 비율이며 게임 수치 확정이 아니다.
- 내장 이미지 도구로 v01을 편집한 컨셉이며 실제 3D가 아니다. [생성 프롬프트](../../../ArtSource/monster/pantry_owner_disguise/concept_v02/prompt.txt), [새 세션 복사·붙여넣기 시작문](../../../ArtSource/monster/pantry_owner_disguise/concept_v02/start_new_session.txt). 이전 시안은 보존한다.
- 다음 요청 범위: Blender 탈·본체 모델, 리깅, 탈 벗기와 성장의 단일 변신 클립, 다각도 렌더·1배속 영상·QA. 실제 3D 외형 확인 후 어려운 2~3초 통과·성장 구간부터 검증한다. 재착용·등반 AI·공격·Unreal 연결은 후속 범위다. 이 절을 기존 제작 정본으로 계속 사용한다.

### 2026-10-19 다람쥐 신규 컨셉 후보

- 사용자 요청: “다람쥐랑 벌부터 하자. 다람쥐 괴물이랑해서 아트그려줘”. 이번 실행 범위 `concept_only`, 다람쥐부터 제시. 돼지 v02 실제 제작은 별도 세션 예정이며 여기서 재개하지 않는다.
- [시안 v01](../../../ArtSource/monster/pantry_owner_disguise/concept_v01/squirrel_costume_monster_v01.png): 평상시 / 꼬리·등 봉제 틈에서 본체가 펼쳐짐 / 본체와 빈 탈 / 선반 아래 등반 자세의 네 장면. 남청색 몸·크림 얼굴·큰 말린 꼬리와 밝은 꼬리 끝을 유지했다.
- 기존 V2 구조 제안의 납작한 분절 몸통·네 쌍의 갈고리 다리·쐐기형 머리·양쪽 세 눈을 시각화했다. 스토리의 “꼬리가 여러 갈래로 풀려 천장을 탄다”를 꼬리 안쪽에 접힌 다리가 펼쳐지는 식으로 해석한 후보다. 흰 가면·사람형 본체는 사용하지 않았다.
- 내장 `image_gen` 1회. [실제 프롬프트](../../../ArtSource/monster/pantry_owner_disguise/concept_v01/prompt.txt). 입력 `ArtSource/monster/disguise_trio_preview_v1/reference/owners_reference.png`의 다람쥐. 승인 전 후보이며 실제 3D·탈착·등반 동작을 검증한 자료가 아니다.
- 다음 행동: 사용자 본체 외형 피드백 반영. 벌은 별도 요청 단계에서 스토리의 다수로 흩어지는 설정과 기존 단일 비행형 제안의 차이를 확인하고 진행한다.

### 외형 대조

- 남청색 둥근 몸, 밝은 얼굴 무늬, 큰 검은 눈, 작은 뾰족 귀를 유지한다.
- 몸보다 크게 말리는 꼬리가 주요 실루엣이다. 평상시 꼬리를 생략하거나 똑바른 막대로 바꾸지 않는다.
- **구안 v01(현 제작 기준 아님):** 가로로 길고 납작한 분절 몸통. **현재 승인 v02:** 낮고 넓은 앞몸통과 크고 둥근 단일 배의 거대 거미. 네 쌍, 총 여덟 다리가 앞몸통 좌우에서 나온다. 사람이 기어가는 어깨·골반 구조를 반복하지 않는다.
- 머리는 낮은 쐐기 형태이고 양쪽에 세 개씩 작은 눈이 있다. 입은 가로로 낮게 열리고 짧은 촉지가 양쪽으로 움직인다. 흰 얼굴판은 없다.
- 다리 끝의 짧은 갈고리로 선반 가장자리를 잡는다. 앞 두 다리는 물건·옷을 잡는 기능도 한다. 여덟 개의 사람 손을 달지 않는다.
- 공포 동작: 몸 높이가 거의 변하지 않은 채 옆으로 방향을 틀고 선반 밑으로 붙는다. 등반 동작과 AI는 후속 범위다.

### 변신

1. 꼬리 흔들림이 멈추고, 꼬리 밑에서 등 위로 봉제선이 벌어진다.
2. 앞다리 하나가 틈에서 먼저 나와 바닥이나 낮은 선반 가장자리를 잡는다.
3. 반대 앞다리가 옆으로 뻗고 납작한 머리와 몸통 앞쪽이 낮게 미끄러져 나온다.
4. 몸통이 뒤따르며 나머지 세 쌍의 다리가 앞에서 뒤 순서로 나와 바닥을 잡는다. 한꺼번에 여덟 다리가 튀어나오지 않는다.
5. 큰 꼬리가 붙은 탈이 옆으로 쓰러진다. 꼬리가 먼저 눕고 작은 얼굴은 천에 딸려 내려간다.
6. 본체는 바로 우뚝 서지 않는다. 낮게 정지했다가 머리만 플레이어 쪽으로 돌린다.

후속 재착용: 앞다리로 꼬리 무게에 뒤집힌 옷 방향을 고친다. 뒤쪽 다리부터 몸 옆으로 접고 몸통을 넣는다. 바깥에 남은 앞다리로 입구를 잡아당긴 뒤 마지막으로 회수한다. 다리 수가 늘거나 없어지면 안 된다.
후속 경로: `ArtSource/monster/pantry_owner_disguise/animation_natural_v1/`.

## 3. 벌집 주인 — 접힌 곤충형

### 외형 대조

- 노란 둥근 몸과 넓은 갈색 줄무늬, 작은 더듬이·발, 부드러운 흰 날개를 유지한다.
- 평상시 둥근 몸이 그대로 부풀어 벌 괴수가 되는 변신으로 끝내지 않는다.
- 본체는 작은 가슴과 좁은 허리 아래로 큰 분절 배가 매달린다. 배 끝에는 아래로 휘는 침이 있다. 세 쌍의 관절 다리와 접힌 막날개를 갖는다.
- 머리는 짧은 곤충형이다. 검은 겹눈 두 개, 별도로 벌어지는 집게턱, 가는 더듬이로 얼굴을 만든다. 흰 타원 얼굴이나 사람 입을 붙이지 않는다.
- 공포 동작: 배는 무겁게 늦게 따라오고 머리·집게턱은 목표에 먼저 반응한다. 비행 중 여섯 다리는 같은 각도로 늘어지지 않는다.

### 변신

1. 작은 흰 날개의 움직임이 멈추고 몸이 잠깐 아래로 처진다.
2. 등 쪽 봉제선이 위로 열리고 앞쪽 다리 한 쌍이 먼저 나온다.
3. 머리와 가슴이 위로 빠져나온 뒤 큰 배가 따라온다. 나머지 두 쌍의 다리가 시차를 두고 펼쳐진다.
4. 본체 날개가 펴져 자세를 지탱한 후 탈과 분리된다. 탈만 공중에 남아 떠 있지 않게 한다.
5. 흰 날개와 노란 껍질이 함께 바닥에 떨어진다. 본체의 날개와 겉옷 날개가 뒤바뀌지 않는다.
6. 큰 배가 아래로 매달리고 집게턱이 움직인다. 다리마다 다른 각도로 접힌 채 낮게 떠 있는 위협 자세를 유지한다.

후속 재착용: 먼저 착지해 안정적으로 몸을 받친 뒤 앞다리로 옷을 집는다. 막날개와 긴 다리를 정해진 순서로 접어 넣는다. 여섯 다리를 한 프레임에 접거나 공중에서 탈을 끌어올리는 생략을 피한다.
후속 경로: `ArtSource/monster/hive_owner_disguise/animation_natural_v1/`.
비행/착지의 게임 AI·공격 판정은 별도 설계 사항이다.

## 자연스러운 본 제작의 공통 기준

[닭 제작 인계](chicken_disguise-animation-handoff.md)의 손·천 접촉, 체중 이동, 실제 리그, 재착용 검수 원칙을 이어받는다. 닭의 모델·동작을 덮거나 이 문서를 닭 세션에 강제로 적용하지 않는다.

- 실제 연결된 메시와 스킨 리그, 손·발 IK, 탈 변형 리그를 만든다. 막대 파츠 위치 보간만으로 완료 처리하지 않는다.
- 긴 팔다리는 관절로 접어 탈에 넣는다. 몸 전체/뼈 길이 Scale 변화로 탈 안에 숨기는 것은 본 제작에서 교체한다.
- 위로 열리는 봉제선과 아래로 떨어지는 옷을 구분한다. 옷이 본체 허리에 계속 붙어 있으면 벗기 완료가 아니다.
- 뿔·꼬리·날개는 탈과 함께 남는다. 낙하 후 위치/방향을 보존하고 동일한 탈을 회수한다.
- 손이 옷에 닿고 집은 다음 천이 따라오게 한다. 재착용은 역재생이 아닌 별도 동작이다.
- 천의 큰 접힘과 장식 질량을 먼저 키프레임으로 잡고 자유 부분에 시뮬레이션/보조 뼈를 더한다.
- 프리뷰의 바닥으로 납작해지는 변형은 천 시뮬레이션이 아니다. 최종 재질이나 조명으로 이 한계를 숨기지 않는다.
- 30fps 본 제작, 단위 1BU=1m, +X 정면, +Z 위. 치수·속도는 기획 확정값과 제작 가정을 구분해 기록한다.
- 정면 3/4·측면·게임 거리에서 형태와 관통을 확인한다. 중립광과 어두운 조명 둘 다 검수한다.
- 세 본체의 검은 실루엣을 같은 화면에 놓고 이름·색·옷 없이 구별되는지 확인한다. 머리 장식만 다른 공용 본체로 되돌리지 않는다.
- 결과는 `.blend`, 독립 Action, 벗기/입기 MP4, 전체 약 30초 영상, 주요 자세, manifest, QA 기록으로 남긴다.

## 이번 프리뷰와 본 제작의 차이

이번 영상은 모양과 타이밍을 비교하는 **10초·24fps·1280×720 프리뷰**다.
사지 길이가 변하는 오브젝트 애니메이션과 탈 Shape Key를 사용한다. 스킨 리그/물리 천/재착용은 아직 없다.
양각 얼굴·털·장식의 정밀도, 탈이 떨어지는 접촉과 접힘은 본 제작에서 보완해야 한다.
V2는 본체 외형을 바꾸면서 V1 탈 동작을 유지한 외형 비교용 시안이다. 특히 저장고·찬장의 중간 변신 자세에서 본체가 옷 측면이나 얼굴 부분과 겹친다. 본 제작에서는 커진 본체에 맞춰 옷 입구와 탈출 궤적을 다시 설계하고, 넓어진 사지가 봉제 틈을 통과하는지 검수해야 한다.
이 상태를 게임용 완성 에셋이나 위의 자연스러움 검수 통과로 보고하지 않는다.

초기 프리뷰: `Tools/Blender/build_disguise_trio_preview.py`.
V2 본체 수정: `Tools/Blender/refine_disguise_trio_identity_v2.py`. 합본/검수: `Tools/Blender/check_combine_disguise_trio_v2.py`.
V1은 비교용으로 보존하며, 본체의 형상 기준은 아래 V2 프리뷰다.

| 영상 | 길이 | 원본 |
|---|---:|---|
| [저장고 주인](../../../ArtSource/monster/disguise_trio_preview_v2/storage/preview/storage_transform.mp4) | 10초 | [Blender](../../../ArtSource/monster/disguise_trio_preview_v2/storage/storage_transform.blend) |
| [찬장 주인](../../../ArtSource/monster/disguise_trio_preview_v2/pantry/preview/pantry_transform.mp4) | 10초 | [Blender](../../../ArtSource/monster/disguise_trio_preview_v2/pantry/pantry_transform.blend) |
| [벌집 주인](../../../ArtSource/monster/disguise_trio_preview_v2/hive/preview/hive_transform.mp4) | 10초 | [Blender](../../../ArtSource/monster/disguise_trio_preview_v2/hive/hive_transform.blend) |
| [세 종 합본](../../../ArtSource/monster/disguise_trio_preview_v2/preview/three_owners_transformation_30s.mp4) | 30초 | 저장고 → 찬장 → 벌집 |

파일 검수 완료: 개별 MP4 3개를 각 1280×720·24fps·240프레임·10초로, 합본을 720프레임·30초로 재읽었다. 인코딩된 각 영상의 115·215프레임을 직접 확인했다. 형태 차이를 확인하는 프리뷰이며, 자연스러운 탈의 애니메이션 완성은 아니다. 관통과 리그·천의 한계는 [QA 기록](../../../ArtSource/monster/disguise_trio_preview_v2/qa.md), 수치는 [영상 재읽기 결과](../../../ArtSource/monster/disguise_trio_preview_v2/video_readback.json)에 남겼다.

## 새 세션 시작문

```text
docs/asset/monster/disguise-trio-next-production.md를 읽고, [저장고 / 찬장 / 벌집 중 한 종]의 자연스러운 본 제작을 시작해줘.
정본 이미지와 해당 종 프리뷰를 직접 확인해. 내부 본체 제안의 채택 여부는 현재 사용자 지시를 우선해.
모델·본체/탈 리그·자연스러운 벗기와 재착용·영상 검수까지 진행해.
현재 닭 작업과 공유 Blender MCP 씬은 건드리지 말고, 해당 종 전용 폴더와 별도 background Blender 프로세스를 사용해.
기존 프리뷰의 늘어나는 막대 팔다리나 통째로 납작해지는 옷을 완성품으로 재사용하지 마.
```


## 2026-10-13 소 탈 몬스터 신규 전체 제작 요청

<!-- centered-cycle-current-review -->
**현재 작업: meshy-textured-runtime-v19, 텍스처·언리얼 연결·실제 PIE 촬영 제작 중.** (2026-10-16 정정: 이 줄은 v19 때 쓴 것 — 현재는 아래 "현재 테스트 레벨 — RuntimeWalkV20" 절이 정본이다.) 2026-10-16 사용자의 “텍스쳐 넣어서 언리얼로 제대로 만들어서 영상 찍어 줘”로 이전 Unreal 작업 금지가 해제되었다. 승인 외형·v18 손 집기/당기기·완전히 벗기/낙하·같은 가죽 줍기/재착용·고개 움직임을 유지한다. 기존 v18 Blender 영상 및 모든 이전 엔진 에셋은 보존한다. 현재는 준비 단계이며 엔진 연결 완료로 표시하지 않는다.
<!-- /centered-cycle-current-review -->

사용자 요청 범위는 컨셉부터 게임용 모델·리깅·탈 벗기·같은 탈 회수/입기·이동·실제 인게임 검수까지다. 기존 결과는 보존한다. 컨셉과 실제 Blender 외형은 각각 보여주고 확인받는다. 컨셉 v01에 대한 공포감 강화 피드백을 반영하여 v03 컨셉과 확대된 실제 Blender v02 외형 모두 사용자 승인되었다. 기존 V2 프리뷰와 컨셉 v01/v02는 이력으로 보존한다.

- 형태 기준: 캐러멜 큰 털 덩어리·크림 앞머리·옆뿔·짧은 굽. 본체는 낮은 수평 사족·넓은 어깨/앞발·가로 턱이며 닭의 흰 타원 얼굴/긴 인간형 팔을 재사용하지 않는다.
- 보행/변신 기준: 네 발 지지와 무게 이동, 한 앞발 지지 후 어깨 통과. 재착용은 세 발 지지와 자유 앞발의 집기부터 검증하며 단순 역재생을 사용하지 않는다.
- 기존 V2의 관통, 사지 길이 변화, 탈 납작화는 미해결이며 새 본제작의 합격 증거로 사용하지 않는다.
- 자료 열람·기존 원본 해시 확인 후 신규 컨셉 v01을 별도 경로에 저장했다. 기존 Blender 씬/원본·영상·엔진 에셋은 수정하지 않았다.

```yaml
concept_pipeline: {
  "schema_version": 2,
  "asset_id": "storage_owner_disguise",
  "scope": "production",
  "requested_outputs": [
    "신규 컨셉 이미지와 사용자 검토",
    "실제 Blender 외형 다각도 렌더와 사용자 검토",
    "외형 승인 후 2~3초 탈착 접촉·통과 시험 및 짧은 엔진 왕복",
    "게임용 모델·종별 리그",
    "탈 벗기·바닥의 동일한 탈 회수 및 별도 재착용 동작",
    "필요한 대기·이동 클립",
    "1배속 중립광·측면 검수 영상",
    "제작 방식·버전·1x·검수 수준 워터마크를 포함한 실제 인게임 영상"
  ],
  "review_policy": "concept_and_model",
  "review_override": null,
  "phase": "engine_verify",
  "status": "awaiting_user",
  "updated_at": "2026-10-16T10:20:30+00:00",
  "concept": {
    "candidate_version": "v03",
    "approved_version": "v03",
    "image": "ArtSource/monster/storage_owner_disguise/concept_v03/cow_disguise_concept_v03.png",
    "manifest": "ArtSource/monster/storage_owner_disguise/concept_v03/manifest.json"
  },
  "blender": {
    "candidate_version": "blender_v02",
    "approved_version": "blender_v02",
    "source_concept_version": "v03",
    "approval_evidence": [
      {
        "path": "ArtSource/monster/storage_owner_disguise/blender_v02/storage_owner_review_v02.blend",
        "sha256": "507724071490a12182b196925e744fade85b21dd340bde50ac4df16ccfbd6677"
      },
      {
        "path": "ArtSource/monster/storage_owner_disguise/blender_v02/preview/01_size_comparison.png",
        "sha256": "9eafc891d372ac48070285efb0c230f266147d6b21dd669e076f6fc996be1b09"
      }
    ]
  },
  "production": {
    "candidate_version": "runtime-walk-v20",
    "source_blender_version": "blender_v02",
    "qa": {
      "status": "blender_actual_finger_contact_and_body_garment_pass_visual_review_pending",
      "body_garment": {
        "frames": 601,
        "bad_frames": [],
        "scope": "Entire fitted body mesh versus entire same garment surface, every integer frame, independently from changed-finger subset checks."
      },
      "actual_finger_contact": {
        "frames": 601,
        "bad_body_garment_frames": [],
        "maximum_full_grasp_digit_gap_cm": 0.5584409460425377,
        "method": "Finger chains reach actual cloth surface; any crossing is stopped by bisection at first surface contact. Garment identity/geometry unchanged.",
        "collision_scope": "Finger fit used affected-finger contact tests; the final entire body and garment, including idle paw and floor fixes, are checked in fitted_full_surface_qa.json."
      },
      "saved_source": {
        "saved_source": "review_finger_contact.blend",
        "max_cache_reopen_error_cm": 0.0,
        "max_body_cache_reopen_error_cm": 0.0002813440256406295,
        "bad_contact_frames": [],
        "body_and_garment_visible": true,
        "scope": "selected source reopen frames + per-digit surface distance samples; full 601-frame fitted surface crossing audit is fitted_full_surface_qa.json"
      },
      "same_garment": {
        "object": "SK_SameCowCostume_001",
        "replacement": false,
        "topology_constant": true,
        "samples": 601,
        "floor_rest_frames": [
          231,
          294
        ],
        "floor_max_vertex_drift_cm": 0.0,
        "closed_start_end_max_vertex_difference_cm": 0.0,
        "fully_off_hold_frames": [
          190,
          211
        ],
        "released_at_s": 7,
        "pickup_closing_at_s": [
          9.8,
          10.05
        ],
        "authored_primary_material_point_max_drift_cm": 0.017983894812455326,
        "actual_held_finger_pad_clearance_max_cm": 0.5584409460425377
      },
      "video": {
        "file": "C:\\Work\\CL9\\ArtSource\\monster\\storage_owner_disguise\\meshy_grasp_cycle_v18\\review\\cow_grasp_v18_doff_drop_same_skin_pickup_don_1x.mp4",
        "frames": 600,
        "fps": 30.0,
        "seconds": 20.0,
        "size": [
          1536,
          720
        ],
        "full_decode": "pass",
        "speed": "original authored 30fps / 1.0x, no retiming",
        "watermark": "actual production methods + v18 + Blender review + speed",
        "sha256": "d759787c6b478f5ba51248017c63df9fc2c3ccd30d9c37b9329cee788d81b3b3"
      },
      "garment_self": "Not certified for final game cloth.",
      "v19_engine": {
        "import": "ArtSource/monster/storage_owner_disguise/meshy_textured_runtime_v19/ue_full_import.json",
        "skeletal_morph_export": "ArtSource/monster/storage_owner_disguise/meshy_textured_runtime_v19/export_full_report.json",
        "main_video": "ArtSource/monster/storage_owner_disguise/meshy_textured_runtime_v19/neutral_main_take03_1x.mp4",
        "main_runtime": "ArtSource/monster/storage_owner_disguise/meshy_textured_runtime_v19/neutral_main_take03_runtime_qa.json",
        "side_video": "ArtSource/monster/storage_owner_disguise/meshy_textured_runtime_v19/neutral_side_1x.mp4",
        "side_runtime": "ArtSource/monster/storage_owner_disguise/meshy_textured_runtime_v19/neutral_side_runtime_qa.json",
        "farm_feature": "ArtSource/monster/storage_owner_disguise/meshy_textured_runtime_v19/farm_feature_qa.json",
        "limitations": [
          "Dense226359-triangle LOD0,435 morph targets; close-review asset exceeds draft game budget. Final LOD/memory optimization not certified.",
          "No native monster AI/locomotion integration or multiplayer pass.",
          "v18 contact integer-frame evidence retained; arbitrary subframes and garment self contacts not certified."
        ]
      }
    },
    "runtime": {
      "level": "preview",
      "engine": "UE5.8.3",
      "mesh": "/Game/Monsters/StorageOwnerDisguise/MeshyRuntimeV19/Assets/SK_CowTextured",
      "actor": "/Game/Monsters/StorageOwnerDisguise/MeshyRuntimeV19/BP_CowTexturedV19",
      "anim_bp": "/Game/Monsters/StorageOwnerDisguise/MeshyRuntimeV19/ABP_CowTexturedRuntimeV19",
      "verified": "Two complete real-time PIE state cycles with one persistent visible skeletal component; no GeometryCache/LevelSequence used. Live CL9 farm feature cycle completed; normal round bot recording ongoing.",
      "native_ai_connected": false,
      "notes": "Standalone visual-state actor with proximity input and explicit doff/don API. Native monster AI/damage/data rows and exposed locomotion remain incomplete; do not mark full game production complete."
    },
    "manifest": "ArtSource/monster/storage_owner_disguise/meshy_grasp_cycle_v18/manifest.json",
    "motion_design": {
      "candidate_version": "v19-motion-patch-v03",
      "scope": "short_release_drop_test_only",
      "source_runtime_version": "meshy-textured-runtime-v19",
      "approved_sketch": "v19-motion-sketch-v02",
      "manifest": "ArtSource/monster/storage_owner_disguise/v19_motion_patch_v03/manifest.json",
      "implementation": "Existing v02 cloth/body prefix reused. Authored gravity-like accelerated descent, floor folds and settling; cow hood also drops, low loose neck gathers at floor. No new physics solve.",
      "approval_status": "visual review pending"
    },
    "short_motion_test": {
      "version": "v19-motion-patch-v03",
      "video": "ArtSource/monster/storage_owner_disguise/v19_motion_patch_v03/review/cow_v19_release_drop_1x.mp4",
      "source": "ArtSource/monster/storage_owner_disguise/v19_motion_patch_v03/short_drop.blend",
      "qa": "ArtSource/monster/storage_owner_disguise/v19_motion_patch_v03/review/video_qa.json",
      "qa_scope": "75-frame video decode; original106 cloth frames exact; release-point downward travel and floor bounds. No new global body or self-collision pass.",
      "full_cycle_or_engine_replacement": false
    },
    "test_level_candidate": {
      "version": "runtime-walk-v20",
      "source_runtime": "meshy-textured-runtime-v19",
      "short_motion_source": "v19-motion-patch-v03",
      "user_authorization": "이쯤했으면 그냥만들자. 만들고 테스트레벨에서 옷벗고 돌아다니다가 다시 옷입고 돌아다니는거 만들어줘.",
      "scope": "Implement test-level native skeletal/morph runtime and show actual 1x PIE footage.",
      "source": "ArtSource/monster/storage_owner_disguise/runtime_walk_v20/cow_runtime_walk_v20.blend",
      "engine_directory": "/Game/Monsters/StorageOwnerDisguise/RuntimeWalkV20",
      "delivery_manifest": "ArtSource/monster/storage_owner_disguise/runtime_walk_v20/delivery_manifest.json",
      "video": "ArtSource/monster/storage_owner_disguise/runtime_walk_v20/review/cow_v20_doff_roam_same_skin_don_roam_1x.mp4"
    },
    "preserved_previous_runtime": "meshy-textured-runtime-v19"
  },
  "pending_review": {
    "stage": "engine_verify",
    "version": "runtime-walk-v20",
    "video": "ArtSource/monster/storage_owner_disguise/runtime_walk_v20/review/cow_v20_doff_roam_same_skin_don_roam_1x.mp4",
    "presented_at": "2026-10-16T10:20:30+00:00",
    "scope": "Actual test-level PIE native two-component skeletal/morph actor; explicitly requested test events, not full monster AI/navigation/combat integration."
  },
  "checkpoint": [
    {
      "path": "ArtSource/monster/storage_owner_disguise/blender_v02/approved_appearance_v02.blend",
      "sha256": "507724071490a12182b196925e744fade85b21dd340bde50ac4df16ccfbd6677"
    },
    {
      "path": "ArtSource/monster/storage_owner_disguise/motion_probe_v02/cow_pickup_probe_v02.blend",
      "sha256": "811b58f114183853ef321a6220cff98c71c6e04b9efa116442a7ea7a68254a34"
    },
    {
      "path": "ArtSource/monster/storage_owner_disguise/motion_probe_v02/pickup_probe_1x_review.mp4",
      "sha256": "8ab4df933a00fd1a85ff40ed5a4bac45b7c0d24f942f715e30e9c8b0291517b0"
    },
    {
      "path": "ArtSource/monster/storage_owner_disguise/motion_probe_v03/cow_pickup_clearance_v03.blend",
      "sha256": "f450ee8a211da13cf9dda17b717053c1d6cc33e4d7710a6984277bbdd88a87b7"
    },
    {
      "path": "ArtSource/monster/storage_owner_disguise/fit_rule_review_v01/size_options_review.png",
      "sha256": "ae89a7d44dd9a3bf659a80a53cc4f9b7fed3c49eb5727eeb407be1232b39166f"
    },
    {
      "path": "ArtSource/monster/storage_owner_disguise/animation_natural_v05/cow_production_v05.blend",
      "sha256": "79db40ed50dadc35ea531efcc161a9606720e0b756cdc2c7baac5793fc70a3c5"
    },
    {
      "path": "ArtSource/monster/storage_owner_disguise/animation_natural_v05/manifest.json",
      "sha256": "58534bcaf8b4f421741340bef7d6de43e6f44612360843ad3969b4e33bc489b5"
    },
    {
      "path": "ArtSource/monster/storage_owner_disguise/animation_natural_v05/ue_production_roundtrip_qa.json",
      "sha256": "f31d9105081e30f57acaa44894cdb34a58931871eb8f9f501ad278d32e000b27"
    },
    {
      "path": "ArtSource/monster/storage_owner_disguise/motion_probe_v16/cow_contact_dual_view_1x.mp4",
      "sha256": "c61751c45b19cf8e837801f4bea37505534ea37be4612b8d1b094c02d8429e82"
    },
    {
      "path": "ArtSource/monster/storage_owner_disguise/centered_cycle_v20/manifest.json",
      "sha256": "a3a10274d7f3f3ed75c1aa26e332e720bf4c367cd532f586a8c4ca7c63b63ff3"
    },
    {
      "path": "ArtSource/monster/storage_owner_disguise/centered_cycle_v20/review/cow_centered_cycle_v20_doff_don_1x.mp4",
      "sha256": "fdb0b4c9787bf17138cc06574abe38897447397cb1c663fb02d7ac1d8a60bfe0"
    },
    {
      "path": "ArtSource/monster/storage_owner_disguise/meshy_centered_cycle_v17/motion_corrected.blend",
      "sha256": "6ffaaf4b8f4aed48a69069079466b7045e2d5a52b09a18d61eaf95b3403d3976"
    },
    {
      "path": "ArtSource/monster/storage_owner_disguise/meshy_grasp_cycle_v18/review_finger_contact.blend",
      "sha256": "ad6ab065228f7f0ef41762f5decc51658b734386d824a6573bbc35d853c0a530"
    },
    {
      "path": "ArtSource/monster/storage_owner_disguise/meshy_grasp_cycle_v18/review_finger_contact.blend",
      "sha256": "9439b90c1d62ec003aa7bd6e6b219e53e9221f1e7277c020034a260518bbccf8"
    },
    {
      "path": "ArtSource/monster/storage_owner_disguise/v19_motion_patch_v02/manifest.json",
      "sha256": "969c8f3ff7d44369cb3569e9045c51fc13fe726ca5d35724d20ccf303304fcd3"
    },
    {
      "path": "ArtSource/monster/storage_owner_disguise/v19_motion_patch_v02/short_patch_final.blend",
      "sha256": "5233645eb4a9e2c2cbee432fe0ab38cdb87023cac8d5815269fdc6d9cde39e8c"
    },
    {
      "path": "ArtSource/monster/storage_owner_disguise/v19_motion_patch_v02/review/cow_v19_shoulder_pelvis_last_foot_patch_1x.mp4",
      "sha256": "46b313f9fda82b5a846fb39415f6a1183051b9be1c44d2c15e9189ccde249db5"
    }
  ],
  "job": null,
  "last_completed": "Test level doff/drop, bare movement, return/same garment pickup/don, dressed movement; actual runtime evidence/video saved.",
  "next_action": "User visual review of test-level flow; preserve v19 and previous videos. Remaining final optimization and any broader monster AI/combat integration are separate from this visual test actor.",
  "blocker": null,
  "engine": {
    "target": "Unreal Engine 5.8",
    "confirmation": "기존 CL9의 UE 5.8 기준으로 진행",
    "confirmed_at": "2026-10-13T22:05:33.821+09:00"
  },
  "source_baselines": [
    {
      "path": "ArtSource/monster/disguise_trio_preview_v1/reference/owners_reference.png",
      "sha256": "c69a9a47fc0d409585c043ee8eba2db77040872d57ef7d572f2a3d4fa046fd35"
    },
    {
      "path": "ArtSource/monster/disguise_trio_preview_v2/storage/storage_transform.blend",
      "sha256": "f5efc45c459d93ae11059deea1a6ef6457a708493f6ffdcbab7b8dbe4136c8ec"
    },
    {
      "path": "ArtSource/monster/disguise_trio_preview_v2/storage/preview/storage_transform.mp4",
      "sha256": "1a800d8d94637b3bfc8b4a23c40285d8f14145d4862a8396ac468d9c29f46a8d"
    }
  ],
  "feedback_history": [
    {
      "at": "2026-10-13T23:05:17.290+09:00",
      "version": "v01",
      "verbatim": "괴물이 좀더 무서웠으면좋겠어. 닭이랑 맞춰봐봐",
      "decision": "revise",
      "interpretation": "닭의 무표정·어두운 피부·각진 관절 분위기를 반영하되 소의 낮고 무거운 사족 체형과 귀여운 탈은 유지"
    },
    {
      "at": "2026-10-15T15:29:29.240299+00:00",
      "version": "meshy-centered-cycle-v17",
      "decision": "revise",
      "verbatim": "저 가죽을 손으로 잡아서 벗는느낌을내야돼",
      "interpretation": "Fingers must curl around the actual rim and pull; hold until fully extracted; release as clothing; pick up same dropped rim for independent redon."
    }
  ],
  "approval_history": [
    {
      "at": "2026-10-13T23:12:00.664+09:00",
      "stage": "concept",
      "version": "v03",
      "evidence": "ㅇㅇ 근데좀더 크게해줘 몬스터를",
      "scope": "v03 디자인 채택, 소 탈 대비 본체 크기 확대 조건; 실제 Blender 외형 승인은 아직 없음",
      "manifest_sha256": "45df835d1557bf88fae1009eec2f0b7efa1cca7cbb2a33c8dc77af5d7fcdf087"
    },
    {
      "at": "2026-10-13T14:35:26.901Z",
      "stage": "blender_appearance",
      "version": "blender_v02",
      "evidence": "이정도면돼 이어서해",
      "scope": "제시된 실제 Blender 확대 외형 승인. 리깅·접촉·수납·엔진 검수는 미실행.",
      "files": [
        {
          "path": "ArtSource/monster/storage_owner_disguise/blender_v02/storage_owner_review_v02.blend",
          "sha256": "507724071490a12182b196925e744fade85b21dd340bde50ac4df16ccfbd6677"
        },
        {
          "path": "ArtSource/monster/storage_owner_disguise/blender_v02/preview/01_size_comparison.png",
          "sha256": "9eafc891d372ac48070285efb0c230f266147d6b21dd669e076f6fc996be1b09"
        }
      ]
    },
    {
      "at": "2026-10-13T15:06:17.459Z",
      "stage": "transformation_fit_rule",
      "evidence": "현재 외형 유지 + 착용 중 본체 축소 허용",
      "decision": "A",
      "scope": "승인된 본체/탈 외형 유지. 착용·탈의 중 본체 전체 균일 축소/성장 허용. 같은 탈 회수와 접촉·수납 검수는 유지."
    },
    {
      "at": "2026-10-15T15:29:29.240299+00:00",
      "stage": "grasp_action_storyboard",
      "version": "grasp-storyboard-v01",
      "evidence": "ㅇㅇ 이 느낌으로 만들어줘. 손이 가죽을 확실히 잡고 당기게 하고, 완전히 벗어서 떨어뜨린 다음 같은 가죽을 주워 입는 것까지 원속도 영상으로 보여줘. 언리얼은 아직 하지말고",
      "scope": "Action direction approved; future Blender animation has not been reviewed.",
      "path": "ArtSource\\monster\\storage_owner_disguise\\meshy_grasp_cycle_v18\\approved_grip_storyboard.png"
    }
  ],
  "size_proposal": {
    "status": "appearance_approved_fit_unverified",
    "disguise_height_cm": 265,
    "monster_shoulder_height_cm": 340,
    "chicken_reference_height_cm": 390,
    "note": "닭 기준 높이 3.9m는 기존 제작 문서의 외형 가정. 소 본체를 탈보다 크고 넓게 제작하며 실제 바운드/수납은 별도 확인; 게임 판정 치수 변경 없음."
  },
  "transformation_rule": {
    "body_scale_change_allowed": true,
    "approval": "현재 외형 유지 + 착용 중 본체 축소 허용",
    "costume_scale_fixed": 1,
    "body_revealed_scale": 1,
    "body_worn_scale": 0.35,
    "individual_limb_stretch": false,
    "candidate_note": "Current centered cycle uses whole-body uniform 0.35 worn to 1.0 revealed; approved silhouettes retained. Prior 0.2 value belonged to earlier motion."
  },
  "motion_revision": {
    "feedback": "저게아니지. 닭봐봐 가죽을 벗고 나오는느낌이여야돼",
    "rejected_candidate": "animation_natural_v13 / rigid dorsal panels and airborne exit",
    "reference": "ArtSource/monster/chicken_disguise/animation_round_shrink_v1/preview/LV_Flat_ChickenShrinkLoop.mp4",
    "requirement": "Continuous soft costume peels/slips down the emerging body and collapses under its own weight; grounded broad quadruped emergence; preserve approved worn/revealed appearance and whole-body shrink permission.",
    "candidate_directory": "ArtSource/monster/storage_owner_disguise/centered_cycle_v20",
    "qa": {
      "scope": "3-second shoulder/arm withdrawal motion study, not complete doff/don",
      "frames": 91,
      "bad_frames": [
        22,
        23,
        24,
        25,
        26,
        27,
        28,
        29,
        30,
        31,
        32,
        33,
        34,
        35,
        36,
        37,
        38,
        39,
        40,
        41,
        42,
        43,
        44,
        45,
        46,
        47,
        48,
        49,
        50,
        51,
        52,
        53,
        54,
        55,
        56,
        57,
        58,
        59,
        60,
        61,
        62,
        63,
        64,
        65,
        66,
        67,
        68,
        69,
        70,
        71,
        72,
        73,
        74,
        75,
        76,
        77,
        78,
        79,
        80,
        81,
        82,
        83,
        84,
        85,
        86,
        87,
        88,
        89,
        90,
        91
      ],
      "max_outer_pairs": 1539,
      "max_liner_pairs": 1577,
      "min_body_z_cm": 2.3706430569291115,
      "self_collision": "not certified",
      "runtime": "not tested; Blender review only",
      "status": "fail"
    },
    "review_order": "Blender doff + same-skin pickup/don 1x video -> user direction review -> remaining technical corrections -> Unreal only after confirmation",
    "latest_feedback": "이 방향으로 더 자연스럽게 다듬고 관통도 고쳐줘. 완전히 벗고 바닥의 같은 가죽을 주워 입는 것까지 영상으로 보여줘. 언리얼은 아직 하지말고. 벗은건 옷이라서 바닥에 떨어져야돼. 옷처럼. 나중에 입는것도.",
    "approach": "Torso stays at x=-0.3m inside original costume footprint. Dorsal opening rolls down flanks; first arm folds up, exits, presses cloth down; opposite side remains over shoulder. No forward-sheet collapse.",
    "internal_trials": [
      {
        "version": "coupled-skin-v01",
        "decision": "internal reject: shoulder catch too low and collapsed skin too flat"
      },
      {
        "version": "coupled-skin-v02",
        "decision": "internal reject: body-facing control points create broad cloth/body penetration, foot floor errors in independent don",
        "evidence": "ArtSource/monster/storage_owner_disguise/motion_coupled_skin_v02/geometry_audit.json"
      }
    ],
    "current_work": {
      "timeline_seconds": 18,
      "doff_seconds": [
        0,
        8
      ],
      "independent_don_seconds": [
        8,
        18
      ],
      "mechanics": "centered asymmetric dorsal roll-down, all feet exit opening, full-size retreat, same fabric settles on floor with a curled grasp edge, crouch and actual thumb-surface pickup, foot entry, last arm withdrawal then closure",
      "known_contacts": "v20 saved/reloaded 1081 samples at 60Hz: zero body/garment exact crossing pairs. Held thumb/anchor normal clearance 4.5 cm; same garment rests stationary, start/end shape identical.",
      "rejected_automatic_correction": "v07 proxy inflation plus causal warm-start diverged; stopped only our background process, no candidate saved. Use bounded independent-frame correction after authored paths.",
      "prior_failure_evidence": "ArtSource/monster/storage_owner_disguise/centered_cycle_v13/contact_audit_60Hz.json",
      "self_contact": "Garment self intersections remain in retained sculpt and folded areas; secondary_qa.json records them. No claim of self-collision-free cloth or final game-ready animation."
    }
  },
  "previous_delivery_v13": {
    "source_blender_version": "blender_v02",
    "candidate_version": "animation_natural_v13",
    "manifest": "ArtSource/monster/storage_owner_disguise/animation_natural_v13/manifest.json",
    "qa": {
      "don_body_costume_lining_handle": "pass, 961 samples at 120Hz, zero crossing pairs",
      "handle_self": "pass, zero all Don 120Hz samples",
      "lining_self": "no interior intersections in sampled exact segment/barycentric audit; coincident seam boundaries are classified separately",
      "outer_self": "Known non-manifold exterior sculpt overlap: 228 static triangle pairs (227 in one worn-walk sample). Retained exterior detail, not a new opening/lining/handle penetration. Exact intersection audit and visual frames retained; not claimed intersection-free or watertight.",
      "engine_roundtrip": "576 Don bone samples, maximum error 0.0003954cm",
      "runtime_scope": "actual CL9 game-mode PIE visual state adapter; encounter/combat AI and full round playthrough are not covered",
      "doff_body_costume_lining_handle": "pass, 721 samples at 120Hz, zero crossing pairs",
      "visual_review": "neutral Eevee doff/don, flat texture gait/idle and close contact views, front/back risk segments, actual CL9 night video reviewed"
    },
    "runtime": {
      "level": "gameplay_verified",
      "engine": "5.8.3",
      "blueprint": "/Game/Monsters/StorageOwnerDisguise/RuntimeV03/BP_CowDisguiseV03",
      "anim_blueprint": "/Game/Monsters/StorageOwnerDisguise/RuntimeV03/ABP_CowDisguiseV03",
      "qa_map": "/Game/Monsters/StorageOwnerDisguise/RuntimeV03/QA_CowDisguise_GameV03",
      "game_mode": "BP_CL9GameMode",
      "gameplay_connected": true,
      "encounter_ai_connected": false,
      "scope": "same-costume visual state adapter in actual CL9 game-mode PIE; not combat AI or full-round playthrough",
      "evidence": "ArtSource/monster/storage_owner_disguise/animation_natural_v13/runtime_final_summary.json",
      "save_reload": "ArtSource/monster/storage_owner_disguise/animation_natural_v13/runtime_build_reload.json",
      "normal_speed_video": "ArtSource/monster/storage_owner_disguise/animation_natural_v13/review/cow_runtime_dark_v03_1x.mp4",
      "performance_benchmark": "not_run; concurrent editor/capture workload"
    },
    "source_file": {
      "path": "ArtSource/monster/storage_owner_disguise/animation_natural_v13/cow_production_v13.blend",
      "sha256": "2ee0bba80250c2b1f82ff7be93f62bd22b23d65b4e0943943dd8de76e0c7f740"
    },
    "review_videos": [
      {
        "path": "review\\cow_don_1x.mp4",
        "frames": 240,
        "fps": 30,
        "seconds": 8.0,
        "sha256": "f36b03adfb81b0aabc9ccefe4775e3bd550d078f8535abb5cbade9e8ab1016de"
      },
      {
        "path": "review\\cow_doff_1x.mp4",
        "frames": 180,
        "fps": 30,
        "seconds": 6.0,
        "sha256": "895a4097c694a4dca8eca8fa219875280b49cc3346ac7703f627c7c8e2fdfcb4"
      },
      {
        "path": "review\\cow_walk_1x.mp4",
        "frames": 72,
        "fps": 30,
        "seconds": 2.4,
        "sha256": "4866daf7d7244fcd6dd6aca0548932a55414f6bf6747ef00840f025aee919fd2"
      },
      {
        "path": "review\\cow_run_1x.mp4",
        "frames": 27,
        "fps": 30,
        "seconds": 0.9,
        "sha256": "74316cabbc3aed6bc4a8f8fd01def625e545107cfb010d9fb881461afd8714d8"
      },
      {
        "path": "review\\cow_backwalk_1x.mp4",
        "frames": 72,
        "fps": 30,
        "seconds": 2.4,
        "sha256": "d94230c2d38408479088e235ec2079756b7517b211dc96c05c483728fb80a911"
      },
      {
        "path": "review\\cow_wornwalk_1x.mp4",
        "frames": 36,
        "fps": 30,
        "seconds": 1.2,
        "sha256": "7f26ee5265c6a32f3bd5eb6dc605c3b046f6225be9095f096ee69184aec13dd0"
      },
      {
        "path": "review\\cow_revealedidle_1x.mp4",
        "frames": 90,
        "fps": 30,
        "seconds": 3.0,
        "sha256": "da6ec065eb810cc57c904aabcdaccf0f5db1f1075fa0c60c40fa63e9cf6ee2b4"
      },
      {
        "path": "review\\cow_wornidle_1x.mp4",
        "frames": 90,
        "fps": 30,
        "seconds": 3.0,
        "sha256": "ebb2aec61903a4eaf372eb7886db45a631008511e83d7a5145eb8d37ba8b64b4"
      },
      {
        "path": "review\\cow_emptycostume_1x.mp4",
        "frames": 30,
        "fps": 30,
        "seconds": 1.0,
        "sha256": "8f47add3fa41348fc882a61d259bfb32a15ee5352c39b3ea6edbbe29eac5ccfc"
      },
      {
        "path": "review\\cow_don_risk_38_110_1x.mp4",
        "frames": 73,
        "fps": 30,
        "seconds": 2.433333333333333,
        "sha256": "9a9f2c9acd08f544235fd045e69da345fdd79b0b82a4bd7ec6c1ad6a4acfdbbb"
      },
      {
        "path": "review\\cow_don_risk_145_180_1x.mp4",
        "frames": 36,
        "fps": 30,
        "seconds": 1.2,
        "sha256": "cf38e5c2c8666dc0ddb9e9805cc10dd637a82c6a662028aa86202bdb8410d82e"
      },
      {
        "path": "review\\cow_doff_risk_40_100_1x.mp4",
        "frames": 61,
        "fps": 30,
        "seconds": 2.033333333333333,
        "sha256": "387e491b279f5628aa9bb0a8da9419ea989efb1eabe8dc3525127698f6e114c6"
      },
      {
        "path": "review\\cow_contact_close_1x.mp4",
        "frames": 73,
        "fps": 30,
        "seconds": 2.433333333333333,
        "sha256": "878767d06cf3d3917c6b7bd0cab19d8e8e9fbb0e2fce0c1beeebffed60802108"
      },
      {
        "path": "review\\cow_off_on_full_1x.mp4",
        "frames": 420,
        "fps": 30,
        "seconds": 14,
        "sha256": "b92bb5fa7649678302fc2ea6f7434817ec72fb106e57b3e1aec95e3365c74d30"
      },
      {
        "path": "C:\\Work\\CL9\\ArtSource\\monster\\storage_owner_disguise\\animation_natural_v13\\review\\cow_runtime_dark_v03_1x.mp4",
        "frames": 1558,
        "seconds": 51.93333333333333,
        "fps": 30,
        "source_capture_frames": 2081,
        "source_capture_average_fps": 24.01892890120037,
        "speed": "1x wall-clock timestamps; duplicates only, no interpolation/retiming",
        "capture_offset_seconds": 10.485767364501953,
        "scope": "Actual PIE asset state test, not encounter AI or full round gameplay",
        "sha256": "33d6f62b45c645b7cb0f857d0c6fc060967dbaf1962766d8f7808ab6630d1303"
      }
    ]
  },
  "rejected_motion_candidates": [
    {
      "version": "skin-probe-v12",
      "feedback": "가죽이 몸에 걸렸다가 벗겨지는 느낌이 더 나야돼. 지금은 탈이 따로 떨어져서 납작해지는 느낌이야. 닭처럼 자연스럽게 고치고 언리얼 말고 영상부터 다시 보여줘. 다시 입을 수도 있어서 입는 것까지 고려.",
      "previous_production": {
        "source_blender_version": "blender_v02",
        "candidate_version": "skin-probe-v12",
        "manifest": "ArtSource/monster/storage_owner_disguise/motion_probe_skin_v12/manifest.json",
        "source_file": {
          "path": "ArtSource/monster/storage_owner_disguise/motion_probe_skin_v12/cow_skin_probe_v12.blend",
          "sha256": "15a1c178251fc5bc393e41e8fd660197af4ff04eab938b7a89241b16e940a907"
        },
        "qa": {
          "body_skin": "fail",
          "bad_frames": [
            16,
            17,
            18
          ],
          "max_outer_pairs": 66,
          "max_inner_pairs": 73,
          "minimum_body_z_cm": -3.472280874848366,
          "garment_self": "not passed; visible folding/interior overlap remains",
          "feet": "requires correction",
          "scope": "91 integer frames, body/outer/lining triangle tests; no subframe pass claimed"
        },
        "runtime": {
          "level": "not_tested",
          "reason": "Current v12 Blender motion video must be reviewed before further UE work."
        }
      },
      "previous_review": {
        "stage": "motion_probe",
        "version": "skin-probe-v12",
        "presented_at": "2026-10-14T01:24:29.463388+00:00",
        "evidence": [
          "ArtSource/monster/storage_owner_disguise/motion_probe_skin_v12/review/cow_skin_doff_probe_v12_1x.mp4"
        ],
        "question": "가죽이 벗겨져 내려앉는 동작 방향 확인. 접촉/주름은 아직 보정 중이며 최종 기술 합격본이 아님."
      }
    },
    {
      "version": "coupled-skin-v04",
      "feedback": "옷을 벗고 나오는 느낌이없는데",
      "previous_production": {
        "source_blender_version": "blender_v02",
        "candidate_version": "coupled-skin-v04",
        "manifest": "ArtSource/monster/storage_owner_disguise/motion_coupled_skin_v04/manifest.json",
        "source_file": {
          "path": "ArtSource/monster/storage_owner_disguise/motion_coupled_skin_v04/cow_coupled_skin_v04.blend",
          "sha256": "db136d7f0a5ce0681c90052028bb7a4c5ccd2dee41bf2175a7de135cfbe14377"
        },
        "qa": {
          "body_skin": "fail",
          "bad_frame_count": 259,
          "max_outer_pairs": 1654,
          "max_liner_pairs": 1664,
          "minimum_body_z_cm": -12.076885998249054,
          "garment_self": "not certified; retained source sculpt and folded geometry need separate exact inspection",
          "scope": "421 integer frames at 30fps; exact triangle interior segment tests after BVH broadphase; not subframe or self-intersection certification",
          "control_max_held_lip_error_cm": 7.673275988644742,
          "clearance_max_displacement_cm": 145.06025863915684,
          "video_decode": "pass",
          "max_crossing_segment_cm": 15.53969844836354
        },
        "runtime": {
          "level": "not_tested",
          "reason": "User explicitly requested Blender video first; no Unreal operations for this revision."
        }
      },
      "previous_review": {
        "stage": "motion_probe",
        "version": "coupled-skin-v04",
        "presented_at": "2026-10-14T03:16:25.278769+00:00",
        "evidence": [
          "ArtSource/monster/storage_owner_disguise/motion_coupled_skin_v04/review/cow_coupled_skin_v04_doff_don_1x.mp4",
          "ArtSource/monster/storage_owner_disguise/motion_coupled_skin_v04/review/cow_peel_contact_v04_1x.mp4",
          "ArtSource/monster/storage_owner_disguise/motion_coupled_skin_v04/review/cow_pickup_pull_v04_1x.mp4"
        ],
        "question": "가죽이 몸에 걸려 벗겨지는 흐름과 바닥의 같은 가죽을 집어 다시 입는 방향 검토. 기술 QA 합격본은 아님."
      }
    }
  ],
  "motion_direction_approvals": [
    {
      "version": "centered-peel-v02",
      "user": "이 방향으로 더 자연스럽게 다듬고 관통도 고쳐줘. 완전히 벗고 바닥의 같은 가죽을 주워 입는 것까지 영상으로 보여줘. 언리얼은 아직 하지말고. 벗은건 옷이라서 바닥에 떨어져야돼. 옷처럼. 나중에 입는것도.",
      "scope": "direction only, not technical QA approval"
    }
  ],
  "generator_comparison": {
    "version": "meshy-compare-v01",
    "status": "placed_preview_verified",
    "model": "Meshy 6 Lite",
    "rigging_test": {
      "type": "Meshy quadruped canine",
      "bones": 27,
      "unweighted_body_vertices": 0,
      "walk_seconds": 1,
      "review_video": "ArtSource/monster/storage_owner_disguise/meshy_compare_v01/rig_test/meshy_autorig_walk_1x.mp4",
      "conclusion": "Editable skeleton and weights save initial setup work. Shoulder collapse and ground contact issues remain; no individual digits or jaw bone. Needs custom additions for garment handling. Smart Rig Beta not tested."
    },
    "license": "CC BY 4.0",
    "credits_spent": 10,
    "comparison_image": "ArtSource/monster/storage_owner_disguise/meshy_compare_v01/comparison_board.jpg",
    "result": "User selected the Meshy supplemented grounded probe for an isolated Unreal preview. Original authored full cow production is preserved; the Meshy candidate does not replace full runtime behavior.",
    "manifest": "ArtSource/monster/storage_owner_disguise/meshy_compare_v01/manifest.json",
    "submitted": true,
    "scope": "Separate one-model Meshy comparison, existing cow animation candidate unchanged. Same approved input; existing baseline is TRELLIS.2, not Tripo.",
    "custom_garment_rig_test": {
      "version": "peel_probe_v06",
      "video": "ArtSource/monster/storage_owner_disguise/meshy_compare_v01/peel_probe_v06/direct_vs_meshy_peel_1x.mp4",
      "manifest": "ArtSource/monster/storage_owner_disguise/meshy_compare_v01/peel_probe_v06/manifest.json",
      "qa": {
        "direct": {
          "samples": 16,
          "samples_with_crossing": 0,
          "max_crossing_triangle_pairs": 0
        },
        "meshy": {
          "samples": 16,
          "samples_with_crossing": 6,
          "max_crossing_triangle_pairs": 121
        }
      },
      "recommendation": "Keep existing custom cow rig for garment animation. No demonstrated benefit from replacing it with this Meshy canine rig after required manual additions and contact fixes."
    },
    "grounding_followup": {
      "version": "peel_probe_v11",
      "feedback": "둘다 다리쪽이 너무 이상한데? 좀 땅에 닿아야되는거아니냐",
      "scope": "Correct ground support in both sides of the three-second Meshy-versus-custom comparison, not a full doff/don replacement.",
      "video": "ArtSource/monster/storage_owner_disguise/meshy_compare_v01/peel_probe_v11/direct_vs_meshy_peel_1x.mp4",
      "manifest": "ArtSource/monster/storage_owner_disguise/meshy_compare_v01/peel_probe_v11/manifest.json",
      "qa": {
        "direct": {
          "support_foot_min_clearance_cm": -0.14766150712966913,
          "support_foot_max_clearance_cm": 0.10672515630722051,
          "support_samples_outside_0_5cm": 0,
          "all_body_min_clearance_cm": -0.5301132202148438
        },
        "meshy": {
          "support_foot_min_clearance_cm": -0.0473834909498691,
          "support_foot_max_clearance_cm": 0.03244473040103918,
          "support_samples_outside_0_5cm": 0,
          "all_body_min_clearance_cm": -0.5000402554869652
        }
      },
      "remaining": "Ground-support criteria pass; garment clearance and Meshy volume/weight defects remain. Earlier ungrounded naturalness comparison is qualified.",
      "changes": "Both grounded: fixed hind support and alternating front-limb exit, sole-surface calibration, 60Hz bake and quaternion sign continuity. Meshy planted-palm weights corrected; garment clearance remains unfinished.",
      "status": "review_ready",
      "user_direction_approval": {
        "user": "좋아 이대로 배치하자",
        "client_date": "2026-10-15",
        "applies_to": "grounded-peel-probe-v11 grounding/motion direction",
        "selection": "Meshy supplemented, explicitly selected 2026-10-15",
        "technical_qa": "Approval does not erase recorded cloth intersections or imply completed full doff/don."
      }
    },
    "placement_request": {
      "status": "placed_preview_verified",
      "selected_candidate": "Meshy supplemented",
      "selection_quote": "Meshy 보강본을 배치",
      "client_date": "2026-10-15",
      "destination": "Unreal isolated review level",
      "level": "/Game/Monsters/StorageOwnerDisguise/MeshyReviewV01/QA_MeshyGroundedReview",
      "sequence": "/Game/Monsters/StorageOwnerDisguise/MeshyReviewV01/LS_MeshyGrounded_3s",
      "source_clip_seconds": 3,
      "actor_scale": [
        1,
        1,
        1
      ],
      "preserve_existing": true,
      "scope_note": "Selected 3-second grounded probe placed for preview. Not a full doff/don/locomotion or gameplay AnimBP replacement.",
      "directory": "ArtSource/monster/storage_owner_disguise/meshy_compare_v01/peel_probe_v11/unreal_placement_v01",
      "import_qa": {
        "MeshyGrounded": {
          "duration_s": 3.0,
          "bone_sample_count": 370,
          "max_bone_error_cm": 0.00029963314768339486,
          "morph_targets": 0,
          "max_active_morph_error": null,
          "pass": true
        },
        "SameCoatGrounded": {
          "duration_s": 3.0,
          "bone_sample_count": 720,
          "max_bone_error_cm": 0.00033023444682299814,
          "morph_targets": 91,
          "max_active_morph_error": 0.0,
          "pass": true
        }
      },
      "remaining": [
        "Body/coat crossings remain in grounded probe.",
        "Full same-coat pickup/don, cloth contact and final locomotion still require production work.",
        "AI/AnimBP game-state integration and multiplayer validation not completed for this Meshy candidate."
      ],
      "video_qa": {
        "file": "ArtSource\\monster\\storage_owner_disguise\\meshy_compare_v01\\peel_probe_v11\\unreal_placement_v01\\meshy_unreal_placement_1x.mp4",
        "duration_s": 6,
        "fps": 30,
        "frames": 180,
        "size": [
          1280,
          832
        ],
        "source_clip_s": 3,
        "views": [
          "garment included",
          "body-only side ground audit"
        ],
        "speed": 1,
        "full_decode_pass": true,
        "capture_method": "Frame-stepped actual PIE; not wall-clock performance recording",
        "sha256": "273b95bb74975d64069072879a348c6d07d0ca7d6369894433ba50b678a69577",
        "watermark": "Meshy 6 Lite + Auto Rig + Blender manual supplement | v01 | 1x | UE 5.8 PIE | frame capture",
        "live_autoplay_samples": 29,
        "observed_autoplay_wraps": 2,
        "all_live_samples_playing": true,
        "runtime_max_bone_error_cm": 0.005473884418072214,
        "runtime_max_active_curve_error": 7.152557373046875e-06,
        "runtime_sample_rows": 18,
        "readback_stills": [
          "readback_01.jpg",
          "readback_02.jpg",
          "readback_03.jpg"
        ],
        "cloth_qa_pass": false,
        "game_AI_AnimBP_connected": false
      },
      "saved_reload_pie": {
        "saved_level_reloaded": "/Game/Monsters/StorageOwnerDisguise/MeshyReviewV01/QA_MeshyGroundedReview.QA_MeshyGroundedReview",
        "sequence": "/Game/Monsters/StorageOwnerDisguise/MeshyReviewV01/LS_MeshyGrounded_3s.LS_MeshyGrounded_3s",
        "display_rate": [
          60,
          1
        ],
        "range_frames": [
          0,
          180
        ],
        "auto_play": true,
        "actor_play_rate": 1.0,
        "tracks": [
          {
            "binding": "MeshyGrounded",
            "animation": "/Game/Monsters/StorageOwnerDisguise/MeshyReviewV01/MeshyGrounded/AN_MeshyGrounded.AN_MeshyGrounded",
            "play_rate": 1.0
          },
          {
            "binding": "SameCoatGrounded",
            "animation": "/Game/Monsters/StorageOwnerDisguise/MeshyReviewV01/SameCoatGrounded/AN_SameCoatGrounded.AN_SameCoatGrounded",
            "play_rate": 1.0
          }
        ],
        "floor_top_z_cm": 1.5,
        "camera": {
          "location": [
            650.0,
            1050.0,
            430.0
          ],
          "rotation": [
            -2.3179006802820368e-07,
            -13.95626163482666,
            -124.0661849975586
          ]
        },
        "preview_only": true
      },
      "engine_editor_open": "QA_MeshyGroundedReview with LS_MeshyGrounded_3s open at 2 seconds",
      "capture_rejected": {
        "directories": [
          "pie_frames",
          "pie_frames_v02"
        ],
        "reason": "Automation high-resolution screenshot tasks stalled game ticks during chained Slate captures. Runtime pose/morph checks failed; these captures are excluded from delivery. Actual Slate PIE screenshot capture v03 passed."
      },
      "original_v20_preserved_sha256": "6c1ca33ab096c22e07876102bebe69e6de454426379ac85c41c5bf022d209a68",
      "license": {
        "body_base": "Meshy 6 Lite, CC BY 4.0, credit Meshy",
        "rig": "Meshy canine auto rig + manually added digits and root",
        "motion": "Our Blender-authored motion and ground support",
        "coat": "Existing approved cow costume and same-coat morphs; not Meshy-generated"
      }
    },
    "full_cycle_rework": {
      "status": "corrected_blender_motion_review_ready",
      "user_request": "Meshy 보강본으로 가죽 관통 수정, 완전히 벗어 바닥에 떨어진 같은 옷을 주워 입기, 나오거나 일어날 때 고개 움직임, 원속도 영상",
      "selected_body": "Meshy 6 canine auto-rig supplement + Blender manual motion/support corrections",
      "scope": "Complete independent Blender doff/drop/pickup/don motion review before any further Unreal integration",
      "appearance_approval": "Preserved previously approved large cow body and same cow hide; user-approved uniform body scale change while worn/inside hide retained.",
      "current_directory": "ArtSource/monster/storage_owner_disguise/meshy_full_cycle_v18",
      "source_probe": "meshy_compare_v01/peel_probe_v11",
      "duration_seconds": 20,
      "fps": 30,
      "same_garment": "Existing SK_SameCowCostume_001 / SAME_COSTUME_001 (7,426 vertices), original centered_cycle_v20 corrective action and 72-bone garment rig reused; same mesh remains the dropped and re-worn hide.",
      "head_motion": "Authored pitch range -24.0642 to +21.1994 degrees and yaw -12.6051 to +9.1673 degrees; anticipatory duck, lift/glance on emergence, look toward floor, duck/reorient on re-entry.",
      "completed": [
        "v16 contact sheet exposed the readability bug: body scale was 2-8% before the hide cleared and shrank while still outside before redon.",
        "v18 preserves the clearance-safe scale through the peel, grows after clearance, remains full size while outside beside the same hide, and reduces as it approaches the hide entry.",
        "Same-hold doff/drop/pickup/don remains a 20s sequence with head pitch/yaw motion and previously approved body scale rule.",
        "Exact evaluated body-vs-hide triangle crossing QA: 0 pairs in all 601 sampled scene poses at 30fps.",
        "Two-view 1.0x watermarked Blender review video completed and full decode passed: 600 frames, 20.0s, 1280x360."
      ],
      "qa_not_passed": [
        "Subframe body-vs-hide motion has not been certified.",
        "Hide self-contact/self-intersection is not certified.",
        "No v18 Unreal import, AnimBP/runtime, PIE, or actual in-game capture; this is Blender Motion Review only."
      ],
      "deliverables": {
        "video": "ArtSource/monster/storage_owner_disguise/meshy_full_cycle_v18/review/cow_meshy_full_cycle_v18_1x.mp4",
        "manifest": "ArtSource/monster/storage_owner_disguise/meshy_full_cycle_v18/manifest.json",
        "body_coat_qa": "ArtSource/monster/storage_owner_disguise/meshy_full_cycle_v18/body_coat_contact_qa.json",
        "diagnostic_contact_sheet": "ArtSource/monster/storage_owner_disguise/meshy_full_cycle_v18/review/emergence_contact_sheet.jpg"
      },
      "video_sha256": "3c1b1990b3dcfff3594467ba7c87b15f90228b5384b87fb52fb372b0be57b640",
      "excluded": [
        "v17 and v19 exploratory scale/retiming candidates failed full-frame body-vs-hide crossing QA and are preserved but not deliverables."
      ],
      "engine": "Blender review only. Unreal integration and actual in-game review remain pending; do not treat this video as a game capture."
    }
  },
  "history": [
    {
      "at": "2026-10-16T00:46:01+00:00",
      "event": "user_engine_authorization",
      "quote": "텍스쳐 넣어서 언리얼로 제대로 만들어서 영상 찍어 줘",
      "effect": "Prior Unreal prohibition superseded by explicit user request. Preserve approved appearance, v18 grasp/pull/drop/same-hide pickup/don and head motion."
    },
    {
      "at": "2026-10-16T01:35:31+00:00",
      "event": "motion_sketch_review",
      "user_feedback": "손은 제대로 잡고있는데 다른부분이 그냥 좀 어색하게 흘러갓. 스케치로 자세하게 만들어보자",
      "version": "v19-motion-sketch-v01",
      "scope": "Two six-pose AI sketch sheets; no model/engine modification."
    },
    {
      "at": "2026-10-16T02:11:22+00:00",
      "event": "motion_sketch_revision",
      "version": "v19-motion-sketch-v02",
      "user_request": "Keep v19 body and grasp; correct hand side and feet positions first; clearly show shoulder snag, pelvis and rearfoot clearance.",
      "scope": "Six-pose sketch only; earlier eight-pose intermediates not accepted due ending hand swap. No model/engine/video changes."
    },
    {
      "at": "2026-10-16T03:58:33+00:00",
      "event": "user_authorized_short_motion_patch",
      "scope": "Apply approved sketch order to v19 shoulder through last hindtoe only. Preserve proportions and grasp impression; individual digit detail is not required. Garment should sag/fall like clothing under gravity. Show local 1x video; do not recapture full cycle.",
      "version": "v19-motion-patch-v01"
    },
    {
      "at": "2026-10-16T05:08:38+00:00",
      "event": "short_motion_review",
      "version": "v19-motion-patch-v02",
      "user_scope": "Keep v19 body/grasp, show only shoulder through last rearfoot at 1x; clothing should drape/fall naturally, per-digit refinement not required, no full reshoot.",
      "result": "3.5s two-view Blender prototype; original body/bind exact, held anchor maintained, scoped major-body contact cleanup; original runtime preserved.",
      "efficiency": "Frames1-41 cache/body unchanged and reused; only frames42-105 re-rendered after later carry-path fix; one remaining failed frame90 repaired locally."
    },
    {
      "at": "2026-10-16T09:14:00+00:00",
      "event": "short_release_drop_review",
      "version": "v19-motion-patch-v03",
      "user_scope": "Do not overthink physical equations; garment released from hand must fall.",
      "result": "Existing v02 cloth/body prefix reused. Authored gravity-like accelerated descent, floor folds and settling; cow hood also drops, low loose neck gathers at floor. No new physics solve.",
      "video": "ArtSource/monster/storage_owner_disguise/v19_motion_patch_v03/review/cow_v19_release_drop_1x.mp4"
    },
    {
      "at": "2026-10-16T09:56:28+00:00",
      "event": "test_level_execution_authorized",
      "version": "runtime-walk-v20"
    },
    {
      "at": "2026-10-16T10:20:30+00:00",
      "event": "test_level_delivery",
      "version": "runtime-walk-v20"
    }
  ],
  "pause_note": "User stopped work and raised token waste. Neutral 1x engine videos preserved. Farm feature cycle recorded; full-round film failed when engine crashed on existing Lamp_16 PointLight/SpotLight object-name collision during return. Do not claim full-round completion."
}
```


### 소 컨셉 v01 제시 — 2026-10-13T22:07:15.373+09:00

사용자 엔진 기준 확인: “기존 CL9의 UE 5.8 기준으로 진행”. 이번 제작에는 기존 Blender→UE 5.8 절차를 적용한다.

![소 탈 몬스터 컨셉 v01](../../../ArtSource/monster/storage_owner_disguise/concept_v01/cow_disguise_concept_v01.png)

- 제작: 내장 `image_gen.imagegen`. [프롬프트](../../../ArtSource/monster/storage_owner_disguise/concept_v01/prompt.txt), [manifest](../../../ArtSource/monster/storage_owner_disguise/concept_v01/manifest.json). 특정 채팅 모델/3D 생성기 출처를 추정하지 않는다.
- 디자인: 귀여운 소 탈·단단한 뿔·목 뒤부터 등까지의 개구부·보강 집기 탭, 낮고 두꺼운 사족 본체.
- 이동 계획: 착용 상태는 짧은 보폭의 네발 걸음, 본체는 낮은 무게중심과 앞발 지지가 읽히는 무거운 네발 걸음. 닭의 리그/팔다리/타이밍을 복제하지 않는다.
- 탈착 계획: 벗기는 한 앞발 지지→어깨/등 통과→뒷발 순차 회수. 입기는 세 발 지지→같은 탈의 탭 집기/정렬→관절 접기/수납→마지막 앞발 회수로 별도 제작한다.
- 아직 검증하지 않은 것: 같은 크기의 몸이 접혀 실제로 들어가는지, 개구부 통과, 발가락/집기 접촉, 탈 두께/관통, 리그 및 UE 재현. 컨셉의 수납 그림을 기술 합격으로 보지 않는다.
- 당시 검토 대상은 **컨셉 v01**이었다. 이후 공포감 강화 피드백으로 수정했으며 현재 대상은 아래 **v03**이다. 실제 Blender 외형은 컨셉 확인 후 제작하여 별도 검토한다.


### 소 컨셉 v03 — 닭과 공포감 맞춤 · 2026-10-13T23:09:02.723+09:00

사용자 피드백: “괴물이 좀더 무서웠으면좋겠어. 닭이랑 맞춰봐봐”. v01 승인으로 해석하지 않고 수정 요청으로 기록한다.

닭 정본 `approved_chicken_disguise_20261011.png`와 제작용 `monster_fullbody.png`를 직접 확인했다. 닭의 무표정·어두운 피부·각진 어깨/관절을 공통 분위기로 삼으며, 흰 타원 얼굴과 긴 인간형 팔다리는 소에 복제하지 않는다.

![소 탈 몬스터 컨셉 v03](../../../ArtSource/monster/storage_owner_disguise/concept_v03/cow_disguise_concept_v03.png)

- v02: 재질·관절 분위기 비교안. 둥근 눈/웃는 듯한 입이 남아 자체 검토 후 v03으로 보정했다. [보존 이미지](../../../ArtSource/monster/storage_owner_disguise/concept_v02/cow_disguise_concept_v02.png).
- v03: 낮게 숨긴 쐐기 머리·깊은 눈·검은 입 내부·가는 불규칙 이빨, 넓게 짚는 앞발을 적용했다. 귀여운 소 탈과 무거운 사족 체형 유지. 비교 패널은 실루엣/분위기 비교이며 확정 스케일이 아니다.
- 제작: 내장 `image_gen.imagegen` 편집. [최종 프롬프트](../../../ArtSource/monster/storage_owner_disguise/concept_v03/prompt.txt), [manifest/해시](../../../ArtSource/monster/storage_owner_disguise/concept_v03/manifest.json).
- 기술 검수: 모델·리그·동작·실제 수납/접촉·UE 모두 미실행. 피부의 밝은 균열 무늬는 모델 재질에서 줄여 가죽으로 읽히게 조정할 항목.
- 다음: 컨셉 v03 확인 → 실제 Blender 외형 초안/다각도 검토 → 승인 후 어려운 2~3초 탈착 시험. 전체 production 범위 유지.


### 실제 Blender v02 외형 승인 — 2026-10-13T14:35:26.901Z

사용자: “이정도면돼 이어서해”. 높이 3.4m의 큰 본체와 2.65m 소 탈 외형을 채택했다. 승인 사본 `blender_v02/approved_appearance_v02.blend`와 제시 이미지 해시를 보존했다. 다음은 2~3초 접촉·수납 시험이며 외형 승인을 동작·수납 합격으로 해석하지 않는다.


### 2.4초 접촉·수납 시험 및 첫 UE 왕복 — 2026-10-13T14:53:48.537Z

- v01: 생성된 분리 표면의 자동 웨이트 실패. 거리 기반 임시 웨이트는 큰 관절 접기에서 표면이 찢어져 최종 리그로 채택하지 않음. 기존 파일과 1x 영상 보존.
- v02: 승인 형상의 연속 변형 표면을 별도 생성(6.5cm voxel). 23,768 vertices, non-manifold edge 0. 이후 시험 프록시에 bone heat 자동 웨이트 성공. 중립 클레이 재질은 변형 검수용이며 승인 외형/최종 텍스처 대체 아님.
- 같은 탈 하나(SAME_COSTUME_001), 30fps·2.4초, 손목 IK 및 38cm 들어 올리기. 관절 scale 1 유지. 손가락 집기, 전체 수납, 탈 천 변형은 미합격.
- UE 5.8.3 Interchange: `/Game/Tests/CowDisguiseContactProbeV01` 격리 임포트. 2.4000001초 클립, 4개 시각의 모든 본 위치를 Blender와 비교해 최대 오차 0.000194cm. 이는 skeletal roundtrip이며 실제 게임 AI/AnimBP 연결·PIE 검수 완료가 아님. 명시적 PNG 재질 전달 보완.
- 원본 직접 체적은 내부 분리 표면 때문에 무효. 닫힌 외곽으로 재측정한 근사 부피는 본체 19.416m³ / 탈 전체 11.932m³(뿔·굽 포함). 착용 중 축소 허용 또는 탈 확대 중 사용자 선택 질문을 보냈으며, 답변 전 변신 크기 변경을 적용하지 않음. 이 측정은 모든 접기 자세의 불가능 증명이 아님.


### 저장된 다음 작업 — 2026-10-13T15:02:28.202Z

- [1배속 2.4초 집기 시험](../../../ArtSource/monster/storage_owner_disguise/motion_probe_v02/pickup_probe_1x_review.mp4): 960×600, 30fps, 72프레임 전체 decode 확인. 우측 상단 제작 방식·버전·1x·Blender 및 미합격 표시. 원속도 영상 파일 검수만 통과하며 손가락 접촉/수납은 미합격.
- v03 위치 탐색: 같은 탈을 (+0.2,-1.4,0)m 이동한 후보에서 시작 프레임 본체/탈 triangle intersection 0. 접촉 시 84~87쌍 남음. 손가락/탭 수정 필요. 최대 IK 오차 0.000535cm. 게임용 대비 웨이트를 4개로 제한/정규화했으나 최종 리그 합격 아님.
- [크기 선택 실제 Blender 비교](../../../ArtSource/monster/storage_owner_disguise/fit_rule_review_v01/size_options_review.png): A 기존 2.65m 탈 유지 + 착용 중 본체 크기 변화 허용 여부, B 탈 1.4배(3.71m) 검토용 후보. B는 수납 검증/승인 전이며 생산 원본에 적용하지 않음.
- 기존 컨셉·옛 소 변신 원본 3개 해시 재검증 모두 유지. Blender 사용자 라이브 씬 미변경. 에디터 임대 해제, UE는 열린 상태 유지.
- 현재 요청 범위 production 유지. 전체 벗기/재착용/이동/실제 인게임 검수와 워터마크 인게임 영상은 아직 남아 있다.


### 2026-10-14 수납 규칙 A 승인

사용자: “현재 외형 유지 + 착용 중 본체 축소 허용”. 소 탈 2.65m와 본체 3.4m 승인 외형은 그대로 유지하며 착용/탈의 중 본체의 균일 축소/성장을 허용한다. 앞선 일반 규칙의 scale 금지는 이번 소 본체 전체 크기 변신에 한해 사용자 지시로 예외 적용한다. 탈 자체 크기와 개별 팔다리 비율은 유지하고 실제 접촉·수납·같은 탈 회수는 계속 검증한다.


### 소 본제작 v05 — 축소 승인 이후

사용자 승인 “현재 외형 유지 + 착용 중 본체 축소 허용”을 적용했다. 본체 전체 scale 1↔0.20이며 탈은 scale 1, 개별 사지 길이 변화 없음. 열리는 등 패널과 실제 연속 안감, 변형되는 천 손잡이를 제작했다. 천은 키프레임 변형이며 물리 천 시뮬레이션으로 표시하지 않는다. 손잡이는 일부 손가락으로 거는 동작이며 모든 손가락의 완전 밀착으로 주장하지 않는다.

3초 접촉 시험 v16(68본)의 UE 최대 위치 오차 0.000364cm. 이후 4개 굽 본을 추가한 본제작은 72본이며 9클립 각3시각 왕복 최대 오차 0.000361cm. 본체 LOD0 28,518tri, 탈·안감·손잡이 합계33,106tri; 두 외형을 동시에 유지하는 변신용 예산 예외이며 LOD1/2 약50%/25%, 모든 LOD 최대4웨이트. 성능 합격으로 해석하지 않는다.

벗기6초, 독립 입기8초, 본체 대기·걷기·달리기·뒤걷기, 착용 대기·짧은 네발 걸음, 바닥의 빈 탈 대기 클립을 저장했다. 제작 원본/FBX/PNG/manifest는 `animation_natural_v05/`, 원속도 중립광 정면사선·측면 영상은 `animation_natural_v05/review/`. 보행 영상은 제자리 클립이며 이동 속도 검수는 실제 PIE에서 수행한다. 이전 후보와 승인 외형은 보존했다.

새 BP/AnimBP는 동일한 CostumeMesh를 지면에 두고 본체만 이동한 뒤 다시 회수하는 시각 상태 어댑터다. 현재 게임 AI/전투·네트워크 연결 완료로 표시하지 않는다. 실제 PIE·재로드 및 인게임 영상은 진행 중이다.


## 소 v13 납품 — 2026-10-14

- 원본: `ArtSource/monster/storage_owner_disguise/animation_natural_v13/cow_production_v13.blend` (텍스처 패킹). 같은 폴더 `export/`에 본체/탈 LOD0–2, 9개 FBX 클립과 PNG를 묶음.
- 현재 엔진: `/Game/Monsters/StorageOwnerDisguise/RuntimeV03/BP_CowDisguiseV03`, `ABP_CowDisguiseV03`, `QA_CowDisguise_GameV03`. 기존 ProductionV01·RuntimeV02와 승인 외형 원본 보존. 구형 ABP의 잘못된 참조는 원본 바이너리 백업 후 정지 시안용으로 정리했고 새 RuntimeV03가 실제 상태 연결을 담당함.
- 검수: Doff 721 / Don 961개 120Hz 샘플에서 본체·외피·안감·손잡이 사이 교차 0. 손잡이 자기 겹침 0. 새 Don 엔진 본 위치 576샘플 최대 오차 약 0.0004cm. 실제 CL9 모드에서 동일 탈로 두 차례 탈착 및 이동 상태 전환, 저장 후 재로드 확인.
- 영상: `review/cow_off_on_full_1x.mp4`(14초 전체 탈착), `cow_doff_1x.mp4`, `cow_don_1x.mp4`, `cow_contact_close_1x.mp4`, 7개 이동/대기 영상, 3개 정면/후면 위험 구간, `cow_runtime_dark_v03_1x.mp4`. 전부 1배속 및 제작 방식/버전/Blender·UE 구분 워터마크. 밝은 질감 검사 화면은 flat texture inspection으로 명시함.
- 한계: 정적 외피 조형에 교차 삼각형 228쌍이 남아 있어 매니폴드/교차 없는 메시로 주장하지 않음. 움직이는 등판 절개선 접촉과 실제 면 관통은 별도 분류했고 안감·손잡이 내부 교차는 발견되지 않음. 전투/조우 AI, 네트워크, 정식 성능 벤치마크 및 한 판 전체 플레이는 이번 에셋 상태 검수 범위에 포함하지 않음.
- 에디터: PIE 종료, 창/클라이언트 수·한국어·FPS 제한 원복, 원래 레벨 복귀, 세션 잠금 해제. 사용자 Blender 장면은 변경하지 않음.


## 소 탈착 연기 재작업 — 2026-10-14

사용자 “저게아니지. 닭봐봐 가죽을 벗고 나오는느낌이여야돼”로 v13의 경첩식 등판·공중 도약 연기를 반려했다. 앞 절의 완료는 당시 기술 검수 이력이며 현재 최종 합격이 아니다. 닭 최신 가죽 낙하/회수 레퍼런스를 실제 확인했고, 소의 승인된 큰 사족 본체와 평상시 외형은 유지하면서 연속 가죽의 걸림·밀림·벗겨짐·처짐을 다시 저작한다. 기존 원본·영상·엔진 결과를 보존한다.


## 소 가죽 동작 영상 우선 검토 — 2026-10-14

사용자 “언리얼전에 영상으로 만들고 시작해야되는겅나ㅣㅁ?”에 따라 **Blender 원속도 영상 → 사용자 확인 → Unreal** 순서를 명시했다. 앞서 v09에 대해 실행한 엔진 시험은 이력으로 보존하되 이후 UE 작업을 중지했고 에디터 임대를 해제했다. 현재 후보는 `motion_probe_skin_v12/cow_skin_probe_v12.blend`와 같은 폴더 `review/cow_skin_doff_probe_v12_1x.mp4`의 3초/30fps 사선·측면 동작 초안이다. 몸-가죽 관통과 바닥 접촉, 접힘 결함이 남아 있어 최종 제작/검수 합격으로 표시하지 않는다. 기존 승인 외형 및 v13 원본·게임 결과는 보존했다.


## 소 가죽의 걸림·재착용 영상 수정 — 2026-10-14

사용자 요청에 따라 탈이 따로 떨어져 납작해지는 v12를 반려하고, 몸 표면을 따라 걸리는 어깨→팔꿈치→손목 구간과 좌우 해제를 저작했다. 바닥의 같은 탈을 집어 올리는 재착용은 별도 동작이다. 승인된 큰 본체와 전체 축소 허용은 유지했다. 현재 후보 `coupled-skin-v04`의 [14초 원속도 영상](../../../ArtSource/monster/storage_owner_disguise/motion_coupled_skin_v04/review/cow_coupled_skin_v04_doff_don_1x.mp4)은 사선·측면과 제작 방식 워터마크를 포함한다. 검사는 최종 합격이 아니며 상세 실패는 `ArtSource/monster/storage_owner_disguise/motion_coupled_skin_v04/geometry_audit.json`에 보존했다. 이번 수정에서 Unreal 및 구매 팩은 조작하지 않았다. 기존 결과를 보존했고, 사용자 영상 확인 전 엔진 단계로 넘어가지 않는다.


## 옷 안에서 빠져나오는 동작 재설계 — 2026-10-14

사용자 “옷을 벗고 나오는 느낌이없는데”에 따라 coupled-skin-v04를 반려했다. 본체의 후방 이동을 재사용하지 않고 본체를 원래 탈의 중심에 남긴 3초 어깨/팔 통과 시험을 새로 저작했다. 등 입구가 몸통 양옆으로 내려오고, 한쪽 팔을 접어 빼면서 가죽을 아래로 누르는 흐름이다. 현재 원본/영상은 `ArtSource/monster/storage_owner_disguise/centered_peel_v02`에 보존했다. 완전 탈의와 동일한 탈의 재착용은 이 방향 확인 후 이어 만들며, 기존 v04 재착용이나 인게임 결과를 현 후보의 합격 증거로 쓰지 않는다. 기술 QA 실패는 manifest에 명시했다.


### Meshy 보강본 Unreal 검수 배치 — 2026-10-15

사용자 선택: **“Meshy 보강본을 배치”**. 기존 결과를 보존하고 `/Game/Monsters/StorageOwnerDisguise/MeshyReviewV01/QA_MeshyGroundedReview`에 배치했다. `LS_MeshyGrounded_3s`는 본체와 같은 가죽의 skeletal/morph 동작을 1배속으로 함께 재생한다. 에디터에는 검수 맵과 2초 위치의 시퀀스를 열어두었다.

- [실제 PIE 검수 영상 · 1배속](../../../ArtSource/monster/storage_owner_disguise/meshy_compare_v01/peel_probe_v11/unreal_placement_v01/meshy_unreal_placement_1x.mp4): 가죽 포함 3초 + 가죽 숨긴 측면 접지 3초. 우측 상단 제작 방식·버전·속도·PIE 프레임 캡처 워터마크, Meshy CC BY 4.0 표기. 실시간 성능 녹화가 아니다.
- [검수 기록](../../../ArtSource/monster/storage_owner_disguise/meshy_compare_v01/peel_probe_v11/unreal_placement_v01/video_qa.json), [임포트 왕복](../../../ArtSource/monster/storage_owner_disguise/meshy_compare_v01/peel_probe_v11/unreal_placement_v01/ue_import_report.json), [저장 후 재열기](../../../ArtSource/monster/storage_owner_disguise/meshy_compare_v01/peel_probe_v11/unreal_placement_v01/saved_reload_qa.json). 실제 PIE 원본 대조 최대 뼈 위치 차이 0.00548cm, 활성 가죽 변형 값 차이 0.00000716 이하. 자동 재생 29표본, 두 번의 반복 경계를 관찰했다.
- 원본 18초 v20 해시는 그대로다. 신규 Meshy 본체/리그, 기존 가죽, 60Hz 동작은 별도 에셋이며 기존 닭·소 런타임·Content/Peb는 이 작업에서 수정하지 않았다.
- **현재 완료 범위는 선택한 3초 접지 시험의 엔진 임포트·검수 배치·프리뷰 재생이다.** 가죽 관통은 남아 있다. 이 Meshy 후보의 전체 탈 벗기/바닥의 같은 옷 주워 입기·이동 클립·게임 상태/AnimBP·멀티플레이 연결은 완료로 표시하지 않는다.
- 초기 고해상도 촬영 두 번은 게임 틱 정지로 실제 포즈 검사를 통과하지 못해 납품에서 제외했다. 실패 프레임과 QA는 보존했고, 최종 영상은 실제 PIE Slate 창에서 다시 캡처하여 대조 검사를 통과한 v03 프레임만 사용한다.


### Meshy 보강본 전체 탈착·재착용 원속도 검수 — 2026-10-15

- v16에서 기존 동일 소 가죽을 유지하고 Meshy 보강 본체가 완전히 빠져나와 바닥에 놓인 뒤, 그 가죽을 회수해 다시 입는 20초 동작을 만들었다. 사용자 승인 범위 안에서 가죽 안의 본체 축소/노출 중 성장을 적용했고, 나오거나 일어날 때 고개 pitch/yaw도 움직인다.
- [두 시점 1배속 영상](../../../ArtSource/monster/storage_owner_disguise/meshy_full_cycle_v16/review/cow_meshy_full_cycle_v16_1x.mp4) — 30fps, 600프레임, 20초, 1280×360. 화면 워터마크에 Meshy 보강·같은 가죽·v16·Blender Motion Review·1.0x를 표시하고 전체 디코드했다. 이 영상은 Unreal 인게임 촬영본이 아니다.
- 몸–가죽 정확 삼각형 교차는 30fps 장면 포즈 601개에서 모두 0쌍이다. 이 표본 밖의 서브프레임 움직임, 가죽 자체 접힘/자기 접촉은 합격으로 주장하지 않는다.
- 원본 centered_cycle_v20 및 v01–v15 후보는 보존했다. `Content/Peb`는 수정하지 않았다. 상세 수치와 해시는 [v16 manifest](../../../ArtSource/monster/storage_owner_disguise/meshy_full_cycle_v16/manifest.json)에 기록했다.
- Blender 영상 우선 검토 흐름에 따라 여기서 엔진 후속 연결 전 단계로 저장했다. UE 임포트·런타임·PIE 및 실제 인게임 촬영은 v16에 대해 아직 하지 않았다.


### Meshy 본체 가시성 수정 — 2026-10-15

v16 영상에서 몬스터가 사라져 보인 원인은 착용 축소 커브가 두 구간에 어긋났기 때문이다. 첫 탈출 때 옷의 통과가 끝나기 전에 본체가 2–8% 크기로 남았고, 다시 입을 때는 바닥 옷에 들어가기 전부터 축소됐다. v18에서는 가죽과 교차하는 초기 구간의 안전 크기는 유지하고, 바깥으로 나온 뒤 크기를 키우며, 가죽 입구에 접근할 때까지 본체 크기를 유지한다. 전 프레임 표본 QA는 601개 모두 몸–가죽 삼각형 교차 0건이다.

- [v18 1배속 두 시점 영상](../../../ArtSource/monster/storage_owner_disguise/meshy_full_cycle_v18/review/cow_meshy_full_cycle_v18_1x.mp4) — Blender 검수본, 20초·30fps·워터마크 포함. Unreal 인게임 캡처는 아니다.
- [v18 manifest/검수 한계](../../../ArtSource/monster/storage_owner_disguise/meshy_full_cycle_v18/manifest.json). 서브프레임과 가죽 자체 접힘·자기 접촉은 미인증이다. v16 및 실패한 탐색본을 보존했고 Content/Peb는 수정하지 않았다.


### 소 탈착 가시성·재착용 궤적 수정 — 2026-10-15

사용자 “나올때 몬스터가 왜 사라지면서 안보임?”에 따라 원인을 확인했다. 본체는 숨김 처리되지 않지만, 축소 상태로 탈 뒤쪽에 남고 큰 탈이 카메라 앞을 가려 탈출 초기에 사라져 보였다. v08은 등 개방 뒤 본체 성장 시점을 앞당겨 시야에 빨리 들어오게 하고, 같은 가죽이 몸 뒤를 따라 바닥에 놓인 다음 원래 위치로 돌아와 집히게 했다. 앞 시점에서 탈이 본체를 가리는 각도 문제는 Blender 양 시점 영상에 남겨 확인할 수 있도록 했다.

- [1배속 두 시점 20초 영상](../../../ArtSource/monster/storage_owner_disguise/meshy_chicken_style_v08/review/cow_meshy_chicken_style_v08_1x.mp4): Meshy 6 보강 리그 + 수작업 모션, 같은 소 가죽, Blender 검수 워터마크. 30fps·600프레임·전체 디코드와 읽기 검사를 통과했다.
- 저장 원본의 몸–가죽 정확 삼각형 교차는 601/601 프레임 0쌍. 서브프레임 및 가죽 자체 접힘은 미검수이며, 시각 승인은 대기 중이다.
- v01–v07, v20 기존 원본/결과는 보존했다. 이번 변경에서 Unreal과 `Content/Peb`는 건드리지 않았다.


### v20 중심 동작으로 Meshy 탈착 재저작 — meshy-centered-cycle-v17

v08은 사용자 피드백에 따라 시각 검수 탈락이다. 사용자는 v20의 가죽 안에서 이어져 나오는 동작을 기준으로 요구했고, 설명과 이전 영상 재링크 대신 실제 수정 영상을 다시 제작하라고 지시했다. 위 현재 검토란의 meshy-centered-cycle-v17이 그 재작업 결과다. 기존 원본과 후보는 보존했다. 제작 방식은 실제 Meshy 리그 보강·수작업 동작·Blender 가죽 보정이며, Unreal 작업은 이번 수정에서 수행하지 않았다.


<!-- motion-sketch-v01-review -->
### v19 동작 수정 스케치 — 검토 제안

손 집기는 유지하고 어깨·골반·마지막 뒷발의 저항과 지지 순서를 보강하는 6컷 도해 두 장이다. ImageGen으로 만든 동작 설계 제안이며 구현·관통 검사 완료 증거가 아니다. 기존 v18/v19 원본과 통과한 검사는 보존했다.

- [벗기 스케치](../../../ArtSource/monster/storage_owner_disguise/motion_sketch_v01/01_doff_sequence.png)
- [같은 가죽 줍기·입기 스케치](../../../ArtSource/monster/storage_owner_disguise/motion_sketch_v01/02_pickup_don_sequence.png)
- [생성 프롬프트](../../../ArtSource/monster/storage_owner_disguise/motion_sketch_v01/prompts.txt)


<!-- motion-sketch-v02-review -->
### v19 motion sketch v02 - hand/foot continuity review

Current sketch-only review supersedes v01 for the shoulder/pelvis/rearfoot interval. R is the same near-side grasp; L is the far-side brace. RH exits first, LH clears last. The six poses separate shoulder snag, pelvic contact, pelvic clearance and each rearfoot. Existing v19 production candidate, source body, grasp and QA remain unchanged. Pickup/don v01 remains historical and is not newly approved here.

- [Six-pose corrected sketch](../../../ArtSource/monster/storage_owner_disguise/motion_sketch_v02/01_doff_hand_foot_sequence.png)
- [Prompts](../../../ArtSource/monster/storage_owner_disguise/motion_sketch_v02/prompts.txt)
- [Manifest](../../../ArtSource/monster/storage_owner_disguise/motion_sketch_v02/manifest.json)


<!-- v19-short-motion-current -->
### v19 짧은 옷 동작 시험 — 3.5초 · 1배속

v19 괴물 본체의 원본 정점·UV·바인드 비율과 R 손 집기 자세를 보존했다. 어깨 걸림 → 골반 통과 → 가까운 뒷발 → 먼 뒷발 구간만 수정한 Blender 시험본이다. 옷 몸통은 연속된 천 면의 구조 시험이며 기존 소 머리 텍스처를 유지했다. 자유 천 물리와 큰 접힘·운반 제어를 함께 사용한다. 손가락별 정밀 관통 보정은 이번 사용자 범위에서 제외했다.

106개 정수 프레임의 몸통·팔다리·발 접촉 검수에서 현재 범위의 잔여 교차 0이며, 잡는 R 손바닥/손가락은 잡기 인상·고정 재질점 기준이다. 원본 v19/전체 영상/Unreal 에셋은 바꾸지 않았다. 옷의 최종 착용 외형·재질·머리 연결부와 전체 탈착/재착용 연결은 미완료이고 사용자 시각 검토 전이다.

- [3.5초 1배속 사선·측면 영상](../../../ArtSource/monster/storage_owner_disguise/v19_motion_patch_v02/review/cow_v19_shoulder_pelvis_last_foot_patch_1x.mp4)
- [현재 Blender 시험본](../../../ArtSource/monster/storage_owner_disguise/v19_motion_patch_v02/short_patch_final.blend)
- [정확한 범위·QA·해시·재개 정보](../../../ArtSource/monster/storage_owner_disguise/v19_motion_patch_v02/manifest.json)
<!-- /v19-short-motion-current -->

<!-- v19-release-drop-v03 -->
### v19 옷 놓기·낙하 짧은 수정 — v03

손을 놓은 다음 옷이 아래로 떨어지고 바닥에 접혀 남는 동작을 직접 저작했다. 기존 v02의 1–106 옷 캐시와 본체 동작을 재사용하고 소 머리도 함께 내렸다. 새 물리 시뮬레이션은 돌리지 않았다. 2.5초·30fps·1배속 Blender 구간 영상이며 기존 Unreal/전체 영상은 보존했다. 이번 검사는 영상 디코드·재사용 캐시·낙하/바닥 높이 범위이며 전체 관통·자기 충돌 합격을 뜻하지 않는다. 옷 몸통의 기존 시험용 외형과 머리 연결부는 여전히 최종 제작 전이다.

- [짧은 낙하 영상](../../../ArtSource/monster/storage_owner_disguise/v19_motion_patch_v03/review/cow_v19_release_drop_1x.mp4)
- [Blender 원본](../../../ArtSource/monster/storage_owner_disguise/v19_motion_patch_v03/short_drop.blend)
- [범위·검수·재개 정보](../../../ArtSource/monster/storage_owner_disguise/v19_motion_patch_v03/manifest.json)
<!-- /v19-release-drop-v03 -->


<!-- cow-runtime-walk-v20-current -->
### 현재 테스트 레벨 — RuntimeWalkV20

사용자 지시 “이쯤했으면 그냥 만들자”에 따라 v19 체형·집기와 v03 놓기/낙하를 이어 테스트 레벨로 연결했다. Play하면 네이티브 Blueprint가 **벗기·낙하 → 괴물 걷기 → 같은 옷으로 복귀 → 별도 집기·입기 → 소 모습 걷기**를 자동 실행한다. 바닥 옷은 같은 메시/컴포넌트를 유지하며 괴물 이동 중 월드 위치 드리프트 0cm다. 최종 영상은 1배속 실시간 PIE 녹화다. 스켈레톤의 의상 모프 메타데이터 누락을 수정한 뒤 실제 화면에서 낙하/바닥 유지/재착용을 확인했다. 앞선 잘못된 녹화와 로그도 버전 폴더 안에 보존했으며 현재 납품 영상으로 취급하지 않는다.

- [테스트 레벨 1배속 영상](../../../ArtSource/monster/storage_owner_disguise/runtime_walk_v20/review/cow_v20_doff_roam_same_skin_don_roam_1x.mp4)
- Unreal: `/Game/Monsters/StorageOwnerDisguise/RuntimeWalkV20/QA_CowWalkV20` · 배치 액터 `Cow_V20_SameCostume`, `AutoDemo=true`.
- [편집 가능한 Blender 원본](../../../ArtSource/monster/storage_owner_disguise/runtime_walk_v20/cow_runtime_walk_v20.blend)
- [납품·런타임 QA·남은 범위](../../../ArtSource/monster/storage_owner_disguise/runtime_walk_v20/delivery_manifest.json)

검사는 실제 상태 전환·이동·같은 옷 유지·표본 클립 포즈/모프·영상 디코드 범위다. 전체 삼각형/서브프레임/자기 충돌, 최종 LOD·게임 예산, 본편 몬스터 AI·내비·전투·네트워크 통합 합격으로 확대하지 않는다. v19 및 모든 이전 결과와 Content/Peb는 보존했다. 사용자 시각 검토 대기.
<!-- /cow-runtime-walk-v20-current -->

<!-- cow-mixamo-motion-probe-v01-current -->
### Mixamo 무료 모션 비교 시험 — 영상 제작 완료·사용자 검토 대기 (2026-10-16)

- 사용자 요청: 기존 체형·손 집기를 유지하고 무료 Mixamo 네발 이동·줍기만 짧은 1배속 비교 영상으로 확인. Unreal 후보 가격 및 기상·IK 도입 필요성도 확인.
- 기존 원본: `runtime_walk_v20/cow_runtime_walk_v20.blend`; 실제 리그 53뼈, 본체 69,559정점. 현재 Blender GUI와 Unreal 테스트 레벨은 조작하지 않았다. 실행 중인 별도 Blender 렌더는 없음을 확인한 뒤 읽기 전용 원본 검사를 완료했다.
- 작업 폴더: `ArtSource/monster/storage_owner_disguise/mixamo_motion_probe_v01/`; 원본 조사: `source_inspection.json`. 검사 스크립트: `Tools/Blender/inspect_cow_mixamo_probe.py`.
- 모션 다운로드: 사용자가 지정한 계정으로 새 Mixamo 인증 요청을 완료한 뒤 공식 X Bot 기준 `Running Crawl.fbx`, `Lifting Object.fbx`를 확보했다. FBX Binary·Without Skin·30fps·Keyframe Reduction none. 실제 파일·해시·원본 프레임은 작업 폴더의 `mixamo_source_report.json`, FBX는 `source_mixamo/`에 보존했다.
- 독립 확인 완료: [조사 문서](../tools/monster-animation-sources-20261016.md)에 Fab 5개 후보의 개인용/기업용 실제 원화 가격·표시 지원 버전, 기상 모션·기본 IK 대안 판단을 추가했다. 구매·설치는 하지 않았다.
- **비교 영상:** [7초·30fps·1배속](../../../ArtSource/monster/storage_owner_disguise/mixamo_motion_probe_v01/review/cow_mixamo_vs_v20_walk_pickup_7s_1x.mp4). 왼쪽은 기존 v20, 오른쪽은 Mixamo 보강 시험. 0–3초 네발 이동, 3–7초 같은 옷 줍기. 양쪽 같은 중립광·카메라로 렌더했고, 제작 방식·버전·속도·Blender 구분을 표시했다. 원본 모션의 재생 속도를 바꾸지 않았다.
- **실제 적용:** Running Crawl 원본 1–20프레임을 원속도로 반복해 네 발 목표점과 몸·고개 움직임을 기존 리그에 옮겼다. Lifting Object 원본 31–150프레임의 몸통·고개 층을 기존 줍기에 더했다. 줍는 손목·25개 보호 뼈의 손가락 및 같은 옷 동작은 기존 제어를 유지했다. 줍기 전체를 자동 이식한 것으로 표시하지 않는다.
- **편집 원본:** `mixamo_motion_probe_v01/cow_mixamo_motion_probe_v01.blend`. 제작·검수·렌더: `Tools/Blender/{sample_mixamo_cow_probe,build_cow_mixamo_probe,audit_cow_mixamo_probe,render_cow_mixamo_probe}.py`; 영상 합성: `Tools/Film/package_cow_mixamo_probe.py`.
- **검증:** 다시 연 파일 비교에서 본체 원본 정점과 리그 바인드 동일, v20 원본 SHA-256 불변. 줍기 120프레임·손목/손가락 25뼈의 최대 월드 위치 차이 **0.006362cm**, 허용 0.05cm 이내. 네발 이동의 표본 발바닥 최저 높이 약 2.8cm로 기존 접지 여유 유지. `preservation_qa.json`, `probe_qa.json`, `review/video_manifest.json`에 범위·수치·한계를 기록했다. MP4 전체 디코드 통과 후 실제 인코딩 프레임과 별도 측면 주요 자세를 확인했다. 전체 삼각형 관통·자기 충돌 인증이나 사용자 채택을 뜻하지 않는다.
- **도입 판단:** 무료 이동은 빠른 추격용 모션 재사용의 이득이 있다. 이번 Running Crawl은 느리고 무거운 기존 Walk와 속도를 맞추지 않은 비교이므로 배회 걷기를 그대로 대체한다는 판단은 보류한다. 줍기는 몸통·고개 보강의 제한적 이득이며 옷·손 접촉 저작은 계속 필요하다. 이번 평지 시험은 기존 Blender IK로 가능했고 Dragon IK 구매는 불필요했다. 불규칙 지형의 실제 Unreal 접지는 별도 검증 범위다. 기상 하나 때문에 유료 좀비 팩을 먼저 구매할 이유는 아직 확인되지 않았다.
- **현재 상태:** `phase: motion_probe`, `status: awaiting_user`, `job: null`. 전체 탈착·Unreal 재촬영은 하지 않았고 기존 RuntimeWalkV20 테스트 레벨은 유지했다. 다음 행동은 비교본 채택 여부를 확인한 뒤 필요한 동작만 이어서 보정하는 것이다.
<!-- /cow-mixamo-motion-probe-v01-current -->

<!-- cow-mixamo-getup-v02-current -->
### 무료 Getting Up — 5초 영상 완료·사용자 검토 대기 (2026-10-16)

- 요청: 기존 체형·손 집기를 유지한 무료 Getting Up 적용, 자연스러운 고개 움직임과 짧은 1배속 영상, 관련 무료 모션 다운로드 및 유료 모션 100,000원 내 후보 검토.
- **영상:** [Getting Up 5초·30fps·1배속](../../../ArtSource/monster/storage_owner_disguise/mixamo_getup_probe_v02/review/cow_getting_up_5s_1x.mp4). 왼쪽 사선, 오른쪽 같은 동작의 측면. 중립광·제작 방식 워터마크 포함. 완성 MP4 전체 디코드·추출 프레임 확인 완료. Unreal·전체 탈착 재촬영은 하지 않았다.
- **편집 원본:** `ArtSource/monster/storage_owner_disguise/mixamo_getup_probe_v02/cow_getting_up_v02.blend`. 공식 Mixamo `Getting Up From Stomach` 원본 41–190프레임을 30fps 그대로 사용해 새 구간 837–986에 추가했다. 낮은 자세 → 앞발 지지 → 몸·고개 들기 → 네발 안정으로 조정했으며 사람의 두발 서기를 그대로 복사하지 않았다. 긴 주둥이의 숙임을 제한하고 고개가 몸보다 약간 먼저 반응하도록 보강했다.
- **보존 확인:** 다시 연 파일 비교에서 본체 기본 정점·리그 바인드 동일, 기존 v20 원본 해시 불변. 기존 836프레임의 전체 뼈 월드 행렬·옷 모프·손 집기 행렬 차이 모두 0. 기존 집기는 기존 클립에 그대로 보존했고, 새 기상 구간에서는 옷을 들지 않는다.
- **새 구간 QA:** 150프레임 전체 메시 최저 높이 최소 0.027992m. 고개 최대 프레임 이동 1.9635cm, 주요 관절 최대 프레임 회전 4.77도. 손가락 접지 표본에 GraspDigit 정점을 포함했고, 두꺼운 배가 지면을 통과하지 않도록 실제 스킨 표면으로 웅크림 높이를 제한했다. 체형 축소나 뼈 스트레칭은 사용하지 않았다. 전체 자기 충돌·옷 관통 인증은 아니다.
- QA: 작업 폴더의 `getup_qa.json`, `preservation_qa.json`, `continuity_qa.json`, `review/video_manifest.json`. 초기 실패 자세·진단도 보존했다. 두 시점 300프레임 렌더 후 PowerShell 로그 리다이렉션만 실패했으나 각 150프레임과 완성 영상 디코드를 확인해 렌더를 반복하지 않았다.
- **무료 보관:** [파일 목록·해시](../../../ArtSource/monster/storage_owner_disguise/mixamo_free_library_20261016/manifest.json). 개별 17개 + Creature NPC Pack 12개 + Scary Zombie Pack 12개 + Not So Scary Zombie Pack 24개 = 모션 FBX 65개, 별도 X Bot 참조 모델 3개. 팩 간 중복 가능. ZIP과 추출 FBX를 보존했고 다운로드 전부를 적용·시각 검수 완료로 취급하지 않는다. 이번 괴물 관련 모션을 우선 수집했으며 Mixamo 전체 라이브러리 다운로드는 아니다.
- **유료 후보:** [Crawling Ghoul](https://www.fab.com/listings/9e96b3f7-30ca-4221-a2eb-2bd46146b286). 현재 VAT 포함 개인용 ₩29,480, 기업용 ₩147,510(예산 초과). 해당 자격에 맞는 가격을 적용한다. 느린/빠른 네발 8방향 이동 묶음이 장점이지만 우리 리그 보정·UE 5.8 검증은 별도다. 이번 기상에는 무료로 충분했고 Dragon IK·Werewolf 추가 구매는 보류 판단. 결제·구독 미실행, 사용액 0원.
- 도구: `Tools/Blender/build_cow_getup_probe_v02.py`, `audit_cow_getup_preservation.py`, `check_cow_getup_continuity.py`, `catalog_cow_mixamo_library.py`; 기존 샘플·렌더·영상 합성 도구의 `--getup` 경로 재사용.
- **현재:** `phase: motion_probe`, `status: awaiting_user`, `job: null`. 다음은 기상 시각 검토·채택 여부 확인. 이전 Mixamo v01·RuntimeWalkV20·Content/Peb·사용자 Blender GUI는 보존했다.
- **무료 수집 범위 확정:** 사용자 답변 “괴물에 관련된 것 중에 우리가 사용하려는 것”. 기상·네발 이동·줍기·놓기·대기 등 실제 사용 예정 동작을 우선 선별한다. 받아둔 65개 파일은 참고 보관하며 전부 적용 대상으로 취급하지 않는다. 이후 다운로드도 이 범위 안에서 필요한 후보만 추가한다.
<!-- /cow-mixamo-getup-v02-current -->

<!-- cow-ghoul-purchase-20261016 -->
### Crawling Ghoul 구매·파일 확인 및 탈착 도구 조사 — 2026-10-16

- 사용자가 Crawling Ghoul Anims 구매 및 UE 5.7용 다운로드 완료를 알렸다. 기존 구매 에셋으로 취급한다. `Content/CrawlingGhoulAnims`에서 UE5 애니메이션·검수 레벨 파일을 확인했다. 파일 존재 확인이며 UE 5.8 로드·리타게팅·우리 괴물 적용 성공 판정은 아니다. 구매 원본과 기존 본체·손 집기·v20 검수 결과는 보존한다.
- 현재 전체 연결 요청은 유효하다. 다음 제작 단계는 실제 에디터 잠금과 변경 상태 확인 후 구매 모션 중 느린 이동·시작/정지·회전을 조사해 기존 체형에 맞추고, 무료 기상 및 기존 탈착과 전환을 짧게 검수하는 것이다. 새 전체 인게임 영상은 아직 제작하지 않았다.
- 추가 조사는 구매 실행 없이 진행했다. Simply Cloth Studio 2.0은 1인 옵션 $36, Auto Drape 개인용은 VAT 포함 ₩44,240. 두 도구 모두 전체 벗기→바닥의 같은 옷 줍기→입기 완성 에셋으로 확인되지 않았다. Simply Cloth는 천 고정점·충돌·시뮬레이션 설정 보조이고, Auto Drape는 낙하/정착 베이크 보조이며 자기 옷 Dropped Garment는 베타다.
- 공식 시연: [Simply Cloth Studio 전체 사용법](https://www.youtube.com/watch?v=rSP8X2DfGX4), [Auto Drape 실제 사용법](https://www.youtube.com/watch?v=E0QyVQhnhdA). 브라우저 재생과 화면 표본을 확인했으며 우리 소 옷 적용 검증이나 영상 전체 검수로 확대하지 않는다. 상세 가격·역할은 기존 조사 문서에 추가한다.
- `phase: motion_integration_pending`, `job: null`. 이번 확인 과정에서 Blender 원본·Unreal 에셋·Content/Peb는 수정하지 않았다.
<!-- /cow-ghoul-purchase-20261016 -->

<!-- cow-doff-tool-research-20261016 -->
### 손 상세 완화 및 실제 탈의 도구 재조사 — 2026-10-16

- 사용자 기준: 손가락별 정밀 집기보다 잡는 시늉이면 충분하다. 다만 옷이 몸에서 벗겨지고, 놓으면 떨어지고, 바닥의 같은 옷을 집어 다시 입는 흐름은 유지한다. 이번 메시지는 도구 조사이며 제작·구매·설치 실행으로 확대하지 않았다.
- 새 우선 후보는 Marvelous Designer 개인용 월간 $39다. 공식 가격 페이지 월간 표시와 RenderRides의 실제 티셔츠 탈의 제작 시연을 확인했다. 무료 체험은 종료 후 자동 결제 조건이 있다. 도구는 구매하지 않았다.
- [상세 가격·시연·제약](../tools/monster-animation-sources-20261016.md#탈의-제작을-실제로-시연한-우선-후보-marvelous-designer-개인용-월간). 재착용까지 완성된 우리 소 전용 에셋은 확인하지 못했으며, 옷 메시 적합성·게임용 출력은 미검증이다. 기존 원본·검증·Crawling Ghoul 구매 파일은 유지했다.
<!-- /cow-doff-tool-research-20261016 -->

<!-- cow-cloth-lab-v01-current -->
### Cloth Lab 실제 소 옷 시험 — 사용자 어색함 지적·보존 후 중단 (2026-10-17)

- 최신 결정: 사용자가 결과가 매우 어색하다고 평가하고, 이 작업은 여기까지 보존한 뒤 전체 몬스터 다운로드 모션 제작으로 전환하도록 지시했다. 아래 검토 대기 기록은 당시 이력이며 현재 자연스러움 승인/추가 탈착 제작 대기가 아니다. `status: rejected_paused_by_user`, `job: null`. 원본·영상·검증 결과 유지.

- 사용자 요청: 무료 Cloth Lab 적용, 기존 체형 유지, 손은 집는 인상만 유지, 벗기→낙하→같은 옷 줍기→재착용의 짧은 1배속 영상. 이번 단계는 Blender 시험이며 Unreal 새 작업으로 확대하지 않는다.
- 범위 `motion_probe`; 작업 폴더 `ArtSource/monster/storage_owner_disguise/cloth_lab_probe_v01/`. 기존 v20 본체/리그/원본 옷 UV 메시와 별도 저작된 탈착을 재사용한다. Cloth Lab 0.8.1 공식 ZIP은 `vendor/`에 보존했고 SHA-256 `60320a61f88898c6b8e6b96899cc635ca27332373aa2f3b8a76866d2f479ec35`다. 사용자 Blender 환경에 영구 설치하지 않고 별도 background 프로세스에만 등록한다.
- 실제 적용: 기존 연속 천 프록시 7,426정점에 Cloth Lab의 `setup_cloth`, `add_pin`, `clab.control RELEASE`를 실행했다. 150–420프레임을 Blender 기본 천 솔버로 계산하고, 같은 원본 43,928정점 옷의 형태 키에 변형 차이를 전달했다. 기존 몸·옷의 큰 벗기/낙하/별도 입기 경로는 재사용했다. 주 동작 안내점과 손 집기 레이어는 별개이며 손 레이어는 놓기/다시 잡기를 수행한다. **자유 천만으로 전체 탈착을 자동 생성한 결과가 아니다.**
- 실패 원인·보존: 초기 겹친 자유 천의 자체 충돌은 반발·팽창, 높은 공기 저항은 과도한 낙하 지연, 접힌 쉘의 자유 풀림은 형태 복원/회전을 만들었다. 실패 로그를 `failed_self_contact_pass.log`, `overdamped_drop_pass.log`, `failed_folded_rest_pass.log`, `failed_free_shell_pass.log`에 보존하고 본 촬영 전에 종료했다. 최종은 큰 경로를 안내점으로 유지하고 천 자체 충돌을 끈 보조 시뮬레이션이다. 시뮬레이션 프록시의 일부 집기 안내점은 지면 아래에 있어 최종 표면 전송에서 1.8cm 바닥 제한을 적용했다. 자유 낙하·자기 충돌·전체 관통 합격으로 표시하지 않는다.
- 보존 검사: 원본 SHA-256 `0d4b2fa9373ca2fed31a19f817081fb0bf6eb20ef2fd597e7224b8ea86525f38` 불변. 다시 연 파일에서 본체 기본 정점·리그 바인드·원본 옷 UV 동일, 1–568 전체 정수 프레임의 본체 월드 뼈 행렬 최대 차이 0. 같은 옷을 유지했고 바닥 대기 구간 최대 프레임 간 정점 이동 약 0.0373cm. 천 보조 변위는 평균 1.57mm·95백분위 8.40mm로 제한적이며 큰 동작 개선 효과를 과장하지 않는다. 상세 `preservation_qa.json`.
- **영상:** [소 옷 Cloth Lab 보조 시험 · 약 18초·1배속](../../../ArtSource/monster/storage_owner_disguise/cloth_lab_probe_v01/review/cow_cloth_lab_doff_drop_pickup_don_1x.mp4). 864×636·24fps·431프레임·17.958초. 원본 30fps 동작을 시간 변경 없이 24fps로 표본화했고, 중복 바닥 대기와 마지막 대기만 제외했다. 우측 상단에 실제 제작 방식·버전·Blender·1배속을 표시했다. 전체 MP4 디코드 및 추출된 낙하/같은 옷 집기/입기 화면을 확인했다. 사선·측면 주요 자세 18장과 기존 v20 동일 카메라 표본도 확인했다.
- **판정:** 제한을 포함한 도구 시험 영상 제작 완료이며 사용자 검토 대기다. 천 보조 변화는 작고 기존 옷의 납작한 접힘/목 연결 형태·펼침의 어색함은 남아 있다. Cloth Lab 단독 해결, 완전 자유 천, 자연스러움 승인 또는 전체 관통 합격으로 처리하지 않는다. [납품·한계 manifest](../../../ArtSource/monster/storage_owner_disguise/cloth_lab_probe_v01/delivery_manifest.json).
- 실행: `Tools/Blender/{build_cow_cloth_lab_probe,finish_cow_cloth_lab_probe,audit_cow_cloth_lab_probe,render_cow_cloth_lab_probe}.py`; `Tools/Film/package_cow_cloth_lab_probe.py`; 로그 `full_build.log`, `preservation.log`, `render.log`. 편집 원본 `cow_cloth_lab_probe_v01.blend`, 계산 설정 `cloth_lab_full_setup.blend`, 실제 시뮬레이션 `simulation_full.npy`. `status: awaiting_user`, `job: null`. 기존 GUI Blender·Unreal·Content/Peb는 수정하지 않았다.
<!-- /cow-cloth-lab-v01-current -->

### 전체 몬스터 모션 전환 — 소 Ghoul 기기 1차 시험 (2026-10-17)

- 사용자 범위는 전체 현행 7종 모션. 소 탈착은 위 거절 결과를 보존·중단한다. 종별 범위/미준비 대상은 [motion-plan](../motion-plan.md)의 2026-10-17 절 참조.
- 소 작업본: `ArtSource/monster/motion_library_20261017/cow_ghoul_probe/cow_ghoul_crawl_probe.blend`. v20 원본을 읽고 구매한 Ghoul CrawlForward의 41프레임 주기(중복 끝점 제외)를 원래 30fps로 리타깃. 네 끝점·몸통·고개와 발바닥 IK 적용. 이동 거리만 체형에 맞게 환산하며 시간 배율 1. 기존 옷/집기 동작 원본은 수정하지 않았다.
- 90프레임 QA: 원본 본체 정점과 53뼈 바인드 동일, 전신 최저 Z 약 2.50cm, 매 프레임 최소 한 발바닥 Z가 5cm 이내. 손끝 보정에 기존 GraspDigit 그룹 포함. 바닥 침범 검사를 천/몸 자기 충돌·미끄러짐·전체 게임 상태 합격으로 확대하지 않는다.
- [사선·측면 3초·1배속 영상](../../../ArtSource/monster/motion_library_20261017/cow_ghoul_probe/review/cow_ghoul_probe_1x.mp4). 제작 방식·Blender·속도 워터마크, 전체 디코드 통과 및 디코드 표본 확인. 편집 원본·QA·리그 전용 FBX는 같은 폴더. `status: locomotion_probe_review`, `job: null`. 새 클립 UE 임포트/게임 연결 미실행. 기존 엔진 콘텐츠·Content/Peb 수정 없음.


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
