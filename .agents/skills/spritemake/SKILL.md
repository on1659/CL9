---
name: spritemake
description: CL9에서 SpriteMake를 참조하여 게임용 2D 스프라이트, 투명 PNG, 아이콘, 애니메이션 아틀라스의 제작 배치, 프롬프트, manifest, QA, 프리뷰와 납품을 준비한다. SpriteMake 호출이나 스프라이트 제작·검수 요청에 사용하며, 3D 모델·리깅·Blender 제작은 기존 CL9 에셋 워크플로우를 따른다.
---

# SpriteMake for CL9

CL9의 짧은 리소스 요청을 SpriteMake 제작 배치로 연결한다. 사용자에게 긴 요청서 양식을 요구하지 않는다. 응답과 작업 보고는 한국어로 한다.

## 작업 위치와 사전 확인

- 게임 프로젝트: 이 스킬이 설치된 CL9 저장소. 현재 위치는 `C:\Work\CL9`.
- 제작 프로젝트: 현재 `C:\Work\SpriteMake`. 기본적으로 CL9의 형제 폴더 `SpriteMake`를 찾고, 사용자가 다른 위치를 지정하면 그 위치를 쓴다.
- SpriteMake의 훅·템플릿·문서를 원본에서 읽는다. CL9에 전체 프로젝트나 생성 배치를 복제하지 않는다.
- CL9의 `CLAUDE.md`, 적용되는 `AGENTS.md`, `docs/asset/README.md` 및 해당 리소스의 소비 코드/제작 카드를 읽는다.
- SpriteMake의 `AGENTS.md`, `README.md`, `USAGE.md`, `HANDOFF.md`를 읽는다. 문서 안의 과거 `D:\Work\vibe\SpriteMake` 경로보다 실제 확인한 제작 루트를 사용한다.
- 제작 루트나 훅이 없으면 누락 경로를 알리고 해당 작업을 보류한다. CL9에서 같은 이름의 훅이 있다고 가정하지 않는다.

현재 두 프로젝트 경로에서 사용할 초기 설정:

```powershell
$gameRoot = (Resolve-Path -LiteralPath 'C:\Work\CL9').Path
$spriteRoot = (Resolve-Path -LiteralPath 'C:\Work\SpriteMake').Path
```

## 새 리소스 요청

사용처, 게임 안의 표시 크기, 기존 스타일, 소비 코드, 최종 경로를 확인한다. 모르는 값은 명시적인 미정 항목으로 남겨 세팅을 진행하며, 런타임 배치에 영향을 주는 값은 QA와 통합 전에 확정한다.

사용자의 원문 요청을 문자열 변수에 넣고 SpriteMake의 intake를 호출한다. 요청문을 셸 코드로 조립하거나 실행하지 않는다.

```powershell
$requestText = 'CL9 퀵슬롯용 음식 아이콘 제작 요청'
& (Join-Path $spriteRoot '.agents\hooks\sprite-intake.ps1') -Request $requestText -GameProjectRoot $gameRoot -ProjectRoot $spriteRoot -Mode codex
```

- 사용자가 생성 방식을 지정하지 않고 만들기/그리기를 요청하면 `codex`를 기본값으로 쓴다. 매번 선택 질문을 요구하지 않는다.
- 세팅만 요청하면 `codex-setup`, 엄격한 `gpt-image-2`를 요청하면 `gpt-image-2`를 쓴다.
- 훅이 출력한 batch 경로를 사용한다. 이름과 timestamp를 별도로 재계산하지 않는다. CL9 관련 batch임을 `BATCH.md`에 기록한다.
- 기존 batch 이어가기나 PNG 검수 요청은 그 batch를 재사용하며 intake로 새 배치를 만들지 않는다.
- 생성 배치 준비가 끝난 후에만 요청된 이미지 생성 단계로 진행한다. 스킬 설치나 세팅 요청 자체는 이미지 생성 요청이 아니다.

배치에는 `BATCH.md`, `REQUESTS.md`, `PROMPT.md`, `MANIFEST.md`, `QA.md`와 `prompts/`, `generated/`, `source/`, `manifests/`, `tools/`, `qa/`, `final/`이 있어야 한다.

SpriteMake의 다음 템플릿을 필요한 작업에 맞게 적용한다:

- 요청/폴더: `templates/folder-md-setup-template.md`, `templates/batch-sprite-request-template.md`, `templates/resource-request-template.md`
- 아틀라스/검수: `templates/sprite-atlas-template.md`, `templates/asset-qa-checklist.md`
- 애니메이션: `templates/animation-manifest-template.json`, `templates/animation-player-template.html`

## 생성과 런타임 계약

SpriteMake는 이미지 API 클라이언트가 아니다. API 키를 저장하거나 이미지 API를 직접 호출하는 스크립트를 추가하지 않는다.

- Codex 직접 생성은 세션의 이미지 생성 도구로 수행하며 모델은 미검증이라고 기록한다. `gpt-image-2`를 강제하거나 검증했다고 주장하지 않는다. 생성 도구가 없으면 준비된 프롬프트와 제약을 인계하고 생성 완료로 표시하지 않는다.
- 엄격한 외부 생성용 프롬프트는 기본 `gpt-image-2`를 요구한다. 사용자가 다른 모델을 명시하면 그 선택을 따른다. 실제 엄격 생성은 사용자가 외부 생성 환경에 프롬프트를 입력한다.
- 기본 모델 프롬프트에 `Generation Target` 블록이 없으면 생성 전달 전에 아래 훅을 실행한다. 사용자가 다른 모델을 지정한 경우 기존 `gpt-image-2` 요구와 충돌하지 않게 프롬프트를 수정한다.

```powershell
$promptPath = Join-Path $batchRoot 'PROMPT.md' # $batchRoot는 intake 출력 경로
& (Join-Path $spriteRoot '.agents\hooks\prompt-inject.ps1') -InputPath $promptPath -OutputPath $promptPath
```

아틀라스는 최종 canvas 크기, columns/rows, cell 크기, 각 열/행 경계, 행/열의 의미와 프레임 순서를 명시한다. gutter와 canvas 바깥 padding은 없고 투명 여백은 고정 cell 내부에만 둔다. 느슨한 contact sheet는 중간 산출물이다.

Contact Anchor는 게임이 배치할 때 사용하는 cell 내부 좌표와 의미다. Y-axis baseline/source plane은 프레임 사이에 유지할 로컬 기준선이다. 둘을 프롬프트, manifest, QA에 각각 적고 좌표가 pixel인지 normalized인지 표시한다.

정적 PNG는 해당 크기·alpha·anchor 계약을 정하고 불필요한 animation grid를 만들지 않는다. CL9의 기존 퀵슬롯 아이콘은 `docs/asset/README.md`와 소비 코드를 확인하여 흰 실루엣+알파, 128×128, 직교 3/4 위 시점, 긴 변 88% 규격을 맞춘다. 다른 UI 이미지에 이 아이콘 규격을 일괄 적용하지 않는다.

## 검수와 프리뷰

PNG는 배치의 `generated/`에 저장하거나 그곳에서 원본을 참조한다. 이미지를 직접 확인하고 실제 파일에서 다음을 측정해 `qa/` 보고서와 `QA.md`에 기록한다:

- 최종 크기, alpha 유무, 투명 모서리, 전체 alpha bbox.
- 아틀라스의 grid divisibility, cell 크기, cell별 alpha bbox, 각 cell의 의도된 한 프레임, 잘림과 인접 cell 침범.
- anchor 의미/좌표, baseline/source plane 안정성, 실제 표시 크기에서의 가독성.

애니메이션이면 manifest와 배치별 `tools/animation-preview.html`을 만든다. 빈 원본 템플릿을 완성된 프리뷰로 안내하지 않는다. 더블클릭 `file://` 실행을 지원하고 기본 manifest/sheet를 자동 로드한다. 필요하면 manifest와 현재 PNG를 data URL로 내장하고 재생, sheet/animation 전환, FPS/scale, grid, frame bounds, anchor, baseline, bbox 표시를 제공한다.

실제 브라우저에서 `ready: true`, 비어 있지 않은 canvas, console error 0, alpha bbox pixel read가 차단되지 않음을 확인한다. 검증 도구가 없거나 항목을 확인하지 못하면 미검증이라고 기록하며 QA 통과로 처리하지 않는다. QA 실패 파일은 `generated/`에 유지한다.

기존 배치 상태는 아래처럼 원본 훅으로 확인한다. `final-candidate` 상태는 실제 QA 통과를 대신하지 않는다.

```powershell
& (Join-Path $spriteRoot '.agents\hooks\symphony.ps1') -ProjectRoot $spriteRoot
# 지속적인 상태 보고서가 필요한 경우에만 -WriteReport 추가
```

## CL9 납품

QA 통과 파일만 SpriteMake batch의 `final/`로 승격하고, 사용자 요청에 게임 반영이 포함되어 있으면 CL9로 복사한다. SpriteMake 배치에는 CL9 target source PNG 경로, manifest 경로, UE import destination을 구분하여 남긴다.

- CL9 원본 파일은 기존 `ArtSource/` 규칙을 따른다. 아이템 아이콘의 원본은 `ArtSource/item/<asset_id>/icons/T_Icon_<alias>.png`, UE 목적지는 `/Game/UI/Icons/T_Icon_<alias>`이다. 다른 리소스는 소비 코드와 문서에서 목적지를 확정한다.
- PNG를 `Content/`에 복사한 것만으로 Unreal 임포트가 완료되었다고 표시하지 않는다. UE texture/material 설정과 slicing, animation key, pivot/anchor 변환은 실제 소비 방식에 맞춰 확인한다.
- UE 임포트나 에디터/빌드/PIE 작업이 필요한 경우 `docs/workflow/sessions.md`의 잠금·세션 규약을 적용한다.
- 스프라이트 제작 QA, UE 임포트 QA, 게임 연결 QA는 별개 상태로 기록한다. 요청 범위 안에서 필요한 통합까지 진행하고, 확인하지 않은 단계를 완료로 표시하지 않는다.
- 인계할 때 batch 경로, PNG/manifest/프리뷰 경로, QA 결과, CL9 목적지와 남은 통합 항목을 제공한다.
