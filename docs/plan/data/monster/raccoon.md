# 너구리 — `raccoon`

세 번째 몬스터. **몰래 뒤에서 처형하는 스토커 (2026-09-20 저녁 사용자 결정 — 돼지 = 잡히면 처형, 너구리 =
스토커).** 보지 않는 사람의 등 뒤로 다가가 즉사시키는 리썰 컴퍼니 브레켄식 — 보면 멈추고(가까우면 물러나고),
오래 노려보면 격분해 보통 추격(잡기 예고·잡기)으로 넘어간다. 처형이 시작되면 구출은 없다. 사용자 확정값 셋:
**`StalkSpeed` 450(걷는 사람도 따라붙는다 — 달려야 뗀다) · `StalkStareAnger` 6(흘깃은 안전) · `SleepTime` 0(안 잔다)**.
나머지 처형·스토킹 열은 너구리 몸(`Radius` 40·`CatchRadius` 84·`ChaseSpeed` 462·`RetreatSpeed` 220)에 맞춘 초안 —
PIE C-처형-너구리 관찰 전. 공통 열 값은 3g 초안(2026-09-03) 그대로다.

컬럼 정의는 [common.md](common.md) (처형·스토킹 13열 + `bExecuteOnCatch` 는 common.md "Execute" 절).

---

## 공통 컬럼 값 (전부 초안 — 플레이해보고 조정)

| 컬럼 | 값 | 근거 |
|---|---|---|
| `HpMin` / `HpMax` | 4 / 5 | 닭(3~4)보다 한 대 더 버틴다. 처형 중엔 경직·넉백이 없고(D4) 너구리가 죽어도 피해자는 죽으니(D4) HP 는 "구출 없음"과 무관하다 — 돼지처럼 올리지 않는다 |
| `HitStunMs` | 300 | 닭과 동일 |
| `WanderSpeed` | 140 | 닭 126보다 약간 빠름 |
| `PatrolSpeed` | 210 | |
| `ChaseSpeed` | **462** | 격분 뒤 — 닭 396 < 너구리 462 < 지친 빈손 506 — **A1 부등식 유지** (잡는 종이라 필수). `StalkSpeed` 450 과 거의 같다 — 격분해도 크게 빨라지지 않고, 대신 잡기 예고·덮침이 생긴다 |
| `RetreatSpeed` | 220 | 시선에 잡혀 물러날 때 — `StalkRetreatRadius` > 0 이라 0 이면 로드 실패(A7) |
| `TurnRate` | 320 | 닭 280보다 민첩 |
| `TurnDrag` | 0.35 | 코너 손해가 닭(0.45)보다 작다 — 미로가 덜 통하는 종. 스토커에겐 맞는 몸: 등 뒤 지점까지 경로를 잘 따라온다 |
| `OutdoorChaseMult` | 0.9 | |
| `Radius` | 40 | 에셋 raccoon_a 211×69×104cm — A6 하한 40 + 39 = 79 |
| `SeparationDist` | 100 | |
| `SightIn` / `SightOut` / `SightDark` | 390 / 260 / 130 | 닭보다 눈이 좋다 — Stalk 는 여기서 시작한다. A2 3924 > 390 × 1.3 = 507 |
| `HearRadius` | 210 | |
| `DetectNearRadius` | 130 | |
| `DetectRateNear` / `Far` / `Decay` / `Grace` | 2.8 / 0.6 / 0.4 / 0.5 | |
| `ChaseBudget` | 30 | 닭 45보다 짧게 — 빠른 대신 끈기가 없다. 스토킹의 놓침 규칙도 이 값을 쓴다 |
| `ChaseLoseTime` | 5.0 | |
| `ChaseBreakTime` | 1.2 | |
| `ChaseCooldown` | 3.0 | |
| `HomeRange` | 1200 | 닭 840보다 넓게 돈다 |
| `DeaggroRadius` | 510 | |
| `CatchRadius` | 84 | 잡는 종 (격분 뒤) |
| `CatchDamage` | 1 | 격분 잡기는 보통 피해 — `bExecuteOnCatch` false. 너구리의 치명은 처형에만 있다 |
| `PounceRadius` | 150 | 닭 130 + 20 — 덮침이 20 길어진 만큼 예고도 멀리서 시작해 이상적 후퇴 거리 `CatchRadius + PounceLunge − PounceRadius` = 64cm(걷기 0.16초)를 닭과 같게 둔다 (2026-09-20 초안) |
| `PounceWindup` | 0.6 | 닭과 동일 — 예고 길이는 사람 반응 0.25 + 후퇴 0.16의 여유라 종에 붙는 값이 아니다 |
| `PounceLunge` | 130 | 닭 110 × (462/396) = 128 → 130 — 덮침 = 추격 속도 0.28초 거리(닭과 같은 비율). `WalkSpeed × PounceWindup` 238 > 84 + 130 = 214 라 0cm에서 예고해도 걸어서 헛침(여유 24 — 닭 44보다 얇다, PIE 관찰) |
| `PounceMissCooldown` | 0.6 | 닭과 동일 |
| `SearchHops` / `SearchDwellTime` | 3 / 2.0 | |
| 각성·경보 | 닭과 동일 (WakeTelegraph 0.8 · WakeSightRadius 900 · WakeRetryTime 2 · PackAlarmRadius 600 · KillAlarmRadius 2100 · bAlertsOthers false) | |
| 잠 컬럼 5종 | 전부 0/false — `SleepTime` **0** (사용자 확정 2026-09-20: 잠들지 않는 종) | 자는 스토커는 없다 — 밤 내내 돈다 |

**잡기 예고 회피 산수(A5, [../../economy.md](../../economy.md), 2026-09-20 초안)** — 예고 진입
150cm에서 뒤로 걷기 150 + 238 = **388 > 214** 헛침(여유 174 = 닭과 같다) · 옆걸음
√(150² + 238²) = **281 > 214** 헛침 · 제자리 잡힘. 닭과 다른 점은 헛친 뒤다: 간격 ≈ 258cm(388 − 130)를
462 − 396 = 66cm/s로 다시 좁혀 **걸어서 물러나는 사람에게도 약 1.6초마다 재예고**한다(닭은 못
따라붙는다). 걷기만으로는 잡히지 않되 계속 시달리고, 벗어나려면 달려야 한다 — A1 그대로.

## 처형과 스토킹 (Execute 13열 + `bExecuteOnCatch` — 2026-09-20 저녁, 확정 3 + 초안)

| 컬럼 | 값 | 근거 |
|---|---|---|
| `bExecuteOnCatch` | false | 격분 뒤 잡기는 보통 잡기(HP −1) — 즉사는 등 뒤 처형뿐. true 는 돼지([pig.md](pig.md)) |
| `ExecuteRadius` | 140 (cm) | 앵커 120 + 20 여유 — 등 뒤 지점에 도착하는 순간 거리 조건이 차서(경로 도착 오차 흡수) "도착 = 처형"이 된다. **0 이면 롤백**(옛 추격·예고·잡기 종) |
| `ExecuteBehindAngle` | 60 (도, 반각) | 등 뒤 120° 호. 180 − `StalkGazeHalfAngle` 55 = 125 > 60 — 피해자 시야 원뿔과 65° 떨어져, 등 뒤 원뿔 안의 너구리를 피해자 본인이 보고 있을 수는 없다(보고 있으면 이미 등 뒤가 아니다). 3/4 후방도 등 뒤로 친다 |
| `ExecuteUnseenTime` | 3.0 (s) | 복제 지연 0.1의 30배. 돌아본 순간부터 다시 3초가 차야 하니 반응 여유 = 3.0 − 0.1(복제) − 0.3(180° 회전) ≈ **2.6초**. 혼자일 땐 "3초에 한 번 돌아본다"가 생존 주기, 2인에선 동료가 너구리를 발견해 볼 시간. `StalkSpeed` 450 이 이미 걷는 사람을 잡게 했으니 시계까지 줄이지 않는다 |
| `ExecuteDuration` | **2.0** (s, 초안) | Run 2 처형 전용 2인 클립(art, 돼지·너구리 공통 초안 길이) — 카메라 블렌드 0.3 + 접촉 0.5~0.6초(피해자 화면 안에 들어와야 한다) + 넘어짐 0.6초(즉사답게 읽힌다) + 착지 뒤 정지 0.6초 이상("죽는 순간"이 시체 위에서 끊긴다). 1.2 로는 블렌드 뒤 0.9초 안에 접촉·낙하·정지를 다 못 넣는다. `AN_Raccoon_Catch`(0.8+유지 0.4) 는 Run 1 임시값이었다 — art 가 다른 길이로 구우면 그 값으로 맞춘다(리드가 재굽기 지시). 로드 검사 `ExecuteRadius > 0 ⇒ ExecuteDuration > 0` |
| `ExecuteAnchorDistance` | 120 (cm) | **≥ `Radius` 40 + 39 = 79 (A6 여유 41)**. 코끝이 중심 앞 85cm(`build_raccoon_a.py` Nose 0.792 + 0.058) → 기본 자세에서 캡슐 표면(120 − 39 = 81)에 4cm 들어가 등에 닿고, Catch 에서 몸이 10cm 앞으로 서며 앞발(중심 앞 ≈ 52 + 10 + 뻗기)이 캡슐 표면에 떨어진다 — 피해자 메시 등(캡슐 표면 안쪽 ≈ 20cm)엔 안 박힌다. 통로 255 에서 벽을 보고 선 사람 뒤엔 120 + 40 = 160 이 필요 — 돼지(185)보다 스윕이 덜 막힌다 |
| `ExecuteCameraSocket` | **None** | 액터 루트(구 콜리전)에 붙인다 — 2026-09-20 밤 돼지와 함께 `head` 본 소켓안(−75,0,110 / −28)을 폐기(본 소켓 공간은 메시의 `MeshYawOffset`·`Radius` 내림 때문에 액터 공간과 어긋나 종마다 엉뚱한 축으로 갔다). 본 소켓(머리 흔들림)은 열이 남아 있어 나중에 되살릴 수 있다 |
| `ExecuteCameraOffset` | (X=-130, Y=0, Z=165) (cm) | **액터 공간** — X = 전방(타깃 쪽)·Z = 위. 너구리 중심 뒤 130·위 165, 등 위에서 어깨 너머로 피해자(앵커 120cm 앞)를 내려다보는 자리. 돼지와 같은 값(종 공통 눈금) |
| `ExecuteCameraRotation` | (P=-30, Y=0, R=0) (도) | 액터 공간 피치 −30 (2026-09-20 밤 확정 — 돼지 PIE 스크린샷과 같은 셀). Catch 의 뒷발 서기로 카메라가 들리지 않는다 — 루트에 붙어 메시 애니와 무관 |
| `StalkSpeed` | **450** (cm/s, 사용자 확정) | **걷기 396 보다 빠르다** — 걷는 사람 등 뒤에도 54cm/s 로 붙는다(`SightIn` 390 → 앵커 120 까지 270cm = 걷는 타깃 5.0초·정지 타깃 0.6초). 달리기 654 로만 뗀다(204cm/s 벌어짐 — 브레켄 6 > 걷기 4.6 과 같은 관계). 지친 빈손 506.6 > 450 이라 달리기·걷기 순환으로도 벌어진다. Run 클립 기준 462 → 배율 0.97 |
| `StalkGazeHalfAngle` | 55 (도) | D5: 월드 FOV 90/2 + 10 여유 — 화면 가장자리에 걸친 너구리도 "본 것". 사람 쪽 상수라 종에 붙지 않는다 |
| `StalkGazeRadius` | 900 (cm) | `WakeSightRadius` 900 과 같은 눈금 — "이 안에서 보고 있으면 멈춘다". 어둠은 판정에 없다(범위 밖 — 관찰 뒤) |
| `StalkStareAnger` | **6** (s, 사용자 확정) | 흘깃(0.5~1초)은 안전, 붙잡아 두려 계속 봐도 6초까지는 서 있는다 — 노려보기가 동료가 지나갈 시간을 산다. 6초를 넘기면 격분(→ `ChaseSpeed` 462·예고·잡기). 안 볼 땐 같은 속도로 감쇠하니 번갈아 보면 무한정 묶인다(브레켄과 같다) |
| `StalkRetreatRadius` | 250 (cm) | `ExecuteRadius` 140 + 110 — 코앞에서 들키면 처형 반경 밖으로 물러난다(220cm/s 로 ≈ 0.5초). 다시 붙는 데 (250 − 120)/450 ≈ 0.3초 + 미발견 3초. 250 > `PounceRadius` 150 + 100 이라 물러난 자리에서 격분해도 예고 밖에서 추격이 시작된다(예고를 볼 수 있다) |

`CL9Rules.ExecuteCameraBlend` 0.3 은 [../rules.md](../rules.md) "죽음과 회복" — 종 공통.

### 처형까지의 시간 (혼자 — 최악)

- **정지한 혼자**: 발견(게이지 채움 뒤 Stalk 진입, 390) → 접근 (390 − 120)/450 = 0.6초 → 미발견 3.0초 충족 →
  **Execute 진입 3.0초** → 죽음 **5.0초**(`ExecuteDuration` 2.0) → 관전 3.0초 뒤(`SpectateDelay`) 너구리 `Break`
  (최대 2.0 + 3.0 = 5.0초 카메라 유지, 절대 시각 8.0초).
- **걷는 혼자(안 돌아봄)**: 270cm 를 54cm/s 로 좁혀 **5.0초에 등 뒤 도착 = 즉시 Execute**(미발견 시계는 이미
  찼다) → 죽음 7.0초. 돌아보면 3초 시계가 0 으로 돌아가고, 250 안이면 물러난다. **달리면**(654) 204cm/s 로
  벌어져 `DeaggroRadius` 510 밖으로 1초 남짓에 나간다.
- `KillCooldown` 9초 동안 다음 처형은 없다.

## 에셋

자체 제작 `Content/Monsters/RaccoonA/` — [../../../asset/monster/raccoon_a.md](../../../asset/monster/raccoon_a.md)
(2026-09-11 이후). 그 전엔 무료 Animal Variety Pack의 **Fox**를 임시 메시로 썼다 (2026-09-03 —
`Content/AnimalVarietyPack/Fox/`).

| 컬럼 | 에셋 |
|---|---|
| `SkeletalMesh` | `SK_Fox` |
| `Skeleton` | `SK_Fox_Skeleton` |
| `AnimBP` | `ABP_Raccoon_A` (Idle/Walk/Run 속도 블렌드 + Catch·HitReact·PushReact·WakeTelegraph·Death — Execute 가지는 `ABP_Pig_A` 와 같은 사양, 평가기 `AN_Raccoon_Catch` 마지막 프레임 유지. 2026-09-20 정정 — 옛 `ABP_Fox` 기록은 임시 메시 시절) |

## 미결

- 처형·스토킹 초안값(확정 3 제외) — PIE C-처형-너구리 관찰 전. 관찰 항목: 450 이 걷는 사람 뒤에서 읽히는가(발소리·파동 없이
  붙으면 예고 없는 즉사다 — `SightIn` 390 → 600 후보), 6초 응시가 방패로 과한가, 처형 성공률(2인에서 동료가 지켜보는 대응),
  카메라 자리(소켓 축·천장·뒷발 서기 들림).
- `ExecuteCameraOffset` 의 축은 `head` 본의 UE 임포트 축(+X = 본 방향, +Z = 위)을 가정했다 — 에디터에서 본 축이 다르면 같은 월드 자리(뒤 75·위 110)로 환산한다.
- `SkeletalMesh`/`Skeleton` 행은 Fox 기록 그대로다 — 실제 연결(`SK_Raccoon_A`/`SKEL_Raccoon_A`)로 정정은 docs 몫.
- 컨셉의 나머지(도둑 행동? 밀치기 반응?)·드롭 연결(`DropItems`) — 닭과 함께 미정.
