# CL9 에셋 제작 규격

작성: 2026-09-10. **Blender MCP로 제작하기 전에 읽는 모델·리깅·애니메이션 납품 기준.**
이 폴더는 제작 방법을 정한다. 게임 수치와 행동은 `docs/plan/data/`, 엔진 연결은
`docs/systems/`가 정본이며, 에셋을 맞추려고 판정 크기나 게임 시간을 바꾸지 않는다.

## 읽는 순서

| 문서 | 내용 |
|---|---|
| [common.md](common.md) | 단위·축·명명·재질·성능 예산 |
| [reference-direction.md](reference-direction.md) | 리썰 컴퍼니·R.E.P.O.의 방식과 분위기를 제작 규칙에 적용 |
| [workflow.md](workflow.md) | 제작 의뢰 → Blender MCP → FBX → UE 검수 |
| [전체 제작 goal](../goal/asset-production-loop.md) | 전체 목록 기반 제작·검수·규격 갱신 반복 |
| [production-progress.md](production-progress.md) | 장기 제작의 진행 기록과 다음 재개 지점 |
| [brief-template.md](brief-template.md) | 개별 에셋 제작 카드 양식 |
| [monster/README.md](monster/README.md) | 동물 모델·종별 스켈레톤·동작 목록 |
| [item/README.md](item/README.md) | 운반물·요리 상태·피벗·현재 크기 보정 |
| [map/README.md](map/README.md) | 500cm 모듈러 조각·소품·충돌 |
| [map/henhouse.md](map/henhouse.md) | 첫 스테이지 양계장의 시각 방향과 첫 제작 묶음 |
| [character/README.md](character/README.md) | 플레이어 풀바디·1인칭 팔·기존 리그 호환 |

## 규격의 상태

- **준수:** 현재 기획/코드 연결 조건 또는 이번에 정한 공통 제작 절차. 모든 신규 제작에 적용한다.
- **초안:** 시각 스타일, 폴리곤·텍스처 예산, 아직 확정되지 않은 외형 치수. 첫 샘플에서 조정한다.
- **연결 필요:** 만들 수 있지만 현재 런타임이 소비하지 않는 소켓·상태·표현. 에셋 완성과 게임 연결 완료를 구분한다.

종별 숫자는 기존 기획을 링크하고, 복사할 때는 기준일과 용도를 함께 적는다. 문서끼리 다르면
변경 이력과 소비 코드를 확인한다. 과거의 “없음/미구현” 표제만으로 현재 상태를 판단하지 않는다.

## 파일 배치 — 신규 자체 제작분

```text
ArtSource/<monster|item|map|character>/<asset_id>/
  <asset_id>.blend
  textures/                 # 원본 및 베이크 이미지
  export/                   # 실제 납품 FBX·텍스처
  preview/                  # 정면·측면·3/4·동작 영상
docs/asset/<분류>/<asset_id>.md  # 제작 카드·검수 결과
Tools/Blender/              # 재실행 가능한 제작/검사 스크립트
Saved/BlenderMCP/           # 임시 파일·실행 로그, 납품 원본을 보관하지 않음
Content/CL9Art/<Monsters|Items|Maps|Characters>/<AssetName>/
                            # 신규 UE 임포트 목적지 제안
```

`ArtSource`와 `Content/CL9Art`는 새 제작부터 쓸 경로이며, 기존 팩을 이동하거나 이미 생성된
경로라고 가정하지 않는다. 구매 에셋은 원본 팩을 보존한다. 큰 바이너리는 커밋 전 크기와
저장 정책을 확인하며, 별도 요청 없이 Git LFS나 저장소 제외 규칙을 바꾸지 않는다.

## 작업 완료의 의미

**제작됨 → Blender 검수됨 → UE 임포트 검수됨 → 게임 연결 검수됨**을 따로 기록한다.
렌더 한 장이나 FBX 생성만으로 “게임에서 사용 가능”이라고 표시하지 않는다.
첫 에셋의 UE 왕복 검수가 끝나기 전에는 같은 규격으로 대량 생산하지 않는다.
