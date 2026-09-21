# 파동 시스템 (소음 반경 시각화)

> 소리가 난 자리에서 바닥 링이 퍼져 나가 **몬스터가 듣는 반경을 눈으로 보여 준다.**
> 호스트가 소음 사건을 한 곳으로 모아 방송하고, 각 머신이 같은 표를 읽어 똑같은 링을 그린다 —
> 판정은 여기 없다. 무엇을 왜 만드는지는 [../plan/wave.md](../plan/wave.md), 값은
> [../plan/data/wave.md](../plan/data/wave.md).

**구현 상태: 구현됨 (2026-09-16).** C++는 아래 설계 그대로다 — `ECL9NoiseSource`(CL9Types.h) ·
`UCL9WaveTable`(Data/CL9WaveTable.h) · `UCL9WaveRenderer`(FX/CL9WaveRenderer.h) ·
`ACL9GameState::HostReportNoise`/`Multicast_Noise` · `ACL9PlayerState::GetSlotColor`.
수치는 하나도 코드에 없다 — 전부 표의 열이고 값은 balance가 채운다.

나이아가라 에셋 `NS_CL9_Wave`(`/Game/Effects/`)는 리드가 같은 날 만들었다 — 사용자 파라미터
여덟, 차폐는 에셋 안의 CPU Collision, 자국은 같은 이미터의 파티클 변형(아래 "벽 차폐"·"벽 자국").

남은 것 셋: **미끼(`decoy`)는 발생 이벤트가 없다**(코드에 미끼를 놓는 자리가 아직 없다 —
`grep Decoy Source/` 0건), **낮은 조각의 `CL9_Occlusion` Ignore는 맵 코드 몫이라 아직 없다**
(지금은 모든 조각이 파동을 막는다), **`User.Distances`(배열 DI)는 보류** — C++ 진단 트레이스는
살아 있고(`cl9.shownoise`) 배열 DI가 생기면 다시 잇는다.

---

## 한눈에 보는 흐름

```
호스트 ── 소음이 나는 순간 (발걸음 · 줍기 · 밀치기 · 깨짐 · 착지 · 미끼 · 수레 · 몬스터)
  │
  └→ ACL9GameState::HostReportNoise(Source, Location, Radius, Instigator)
        · Instigator = 소리를 낸 액터 — 플레이어면 그 ACL9PlayerState, 몬스터면 그 ACL9Monster
        · Radius = 그 행동의 판정 반경 (없는 행동은 0)
        · 여기가 "소음 사건"의 단일 입구다. 몬스터 청각이 이 이벤트를 소비하도록
          통일하는 것은 별건 (plan/wave.md 미결 1) — 지금은 파동만 소비한다
        │
        └→ Multicast_Noise(Source, Location, Instigator)   // Unreliable, 반경은 안 싣는다
              ↓ 각 머신 (리슨 호스트 포함)
UCL9WaveRenderer (UWorldSubsystem — 로컬 표현 전용, 복제 없음)
  1. Instigator가 이 머신의 로컬 플레이어면 버린다 (이미 로컬에서 먼저 그렸다)
  2. 표에서 행을 찾는다 → bEnabled · MinInterval → (SoundRow가 있으면 소리) → Visibility · CullDistance 검사
  3. 반경 = (RadiusFrom이 가리키는 표 컬럼 또는 FallbackRadius) × RadiusScale
       — RadiusFrom이 CL9Rules 소리 컬럼이면 CL9Rules에서, PackAlarm이면
         Instigator 몬스터의 행에서 읽는다
  4. OcclusionRays개 수평 트레이스(CL9_Occlusion) → 광선별 최대 거리 배열
  5. Niagara 시스템 스폰 + 사용자 파라미터 주입 (반경·속도·두께·색·파워·거리 배열)

소유 클라 ── 자기가 낸 소음은 Server RPC를 올리는 그 순간 로컬에서 같은 함수를 먼저 부른다
             (예측 — 표현 전용이라 틀려도 링 하나가 헛돈다)
```

**반경을 네트워크에 싣지 않는 것이 이 설계의 핵심이다.** 모든 머신이 같은 `CL9Rules`·몬스터 표·
파동 표를 갖고 있으므로(같은 빌드·같은 에셋 — 접속 때 표를 대조하는 `CL9TuningHash`는 **설계만 있고
미구현**이다, [../conventions.md](../conventions.md) § 5 · plan/build-order.md), 반경은 각 머신이 파생하면 된다. 호스트가 임의의
반경을 실어 보낼 수 없으니 "링 = 판정 반경"이 구조적으로 보장되고, 발걸음마다 도는 RPC의
페이로드도 줄어든다.

---

## 소음원 enum

```cpp
UENUM()
enum class ECL9NoiseSource : uint8
{
    None,
    Run,           // 달리는 발걸음
    PickupQuiet,   // 걸으면서 줍기
    PickupLoud,    // 최근에 달렸으면(RecentRunTime) 줍기
    Push,          // 밀치기
    Decoy,         // 미끼가 놓임 (탄 요리 등)
    ThrowBreak,    // 던진 것이 깨짐
    Cart,          // 수레 밀기
    Land,          // 점프 착지
    Walk,          // 걷는 발걸음 (표에서 기본 꺼짐)
    MonsterProx,   // 몬스터 근접 소나 — 몬스터 자리에서 핀다
    MonsterCall,   // 몬스터 울음 (기상 예고·경보)
    MonsterPounce, // 잡기 예고 진입 — 몬스터 자리에서 한 번 (2026-09-20)
};
```

**표의 키가 이 enum이다** — `TMap<ECL9NoiseSource, FCL9WaveInfo>`. 이름을 FName으로 두지 않은
이유는 셋이다.

1. 발걸음마다 도는 RPC에 `FName`을 실으면 이름 문자열이 따라간다. enum은 1바이트다.
2. 소음원이 하나 늘면 **어차피 그 사건을 내는 코드에 호출 한 줄이 필요하다** — 표만으로 종류를
   늘릴 수 있는 종류의 데이터가 아니다. enum이 그 사실을 정직하게 드러낸다.
3. 에디터에서 키가 드롭다운이 되어 오타로 죽은 행이 생기지 않는다.

문서·표에서 부르는 이름(alias)은 enum 값의 소문자 스네이크 표기다 — `Run` ↔ `run`,
`PickupQuiet` ↔ `pickup_quiet`. [../plan/data/wave.md](../plan/data/wave.md)가 그 표기를 쓴다.

---

## 표 — `WaveTable`

### 어느 틀인가

**`UCL9ShakeTable`과 같은 틀이다** — `UDataAsset` 하나가 `TMap`으로 행을 품고,
`BP_CL9GameInstance`가 그 에셋을 물고 있어 C++는 에셋 경로를 한 글자도 갖지 않는다
([../conventions.md](../conventions.md) § 5).

```cpp
UCLASS()
class UCL9WaveTable : public UDataAsset
{
    GENERATED_BODY()
public:
    /** 소음원 → 그 소음원이 그리는 파동 */
    UPROPERTY(EditAnywhere, Category="Wave")
    TMap<ECL9NoiseSource, FCL9WaveInfo> Rows;

    /** 표 전역 상한 — 아래 "성능 상한" */
    UPROPERTY(EditAnywhere, Category="Wave")
    int32 MaxConcurrent = 0;

    UPROPERTY(EditAnywhere, Category="Wave")
    float CullDistance = 0.f;
};
```

`UCL9DefBase` 파생(+`CL9DefList`)을 쓰지 않은 이유: 파동 행에는 `DisplayName`·`Description`·
`Icon`·`Mesh`·`ActorClass`·`Gimmicks`가 전부 무의미하다. 화면에 이름이 뜨지 않고, 월드 메시도
전용 BP도 없으며, `SetAlias`→`LoadData` 경로를 타는 액터 행이 아니다. 상속 컬럼 일곱 중 여섯이
빈칸으로 남는 표는 `UCL9DefBase`가 아니라 셰이크 표 쪽이다 — 셰이크도 같은 이유로 그 틀을
골랐고(표현 전용·GameInstance 주입), 둘을 같은 모양으로 두면 읽는 사람이 같은 것으로 읽는다.

### 행 스키마 (balance가 값을 채운다)

키는 `ECL9NoiseSource`. 아래가 행 구조체 `FCL9WaveInfo`의 전부다.

| 열 | 타입 | 단위 | 뜻 | 값 |
|---|---|---|---|---|
| `Niagara` | `TSoftObjectPtr<UNiagaraSystem>` | — | 이 파동이 재생할 나이아가라 시스템. **소프트 참조** — 비어 있으면 그 행은 아무것도 그리지 않는다. `UCL9GameInstance::Init` 이 미리 올려 붙들고, `UCL9WaveRenderer::OnWorldBeginPlay` 가 판 시작 때 한 번 켰다 끈다(2026-09-21 — 첫 링에서 동기 로드 + 첫 활성화 컴파일로 ≈500ms 스톨, [performance.md](performance.md)) | balance (에셋은 fx) |
| `bEnabled` | `bool` | — | 이 소음원이 파동을 내는가. 구현은 전부 되어 있고 이 체크박스가 켜고 끈다 | balance |
| `Visibility` | `ECL9WaveVisibility` | — | `Always` / `NightOnly` / `BlackoutOnly` — 기본 `Always` | balance |
| `RadiusFrom` | `ECL9NoiseRadiusSource` | — | 반경을 어느 표의 어느 컬럼에서 가져오나. `None` / `RunNoise` / `PickupQuiet` / `PickupLoud` / `PushNoise` / `Decoy`(여기까지 `CL9Rules` 소리) / `PackAlarm`(**발신한 몬스터 행의 `PackAlarmRadius`**) | balance |
| `FallbackRadius` | `float` | cm | `RadiusFrom == None`인 행의 반경. 판정 반경이 없는 행동(수레·착지·깨짐·몬스터 근접 소나)만 쓴다 | balance |
| `RadiusScale` | `float` | 배율 | 최종 반경 = (위 둘 중 하나) × 이 값. **기본 1.0** — 1.0이 아니면 링이 판정과 달라진다 | balance |
| `Speed` | `float` | cm/s | 링이 퍼지는 속도. 수명 = 반경 ÷ 속도 | balance |
| `Thickness` | `float` | cm | 링 띠의 두께 | balance |
| `Color` | `FLinearColor` | — | 기본 색 | balance |
| `bUseInstigatorColor` | `bool` | — | 낸 사람의 슬롯 색으로 덮어쓸지. 몬스터 행은 false | balance |
| `Power` | `float` | 배율 | 밝기·알파. 나이아가라에 그대로 넘어간다 | balance |
| `OwnerPowerScale` | `float` | 배율 | **자기가 낸 파동**에만 곱하는 밝기 — 내 소리와 남의 소리를 가른다 | balance |
| `OcclusionRays` | `int32` | 개 | 생성 때 쏘는 수평 광선 수. 0이면 차폐 계산을 하지 않는다(벽 없는 마당용) | balance |
| `TraceHeight` | `float` | cm | 광선을 쏘는 높이(발원점 기준) — 바닥 턱에 걸리지 않을 만큼 | balance |
| `bWallStamp` | `bool` | — | 벽에 닿은 자리에 세로 자국을 찍나 | balance |
| `StampHeight` | `float` | cm | 자국 기둥의 높이 | balance |
| `StampDensity` | `float` | 배율 | 자국 한 자리에 찍는 점 수 | balance |
| `StepDistance` | `float` | cm | 발신자가 이 거리를 이동할 때마다 한 번 발동. **0 = 원샷**(이동과 무관한 사건) | balance |
| `RepeatInterval` | `float` | s | 이 시간마다 반복 발동. **0 = 반복 없음**. 몬스터 근접 소나용 | balance |
| `MinInterval` | `float` | s | 같은 발신자·같은 행이 이 간격 안에 두 번 나지 않는다(폭주 안전판) | balance |
| `TriggerRadius` | `float` | cm | 이 안에 플레이어가 있을 때만 발동. **0 = 조건 없음**. `monster_prox` 전용 | balance |
| `SoundRow` | `FName` | — | **이 소음이 나면 같이 낼 [sound.md](sound.md) 행 이름**(2026-09-20). 기본 `None` = 소리 없음. 링과 같은 사건·같은 박자로 `CL9Sound::PlayAt(발원지)` — 아래 "소나 = 소리" | balance |

전역 열 둘:

| 열 | 타입 | 단위 | 뜻 | 값 |
|---|---|---|---|---|
| `MaxConcurrent` | `int32` | 개 | 한 머신에서 동시에 살아 있는 파동 수 상한. 넘으면 가장 오래된 것을 지운다 | balance |
| `CullDistance` | `float` | cm | 로컬 카메라에서 이 거리보다 먼 발원지는 아예 만들지 않는다 | balance |

### 행 목록 (열두 개 — 1차 열한 개 + `monster_pounce` 2026-09-20)

`run` · `pickup_quiet` · `pickup_loud` · `push` · `decoy` · `throw_break` · `cart` · `land` ·
`walk`(기본 꺼짐) · `monster_prox` · `monster_call` · `monster_pounce`.
각 행이 무엇을 뜻하는지는 [../plan/wave.md](../plan/wave.md) 규칙 4.

**`monster_pounce`는 잡기 예고의 링이다** — 몬스터가 `Pounce` 상태에 들어가는 순간 몬스터 자리에서 한 번 피어
"뒤에서 온다"를 바닥에 그린다([monster.md](monster.md) 잡기 예고). 원샷 행이라 `StepDistance`·`RepeatInterval`·
`TriggerRadius`는 0, `RadiusFrom`은 `None`(판정 반경이 없는 사건)이라 **`FallbackRadius > 0`이어야 로드 검사를
통과한다**(0이면 행이 비활성 = 링도 사건도 없다). `SoundRow`는 **비운다** — 예고 소리 `MonsterPounce`는 파동이
아니라 몬스터의 상태 전이가 낸다([sound.md](sound.md)). 반경·두께·색·광선·`MinInterval`은 balance 초안(값은
[../plan/data/wave.md](../plan/data/wave.md)).

### 소나 = 소리 — `SoundRow` (2026-09-20)

파동 행은 "이 소음이 나면 낼 소리"를 한 열로 가리킨다. `monster_prox`의 `SoundRow = MonsterNear`가 첫 사용자다 —
다가오는 몬스터의 근접 소나마다 링과 **같은 사건·같은 박자**(`RepeatInterval`)로 짧은 꼬꼬댁이 난다. 새 사건도 새
복제도 없다(멀티캐스트 하나에 링과 소리가 같이 얹힌다). 규칙 셋:

- **링이 안 그려져도 소리는 난다.** 재생은 `MinInterval` 뒤·시각 게이트(표시 시점·`CullDistance`·나이아가라 에셋) 앞이다
  (아래 "각 머신의 재생" 3단계). 정전이 아니어서 링이 없거나, 카메라에서 멀어 링이 컬링돼도 소리는 난다 — 거리 감쇠는
  사운드 에셋의 몫이다.
- **행이 꺼지면 사건 자체가 없다.** `bEnabled` false·로드 검사 탈락이면 `HostReportNoise` 입구에서 끝나므로 링도 소리도
  없다. `SoundRow`만 비우면 옛날처럼 링만 남는다.
- **코드에 소음원 분기가 없다.** `if (Source == MonsterProx)` 같은 것은 없다 — 다른 소음원(달리기·깨짐)에 소리를 붙이고
  싶으면 그 행의 `SoundRow`만 채운다(행 이름이 `SoundTable`에 없으면 조용히 통과).

리슨 호스트의 이중 재생은 상위 가드가 막는다 — `HostReportNoise`가 호스트 화면을 그리고(소리도 거기서), 자기 멀티캐스트는
`HasAuthority()`면 즉시 버리며, 소유 클라의 예측분은 `IsLocalInstigator`로 버린다 — 소리도 링과 같은 가드 안이다
(아래 "소유 클라의 예측"). 잠든 계열은 소나가 없으니 소리도 없고([monster.md](monster.md)), 2D 거리 소나라 위층 몬스터도
들린다 — 층 검사는 wave 미결이다.

**`monster_call`은 `RadiusFrom = PackAlarm` · `FallbackRadius = 0`이다** (2026-09-16 사용자).
몬스터의 울음 파동은 **무리 경보의 시각 표현**이라, 링 반경이 그 개체의 `PackAlarmRadius`와 같다 —
링이 닿은 동료는 실제로 깨어난다. 종마다 경보 반경이 다르면 링 크기도 저절로 달라진다.
사살 경보(`KillAlarmRadius`)를 `monster_death` 같은 별도 행으로 둘지는 미결 —
지금은 울음 하나로 시작한다.

---

## 소음 이벤트 — 누가 어디서 부르나

```cpp
// ACL9GameState — 호스트 전용. 소음 사건의 단일 입구.
void HostReportNoise(ECL9NoiseSource Source, const FVector& Location,
                     float Radius, AActor* NoiseInstigator);
```

인자 이름이 `Instigator`가 아닌 것은 `AActor`에 같은 이름의 멤버가 있어서다 — 액터 안에서
`Instigator`라는 이름의 인자를 쓰면 이 프로젝트의 빌드 설정(V7, 그림자 변수 = 에러)에서
컴파일이 실패한다. 액터가 아닌 `UCL9WaveRenderer` 쪽은 그대로 `Instigator`다.

`Instigator`는 **소리를 낸 액터**다 — 플레이어가 냈으면 그 `ACL9PlayerState`, 몬스터가 냈으면 그
`ACL9Monster`. 받는 쪽이 캐스팅으로 가른다: 플레이어면 슬롯 색과 "내 것인가" 판정에 쓰고,
몬스터면 그 개체의 행에서 `PackAlarmRadius`를 읽는다(`monster_call`). 둘 다 복제되는 액터라
참조가 각 머신에서 해석된다.

`Radius`는 호스트가 그 행동의 판정 반경(없으면 0)을 그대로 넘긴다. **파동은 이 값을 쓰지
않는다**(각 머신이 다시 파생한다) — 몬스터 청각을 이 이벤트로 통일할 때 쓰려고 지금부터
자리를 잡아 두는 인자다.

**꺼진 행은 사건도 내지 않는다** (2026-09-16 사용자). `HostReportNoise`는 입구에서
`UCL9WaveTable::IsEnabled(Source)`(행이 있고 · 로드 검사를 통과했고 · `bEnabled`)를 보고, 아니면
로컬 재생도 멀티캐스트도 하지 않고 돌아간다 — 걷기처럼 늘 나는 소음이 아무 그림도 없이 4인분
Unreliable RPC로 오가지 않게 하려는 것이다. 발신 쪽도 같은 기준을 미리 본다: 걸음 누적
(`GetNoiseStepDistance`)은 꺼진 행에 0을 돌려 걸음을 세지 않고, 소유 클라의 예측(`EmitNoise`)도
꺼진 행이면 그리지 않는다. 몬스터 청각을 이 입구로 통일하는 날에는 "꺼진 행 = 안 들린다"가
되어 버리므로 그때 다시 본다.

| 소음원 | 부르는 곳 | 비고 |
|---|---|---|
| `Run` · `Walk` | `ACL9Character::TickStepNoise` — 누적 이동 거리가 `StepDistance`를 넘을 때마다 | 달리는 중인지는 `GetCL9CharacterMovement()->IsRunning()`. 땅을 딛고 있을 때만 센다. **꺼진 행은 사건도 내지 않는다**(아래 "꺼진 행") — 기본값의 `walk`는 걸음을 세지도 않는다 |
| `PickupQuiet` / `PickupLoud` | `ACL9Character::HostPickUp` | 어느 쪽인지는 `RecentRunTime` 안에 달렸는가 ([../plan/data/rules.md](../plan/data/rules.md) 소리) |
| `Push` | `ACL9Monster::OnShove` — 민 사람이 `ACL9Character`일 때 | 위치는 밀린 몬스터가 아니라 **민 사람의 발밑**(`ACL9Character::GetFootLocation`), `Instigator`도 민 사람의 `ACL9PlayerState`다 |
| `Decoy` | 미끼가 놓이는 호스트 코드 (탄 요리·거위 잠듦) | **아직 없다** — 코드에 미끼를 놓는 자리가 없어 이 행은 표에만 있다 |
| `ThrowBreak` | `ACL9Item::HandleImpact` — 깨짐 문턱을 넘은 가지 | 금 감(`Crack`)에는 붙이지 않는다. `Instigator`는 깨진 아이템이라 사람 색을 쓰지 않는다 |
| `Cart` | `ACL9Character::TickStepNoise` — 수레를 끄는 동안 `StepDistance`마다 | 거리를 재는 것은 끄는 사람이고 **위치는 수레**다 |
| `Land` | `ACL9Character::Landed` 오버라이드 | 착지 속도 문턱은 두지 않는다(원하면 `MinInterval`로 억제) |
| `MonsterProx` | `ACL9Monster::HostTickProxNoise` — `TriggerRadius` 안에 산 플레이어가 있고 `RepeatInterval`이 지났을 때 | `Instigator`는 그 몬스터. **깨어 있는 상태만** 낸다 — `Asleep`·`WakeTelegraph`(잠든 계열)와 `Dead`는 내지 않는다(2026-09-16 사용자: 잠든 닭까지 자리를 알려 주면 기상 예고로 배우던 것이 값싸진다). `RepeatInterval`이 0이면 이 행은 울리지 않는다(반복으로만 나는 소음원이다) |
| `MonsterCall` | `ACL9Monster::FirePackAlarm` — **무리 경보가 실제로 나가는 자리** | `Instigator`는 그 몬스터 — **반경은 그 개체 행의 `PackAlarmRadius`**. 거위는 `AlertWindup`이 끝나 우는 순간이 곧 이 자리이고, 기상 예고(`WakeTelegraph`)는 경보가 아니라 링도 없다 |
| `MonsterPounce` | `ACL9Monster::EnterState(Pounce)` — 잡기 예고 진입 한 줄 (2026-09-20) | `Instigator`는 그 몬스터, 위치는 몬스터 자리, `Radius` 0(판정 없음). 덮침·헛침에는 사건이 없다. 별칭 `monster_pounce`(`CL9Wave::ToAlias`가 enum 이름의 소문자 스네이크라 자동). `ValidateRows`의 `PackAlarm` 허용 집합(몬스터 소음원)에 든다 |

**소유 클라의 예측**: 자기가 낸 소음은 서버 왕복을 기다리지 않는다. 발걸음은 짧은 간격으로
나므로 RTT만큼 늦으면 링이 걸음과 어긋나 보인다. 규칙은 둘뿐이다.

- 소유 클라는 자기 캐릭터가 내는 소음을 **로컬에서 먼저** `UCL9WaveRenderer::PlayWave`로 그린다.
  예측하는 것은 **클라가 스스로 아는 것**뿐이다 — 발걸음(`Run`·`Walk`)·수레(`Cart`)·착지(`Land`).
  줍기·밀치기·깨짐은 호스트가 판정한 뒤에야 알 수 있어 예측하지 않는다.
- 멀티캐스트가 도착했을 때 `Instigator`가 이 머신의 로컬 플레이어면 **버린다.**
- **호스트는 멀티캐스트를 통째로 버린다** — `HostReportNoise`가 자기 렌더러에 이미 그렸기 때문이다.
  (멀티캐스트는 리슨 호스트에서도 실행되므로, 자기 것만 거르면 **클라이언트가 낸 소리**가 호스트
  화면에 두 번 핀다. 그래서 가드가 호스트에서는 `HasAuthority()` 하나로 넓어진다.)

예측이 호스트 판정과 어긋날 수 있지만 표현 전용이라 링 하나가 헛돌 뿐이고, 몬스터가 무엇을
듣는가는 여전히 호스트의 이벤트만 정한다.

---

## 복제 — Unreliable NetMulticast

```cpp
// ACL9GameState
UFUNCTION(NetMulticast, Unreliable)
void Multicast_Noise(ECL9NoiseSource Source, FVector_NetQuantize Location,
                     AActor* NoiseInstigator);
```

셰이크가 쓰는 "복제 시각 비교"([character.md](character.md) 맞는 느낌) 대신 멀티캐스트를 고른
근거다.

- **위치가 매번 다르고 빈도가 높다.** 시각 하나를 복제해 틱에서 비교하는 방식은 값이 하나뿐인
  1회 연출(`SwingHitServerTime`)에나 맞는다. 발걸음은 초당 여러 번 나고 그때마다 좌표가 달라
  시각·좌표·발신자를 한 벌로 실어야 한다.
- **전원이 보는 사건이다.** 셰이크는 `COND_OwnerOnly`로 본인만 느끼는 연출이라 시각 하나로
  됐지만, 파동은 남의 소리도 보여야 하는 것이 목적이다.
- **유실을 허용한다.** 발걸음 링 하나가 빠져도 다음 걸음이 온다. 그래서 Unreliable이고,
  이는 사운드가 이미 쓰는 채널과 같다 — "호스트발 Multicast RPC(Unreliable — 사운드는 유실
  허용)" ([../conventions.md](../conventions.md) § 9 · [sound.md](sound.md) 호출 규칙).
  **파동은 소리의 시각 쌍둥이이므로 같은 채널을 쓰는 것이 옳다.**
- **Reliable이면 다른 RPC를 밀어낸다.** 넷이 동시에 달리면 초당 수십 개가 신뢰 큐에 쌓여
  줍기·문 열기 같은 판정 RPC의 도착이 늦어진다.

`ACL9GameState`에서 보내므로 전원에게 간다 — 맵 반대편의 발걸음도 페이로드가 도착한다.
페이로드가 작아(enum 1바이트 + 양자화 좌표 + 객체 참조) 지금은 그대로 두고, **거리 컷은 수신
측에서** `CullDistance`로 한다. 대역폭이 문제가 되면 발신 액터(캐릭터·몬스터)에서 멀티캐스트해
엔진의 네트워크 관련성으로 거르는 쪽으로 옮긴다 (미결).

---

## 각 머신의 재생

```cpp
UCLASS()
class UCL9WaveRenderer : public UWorldSubsystem   // 로컬 표현 전용 — 복제 무접촉
{
public:
    /** 이 머신에 파동 하나를 그린다. bPredicted면 OwnerPowerScale을 적용한다 */
    void PlayWave(ECL9NoiseSource Source, const FVector& Location,
                  AActor* Instigator, bool bPredicted);
};
```

`UCL9UIManager`가 `UGameInstanceSubsystem`인 것과 같은 선례를 따르되, 파동은 레벨과 함께 살고
죽으므로 `UWorldSubsystem`이다. 레벨에 액터를 놓아야 하는 `ACL9NightLighting` 방식은 쓰지
않는다 — 맵이 생성기로 만들어지는 이 게임에서 "레벨에 하나 놓아야 한다"는 전제가 취약하다.

재생 한 번의 순서 (2026-09-20 — `SoundRow`가 들어오며 2·3단계의 자리가 바뀌었다, 아래 주):

1. **행 조회** — `GameInstance`가 물고 있는 `UCL9WaveTable`에서 `Source`로
   (`UCL9WaveTable::FindRow` — 로드 검사에 떨어진 행은 여기서 이미 없는 것이 된다).
   행이 없거나 `bEnabled`가 false면 조용히 끝낸다.
2. **폭주 안전판 + 시각 스탬프** — 같은 발신자·같은 행이 `MinInterval` 안에 두 번 나지 않는다.
   통과하면 **여기서 시각을 찍는다** — 뒤 단계(표시 시점·거리 컷)에서 걸려 링이 안 생겨도 간격은 찍힌다.
3. **소리** — `Row.SoundRow`가 있으면 `CL9Sound::PlayAt(발원지)`. 시각 게이트 **앞**이라 링이 안 그려져도
   난다(위 "소나 = 소리"). 행이 `SoundTable`에 없으면 조용히 통과.
4. **표시 시점** — 아래 "표시 시점 판정".
5. **거리 컷** — 로컬 카메라에서 `CullDistance` 밖이면 끝낸다.
6. **반경** — `RadiusFrom`에 따라 `CL9Rules`의 소리 컬럼, 또는 `PackAlarm`이면 `Instigator` 몬스터
   행의 `PackAlarmRadius`, 또는 `None`이면 `FallbackRadius`. 거기에 `RadiusScale`을 곱한다.
   몬스터 행의 값이 0이면 그릴 것이 없으므로 조용히 끝낸다.
7. **색** — `bUseInstigatorColor`이고 `Instigator`가 플레이어면 그 사람의 슬롯 색, 아니면 `Color`.
   밝기는 `Power` × (자기 것이면 `OwnerPowerScale`)이고, "자기 것"은 `bPredicted`이거나
   `Instigator`가 이 머신의 로컬 플레이어인 경우다.
8. **진단 트레이스** — `OcclusionRays`개 트레이스로 광선별 최대 거리 배열을 만든다. **지금은
   나이아가라에 넘기지 않는다**(아래 "벽 차폐" — 차폐는 에셋 안의 Collision이 한다). `cl9.shownoise`
   로그의 광선 수가 이 배열이고, 배열 DI가 생기면 다시 잇는 자리다.
9. **스폰** — `UNiagaraFunctionLibrary::SpawnSystemAtLocation`으로 행의 나이아가라 시스템을
   발원지에 띄우고, 사용자 파라미터를 넣는다. 컴포넌트는 `bAutoDestroy`로 스스로 사라지고,
   `MaxConcurrent`를 넘으면 가장 오래된 것을 즉시 없앤다.
   **`Niagara`가 비어 있으면 여기서만 멈춘다** — 위의 판정·트레이스·로그는 그대로 돈다(에셋이
   들어오기 전에도 `cl9.shownoise`로 사건을 검증할 수 있게).

**주 — `MinInterval` 스탬프가 시각 게이트 앞으로 왔다** (2026-09-20). 옛 순서는 행 → 표시 시점 → `MinInterval` →
거리 컷 → 반경 → … → 스폰 직전 스탬프였다 — 링이 실제로 생길 때만 간격을 찍었다. 소리가 `MinInterval` 뒤·시각 게이트
앞에 있어야 하므로 스탬프도 그 자리로 옮겼고, 그 결과 **표시 시점·거리 컷에 걸린 사건도 간격을 찍는다**. 폭주 안전판의
뜻("같은 발신자·같은 행이 이 간격 안에 두 번 나지 않는다")에 더 맞고 링 그림에는 차이가 없다 — 정전 전용 행이 정전
밖에서 사건을 내면 그 사건도 간격을 소비한다는 것만 달라졌다.

나이아가라가 받는 사용자 파라미터 **여덟** (이름은 `NS_CL9_Wave`의 `User.` 변수와 1:1 —
2026-09-16 리드가 만든 에셋 기준):

| 파라미터 | 타입 | 무엇 |
|---|---|---|
| `User.Radius` | float | 최종 반경 (cm) |
| `User.Speed` | float | 전파 속도 (cm/s) |
| `User.Thickness` | float | 링 두께 (cm) |
| `User.Color` | LinearColor | 최종 색 |
| `User.Power` | float | `Power` × (자기 것이면 `OwnerPowerScale`) |
| `User.RayCount` | int | 진단 트레이스의 광선 수 (= `OcclusionRays`, 0이면 안 쐈다) |
| `User.StampHeight` · `User.StampDensity` | float | 벽 자국 (`bWallStamp`가 false면 0 — 0이면 벽에 닿은 파티클이 그냥 죽는다) |

**`User.Distances`(Float Array)는 에셋에 없다.** 배열 DI 사용자 변수를 지금의 에디터 경로(MCP)로
만들 수 없어서, 차폐를 나이아가라 쪽 Collision으로 옮겼다(아래). C++는 그 이름을 부르지 않는다 —
없는 파라미터에 매번 경고가 난다. 배열 DI가 생기면 `PlayWave`의 주석 자리에서 다시 잇는다.

**C++는 파라미터 이름과 조회 키만 안다.** 수치도 에셋 경로도 갖지 않는다.

---

## 벽 차폐

소리가 지형을 속이면 안 된다 — [../plan/data/segkind.md](../plan/data/segkind.md)의
`bBlocksSight` 한 컬럼이 시야·파동·카메라를 한꺼번에 정하고, 구현은 **전용 트레이스 채널
`CL9_Occlusion`** 하나를 셋이 공유한다. 물통·상자처럼 일부러 시야를 안 가리게 해둔 낮은 조각은
그 채널에서 Ignore이므로 파동도 가리지 않는다.

**차폐는 나이아가라가 한다 (2026-09-16 리드 — 에셋 `NS_CL9_Wave` 기준).** 링 파티클이 나이아가라
CPU Collision(채널 `CL9_Occlusion`)으로 스스로 벽에 멈춘다 — 벽에 닿은 파티클은 거기서 죽거나
(아래 "벽 자국") 자국으로 바뀐다. C++가 광선 거리를 넘겨주는 구조(`User.Distances`, Data Interface
Array)는 에셋에 배열 DI 사용자 변수를 만들 수 없어 **보류**했다.

- **C++ 트레이스는 진단용으로 남아 있다.** `OcclusionRays`개 광선을 발원지에서 수평(2D)으로 균등하게,
  `TraceHeight` 높이에서, 최종 반경까지 `LineTraceSingleByChannel(CL9_Occlusion)` — 생성 때 한 번.
  결과 배열은 `cl9.shownoise` 로그의 광선 수로만 쓰이고 나이아가라에는 가지 않는다. 배열 DI가 생기면
  이 배열을 `User.Distances`로 넘기고 링 파티클이 자기 각도의 거리를 넘지 못하게 하는 원안으로 돌아간다.
  **이후 틱에는 트레이스가 없다** — 링이 커지는 것은 나이아가라 안의 일이다(CCL8도 같은 구조였다).
- **비용**: 지금은 나이아가라 Collision 비용(파티클 수 × 수명)이 본체이고, C++ 트레이스는
  파동 하나당 `OcclusionRays`회가 덤으로 붙는다. 넷이 동시에 달리면 초당 (인원 × 걸음 빈도 × 광선 수)라
  **광선 수는 행마다 다르게 둔다** — 원안도 큰 파동 540개, 발소리 180개로 갈랐다. 진단이 필요 없으면
  `OcclusionRays`를 0으로 두면 트레이스가 통째로 빠진다(차폐는 그대로 된다).

---

## 벽 자국

링이 벽에 닿은 자리에 세로 점 기둥이 한 번 찍힌다. 손전등이 닿지 않는 곳의 윤곽이 잠깐
드러나는, CCL8에서 가장 강한 그림이다.

- **구현 (2026-09-16 리드, `NS_CL9_Wave`)**: 같은 이미터의 **파티클 변형**이다. 링 파티클이
  Collision으로 벽에 닿으면 그 자리에서 `StampHeight` 높이의 세로 막대로 바뀌어 잠깐 남는다.
  `StampHeight`가 0이면(`bWallStamp` false — C++가 0을 넣는다) 닿은 파티클은 그냥 죽는다.
  이미터가 하나라 이벤트 왕복도 두 번째 이미터도 없다.
- 위치와 시각은 따로 계산하지 않는다 — 파티클이 벽에 닿는 순간이 곧 링이 그곳에 닿는 순간이다.
- 데칼은 쓰지 않는 쪽이 기본이다 — 수가 많고 수명이 짧아 데칼 풀이 금방 찬다. 굵고 오래 남는
  자국을 원하게 되면 그때 다시 본다.
- `bWallStamp` · `StampHeight` · `StampDensity`가 끄고 켜고 조절한다.

---

## 플레이어 고유색

파동은 **`ACL9PlayerState`에 색을 하나 물어볼 뿐이다.**

```cpp
const ACL9PlayerState* PS = Cast<ACL9PlayerState>(Instigator);
FLinearColor Color = (Row.bUseInstigatorColor && PS != nullptr)
    ? PS->GetSlotColor(Row.Color)   // 캐릭터 행이 아직 없으면 Row.Color로 떨어진다
    : Row.Color;
```

색의 출처는 파동이 정하지 않는다. 기존 기획이 이미 정해 두었다 — **슬롯 0~3이 신원이고
슬롯 색(`TintColor`)이 어둠 속에서 누가 누군지 아는 유일한 단서**
([../plan/data/character.md](../plan/data/character.md) `TintColor`와 슬롯 ·
[session.md](session.md) 슬롯 배정).

`ACL9PlayerState::GetSlotColor(Fallback)`은 **그 사람의 폰이 물고 있는 캐릭터 행의 `TintColor`**를
돌려준다(2026-09-16 구현) — C++에 색 배열은 없다. 행이 아직 도착하지 않았으면(스폰 직후·관전)
`Fallback`, 즉 행의 `Color`로 떨어진다. `SlotIndex`는 아직 없다 — 슬롯 번호가 캐릭터 행을
고르게 되면([session.md](session.md) 슬롯 배정) 이 함수는 그대로 두고 색만 갈린다.

**자기 파동이 더 밝은 것**은 색이 아니라 `OwnerPowerScale`이 한다 — 슬롯이 없어도 동작한다
(판정은 `Instigator == 내 PlayerState`).

몬스터가 낸 파동은 `Instigator`가 `ACL9Monster`라 이 경로를 타지 않는다 — 언제나 행의 `Color`다.

---

## 표시 시점 판정

```cpp
switch (Row.Visibility)
{
case ECL9WaveVisibility::Always:       return true;
case ECL9WaveVisibility::NightOnly:    return GameState->GetPhase() == ECL9NightPhase::Night;
case ECL9WaveVisibility::BlackoutOnly: return GameState->IsBlackout();
}
```

`IsBlackout()`은 `BlackoutServerTime`과 서버 시간을 비교해 각 머신이 파생하는 값이다
([light.md](light.md) 정전) — 복제 bool이 아니므로 클라도 그대로 읽는다.
**스폰 직전에 한 번만 본다.** 파동 하나의 수명이 1초 남짓이라 도중에 정전이 시작되는 경우를
따로 다루지 않는다.

---

## 로드 검사

표를 읽을 때 도는 검사다 — `UCL9WaveTable::ValidateRows()`가 `UCL9GameInstance::Init`에서
한 번 돌고, 어긋나면 어느 행이 깨졌는지 로그에 찍는다
([../plan/data/README.md](../plan/data/README.md) 값이 서로 지켜야 하는 관계).
**Error가 난 행은 그 자리에서 비활성이 된다** — `FindRow`가 없는 것으로 답하므로 그 소음원은
그리지도, 걸음으로 사건을 내지도 않는다. 경고 둘은 행을 끄지 않는다.

| 검사 | 왜 |
|---|---|
| `Speed > 0` · `RadiusScale > 0` | 0이면 링이 영원히 안 퍼지거나 생기지 않는다 |
| `RadiusFrom != None`인 행은 `FallbackRadius == 0` | 둘 다 채우면 어느 쪽이 이기는지 표를 보고 알 수 없다 |
| `RadiusFrom == None`인 행은 `FallbackRadius > 0` | 반경이 0인 파동은 아무것도 아니다 |
| `RadiusFrom == PackAlarm`인 행은 몬스터가 내는 소음원이어야 한다 | 플레이어에게는 읽을 몬스터 행이 없다 |
| `monster_prox`: 최종 반경 < `TriggerRadius` | **링이 플레이어에게 닿으면 공포 발신기가 레이더가 된다** ([../plan/wave.md](../plan/wave.md) 규칙 7). 로드 때 반경을 알 수 있는 `RadiusFrom == None`일 때만 본다 |
| `MaxConcurrent > 0` · `CullDistance > 0` | 상한이 없으면 파티클이 프레임을 먹는다 |
| (경고) `RadiusScale != 1.0` | 링이 판정 반경과 달라진다 — 에러는 아니지만 로그에 남긴다 |
| (경고) `bEnabled`인데 `Niagara`가 비었다 | 켜 놓고 아무것도 안 그리는 행 |

---

## 성능 상한

- **동시 수**: 살아 있는 파동 컴포넌트를 `UCL9WaveRenderer`가 선입선출 큐로 들고 있고,
  `MaxConcurrent`를 넘으면 가장 오래된 것을 즉시 없앤다. 나이아가라 시스템 자체의 파티클
  상한은 에셋의 몫이다(표 열이 아니다).
- **거리**: `CullDistance` 밖의 발원지는 트레이스도 스폰도 하지 않는다. 판정과 무관하므로
  안 그려도 게임이 달라지지 않는다.
- **트레이스**: 파동 하나당 `OcclusionRays`회, 생성 때 한 번 — 지금은 진단용이라 `OcclusionRays`를
  0으로 두면 통째로 빠진다. 차폐 본체인 나이아가라 Collision의 비용은 에셋의 파티클 수가 정한다.
- 파동은 **틱을 돌지 않는다.** 스폰 뒤의 모든 움직임은 나이아가라 안에서 일어난다.

---

## 어디서 무엇을 고치나

| 하려는 일 | 어디를 고치나 | 빌드 |
|---|---|---|
| **색을 바꾸고 싶다** | `WaveTable` 행의 `Color`. 플레이어 고유색이면 캐릭터 표의 `TintColor` | 불필요 |
| **반경을 바꾸고 싶다** | `CL9Rules`의 소리 컬럼 — 판정과 링이 함께 움직인다. 링만 따로 움직이려면 `RadiusScale`(권장하지 않음, 경고가 뜬다) | 불필요 |
| **몬스터 울음 링의 크기를 바꾸고 싶다** | 그 몬스터 행의 `PackAlarmRadius` — 무리 경보 반경과 링이 함께 움직인다 | 불필요 |
| **어떤 소음원을 끄고 싶다** | 그 행의 `bEnabled` — 링도 소리도 사건째 사라진다 | 불필요 |
| **어떤 소음에 소리를 붙이고 싶다 / 떼고 싶다** | 그 행의 `SoundRow`에 [sound.md](sound.md) 행 이름 / `None` (`monster_prox` = `MonsterNear`) | 불필요 |
| **정전에만 보이게 하고 싶다** | 그 행의 `Visibility` = `BlackoutOnly` | 불필요 |
| **리소스(모양)를 바꾸고 싶다** | 그 행의 `Niagara` | 불필요 |
| **링이 더 빨리·두껍게·밝게** | `Speed` · `Thickness` · `Power` | 불필요 |
| **발걸음 간격을 바꾸고 싶다** | `StepDistance` (수레도 같다) | 불필요 |
| **벽 자국을 끄고 싶다** | `bWallStamp` | 불필요 |
| **무거워서 줄이고 싶다** | `OcclusionRays` ↓ · `MaxConcurrent` ↓ · `CullDistance` ↓ | 불필요 |
| **새 소음원을 더하고 싶다** | `ECL9NoiseSource`에 값 하나 + 그 사건 코드에서 `HostReportNoise` 한 줄 + 표에 행 하나 | **필요** |
| **낮은 물건이 소리를 가리게/안 가리게** | 조각 표의 `bBlocksSight` — 시야·파동·카메라가 한꺼번에 바뀐다 | 불필요 |

---

## 네트워크 정리

| 것 | 방식 |
|---|---|
| 소음 사건 | 호스트 판정 → `Multicast_Noise` (Unreliable) → 각 머신 표현 |
| 내가 낸 소음 | 소유 클라 로컬 예측 재생 + 멀티캐스트 수신 시 본인 것 버림 |
| 반경 | **복제 없음** — 각 머신이 `CL9Rules`·몬스터 표·파동 표에서 파생 (표 일치는 같은 빌드가 전제 — `CL9TuningHash` 대조는 미구현) |
| 누가 냈나 | `Multicast_Noise`의 `AActor* Instigator` — 플레이어면 `ACL9PlayerState`, 몬스터면 `ACL9Monster` |
| 링의 모양·색·밝기 | 복제 없음 — 로컬 표에서 |
| 소음에 붙는 소리(`SoundRow`) | 복제 없음 — 같은 멀티캐스트를 받은 각 머신이 로컬 표를 읽어 튼다. 리슨 호스트·소유 클라의 이중 재생 가드는 링과 같다 |
| 차폐 트레이스 | 각 머신 로컬 (같은 레벨이므로 같은 결과) |
| 플레이어 색 | `ACL9PlayerState`의 슬롯에서 파생 |

---

## PIE 확인 시나리오 (초안)

[../testing/pie-scenarios.md](../testing/pie-scenarios.md)에 **새 절 `T. 파동`**으로 붙일 것을
제안한다(현재 A~S가 쓰였다). 구현 실행이 값을 넣은 뒤 확정한다.

쓰는 치트는 둘이다 ([cheats.md](cheats.md)) — `cl9.wave <alias>`(내 발밑에서 그 행을 한 번, 호스트) ·
`cl9.shownoise`(판정 반경 디버그 원 + 파동마다 반경·광선 수 로그, 로컬).

- **T1. 달리면 발밑에 링이 핀다** — 걸을 때는 아무것도 안 뜨고, 달리면 발걸음마다 링이 하나씩
  퍼진다. 링이 멈추는 자리가 `RunNoiseRadius`와 눈으로 같다(치트로 반경 디버그 원을 함께 켜서 대조).
- **T2. 벽에서 잘리고 자국이 찍힌다** — 벽 앞에서 달린다. 링이 벽을 통과하지 않고, 벽에 닿은
  자리에 세로 자국이 한 번 남는다. 물통·상자 뒤에서는 **통과한다**(`bBlocksSight` false).
- **T3. 2인 PIE — 남의 링은 그 사람 색, 내 링이 더 밝다** — 클라가 달리는 것을 호스트에서 본다.
  내 발걸음 링은 내 화면에서 지연 없이 뜬다(예측). 이중으로 그려지지 않는다.
- **T4. 표로 끄고 켠다** — `walk` 행을 켜면 걷기에도 링이 뜨고, `run` 행의 `bEnabled`를 끄면
  달려도 안 뜬다. 에디터에서 바꾸고 **빌드 없이** 확인된다.
- **T5. 몬스터 근접 소나** — 몬스터가 다가오면 몬스터 자리에서 링이 반복해 핀다. **링이 내
  발밑까지 오지 않는다.** 벽 너머 몬스터의 링은 잘려서 보인다.
- **T6. 정전 전용 행** — 한 행을 `BlackoutOnly`로 바꾸고 `cl9.blackout` 전후를 비교한다.
- **T7. 울음 링 = 무리 경보 반경** — 거위를 깨워 울리고, 링이 닿은 동료가 깨어나는지 본다.
  몬스터 행의 `PackAlarmRadius`를 크게 바꾸면 **링도 같이 커진다**(빌드 없이).

---

## 나이아가라 에셋의 실제 모양 (2026-09-16 리드 구현 — 사용자 지적 반영)

`/Game/Effects/NS_CL9_Wave`, 이미터 하나(`Ring`). 사용자 지적 셋을 반영한 상태다: **끝 지점에 원이 없다**(흰 원은
`cl9.shownoise` 디버그 그림뿐 — 평소엔 꺼져 있다), **점이 아니라 원이 퍼진다**, **바닥에 붙어 나간다**.

- **한 번 터지고 끝**: EmitterState `Life Cycle Mode Self` · `Loop Once` · `Loop Duration 0.2` · `Inactive Complete` — 컴포넌트가
  스스로 사라진다(무한 루프면 `MaxConcurrent`까지 쌓인다).
- **점 수는 둘레에서 정한다**: `SpawnBurst_Instantaneous.Spawn Count = max(User.RayCount, 2π·User.Radius ÷ (User.Thickness × 0.35))`.
  달리기(390cm·두께 12)면 583개 — 점 간격이 점 크기보다 좁아 **끊김 없는 선**으로 읽힌다. `User.RayCount`는 하한일 뿐이다.
- **바닥 밀착**: `CylinderLocation` 반경 45cm · 높이 0 · 원점 `Engine.Owner.Position + (0,0,3)` — 바닥 3cm 위. `AddVelocityFromPoint`
  원점도 같은 식(수평으로만 퍼진다). 45cm는 **플레이어 캡슐 반경 밖**이다 — 캡슐이 `CL9_Occlusion`을 막아 발밑에서 태어나면 즉시
  충돌한다(폰을 이 채널에서 Ignore로 두면 2cm로 줄일 수 있다 — 후속). 수명 = `max(User.Radius − 45, 1) ÷ User.Speed`.
- **차폐**: `Collision` 모듈(CPU Ray Traced, 채널 `CL9_Occlusion`, `Particle Radius Scale 0.1`, `KillOnCollision` 토글 **끔**, `Restitution 0`,
  휴식 상태 끔). 닿은 점은 `SetVariables`(Update)가 속도 0으로 멈추고 크기를 두께의 2배로 키운다(= 벽 자국은 벽 밑에 남는 점).
  `StampHeight`가 0인 행은 `KillParticles`가 닿은 점을 지운다(링이 벽에서 끊긴다).
- **렌더러**: 스프라이트, `FaceCamera`, `Unaligned`, 피벗 중앙, `DefaultSpriteMaterial`. 색 = `User.Color` 직접 링크.
- **이 에디터 경로(MCP)에서 안 되는 것 — 시도했다가 뺀 것들**: 리본 렌더러(재질을 넣어도 아무것도 안 그림), `CustomFacingVector`(위를
  향한 면 — 파티클이 통째로 사라짐), 색을 HLSL로 만들기(`User.Color × Power` — 사라짐), `Particles.Position`을 HLSL로 넣기(사라짐),
  배열 DI 사용자 변수. 그래서 **`User.Power`(밝기)와 알파 페이드는 아직 링에 반영되지 않는다** — C++이 `User.Color`에 Power를 곱해
  넘기는 한 줄이 후속이다. 위치·float·int·enum HLSL과 링크 입력은 된다.
- 손으로 만질 때: 두께를 줄이면(`Thickness` 6~8) 선이 더 날카로워지고 점 수는 자동으로 는다. 반경·색·개수는 전부 표(`WaveTable`)다.

---

## 구현 시점과 선행 작업

파동 자체는 어느 스텝에도 매여 있지 않다(연출이라 어느 시점에 넣어도 게임이 안 깨진다).
다만 아래 셋이 먼저 있어야 한다.

1. ~~**`Niagara` 모듈 의존성**~~ — `CL9.Build.cs`에 `"Niagara"`가 들어갔다 (2026-09-16).
2. **`CL9_Occlusion` 트레이스 채널** — `Config/DefaultEngine.ini`에 `ECC_GameTraceChannel2`로
   들어갔다 (2026-09-16, 기본 Block). **조각 표의 `bBlocksSight`가 false인 종류를 그 채널에서
   Ignore로 두는 것은 맵 코드 몫이라 아직 없다** — 지금은 물통·상자도 파동을 막는다.
   시야 판정도 이 채널로 옮겨 갈 자리다.
3. ~~**나이아가라 시스템 에셋**~~ — `NS_CL9_Wave`가 들어갔다 (2026-09-16 리드): 이미터 하나(링 파티클이
   Collision으로 벽에 멈추고 자국으로 변형) + `User.` 파라미터 여덟. 배열 DI(`User.Distances`)는 보류.

슬롯 색(`ACL9PlayerState::GetSlotColor()`)은 선행이 아니었다 — 파동과 함께 들어갔다.

---

## 참고

- 기획: [../plan/wave.md](../plan/wave.md) · 값: [../plan/data/wave.md](../plan/data/wave.md)
- 판정 반경: [../plan/data/rules.md](../plan/data/rules.md) 소리 · 몬스터 감각: [monster.md](monster.md)
- 경보 반경(`PackAlarmRadius`·`KillAlarmRadius`): [../plan/data/monster/common.md](../plan/data/monster/common.md)
- 차폐 목록: [../plan/data/segkind.md](../plan/data/segkind.md) `bBlocksSight`
- 정전·페이즈: [light.md](light.md) · 표현·복제 관례: [character.md](character.md) · [sound.md](sound.md)
- 위험 전달의 결: [warning.md](warning.md) · 화면 효과인 스캔 파동과의 구분: [ui.md](ui.md)
