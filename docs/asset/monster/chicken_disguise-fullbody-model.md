# chicken_disguise 전신 외형 모델

> **2026-10-11 정지 외형 제작 이력.** 현재 후속 작업은 [TRELLIS 닭·탈착 애니메이션 인계서](chicken_disguise-animation-handoff.md)가 정본이다.
> 아래 파일·수치·중단 상태는 이 정지 모델 버전의 기록이며, 새 제작 결과의 상태로 승계하지 않는다.

상태: 정지 외형 모델 1차본과 Blender 검수 렌더 제작 완료 — 2026-10-11. 사용자 외형 채택은 미확정.

당시 사용자 지시(2026-10-11): 새 닭·몬스터 전신 그림을 기준으로 모델을 제작한다. 애니메이션은 중단 상태를 유지한다.

## 용도와 모습

- 레퍼런스: ArtSource/monster/chicken_disguise/reference/fullbody_redraw_20261011_v1/{chicken_fullbody,monster_fullbody}.png. 두 파일을 직접 열어 확인했다.
- 범위: 두 개의 독립 정지 외형 모델, Blender 원본, 정면·측면·뒤·3/4 렌더, 메시 FBX.
- 기존 반려 모델과 리그를 이어 쓰지 않고 새로 저작한다. 열린 사용자 Blender 파일은 유지하며 독립 background 프로세스를 사용한다.
- 닭: 세로 달걀형 천 몸통, 두툼한 날개, 세 갈래 붉은 벼슬, 짧은 발, 등 봉제선.
- 몬스터: 높은 넓은 어깨, 좁은 몸통, 매우 긴 팔, 바닥을 짚는 손, 밖으로 굽힌 다리, 짧은 목과 창백한 타원 얼굴.

## 치수와 기술 계약

1BU=1m, +X 정면, +Z 위, 바닥 원점. 닭 약 265cm, 몬스터 약 390cm는 외형 제작 가정이며 판정 크기는 미확정. 실제 바운드와 메시 수는 각 manifest.json에 기록한다.
원본: ArtSource/monster/chicken_disguise/model_fullbody_v1/{chicken,monster}/.
클립/리그: 이번 범위에서 제작하지 않는다. UE 임포트·게임 연결·LOD·리토폴로지 검증은 미실행.

## 검수

형태와 재질을 중립광의 정면·측면·3/4에서 레퍼런스와 비교한다. 그림에 보이지 않는 뒤·아래는 제작자가 추정한 구조다. Blender 절차형 재질은 FBX로 자동 이식됐다고 간주하지 않는다.

- Blender 5.2.2 LTS의 독립 background 프로세스에서 각각의 원본을 다시 열고 메시를 검사했다. 열린 사용자 Blender 장면은 수정하지 않았다.
- 두 모델 모두 UV0, 단위 스케일, 바닥 원점을 확인했다. 고립 정점과 면적 0인 면이 없다.
- 몬스터의 몸통·팔·손·다리·발은 하나의 연결된 manifold 표면이다. 얼굴·눈·입·손톱은 별도 편집 파트다.
- 닭의 몸통은 두께가 있는 셸, 날개·벼슬·발·꼬리·봉제선은 별도 편집 파트다. 봉제선 끝과 얼굴 입 표시의 열린 경계는 장식용 구조다.
- 실제 렌더로 정면·측면·뒤·3/4를 생성했다. 초기 손목·발목 연결, 얼굴 비율, 얼굴 슬릿 겹침과 색상을 수정했다.
- 모디파이어를 반영한 삼각형 수: 닭 134,942 / 몬스터 70,356. 게임용 LOD가 아닌 외형 저작 원본이며, 리토폴로지는 별도 작업이다.
- 리그·액션은 없으며 애니메이션 중단 요청을 유지했다. 손/천 접촉이나 의상 탈착 시 관통 검수는 이번 정지 모델 검수에 포함되지 않는다.
- 그림의 미세한 천 섬유와 해부학적 면은 완전히 재현된 상태가 아니다. 이번 산출물은 사용자가 형상과 비율을 검토할 수 있는 1차 모델이다.

## 산출물

- `ArtSource/monster/chicken_disguise/model_fullbody_v1/chicken/chicken_fullbody.blend`
- `ArtSource/monster/chicken_disguise/model_fullbody_v1/monster/monster_fullbody.blend`
- 각 폴더의 `export/*_fullbody.fbx`: 메시/기본 재질. 절차형 표면 텍스처 베이크는 미실행.
- 각 폴더의 `preview/{front,side,back,three_quarter,views}.png`: 실제 Blender 렌더와 이를 배열한 검수 시트.
- `ArtSource/monster/chicken_disguise/model_fullbody_v1/models_preview.jpg`: 두 모델 실제 렌더 비교.
- 각 폴더의 `manifest.json`, `mesh_qa.json`: 레퍼런스 해시, 치수, 메시/UV, 연속성 검수 기록.

재생성: `Tools/Blender/build_chicken_disguise_appearance.py`를 factory-startup Blender에서 `--asset chicken` 또는 `--asset monster`로 실행한다. 기존 본 작업 출력 재생성은 `--replace-owned`를 명시해야 한다. 검수는 `audit_chicken_disguise_appearance.py`, 시트 생성은 `preview_chicken_disguise_appearance.py`를 사용한다.
