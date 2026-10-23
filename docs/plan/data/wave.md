# 파동 — `UCL9WaveTable`

소음원마다 "그 소리가 그리는 링"을 정하는 표다. 링의 반경은 이 표가 정하지 않는다 —
반경은 [rules.md](rules.md) 소리 컬럼과 몬스터 표에서 오고, 이 표는 **어느 컬럼에서 가져올지와
어떻게 보일지**만 갖는다. 왜 만드는지는 [../wave.md](../wave.md), 열의 타입·뜻의 정본은
[../../systems/wave.md](../../systems/wave.md) "행 스키마"다.

> **이 문서의 수치는 전부 초안이다 (2026-09-16).** 구현은 끝났다 — C++ `Source/CL9/Data/CL9WaveTable.*`·
> `Source/CL9/FX/CL9WaveRenderer.*`, 표 에셋 `Content/Data/WaveTable`, 나이아가라 `Content/Effects/NS_CL9_WaveDots`
> (2026-10-15 CCL8 점 그림으로 다시 만듦 — [../../systems/wave.md](../../systems/wave.md) 점 그림). 파동은 판정에 닿지 않는 표현
> 값이라 확정은 **PIE에서 눈으로 보고** 한다. 행마다 "무엇을 보고 확정하나"를 아래
> [PIE 관찰](#pie-관찰--무엇을-보고-값을-확정하나) 절에 한 줄씩 적어 두었다.

**행은 열두 개, 열은 행 26개 + 전역 6개다**(2026-10-15 긴박도 행 열 다섯 `bUrgencyFrom*`·`UrgentColor`와 전역 `Ripple`·`Urgency`는
아래 "물결·긴박도" 절). 아래 네 표는 그 앞 21열을 주제별로 나눠 실은
것이고, 네 표의 열을 합치면 스키마의 21열과 정확히 같다. (2026-09-20: `SoundRow` 열과 `monster_pounce` 행이 늘었다 —
아래 4 절. 2026-10-15: 행의 `Niagara`·`Thickness` 열을 지우고 전역 `DotSystem`·`Dots`를 더했다 — 점은 모든 행이
같은 시스템 하나로 그리고, CCL8 점에는 띠 두께가 없다.)

---

## 단위 — CCL8 값은 3을 곱해 읽는다

1차 근거는 전신 프로젝트 CCL8(`chicken-heist.html`)의 파동 상수다. **CCL8의 1u는 CL9의 3cm**다
([README.md](README.md) 단위 — `AggroRunU 130` ↔ `RunNoiseRadius` 390cm가 그 환산의 증거다).
아래에서 `130u → 390cm`처럼 적은 것은 전부 이 환산이다. 시간·개수·배율에는 환산을 걸지 않는다.

---

## 열 (스키마 요약)

정본은 [../../systems/wave.md](../../systems/wave.md)다. 여기서는 값을 읽는 데 필요한 만큼만 옮긴다.

| 열 | 타입 | 단위 | 값을 정하는 기준 |
|---|---|---|---|
| `bEnabled` | `bool` | — | 그 소음원이 파동을 내는가 |
| `Visibility` | `ECL9WaveVisibility` | — | `Always` / `NightOnly` / `BlackoutOnly` |
| `RadiusFrom` | `ECL9NoiseRadiusSource` | — | 반경을 가져올 판정 컬럼 |
| `FallbackRadius` | `float` | cm | 판정 반경이 없는 행만 |
| `RadiusScale` | `float` | 배율 | **1.0이 아니면 링이 거짓말이 된다** |
| `Speed` | `float` | cm/s | 파면이 퍼지는 속도 |
| `Color` | `FLinearColor` | — | 기본 색 |
| `bUseInstigatorColor` | `bool` | — | 낸 사람의 슬롯 색으로 덮나 |
| `Power` | `float` | 배율 | 밝기 |
| `OwnerPowerScale` | `float` | 배율 | 자기 파동에만 곱하는 밝기 |
| `OcclusionRays` | `int32` | 개 | 생성 때 쏘는 수평 광선 수 — 바닥 점이 서는 각도이자 벽 자국 자리. 0이면 행이 꺼진다 |
| `TraceHeight` | `float` | cm | 광선 높이 |
| `bWallStamp` | `bool` | — | 벽 자국을 찍나 |
| `StampHeight` | `float` | cm | 자국 기둥 높이 |
| `StampDensity` | `float` | 배율 | 벽 자국 세로 점선의 촘촘함 — 점 간격 = 전역 `Dots.RingSpacing` ÷ 이 값 (1 = 바닥 바퀴살과 같은 간격) |
| `bUntilWall` · `UntilWallRadius` · `UntilWallCooldown` | `bool` · `float` · `float` | — · cm · 초 | 핑 — 벽까지 퍼짐·상한·재사용 대기 (아래 3-1) |
| `bPassThroughBodies` · `RideHeight` | `bool` · `float` | — · cm | 사람·몬스터·아이템 통과·타고 넘기, 오를 수 있는 높이 (아래 3-1) |
| `StepDistance` | `float` | cm | 이 거리마다 발동 (0 = 원샷). `StepsPerWave > 0` 이면 읽지 않는다 |
| `StepsPerWave` | `int32` | 걸음 | 발소리 몇 번에 한 번 — 발걸음 행만. 0 = `StepDistance` 누적 (2026-10-22) |
| `RepeatInterval` | `float` | s | 이 시간마다 발동 (0 = 반복 없음) |
| `MinInterval` | `float` | s | 폭주 안전판 |
| `TriggerRadius` | `float` | cm | 이 안에 플레이어가 있을 때만 (0 = 조건 없음) |
| `SoundRow` | `FName` | — | 이 소음이 나면 같이 낼 `SoundTable` 행 이름(`CL9SoundNames` — [../../systems/sound.md](../../systems/sound.md); None = 소리 없음). 링과 같은 사건·같은 박자로, 링이 안 그려져도(정전 아님·컬링 밖) 난다. 행이 꺼지면 사건 자체가 없어 소리도 없다 |

---

## 1. 반경과 표시 — 값은 전부 초안

`최종 반경`과 `수명`은 표의 열이 아니라 **위 값에서 나오는 파생값**이다
(최종 반경 = 출처 반경 × `RadiusScale`, 수명 = 최종 반경 ÷ `Speed`). 읽는 사람이 계산하지
않아도 되게 같이 적었다.

| 행 | `bEnabled` | `Visibility` | `RadiusFrom` | `FallbackRadius` | `RadiusScale` | (최종 반경) | (수명) |
|---|---|---|---|---|---|---|---|
| `run` | true | `Always` | `RunNoise` | 0 | 1.0 | 390 | 0.76s |
| `pickup_quiet` | true | `Always` | `PickupQuiet` | 0 | 1.0 | 150 | 0.29s |
| `pickup_loud` | true | `Always` | `PickupLoud` | 0 | 1.0 | 390 | 0.76s |
| `push` | true | `Always` | `PushNoise` | 0 | 1.0 | 255 | 0.50s |
| `decoy` | true | `Always` | `Decoy` | 0 | 1.0 | 330 | 0.65s |
| `throw_break` | true | `Always` | `None` | 480 | 1.0 | 480 | 0.94s |
| `cart` | true | `Always` | `None` | 390 | 1.0 | 390 | 0.76s |
| `land` | true | `Always` | `None` | 378 | 1.0 | 378 | 0.74s |
| `walk` | **true** (2026-10-17 결정 D) | `Always` | **`WalkNoise`** | 0 | 1.0 | 200 | 0.39s |
| `crouch` | true (2026-10-22) | `Always` | `None` | 90 | 1.0 | 90 | 0.18s |
| `swing` | true (2026-10-17 결정 C) | `Always` | `SwingNoise` | 0 | 1.0 | 300 | 0.59s |
| `hit` | true (2026-10-17 결정 C) | `Always` | `HitNoise` | 0 | 1.0 | 600 | 1.18s |
| `monster_prox` | true | `Always` | `None` | 330 | 1.0 | 330 | 0.65s |
| `monster_call` | true | `Always` | **`PackAlarm`** | 0 | 1.0 | 600 (닭·너구리) | 1.18s |
| `monster_pounce` | true | `Always` | `None` | 306 | 1.0 | 306 | 0.60s |

`RadiusFrom`이 가리키는 곳: `RunNoise`·`PickupQuiet`·`PickupLoud`·`PushNoise`·`Decoy`는
[rules.md](rules.md) 소리 표의 같은 이름 컬럼, **`PackAlarm`은 그 몬스터 행의
`PackAlarmRadius`**([monster/common.md](monster/common.md) — 닭·너구리 600)다.

---

## 2. 모양과 색 — 값은 전부 초안

`Color`는 `FLinearColor`의 RGB이고 A는 전부 1.0이다(밝기는 `Power`가 한다).

| 행 | `Speed` | `Color` (R,G,B) | `bUseInstigatorColor` | `Power` | `OwnerPowerScale` |
|---|---|---|---|---|---|
| `run` | 510 | 0.55, 0.78, 0.95 | true | 1.0 | **1.4** (2026-10-22, 1.7 에서) |
| `pickup_quiet` | 510 | 0.60, 0.90, 1.00 | true | 1.0 | 1.7 |
| `pickup_loud` | 510 | 0.60, 0.90, 1.00 | true | 1.0 | 1.7 |
| `push` | 510 | 0.90, 0.95, 1.00 | true | 1.0 | 1.7 |
| `decoy` | 510 | 0.55, 0.95, 0.60 | false | 1.0 | 1.0 |
| `throw_break` | 510 | 1.00, 0.45, 0.35 | false | 1.0 | 1.0 |
| `cart` | 510 | 0.55, 0.78, 0.95 | true | 1.0 | **1.4** (2026-10-22) |
| `swing` | 510 | 0.90, 0.95, 1.00 | true | 0.8 | 1.7 |
| `hit` | 510 | 1.00, 0.70, 0.45 | true | 1.0 | 1.7 |
| `land` | 510 | 1.00, 0.82, 0.55 | true | 1.0 | 1.7 |
| `walk` | 510 | 0.42, 0.62, 0.78 | true | **0.5** (2026-10-22, 0.6 에서) | **1.4** (2026-10-22) |
| `crouch` | 510 | 0.35, 0.50, 0.62 | true | 0.35 | 1.4 |
| `monster_prox` | 510 | 0.85, 0.25, 0.38 | false | 1.0 | 1.0 |
| `monster_call` | 510 | 1.00, 0.65, 0.35 | false | 1.0 | 1.0 |
| `monster_pounce` | 510 | 1.00, 0.15, 0.15 | false | 1.5 | 1.0 |

**점은 모든 행이 전역 `DotSystem` 하나(`/Game/Effects/NS_CL9_WaveDots`)로 그린다** (2026-10-15 — 행마다 다른 것은 색·밝기·
반경·광선뿐이고, 그건 C++이 점을 찍을 때 정한다). 경로를 `/Game/Effects/` 아래로 잡은 것은 같은 성격의 에셋이 거기 있기
때문이다([rules.md](rules.md) `HitFlashMaterial` = `/Game/Effects/M_CL9_HitFlash`). **`DotSystem`이 비면 판정·소리·로그는 그대로
돌고 아무것도 그리지 않는다** ([../../systems/wave.md](../../systems/wave.md) 재생 9단계).

---

## 3. 차폐와 벽 자국 — 값은 전부 초안

| 행 | `OcclusionRays` | (광선 간격) | `TraceHeight` | `bWallStamp` | `StampHeight` | `StampDensity` |
|---|---|---|---|---|---|---|
| `run` | 180 | 13.6 | 30 | true | 195 | **0** |
| `pickup_quiet` | 180 | 5.2 | 30 | **false** | 0 | 0 |
| `pickup_loud` | 180 | 13.6 | 30 | true | 195 | **0** |
| `push` | 180 | 8.9 | 30 | true | 195 | **0** |
| `decoy` | 180 | 11.5 | 30 | true | 195 | **0** |
| `throw_break` | 180 | 16.7 | 30 | true | 195 | **0** |
| `cart` | 180 | 13.6 | 30 | true | 195 | **0** |
| `land` | 180 | 13.2 | 30 | true | 195 | **0** |
| `walk` | 180 | 7.0 | 30 | **false** | 0 | 0 |
| `crouch` | 90 | 6.3 | 30 | **false** | 0 | 0 |
| `swing` | 180 | 10.5 | 30 | **false** | 0 | 0 |
| `hit` | 180 | 20.9 | 30 | true | 195 | **0** |
| `monster_prox` | 180 | 11.5 | 30 | **false** | 0 | 0 |
| `monster_call` | 540 | 7.0 | 30 | **false** | 0 | 0 |
| `monster_pounce` | 180 | 10.7 | 30 | **false** | 0 | 0 |

(광선 간격)은 열이 아니라 파생값이다 — `2π × 최종 반경 ÷ OcclusionRays` (cm), 반경 끝에서 잰 이웃 광선 사이다.
바닥 점은 광선 `Dots.StrideMine`/`StrideOther`개마다 하나씩 서므로 **바닥 바퀴살 수 = `OcclusionRays` ÷ 간격 수**다
(달리기 180 ÷ 3 = 내 파동 60줄, 남의 파동 45줄). 2026-10-15 CCL8 규칙(`spawnWave` — 반경 200u 이하 `STEP_RAYS` 180,
그 위 `WAVE_RAYS` 540)에 맞춰 64·120이던 행을 180으로 올렸다 — 64·120이면 바퀴살이 듬성듬성해 CCL8 그림이 안 선다.

### 3-1. 착지 핑 — 벽까지·통과·타고 넘기 (2026-10-16 사용자, [../../goal/wave-landing-ping.md](../../goal/wave-landing-ping.md))

| 행 | `bUntilWall` | `UntilWallRadius` | `UntilWallCooldown` | `bPassThroughBodies` | `RideHeight` |
|---|---|---|---|---|---|
| `land` | **true** | 1200 | 2.0 | **true** | 250 |
| 나머지 행 | false | 0 | 0 | false | 0 |

- **착지만** — 점프는 가끔 일부러 하는 행동이라 "뛰어서 주변을 한 번 비춰 본다"는 도구가 된다. 달리기까지 켜면 걸음마다(0.24초) 방
  전체가 드러나 파동이 손전등을 대신한다(사용자와 합의, 2026-10-16).
- **상한 12m** (사용자 선택) — 벽이 없는 쪽에서 멈추는 거리. 양계장 방·복도 두세 칸.
- **재사용 대기 2초** (사용자 선택, 표 값) — 착지 자체는 `MinInterval` 0.3초마다 나므로, 제자리 점프로 지도를 계속 띄우지 못하게 한다.
  대기 중 착지는 보통 링(378cm)이다.
- **몸 통과·타고 넘기** — 어둠 속 몬스터의 위치·실루엣이 드러나는 것은 의도다(사용자). 대신 소리가 멀리 가는 것은 조심할 점으로 남긴다 —
  착지에는 아직 청각 판정이 없어 몬스터가 이 소리를 듣지 않는다(몬스터 기획 몫).
- **`RideHeight` 250** — 사람(약 180cm)·소 탈 몬스터까지 넘고, 양계장 천장(그 위)에는 닿지 않는 높이.
- **실루엣 `bRevealBodies` = true (`land`·`run` — `run` 은 2026-10-22 사용자 "몬스터에 부딪히는 것도 예쁘게")**, 전역 `Reveal`: 재질 `M_CL9_WaveReveal` · `Spacing` 3.5cm · `DotRadius` 0.22 · `Push` 0.5cm ·
  `Brightness` 3.0 · `Hold` 0.6초 · `Fade` 0.8초 (처음 4 / 0.18 / 1.0 / 0.4 는 바퀴살 옆에서 너무 흐렸다) — 어둠 속 시험(2026-10-16)에서 바퀴살만으로는 사람이 기둥
  두어 개로 보여 더했다. 값은 영상을 보고 굳힌다. `run` 의 실루엣은 판정 반경(390) 안의 몸만이라 착지 핑처럼 방을 드러내지는 않는다 —
  달리는 사람 옆 3.9m 의 몬스터가 어둠 속에서 점무늬로 드러나는 것이 지나치면 이 칸만 끈다.
- **`Reveal.bDarkOnly` = true · `LitReach` 0.6 · `DirectionalLitIntensity` 2 lux (2026-10-23 사용자 "밝을 때는 아니고 어두울 때만", "몸에 빛이 안 닿을 때")** — 켜진
  점·스포트 조명(손전등·헤드램프 포함)이 감쇠 반경의 60% 안에서 가리는 것 없이 닿거나, 2 lux 이상의 해가 가리는 것 없이 닿는 몸은 실루엣이 뜨지 않는다.
  빛줄 실루엣 재질 값은 `LineWidth` 6cm · `RimPower` 3 · `RimAlpha` 0.6.
- **`Reveal.bRevealFacilities` = false (2026-10-22)** — 우리·조리대 같은 붙박이는 드러내지 않는다. 사용자가 "이런 물결 매우 별로"라고 보낸 그림(가로줄 셋 +
  세로 기둥)은 물결이 아니라 **우리의 가로대·기둥이 점무늬로 드러난 실루엣**이었다(촬영 wave_modes_1022e 에서 착지 핑이 우리를 넘을 때 재현). 사람·몬스터·
  달걀은 그대로 드러난다. 붙박이 실루엣이 필요해지면 이 칸만 켠다.

### 모양 벌 `Styles` — 11개 (2026-10-22 사용자 "10개 정도 만들어서 골라 보겠다", 11번은 2026-10-23 실루엣 비교용, 값은 전부 초안)

디버그 패널 "파동" 탭·`cl9.wavestyle N` 으로 고른다([../../systems/wave.md](../../systems/wave.md) 모양 벌). 점 벌(1~4·10)은 전역 `Dots`·`Ripple` 을 복사한 뒤
아래 칸만 덮어쓴 것이고, 네온 띠·빛 테 공통값은 **관**: 메시 `/Engine/BasicShapes/Cylinder` · 재질 `M_CL9_WaveTube` · 굵기 3cm · 높이 4cm · 밝기 2.5 · 꼬리 0 · 끝에서 0.35초 옅어짐 · 벽 타기 켬(상한 200cm),
그리고 2026-10-23 다시 만든 띠의 지형 걷기 값 — 띠 광선 `Rays` 72 · 보폭 `MarchStep` 30cm · 오르는 턱 `StepUp` 30cm · 내려가는 턱 `StepDown` 45cm(더 깊으면 바닥 끝) ·
벽 찾는 높이 `ProbeHeight` 40cm · 얇은 것 `ThinWidth` 60cm(이보다 좁은 기둥·말뚝은 띠를 막지 않는다) · 끝 옅어짐 `EndTaper` 40cm · 벽 호 나눔 `WallSubdiv` 6 ·
이어진 면 `SurfaceGap` 광선 간격 2.5배 · 모서리 각 `CornerAngle` 35°, **테**: 재질 `M_CL9_WaveDecal` · 두께 35cm · 부드러움 0.35 · 밝기 2.0 · 끝에서 0.4초 · 투영 상하 150cm.

| # | 키 | 점 | 덮어쓴 값 | 관 | 테 |
|---|---|---|---|---|---|
| 1 | `Debug.Wave.Style1` 점 · 띠 (지금 값) | 켬 | — | — | — |
| 2 | `Style2` 점 — 띠만 | 켬 | `bSpokes` false | — | — |
| 3 | `Style3` 점 — 가는 선처럼 | 켬 | `bSpokes` false · `CrestDotGap` 2 · `CrestSize` 2.4 · `CrestSpacing` 30 · `CrestLife` 0.05 · `CrestBrightness` 3.5 — 점이 붙어 선으로 읽힌다 | — | — |
| 4 | `Style4` 점 — 큰 점 성글게 | 켬 | `bSpokes` false · `CrestDotGap` 28 · `CrestSize` 9 · `CrestSpacing` 60 · `CrestLife` 0.3 · `CrestBrightness` 2.0 | — | — |
| 5 | `Style5` 네온 띠 한 줄 | 끔 | — | 공통 | — |
| 6 | `Style6` 네온 띠 + 꼬리 셋 | 끔 | — | `Trails` 3 · `TrailGap` 45 · `TrailFalloff` 0.45 | — |
| 7 | `Style7` 네온 띠 굵게 · 천천히 | 끔 | — | `Thickness` 7 · `Fade` 0.8 · `Brightness` 1.6 | — |
| 8 | `Style8` 빛 테 (바닥 데칼) | 끔 | — | — | 공통 |
| 9 | `Style9` 빛 테 + 네온 띠 | 끔 | — | `Thickness` 2.5 | `Width` 50 · `Brightness` 1.2 |
| 10 | `Style10` 네온 띠 + 옅은 바퀴살 | 켬 | `Brightness` 0.12 · `Stride` 9/12 · `Ripple.bEnabled` false | 공통 | — |
| 11 | `Style11` 네온 띠 + 빛줄 실루엣 | 끔 | 6번과 같고 `RevealMaterial` = `M_CL9_WaveRevealLine` — 실루엣을 점무늬 대신 파면이 몸을 훑는 빛줄 + 남는 윤곽으로 (2026-10-23 사용자 "이건 봐야 알 듯") | 6번과 같음 | — |

**벌마다 색·속도 — 열 `bColor`·`Color`·`SpeedScale` (2026-10-23 사용자 "벌마다 색이나 스피드 같은 게 바뀌었으면", 값은 초안).** 색은 사람이 낸 소리 행
(`bUseInstigatorColor` 켜진 행 — 달리기·걷기·착지·줍기…)의 기본 색만 바꾼다. 몬스터·물건 행(경고 빨강·미끼 초록·깨짐 주황)은 뜻이 있는 색이라 그대로이고,
동료 슬롯 색·긴박도 색은 그 위에 얹힌다. `SpeedScale` 은 행 `Speed`(510)에 곱하는 배율이고, 반경은 그대로이며 그림만 바뀐다(실루엣 빛줄도 같은 속도).
1번(지금 값)·6번(평소 벌)은 표 그대로라 게임의 평소 모습은 바뀌지 않는다. 색을 넣은 네온 띠 벌(5·7·9·10·11)은 관 `Brightness` 를 절반(1.25, 7번은 0.8)으로
내렸다 — 2.5 면 색이 포화돼 흰색으로 보였다(촬영 `wave_styles_1023f`). 색은 한 채널을 0 가까이 둔 진한 색이어야 빛나도 색이 남는다.

| # | 색 (R,G,B) | `SpeedScale` |
|---|---|---|
| 1 | 표 행 색 | 1.0 |
| 2 | 민트 0.00, 1.00, 0.35 | 0.8 |
| 3 | 하늘 0.10, 0.45, 1.00 | 1.3 |
| 4 | 호박 1.00, 0.40, 0.00 | 0.7 |
| 5 | 자홍 1.00, 0.00, 0.60 | 1.0 |
| 6 | 표 행 색 | 1.0 |
| 7 | 주황 1.00, 0.15, 0.00 | 0.6 |
| 8 | 연두 0.25, 1.00, 0.00 | 1.2 |
| 9 | 보라 0.45, 0.00, 1.00 | 0.9 |
| 10 | 노랑 1.00, 0.80, 0.00 | 1.15 |
| 11 | 청록 0.00, 0.85, 0.75 | 0.85 |

**`DefaultStyle` = 5 (6번 "네온 띠 + 꼬리 셋") — 2026-10-22 사용자 "6번으로 가자".** 평소 파동은 이 벌로 그린다. 벌 목록은 남겨 둔다(패널에서 견주기). 전역
`Dots`·`Ripple` 은 `DefaultStyle` 이 -1 일 때와 `cl9.wavestyle -2` 때만 쓴다.

---

## 4. 언제 발동하나, 무슨 소리를 내나 — 값은 전부 초안

| 행 | `StepDistance` | `StepsPerWave` | `RepeatInterval` | `MinInterval` | `TriggerRadius` | `SoundRow` |
|---|---|---|---|---|---|---|
| `run` | 156 (읽지 않음) | **1** | 0 | **1.0** | 0 | None |
| `pickup_quiet` | 0 | 0 | 0 | 0.20 | 0 | None |
| `pickup_loud` | 0 | 0 | 0 | 0.20 | 0 | None |
| `push` | 0 | 0 | 0 | 0.30 | 0 | None |
| `decoy` | 0 | 0 | 0 | 0.50 | 0 | None |
| `throw_break` | 0 | 0 | 0 | 0.20 | 0 | None |
| `cart` | 156 (읽지 않음) | **1** | 0 | **1.0** | 0 | None |
| `land` | 0 | 0 | 0 | 0.30 | 0 | None |
| `walk` | 126 (읽지 않음) | **1** | 0 | **1.5** | 0 | None |
| `crouch` | 0 | **1** | 0 | **2.0** | 0 | None |
| `swing` | 0 | 0 | 0 | 0.20 | 0 | None — 소리는 `Swing` 행이 따로 난다 |
| `hit` | 0 | 0 | 0 | 0.20 | 0 | None — 소리는 `Hit`·`HitSurface` 행이 따로 난다 |
| `monster_prox` | 0 | 0 | 2.5 | 0.50 | 780 | **`MonsterNear`** |
| `monster_call` | 0 | 0 | 0 | 0.50 | 0 | None |
| `monster_pounce` | 0 | 0 | 0 | 0.50 | 0 | None — 소리는 파동 행이 아니라 몬스터 상태 전이(`MonsterPounce`)가 낸다 |

**`StepsPerWave` (2026-10-22 사용자 "항상 나가는 것보다 n에 한 번씩").** 발걸음 링은 이제 이동 거리가 아니라 **발소리 박자**로 난다 —
`TickFootstepSound` 의 걸음(달리기 3.3걸음/초·걷기 2.6걸음/초, [rules.md](rules.md) `FootstepStride*`)을 세어 n 번째마다 링 하나. 행이 바뀌면
(걷다가 달리기·앉기) 첫 발소리에 바로 하나 — "지금 달린다"가 그 걸음에 보인다. `StepDistance` 는 값만 남아 있고 `StepsPerWave` 가 0 인 행만 읽는다.

**초 단위 박자 = `StepsPerWave` 1 + `MinInterval` (2026-10-22 사용자 "걸을 때마다가 맞는지 모르겠고, 뛸 때 n초에 한 번씩만").** 걸음마다 사건은 나되
`MinInterval` 안의 사건은 렌더러가 버리므로, 링은 "`MinInterval` 이 지난 뒤 처음 발이 닿는 순간"에 핀다 — 초 단위 간격이면서 발소리와 박자가 맞는다.
달리기 1.0초(링 수명 0.76초라 하나가 사라진 뒤 다음이 핀다), 걷기 1.5초(옅은 띠 하나가 가끔), 앉아 걷기 2.0초(발밑 깜빡임). 걷기 링을 아예 없애려면
`walk` 행의 `bEnabled` 를 끈다 — 몬스터가 듣는 것(`WalkNoiseRadius`)은 그대로다. 점프는 `land` 행(착지 핑)이 따로 낸다.

`SoundRow`(2026-09-20)는 "이 소음이 나면 낼 소리"다 — `monster_prox`가 `MonsterNear`(짧은 꼬꼬댁)를
가리켜 근접 소나마다 링과 같은 박자로 다가오는 소리가 난다. 다른 행에 소리를 붙이고 싶으면 행만
채운다(코드 분기 없음). `monster_pounce`(`ECL9NoiseSource::MonsterPounce`)는 잡기 예고 진입에 몬스터
자리에서 한 번 나는 원샷 행이라 `RepeatInterval`·`TriggerRadius`는 0이고, 반경은 판정이 없으니
`FallbackRadius`다 — 네 표의 `monster_pounce` 값은 **초안(balance 2026-09-20)**이다.

---

## 전역 — 값은 전부 초안

| 열 | 값 | 근거 |
|---|---|---|
| `MaxConcurrent` | 32 | 최악의 순간을 세어 본 값이다. 넷이 동시에 달리면 사람당 살아 있는 링이 `수명 0.76s ÷ 걸음 간격 0.24s ≈ 3.2`개라 13개, 수레 둘이면 +6, 근처 몬스터 여덟이 소나를 돌리면 `0.65s ÷ 1.2s`라 순간 최대 8개, 원샷(줍기·착지·깨짐)이 겹쳐 4개 — 합쳐 31개다. 여기서 잘리기 시작하면 가장 오래된 링(거의 다 퍼진 링)부터 사라지므로 손실이 가장 덜 보인다 |
| `CullDistance` | 4000 | 존 이격 1050cm([README.md](README.md))의 약 네 배 — 같은 구역과 인접 구역의 파동은 보이고 맵 반대편은 만들지도 않는다. 동료의 위치를 읽는다는 목적([../wave.md](../wave.md) 왜 만드나 2)이 성립하는 거리는 "소리를 지르면 닿을 만한 거리"이고, 그보다 먼 링은 점이 너무 작아 정보가 되지 않는다 |
| `FloorProbeStep` | **80** (2026-10-22) | 바닥 끊김을 더듬는 간격 — 광선 180개 × 반경 390/80 ≈ 5표본 = 달리기 한 번에 짧은 아래 광선 900개. 계단 폭(≈100cm)보다 좁아 내려가는 계단 입구를 놓치지 않는다 (2026-10-22 사용자 "계단 밑에도 파동이 퍼진다") |
| `FloorDrop` | **60** (2026-10-22) | 발원지 바닥보다 이만큼 아래까지 바닥이 없으면 끊는다 — 한 계단(17cm)·문턱은 넘고, 내려가는 계단 네 단째(68cm)·구덩이·난간 밖은 끊긴다 |

| `DotSystem` | `/Game/Effects/NS_CL9_WaveDots` | 점을 그리기만 하는 시스템 하나 — 모든 행이 같이 쓴다(2026-10-15) |
| `Dots` | 아래 표 | 점 모양 — 모든 행이 같이 쓴다 |

### `Dots` — 점 모양 (2026-10-15, CCL8 `updWaves`·`dotMat` 환산)

| 필드 | 값 | 근거 |
|---|---|---|
| `PoolSize` | 49152 | 동시에 살아 있는 점 수 = 나이아가라 파티클 수. 혼자 달리면 한 걸음 파동이 (390 ÷ 3.5 ≈ 111바퀴 × 60점) ≈ 6,700점, 걸음 0.24초·수명 0.9초라 약 2.5만 점이 산다. 물결 마루가 파면당 약 1,400점(메아리마다 더)을 얹어 32768에서 올렸다(2026-10-15). CCL8은 40,000점 풀이었다 — 넷이 함께 달리면 넘쳐 가장 오래된(거의 다 옅어진) 점부터 덮인다. 2026-10-15 PIE: 0.24초마다 달리기 파동을 4초 내도(메아리 2) 프레임 16.75ms(60fps 상한)로 없을 때와 같았다 — **상한 아래 여유는 아직 재지 않았다** |
| `RingSpacing` | 3.5 cm | CCL8은 매 프레임 파면 자리에 점을 찍어 간격이 모니터 주사율에 묶였다(60Hz 8.5cm, 144Hz 3.5cm). 사용자 녹화(2026-10-15)는 바퀴살 점이 서로 붙어 이어진 선이라 144Hz 간격을 골랐다 — 8.5cm에서는 점이 낱알로 떨어져 "총알이 튀는" 것 같다는 지적. CL9은 프레임 대신 이 거리마다 찍어 프레임 수와 무관하게 같은 점선이 선다 |
| `bSpokes` | true (2026-10-22 새 열) | 바닥 바퀴살을 찍나 — 전역은 켜 둔다. 모양 벌 2~4("띠만"·"가는 선"·"큰 점")가 끈다 |
| `StrideMine` / `StrideOther` | **9 / 12** (2026-10-22, CCL8 3 / 4 에서) | 광선 몇 개마다 바닥 점 하나 — 바퀴살 60 → 20줄(남의 것 15줄). 2026-10-22 사용자 "너무 안 예쁘다": 촘촘하고 밝은 바퀴살과 고리가 겹쳐 그물로 읽혔다. 바퀴살은 옅은 바탕(차폐 그림자)으로 물러나고 밝은 앞띠(마루)가 주인공이다. 6/8 로 찍은 촬영(1022c)에서도 벽 앞의 바퀴살이 빗살처럼 띠만큼 밝아 더 줄였다 |
| `LifeMine` / `LifeOther` | **0.7 / 0.8 s** (2026-10-22, CCL8 0.9 / 1.1 에서) | 바닥 점 수명 — 꼬리를 짧게 |
| `SizeMine` / `SizeOther` | 2.2 / 1.9 cm | CCL8 `aSize` 3.1·2.7은 화면 픽셀 계수(`aSize × 300 ÷ 깊이` px)라 모니터가 클수록 얇아 보인다. 2026-10-15 사용자 "점이 너무 두껍다" — 5.0·4.4 → 3.2·2.8 → 2.2·1.9로 줄였다 |
| `FloorHeightMin` / `Max` | 2.4 / 6.6 cm | CCL8 `0.8 + rand × 1.4`u |
| `Brightness` | **0.12** (2026-10-22, CCL8 0.5 → 0.3 → 0.12) | 바퀴살 기본 밝기. 내 것은 × `OwnerPowerScale` 1.4(사람 행, 1.7 에서) = 0.17. 감쇠는 남의 것 직선 `1 − r/R`, 내 것 제곱근. 마루는 이 값 × `CrestBrightness` 3.0 — 바퀴살은 멀리서 `MinBrightness` 아래로 떨어져 아예 안 찍히고, 발밑 가까이서만 옅은 바탕으로 남는다 |
| `MinBrightness` | 0.02 | CCL8 `base > 0.02` |
| `FadeExponent` | **1.6** (2026-10-22, CCL8 0.9 에서) | 알파 `pow(1 − t, 1.6)` — 처음에 빠르게 옅어져 앞이 또렷하고 뒤가 부드러운 꼬리 |
| `StampLifeMin` / `Max` | **0.5 / 0.7 s** (2026-10-22, 0.5 / 0.5 에서) | 벽 자국 점 수명(점이 나타난 순간부터). 2026-10-16 사용자 "차오름 속도는 좋은데 좀 빨리 사라졌으면" → 2.0/2.5(CCL8 `2.0 + rand × 0.5`)에서 0.5. 점마다 조금 다르게 사라지라고 폭을 두었다 |
| `StampSize` | 2.3 cm | CCL8 `aSize` 3.2 — 바닥 점과 같은 비율로 줄였다 |
| `StampBaseHeight` | 9 cm | CCL8 자국 밑동 +3u. 꼭대기는 행의 `StampHeight`(195 = 65u)와 실제 벽 꼭대기 중 낮은 쪽 — CCL8 벽은 늘 그보다 높았지만 CL9 우리·울타리는 1.5m 안팎이라, 재지 않으면 벽 위 허공에 점이 흩날린다(2026-10-15) |
| `StampInset` | 1.5 cm | 벽면의 충돌점에서 **벽 법선 쪽**으로 띄우는 거리 — 점(2.3cm)이 벽에 묻히지 않을 만큼만. CCL8 은 `hd − 3`u(9cm)를 광선 방향으로 당겼지만 1인칭 3D 에서는 벽과 점 사이가 벌어져 커튼이 벽 앞 허공에 떠 보였다(2026-10-15 사용자 "벽에 붙어 올라가야 하는데 밖에 떠서 올라간다") |
| `StampJitter` | 0 cm | 2026-10-16 7.5 → 0: 벽 자국이 바닥 바퀴살을 이어 올린 곧은 세로 점선이 되었다(흩뿌리면 점 구름이 공중에 뜬 것처럼 읽혔다). 옛 값 7.5 = CCL8 `(rand − 0.5) × 2.5`u — 전체 폭. **벽 접선 방향만** 흩뿌린다(법선 방향으로 흩뿌리면 벽 앞 허공과 벽 속으로 갈라진다, 2026-10-15) |
| `StampRiseScale` | 1.0 | **2026-10-22 뜻이 바뀌었다 — 높이 배율.** 파동은 구라, 벽면의 높이 h 는 구의 반지름이 √(벽 거리² + h²) 에 닿을 때 켜진다(닿는 순간 빠르게 솟다가 소리 속도로 느려진다 — 물결이 벽에 튀는 모양, [../../systems/wave.md](../../systems/wave.md) 점 그림 벽). 1.0 = 물리 그대로, 2.0 = 같은 시간에 두 배 높이. 기둥 꼭대기도 구가 닿는 높이 √(반경² − 거리²)까지라 가까운 벽은 높이, 먼 벽은 밑동만. 옛 뜻(차오르는 속도 = `Speed` × 이 값, 2026-10-16 사용자 "밑에서부터 차근차근 올라와야 한다" → 1.0 "아직 느리다" → 1.5 → 2026-10-20 1.0)은 일정 속도였다. 0 이하면 기둥 전체가 한꺼번에 선다 |
| `StampOwnerScale` | 1.35 | CCL8 내 파동 자국 세기 배율 |
| `StampMinStrength` | 0.03 | CCL8 `s > 0.03` |

### 물결·긴박도 (2026-10-15 — [../../goal/wave-urgency-ripple.md](../../goal/wave-urgency-ripple.md), 구조는 [../../systems/wave.md](../../systems/wave.md) 물결·긴박도)

**행 열 (긴박도 입력)** — 몬스터 셋(`monster_prox`·`monster_call`·`monster_pounce`)은 `bUrgencyFromMonsterState`·`MonsterHp`·`Rage` 켬,
사람이 내는 일곱(`run`·`pickup_quiet`·`pickup_loud`·`push`·`cart`·`land`·`walk`)은 `bUrgencyFromCharacterHp` 켬, `decoy`·`throw_break`는
전부 끔(물건이 낸 소리라 누구의 긴박도도 아니다). `UrgentColor`는 사람 행 주황빛 빨강 (1.00, 0.42, 0.12), 그 밖은 진한 빨강
(1.00, 0.08, 0.05) — 처음 HSV 로 파랑 → 빨강을 섞었더니 중간이 분홍이 되어 다친 사람의 파동이 닭 소나(분홍)와 헷갈렸다.

| `Ripple` 필드 | 값 | 근거 |
|---|---|---|
| `bEnabled` | true | 사용자 "물결" 선택 |
| `CrestSpacing` | **10 cm** (2026-10-22, 8 에서) | 마루 고리 간격 — 띠 안에 고리 셋이 겹쳐 하나의 띠로 읽힌다 |
| `CrestDotGap` | **7 cm** (2026-10-22, 5 에서) | 둘레 점 간격 — 처음 광선 2개마다(90방향) 찍었더니 2.7m에서 점 사이 19cm라 띠가 아니라 바퀴살 한 줄 더로 보였다. 5cm 는 발밑에서 점 융단이 됐다(촬영 wave_modes_1022) |
| `CrestLife` | **0.07 s** (2026-10-22, 0.1 → 0.07) | × 510 cm/s ≈ 36 cm 띠 — 앞이 밝고 뒤로 옅어지는 꼬리. 같은 날 0.06 → 0.1 로 올렸다가 첫 촬영에서 51cm × 고리 6개가 발밑에서 두꺼운 융단으로 보여 0.07 로. 2026-10-15 속도를 510 → 340 으로 내리며 0.06 → 0.09 로 올렸다가, 2026-10-20 속도를 510 으로 되돌리며 0.06 으로 되돌렸다 |
| `CrestSize` | 3.2 cm | 바퀴살(2.2) 보다 커서 앞선 띠가 읽힌다 |
| `CrestBrightness` | **3.0** (2026-10-22, 1.8 에서) | 바퀴살 밝기(0.3) 대비 — 마루가 주인공, 바퀴살은 바탕 |
| `EchoGap` | 60 cm | 메아리 사이 |
| `EchoFalloff` | **0.5** (2026-10-22, 0.55 에서) | k 번째 메아리 = 0.5^k |

벽 마루(2026-10-22): 벽 자국을 찍는 행(`run`·`cart`·`land`·`pickup_loud`·`push`·`decoy`·`throw_break`·`hit`)은 같은 마루가 벽면에서 **호**를 그리며 오른다 —
`CrestDotGap`(5cm)으로 벽면을 나눈 자리마다 구가 닿는 높이에 점이 선다. 값은 따로 없다(마루 값·`StampRiseScale`·행 `StampHeight` 를 같이 쓴다).

| `Urgency` 필드 | 값 | 근거 |
|---|---|---|
| `StateUrgency` | Investigate 0.3 · Chase 0.7 · Stalk 0.6 · Pounce 1.0 · Catch 1.0 · Execute 1.0 · Return 0.2 · Break 0.2 · Stun 0.3 (나머지 0) | 사냥에 가까울수록 높게 — 초안 |
| `bColor` / `BrightnessMax` | true / **1.5** (2026-10-22, 1.6 에서) | 긴박도 1 에서 밝기 1.5배 |
| `bEchoCount` / `EchoMin` / `EchoMax` | true / **0** / **2** (2026-10-22, 1 / 3 에서) | 평소 메아리 없이 띠 하나, 긴박하면 2겹 — 평소 그림을 홀가분하게(2026-10-22 사용자 "주기·세기가 과하다") |
| `bJitter` / `JitterMax` | true / 3 cm | 6 cm 는 점이 흩어져 어지러웠다(PIE) |
| `bPulseRate` / `PulseRateMax` | true / 1.5 | 긴박도 1 이면 소나 간격 2.5 → 1.67초. 처음 2.0(1.2 → 0.6초)은 "너무 자주 나온다"(사용자 2026-10-15) |

**밝기 보정 — 재질 `M_CL9_WaveDot`의 `EmissiveScale` = 0.3 (2026-10-15).** CCL8은 점 색을 화면에 그대로 찍지만, UE는 같은 값이
밤 노출 고정(`ExposureLock`)·톤매핑을 거쳐 하얗게 탔다(내 달리기 점 255,255,255). 같은 장면에서 CCL8 점 화소 (96~114,
126~147, 148~171)에 맞춰 1.0·0.4·0.3·0.1을 재어 0.3(112,135,149)을 골랐다. 표 값(`Brightness` 등)은 CCL8 그대로 두고
화면 변환만 재질이 맞춘다.

파티클 예산 대조: 점은 고정 풀(`PoolSize`)에 쌓이므로 파티클 수는 판 내내 `PoolSize` 그대로다 — `MaxConcurrent`는 파면
수만 자르고 이미 떨군 점은 제 수명대로 옅어진다. **CCL8은 three.js 포인트 풀이고 CL9은 나이아가라 CPU 파티클이라 비용
구조가 다르다 — `PoolSize`는 PIE에서 프레임을 보고 다시 재야 하는 값이다.**

---

## 근거

### 모든 행에 공통인 값

- **`Visibility` = `Always`** — 사용자 결정(2026-09-16). CCL8은 파동을 정전 전용으로 묶어
  두었지만 CL9은 소음 반경을 배우는 일이 밤 내내 일어나야 한다([../wave.md](../wave.md) 규칙 3).
  열이 있으니 나중에 특정 행만 `BlackoutOnly`로 돌릴 수 있다.
- **`RadiusScale` = 1.0** — 1.0에서 옮기는 순간 링이 판정과 달라지고 로드 검사가 경고를 남긴다
  ([../wave.md](../wave.md) 규칙 1). **초안에서 1.0이 아닌 행은 하나도 없다.**
- **`Speed` = 510 (전 행 동일)** — 2026-10-20 플레이테스트 8번에서 되돌렸다: 착지 음파가 발밑에서 시작해 앞을 보면 0.8초 뒤에야 화면에 들어와 "바로 안 보인다"(사용자 "전체 속도 되돌리기", decisions.md 2026-10-20). 그 전: 사용자 2026-10-15 "퍼지는 것도 느리게"(질문 창, 예시 510 → 340). 처음 값은 CCL8 `WAVE_SPEED` 170u/s → 510cm/s 였다. 행마다 다르게 두지
  않은 이유가 있다: 속도가 다르면 같은 거리의 링이 서로 다른 시간에 도착해 **"링이 멈춘
  곳까지가 들리는 곳"을 거리감으로 읽는 일이 무너진다.** 소리의 속도는 하나다.
- **`TraceHeight` = 30 (전 행 동일)** — 바닥 슬래브 조각 `floor`의 높이가 30cm다
  ([segkind.md](segkind.md) 구현 상태). 발원점에서 그만큼 올려 쏘면 자기가 선 바닥면에 광선이
  스치지 않고, 물통 78cm·상자 120cm는 애초에 `CL9_Occlusion`에서 Ignore라 높이로 피할 필요가 없다.
- **`OwnerPowerScale` = 1.7 (사람이 내는 행)** — CCL8은 자기 파동의 기본 밝기가 0.85, 남의
  것이 0.5였다(`updWaves`). 0.85 ÷ 0.5 = 1.7이 그대로 이 값이다. 발원지가 자기 발밑이 아닌
  행(`decoy`·`throw_break`·몬스터 둘)은 1.0 — 물건이 있는 자리를 "내 소리"로 밝히면 내가 거기
  있는 것처럼 읽힌다.
- **`Color`** — CCL8 `WCOL` 팔레트를 그대로 옮겼다(달리기 `run` 밝은 청색, 걷기 `dim` 연한
  청색, 줍기 `pick`, 착지 `amber`, 깨짐 `red`, 밀치기 `white`, 울음 `cluck`, 소나 `prox`).
  **CCL8 값은 three.js에서 점 색으로 곱해 쓰던 수라 UE의 선형 색 공간에서 같은 톤으로 보인다는
  보장이 없다 — 연출 확정은 PIE 관찰 뒤다.** 사람이 내는 행은 어차피 멀티에서
  `bUseInstigatorColor`가 **남의** 파동을 슬롯 색으로 덮는다 — 내 파동은 늘 이 색이다(2026-10-15, CCL8 D16).
- **`StampHeight` = 195** — CCL8 자국 기둥은 지면 +3u에서 +65u까지 뿌려졌다(`3+Math.random()*62`).
  65u → 195cm다. 사람 눈높이 156cm보다 조금 높아 벽이 "위로 솟은 면"으로 읽힌다.
- **`StampDensity` = 0 (2026-10-22, 1.0 에서) — 세로 기둥을 끈다.** 벽에는 구가 닿는 **호**([../../systems/wave.md](../../systems/wave.md) 점 그림
  "벽 마루")만 남는다. 광선마다 세로 점선을 세운 커튼은 띠 여러 겹과 겹쳐 "빗살"로 읽혔다(2026-10-22 사용자 "이런 물결 매우 별로").
  `bWallStamp` 는 그대로 true 다 — 호를 세울 벽 자리(`BuildWallPoints`)가 이 플래그로 모인다. 기둥을 되살리려면 이 값을 1.0 으로.
  (역사: 1.0 은 바닥 바퀴살과 같은 간격 — 2026-10-16; 그 전에는 CCL8 처럼 3~8점을 높이 무작위로 뿌려 공중에 뜬 점 구름이었다.)
- **`OcclusionRays`** — 광선은 차폐만이 아니라 **바닥 점이 서는 각도**다(2026-10-15). 계급은 둘이다: 180은 CCL8
  `STEP_RAYS`(반경 200u = 600cm 이하), 540은 CCL8 `WAVE_RAYS`(그보다 큰 울음). 예전에 반경이 작은 행을 64·120으로
  깎았던 것은 링을 한 줄로 그리던 시절의 절약이었고, 점 그림에서는 바퀴살이 듬성듬성해져 CCL8 그림이 안 서서
  180으로 되돌렸다. 비용은 생성 때 한 번 (광선 수)회 트레이스다([../../systems/wave.md](../../systems/wave.md) 벽 차폐).
- **벽 자국을 찍는 행과 안 찍는 행** — 자국은 지형을 공짜로 알려 주는 채널이라, **자기가
  소리를 낸 대가로만** 나와야 한다([../wave.md](../wave.md) 왜 만드나 3). 그래서 사람이 큰
  소리를 낸 행(`run`·`pickup_loud`·`push`·`decoy`·`throw_break`·`cart`·`land`)만 true다.
  몬스터 둘은 **내가 아무 소리도 안 내고 지형을 읽는 길**이 되므로 false이고, `walk`도 같은
  이유로 false다(켠다 해도 무소음 이동에 지도가 딸려 오면 안 된다). `pickup_quiet`은 반경
  150cm라 자국이 거의 자기 발밑에 찍혀 정보가 없어서 false다.

### 행마다

| 행 | 값의 근거 |
|---|---|
| `run` | 반경은 `RunNoiseRadius` 390. **2026-10-22 — 1초에 한 번**: `StepsPerWave` 1 + `MinInterval` 1.0 이라 1초가 지난 뒤 처음 발이 닿을 때 링이 핀다(4절). 옛 `StepDistance` 156(CCL8 `stepAcc` 52u, 걸음 간격 0.24초)과 `MinInterval` 0.15 는 "걸음마다 링"이었고, 그 그물이 사용자가 "밤티"라 한 그림이다 |
| `pickup_quiet` | `PickupQuietRadius` 150. 수명이 0.29초로 가장 짧아 "소리가 거의 안 났다"가 그림으로 온다. 광선 64개로도 간격이 14.7cm다 |
| `pickup_loud` | `PickupLoudRadius` 390 — 달리기와 같은 반경이라 링도 같은 크기다. 그것이 이 행의 교훈이다: **뛰다가 주우면 달린 것과 똑같이 들린다**(`RecentRunTime` 1.0초 안). 색은 줍기 계열로 묶었고, 조용한 줍기와는 반경(150 대 390)과 벽 자국 유무로 갈린다(2026-10-15 띠 두께 열이 없어졌다) |
| `push` | `PushNoiseRadius` 255. 위치는 밀린 몬스터가 아니라 민 사람의 발밑이다([../../systems/wave.md](../../systems/wave.md) 호출 지점) — 그래서 사람 색을 쓴다. `MinInterval` 0.30은 밀치기 쿨다운을 문서에서 찾지 못해 잡은 안전값이다(미결 4) |
| `decoy` | `DecoyRadius` 330. 발원지가 놓인 물건이라 사람 색·자기 밝기를 쓰지 않는다. **색은 CCL8에 없던 신규다** — 청(사람의 이동)·적(위험)·호박(충격)과 겹치지 않는 자리가 녹색뿐이라 (0.55, 0.95, 0.60)으로 잡았다. 미끼를 쓰는 두 시스템(거위 잠듦·탄 요리)이 같은 링을 낸다 |
| `throw_break` | 판정 반경이 없어 `FallbackRadius`. CCL8 `VIS_BREAK` 160u → 480cm로, 초안 중 사람이 내는 가장 큰 링이다 — 던져 깨는 것이 가장 시끄러운 행동이라는 그림이 여기서 온다. **이 480cm는 아직 어떤 판정에도 없다**(미결 1). 금 가는 것(`Crack`)에는 안 붙는다 |
| `cart` | `FallbackRadius` 390은 CCL8이 수레 파동에 `VIS_RUN`을 그대로 쓴 것을 옮겼다(4780행). `StepDistance`도 `run`과 같은 156이고 위치만 수레다 — 수레를 끌면 걸음마다 수레 자리에서도 링이 하나 더 핀다. CL9의 수레 소음 판정이 생기면 반경은 그리로 옮긴다(미결 1) |
| `land` | CCL8 `VIS_LAND` 126u → 378cm. 착지 속도 문턱이 없으므로([rules.md](rules.md) 점프) 제자리 점프도 링을 낸다 — `MinInterval` 0.30이 연타를 막는다 |
| `walk` | **2026-10-17 결정 D 로 켰다** — 판정 근거가 생겼다: `WalkNoiseRadius` 200 안에서 몬스터가 걷는 사람을 듣는다([rules.md](rules.md) 소리). `RadiusFrom` 을 `WalkNoise` 로 묶고 옛 장식값 `FallbackRadius` 228 은 0 으로(로드 검사 — 출처가 있으면 폴백은 0). `Power` 0.5(2026-10-22, 0.6 에서)와 벽 자국 false — 달리기보다 확실히 약하고 지형을 알려 주지 않는다. `StepsPerWave` 1 + `MinInterval` 1.5 — 1.5초에 한 번 옅은 띠 하나(4절). `StepDistance` 126(CCL8 걷기 42u)은 이제 읽지 않는다 |
| `crouch` | **2026-10-22 사용자 — 뛰기·걷기·앉아 걷기 셋의 파동이 달라야 한다.** 앉아 걷기는 몬스터가 듣지 않으므로 판정 반경이 없고(`RadiusFrom` None) `FallbackRadius` 90 — 발밑 한 걸음 안, 수명 0.18초의 깜빡임이다. `Power` 0.35, 광선 90(바퀴살 22줄), 벽 자국·핑·실루엣 전부 끔(무소음 이동에 지도가 딸려 오면 안 된다). `StepsPerWave` 1 + `MinInterval` 2.0 — 2초에 한 번(4절). 색은 걷기보다 어두운 청회색. "링 = 들리는 범위" 계약의 예외라([../../systems/wave.md](../../systems/wave.md) 행 목록) 혼동되면 행을 끈다 |
| `swing` | **2026-10-17 결정 C.** `SwingNoiseRadius` 300, 휘두른 사람 발밑, 맞든 빗나가든. 색·속도는 `push` 를 따르고 `Power` 0.8 — 때리는 것이 조용하지 않다는 것만 보이면 된다. 벽 자국 없음(순간 소음) |
| `hit` | **2026-10-17 결정 C.** `HitNoiseRadius` 600 — 사람이 내는 가장 큰 링(던져 깨짐 480 보다 크다). 지형을 치면 닿은 자리에서, 몬스터·사람을 치면 친 사람 발밑에서. 주황빛(1.0, 0.7, 0.45)으로 달리기 청색과 가른다. 벽 자국 true — 큰 소리는 지형을 드러낸다 |
| `monster_prox` | `FallbackRadius` 330을 **닭의 시야 반경 330cm와 같게** 잡았다 — 링이 플레이어에게 닿는 거리면 어차피 이미 눈에 띄는 거리라, 링이 새로 알려 주는 것이 없다. `TriggerRadius` 780은 CCL8 `PROX_R` 260u의 환산이고 존 이격 1050cm보다 작다(CCL8이 같은 검사를 갖고 있었다). 780은 너구리 시야 390cm의 두 배라 **발각되기 한참 전에 "뭔가 가까이 있다"만** 온다. `RepeatInterval` 1.2는 CCL8의 거리별 케이던스 0.7~2.5초의 가운데 값이다(미결 2) |
| `monster_call` | **2026-09-16 사용자 결정으로 반경을 시각 전용 값에서 떼어 무리 경보 반경에 묶었다** — 울음 파동은 경보의 시각 표현이므로 `RadiusFrom` = `PackAlarm`, `FallbackRadius` 0이다. 닭·너구리 기준 600cm가 되고([monster/chicken.md](monster/chicken.md)), 종마다 그 종의 값을 따른다. 광선 540은 CCL8이 경보 파동(`VIS_ALARM` 210u)에 쓰던 계급 그대로다 — 드물게 나는 행이라 광선을 아낄 이유가 없고, 경보는 가장 중요한 정보다. `MinInterval` 0.50은 거위 `AlertCooldown` 2.5초([monster/goose.md](monster/goose.md))의 5분의 1이라 정상 울음을 먹지 않는다 |
| `monster_pounce` | 초안(balance 2026-09-20). 306 = `Speed` 510 × `PounceWindup` 0.6 — 링이 다 퍼지는 순간이 덮치는 순간. 최대 `PounceRadius` 150(너구리) 이상이라 피해자에게 닿는다. `MinInterval` 0.50은 같은 몬스터 재예고 최소 주기 1.2초(예고 0.6 + 휴지 0.6)의 절반 아래 |

---

## 로드 검사와의 대조

[../../systems/wave.md](../../systems/wave.md) 로드 검사 일곱 줄을 초안 값으로 직접 돌려 본 결과다.

| 검사 | 결과 |
|---|---|
| `Speed > 0` · `RadiusScale > 0` | 통과 — 전 행 510 · 1.0 |
| `RadiusFrom != None`인 행은 `FallbackRadius == 0` | 통과 — `run`·`pickup_quiet`·`pickup_loud`·`push`·`decoy`·`monster_call`·`walk`·`swing`·`hit` 아홉 행 전부 0 (2026-10-17 `walk` 228 → 0) |
| `RadiusFrom == None`인 행은 `FallbackRadius > 0` | 통과 — `throw_break` 480 · `cart` 390 · `land` 378 · `monster_prox` 330 · `monster_pounce` 306 |
| `monster_prox`: 최종 반경 < `TriggerRadius` | 통과 — 330 < 780 (여유 450cm) |
| `MaxConcurrent > 0` · `CullDistance > 0` | 통과 — 32 · 4000 |
| (경고) `RadiusScale != 1.0` | 경고 없음 — 전 행 1.0 |
| (경고) `no DotSystem` | 경고 없음이 정상 — 전역 `DotSystem`이 `NS_CL9_WaveDots`를 가리킨다(2026-10-15). 이 경고가 남아 있으면 그 칸이 비어 있는 것이다 |
| (오류) `OcclusionRays is 0` | 오류 없음이 정상 — 전 행이 180·540이다. 광선이 0인 행은 점이 설 각도가 없어 꺼진다 |

---

## PIE 관찰 — 무엇을 보고 값을 확정하나

표 에셋과 구현이 생긴 뒤, 행마다 아래를 보고 값을 고친다. 붙일 자리는
[../../testing/pie-scenarios.md](../../testing/pie-scenarios.md)의 새 절 `T. 파동`이다.

| 행 | 볼 것 |
|---|---|
| `run` | 발을 디디는 순간과 링이 피는 순간이 어긋나 보이지 않는가(`StepDistance` 156). 링 열 개가 겹쳐 화면이 하얘지지 않는가 |
| `pickup_quiet` | 0.29초가 너무 짧아 **주웠는데 아무것도 안 뜬 것처럼 보이지 않는가** — 그렇게 보이면 `Speed`를 이 행만 내릴 것이 아니라 밝기(`Power`)를 올린다(속도는 전 행 공통이다) |
| `pickup_loud` | 달리다 주운 링과 달리기 링이 **같은 크기로 보이는가.** 이것이 안 보이면 이 행의 존재 이유가 없다 |
| `push` | 링이 민 사람 발밑에서 피는가(밀린 몬스터 자리가 아니라). 연타로 링이 겹치면 `MinInterval`을 올린다 |
| `decoy` | 녹색이 달리기 청색·울음 호박색과 한눈에 갈리는가. 미끼를 놓은 자리와 사람이 있는 자리를 헷갈리지 않는가 |
| `throw_break` | 480cm 링이 화면을 덮지 않는가. 깨는 것이 가장 시끄럽다는 인상이 링 크기로 오는가 |
| `cart` | 사람 발밑 링과 수레 링이 **둘 다** 뜨는 것이 시끄러운지 — 겹쳐 보이면 `cart`를 끄거나 `StepDistance`를 키운다 |
| `land` | 제자리 점프를 반복할 때 링이 도배되지 않는가(`MinInterval` 0.30) |
| `walk` | 임시로 켜 봤을 때 달리기 링과 확실히 구별되는가. 구별이 안 되면 **켜지 않는 쪽이 옳다** |
| `monster_prox` | 링이 내 발밑까지 오지 않는가. 1.2초 고정 간격이 "다가온다"로 읽히는가, 아니면 CCL8의 심박 케이던스가 필요한가(미결 2) |
| `monster_call` | 경보 반경 600cm와 링이 멈추는 자리가 눈으로 같은가(치트 `cl9.shownoise`의 디버그 원과 대조). 광선 540이 프레임을 먹지 않는가 |
| `monster_pounce` | 링이 멎는 순간과 덮치는 순간이 같아 보이는가 — 어긋나면 `FallbackRadius`를 510 × `PounceWindup`으로 다시 맞춘다 |
| 전역 | 넷이 동시에 달리는 구간에서 프레임이 떨어지지 않는가(`MaxConcurrent` 32). 맵 반대편 링이 안 보이는 것이 답답하지 않은가(`CullDistance` 4000) |

---

## 미결

1. **판정 반경이 없는 네 행**(`throw_break` 480 · `cart` 390 · `land` 378 · `walk` 228)의
   수치는 지금 **이 표에만 있다.** 그 행동들의 청각 판정이 생기면 값을 [rules.md](rules.md)로
   옮기고 이 표는 `RadiusFrom`만 남긴다([../wave.md](../wave.md) 미결 2). 그때까지 이 넷은
   "링은 보이는데 몬스터는 못 듣는" 상태라 **링 = 들리는 범위 계약의 예외**다.
2. **`monster_prox`의 케이던스.** CCL8은 거리에 따라 0.7~2.5초로 빨라지는 심박이었는데
   `RepeatInterval` 열이 하나뿐이라 고정 1.2초로만 적을 수 있다. 심박을 되살리려면 열이 둘
   (가까울 때·멀 때) 또는 곡선이 필요하다 — **스키마 변경이라 docs·gameplay의 몫이다.**
3. **거위의 `PackAlarmRadius` 값이 미결이다** ([monster/goose.md](monster/goose.md) 표에 행은 있으나 값이
   "미결", 2026-09-21). `monster_call`의 반경이 이제 그 컬럼에서 오는데, 값이 없는 종은 링 크기를 알 수 없다.
   경보를 내는 유일한 종(`bAlertsOthers` true)이 거위라 이 값이 사실상 이 행의 주된 값이다.
4. **밀치기 쿨다운**이 표 문서에 없어 `push`의 `MinInterval` 0.30이 안전한지 확인하지 못했다.
   쿨다운이 0.3초보다 짧으면 이 값이 정상 밀치기를 먹는다.
5. **`Color`의 색 공간.** CCL8 값을 그대로 옮겼지만 three.js의 점 색과 UE 나이아가라의
   `FLinearColor`가 같은 톤으로 보이리라는 보장이 없다. PIE에서 보고 다시 잡는다.
6. **`MaxConcurrent` 32 · `CullDistance` 4000**은 CCL8의 파티클 풀에서 계급만 옮긴 값이다.
   나이아가라는 비용 구조가 달라 **프레임을 보고 다시 재야 한다.**

---

## 참고

- 기획: [../wave.md](../wave.md) · UE 설계·스키마 정본: [../../systems/wave.md](../../systems/wave.md)
- 판정 반경: [rules.md](rules.md) 소리 · 경보 반경: [monster/common.md](monster/common.md)
- 차폐 목록: [segkind.md](segkind.md) `bBlocksSight` · 슬롯 색: [character.md](character.md)
- 단위 환산: [README.md](README.md) · 부등식 목록: [../economy.md](../economy.md)
  (파동 열은 판정에 안 들어가 부등식 대상이 아니다)
- CCL8 원안: `chicken-heist.html`의 `spawnWave`·`updWaves`·`WCOL`·`VIS_*` 상수 (읽기 전용 참고)
