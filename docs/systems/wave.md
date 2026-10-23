# 파동 시스템 (소음 반경 시각화)

> 소리가 난 자리에서 바닥 링이 퍼져 나가 **몬스터가 듣는 반경을 눈으로 보여 준다.**
> 호스트가 소음 사건을 한 곳으로 모아 방송하고, 각 머신이 같은 표를 읽어 똑같은 링을 그린다 —
> 판정은 여기 없다. 무엇을 왜 만드는지는 [../plan/wave.md](../plan/wave.md), 값은
> [../plan/data/wave.md](../plan/data/wave.md).

**구현 상태: 구현됨 (2026-09-16).** C++는 아래 설계 그대로다 — `ECL9NoiseSource`(CL9Types.h) ·
`UCL9WaveTable`(Data/CL9WaveTable.h) · `UCL9WaveRenderer`(FX/CL9WaveRenderer.h) ·
`ACL9GameState::HostReportNoise`/`Multicast_Noise` · `ACL9PlayerState::GetSlotColor`.
수치는 하나도 코드에 없다 — 전부 표의 열이고 값은 balance가 채운다.

**점 그림 (2026-10-15 사용자 — CCL8 웹판과 그림이 다르다는 지적).** 처음 에셋 `NS_CL9_Wave`는 점 583개를 한 번에 터뜨려
**점 자체가 날아가는** 링이었다 — 지나간 자리에 아무것도 안 남아 바닥 바퀴살이 없고, 벽 세로 커튼과 페이드가 에디터
경로 제약으로 빠져 있었다. CCL8은 정반대로 **보이지 않는 파면이 지나가며 제자리에 점을 떨구고, 점은 남아서 옅어진다.**
그래서 C++이 매 틱 파면을 키워 점을 고정 풀에 찍고(바닥 바퀴살·벽 세로 커튼·페이드 전부), 나이아가라
`NS_CL9_WaveDots`(`/Game/Effects/`)는 그 풀을 배열로 받아 **그리기만 한다**(아래 "점 그림"·"점 시스템"). `NS_CL9_Wave`는
더 이상 어디서도 가리키지 않는다.

남은 것 셋: **미끼(`decoy`)는 발생 이벤트가 없다**(코드에 미끼를 놓는 자리가 아직 없다 —
`grep Decoy Source/` 0건), **낮은 조각의 `CL9_Occlusion` Ignore는 맵 코드 몫이라 아직 없다**
(지금은 모든 조각이 파동을 막는다). 예전의 세 번째 항목이던 배열 DI 보류는 풀렸다 — 2026-10-15 MCP로 스택 입력을 없는
`User.*` 배열에 링크하면 에디터가 그 사용자 파라미터를 만들어 준다(아래 "점 시스템").

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
UCL9WaveRenderer (UTickableWorldSubsystem — 로컬 표현 전용, 복제 없음)
  1. Instigator가 이 머신의 로컬 플레이어면 버린다 (이미 로컬에서 먼저 그렸다)
  2. 표에서 행을 찾는다 → bEnabled · MinInterval → (SoundRow가 있으면 소리) → Visibility · CullDistance 검사
  3. 반경 = (RadiusFrom이 가리키는 표 컬럼 또는 FallbackRadius) × RadiusScale
       — RadiusFrom이 CL9Rules 소리 컬럼이면 CL9Rules에서, PackAlarm이면
         Instigator 몬스터의 행에서 읽는다
  4. OcclusionRays개 수평 트레이스(CL9_Occlusion) → 광선별 벽까지 거리 배열 (막히지 않으면 무한대)
  5. 파면 하나를 목록에 올린다 (반경·속도·색·밝기·광선 거리)
  매 틱: 파면을 키우며 지난 자리에 바닥 점 고리를, 벽을 지난 광선에 세로 점 기둥을 점 풀에 찍는다
         → 풀을 배열(위치·색+페이드 알파·크기)로 펴서 NS_CL9_WaveDots 에 넘긴다 (아래 "점 그림")

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
    Swing,         // 프라이팬 휘두르기 — 휘두른 사람 발밑, 맞든 빗나가든 (2026-10-17 결정 C)
    Hit,           // 휘두르기가 닿음 — 지형은 닿은 자리, 몬스터·사람은 친 사람 발밑 (2026-10-17 결정 C)
    Crouch,        // 앉아 걷는 발걸음 — 몬스터는 듣지 않는다, 아주 작은 링으로 "거의 소리가 없다"만 (2026-10-22)
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

    /** 바닥 끊김 — 광선을 이 간격으로 더듬어 FloorDrop 아래까지 바닥이 없으면 끊는다 (0 = 안 함) */
    UPROPERTY(EditAnywhere, Category="Wave")
    float FloorProbeStep = 0.f;

    UPROPERTY(EditAnywhere, Category="Wave")
    float FloorDrop = 0.f;
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
| `bEnabled` | `bool` | — | 이 소음원이 파동을 내는가. 구현은 전부 되어 있고 이 체크박스가 켜고 끈다 | balance |
| `Visibility` | `ECL9WaveVisibility` | — | `Always` / `NightOnly` / `BlackoutOnly` — 기본 `Always` | balance |
| `RadiusFrom` | `ECL9NoiseRadiusSource` | — | 반경을 어느 표의 어느 컬럼에서 가져오나. `None` / `RunNoise` / `PickupQuiet` / `PickupLoud` / `PushNoise` / `Decoy`(여기까지 `CL9Rules` 소리) / `PackAlarm`(**발신한 몬스터 행의 `PackAlarmRadius`**) | balance |
| `FallbackRadius` | `float` | cm | `RadiusFrom == None`인 행의 반경. 판정 반경이 없는 행동(수레·착지·깨짐·몬스터 근접 소나)만 쓴다 | balance |
| `RadiusScale` | `float` | 배율 | 최종 반경 = (위 둘 중 하나) × 이 값. **기본 1.0** — 1.0이 아니면 링이 판정과 달라진다 | balance |
| `Speed` | `float` | cm/s | 파면이 퍼지는 속도. 파면이 반경에 닿는 시간 = 반경 ÷ 속도 | balance |
| `Color` | `FLinearColor` | — | 기본 색 | balance |
| `bUseInstigatorColor` | `bool` | — | 낸 사람의 슬롯 색으로 덮어쓸지 — **남의 파동에만**(내 파동은 늘 행 색, CCL8 D16 — 2026-10-15). 몬스터 행은 false | balance |
| `Power` | `float` | 배율 | 밝기 — 점 색에 `Dots.Brightness` × 이 값(× 자기 것이면 `OwnerPowerScale`)을 곱한다. 벽 자국 세기에도 곱한다 | balance |
| `OwnerPowerScale` | `float` | 배율 | **자기가 낸 파동**에만 곱하는 밝기 — 내 소리와 남의 소리를 가른다 | balance |
| `OcclusionRays` | `int32` | 개 | 생성 때 쏘는 수평 광선 수 — **바닥 점이 서는 각도이자 벽 자국 자리**다. 0이면 로드 검사가 행을 끈다(2026-10-15 — 예전엔 "차폐 생략") | balance |
| `TraceHeight` | `float` | cm | 광선을 쏘는 높이(발원점 기준) — 바닥 턱에 걸리지 않을 만큼 | balance |
| `bWallStamp` | `bool` | — | 벽에 닿은 자리에 세로 자국을 찍나 | balance |
| `StampHeight` | `float` | cm | 자국 기둥의 꼭대기 높이(발원지 바닥 기준) | balance |
| `StampDensity` | `float` | 배율 | 벽 자국 세로 점선의 촘촘함 — 점 간격 = 전역 `Dots.RingSpacing` ÷ 이 값(1 = 바닥 바퀴살과 같은 간격) | balance |
| `bUntilWall` | `bool` | — | **핑** — 켜면(같은 발신자의 지난 핑에서 `UntilWallCooldown`이 지났으면) 반경 대신 `UntilWallRadius`까지 퍼져 바퀴살마다 벽에서 멈춘다. 플레이어 소음원 행만(몬스터 행이면 로드 검사 Error·무시) | balance |
| `UntilWallRadius` | `float` | cm | 핑이 벽을 못 만난 쪽에서 멈추는 거리 — 밝기 감쇠·벽 자국 세기도 이 거리로 잰다 | balance |
| `UntilWallCooldown` | `float` | 초 | 같은 발신자의 핑 뒤 이 시간 안의 사건은 보통 링(판정 반경) | balance |
| `bPassThroughBodies` | `bool` | — | 사람·몬스터·아이템을 차폐에서 통과하고 바닥 점선이 그 위를 타고 넘는다. 플레이어 소음원 행만 | balance |
| `RideHeight` | `float` | cm | 타고 넘을 수 있는 가장 높은 면(발원지 바닥 기준) — 높이를 재는 광선이 내려오는 높이 | balance |
| `bRevealBodies` | `bool` | — | 파면이 닿은 몸의 실루엣을 점무늬 겉껍데기로 드러낸다(전역 `Reveal`). 플레이어 소음원 행만 | balance |
| `StepDistance` | `float` | cm | 발신자가 이 거리를 이동할 때마다 한 번 발동. **0 = 원샷**(이동과 무관한 사건). `StepsPerWave > 0` 이면 읽지 않는다 | balance |
| `StepsPerWave` | `int32` | 걸음 | **발소리 몇 번에 한 번** 파동을 내나 — 발걸음 행(`run`·`walk`·`crouch`·`cart`)만. 발소리 시계(`ACL9Character::TickFootstepSound` 의 `StepCount`)를 세므로 링이 발소리와 같은 박자로 핀다. 행이 바뀐 뒤(걷다 달리기 등) 첫 발소리는 바로 낸다. **0 = 옛 방식(`StepDistance` 누적)** (2026-10-22 사용자 "항상 나가는 것보다 n에 한 번씩") | balance |
| `RepeatInterval` | `float` | s | 이 시간마다 반복 발동. **0 = 반복 없음**. 몬스터 근접 소나용 | balance |
| `MinInterval` | `float` | s | 같은 발신자·같은 행이 이 간격 안에 두 번 나지 않는다(폭주 안전판) | balance |
| `TriggerRadius` | `float` | cm | 이 안에 플레이어가 있을 때만 발동. **0 = 조건 없음**. `monster_prox` 전용 | balance |
| `SoundRow` | `FName` | — | **이 소음이 나면 같이 낼 [sound.md](sound.md) 행 이름**(2026-09-20). 기본 `None` = 소리 없음. 링과 같은 사건·같은 박자로 `CL9Sound::PlayAt(발원지)` — 아래 "소나 = 소리" | balance |
| `bUrgencyFromMonsterState` | `bool` | — | 긴박도 입력 — 몬스터 상태(전역 `Urgency.StateUrgency`) (2026-10-15, 아래 "긴박도") | balance |
| `bUrgencyFromMonsterHp` | `bool` | — | 긴박도 입력 — 몬스터의 잃은 체력 비율 | balance |
| `bUrgencyFromCharacterHp` | `bool` | — | 긴박도 입력 — 소리를 낸 캐릭터의 잃은 체력 비율 | balance |
| `bUrgencyFromRage` | `bool` | — | 긴박도 입력 — 몬스터 광폭화 게이지(게이지 구현 전이라 늘 0) | balance |
| `UrgentColor` | `FLinearColor` | — | 긴박할 때 옮겨 가는 색(선형 섞기). 행마다 두어 다친 사람의 파동이 몬스터 소나 색으로 가지 않게 한다 | balance |

전역 열 넷 (2026-10-15 — 행의 `Niagara`·`Thickness` 열을 지우고 `DotSystem`·`Dots`를 더했다):

| 열 | 타입 | 단위 | 뜻 | 값 |
|---|---|---|---|---|
| `MaxConcurrent` | `int32` | 개 | 한 머신에서 동시에 퍼지고 있는 파면 수 상한. 넘으면 가장 오래된 파면을 버린다(이미 떨군 점은 제 수명대로 옅어진다) | balance |
| `CullDistance` | `float` | cm | 로컬 카메라에서 이 거리보다 먼 발원지는 아예 만들지 않는다 | balance |
| `FloorProbeStep` | `float` | cm | **바닥 끊김** — 생성 때 광선마다 이 간격으로 바닥을 더듬어(광선 높이에서 `FloorDrop` 아래까지 내려 쏜다), 바닥이 없는 첫 자리 앞에서 광선을 끊는다. 내려가는 계단·구덩이·난간 밖 허공에 점이 뜨지 않는다. 끊긴 광선은 벽이 아니다(법선 영벡터) — 자국·호가 서지 않고 그냥 끝난다. 0 = 더듬지 않음 (2026-10-22 사용자 "계단 밑에도 파동이 퍼진다 — 알아서 잘라라") | balance |
| `FloorDrop` | `float` | cm | 발원지 바닥보다 이만큼 아래까지 바닥이 없어야 끊긴 것이다 — 한 계단 턱은 넘긴다 | balance |
| `DotSystem` | `TSoftObjectPtr<UNiagaraSystem>` | — | 점을 그리는 나이아가라 시스템 — 모든 행이 같이 쓰는 하나. **소프트 참조** — 비어 있으면 판정·소리·로그는 돌고 아무것도 그리지 않는다. `UCL9GameInstance::Init` 이 미리 올려 붙들고, `UCL9WaveRenderer::OnWorldBeginPlay` 가 판 시작 때 띄워 판 내내 쓴다(2026-09-21 — 첫 링에서 동기 로드 + 첫 활성화 컴파일로 ≈500ms 스톨, [performance.md](performance.md)) | fx |
| `Dots` | `FCL9WaveDotLook` | — | 점 모양 — 풀 크기·고리 간격·간격 수·수명·크기·높이·밝기·페이드·벽 자국 점(필드 22개, 값과 CCL8 근거는 [../plan/data/wave.md](../plan/data/wave.md) `Dots`) | balance |
| `Ripple` | `FCL9WaveRipple` | — | 물결 — 켜고 끄기·마루 고리 간격·둘레 점 간격·수명·크기·밝기·메아리 간격·메아리 감쇠 (2026-10-15, 아래 "물결") | balance |
| `Urgency` | `FCL9WaveUrgency` | — | 긴박도 효과 — 상태별 긴박도 지도, 효과 넷(색·밝기 / 물결 겹수 / 흔들림 / 펄스 빈도)의 켜고 끄기와 범위 (2026-10-15, 아래 "긴박도") | balance |

### 행 목록 (열다섯 개 — 1차 열한 개 + `monster_pounce` 2026-09-20 + `swing`·`hit` 2026-10-17 + `crouch` 2026-10-22)

`run` · `pickup_quiet` · `pickup_loud` · `push` · `decoy` · `throw_break` · `cart` · `land` ·
`walk`(2026-10-17 부터 켬 — `WalkNoiseRadius`) · `crouch`(2026-10-22 — 앉아 걷기, 판정 반경 없음·`FallbackRadius`) · `monster_prox` · `monster_call` · `monster_pounce` · `swing` · `hit`(2026-10-17 — `SwingNoiseRadius`·`HitNoiseRadius`;
`ACL9Character::HostSwing` 과 `HostTickSwingHit` 의 첫 명중·표면 자국이 낸다).
각 행이 무엇을 뜻하는지는 [../plan/wave.md](../plan/wave.md) 규칙 4.

**`crouch` 는 "링 = 들리는 범위" 계약의 예외다 (2026-10-22 사용자 — 뛰기·걷기·앉아 걷기 셋의 파동이 달라야 한다).** 앉아 걷기는 몬스터가 듣지 않으므로
(`ACL9Monster::SenseTick`) 판정 반경이 없고, 행은 `FallbackRadius`(아주 작게)로 "거의 소리가 없다"만 발밑에 보여 준다. 벽 자국·핑·실루엣은 켜지 않는다 — 무소음 이동에
지도가 딸려 오면 안 된다(`walk` 와 같은 이유). 행을 끄면 예전처럼 앉아 걷기에 아무것도 안 뜬다.

**`monster_pounce`는 잡기 예고의 링이다** — 몬스터가 `Pounce` 상태에 들어가는 순간 몬스터 자리에서 한 번 피어
"뒤에서 온다"를 바닥에 그린다([monster.md](monster.md) 잡기 예고). 원샷 행이라 `StepDistance`·`RepeatInterval`·
`TriggerRadius`는 0, `RadiusFrom`은 `None`(판정 반경이 없는 사건)이라 **`FallbackRadius > 0`이어야 로드 검사를
통과한다**(0이면 행이 비활성 = 링도 사건도 없다). `SoundRow`는 **비운다** — 예고 소리 `MonsterPounce`는 파동이
아니라 몬스터의 상태 전이가 낸다([sound.md](sound.md)). 반경·색·광선·`MinInterval`은 balance 초안(값은
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
Unreliable RPC로 오가지 않게 하려는 것이다. 발신 쪽도 같은 기준을 미리 본다: 걸음 박자·누적
(`GetNoiseRow`)은 꺼진 행을 없는 행으로 보아 걸음을 세지 않고, 소유 클라의 예측(`EmitNoise`)도
꺼진 행이면 그리지 않는다. 몬스터 청각을 이 입구로 통일하는 날에는 "꺼진 행 = 안 들린다"가
되어 버리므로 그때 다시 본다.

| 소음원 | 부르는 곳 | 비고 |
|---|---|---|
| `Run` · `Walk` · `Crouch` | `ACL9Character::TickStepNoise` — 발소리 `StepsPerWave` 번마다(행이 0 이면 누적 이동 거리가 `StepDistance`를 넘을 때마다) | 셋 중 하나다: 앉았으면(`bIsCrouched`) `Crouch`, 달리는 중(`GetCL9CharacterMovement()->IsRunning()`)이면 `Run`, 아니면 `Walk`. 땅을 딛고 있을 때만 센다. 발소리 수는 `TickFootstepSound` 의 `StepCount` — 그 함수가 한 틱 뒤에 돌아 링이 발소리보다 한 틱 늦지만 같은 박자다. 행이 바뀌면 첫 발소리에 바로 낸다. **꺼진 행은 사건도 내지 않는다**(아래 "꺼진 행") |
| `PickupQuiet` / `PickupLoud` | `ACL9Character::HostPickUp` | 어느 쪽인지는 `RecentRunTime` 안에 달렸는가 ([../plan/data/rules.md](../plan/data/rules.md) 소리) |
| `Push` | `ACL9Monster::OnShove` — 민 사람이 `ACL9Character`일 때 | 위치는 밀린 몬스터가 아니라 **민 사람의 발밑**(`ACL9Character::GetFootLocation`), `Instigator`도 민 사람의 `ACL9PlayerState`다 |
| `Decoy` | 미끼가 놓이는 호스트 코드 (탄 요리·거위 잠듦) | **아직 없다** — 코드에 미끼를 놓는 자리가 없어 이 행은 표에만 있다 |
| `ThrowBreak` | `ACL9Item::HandleImpact` — 깨짐 문턱을 넘은 가지 | 금 감(`Crack`)에는 붙이지 않는다. `Instigator`는 깨진 아이템이라 사람 색을 쓰지 않는다 |
| `Cart` | `ACL9Character::TickStepNoise` — 수레를 끄는 동안 끄는 사람의 발소리 `StepsPerWave` 번마다(0 이면 `StepDistance`마다) | 박자를 재는 것은 끄는 사람이고 **위치는 수레**다 |
| `Land` | `ACL9Character::Landed` 오버라이드 | 착지 속도 문턱은 두지 않는다(원하면 `MinInterval`로 억제) |
| `MonsterProx` | `ACL9Monster::HostTickProxNoise` — `TriggerRadius` 안에 산 플레이어가 있고 `RepeatInterval`이 지났을 때 | `Instigator`는 그 몬스터. **깨어 있는 상태만** 낸다 — `Asleep`·`WakeTelegraph`(잠든 계열)와 `Dead`는 내지 않는다(2026-09-16 사용자: 잠든 닭까지 자리를 알려 주면 기상 예고로 배우던 것이 값싸진다). `RepeatInterval`이 0이면 이 행은 울리지 않는다(반복으로만 나는 소음원이다) |
| `MonsterCall` | `ACL9Monster::FirePackAlarm` — **무리 경보가 실제로 나가는 자리** | `Instigator`는 그 몬스터 — **반경은 그 개체 행의 `PackAlarmRadius`**. 거위는 `AlertWindup`이 끝나 우는 순간이 곧 이 자리이고, 기상 예고(`WakeTelegraph`)는 경보가 아니라 링도 없다 |
| `MonsterPounce` | `ACL9Monster::EnterState(Pounce)` — 잡기 예고 진입 한 줄 (2026-09-20) | `Instigator`는 그 몬스터, 위치는 몬스터 발밑, `Radius` 0(판정 없음). 덮침·헛침에는 사건이 없다. 별칭 `monster_pounce`(`CL9Wave::ToAlias`가 enum 이름의 소문자 스네이크라 자동). `ValidateRows`의 `PackAlarm` 허용 집합(몬스터 소음원)에 든다 |

**몬스터 소음의 위치는 발밑이다 (2026-10-15 사용자 — "땅바닥에서 나와야 되는데 몬스터 가운데서 나온다").** 몬스터 셋(`MonsterProx`·`MonsterCall`·
`MonsterPounce`)은 `ACL9Monster::GetFootLocation()`(루트 구의 중심 − 반지름)을 보고한다. 예전엔 `GetActorLocation()`(구 중심, 닭은 바닥 위
약 40cm)이라 점이 몸 한가운데 높이에서 퍼졌다. 사람 소음은 처음부터 `ACL9Character::GetFootLocation()`(캡슐 바닥)이다.

**소유 클라의 예측**: 자기가 낸 소음은 서버 왕복을 기다리지 않는다. 발걸음은 짧은 간격으로
나므로 RTT만큼 늦으면 링이 걸음과 어긋나 보인다. 규칙은 둘뿐이다.

- 소유 클라는 자기 캐릭터가 내는 소음을 **로컬에서 먼저** `UCL9WaveRenderer::PlayWave`로 그린다.
  예측하는 것은 **클라가 스스로 아는 것**뿐이다 — 발걸음(`Run`·`Walk`·`Crouch`)·수레(`Cart`)·착지(`Land`).
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
죽으므로 월드 서브시스템이다 — 파면을 매 틱 키우므로 `UTickableWorldSubsystem`(2026-10-15). 레벨에 액터를 놓아야 하는 `ACL9NightLighting` 방식은 쓰지
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
8. **차폐 트레이스** — `OcclusionRays`개 트레이스로 광선별 벽까지 거리 배열을 만든다(막히지 않은 광선은 무한대).
   이 배열이 바닥 점의 각도·한계이자 벽 자국 자리다(아래 "벽 차폐"·"점 그림"). `cl9.shownoise` 로그의 광선 수가 이 배열이다.
9. **파면 등록** — 파면 하나(발원지·반경·속도·색·밝기·광선 거리)를 목록에 올린다. `MaxConcurrent`를 넘으면 가장 오래된
   파면부터 버린다. 이후는 매 틱 `UCL9WaveRenderer::Tick`이 파면을 키우며 점을 찍는다.
   **`DotSystem`이 비어 있으면 여기서만 멈춘다** — 위의 판정·트레이스·로그는 그대로 돈다(에셋이
   없어도 `cl9.shownoise`로 사건을 검증할 수 있게).

**주 — `MinInterval` 스탬프가 시각 게이트 앞으로 왔다** (2026-09-20). 옛 순서는 행 → 표시 시점 → `MinInterval` →
거리 컷 → 반경 → … → 스폰 직전 스탬프였다 — 링이 실제로 생길 때만 간격을 찍었다. 소리가 `MinInterval` 뒤·시각 게이트
앞에 있어야 하므로 스탬프도 그 자리로 옮겼고, 그 결과 **표시 시점·거리 컷에 걸린 사건도 간격을 찍는다**. 폭주 안전판의
뜻("같은 발신자·같은 행이 이 간격 안에 두 번 나지 않는다")에 더 맞고 링 그림에는 차이가 없다 — 정전 전용 행이 정전
밖에서 사건을 내면 그 사건도 간격을 소비한다는 것만 달라졌다.

### 점 그림 (2026-10-15 — CCL8 `updWaves` 이식)

파면은 보이지 않는 원이다. `Tick`이 파면마다 `Radius += Speed × dt`로 키우고 두 가지를 찍는다:

- **바닥 — 방사형 점선.** 파면이 `Dots.RingSpacing`(8.5cm)을 지날 때마다 그 반경에 점 한 바퀴를 찍는다. 점은 광선
  `StrideMine`/`StrideOther`개마다 하나, 그 광선의 벽 거리 안쪽에만 선다(벽 너머로 안 나간다). 점은 발원지 바닥 높이의 평면에
  찍히므로 바닥이 꺼지는 곳(내려가는 계단·구덩이)에서는 허공에 뜬다 — 그래서 생성 때 `TraceOcclusion` 이 광선마다 전역 `FloorProbeStep`
  간격으로 바닥을 더듬어(`TraceFloorEdge`) 발원지보다 `FloorDrop` 아래까지 바닥이 없는 첫 표본 앞에서 광선을 끊는다. 끊긴 광선은
  거리만 유한하고 법선이 영벡터라(`IsWallRay` 거짓) 바퀴살·마루는 거기서 멈추되 벽 자국·호는 서지 않는다(2026-10-22 사용자 "계단
  밑에도 파동이 퍼진다"). 오르막은 끊지 않는다 — 수평 광선이 먼저 턱을 벽으로 맞는다. **점은 움직이지 않고
  제자리에서 옅어지므로** 파면이 지나간 길을 따라 같은 각도에 점이 줄지어 남아 바퀴살이 된다. 고리마다 "파면이 그 반경을
  지난 시각"을 생일로 삼아 프레임 수와 무관하게 같은 점선이 선다(CCL8은 매 프레임 찍어 60fps에 묶여 있었다).
  밝기 = 감쇠 × `Dots.Brightness` × `Power`(× `OwnerPowerScale`) — 감쇠는 남의 파동 `1 − r/R`, **내 파동은 √(1 − r/R)**라
  발밑에서 멀리까지 읽힌다. `MinBrightness` 아래면 안 찍는다. 높이는 `FloorHeightMin~Max`에서 점마다 흔든다.
- **벽 — 바퀴살을 이어 올린 세로 점선.** 이번 틱에 파면이 지나간 **바퀴살 광선**(바닥과 같은 `Stride` 간격) 중 막힌 광선(벽 거리 ≤ 반경)마다
  한 번, 광선이 벽에 닿은 충돌점에서 **벽의 수평 법선 쪽으로 `StampInset`만큼 띄운 자리**에 `StampBaseHeight`부터 min(`StampHeight`, 벽
  꼭대기)까지 **`Dots.RingSpacing` ÷ 행 `StampDensity` 간격**으로 점을 세운다 — 바닥 바퀴살 점선이 벽에서 꺾여 같은 촘촘함으로 벽면을
  타고 올라간다(2026-10-16 사용자 "벽에 딱 붙어서 올라가야 하는데 공중에 올라간다" — 그 전의 높이 무작위 3~8점 구름은 공중에 뜬
  먼지처럼 읽혔다). `StampJitter`(0)가 있으면 벽 접선 방향(법선 × 위)으로만 흔든다. 광선 방향으로 당기면 비스듬한 벽에서 벽과 점 사이가
  벌어져 허공에 뜬다(2026-10-15).
  법선은 생성 때 차폐 트레이스가 광선별로 같이 담는다(`FLiveWave::RayNormals`, 수평 성분만 — 닿은 면이 거의 수평이면 광선을 되돌린
  방향). 차폐 트레이스와 꼭대기 재기는 **복잡 충돌(보이는 삼각형)**에 맞힌다(`bTraceComplex`) — 벽 조각의 단순 충돌체는 기둥까지
  감싼 40cm 슬래브라 단순 충돌로 재면 판벽보다 앞에서 멈춰 커튼이 떠 보였다(2026-10-15). 세기 = `Power` × (내 것이면
  `StampOwnerScale`) × (1 − 벽 거리 ÷ 반경)이 점 색의 밝기다. `bWallStamp`인 행만. **2026-10-22 — 표의 모든 행이 `StampDensity` 0 이라
  이 기둥은 꺼져 있다**: 광선마다 세운 세로 점선이 띠 여러 겹과 겹쳐 빗살 커튼으로 읽혔다(사용자 "이런 물결 매우 별로"). 벽에는 아래
  "벽 마루" 호만 선다 — `bWallStamp` 는 그 호의 벽 자리를 모으는 플래그로 계속 쓴다.
  **핑과 타고 넘기 (2026-10-16, [../goal/wave-landing-ping.md](../goal/wave-landing-ping.md))** — 행 `bUntilWall`이면 `PlayWave`가 같은
  발신자·같은 행의 지난 핑 시각(`LastPingTimes`)을 보고, `UntilWallCooldown`이 지났으면 반경을 `UntilWallRadius`로 바꾼다(차폐 광선 길이·
  밝기 감쇠·벽 자국 세기가 다 이 거리를 쓴다). 대기 중이면 보통 반경이다. 장부는 머신마다 받은 사건으로 따로 센다 — 같은 순서로 받으니
  결과가 같고, 사건 신호(`Multicast_Noise`)는 그대로다. 행 `bPassThroughBodies`이면 ① 차폐 광선이 사람(`ACL9Character`)·종류가
  아이템/몬스터인 `ACL9ActorBase`(수레 포함)에 막히면 그 몸을 빼고 다시 쏴 뒤의 벽까지 가고(한 광선 8개까지) ② 바닥 바퀴살 점마다
  파면이 지나는 틱에 `RideHeight`에서 아래로 물체 종류 광선(WorldStatic·WorldDynamic·Pawn·PhysicsBody, 복잡 충돌, 낸 사람 몸 제외)을
  쏴 가장 높은 몸의 꼭대기에 선다 — 몸이 아닌 면은 발원지 바닥 +60cm 안만 바닥으로 받아 문틀 윗부분·탁자 위에 올라앉지 않는다.
  ③ 한 바퀴살에서 높이가 한 점 간격보다 크게 **올라가면** 그 자리를 앞면으로 적는다(`FLiveWave::FaceRadius`·`FaceLow`·`FaceHigh`, 내려가면
  지운다) ④ 마루 점은 가장 가까운 바퀴살이 잰 높이에 서되, 그 바퀴살이 앞면을 만났으면 구가 그 면과 만나는 높이 `FaceLow + StampRiseScale ×
  √(마루 반경² − 앞면 반경²)` 까지는 **앞면 자리(앞면 반경)에 서서 호를 그리며 오르고**, 꼭대기(`FaceHigh`)에 닿은 뒤 위를 탄다 — 벽 마루와 같은
  그림이다. 두 칸 다 몬스터 행이면 무시한다(로드 검사 Error). **2026-10-22 — 바퀴살마다 앞면을 세로 점선으로 채우던 옛 ③은 지웠다**: 착지 핑이
  우리를 넘을 때 띠 위에 빗살처럼 섰다(사용자 "이런 물결 매우 별로").
  **실루엣 (2026-10-16 사용자 "파동으로 해당 물체의 실루엣을 알 수 있어야 돼")** — 바퀴살만으로는 어둠 속 사람에 선이 두세 줄뿐이라 형체가
  안 읽혔다(6° 간격 — 2m 에서 21cm). 행 `bRevealBodies`면 `StartReveals`가 반경 안·벽 너머가 아닌 몸마다(통과 대상과 같은 몸, 낸 사람 제외)
  보이는 메시 컴포넌트를 **한 겹 더 붙인다**(겉껍데기 — 스켈레탈은 `SetLeaderPoseComponent`로 원래 포즈를 따른다, 충돌 없음·그림자 없음·
  주인만 보기 플래그 복사). 겉껍데기는 `Reveal.Material`(`M_CL9_WaveReveal`, 조명 없는 더하기 — 월드 격자 점무늬, `FrontRadius` 안쪽만,
  법선 방향 `Push` 띄움)의 MID 를 입고, `TickReveals`가 매 틱 파면 반경을 넣어 몸이 앞쪽부터 켜지게 한 뒤 파면이 몸을 다 지나면
  `Hold`초 머물고 `Fade`초에 옅어져 떼어진다. 덧그림 재질 칸(피격 깜빡임·조준 윤곽선·조리 표시가 같이 쓴다)은 건드리지 않는다.
  **붙박이는 표가 켤 때만 (2026-10-22, `Reveal.bRevealFacilities`)** — 우리·조리대(`ECL9ActorKind::Facility`)의 가로대·기둥이 점무늬로 드러나면
  파동 위에 가로줄 세 개·세로줄이 서는 "빗살"로 읽혔다(사용자 "이런 물결 매우 별로" — 그 그림은 물결이 아니라 우리의 실루엣이었다). 사람·몬스터·
  아이템(달걀)은 늘 드러낸다. 타고 넘기(④)는 붙박이도 그대로 — 조리대 위를 띠가 타고 넘는 것은 깨끗하다.
  **어둠 속에서만 (2026-10-23, `Reveal.bDarkOnly`)** — 사용자 "밝을 때는 아니고 어두울 때만", "몸에 빛이 안 닿을 때". 몸 가운데(액터 경계 중심)에
  켜진 점·스포트 조명(손전등·헤드램프 포함)이 감쇠 반경 × `LitReach` 안에서, 스포트면 원뿔 안에서 가리는 것 없이(Visibility, 몸과 조명 주인은 빼고) 닿거나,
  `DirectionalLitIntensity` 이상인 해가 가리는 것 없이 닿으면 밝다 — 그 몸은 드러내지 않는다(`IsBodyLit`). 몸마다 따로라 같은 파동에서도 어떤 몸은 뜨고 어떤 몸은 안 뜬다.
  **빛줄 실루엣 (2026-10-23, 견주기용)** — 사용자 "실루엣이 파동이 지나가는 것처럼 쓰윽 보여 줬으면", "이건 봐야 알 듯". 벌의 `RevealMaterial` 이
  `M_CL9_WaveRevealLine`(`Tools/Unreal/create_wave_reveal_line_material.py`)을 가리키면 점무늬 대신 그 재질을 입힌다 — 파면 자리의 빛줄(`LineWidth`)이 몸을 훑고,
  지나간 쪽에는 프레넬 윤곽(`RimPower`·`RimAlpha`)이 `Hold`·`Fade` 동안 남는다. 벌 11 이 6번에 이것만 바꾼 벌이다.
  **커튼은 밑에서부터 차오른다 — 파동은 구다 (2026-10-22, 그 전 2026-10-16 은 일정 속도).** 수평 거리 d 인 벽면의 높이 h 인 점은 구의 반지름이
  √(d² + h²) 에 닿는 순간 켜진다 — 생일 = 파면이 벽 밑동에 닿은 시각(틱 안 보정) + (√(d² + h'²) − d) ÷ `Speed`, h' = (점 높이 − `StampBaseHeight`) ÷
  `Dots.StampRiseScale`(`UCL9WaveRenderer::WallBirth`). 닿는 순간 빠르게 솟다가 소리 속도로 느려져 물결이 벽에 튀는 모양이 되고, **기둥 꼭대기도 구가 닿는
  높이 √(판정 반경² − d²) 까지**라 가까운 벽은 높이, 반경 끝 가까이의 벽은 밑동만 적신다. 풀 업로드는 생일 전 점을 투명·크기 0으로 둔다. 수명은 각 점의
  생일부터라 사라질 때도 밑에서부터 옅어진다. `StampRiseScale` 0 이하면 한꺼번에 선다. 타고 넘는 앞면(④)의 마루 높이도 같은 규칙이다.
  **벽 마루 (2026-10-22 사용자 "벽·몬스터에 부딪혀 올라가는 것도 예쁘게").** 물결이 켜져 있고 행이 벽 자국을 찍으면, 생성 때 이웃한 두 광선이 같은 벽면을
  맞힌 구간(법선 코사인 0.95 이상·충돌점 사이가 광선 간격의 4배 안)을 `Ripple.CrestDotGap` 으로 나눈 **벽면 자리**(`FLiveWave::WallPoints`)를 만들어 두고,
  본 파면이 `CrestSpacing` 을 지날 때마다 자리마다 구가 그 자리에 닿는 높이 `StampBaseHeight + StampRiseScale × √(파면 반경² − d²)` 에 마루 점(`CrestLife`·
  `CrestSize`·`CrestBrightness`)을 세운다 — 바닥의 밝은 띠가 벽에서 꺾여 **호를 그리며** 타고 오르고, 굽은 몸(닭 몸통)도 이웃 광선끼리는 같은 면이라 몸을 감싸는
  호가 된다. 벽 꼭대기는 광선마다 처음 물을 때 한 번 잰다(`RayTop`, `MeasureWallTop` 캐시). 메아리는 벽에 오르지 않는다.
  벽 꼭대기는 충돌점에서 법선 반대쪽 1cm 안에서 `StampHeight` 높이부터 아래로 한 번 쏴서 잰다(`MeasureWallTop`, 자국 하나당 트레이스
  하나) — CL9 우리·울타리는 1.5m 안팎이라, 재지 않으면 벽 위 허공에 점이 흩날려 파편처럼 보였다(2026-10-15 사용자).
- **점 풀** — 점은 `Dots.PoolSize`칸 고리 버퍼에 쌓이고 넘치면 가장 오래된 칸을 덮는다. 매 틱 풀을 배열로 펴서(죽은 칸은
  알파 0·크기 0) 점 시스템에 넘긴다. 알파 = (1 − 경과 비율)^`FadeExponent`. 보이는 점이 하나도 없어지면 마지막으로 빈
  배열을 한 번 올리고 시스템을 멈추며(`SetPaused`), 파면도 점도 없으면 틱 자체가 쉰다(`IsTickable`).

### 물결 (2026-10-15 사용자 — "레이더나 물결처럼 예쁘게", 물결을 고름)

`Ripple.bEnabled`이면 바퀴살 위에 **마루**가 얹힌다. 본 파면(0번)과 그 뒤를 `EchoGap`(60cm)씩 따르는 **메아리 파면**들이
`CrestSpacing`(8cm)을 지날 때마다 짧게 사는(`CrestLife` 0.06초) 크고 밝은 점 고리를 떨군다. 고리는 광선 각도가 아니라 **둘레를
`CrestDotGap`(5cm) 간격으로 채워** 이어진 띠로 읽히고(차폐는 가장 가까운 광선의 벽 거리), 옅어지는 꼬리(0.06초 × 510cm/s ≈
31cm)가 앞이 밝고 뒤가 옅은 띠가 된다. 메아리는 바퀴살·벽 자국 없이 마루만, k 번째는 `EchoFalloff`(0.55)의 k 제곱만큼 옅다.
파면은 마지막 메아리가 반경 끝을 지날 때 끝난다. 반경·속도는 그대로다 — 마루가 멈추는 자리가 곧 판정 반경이다.

### 긴박도 (2026-10-15 사용자 — 체력·긴박도에 따라 퍼지는 모양이 달라야 한다)

파동마다 **긴박도 0..1** 하나를 재생할 때 정한다 — `CL9Wave::ComputeUrgency`(Data/CL9WaveTable). 행이 켠 입력들 중 **가장 큰 값**이고,
발신자가 그 입력의 종류가 아니면 그 입력은 0 이다. 전부 복제된 값이라 어느 머신에서 재도 같다.

| 입력 (행 스위치) | 값 | 출처 |
|---|---|---|
| `bUrgencyFromMonsterState` | 전역 `Urgency.StateUrgency[State]` (없는 상태는 0) | `ACL9Monster::GetState()` |
| `bUrgencyFromMonsterHp` | 1 − Hp ÷ MaxHp | `ACL9Monster::GetLostHpAlpha()` — `MaxHp` 는 Hp 를 굴린 값을 같은 자리에서 찍어 복제(2026-10-15 추가) |
| `bUrgencyFromCharacterHp` | 1 − Hp ÷ MaxHp | 발신자 `ACL9PlayerState` 의 `Hp`·`MaxHp` |
| `bUrgencyFromRage` | 광폭화 게이지 0..1 | `ACL9Monster::GetRageAlpha()` — **게이지 구현 전이라 늘 0**. 게이지([../plan/data/monster/common.md](../plan/data/monster/common.md))가 생기면 이 함수 하나에 잇는다 |

효과는 전역 `Urgency`에서 하나씩 켜고 끈다:

| 효과 (스위치) | 하는 일 | 어디서 |
|---|---|---|
| 색·밝기 (`bColor`) | 색을 행의 `UrgentColor` 쪽으로 긴박도만큼 선형으로 섞고, 밝기(바퀴살·마루·벽 자국)에 1 → `BrightnessMax` 를 곱한다 | `PlayWave` 7-1 |
| 물결 겹수 (`bEchoCount`) | 메아리 수 = `EchoMin` → `EchoMax` 반올림(끄면 늘 `EchoMin`). 물결이 꺼져 있으면 0 | `PlayWave` 7-1 |
| 흔들림 (`bJitter`) | 바퀴살·마루 점이 반경·접선 방향으로 최대 긴박도 × `JitterMax` cm 흐트러진다 | `DotPosition` |
| 펄스 빈도 (`bPulseRate`) | 반복하는 행(`RepeatInterval > 0` — `monster_prox`)의 간격 = RepeatInterval ÷ (1 → `PulseRateMax`). **호스트**에서 소나를 낼 때 정한다 | `ACL9Monster::HostTickProxNoise` |

반경과 파면 속도는 바꾸지 않는다 — 링 = 판정 반경(규칙 1), 소리의 속도는 하나(거리감). 확인(2026-10-15 PIE): 체력 1/3 의 달리기 파동은
긴박도 0.67·메아리 2·밝기 ×1.4 로 주황빛, 한 대 맞은 닭(잃은 체력 0.25)의 소나는 긴박도 0.25·메아리 2·간격 1.2 → 0.97초.
**추격·덮침 상태 입력은 PIE 에서 닭이 추격에 들어가지 않아 직접 보지 못했다**(같은 함수 경로라 체력 입력과 같이 돈다).

### 점 시스템 — `NS_CL9_WaveDots`

나이아가라는 **그리기만 한다.** C++이 넘기는 사용자 파라미터 넷:

| 파라미터 | 타입 | 무엇 |
|---|---|---|
| `User.PoolSize` | int | 풀 크기 — 활성화 **전에** 넣는다. 이미터가 첫 루프에 이 수만큼 파티클을 한 번 만들고 끝까지 쓴다 |
| `User.DotPositions` | Array Position | 점 위치 (월드) |
| `User.DotColors` | Array Color | 점 색 — RGB는 밝기를 곱한 색, A는 페이드 |
| `User.DotSizes` | Array Vector2D | 점 지름 (cm) — 0이면 빈 칸 |

- **이미터 `Dots`** (CPU, 월드 공간, **영구 ID 켬**, 경계 고정 ±1,000,000cm — 점이 맵 전체에 놓인다. C++도 컴포넌트에
  `SetSystemFixedBounds`로 같은 경계를 준다): 이미터 업데이트 `SpawnBurst_Instantaneous`(개수 = `User.PoolSize`, 루프 1회
  제한) — 이미터 상태는 무한 루프라 파티클은 판 내내 산다. `ParticleState`를 빼서 수명으로 죽지 않는다.
- **파티클 업데이트 Set Parameters 하나**: `Particles.Position` = `SelectPositionFromArray`, `Particles.Color` =
  `SelectLinearColorFromArray`, `Particles.SpriteSize` = `SelectVector2DFromArray` — 셋 다 샘플링 모드 Direct Set, 인덱스 =
  `Particles.UniqueID`, 배열 = 위의 `User.*`. 파티클 i가 풀 칸 i를 그린다.
- **렌더러**: 스프라이트, `FaceCameraPosition`, `Unaligned`, 그림자 끔, 재질 `M_CL9_WaveDot`(조명 없는 반투명 — 색 =
  파티클 색 RGB × 스칼라 파라미터 `EmissiveScale`(0.3), 불투명도 = 파티클 알파 × 반지름 0.5 원 마스크(경도 100%) = CCL8 `dotMat`의
  또렷한 원판). `EmissiveScale`은 밤 노출 고정·톤매핑을 거친 화면 색을 CCL8 점 화소에 맞추는 보정이다 — 값과 잰 근거는
  [../plan/data/wave.md](../plan/data/wave.md) `Dots` 아래 "밝기 보정".
- **만든 길 (2026-10-15)**: MCP `NiagaraToolset_System`. `AddUserVariables`로는 배열 DI 사용자 변수가 안 생긴다(엔진 로그
  "We do not support setting data interfaces in SetParameter_InternalUseOnly()") — **스택 입력을 아직 없는 `User.Dot*`에
  `SetStackInputData`(Linked)로 링크하면 에디터가 그 DI 사용자 파라미터를 만들어 준다.** 2026-09-16 "배열 DI 보류"는 이걸로 풀렸다.

**C++는 파라미터 이름과 조회 키만 안다.** 수치도 에셋 경로도 갖지 않는다.

### 모양 벌 (2026-10-22 사용자 "점점이 나가니까 보기 싫다 — 10개 정도 만들어서 골라 보겠다")

표 `Styles`(`FCL9WaveStyle` 배열)에 모양 한 벌씩 두고, **평소에는 표의 `DefaultStyle`**(벌 번호, -1 = 전역 점만) 벌로 그린다 —
2026-10-22 사용자 선택은 6번 "네온 띠 + 꼬리 셋"(`DefaultStyle` 5). **디버그 패널 "파동" 탭** 또는 `cl9.wavestyle N`(콘솔 변수, 0부터,
-1 = 표 기본값, -2 = 전역 점만)이 그 위를 덮는다 — 로컬 표현 스위치라 판정·복제에 닿지 않고, 살아 있는 파면도 다음 틱부터 그 벌로 그린다
(`UCL9WaveRenderer::ActiveStyle`, `CurrentStyle()`). 한 벌은 세 가지를 켜고 끄며 섞는다:

- **점** (`bDots` + 벌의 `Dots`·`Ripple`) — 위의 점 그림·물결 그대로, 값만 벌의 것. 벌의 `Dots.PoolSize` 는 읽지 않는다(풀은 판 시작 때 전역으로 한 번).
  `Dots.bSpokes`(새 열)를 끄면 바퀴살 없이 마루·벽 자국만 남는다.
- **네온 띠** (`bTube` + `FCL9WaveTube`) — 점이 아니라 **선**. 파면(발원지 중심의 구)이 지형과 만나는 선을 스플라인 메시 관
  (`USplineMeshComponent`, 엔진 원기둥 `/Engine/BasicShapes/Cylinder` 를 굵기 `Thickness` 로 눌러)으로 그린다.
  **2026-10-23 처음부터 다시 만들었다** (사용자 "땜질을 반복해서 망가졌다, 설계부터 다시"). 그 전 판은 점 광선(`OcclusionRays` 180개)의 멈춘 자리를
  이어 놓고 규칙을 덧댔다 — 이웃 거리로 끊으면 벽 앞에서 토막, 벽 점과 바닥 점을 곧장 이으면 가로등 기둥에서 V 자, 벽 밑동을 거쳐 ㄱ 자로 이으면 벽 끝에서
  수직 꺾임·가시가 났다. 지금 판의 원칙 셋 — 사용자가 고른 목표 그림(넓은 벽은 타고 오른다 · 얇은 기둥은 무시하고 지나간다 · 지형에 붙고 끝은 옅어진다)에서 나왔다:
  1. **띠는 제 광선으로 지형을 걷는다** (`MarchBandRay`, 광선 `Rays` 개, 점 광선과 따로). 보폭 `MarchStep` 마다 작은 공으로 바닥 높이를 쓸어내려 재고
     (`StepUp` 위 ~ `StepDown` 아래), 그 사이를 그 자리 바닥에서 `ProbeHeight` 위로 앞으로 쏜다. 앞이 막히면 **벽**(충돌점·수평 법선·그 앞 바닥 높이를 적는다),
     바닥이 `StepDown` 보다 깊게 꺼지면 **바닥 끝**, 반경 끝까지 가면 **열림**. 파면 바로 앞까지만 그때그때 걷는다(트레이스가 파면 수명에 퍼진다).
     계단·경사는 바닥 높이를 따라 띠가 오르내린다 — 바닥 끊김(`FloorProbeStep`·`FloorDrop`)과 따로다.
  2. **막는 것은 넓은 것뿐이다.** 앞 광선이 몸(사람·몬스터·아이템 — `IsPassThroughBody`)에 맞으면 그 몸을 빼고 다시, **얇은 것**에 맞으면 그 뒤에서 다시 쏜다.
     얇은 것 = 맞은 자리에서 벽면을 따라 양옆 `ThinWidth`/2 에 같은 물체(같은 컴포넌트 — 벽 조각 인스턴스가 이어진 것도 넓다)가 없는 것(`IsThinBandHit`,
     물체·인스턴스마다 한 번 재서 둔다). 가로등 기둥·말뚝·의자 다리는 없는 것처럼 원이 그대로 지나간다. 벽 끝 가까이를 맞혀도 한쪽은 벽이라 얇지 않다.
  3. **띠의 모양은 구의 기하가 정한다** (`BuildBandNodes`). 반경 R 에서 광선마다 — 아직 안 막혔으면 바닥 점(수평 R, 그 자리 바닥 + `Height`), 벽에 막혔으면
     **벽 위의 호 점**: 벽 밑동의 수평 거리 r 에서 구가 닿는 높이 √(R² − r²)(벽 꼭대기·`ClimbMax` 에서 눕는다, 벽면에서 굵기만큼 띄운다), 바닥 끝에 막혔으면 없음.
     높이가 수평 거리만으로 정해지므로 **안쪽 모퉁이에서 두 벽의 호가 저절로 이어지고**, 바닥 원이 벽면과 만나는 자리(선분 위 수평 거리 = R, `BandCrossing`)에서
     호의 높이가 0 이라 **바닥 띠와 벽 호가 밑동에서 정확히 만난다** — 곧장 잇기·ㄱ 자 같은 이음 규칙이 없다. 이웃한 벽 충돌점은 "이어진 한 면"일 때만 잇는다
     (광선 간격의 `SurfaceGap` 배 안이거나 비스듬한 같은 평면) — 벽 사이를 `WallSubdiv` 번 더 나눠 밑동 근처의 가파른 호를 살린다.
     이어지지 않는 자리(벽 끝·문틀 — 앞 벽 끝과 뒤 바닥, 바닥 끝, 그늘 가장자리)는 **끊고**, 끊긴 조각의 양 끝은 `EndTaper` 동안 굵기가 0 으로 줄어 **옅어지며** 끝난다
     (사용자 "지형에 붙고 끝은 옅어짐"). 벽 뒤 그늘에는 띠가 없다.
  관은 이음점마다 이웃 둘을 잇는 방향의 접선(길이 = 그 토막 길이 — 간격이 고르지 않아도 넘쳐 나가지 않는다)으로 곡선, 이웃 토막이 `CornerAngle` 보다 꺾이는
  이음점(바닥 → 벽 밑동)은 모서리로 꺾는다(곡선 접선이 튀어나가 가시가 되지 않게). `bClimbWalls` 를 끄면 벽 위의 점이 밑동 높이에 남는다.
  `Trails` 개의 꼬리 띠가 `TrailGap` 간격으로 뒤따르며 `TrailFalloff` 씩 옅고, 반경 끝에 닿으면 그 자리에서
  `Fade` 초 옅어진다(그동안 파면을 더 살린다 — `AdvanceWave` 의 ExtraRadius). 재질 `M_CL9_WaveTube`(조명 없는 더하기, Color·Alpha) 의 MID 를
  띠마다 하나. 관·MID 는 풀에 두고 매 틱 다시 놓으며 안 쓴 것은 숨긴다(`TubePool`·`TubeMaterialPool`). 액터 없이 월드에 직접 등록한 컴포넌트다.
- **빛 테** (`bDecal` + `FCL9WaveDecal`) — 발원지에 바닥으로 투영하는 데칼(`UDecalComponent`, 상자 X 가 투영축이라 −90° 눕힘) 하나. 재질
  `M_CL9_WaveDecal`(데칼 도메인·반투명, 표준 노드만 — 데칼 UV 의 중심 거리로 `Inner`~`Outer` 테, `Soft` 가장자리) 에 매 틱 반경(상자 반폭 = 반경 +
  `Width`)·밝기를 넣는다. 재질의 `Soft` 는 상자 반폭 기준이라 표의 `Soft`(테 두께 비율) × `Width` ÷ 반폭으로 환산한다. **벽을 모른다** — 벽 너머 바닥에도 비친다. 차폐가 필요한 그림은 점·네온 띠다. `DecalPool` 에 두고 안 쓴 것은 숨긴다.
- **벌의 색·속도** (`bColor`·`Color`·`SpeedScale`, 2026-10-23 사용자 "벌마다 색이나 스피드 같은 게 바뀌었으면") — `PlayWave` 가 파면을 만들 때 한 번 읽는다.
  색은 `bUseInstigatorColor` 행(사람이 낸 소리)의 기본 색만 바꾸고(몬스터·물건 행 색은 뜻이 있어 그대로), 슬롯 색·긴박도 색은 그 위에 얹힌다.
  속도는 `Wave.Speed` = 행 `Speed` × `SpeedScale`(`StyledSpeed`)이고 실루엣(`StartReveals`)도 같은 값을 쓴다. 이미 퍼지는 파면은 벌을 바꿔도 제 속도를 지킨다.

재질 둘은 `Tools/Unreal/create_wave_style_materials.py` 가 만든다. 벌 열 개의 값은 [../plan/data/wave.md](../plan/data/wave.md) 모양 벌.
벌 목록은 남겨 둔다 — `DefaultStyle` 하나로 평소 그림을 바꾸고, 패널에서 언제든 다른 벌과 견준다.

---

## 벽 차폐

소리가 지형을 속이면 안 된다 — [../plan/data/segkind.md](../plan/data/segkind.md)의
`bBlocksSight` 한 컬럼이 시야·파동·카메라를 한꺼번에 정하고, 구현은 **전용 트레이스 채널
`CL9_Occlusion`** 하나를 셋이 공유한다. 물통·상자처럼 일부러 시야를 안 가리게 해둔 낮은 조각은
그 채널에서 Ignore이므로 파동도 가리지 않는다.

**차폐는 C++이 한다 (2026-10-15 — CCL8 `castAll` 구조로 돌아왔다).** `OcclusionRays`개 광선을 발원지에서 수평(2D)으로
균등하게, `TraceHeight` 높이에서, 최종 반경까지 `LineTraceSingleByChannel(CL9_Occlusion)` — **생성 때 한 번**. 광선별 벽
거리(막히지 않으면 무한대)를 파면이 들고 다니며, 바닥 점은 그 거리 안쪽에만 서고 벽 자국은 파면이 그 거리를 지나는 틱에
찍힌다(위 "점 그림"). **이후 틱에는 트레이스가 없다.** (2026-09-16~10-14에는 링 파티클이 나이아가라 CPU Collision으로 스스로
벽에 멈췄다 — 배열 DI를 못 만들어 택한 우회였다.)

- **비용**: 파동 하나당 생성 때 `OcclusionRays`회 트레이스. 넷이 동시에 달리면 초당 (인원 × 걸음 빈도 × 광선 수) — 180광선 ×
  걸음 4회/초 × 4인 ≈ 2,900회/초. 광선 수는 반경 600cm 이하 180, 그 위 540(CCL8 `STEP_RAYS`·`WAVE_RAYS`)이다.

---

## 벽 자국

링이 벽에 닿은 자리에 세로 점 기둥이 한 번 찍힌다. 손전등이 닿지 않는 곳의 윤곽이 잠깐
드러나는, CCL8에서 가장 강한 그림이다.

- **구현 (2026-10-15)**: 파면이 막힌 광선의 벽 거리를 지나는 틱에 C++이 그 자리에 점 기둥을 한 번 찍는다(위 "점 그림" 벽).
  자국 점도 같은 풀·같은 시스템이고 수명만 길다(`StampLifeMin~Max` 2~2.5초). (2026-09-16 에셋은 닿은 링 점을 벽 밑에 남기는
  것뿐이었다 — 세로 기둥은 그 에디터 경로에서 안 그려져 보류했던 것이 이걸로 들어갔다.)
- 위치와 시각은 파면이 정한다 — 파면이 벽에 닿는 순간이 곧 자국이 서는 순간이다.
- 데칼은 쓰지 않는 쪽이 기본이다 — 수가 많고 수명이 짧아 데칼 풀이 금방 찬다. 굵고 오래 남는
  자국을 원하게 되면 그때 다시 본다.
- `bWallStamp` · `StampHeight` · `StampDensity`가 끄고 켜고 조절한다.
- 핑 행(`bUntilWall`)은 판정 반경 밖의 벽에도 자국을 세운다 — 세기는 상한 거리로 잰다(위 "핑과 타고 넘기").

---

## 플레이어 고유색

파동은 **`ACL9PlayerState`에 색을 하나 물어볼 뿐이다.**

```cpp
const ACL9PlayerState* PS = Cast<ACL9PlayerState>(Instigator);
FLinearColor Color = (Row.bUseInstigatorColor && PS != nullptr && !bMine)
    ? PS->GetSlotColor(Row.Color)   // 캐릭터 행이 아직 없으면 Row.Color로 떨어진다
    : Row.Color;
```

**슬롯 색은 남의 파동에만 씌운다 (2026-10-15 — CCL8 `spawnWave` D16 그대로).** 슬롯 색이 답하는 질문은 "다가오는 링이 동료인가
위협인가"이고, 내 발밑의 링은 그 질문이 없다. 내 파동은 행 색 그대로다 — 예전에는 내 파동에도 슬롯 색을 씌워 1인 판에서
발밑이 흰 점(슬롯 기본 색)으로 하얗게 탔다.

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
| `MaxConcurrent > 0` · `CullDistance > 0` | 상한이 없으면 파면이 프레임을 먹는다 |
| `Dots.PoolSize`·`RingSpacing`·`StrideMine`·`StrideOther` > 0 (2026-10-15) | 풀·간격이 없으면 파면이 아무것도 떨구지 못한다 — 표 전체의 문제라 행을 끄지는 않는다 |
| `OcclusionRays > 0` (2026-10-15) | 바닥 점은 광선 각도에 선다 — 광선이 없는 행은 그릴 자리가 없어 꺼진다 |
| (경고) `RadiusScale != 1.0` | 링이 판정 반경과 달라진다 — 에러는 아니지만 로그에 남긴다 |
| (경고) `DotSystem`이 비었다 | 판정·소리는 돌고 아무것도 안 그린다 |

---

## 성능 상한

- **동시 수**: 퍼지고 있는 파면을 `UCL9WaveRenderer`가 선입선출로 들고 있고, `MaxConcurrent`를 넘으면 가장 오래된
  파면을 버린다. **파티클 수는 `Dots.PoolSize`로 고정이다** — 점 시스템 하나가 판 내내 그 수만큼 파티클을 들고,
  점이 넘치면 풀의 가장 오래된 칸이 덮인다.
- **거리**: `CullDistance` 밖의 발원지는 트레이스도 파면도 만들지 않는다. 판정과 무관하므로
  안 그려도 게임이 달라지지 않는다.
- **트레이스**: 파동 하나당 `OcclusionRays`회, 생성 때 한 번.
- **틱**: 파면이나 보이는 점이 있을 때만 돈다. 매 틱 풀 전체(`PoolSize`칸)를 배열 셋으로 펴서 올린다 — 점이 다 사라지면
  시스템을 멈추고 틱도 쉰다. 2026-10-15 PIE(풀 32,768): 0.24초마다 달리기 파동을 4초 내는 동안 프레임 16.82ms(상위 5% 18.9ms)로
  파동 없을 때(16.77ms)와 같았다. 물결·긴박도를 넣은 뒤(풀 49,152, 체력 1/3 이라 메아리 2) 같은 측정도 16.75ms(없을 때 16.84ms) —
  에디터 60fps 상한 안의 측정이라 **상한 아래 여유는 아직 모른다.**

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
| **점 모양(크기·간격·수명·페이드·벽 자국 점)을 바꾸고 싶다** | 전역 `Dots` | 불필요 |
| **파면이 더 빨리·밝게** | `Speed` · `Power` | 불필요 |
| **바퀴살을 더 촘촘하게/성기게** | `OcclusionRays`(광선 수) · `Dots.StrideMine`/`StrideOther` | 불필요 |
| **물결(마루 띠·메아리)을 끄거나 모양을 바꾸고 싶다** | 전역 `Ripple` | 불필요 |
| **긴박도가 무엇을 볼지 / 무엇을 바꿀지** | 행의 `bUrgencyFrom*` 넷 · 행의 `UrgentColor` / 전역 `Urgency`의 효과 스위치 넷과 범위·`StateUrgency` | 불필요 |
| **발걸음 몇 번에 한 번 링을 낼지 바꾸고 싶다** | `StepsPerWave` (`run`·`walk`·`crouch`·`cart`). 0 이면 `StepDistance` 누적 | 불필요 |
| **n초에 한 번 (발 닿는 박자로) 내고 싶다** | `StepsPerWave` 1 + `MinInterval` n — 걸음마다 사건은 나되 렌더러가 간격 안의 것을 버린다. 걷기 링을 없애려면 `walk` 행 `bEnabled` 끔(청각은 그대로) | 불필요 |
| **내려가는 계단·구덩이 위로 점이 뜬다 / 문턱에서 끊긴다** | 전역 `FloorDrop` (끊김 깊이) · `FloorProbeStep` (표본 간격, 0 = 끊지 않음) | 불필요 |
| **파동 모양을 통째로 바꿔 보고 싶다 (점 / 네온 띠 / 빛 테)** | 표 `Styles` 에 벌을 더하고 디버그 패널 "파동" 탭이나 `cl9.wavestyle N` 으로 고른다. 바퀴살만 끄려면 `Dots.bSpokes` | 불필요 |
| **우리·조리대 실루엣이 거슬린다** | `Reveal.bRevealFacilities` | 불필요 |
| **뛰기·걷기·앉아 걷기의 파동을 서로 다르게 하고 싶다** | 세 행(`run`·`walk`·`crouch`)의 `Power`·`Color`·`OcclusionRays`·`bWallStamp`·`bRevealBodies`·`StepsPerWave` — 반경은 `CL9Rules`(`crouch` 만 `FallbackRadius`) | 불필요 |
| **벽·몸에 오르는 호의 높이·모양** | `Dots.StampRiseScale`(1 = 물리 그대로) · `Ripple.CrestDotGap`(호의 점 간격) · 행 `StampHeight`(상한) | 불필요 |
| **벽 자국을 끄고 싶다** | `bWallStamp` | 불필요 |
| **무거워서 줄이고 싶다** | `Dots.PoolSize` ↓ · `OcclusionRays` ↓ · `MaxConcurrent` ↓ · `CullDistance` ↓ | 불필요 |
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

## 옛 에셋 `NS_CL9_Wave` (2026-09-16 ~ 2026-10-14 — 지금은 아무도 안 가리킨다)

이미터 하나(`Ring`)가 점 583개를 한 번에 터뜨려 **점 자체가 바닥을 따라 날아가는** 끊김 없는 원 선이었다(당시 지적 "점이 아니라
원이 퍼진다" 반영). 벽은 나이아가라 CPU Collision으로 멈춘 점이 벽 밑에 커진 채 남는 것뿐이었고, 세로 커튼·밝기·알파 페이드는
그 에디터 경로에서 안 돼 빠져 있었다. 2026-10-15 사용자가 CCL8 웹판 녹화와 비교해 "밤티 같다"고 지적해 **CCL8의 점 그림으로
되돌렸다** — 위 "점 그림"·"점 시스템"이 지금 구조다. 에셋 파일은 지우지 않고 남겨 두었다(참조 0).

---

## 구현 시점과 선행 작업

파동 자체는 어느 스텝에도 매여 있지 않다(연출이라 어느 시점에 넣어도 게임이 안 깨진다).
다만 아래 셋이 먼저 있어야 한다.

1. ~~**`Niagara` 모듈 의존성**~~ — `CL9.Build.cs`에 `"Niagara"`가 들어갔다 (2026-09-16).
2. **`CL9_Occlusion` 트레이스 채널** — `Config/DefaultEngine.ini`에 `ECC_GameTraceChannel2`로
   들어갔다 (2026-09-16, 기본 Block). **조각 표의 `bBlocksSight`가 false인 종류를 그 채널에서
   Ignore로 두는 것은 맵 코드 몫이라 아직 없다** — 지금은 물통·상자도 파동을 막는다.
   시야 판정도 이 채널로 옮겨 갈 자리다.
3. ~~**나이아가라 시스템 에셋**~~ — `NS_CL9_WaveDots` + 재질 `M_CL9_WaveDot` (2026-10-15, 위 "점 시스템"). 그 전의
   `NS_CL9_Wave`(2026-09-16)는 위 "옛 에셋".

슬롯 색(`ACL9PlayerState::GetSlotColor()`)은 선행이 아니었다 — 파동과 함께 들어갔다.

---

## 참고

- 기획: [../plan/wave.md](../plan/wave.md) · 값: [../plan/data/wave.md](../plan/data/wave.md)
- 판정 반경: [../plan/data/rules.md](../plan/data/rules.md) 소리 · 몬스터 감각: [monster.md](monster.md)
- 경보 반경(`PackAlarmRadius`·`KillAlarmRadius`): [../plan/data/monster/common.md](../plan/data/monster/common.md)
- 차폐 목록: [../plan/data/segkind.md](../plan/data/segkind.md) `bBlocksSight`
- 정전·페이즈: [light.md](light.md) · 표현·복제 관례: [character.md](character.md) · [sound.md](sound.md)
- 위험 전달의 결: [warning.md](warning.md) · 화면 효과인 스캔 파동과의 구분: [ui.md](ui.md)
