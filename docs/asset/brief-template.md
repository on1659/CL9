> 기존 카드(2026-09-21 이전 작성)는 `상태:` 줄만 소급해 붙였다. 나머지 절(치수와 기술 계약 / 클립 / 내보내기와 검수 기록)은 새 카드부터 이 양식을 따른다.

# <asset_id> 제작 카드

이미 같은 모델의 현재 제작 인계서가 있으면 그 문서를 보강하고 이 카드를 중복 생성하지 않는다.
공통 규칙은 [에셋 안내](README.md)의 정본 연결을 사용하고 모델별 값·예외·실제 결과만 작성한다.

상태: 초안 / 제작됨 / Blender 검수됨 / UE 임포트 검수됨 / 게임 연결 검수됨
작성일: YYYY-MM-DD · 변경일: YYYY-MM-DD

<a id="concept-checkpoint"></a>

## 제작 범위·승인·재개 기록

`$cl9-model-concept`를 사용할 때 추가한다. 기존 인계서에서는 이 절을 같은 문서에 넣고 중복 카드를 만들지 않는다.
절차·필드 의미·승인 의존성은 [model-concept.md](model-concept.md)가 정본이다.
아래 블록은 빈 양식이다. 실제 대상·확인된 사실로 채우며 작성만으로 승인이 생기지 않는다.

```yaml
concept_pipeline:
  schema_version: 2
  asset_id: null
  scope: null # concept_only / model_only / production — 실제 요청으로 선택
  requested_outputs: []
  review_policy: concept_and_model
  review_override: null
  phase: brief
  status: working
  updated_at: null
  concept:
    candidate_version: null
    approved_version: null
  blender:
    candidate_version: null
    approved_version: null
    source_concept_version: null
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
      level: not_tested # imported / preview / gameplay_verified
      engine: null
      asset_paths: []
      evidence: null
  pending_review: null
  checkpoint: null
  job: null
  last_completed: null
  next_action: null
  blocker: null
```

### 버전 파일 목록

경로는 프로젝트 상대 경로로 기록한다. 각 manifest의 `files`에 실제 원본·렌더·의존 파일과 SHA256을 둔다.
같은 버전 ID/승인 manifest를 덮어쓰지 않는다. 승인 여부의 정본은 아래 사용자 이력이다.

| 단계 | 버전 | 기반 컨셉 버전 | manifest 경로 | manifest SHA256 |
|---|---|---|---|---|

### 사용자 승인·수정 이력

실제 발언과 그 대상 버전을 기록한다. 채팅/메시지 식별자가 있으면 근거에 함께 남기고, 없으면 만들지 않는다.
승인에는 확인한 manifest 해시를 연결한다. NO/수정은 유지할 특징·수정 요구·다음 후보를 적는다.

| 시각(시간대 포함) | 단계·대상 버전 | 결정(OK/NO/수정/철회) | 사용자 발언과 근거 | manifest SHA256 | 다음 행동 |
|---|---|---|---|---|---|

## 용도와 모습

- 분류 / Alias / SubAlias:
- 사용할 장소·플레이 상황:
- 기획 및 시스템 문서:
- 기존 메시·리그를 재사용하는지:
- 형태·재질·색·레퍼런스·출처:
- 기준 이미지의 채택/제작 후보 구분, 입력 경로·SHA256:
- 생성 도구 사용 시 도구 지침 링크, 버전·생성 조건·원시 출력·보정 작업 사본:
- 리썰/R.E.P.O.에서 참고한 특징 → 이번 모델·동작에 적용하는 방식:
- 이번 제작 범위 / 연결이 필요한 기능:

## 동물·탈 구조와 짧은 시험

탈착 없는 대상은 해당 없음을 기록한다. 방법은 [공통 제작 절차](workflow.md#animal-production)를 따르고 여기에는 대상별 결정만 적는다.

- 유지할 외형·비율과 종별 차별점:
- 본체 관절/이동 유형, 탈 내부 수납 자세·열리는 곳:
- 몸/탈/단단한 부속 분리, 손 집기·발/손 지지 위치:
- 가장 어려운 2~3초 시험의 동작·원본/영상·실패 프레임:
- 1배속·중립광·측면 확인과 수치 검사의 범위/단위/허용치:
- 짧은 엔진 왕복에서 검증한 몸/탈 변형의 전달 방식:
- 시험 결론·남은 구조 문제·다음 제작 단계:

## 치수와 기술 계약

| 항목 | 값 및 상태(준수/초안/연결 필요) |
|---|---|
| 원본 크기 X×Y×Z (cm) | |
| 런타임 크기·자동 배율 | |
| 판정 반경/캡슐·출처 | |
| 피벗·정면·기준 포즈 | |
| 충돌 소유자·메시 충돌 필요 여부 | |
| 원본·FBX·UE 목적 경로 | |
| 삼각형·재질 슬롯·텍스처・LOD | |
| Skeleton·뼈 수·최대 영향 뼈 수 | |
| 부착 기준·소켓/마커 소비처 | |

## 클립 / 상태별 변형

| 이름 | 루프 | 원본 길이(초)·fps·범위 | 기준 속도 cm/s | 판정 시간·출처 | 영상 경로 | 연결 상태 |
|---|---|---|---|---|---|---|
| | | | | | | |

애니메이션 없는 소품은 “해당 없음”. 아이템은 날것/익음/탐/균열 등의 연결 방식을 적는다.
애니메이션 제작이 요청되면 [실제 제작·영상 납품 기준](model-concept.md#animation-delivery)을 적용하고,
그 요구를 후속 제작 실행 프롬프트에도 포함한다.

## 내보내기와 검수 기록

- Blender 버전 / exporter 이름·버전 / 축·단위·베이크 옵션:
- UE 버전 / importer / Skeleton 지정 / 임포트 옵션:
- 중립광·어두운 조명·와이어·클립 미리보기 경로:
- 몸–탈 / 탈 자체 / 손 집기 / 발 접지 / 외형 / 전환 QA를 구분한 결과:
- 1배속 및 비교 영상 묶음, 우측 상단 제작 방식·버전·속도·렌더/인게임 표시:
- 엔진 임포트·프리뷰·실제 게임 상태 연결의 구분과 실행/로그/재열기 증거:
- 동작 영상의 실제 재생/디코딩·화면 검수 결과, 사용자에게 제시한 영상·시각·피드백:
- 크기·원점·전방·충돌·리그·루프 확인 결과:
- 게임 시나리오 / 2인 PIE / 성능 측정 결과:
- 미실행 항목과 이유:
- 알려진 문제·다음 수정:
- 재개 정보: 마지막 실제 완료 단계·편집 원본·다음 실행 명령·실행 중 프로세스 유무:
