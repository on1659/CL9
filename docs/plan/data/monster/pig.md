# 돼지 — `pig`

**층: 동물(`Beast`) — 2026-10-13.** 이 행 하나가 그냥 돼지이자 돼지우리 주인(빈 것)이다 — 개체마다 광폭화 문턱을 굴린다
([common.md](common.md) "층"·"광폭화 게이지", [../../story.md](../../story.md) "위협의 세 층"). 같은 날 앞서 적었던 "그냥 돼지 별도 행"은
번복됐다. 잡으면 처형은 이 행의 값이라 그냥 돼지도 그렇다 — 스토리의 "탈을 쓴 채로도 죽이는 유일한 주인"이 그대로다.

**무겁고 느린 잡는 종 — 잡히면 무조건 처형(즉사) (2026-09-20 저녁 재정의, 전부 초안).** 2026-09-05에
몬스터 시스템 검증용으로 닭 값을 복사해 넣었던 자리에, 사용자 결정(2026-09-19/20 —
[../../../goal/execute-sequence-pig.md](../../../goal/execute-sequence-pig.md))으로 처형이 붙었고, 같은 날 저녁
결정으로 역할이 갈렸다: **돼지 = 잡히면 처형, 너구리 = 몰래 뒤에서 처형(스토커 —
[raccoon.md](raccoon.md))**. 돼지는 닭처럼 추격·잡기 예고·덮침으로 잡되, **잡는 순간이 곧 처형이다**
(`bExecuteOnCatch` true — 텔레포트 없이 그 자리에서 피해자 시점이 돼지 카메라로 넘어가고
`ExecuteDuration` 뒤 HP 와 무관하게 죽는다). 스토킹은 하지 않는다(스토킹 열 9개 = 0). 처형이 시작되면 구출은 없다.

돼지다움은 세 값에 있다 — **몸이 크고(`Radius` 45·HP 6~8), 느리고(`ChaseSpeed` 396 = 걷기, `TurnDrag` 0.6 —
미로에선 닭보다도 느리다), 대신 잡히면 죽는다.** "잡히면 즉사"라서 추격 속도를 걷기와 같게 뒀다: 돼지는
**걷는 사람을 못 따라잡는다** — 멈춘 사람(줍기·수레 싣기·조리), 막힌 사람(막다른 길·문 앞·다른 몬스터에
밀린 사람), 수레를 끄는 사람(297 < 396)을 잡는다. 아래 값은 전부 PIE C-처형-돼지 관찰 전 초안이다 —
값마다 근거를 한 줄씩 적었고, 관찰 뒤 조정 후보는 [../../economy.md](../../economy.md) "스토킹과 처형" 절에 있다.

컬럼 정의는 [common.md](common.md) (처형·스토킹 13열 + `bExecuteOnCatch` 는 common.md "Execute" 절 — gameplay/docs 가 기제를 적는다).

---

## 체력·이동

| 컬럼 | 값 | 근거 |
|---|---|---|
| `HpMin` / `HpMax` | 6 / 8 | 닭 3/4·너구리 4/5보다 많이 — 프라이팬(`HitDamage` 2) 3~4대, 고기(1) 6~8대. **`ExecuteDuration` 2.0초면 탭 0.347초 프라이팬 4~5타(8~10) ≥ 8 로 처형 중에 죽일 수 있다** — 그래도 "구출 없음"은 값이 아니라 D4 가 지킨다(처형 중엔 경직·넉백이 없고, 돼지가 처형 중 죽어도 피해자는 그대로 죽는다). 너구리(HP 4/5, raccoon.md)와 같은 이유로 HP 를 올려 막지 않는다. 큰 몸이 값으로도 무겁다 |
| `HitStunMs` | 300 | 닭과 동일 |
| `WanderSpeed` | 100 | 닭 126보다 느린 어슬렁 — Walk 클립 기준 126cm/s 의 0.8배 |
| `PatrolSpeed` | 160 | 닭 195 × 0.8 (배회와 같은 비율) |
| `ChaseSpeed` | **396** | **= 걷기 396 = 닭.** 잡히면 즉사라 추격이 걷는 사람을 좁히면 안 된다(480 이면 84cm/s 로 좁혀 걷는 사람이 무조건 죽는 몬스터가 된다). 직선에서 걷는 사람과 나란히, 코너에서(`TurnDrag` 0.6) 뒤처진다. 수레 끄는 사람(396 × 0.75 = 297)은 99cm/s 로 좁힌다 — 수레를 놓거나 달려야 한다. A1 지친 빈손 506.6 > 396 (여유 +110.6 = 닭과 같다). Run 클립 기준 396 → 배율 1.0. 옛 480(격분 돌진)은 스토커 시절 값 — 그 역할은 너구리로 갔다 |
| `RetreatSpeed` | 200 | Stalk 에서만 쓰는 값이라 돼지는 지금 미사용(`StalkRetreatRadius` 0 — A7 무관). 200 은 스토커 시절 값 그대로 — 돼지를 스토커로 되돌려 시험할 때 로드 검사에 걸리지 않게 둔다 |
| `TurnRate` | 220 | 닭 280보다 둔하다 — 무거움은 회전에서 낸다 |
| `TurnDrag` | 0.60 | 닭 0.45보다 코너 손해가 크다 — 최고 속도가 걷기와 같으니 코너마다 뒤처진다. 미로가 돼지에겐 더 강한 무기 |
| `OutdoorChaseMult` | 0.80 | 닭과 동일 — 밖에서는 317, 걷는 사람도 못 따라간다(무거운 돼지는 밖에서 더 느리다) |
| `Radius` | 45 | 에셋 구 반경 실측(pig_a 187×82×110cm) — 닭 36·너구리 40·거위 45 |
| `SeparationDist` | 110 | 닭 90 = 36 × 2.5 → 45 × 2.5 = 112 → 110 |

## 감각

| 컬럼 | 값 | 근거 |
|---|---|---|
| `SightIn` / `SightOut` / `SightDark` | 480 / 320 / 160 | 닭(330/219/111) × 1.45 — 큰 짐승은 멀리서 본다 (2026-10-17 닭이 600/400 으로 올라 이 배수 관계는 깨졌다 — 돼지는 결정 D 범위 밖, 밸런스 때 다시 본다). 추격이 걷기 속도라 멀리서 시작하는 추격은 위험이 아니라 예고다(거리 = 여유). A2 3924 > 480 × 1.3 = 624 |
| `HearRadius` | 270 | 닭 180 × 1.5 — 돼지는 눈보다 귀·코다 |
| `DetectNearRadius` / `DetectRateNear` / `DetectRateFar` / `DetectDecay` / `DetectGrace` | 120 / 2.5 / 0.5 / 0.4 / 0.5 | 닭과 동일 — 게이지 속도는 종 공통으로 둔다 |

## 추격 (닭과 같은 Chase·Pounce — Catch 가 곧 Execute)

| 컬럼 | 값 | 근거 |
|---|---|---|
| `ChaseBudget` / `ChaseLoseTime` / `ChaseBreakTime` / `ChaseCooldown` | 45 / 6.0 / 1.2 / 3.0 | 닭과 동일 |
| `HomeRange` / `DeaggroRadius` | 840 / 510 | 닭과 동일 |
| `CatchRadius` | 100 | 몸 반경 45 + 닭의 팔 길이(84 − 36 = 48) = 93 → 100 — 코끝이 중심 앞 94cm 까지 나온다. **이 거리 안 = 처형**(`bExecuteOnCatch`) |
| `CatchDamage` | 1 (무의미) | `bExecuteOnCatch` true 면 `HostCatch` 가 `OnDamage` 를 타지 않아 **읽히지 않는다** — 표기만. 1 로 두는 이유: `bExecuteOnCatch false` 롤백 때 보통 잡기(HP −1)로 돌아가기 위해 |
| `PounceRadius` | 146 | `CatchRadius` 100 + `PounceLunge` 110 − 64 (이상적 후퇴 거리 64cm — common.md 비율 규칙). 옛 166 은 `PounceLunge` 130 의 짝 |
| `PounceWindup` | **0.8** | 닭·너구리 0.6보다 길다 — 무거운 몸이 덮치기까지 느리고, **덮침 = 죽음이라 예고가 더 읽혀야 한다**(읽는 시간 +0.2초). 0(예고 없음)으로 두지 않는 이유: D2 "걸어서 물러나면 피한다"는 종 공통 약속이고, 돼지의 불공정은 즉사 하나로 족하다. 파동 `monster_pounce` 링(306 = 510 × 0.6)은 종 공통이라 돼지에겐 덮침 0.2초 전에 다 퍼진다 — 판정이 아니니 그대로 둔다 |
| `PounceLunge` | 110 | common.md 비율 규칙(덮침 = 추격 속도 0.28초 거리) — 396 × 0.28 = 111 → 110 = 닭. 옛 130 은 480 의 짝. 무거운 몸의 긴 덮침(130 유지)은 관찰 뒤 후보 — 그때도 `PounceRadius` 를 64 규칙으로 같이 올리면(166) 회피 산수는 같다 |
| `PounceMissCooldown` | 0.6 | 닭과 동일 |
| `SearchHops` / `SearchDwellTime` | 3 / 2.0 | 닭과 동일 |

회피 산수(A5): 예고 진입 146cm 에서 뒤로 걷기 146 + 396 × 0.8 = **463 > 210** 헛침(여유 +253 — 닭·너구리 174보다
크다, 예고가 0.2초 길어서다) · 0cm 여유 317 − 210 = **+107**(닭 44·너구리 24). 헛친 뒤 간격 ≈ 333cm 를
396 − 396 = 0cm/s 로 좁힌다 — **걸어서 물러나는 사람에겐 재예고가 없다**(닭과 같다). 수레 끄는 사람(297)은
146 + 297 × 0.8 = 384 > 210 으로 역시 헛치지만(여유 +174) 헛친 뒤 간격 274cm 를 99cm/s 로 146 까지 좁혀(1.3초 + 재장전 0.6) **약 2초마다 재예고** —
수레를 놓지 않는 한 예고를 계속 읽어야 하고, 한 번 멈추거나 막히면 죽는다. 잡히면 즉사이므로 A5 여유는
잡는 종 중 제일 두껍게 둔다.

## 각성과 경보

닭과 동일 — `WakeTelegraph` 0.8 · `WakeSightRadius` 900 · `WakeRetryTime` 2.0 · `PackAlarmRadius` 600 ·
`KillAlarmRadius` 2100 · `bAlertsOthers` false. 잠 컬럼 5종은 전부 0/false(`SleepTime` 0 — 잠들지 않는다 —
잠든 돼지 옆을 지나다 깨우는 놀이는 즉사 종엔 과하다). `MeshYawOffset` 은 에셋 절의 값 그대로.

## 처형 (잡기 = 처형 — `bExecuteOnCatch`; 스토킹 없음)

| 컬럼 | 값 | 근거 |
|---|---|---|
| `bExecuteOnCatch` | **true** | 즉시 잡기·덮침 성공 둘 다 `HostCatch` 맨 앞에서 `HostBeginExecute(Target, 돼지 위치, 텔레포트 없음)` — `OnDamage`·`Catch` 상태를 타지 않는다. **false 면 롤백**(보통 잡기 HP −1) |
| `ExecuteDuration` | **2.0** (s, 초안) | Run 2 처형 전용 2인 클립(art, 돼지·너구리 공통 초안 길이) — 카메라 블렌드 0.3 + 접촉 0.5~0.6초(피해자 화면 안에 들어와야 한다) + 넘어짐 0.6초(즉사답게 읽힌다) + 착지 뒤 정지 0.6초 이상("죽는 순간"이 시체 위에서 끊긴다). 1.2 로는 블렌드 뒤 0.9초 안에 접촉·낙하·정지를 다 못 넣는다. `AN_Pig_Catch`(0.8+유지 0.4) 는 Run 1 임시값이었다 — art 가 다른 길이로 구우면 그 값으로 맞춘다(리드가 재굽기 지시). 로드 검사 `bExecuteOnCatch ⇒ ExecuteDuration > 0` |
| `ExecuteCameraSocket` | **None** | 액터 루트(구 콜리전)에 붙인다 — 2026-09-20 밤 사용자 "돼지 뒷다리 사이밖에 안 보임"으로 `head` 본 소켓안(−60,0,125 / −38)을 폐기. 메시가 `MeshYawOffset` −90만큼 돌고 `Radius`만큼 내려가 있어 본 소켓 공간의 오프셋이 종마다 엉뚱한 축으로 갔다. 본 소켓(머리 흔들림)은 열이 남아 있어 나중에 되살릴 수 있다 |
| `ExecuteCameraOffset` | (X=-130, Y=0, Z=165) (cm) | **액터 공간** — X = 전방(타깃 쪽)·Z = 위. 돼지 중심 뒤 130·위 165, 등 위에서 어깨 너머로 피해자를 내려다보는 자리. 잡은 자리는 텔레포트가 없어 피해자와의 거리가 `CatchRadius` 100 이하로 제각각(0~100)이지만 자리 자체는 같다. 너구리와 같은 값(종 공통 눈금) |
| `ExecuteCameraRotation` | (P=-30, Y=0, R=0) (도) | 액터 공간 피치 −30. PIE 스크린샷 `.claude/runs/execute-species-swap/pie_pig_execute_camera.png` — 돼지 등 위에서 피해자 전신과 파동 링이 보인다 (2026-09-20 밤 확정) |
| `ExecuteRadius` | **0** (cm) | 스토킹 없음 — `GetHuntState()` 는 Chase. 0 이 아니면 돼지가 스토커가 된다(너구리 값을 넣으면 시험 가능) |
| `ExecuteBehindAngle` | 0 (도) | 스토킹 없음 — 읽히지 않는다 |
| `ExecuteUnseenTime` | 0 (s) | 동상 |
| `ExecuteAnchorDistance` | 0 (cm) | 동상 — 텔레포트가 없어 앵커도 없다(A6 대상 아님) |
| `StalkSpeed` | 0 (cm/s) | 동상 |
| `StalkGazeHalfAngle` | 0 (도) | 동상 |
| `StalkGazeRadius` | 0 (cm) | 동상 |
| `StalkStareAnger` | 0 (s) | 동상 — 돼지는 노려봐도 멈추지 않고 격분도 없다(처음부터 Chase) |
| `StalkRetreatRadius` | 0 (cm) | 동상 (A7 무관) |

`CL9Rules.ExecuteCameraBlend` 0.3 은 [../rules.md](../rules.md) "죽음과 회복" — 종 공통.

### 잡히기까지의 시간 (정지한 혼자 — 최악)

발견(게이지 채움 뒤 Chase 진입, `SightIn` 480) → 접근 (480 − 146)/396 = 0.84초 → 예고 `PounceWindup` 0.8초 →
덮침 110 → **잡힘 = Execute 진입 ≈ 1.6초** → 죽음 **+2.0초 ≈ 3.6초**(`ExecuteDuration`) → 관전 3.0초 뒤(`SpectateDelay`)
돼지 `Break`(최대 2.0 + 3.0 = 5.0초 카메라 유지). 예고 안에 뒤로 걸으면 헛치고(여유 +253), 걸어서 멀어지면 다시 못
좁힌다(396 = 396). `KillCooldown` 9초 동안 다음 처형은 없다(처형 틱의 `HostKill` 자리에서 재충전).

## 에셋 (자체 제작 — 2026-10-09 확인)

`Content/Monsters/PigA/` — 제작 기록은 [../../../asset/monster/pig_a.md](../../../asset/monster/pig_a.md), 원본은 `ArtSource/monster/`.

| 컬럼 | 에셋 |
|---|---|
| `SkeletalMesh` | `SK_Pig_A` |
| `Skeleton` | `SK_Pig_A_Skeleton` |
| `AnimBP` | `ABP_Pig_A` (`Content/Monsters/`) |

클립(폴더 실물): `AN_Pig_Idle` · `AN_Pig_Walk` · `AN_Pig_Run` · `AN_Pig_LookAround` · `AN_Pig_WakeTelegraph` · `AN_Pig_Catch` · `AN_Pig_Execute` · `AN_Pig_HitReact` · `AN_Pig_PushReact` · `AN_Pig_Death`.

Dreamscape Farmlands 팩의 `SK_Pig`·`A_Pig_*`는 쓰지 않는다 — 팩은 2026-10-04에 지웠다.

## 미결

- 위 값 전부 — PIE C-처형-돼지 관찰 전 초안. 관찰 항목: 걷기와 같은 추격(396)이 "느리지만 무서운" 으로 읽히는가 아니면
  "무해한" 으로 읽히는가(→ `ChaseSpeed` 420~440 후보, economy.md), 예고 0.8 이 즉사 앞에서 충분히 읽히는가, 잡힌 자리의
  카메라(텔레포트 없이 제각각인 거리·각도 — 천장·벽·피해자 몸에 박히는지).
- `ExecuteCameraOffset` 의 축은 `head` 본의 UE 임포트 축(+X = 본 방향, +Z = 위)을 가정했다 — 에디터에서 본 축이 다르면 같은 월드 자리(뒤 77·위 115)로 환산한다.
- `ChaseSpeed` 480 안(스토커 시절 값)은 한 셀로 되돌릴 수 있다 — 걷는 사람을 84cm/s 로 좁혀 4.2초마다 재예고하는 "끈질긴 즉사 종"이 된다. 사용자가 그쪽을 원하면 `PounceLunge` 130·`PounceRadius` 166 도 같이(비율 규칙).
- 어둠 속 시선·손전등은 돼지와 무관해졌다(스토킹 없음).
