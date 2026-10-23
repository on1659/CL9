# 참고 — 무섭게 만드는 장치의 실값: 리썰·R.E.P.O.의 공포 게이지·스팅어·돌아보기·등장 주기

> 2026-10-17. [combat-gap.md](combat-gap.md) 3절 "깜짝 놀랄 요소"가 장치의 **이름**만 적었던 것을, 설치본의
> 디컴파일 코드와 에셋 실값으로 채운 문서다. 두 게임 모두 Mono 빌드라 `Assembly-CSharp.dll` 디컴파일(ilspycmd)과
> 에셋 타입트리(UnityPy + TypeTreeGenerator, 백엔드 AssetsTools)로 읽었다. 코드 초기값과 에셋 실값이 다른 곳은 따로 표시했다.
> **여기서 결정하지 않는다.** 결정은 [../decisions.md](../decisions.md)와 `plan/data/*.md`의 몫이다.

## 결론 — 두 게임은 "무서움"을 숫자 하나로 들고 다닌다

- **리썰은 플레이어 쪽에 공포 게이지(`fearLevel` 0~1)가 있다.** 몬스터·시체·사건이 게이지를 **점프**시키거나 **시간당 올리고**,
  게이지는 초당 0.055 로 혼자 식는다. 심장 소리·긴장 음악 세 겹이 이 게이지 하나로 켜지고 꺼진다. 즉 "무섭다"가 **몬스터 코드가 아니라
  플레이어 코드의 상태**다 — 몬스터는 `JumpToFearLevel(0.9)` 한 줄만 부른다.
- **리포는 몬스터 쪽에 플래그 두 개와 디렉터 음악 세 개가 있다.** 종마다 `SightingStinger`(첫 목격 스팅어)·`EnemyNearMusic`(근접 음악)을
  켜고 끄고, 추격이 시작되는 순간(`ChaseBegin` 0.5초)에 글리치·놀람음·흔들림을 **누구를 쫓는지에 따라 세기를 바꿔** 튼다.
  근접 음악의 실값은 코드 기본값(15m/4m)이 아니라 **5m/1m** 다 — 바로 옆에 와야 들린다.
- **둘 다 "보고 있다"를 몬스터가 안다.** 리포는 배회 중 화면에 **10초** 잡히면 멈췄다 그 플레이어 쪽으로 방향을 틀고(60초 재사용 금지),
  리썰은 Coil-Head·Bracken 이 **플레이어의** 시야 검사로 멈추고 숨는다.
- **등장은 주기다.** 리포 30종 **전부** 스폰 20~40초 유지·디스폰 240~300초 후 재등장이고, 10분마다 재등장 간격이 20% 씩 짧아진다.
  리썰은 벤트 스폰 전 종마다 다른 길이(대부분 **15초**, Crawler 35초, Girl 1초)로 벤트에서 소리가 커진다.

CL9 에 대응하는 것: 공포 게이지 없음, 스팅어·근접 음악 없음, 추격 시작 신호 없음, 배회 중 돌아보기 없음, 등장은 판 시작 일괄 + 7e 시야 밖
한 마리씩(예고 없음). 있는 것은 `WakeTelegraph`·`MonsterPounce` 소리·`monster_prox` 파동·잡힐 때 시선 강제·너구리 스토킹.

---

## 1. 리썰 — 공포 게이지 `fearLevel`

### 1.1 구조

| 어디 | 무엇 | 근거 |
|---|---|---|
| `StartOfRound.fearLevel` (float, 로컬) | 게이지 본체. `fearLevelIncreasing` 이 그 프레임에 안 켜졌으면 **초당 0.055** 감소, 죽어 있으면 0.5 | `StartOfRound.cs:4792-4803` |
| `PlayerControllerB.JumpToFearLevel(target, onlyGoUp=true)` | 목표값으로 **점프**. 기본은 올라갈 때만(차이 0.05 미만이면 무시) | `PlayerControllerB.cs:6577-6586` |
| `PlayerControllerB.IncreaseFearLevelOverTime(mult, cap)` | 초당 `mult` 로 `cap` 까지 올린다 — 가까이 있는 동안 계속 차는 몬스터용 | `PlayerControllerB.cs:6564-6575` |
| `SoundManager.SetFearAudio()` | 게이지로 소리 세 겹을 켠다 (아래) | `SoundManager.cs:511-575` |
| `HUDManager` | 게이지 > 0.4 면 화면 필터(`insanityScreenFilter`) 가중치가 게이지를 따라간다 | `HUDManager.cs:1491-1497` |

### 1.2 소리 세 겹 — 게이지 문턱

| 소리 | 켜지는 구간 | 볼륨 | 비고 |
|---|---|---|---|
| `lowAction` | 0.1 < 게이지 < 0.67 | 게이지 + 0.2 | 낮은 긴장. 꺼질 때 피치를 0.87~1.1 로 다시 굴린다 |
| `highAction1` | 게이지 > 0.4 | 게이지 − 0.2 | 느리게 올라온다(보간 0.75/s) |
| `highAction2` | 게이지 > 0.7 | 게이지 | 빠르게(2/s) |
| `heartbeatSFX` | 항상 재생, 볼륨은 게이지 함수 | — | 코드 `SoundManager.cs:575` 이후 — 이번에 끝까지 읽지 않았다 |

세 겹이 **겹쳐 있어서** 게이지가 오를수록 소리가 쌓이고, 식으면 위부터 빠진다. 전부 로컬 플레이어의 게이지만 본다.

### 1.3 누가 얼마나 올리나 — `JumpToFearLevel` 호출 전수 (17 파일)

| 값 | 사건 | 근거 |
|---|---|---|
| **1.0** | Coil-Head 에게 죽는 순간(`SpringManAI.cs:605`), Crawler 공격(`:539`), Centipede 가 머리에 붙음(`:396·687`), Bush Wolf·Cadaver Bloom·Cave Dweller·Giant Kiwi·Lasso 의 공격 | 각 AI |
| 0.9 | Coil-Head 가 **4m 안에서 보일 때**(`SpringManAI.cs:279`), Ghost Girl 이 가까이 보일 때(`DressGirlAI.cs:426`), **시체를 가까이서 볼 때**(`DeadBodyInfo.cs:328`) | |
| 0.85 | Clay Surgeon | `ClaySurgeonAI.cs:570` |
| 0.7~0.75 | Forest Giant 가까이 보임(0.7), Giant Kiwi(0.7), Cave Dweller(0.7), Crawler 가 보임(0.75) | |
| 0.5~0.6 | Bracken 이 **5m 안에서 발각**(0.6, `FlowermanAI.cs:354`), Bracken 분노(0.5), Hoarding Bug 분노(0.5), Eyeless Dog(0.5), Puma(0.5), 시체 멀리서(0.55) | |
| 0.2~0.4 | Coil-Head 9m 안(0.4), Bracken 멀리서 발각(0.3), Forest Giant 멀리(0.4), Ghost Girl 멀리(0.2), Puma 멀리(0.2·0.08) | |

`IncreaseFearLevelOverTime` 은 Forest Giant 1.4/s, Bracken 0.8/s, Crawler 0.8/s(cap 0.5), Centipede 0.6/s(cap 0.9), Eyeless Dog 0.25~0.4/s, Hoarding Bug 0.4/s, Clay Surgeon 0.37/s(cap 0.25) 등 — **가까이 있는 시간**이 게이지가 된다.

읽을 점 세 가지.

1. **거리 두 단계가 표준 패턴이다.** 같은 몬스터가 가까우면 0.9, 멀면 0.4 — "봤다"와 "코앞이다"를 구분한다.
2. **몬스터가 아닌 것도 올린다.** 동료 시체(0.9 / 0.55)가 Coil-Head 와 같은 급이다.
3. **점프와 누적이 분리돼 있다.** 깜짝(점프)과 짓누름(누적)이 다른 함수다.

### 1.4 광기 `insanityLevel` — 혼자 있으면 찬다

`PlayerControllerB.SetPlayerSanityLevel()` (`PlayerControllerB.cs:6590-6625`): **17m 안에 다른 플레이어가 없고** 무전으로도 안 들리면
`insanitySpeedMultiplier` 가 실내 0.8 · 우주선 0.2 · 해 진 뒤 야외 0.5 · 낮 야외 0.3, **혼자 플레이 중이면 −2**(안 찬다). 누가 곁에 있으면 −3 으로 빠르게 식는다.
상한 `maxInsanityLevel` 50. **Ghost Girl 은 광기가 가장 높은 플레이어를 골라 그 사람에게만 보인다**(`DressGirlAI.ChoosePlayerToHaunt`, `:111-156` — 가중 추첨 뒤 그 플레이어로 소유권 이전).
"혼자 떨어진 사람이 더 무서운 일을 겪는다"가 숫자로 들어 있다.

### 1.5 보고 있으면 — Coil-Head·Bracken 의 실값

| | Coil-Head (`SpringManAI`) | Bracken (`FlowermanAI`) |
|---|---|---|
| 판정 주체 | **플레이어 쪽** `HasLineOfSightToPosition`, 네 명 순회. 몸 두 높이(0.25/1.6m 또는 0.6/3m) 중 하나라도 보이면 | 로컬 플레이어의 `HasLineOfSightToPosition(위치+0.5m, 30m)` |
| 보이면 | `agent.speed = 0`, 전환 최소 간격 **0.15초**(`stopAndGoMinimumInterval`) | 상태 0→1(회피), 소유권을 본 플레이어에게 이전, 속도 0 |
| 안 보이면 | `currentChaseSpeed` **14.5** 로 4.5/s 보간 가속 | 등 뒤로 접근. 회피 지속은 `timesFoundSneaking % 3` 에 따라 **11초 / 24초** |
| 계속 보면 | — | 분노 미터 +1.5/s(1.25초 응시까지), 미터가 넘치면 상태 2 — 속도 3 → 12 까지 1.2/s 로 오른다 |
| 공포 | 4m 안 0.9 · 9m 안 0.4 · 죽일 때 1.0 | 5m 안 발각 0.6 · 멀리 0.3 · 분노 0.5, 근접 누적 0.8/s |

둘 다 **보는 사람의 화면에서 판정**하려고 소유권을 넘긴다 — 호스트 판정 구조(CL9·리포)에서는 "내 화면에 보이는가"를 클라가 올려야 한다
([repo/repo-monsters.md](repo/repo-monsters.md) `EnemyOnScreen` — 리포가 그렇게 한다).

### 1.6 등장 예고 — 벤트 `timeToPlayAudio` (EnemyType 실값, `sharedassets1.assets`)

벤트가 차 있으면 스폰 시각까지 남은 시간이 `timeToPlayAudio` 아래로 내려온 순간부터 벤트 소리가 **0 → 1 로 선형 증가**하고 로우패스가 풀린다
(`EnemyVent.cs:109-128`). 어느 벤트인지는 알려 주지 않는다.

| 길이 | 종 |
|---|---|
| **15초** | 대부분 (Bracken·Hoarding Bug·Jester·Butler·Bunker Spider·Nutcracker 18 …) |
| 길다 | Crawler **35**, Blob 25, Lasso 22 |
| 짧다 | Stingray 12, Masked **10**, Coil-Head **10** |
| 거의 없다 | Ghost Girl **1** |

빠른 추격자(Coil-Head·Masked)일수록 예고가 짧고, Ghost Girl 은 예고 없이 온다 — 예고 길이가 종의 성격이다.

---

## 2. R.E.P.O. — 디렉터 음악 셋과 몬스터 플래그 둘

### 2.1 종별 플래그 (Enemy 컴포넌트 30개, `resources.assets`)

| `SightingStinger` | `EnemyNearMusic` | 종 수 |
|---|---|---|
| 켬 | 켬 | 12 |
| 끔 | 끔 | 11 |
| 켬 | 끔 | 5 |
| 끔 | 켬 | 2 |

종 이름과의 대응은 이번에 못 붙였다 — `Enemy` 컴포넌트 참조가 `EnemyParent` 에 직렬화돼 있지 않다. 30종 중 **17종이 첫 목격 스팅어**, **14종이 근접 음악**을 쓴다.
절반 가까이는 둘 다 끄고 조용히 온다 — 모든 몬스터에 같은 연출을 붙이지 않는다.

### 2.2 첫 목격 스팅어 `MusicEnemySighting` (`level0:3237`)

| 필드 | 실값 | 뜻 |
|---|---|---|
| `DistanceMax` | **12m** | 이 안에서 |
| 조건 | 화면에 보임(`OnScreenLocal`) ∧ 방금 텔레포트하지 않음 ∧ 스폰 상태 아님 | |
| `Cooldown` | **120초** | 한 번 울리면 2분 쉰다 |
| `ActiveTime` | 5초 | |
| 효과 | 배경음악 10초 중단 + 임팩트 셰이크 2/0.1초 + 셰이크 2/1초 + 무작위 클립 | `MusicEnemySighting.cs:50-60` |

### 2.3 근접 음악 `MusicEnemyNear` (`level0:2747`) — **코드 기본값과 다르다**

| 필드 | 코드 기본값 | **에셋 실값** |
|---|---|---|
| `MaxDistance` | 15 | **5m** |
| `MinDistance` | 4 | **1m** |
| `OnScreenDistance` | 10 | 10m |
| `FadeSpeed` | 1 | **0.2** |

화면에 보이면 10m 안에서 최대 볼륨, 안 보이면 **5m 안**에서만 거리 반비례로 올라오고 1m 에서 최대. 벽 너머(레이캐스트 막힘)는 같은 방이 아니면 0.
페이드 0.2/s 라 **천천히 붙고 천천히 빠진다**. [repo/repo-monsters.md](repo/repo-monsters.md) 연출 절의 15/4 는 코드 기본값이었다 — 실값은 이 표가 맞다.

### 2.4 추격 시작 `EnemyStateChaseBegin` (`resources.assets:1318566`)

| 필드 | 실값 |
|---|---|
| `StateTimeMin/Max` | **0.5 / 0.5초** — 멈춰 서서 타깃을 쳐다보는 시간 |
| `Stinger` | 켬 |

로컬 플레이어가 **타깃이거나·같은 방이거나·화면에 보이면** 임팩트 셰이크 5/0.25초 + 셰이크 3/0.5초 + 카메라 글리치 Short + `AudioScare.PlayImpact()`(배경음악 **20초** 중단).
아니면 거리 감쇠판(5~10m)만. 이 컴포넌트가 붙은 에셋은 1개 — 공통 `ChaseBegin` 상태를 쓰는 종만 해당하고, 자기 상태기계를 가진 종(Upscream 등)은 따로 한다.

### 2.5 배회 중 돌아보기·근접 회피 `EnemyStateRoaming` (`resources.assets:1229813`, 인스턴스 1개)

| 필드 | 실값 | 뜻 |
|---|---|---|
| `RoamingOnScreenTime` | **10초** | 누군가의 화면에 이만큼 잡혀 있으면 |
| `RoamingTurnWaitTime` | **2초** | 멈춰 섰다가 |
| — | — | 그 플레이어 **앞쪽 지점**(`GetLevelPointAhead`)으로 목적지를 바꾼다. 멀티에서는 보고 있는 플레이어 중 무작위 한 명 |
| `RoamingOnScreenCooldown` | **60초** | 다시 걸리기까지 |
| `PlayerNearDistance` / `PlayerNearTimeMax` | 15m / **20초** | 이 안에 20초 머물면 |
| `PlayerFarTimeMin/Max` | 80 / 160초 | 그동안 플레이어에게서 가장 먼 지점으로 간다 |
| `RoamingTeleportChance` | 20 | (단위 미확인 — 코드 미추적) |

"보고 있으면 이쪽으로 온다"와 "너무 오래 붙어 있으면 떠난다"가 **같은 컴포넌트**에 있다. 지능이 아니라 연출 규칙이다.

### 2.6 등장 주기 `EnemyParent` (30종 전부 같은 값)

| 필드 | 실값 |
|---|---|
| `SpawnedTimeMin/Max` | **20 / 40초** — 이 뒤 플레이어 20m 안이 아니면 사라진다 (스폰 직후 3~4분은 타이머 정지) |
| `DespawnedTimeMin/Max` | **240 / 300초** — 이 뒤 다시 나타난다 (죽은 상태면 ×3) |
| `EnemyDirector.despawnedDecreaseMinutes/Percent` | **10분 / 0.2** — 10분마다 재등장 간격 배율이 0.2 씩 준다 |

종별로 다르게 둘 수 있는 값인데 **30종 전부 같다** — 디자이너가 종이 아니라 판의 리듬으로 튜닝했다는 뜻이다. 20분 지나면 배율 0.6, 50분이면 0 → 즉시 재등장.

### 2.7 순수 놀람 `EnemyUpscream` — 피해 0 의 공격

`EnemyUpscream.cs` Attack 상태, 타깃이 로컬이면 매 프레임:

- `InputDisable(0.1)` — 입력 차단
- `CameraAim.AimTargetSet(몬스터 눈, 0.1s, 5)` — 시선 강제
- `CameraZoom.OverrideZoomSet(50, …)` — FOV 50 으로 줌
- 비네트 어두운 빨강 0.75
- 첫 프레임: **`physGrabber.ReleaseObject(-1)`** — 들고 있던 것을 놓친다 + `CameraGlitch.PlayLong()`

HP 는 깎지 않는다. 추출 게임에서 "들고 있던 것을 놓친다"가 곧 피해라서 성립한다. 대기 깨기(`idleBreakTimeMin/Max` 45~90초)는 코드 기본값.

### 2.8 시선 강제 `EnemyHeadController`

추격·느린 추격·침대 밑 보기 상태에서 **8m 안** 플레이어마다 `SemiFunc.PlayerEyesOverride(player, 몬스터 눈, 0.1s)` (`EnemyHeadController.cs:40-46`).
잡히는 순간이 아니라 **쫓기는 내내** 시선이 끌린다 — CL9 `HurtLookModifier` 는 잡힌 순간만이다.

---

## 3. CL9 와 겹쳐 보기

| 장치 | 리썰 | 리포 | CL9 지금 | 비용 (추정) |
|---|---|---|---|---|
| 공포 게이지 + 소리 겹 | `fearLevel` 0.055/s 감소, 소리 3겹 문턱 0.1/0.4/0.7 | (없음 — 디렉터 음악이 대신) | 없음 | 플레이어 로컬 float 하나 + 사운드 3개. 몬스터는 `Pounce`·`Chase` 전이에서 값만 던진다 |
| 첫 목격 스팅어 | — | 12m·화면·쿨 120초 | 없음 | 클라 로컬 — 이미 복제되는 위치·상태에서 파생 |
| 근접 음악 | 심장 소리(게이지) | 5m/1m, 벽 차폐 | 없음 (`monster_prox` 파동은 소나 사건) | 로컬 |
| 추격 시작 신호 | (종별 소리) | 0.5초 정지 + 글리치 + 놀람음 + 흔들림(타깃/같은 방/화면이면 풀) | 없음 — `Chase` 진입이 조용하다 | `Chase` 전이는 이미 복제된다 |
| 보면 돌아본다 | Coil-Head/Bracken (반대 방향) | 배회 10초 → 2초 멈춤 → 그쪽으로 | 없음 | "내 화면에 보인다"를 클라→호스트로 올리는 역방향 경로 필요 |
| 오래 붙어 있으면 떠난다 | — | 15m·20초 → 80~160초 이탈 | 없음 (`ChaseBudget` 45초는 추격만) | 호스트 로직 |
| 등장 주기 | 벤트 15초 예고(종별 1~35), 위치 비공개 | 20~40초 있다 240~300초 뒤 재등장, 10분마다 −20% | 판 시작 일괄 + 7e 시야 밖 한 마리씩 | 2026-10-04 "디스폰 없음" 결정의 번복 여부 |
| 혼자 있으면 더 무섭다 | 광기 17m, Ghost Girl 표적 | — | 없음 | 호스트에서 거리 계산 가능 |
| 피해 0 놀람 | — | Upscream — 시선·줌·입력·**짐 놓침** | 없음 | 종 하나 |
| 시선 강제 | — | 추격 중 8m 내내 | 잡힌 순간만 | `HurtLookModifier` 확장 |

가장 싼 다섯 개(공포 게이지·스팅어·근접 음악·추격 시작 신호·시선 강제 확장)는 **전부 로컬 표현 층**이고 새 RPC 가 필요 없다
([../systems/monster.md](../systems/monster.md) "타격의 표현"과 같은 틀). "보면 돌아본다"와 "등장 주기"는 판정·결정 층이라 별도다.

---

## 근거와 한계

- 디컴파일 캐시 `%LOCALAPPDATA%\CL9\UnityAnalysis\{lethal,repo}\decompiled\Assembly-CSharp` (2026-10-17 설치본, [README.md](README.md) 조사한 설치본과 같다).
  에셋은 `Tools/UnityAnalysis` 의 `find_assets`·`read_asset`, 백엔드 전부 AssetsTools.
- 리썰 `heartbeatSFX` 볼륨 식(`SoundManager.cs:575` 이후)과 `HasLineOfSightToPosition` 의 기본 인자는 끝까지 읽지 않았다.
- 리포 `Enemy` 플래그와 종 이름의 대응, `RoamingTeleportChance` 의 단위는 미확인. `EnemyStateRoaming`·`ChaseBegin` 은 에셋 인스턴스가 1개뿐이라
  "공통 상태를 쓰는 종"에만 해당한다 — 어느 종인지는 프리팹 계층을 따로 읽어야 한다.
- 플레이 실측·영상 비교는 하지 않았다.
