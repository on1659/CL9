# CL9 에셋 제작 규격

작성: 2026-09-10 · 문서 연결 정리: 2026-10-13. **모델·리깅·애니메이션 제작의 공통 시작점.**
Blender 직접 저작과 TRELLIS 생성 후 Blender 보정을 함께 다룬다. 별도 `docs/model/` 체계를 만들지 않는다.
이 폴더는 제작 방법을 정한다. 게임 수치와 행동은 `docs/plan/data/`, 엔진 연결은
`docs/systems/`가 정본이며, 에셋을 맞추려고 판정 크기나 게임 시간을 바꾸지 않는다.

## 읽는 순서

| 문서 | 내용 |
|---|---|
| [art-production-plan.md](art-production-plan.md) | 과거 제안 보존 — 몬스터 선정 철회, 제작 시작문 실행 금지. 컨셉아트 기준으로 재논의 중 |
| [common.md](common.md) | 단위·축·명명·재질·성능 예산 |
| [reference-direction.md](reference-direction.md) | 리썰 컴퍼니·R.E.P.O.의 방식과 분위기를 제작 규칙에 적용 |
| [design-concept.md](design-concept.md) | 톤 축(귀여움)·실루엣 계열·소재 불가지 규칙 5개 — 사이트가 바뀌어도 유지되는 것 |
| [workflow.md](workflow.md) | 컨셉 승인 흐름과 본제작 → Blender → FBX → UE 검수의 연결 |
| [model-concept.md](model-concept.md) | `$cl9-model-concept`: 컨셉·모델 검토 → 짧은 동작 시험 → 리깅·클립·인게임 검수, 범위별 저장·재개 |
| [tools/trellis2.md](tools/trellis2.md) | 검증된 로컬 TRELLIS 설치·공통 실행법·중단 재개·생성 결과의 한계 |
| [닭 탈 몬스터 현재 인계](monster/chicken_disguise-animation-handoff.md) | TRELLIS 닭 생성 → 본체/탈 정리·리그 → 벗기·회수·재착용 → 영상 검수 |
| [전체 제작 goal](../goal/asset-production-loop.md) | 전체 목록 기반 제작·검수·규격 갱신 반복 |
| [production-progress.md](production-progress.md) | 장기 제작의 진행 기록과 다음 재개 지점 |
| [catalog.md](catalog.md) | 전체 제작 목록 — 행마다 모델·리그·클립·UE 연결·QA 상태와 원본 경로 |
| [motion-plan.md](motion-plan.md) | 게임에 필요한 모션 — 기존 재사용·닭 탈 괴물의 게임용 분리·신규 보강 우선순위 |
| [animation/asset-todo-2026-10-19.md](animation/asset-todo-2026-10-19.md) | 만들어야 할 모델 20·몬스터 클립 156·플레이어 클립 33 목록(2026-10-19 세부 기획 기준, ★ 작은 안). 엑셀용 .csv 같이 있음 |
| [brief-template.md](brief-template.md) | 개별 에셋 제작 카드 양식 |
| [monster/README.md](monster/README.md) | 동물 모델·종별 스켈레톤·동작 목록 |
| [item/README.md](item/README.md) | 운반물·요리 상태·피벗·현재 크기 보정 |
| [map/README.md](map/README.md) | 500cm 모듈러 조각·소품·충돌 |
| [map/henhouse.md](map/henhouse.md) | 첫 스테이지 양계장의 시각 방향과 첫 제작 묶음 |
| [character/README.md](character/README.md) | 플레이어 풀바디·1인칭 팔·기존 리그 호환 |

## 문서 역할 — 같은 규칙은 한 곳에서 관리

| 내용 | 정본 | 다른 문서에서 할 일 |
|---|---|---|
| 전체 제작 원칙·단위·축·재질·성능 | `common.md`, `design-concept.md`, `reference-direction.md` | 모델별 확정사항/예외와 근거만 기록하고 링크 |
| 제작·내보내기·검수 공통 흐름 | `workflow.md` | 모델별 필요한 단계·실제 검수 결과만 추가 |
| 제작 범위·컨셉/Blender 승인·중단 재개 절차 | `model-concept.md` | 스킬은 실행 진입점, 모델별 승인·피드백·현재 단계는 기존 카드/인계서에만 기록 |
| 생성 도구의 환경·사용법·복구 | `tools/<도구>.md` | 모델 카드에 입력/출력과 해당 도구 지침 링크 |
| 모델 전체 목록·진행 요약 | `catalog.md` | 상세 설명을 복제하지 않고 모델별 정본으로 연결 |
| 게임 전체 모션 제작·재사용 우선순위 | `motion-plan.md` | 상세 규격·상태는 모델 카드/인계서, 기술 연결은 `docs/systems/animation.md`로 연결 |
| 모델별 현재 외형·리그·동작·작업 지시 | 기존 분류 아래 제작 카드 또는 명시한 인계서 하나 | 여러 시안 문서를 동시에 현재 지시로 취급하지 않음 |
| 옛 시안과 실험 이력 | 기존 모델별 기록, `iterations/`, `evidence/` | 과거 상태임을 표시하고 현재 인계서 링크 |
| 실제 원본·검수 증거 | `ArtSource/`의 작업 버전별 파일·manifest·QA | 현재 파일·버전과 문서의 상태를 대조 |

새 모델은 [제작 카드 양식](brief-template.md)으로 해당 분류에 시작한다. 상세 지시서가 이미 있으면 보강한다.
닭 탈은 [현재 인계서](monster/chicken_disguise-animation-handoff.md)가 현재 작업의 정본이며,
`chicken_disguise-{animatic,natural-production,fullbody-model}.md`는 과거 단계별 기록이다.
모델/애니메이션이 아직 없으면 미제작으로 남긴다. AI에게 전달할 실행 시작문도 현재 인계서에서 관리한다.

컨셉부터 시작하거나 모델·리깅·애니메이션·인게임 검수까지 제작할 때는 `$cl9-model-concept <대상> <제작 범위> 시작해줘/이어서 해줘`를 사용한다.
진행 기록은 [제작 카드의 승인·재개 양식](brief-template.md#concept-checkpoint)을 기존 정본에 넣는다.
승인 절차와 완료 조건은 [model-concept.md](model-concept.md)에서 관리한다.

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
ArtSource/ui/               # 게임 메시가 없는 UI 원본(2026-10-11~): icons/T_Icon_<alias>.png + icons/manifest.json(아이템 아이콘과 같은 규약),
                            # 그리는 스크립트도 이 폴더에 둔다(draw_mic_icon.py) → UE /Game/UI/Icons/T_Icon_<alias>
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

## 음성 채팅 에셋 — `/Game/Sounds/Master` · `/Game/UI/Icons`

`Tools/Unreal/create_voice_assets.py`(에디터 파이썬, 멱등 — 값이 이미 맞으면 저장하지 않는다)가 만든다. 2026-10-11 스크립트 작성, 실행은
리드가 C++ 빌드 뒤에 한다. 요구 정본 [../goal/voice-chat.md](../goal/voice-chat.md) 결정 7. 코드는 경로를 갖지 않는다 — `SC_Voice` 는
ini `VoiPSoundClass` 와 `SettingsTable.VoiceClass` 로 들어간다. 증거 `evidence/037-voice-assets.json`.
**음성 감쇠도 이 스크립트가 만든다**(2026-10-12, [../goal/voice-eos-redpoint.md](../goal/voice-eos-redpoint.md) 결정 5) — Redpoint 가 감쇠 에셋을 받는다. 값 정본은 [../plan/data/rules.md](../plan/data/rules.md) "음성 감쇠 에셋 값" 표이고, 에셋은 `SettingsTable.VoiceAttenuation`·`VoiceSpeakerAttenuation` 으로 들어간다. 확인은 `verify` 모드.

- **`SC_Voice`** — 음성 채팅 SoundClass, `SC_Master` 의 자식, 볼륨 1.0. 반응 음성 `SC_VO` 와는 다른 클래스이고 `SC_VO` 는 그대로 둔다.
- **`SA_CL9_Voice`** — 핫라인이 없는 산 화자의 목소리 감쇠. 구, Inner 150 · SilentAt 1500(감쇠 거리 1350) · Linear, 공간화 켬, 오클루전·공기 흡수 끔, 플러그인 없음.
- **`SA_CL9_VoiceSpeaker`** — 핫라인 상대 자리의 무전기 스피커 감쇠. 지금은 값이 `SA_CL9_Voice` 와 같다.
- **`T_Icon_mic`** — HUD 에서 내가 말하는 동안 보이는 마이크 아이콘. 흰 실루엣 + 알파 128×128(아이템 아이콘 규약, 세로 88 %)이고, 게임 메시가
  없어서 렌더하지 않고 `ArtSource/ui/draw_mic_icon.py` 가 정면 픽토그램을 그린다 → `ArtSource/ui/icons/T_Icon_mic.png`.

## 작업 완료의 의미

**제작됨 → Blender 검수됨 → UE 임포트 검수됨 → 게임 연결 검수됨**을 따로 기록한다.
렌더 한 장이나 FBX 생성만으로 “게임에서 사용 가능”이라고 표시하지 않는다.
첫 에셋의 UE 왕복 검수가 끝나기 전에는 같은 규격으로 대량 생산하지 않는다.
`concept_pipeline` v2의 `complete`는 기록된 `scope`에 한정한다. 전체 제작은 요청된 리그·동작·게임 검수까지 완료해야 한다.
v1의 기존 `complete`는 컨셉·Blender 초안 인계 완료일 뿐이며, 게임 검수 완료로 자동 승격하지 않는다.
