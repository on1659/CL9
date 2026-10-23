# 닭 탈 본체 공개 — 벗기·입기 동작 프리뷰

## 현재 기준 — 첨부 컨셉아트 재선택

2026-10-11 사용자가 [원래 닭 탈 괴물 컨셉아트](../../../ArtSource/monster/chicken_disguise/reference/approved_chicken_disguise_20261011.png)를 명시적으로 선택했다.
밝고 길쭉한 단순 얼굴·어두운 긴 팔다리가 외형 정본이며, 아래 v3/v4의 이빨 얼굴은 채택하지 않는다.
벗긴 탈이 바닥에 떨어지고 진정 후 직접 주워 다시 입는 동작 요구는 유지한다.
자연스러운 모델·리그·동작을 새 세션에서 제작할 지시서는 [애니메이션 제작 인계](chicken_disguise-animation-handoff.md)다.
아래 내용은 과거 프리뷰 이력이며 새 제작의 완료 증거가 아니다.

## v4 수정 — 가면 대신 입체 괴물 얼굴

사용자 피드백: v3는 가면 같으므로 실제 괴물처럼 무섭게. 밝은 타원 얼굴판을 제거하고 어두운 두개골·광대·주둥이·벌어진 아래턱을 제작했다. 눈구멍은 Boolean으로 실제 깊이를 만들고, 입은 빈 공간과 깊은 안쪽 메시·휘어진 입체 이빨로 표현했다.

- [얼굴 3/4](../../../ArtSource/monster/chicken_disguise/animatic_v4/preview/face_three_quarter_v4.png)
- [옆모습](../../../ArtSource/monster/chicken_disguise/animatic_v4/preview/face_side_v4.png)
- [전신](../../../ArtSource/monster/chicken_disguise/animatic_v4/preview/body_face_v4.png)
- [Blender 원본](../../../ArtSource/monster/chicken_disguise/animatic_v4/chicken_disguise_animatic_v4.blend)
- 제작: `Tools/Blender/refine_chicken_disguise_face_v4.py`

기존 본체·탈의 동작을 보존한 얼굴 형태 프리뷰다. 정지 렌더를 확인했고 v3 원본 SHA256 보존을 검사했다. v4 영상은 아직 렌더하지 않았다. 턱 별도 리깅·게임용 리토폴로지·UE 연결은 미진행이며 최종 외형 채택 전이다.

---

## v3 수정 — 본체 얼굴의 공포감

2026-10-11 사용자 요청: 몬스터 얼굴을 조금 더 무섭게.
단순 타원 가면의 점 눈과 가는 입을 비대칭의 어두운 눈구멍·작은 비발광 눈·불규칙한 긴 세로 입·작은 이빨 6개로 바꿨다.
턱 외곽을 조금 좁히고 눈 아래에 얇은 균열을 더했다. 전신·탈·기존 벗기/입기 동작은 유지했다.

- 제작 스크립트: `Tools/Blender/refine_chicken_disguise_face_v3.py`
- 출력 폴더: `ArtSource/monster/chicken_disguise/animatic_v3/`
- [얼굴 확대 렌더](../../../ArtSource/monster/chicken_disguise/animatic_v3/preview/face_closeup_v3.png)
- [전신 거리 렌더](../../../ArtSource/monster/chicken_disguise/animatic_v3/preview/body_face_v3.png)

완료: 확대·전신 렌더와 인코딩된 영상의 160·345프레임을 시각 확인했다. 전체 480프레임/16초, 분리 영상 각 240프레임/8초, 30fps로 재읽기를 확인했다.

- [수정된 전체 16초 영상](../../../ArtSource/monster/chicken_disguise/animatic_v3/preview/chicken_disguise_off_on_v3.mp4)
- [벗기 8초](../../../ArtSource/monster/chicken_disguise/animatic_v3/preview/take_off_v3.mp4)
- [입기 8초](../../../ArtSource/monster/chicken_disguise/animatic_v3/preview/put_on_v3.mp4)
- [수정 가능한 v3 원본](../../../ArtSource/monster/chicken_disguise/animatic_v3/chicken_disguise_animatic_v3.blend)

눈구멍은 어두운 표면 메시와 테두리로 표현한 초안이며 실제 깊은 구멍을 조각한 최종 메시가 아니다.
v2 파일의 SHA256을 작업 전후 대조해 원본 보존을 확인했다. 새 파일은 별도 background Blender에서 저장했다.

---

## v2 수정 — 위로 열리는 탈, 바닥 낙하, 재착용

2026-10-11 후속 사용자 요구: 옷이 위로 찢어지고 땅에 떨어져야 한다.
어그로/광폭화가 줄면 다시 입는 동작도 필요하다. 기존 v1 원본과 영상은 보존한다.

현재 상태: v2 주요 포즈·인코딩된 영상의 4개 시점 시각 확인 / 전체 및 분리 영상 재읽기 확인.

- [전체 16초](../../../ArtSource/monster/chicken_disguise/animatic_v2/preview/chicken_disguise_off_on_v2.mp4)
- [벗기 8초](../../../ArtSource/monster/chicken_disguise/animatic_v2/preview/take_off_v2.mp4)
- [입기 8초](../../../ArtSource/monster/chicken_disguise/animatic_v2/preview/put_on_v2.mp4)
- [수정 가능한 v2 원본](../../../ArtSource/monster/chicken_disguise/animatic_v2/chicken_disguise_animatic_v2.blend)
- [영상 읽기 검사 결과](../../../ArtSource/monster/chicken_disguise/animatic_v2/video_readback.json)

- 새 원본: `ArtSource/monster/chicken_disguise/animatic_v2/chicken_disguise_animatic_v2.blend`
- 제작: `Tools/Blender/build_chicken_disguise_off_on.py`
- 영상 검사·분리: `Tools/Blender/check_split_chicken_disguise_off_on.py`
- 16초(480프레임), 30fps, 960×540, 무음. 앞 8초 벗기 / 뒤 8초 입기.

벗기: 아래쪽 실땀부터 위로 풀림 → 밑단 기준으로 탈 위쪽이 벌어짐 → 본체가 위로 빠져나옴
→ 탈이 몸과 분리되어 바닥으로 떨어지고 납작해짐 → 본체와 떨어진 탈이 함께 보임.

입기: 진정 계기를 8초에 가정 → 몸을 숙여 양손으로 바닥의 옷을 잡음 → 옷을 들어 펼침
→ 긴 팔다리와 몸을 접어 들어감 → 봉제선을 닫고 닭 모습으로 돌아옴.
입기는 벗는 영상의 역재생이 아닌 별도 자세·손 이동으로 작성했다.

탈은 같은 파츠를 계속 유지한다. 오브젝트 계층·회전·압축으로 낙하와 재착용을 표현하며,
실제 천 시뮬레이션이나 토폴로지를 찢고 꿰매는 시스템은 아니다.
바닥을 뚫는 큰 파츠를 줄이기 위해 탈 메시 바운드의 최저 높이를 보정했다.
손과 천의 세밀한 접촉, 주변 재료와 천의 충돌은 아직 정밀 검수 전이다.
저장된 MP4의 160·300·345·430프레임을 추출해 직접 확인했다.
전체 480프레임/16초, 각 분리 영상 240프레임/8초, 모두 960×540/30fps로 다시 읽혔다.

현재는 타이밍 프리뷰다. 어그로/광폭화 게이지·회복 문턱·AI 전환·탈 소유권을 게임에 구현한 것이 아니다.
실제 게임 연결 시 벗은 탈 위치 보존, 이동 후 회수 방식, 재착용 도중 재발각 처리 등을 별도로 정해야 한다.

---

2026-10-11. 사용자 선택: **변신 장면 영상 프리뷰 — 간단한 3D 모델로 타이밍부터 확인**.
Alias는 제작용 `chicken_disguise`; 기존 게임 몬스터 행에 연결하지 않은 독립 시안이다.

## 기준과 산출물

- [채택한 컨셉 이미지](../../../ArtSource/monster/chicken_disguise/animatic_v1/approved_concept.png)
- [8초 MP4](../../../ArtSource/monster/chicken_disguise/animatic_v1/preview/chicken_disguise_reveal_v1.mp4)
- [Blender 원본](../../../ArtSource/monster/chicken_disguise/animatic_v1/chicken_disguise_animatic_v1.blend)
- [제작 스크립트](../../../Tools/Blender/build_chicken_disguise_animatic.py)
- [영상 읽기 검사](../../../Tools/Blender/check_chicken_disguise_animatic.py)
- [출력 기록](../../../ArtSource/monster/chicken_disguise/animatic_v1/manifest.json), [MP4 재읽기 결과](../../../ArtSource/monster/chicken_disguise/animatic_v1/video_readback.json)

960×540, 30fps, 240프레임, 8초, H.264/MP4, 무음.
정면 +X, +Z 위, 1BU=1m, 원점은 변신 전 닭의 발 사이 바닥이다.
옆 요리사는 크기를 비교하기 위한 별도 단순 모델이며 farmer_a 원본을 사용하거나 수정하지 않았다.

## 타이밍

| 구간 | 표현 |
|---|---|
| 0~2초 | 작은 호흡 뒤 정지 |
| 약 2~3초 | 봉제선 틈이 벌어지고 실땀이 순서대로 풀림 |
| 약 2.7~4.35초 | 긴 팔이 옆으로 펼쳐져 바닥 쪽으로 내려옴 |
| 약 4~5.8초 | 다리와 몸통이 올라오며 닭 탈이 몸 아래에 남음 |
| 약 6.3~7.8초 | 작은 실제 얼굴이 카메라 쪽으로 천천히 돌아옴 |

카메라는 전조 동안 가까이 있고 변신에 맞춰 뒤로 빠진다. 마지막 자세에서 전체 팔다리와 비교 캐릭터가 보인다.
이 시간은 연출 시안이며 기존 AI/공격 예고/판정 시간 변경이 아니다.

## 검수와 한계

별도 background Blender 5.2.2 LTS 프로세스에서 생성했다. 열린 사용자 캐릭터 작업 파일은 수정하지 않았다.
실제 렌더 1·90·150·225프레임을 확인했고, 마지막 손끝이 잘리던 카메라를 넓혔다.
MP4를 Blender에서 다시 열어 해상도·240프레임·30fps·8초를 확인했다.
인코딩된 영상의 100·140·230프레임도 추출해 직접 열어 봤다.

현재는 오브젝트 위치·회전으로 움직이는 **동작 블록아웃**이다.
컨셉아트의 찢어진 천·복잡한 관절·재질·공포 연출을 완성한 외형이 아니다.
실제 천 시뮬레이션 대신 두 껍질 파츠를 벌리고 실땀을 움직인다. 손 접지와 천 간섭은 정밀 검수 전이다.
변형 스켈레톤·웨이트·게임용 클립 베이크·효과음·FBX·UE 연결은 만들지 않았다.
기존 chicken_a 리그·메시·데이터·게임 상태는 변경하지 않았다.

재생성은 별도 background Blender에 `--factory-startup --python Tools/Blender/build_chicken_disguise_animatic.py`로 실행한다.
이후 저장된 .blend를 background로 열어 `--render-anim`으로 영상을 렌더한다.
스크립트는 저장된 사용자 파일에서 실행을 거부하며, v1 작업 경로의 자기 산출물만 갱신한다.
