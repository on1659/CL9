# 파동 — `UCL9WaveTable`

소음원마다 "그 소리가 그리는 링"을 정하는 표다. 링의 반경은 이 표가 정하지 않는다 —
반경은 [rules.md](rules.md) 소리 컬럼과 몬스터 표에서 오고, 이 표는 **어느 컬럼에서 가져올지와
어떻게 보일지**만 갖는다. 왜 만드는지는 [../wave.md](../wave.md), 열의 타입·뜻의 정본은
[../../systems/wave.md](../../systems/wave.md) "행 스키마"다.

> **이 문서의 수치는 전부 초안이다 (2026-09-16).** 구현은 끝났다 — C++ `Source/CL9/Data/CL9WaveTable.*`·
> `Source/CL9/FX/CL9WaveRenderer.*`, 표 에셋 `Content/Data/WaveTable`, 나이아가라 `Content/Effects/NS_CL9_Wave`
> (2026-09-16, [../../systems/wave.md](../../systems/wave.md) 구현 상태). 파동은 판정에 닿지 않는 표현
> 값이라 확정은 **PIE에서 눈으로 보고** 한다. 행마다 "무엇을 보고 확정하나"를 아래
> [PIE 관찰](#pie-관찰--무엇을-보고-값을-확정하나) 절에 한 줄씩 적어 두었다.

**행은 열두 개, 열은 행 23개 + 전역 2개다.** 아래 네 표는 그 23열을 주제별로 나눠 실은
것이고 **열을 더하거나 이름을 바꾸지 않았다** — 네 표의 열을 합치면 스키마의 23열과 정확히
같다. (2026-09-20: `SoundRow` 열과 `monster_pounce` 행이 늘었다 — 아래 4 절.)

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
| `Niagara` | `TSoftObjectPtr<UNiagaraSystem>` | — | 아직 없는 에셋의 자리표시 |
| `bEnabled` | `bool` | — | 그 소음원이 파동을 내는가 |
| `Visibility` | `ECL9WaveVisibility` | — | `Always` / `NightOnly` / `BlackoutOnly` |
| `RadiusFrom` | `ECL9NoiseRadiusSource` | — | 반경을 가져올 판정 컬럼 |
| `FallbackRadius` | `float` | cm | 판정 반경이 없는 행만 |
| `RadiusScale` | `float` | 배율 | **1.0이 아니면 링이 거짓말이 된다** |
| `Speed` | `float` | cm/s | 링이 퍼지는 속도 |
| `Thickness` | `float` | cm | 띠 두께 |
| `Color` | `FLinearColor` | — | 기본 색 |
| `bUseInstigatorColor` | `bool` | — | 낸 사람의 슬롯 색으로 덮나 |
| `Power` | `float` | 배율 | 밝기 |
| `OwnerPowerScale` | `float` | 배율 | 자기 파동에만 곱하는 밝기 |
| `OcclusionRays` | `int32` | 개 | 생성 때 쏘는 수평 광선 수 |
| `TraceHeight` | `float` | cm | 광선 높이 |
| `bWallStamp` | `bool` | — | 벽 자국을 찍나 |
| `StampHeight` | `float` | cm | 자국 기둥 높이 |
| `StampDensity` | `float` | 배율 | 자국 한 자리의 점 수 |
| `StepDistance` | `float` | cm | 이 거리마다 발동 (0 = 원샷) |
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
| `walk` | **false** | `Always` | `None` | 228 | 1.0 | (228) | (0.45s) |
| `monster_prox` | true | `Always` | `None` | 330 | 1.0 | 330 | 0.65s |
| `monster_call` | true | `Always` | **`PackAlarm`** | 0 | 1.0 | 600 (닭·너구리) | 1.18s |
| `monster_pounce` | true | `Always` | `None` | 306 | 1.0 | 306 | 0.60s |

`RadiusFrom`이 가리키는 곳: `RunNoise`·`PickupQuiet`·`PickupLoud`·`PushNoise`·`Decoy`는
[rules.md](rules.md) 소리 표의 같은 이름 컬럼, **`PackAlarm`은 그 몬스터 행의
`PackAlarmRadius`**([monster/common.md](monster/common.md) — 닭·너구리 600)다.

---

## 2. 모양과 색 — 값은 전부 초안

`Color`는 `FLinearColor`의 RGB이고 A는 전부 1.0이다(밝기는 `Power`가 한다).

| 행 | `Niagara` | `Speed` | `Thickness` | `Color` (R,G,B) | `bUseInstigatorColor` | `Power` | `OwnerPowerScale` |
|---|---|---|---|---|---|---|---|
| `run` | `NS_CL9_Wave` | 510 | 12 | 0.55, 0.78, 0.95 | true | 1.0 | 1.7 |
| `pickup_quiet` | `NS_CL9_Wave` | 510 | 12 | 0.60, 0.90, 1.00 | true | 1.0 | 1.7 |
| `pickup_loud` | `NS_CL9_Wave` | 510 | 18 | 0.60, 0.90, 1.00 | true | 1.0 | 1.7 |
| `push` | `NS_CL9_Wave` | 510 | 18 | 0.90, 0.95, 1.00 | true | 1.0 | 1.7 |
| `decoy` | `NS_CL9_Wave` | 510 | 18 | 0.55, 0.95, 0.60 | false | 1.0 | 1.0 |
| `throw_break` | `NS_CL9_Wave` | 510 | 18 | 1.00, 0.45, 0.35 | false | 1.0 | 1.0 |
| `cart` | `NS_CL9_Wave` | 510 | 12 | 0.55, 0.78, 0.95 | true | 1.0 | 1.7 |
| `land` | `NS_CL9_Wave` | 510 | 18 | 1.00, 0.82, 0.55 | true | 1.0 | 1.7 |
| `walk` | `NS_CL9_Wave` | 510 | 12 | 0.42, 0.62, 0.78 | true | 0.6 | 1.7 |
| `monster_prox` | `NS_CL9_Wave` | 510 | 18 | 0.85, 0.25, 0.38 | false | 1.0 | 1.0 |
| `monster_call` | `NS_CL9_Wave` | 510 | 18 | 1.00, 0.65, 0.35 | false | 1.0 | 1.0 |
| `monster_pounce` | `NS_CL9_Wave` | 510 | 18 | 1.00, 0.15, 0.15 | false | 1.5 | 1.0 |

**`Niagara`는 전부 같은 에셋 `/Game/Effects/NS_CL9_Wave` 하나를 가리킨다** (2026-09-16 생성 —
이미터 하나 `Ring`, 벽 자국은 같은 이미터의 파티클 변형: [../../systems/wave.md](../../systems/wave.md)
"나이아가라 에셋의 실제 모양"). 행마다 다른 것은 사용자 파라미터뿐이므로 시스템을 행마다 만들
이유가 없다. 경로를 `/Game/Effects/` 아래로 잡은 것은 같은 성격의 에셋이 거기 있기 때문이다
([rules.md](rules.md) `HitFlashMaterial` = `/Game/Effects/M_CL9_HitFlash`). **이 열이 빈 행은 조용히
아무것도 그리지 않는다** ([../../systems/wave.md](../../systems/wave.md) 재생 1단계).

---

## 3. 차폐와 벽 자국 — 값은 전부 초안

| 행 | `OcclusionRays` | (광선 간격) | `TraceHeight` | `bWallStamp` | `StampHeight` | `StampDensity` |
|---|---|---|---|---|---|---|
| `run` | 180 | 13.6 | 30 | true | 195 | 1.0 |
| `pickup_quiet` | 64 | 14.7 | 30 | **false** | 0 | 0 |
| `pickup_loud` | 180 | 13.6 | 30 | true | 195 | 1.0 |
| `push` | 120 | 13.4 | 30 | true | 195 | 1.0 |
| `decoy` | 180 | 11.5 | 30 | true | 195 | 1.0 |
| `throw_break` | 180 | 16.7 | 30 | true | 195 | 1.0 |
| `cart` | 180 | 13.6 | 30 | true | 195 | 1.0 |
| `land` | 180 | 13.2 | 30 | true | 195 | 1.0 |
| `walk` | 120 | 11.9 | 30 | **false** | 0 | 0 |
| `monster_prox` | 120 | 17.3 | 30 | **false** | 0 | 0 |
| `monster_call` | 540 | 7.0 | 30 | **false** | 0 | 0 |
| `monster_pounce` | 120 | 16.0 | 30 | **false** | 0 | 0 |

(광선 간격)은 열이 아니라 파생값이다 — `2π × 최종 반경 ÷ OcclusionRays` (cm). 이 값이 링의
성김을 정한다.

---

## 4. 언제 발동하나, 무슨 소리를 내나 — 값은 전부 초안

| 행 | `StepDistance` | `RepeatInterval` | `MinInterval` | `TriggerRadius` | `SoundRow` |
|---|---|---|---|---|---|
| `run` | 156 | 0 | 0.15 | 0 | None |
| `pickup_quiet` | 0 | 0 | 0.20 | 0 | None |
| `pickup_loud` | 0 | 0 | 0.20 | 0 | None |
| `push` | 0 | 0 | 0.30 | 0 | None |
| `decoy` | 0 | 0 | 0.50 | 0 | None |
| `throw_break` | 0 | 0 | 0.20 | 0 | None |
| `cart` | 156 | 0 | 0.15 | 0 | None |
| `land` | 0 | 0 | 0.30 | 0 | None |
| `walk` | 126 | 0 | 0.15 | 0 | None |
| `monster_prox` | 0 | 1.2 | 0.50 | 780 | **`MonsterNear`** |
| `monster_call` | 0 | 0 | 0.50 | 0 | None |
| `monster_pounce` | 0 | 0 | 0.50 | 0 | None — 소리는 파동 행이 아니라 몬스터 상태 전이(`MonsterPounce`)가 낸다 |

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

파티클 예산 대조: 32개 × 링 점 약 50개 ≈ 1,600점이고 벽 자국이 붙어도 수천 점이다. CCL8은
같은 그림을 `WAVE_POOL` 40,000점 풀로 돌렸으므로 계급이 맞는다. **다만 CCL8은 three.js
포인트 풀이고 CL9은 나이아가라라 비용 구조가 다르다 — 이 두 값은 PIE에서 프레임을 보고 다시
재야 하는 값이다.**

---

## 근거

### 모든 행에 공통인 값

- **`Visibility` = `Always`** — 사용자 결정(2026-09-16). CCL8은 파동을 정전 전용으로 묶어
  두었지만 CL9은 소음 반경을 배우는 일이 밤 내내 일어나야 한다([../wave.md](../wave.md) 규칙 3).
  열이 있으니 나중에 특정 행만 `BlackoutOnly`로 돌릴 수 있다.
- **`RadiusScale` = 1.0** — 1.0에서 옮기는 순간 링이 판정과 달라지고 로드 검사가 경고를 남긴다
  ([../wave.md](../wave.md) 규칙 1). **초안에서 1.0이 아닌 행은 하나도 없다.**
- **`Speed` = 510 (전 행 동일)** — CCL8 `WAVE_SPEED` 170u/s → 510cm/s. 행마다 다르게 두지
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
  `bUseInstigatorColor`가 슬롯 색으로 덮으므로, 이 색이 그대로 보이는 것은 1인 판과 슬롯 색이
  없을 때뿐이다.
- **`StampHeight` = 195** — CCL8 자국 기둥은 지면 +3u에서 +65u까지 뿌려졌다(`3+Math.random()*62`).
  65u → 195cm다. 사람 눈높이 156cm보다 조금 높아 벽이 "위로 솟은 면"으로 읽힌다.
- **`StampDensity` = 1.0** — CCL8은 자국 한 자리에 3~8점을 찍었다(`n = 3 + floor(s*4)`).
  그 점 수를 나이아가라 에셋의 기본으로 잡고 배율 1.0이 그 상태다 — 표의 이 열은 에셋 기본에
  대한 배율이지 절대 개수가 아니다.
- **`OcclusionRays`** — 목표는 **광선 간격 15cm 안팎**이다(그보다 벌어지면 링이 점선으로
  보인다). 계급은 넷뿐이다: 64 · 120 · 180 · 540. 180은 CCL8 `STEP_RAYS`, 540은 CCL8
  `WAVE_RAYS`를 그대로 가져온 것이고, 64·120은 반경이 작은 행에서 같은 간격을 더 싸게 얻으려고
  더했다. **자주 나는 행일수록 계급을 낮춘다** — 비용이 (인원 × 걸음 빈도 × 광선 수)이기
  때문이다([../../systems/wave.md](../../systems/wave.md) 벽 차폐).
- **벽 자국을 찍는 행과 안 찍는 행** — 자국은 지형을 공짜로 알려 주는 채널이라, **자기가
  소리를 낸 대가로만** 나와야 한다([../wave.md](../wave.md) 왜 만드나 3). 그래서 사람이 큰
  소리를 낸 행(`run`·`pickup_loud`·`push`·`decoy`·`throw_break`·`cart`·`land`)만 true다.
  몬스터 둘은 **내가 아무 소리도 안 내고 지형을 읽는 길**이 되므로 false이고, `walk`도 같은
  이유로 false다(켠다 해도 무소음 이동에 지도가 딸려 오면 안 된다). `pickup_quiet`은 반경
  150cm라 자국이 거의 자기 발밑에 찍혀 정보가 없어서 false다.

### 행마다

| 행 | 값의 근거 |
|---|---|
| `run` | 반경은 `RunNoiseRadius` 390. `StepDistance` 156은 CCL8 `stepAcc` 달리기 52u의 환산이고, 달리기 속도 654cm/s에서 걸음 간격 0.24초가 된다 — 발을 디딜 때마다 링이 하나씩 핀다. `MinInterval` 0.15는 그 0.24초의 63%라 정상 걸음을 절대 먹지 않는다(`leg_strength` 버프로 +60이 붙어 714cm/s가 되어도 간격은 0.22초다 — [buff.md](buff.md)) |
| `pickup_quiet` | `PickupQuietRadius` 150. 수명이 0.29초로 가장 짧아 "소리가 거의 안 났다"가 그림으로 온다. 광선 64개로도 간격이 14.7cm다 |
| `pickup_loud` | `PickupLoudRadius` 390 — 달리기와 같은 반경이라 링도 같은 크기다. 그것이 이 행의 교훈이다: **뛰다가 주우면 달린 것과 똑같이 들린다**(`RecentRunTime` 1.0초 안). 색은 줍기 계열로 묶고 두께만 18로 키워 조용한 줍기와 갈랐다 |
| `push` | `PushNoiseRadius` 255. 위치는 밀린 몬스터가 아니라 민 사람의 발밑이다([../../systems/wave.md](../../systems/wave.md) 호출 지점) — 그래서 사람 색을 쓴다. `MinInterval` 0.30은 밀치기 쿨다운을 문서에서 찾지 못해 잡은 안전값이다(미결 4) |
| `decoy` | `DecoyRadius` 330. 발원지가 놓인 물건이라 사람 색·자기 밝기를 쓰지 않는다. **색은 CCL8에 없던 신규다** — 청(사람의 이동)·적(위험)·호박(충격)과 겹치지 않는 자리가 녹색뿐이라 (0.55, 0.95, 0.60)으로 잡았다. 미끼를 쓰는 두 시스템(거위 잠듦·탄 요리)이 같은 링을 낸다 |
| `throw_break` | 판정 반경이 없어 `FallbackRadius`. CCL8 `VIS_BREAK` 160u → 480cm로, 초안 중 사람이 내는 가장 큰 링이다 — 던져 깨는 것이 가장 시끄러운 행동이라는 그림이 여기서 온다. **이 480cm는 아직 어떤 판정에도 없다**(미결 1). 금 가는 것(`Crack`)에는 안 붙는다 |
| `cart` | `FallbackRadius` 390은 CCL8이 수레 파동에 `VIS_RUN`을 그대로 쓴 것을 옮겼다(4780행). `StepDistance`도 `run`과 같은 156이고 위치만 수레다 — 수레를 끌면 걸음마다 수레 자리에서도 링이 하나 더 핀다. CL9의 수레 소음 판정이 생기면 반경은 그리로 옮긴다(미결 1) |
| `land` | CCL8 `VIS_LAND` 126u → 378cm. 착지 속도 문턱이 없으므로([rules.md](rules.md) 점프) 제자리 점프도 링을 낸다 — `MinInterval` 0.30이 연타를 막는다 |
| `walk` | **기본 꺼짐.** 걷기의 어그로는 0이라 그릴 반경이 없다([rules.md](rules.md) 소리). 켜는 경우를 위해 CCL8 `VIS_WALK` 76u → 228cm를 적어 두었지만, **이 값은 어떤 판정도 가리키지 않는 순수한 장식이다** — 켜려면 거위 `WakeRadius` 450 같은 판정 근거가 먼저 필요하다. `Power` 0.6과 벽 자국 false는 "켜더라도 달리기보다 확실히 약하고 지형을 알려 주지 않는다"를 지키려는 값이다. `StepDistance` 126은 CCL8 걷기 42u |
| `monster_prox` | `FallbackRadius` 330을 **닭의 시야 반경 330cm와 같게** 잡았다 — 링이 플레이어에게 닿는 거리면 어차피 이미 눈에 띄는 거리라, 링이 새로 알려 주는 것이 없다. `TriggerRadius` 780은 CCL8 `PROX_R` 260u의 환산이고 존 이격 1050cm보다 작다(CCL8이 같은 검사를 갖고 있었다). 780은 너구리 시야 390cm의 두 배라 **발각되기 한참 전에 "뭔가 가까이 있다"만** 온다. `RepeatInterval` 1.2는 CCL8의 거리별 케이던스 0.7~2.5초의 가운데 값이다(미결 2) |
| `monster_call` | **2026-09-16 사용자 결정으로 반경을 시각 전용 값에서 떼어 무리 경보 반경에 묶었다** — 울음 파동은 경보의 시각 표현이므로 `RadiusFrom` = `PackAlarm`, `FallbackRadius` 0이다. 닭·너구리 기준 600cm가 되고([monster/chicken.md](monster/chicken.md)), 종마다 그 종의 값을 따른다. 광선 540은 CCL8이 경보 파동(`VIS_ALARM` 210u)에 쓰던 계급 그대로다 — 드물게 나는 행이라 광선을 아낄 이유가 없고, 경보는 가장 중요한 정보다. `MinInterval` 0.50은 거위 `AlertCooldown` 2.5초([monster/goose.md](monster/goose.md))의 5분의 1이라 정상 울음을 먹지 않는다 |
| `monster_pounce` | 초안(balance 2026-09-20). 306 = `Speed` 510 × `PounceWindup` 0.6 — 링이 다 퍼지는 순간이 덮치는 순간. 최대 `PounceRadius` 150(너구리) 이상이라 피해자에게 닿는다. `MinInterval` 0.50은 같은 몬스터 재예고 최소 주기 1.2초(예고 0.6 + 휴지 0.6)의 절반 아래 |

---

## 로드 검사와의 대조

[../../systems/wave.md](../../systems/wave.md) 로드 검사 일곱 줄을 초안 값으로 직접 돌려 본 결과다.

| 검사 | 결과 |
|---|---|
| `Speed > 0` · `RadiusScale > 0` | 통과 — 전 행 510 · 1.0 |
| `RadiusFrom != None`인 행은 `FallbackRadius == 0` | 통과 — `run`·`pickup_quiet`·`pickup_loud`·`push`·`decoy`·`monster_call` 여섯 행 전부 0 |
| `RadiusFrom == None`인 행은 `FallbackRadius > 0` | 통과 — `throw_break` 480 · `cart` 390 · `land` 378 · `walk` 228 · `monster_prox` 330 · `monster_pounce` 306 |
| `monster_prox`: 최종 반경 < `TriggerRadius` | 통과 — 330 < 780 (여유 450cm) |
| `MaxConcurrent > 0` · `CullDistance > 0` | 통과 — 32 · 4000 |
| (경고) `RadiusScale != 1.0` | 경고 없음 — 전 행 1.0 |
| (경고) `bEnabled`인데 `Niagara`가 비었다 | 경고 없음이 정상 — `NS_CL9_Wave`가 2026-09-16에 생겨 전 행이 가리킨다. 이 경고가 남아 있으면 그 행의 `Niagara` 셀이 비어 있는 것이다 |

---

## PIE 관찰 — 무엇을 보고 값을 확정하나

표 에셋과 구현이 생긴 뒤, 행마다 아래를 보고 값을 고친다. 붙일 자리는
[../../testing/pie-scenarios.md](../../testing/pie-scenarios.md)의 새 절 `T. 파동`이다.

| 행 | 볼 것 |
|---|---|
| `run` | 발을 디디는 순간과 링이 피는 순간이 어긋나 보이지 않는가(`StepDistance` 156). 링 열 개가 겹쳐 화면이 하얘지지 않는가 |
| `pickup_quiet` | 0.29초가 너무 짧아 **주웠는데 아무것도 안 뜬 것처럼 보이지 않는가** — 그렇게 보이면 `Speed`를 이 행만 내릴 것이 아니라 두께·밝기를 올린다(속도는 전 행 공통이다) |
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
