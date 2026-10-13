# CL9 에셋 제작 규격

작성: 2026-09-10. **Blender MCP로 제작하기 전에 읽는 모델·리깅·애니메이션 납품 기준.**
이 폴더는 제작 방법을 정한다. 게임 수치와 행동은 `docs/plan/data/`, 엔진 연결은
`docs/systems/`가 정본이며, 에셋을 맞추려고 판정 크기나 게임 시간을 바꾸지 않는다.

## 읽는 순서

| 문서 | 내용 |
|---|---|
| [common.md](common.md) | 단위·축·명명·재질·성능 예산 |
| [reference-direction.md](reference-direction.md) | 리썰 컴퍼니·R.E.P.O.의 방식과 분위기를 제작 규칙에 적용 |
| [design-concept.md](design-concept.md) | 톤 축(귀여움)·실루엣 계열·소재 불가지 규칙 5개 — 사이트가 바뀌어도 유지되는 것 |
| [workflow.md](workflow.md) | 제작 의뢰 → Blender MCP → FBX → UE 검수 |
| [전체 제작 goal](../goal/asset-production-loop.md) | 전체 목록 기반 제작·검수·규격 갱신 반복 |
| [production-progress.md](production-progress.md) | 장기 제작의 진행 기록과 다음 재개 지점 |
| [catalog.md](catalog.md) | 전체 제작 목록 — 행마다 모델·리그·클립·UE 연결·QA 상태와 원본 경로 |
| [motion-plan.md](motion-plan.md) | 게임에 필요한 모션 — 기존 재사용·닭 탈 괴물의 게임용 분리·신규 보강 우선순위 |
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
  export/                   # 실제 납품 FBX·텍스처·manifest.json
  icons/                    # 아이템만: 퀵슬롯 아이콘 T_Icon_<alias>.png — 흰 실루엣+알파 128×128(RGB 전부 1, 색·흐림은 UI), 직교 3/4 위 시점, 긴 변 88%; Tools/Blender/render_item_icons.py → UE /Game/UI/Icons/T_Icon_<alias>
  preview/                  # 정면·측면·3/4·동작 영상
ArtSource/<분류>/<asset_id>/<part>/<part>.blend
                            # 부품이 여럿인 에셋은 부품 폴더 한 층을 더 둔다 — 부품마다 blend·export·preview 가 따로 있다.
                            # 실례: map/food_truck/{body,steps}, henhouse_structure/{beam,ceiling,pillar},
                            # henhouse_access_parts/{tread,wall125,wall250}, henhouse_yard/{ground,sign_*}, truck_parts/{pad,panel}
docs/asset/<분류>/<asset_id>.md  # 제작 카드·검수 결과
Tools/Blender/              # 재실행 가능한 제작/검사 스크립트
Saved/BlenderMCP/           # 임시 파일·실행 로그, 납품 원본을 보관하지 않음
Content/<Characters|Items|Monsters|Maps>/<AssetName>/
                            # UE 임포트 목적지 — 규약 폴더 그대로 (2026-09-12, CL9Art 폴더 폐지)
```

임포트 목적지의 예외 둘(2026-09-12 이동 결과, 2026-09-21 확인): 종별 AnimBP `ABP_<종>_A` 5개는 `Content/Monsters/` 바로 아래에
있고(클립·SK 는 `Monsters/<종>A/`), 아이템·시설 블루프린트 `BP_CL9*` 9개(`BP_CL9Item`·`Cart`·`FlashItem`·`LampItem`·`Kitchen`·
`Counter`·`Jar`·`TruckPad`·`TruckPanel`)는 `Content/Items/` 바로 아래에 있다 — 시설 메시는 BP 옆 `Items/<시설>/` 폴더다.
캠프파이어·밤하늘처럼 맵 소품·재질은 `Content/Maps/Henhouse/Props/`·`Content/Maps/Sky/` 로 간다([map/README.md](map/README.md)).

`ArtSource`는 새 제작부터 쓸 경로이며, 기존 팩을 이동하거나 이미 생성된
경로라고 가정하지 않는다. 구매 에셋은 원본 팩을 보존한다. 큰 바이너리는 커밋 전 크기와
저장 정책을 확인하며, 별도 요청 없이 Git LFS나 저장소 제외 규칙을 바꾸지 않는다.

## 효과 에셋 — `/Game/Effects`

`Content/Effects/` 의 여섯 에셋은 Blender 를 거치지 않는다 — 에디터 파이썬이 만들거나 리드가 에디터에서 직접 만들었다.
코드는 에셋 경로를 갖지 않는다: 재질은 `CL9Rules.HitFlashMaterial`, 퍼프는 `CL9Rules.HitPuffSystem`, 셰이크는 `ShakeTable`
행, 파동은 `WaveTable` 행으로 들어간다(2026-09-21 확인).

- **`M_CL9_HitFlash`** — 맞은 몬스터와 잡힌 플레이어의 스켈레탈 메시에 `SetOverlayMaterial` 로 덧씌우는 재질(Unlit·Translucent·양면,
  파라미터 `Opacity` 기본 0.6 · `Color` 기본 (1, 0.05, 0.05) — 붉게 번쩍인다). `Tools/Unreal/create_hit_feedback_assets.py` 가 만든다.
- **`CS_HitShake`** — 휘두른 것이 맞았을 때 때린 쪽 카메라가 한 번 튀는 흔들림. **사인파 오실레이터**(`UWaveOscillatorCameraShakePattern`,
  모든 축 InitialOffset Zero): 0.12초 · 블렌드 인 0 · 블렌드 아웃 0.04 · Pitch −3°@4Hz · FOV −3°@4Hz · 위치·Yaw·Roll 0. 2026-09-15 의
  펄린판을 2026-09-18 에 번복했다(펄린은 임의 위상이라 정점을 보장 못 함, decisions.md) — 정본은 같은 스크립트의 `HIT_SHAKE` 상수.
- **`CS_HurtShake`** — 잡힌 쪽의 묵직하고 긴 흔들림. `CS_StaminaShake` 를 복제한 **펄린 노이즈** 그대로: 0.50초 · 블렌드 아웃 0.30 ·
  Pitch 2.5°@7 · Yaw 1.5°@6 · Roll 3.5°@5(Roll 주역) · 위치·FOV 0. 정본 `HURT_SHAKE` 상수.
- **`CS_StaminaShake`** — 스태미나 게이지가 구동하는 지속형 흔들림(`UDefaultCameraShakeBase` + `UPerlinNoiseCameraShakePattern`).
  두 원샷 셰이크의 복제 원본이며 `ShakeTable` `Stamina` 행이 가리킨다.
- **`NS_CL9_HitPuff`** — 타격 퍼프 나이아가라. 몬스터가 Stun·Dead 에 들어가는 순간 메시 중심에서 스폰, `User.Scale` Dead 3.0 / 그 외 1.0
  (`CL9Rules.HitPuffSystem` 열, 2026-09-18). 정본 [../systems/monster.md](../systems/monster.md) "타격의 표현".
- **`NS_CL9_Wave`** — 소음 반경을 그리는 파동 링 나이아가라(2026-09-16, 리드가 에디터에서 제작). 이미터 하나(`Ring`), 벽 차폐는 CPU
  Collision(`CL9_Occlusion` 채널). 정본 [../systems/wave.md](../systems/wave.md) "나이아가라 에셋의 실제 모양".

`ShakeTable` 행(`FCL9ShakeInfo`)은 `Shake`·`StartAt`·**`Scale`(기본 1.0, 2026-09-18)** 이다 — 진폭·주파수·블렌드는 에셋에 있고 표는
배율만 갖는다. `Hit`·`Hurt` 원샷 행은 `StartAt` 을 읽지 않고 `Scale` 세기로 한 번 재생한다(게이지 행 `Stamina` 만 `StartAt` 을 본다).
값을 바꾸려면 스크립트 상수를 고치고 다시 돌린다(멱등 — 다시 돌리면 값만 덮어쓴다). 요구 정본 [../goal/hit-feedback.md](../goal/hit-feedback.md)·
[../goal/game-feel.md](../goal/game-feel.md), 연결 [../systems/character.md](../systems/character.md) "흔들림".

## 작업 완료의 의미

**제작됨 → Blender 검수됨 → UE 임포트 검수됨 → 게임 연결 검수됨**을 따로 기록한다.
렌더 한 장이나 FBX 생성만으로 “게임에서 사용 가능”이라고 표시하지 않는다.
첫 에셋의 UE 왕복 검수가 끝나기 전에는 같은 규격으로 대량 생산하지 않는다.
