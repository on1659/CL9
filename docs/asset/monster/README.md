# 몬스터 제작 규격

먼저 [공통](../common.md)과 [기획 행](../../plan/data/monster/common.md)을 읽는다.
몬스터는 **재료의 주인**이다. 종과 상태가 어두운 실루엣·자세로 구분되어야 한다.

## 신규 외형 제작의 시작점

- **닭 탈 괴물 `chicken_disguise`:** [현재 제작·인계 지시](chicken_disguise-animation-handoff.md).
  TRELLIS 닭 생성, 기존 본체의 보정, 속 빈 탈·리그·벗기/재착용·실제 영상 검수를 이 문서에서 관리한다.
- [TRELLIS 공통 사용법](../tools/trellis2.md)은 도구 환경과 재개 규칙의 정본이다.
- [전신 외형 기록](chicken_disguise-fullbody-model.md), [이전 자연 동작 기록](chicken_disguise-natural-production.md),
  [초기 시안 이력](chicken_disguise-animatic.md)은 비교·실패 분석 자료다. 현재 제작 지시를 별도로 복제하지 않는다.
- `chicken_disguise`는 기존 런타임 `chicken_a`와 구분된 독립 시안이다. 아래 기존 종의 뼈·Catch 시간·UE 연결 상태를
  새 탈착 모델에 자동 적용하지 않는다. 게임 연결은 별도 검수 전까지 미연결로 기록한다.

## 카드

| 카드 | 종 | 역할(2026-09-20 기준) |
|---|---|---|
| [chicken_a.md](chicken_a.md) | chicken | 양계장 주인 — 추격·잡기 예고(`PounceWindup`)·잡기 |
| [goose_a.md](goose_a.md) | goose | 경보꾼 — Catch 없음(Honk·Sleep·AlertWindup) |
| [pig_a.md](pig_a.md) | pig | 잡히는 순간이 곧 처형(`bExecuteOnCatch`, 2026-09-20 저녁) |
| [raccoon_a.md](raccoon_a.md) | raccoon | 스토커 — 등 뒤 처형(2026-09-20 저녁) |
| [sheep_a.md](sheep_a.md) | sheep | 테스트 종 — 닭과 같은 추격·잡기 |

## 모델과 크기

- Skeletal Mesh, 종별 Skeleton 하나. 같은 종의 색/외형 변형은 동일 뼈·레스트 포즈를 공유한다.
- **피벗/root는 두 발 또는 네 발 사이 바닥 Z=0.** `CL9Monster::LoadData`는 메시를
  액터 중심에서 `-Radius` 내려 배치한다. 발 위치를 액터 중심 높이로 모델링하면 다시 내려가 버린다.
- 몬스터 메시에는 아이템식 Radius 자동 배율이 없다. 실제 cm 크기로 제작한다.
  몸 반경 `Radius`와 공격 거리 `CatchRadius`를 혼동하지 않는다. 날개·목 길이를 공격 판정으로 쓰지 않는다.
- 닭의 기획 참조 반경은 36cm([닭](../../plan/data/monster/chicken.md)). **높이는 미확정**이므로
  첫 블록아웃에서 정하고 카드에 적는다. 현실 닭 크기를 임의로 적용하지 않는다.
- 신규 독립 리그는 UE +X 정면/`MeshYawOffset=0`을 목표로 검수한다.
  기존 팩 리그를 재사용하면 그 축을 유지하고 실제 데이터의 Yaw 보정을 따른다.
- 깃털·털은 덩어리로, 눈·부리·귀·꼬리는 멀리서 읽히게 만든다. 어둠을 무력화하는 자체 발광 눈은 기본에 넣지 않는다.

## 리그와 동작

목/머리, 몸통, 좌우 다리, 필요한 날개/꼬리를 독립 제어한다. 새 리그는 표정에 필요한 만큼만
뼈를 추가한다. 종별 뼈 수는 카드에서 정하며 구매 리그와의 호환은 실제 검증한다.

**공격(Attack)** — 종당 **주 공격 하나**를 반드시 만든다(2026-09-21, 사용자 "몬스터 제작 목록에 공격 모션도 — 처형과 공격은 같은 모션"). 보통 잡는 종은
Catch(0.8초), 처형 종(`UCL9MonsterDef::IsExecutioner()` = `ExecuteRadius > 0 || bExecuteOnCatch` — 돼지·너구리)은 Execute(길이 = 행 `ExecuteDuration`)가
그 종의 공격 모션이다. 너구리의 격분 뒤 보통 잡기 `AN_Raccoon_Catch` 는 그대로 둔다(주 공격은 Execute).

| 클립 | 루프 | 용도·길이 규칙 |
|---|---|---|
| Idle | 예 | 대기·호흡. 발 고정 |
| Walk / Run | 예 | 배회·추격. in-place, 기준 속도 필수 |
| LookAround | 선택 | 조사/수색. 목과 시선 변화가 보임 |
| Catch | 아니오 | **잡는 종만**. 접촉 표현, 판정 트리거 금지. 길이 = 코드의 Catch 휴지 0.8초. 판정이 상태 진입 순간이라 예비동작 없이 첫 0.1초에 덮치고, 종별 2차 동작(쪼기·머리 흔들기·앞발) 뒤 돌아온다 (2026-09-15) |
| Execute | 아니오 | **처형 종만**(돼지·너구리, 2026-09-21). 길이 = 행 `ExecuteDuration`(초안 2.0 — `Tools/Blender/execute_anchor.py` `DURATION`), 죽는 순간 = 클립 끝. 첫 0.3초(`ExecuteCameraBlend`)는 피해자가 못 보므로 예비동작. **끝 자세 = 시체 옆에 선 자세**(Idle 첫 프레임 — `SpectateDelay` 까지 유지, Break 전환에 블렌드 없음). 2인 클립: 농부 짝 `AN_Farmer_Executed_<종>`(`animate_farmer_executed.py`)과 **각 blend 따로** 저작하되 **액터 공간 앵커 표**(`execute_anchor.py` — 원점 = 피해자 캡슐 중심 바닥, +X 피해자 전방, 몬스터 자리·요·접촉 시각·구간)를 공유한다. 돼지 = 정면 +100(잡힌 자리, 순간 이동 없음 — 앞 추력 +15cm 이내) 박치기 접촉 0.5, 너구리 = 등 뒤 −120(`ExecuteAnchorDistance`) 도약 접촉 0.6. 접촉 프레임의 코끝·앞발을 피해자 공간으로 환산한 `contact_probe_cm` 이 manifest 에 남는다 |
| HitReact | 아니오 | 전용 경직. 해당 행 `HitStunMs`에 맞춤. 맞는 순간 확 튀고(첫 0.05초) 나머지 동안 돌아온다 — sin 펄스 하나는 맞는 느낌이 없었다 (2026-09-15) |
| PushReact | 아니오 | 밀침(프라이팬 Push) 경직 = 행 `PushStun`(0.8초). 코드가 이 시간 동안 액터를 뒤로 미끄러뜨리므로 젖혀져 붕 떴다가(0~0.15) 발을 허우적거리며 미끄러지고(0.15~0.55) 휘청이며 선다. 5종 공통 (2026-09-15 신설) |
| PounceWindup | 아니오 | **닭만**(2026-09-20). 잡기 예고 = 행 `PounceWindup`(0.6초). 몸을 뒤로 젖히고 날개를 옆·위로 벌려 버티다가 마지막 0.17초에 웅크리며 Catch 첫 프레임으로 돌아와 끝난다(예고 끝을 Catch·헛침이 잇는다). 다른 종은 클립 없이 Catch 첫 프레임 유지 |
| WakeTelegraph | 아니오 | 쓰는 종만. 기상 예고 시간에 맞춤 |
| Death | 아니오 | 끝 포즈 유지. 래그돌은 별도 선택 |
| Sleep / AlertWindup / Alert | 종별 | 거위 등 실제 상태가 있는 종만 제작 |

모든 종에 모든 클립을 억지로 만들지 않는다. 거위는 경보 역할을 먼저 확인하며 닭 Catch를 복사하지 않는다.
Catch·HitReact·PushReact(·닭 PounceWindup·돼지/너구리 Execute) 는 `Tools/Blender/animate_creature_reacts.py --species <종> [--clips A,B]` 이 종 blend 를 열어
반응 클립만 다시 굽는다(원본 `animate_<종>_a.py` 는 로코모션·Death 의 정본으로 그대로; `--clips` 로 고르면 나머지 FBX 는 바이트 그대로다).
manifest `clips` 항목에 `reworked`·`sha256` 이 붙고, UE 재임포트는 `Tools/Unreal/reimport_creature_clips.py`(reworked 표시 클립만 replace,
`CL9_CLIPS` 로 골라서, 증거 JSON 은 `CL9_EVIDENCE` — 원본 임포트 스크립트는 sha 불일치를 assert 로 막는다). **동물 클립은 Interchange 로만 넣는다
(2026-09-21)** — 레거시 FBX 경로(`Interchange.FeatureFlags.Import.FBX 0` + `force_front_x_axis`)로 들어간 클립은 런타임에서 **요가 −90° 돌아** 있었다
(로코모션은 머리 +X, 반응 클립은 −Y — 09-15 5종·09-20 PounceWindup·09-21 Execute 전부, 리드가 Interchange 로 다시 넣어 정상). 기존 에셋을 덮을 땐
임시 폴더 임포트 → `consolidate_assets`(ABP 참조 교체) → 리디렉터 삭제 → rename(`reimport_creature_clips.py`).
미리보기 시트 `preview/react_{catch,hitreact,pushreact,pouncewindup,execute}_sheet.png`. UE 임포트 증거 [032-creature-react-clips.json](../evidence/032-creature-react-clips.json)
(2026-09-20 닭 PounceWindup 판 — 09-15 5종 Catch·HitReact·PushReact 판은 같은 폴더의 `.pre-pounce.bak`), Execute 는 [034-execute-clips.json](../evidence/034-execute-clips.json)
(2026-09-21 — `CL9_SPECIES=Pig,Raccoon CL9_CLIPS=Execute CL9_EVIDENCE=docs/asset/evidence/034-execute-clips.json`).
닭 리그의 날개 뼈는 좌우 롤이 달라 `wing`(로컬 X 회전) 한 값으로는 왼쪽이 위·뒤로, 오른쪽이 아래·앞으로 움직인다(기존 HitReact·PushReact·
WakeTelegraph 의 비대칭 날개짓이 이것). PounceWindup 은 `spread`(왼쪽 +X·오른쪽 +Z)로 대칭으로 벌린다 — 리그는 손대지 않았다(2026-09-20).
닭·돼지·양의 기존 팩 모션을 재사용할지, 새 모델에 리타깃할지 제작 카드에서 선택한다.

## 검수

발 접지·무릎·날개 접힘, 걷기/추격 속도별 발 미끄러짐, 방향 전환, 경직 후 복귀,
사망 끝 포즈를 확인한다. 게임 충돌은 몬스터 액터가 소유하며 Physics Asset이 곧 AI 판정은 아니다.
좁은 통로에서 시각 몸통이 벽을 크게 뚫지 않는지, 2인 화면에서 예고가 읽히는지 확인한다.
기준 속도를 기존 팩의 Walk/Run 값에서 무조건 복사하지 않는다.
