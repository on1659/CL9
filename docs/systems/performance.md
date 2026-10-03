# 성능 — 예산·기준선·측정·규칙

> 프레임은 **재고 나서** 고친다. 기준선은 아래 표, 재는 법은 "측정" 절, 새 기능이 지킬 것은 "규칙" 절.
> 2026-09-21 첫 측정의 결론(**정지 장면 기준** — 몬스터가 잠든 채 거리를 둔 상태): 이 머신에서 GPU 여유가 있고 CPU 바닥 ≈12ms(75~80fps)다.
> 자동화 수치의 "3~8fps" 는 **에디터 창이 최소화·백그라운드일 때 3fps 로 스로틀되는 것**이었다.
> **사용자 보고(2026-09-21) "몬스터가 나올 때 프레임이 팍 떨어진다" 의 원인은 프레임률이 아니라 단발 스톨이다** — 첫 사용 순간의
> **소프트 참조 동기 로드**: 잡히는 첫 순간 카메라 셰이크 BP 클래스를 캐릭터 틱에서 로드(577ms + AddCameraShake 30ms = 694ms 히치,
> `stat dumphitches` 스택), 첫 파동 링의 나이아가라 로드(≈500ms, 프로세스당 1회), 첫 타격의 플래시 재질·퍼프, 종별 첫 스폰의 메시·ABP(35~40ms).
> 고침은 아래 "첫 사용 동기 로드 금지" 규칙 — `UCL9GameInstance::Init` 이 표의 소프트 참조를 전부 미리 올리고 `WarmAssets` 로 붙든다.
> 렌더 기본값(루멘·VSM·하드웨어 RT·Substrate·Nanite·TSR)은 바꾸지 않는다 — 실측 절감 ≤1ms 이고 루멘은 밤 조명의 전제다.

## 예산

| 항목 | 값 | 어디서 재나 |
|---|---|---|
| 목표 프레임 | **60fps(16.7ms)** 를 개발 머신 에디터 PIE·1클라·밤 실내에서 유지. p95 ≤ 25ms | 아래 "측정" 절차 |
| 회귀 문턱 | 기능 하나가 기준선 대비 **avg +1ms** 이상이면 원인을 적고(decisions.md) 조치하거나 예산을 새로 정한다 | 같은 자리·같은 뷰포트 크기로 재측정 |
| 게임 스레드(CL9 코드) | 틱·질의·UI 합쳐 **≤ 4ms**(4인 + 몬스터 최대 스폰 기준). 2026-09-21 실측: 7마리·1인 ≈ 4.9ms 중 애니 3.1ms | `stat dumpframe -ms=0.3` (도구가 남긴다) |
| GPU | 이 머신 1080p 급에서 **≤ 10ms** — 지금 4.8~6.2ms + 비동기 컴퓨트 2.5~3.6ms | `ProfileGPU` (도구가 남긴다) |
| 출시 최소 사양 | **미정** — 하드웨어 RT 루멘은 RT 지원 GPU 가 필요하다. Steam 게이트에서 정한다([../goal/steam-release.md](../goal/steam-release.md) §7) | — |

예산은 기준선이 아니라 상한이다. 예산 안이면 최적화하지 않는다 — 단순함 우선.

## 기준선 (2026-09-21, 커밋 0770f68 + 미커밋 09-20 실행분)

머신 RTX 3070 Ti · Ryzen 5 5600(6c/12t) · 16GB. 에디터 PIE(뷰포트 안, 1466×504) · 리슨 서버 1클라 · Epic · TSR · `Lvl_Henhouse`.
`Tools/Unreal/perf_probe.py` 로 조합마다 2초 안정화 뒤 5초 표본. 원본 JSON 은 `Saved/Perf/`(git 밖) — 아래 표가 기록이다.

| 장면 | avg / p95 (ms) | fps | GPU Graphics + 비동기 Compute (ms) |
|---|---|---|---|
| 로비(낮), 트럭 앞 | 16.4 / 21.3 | 61 | 6.2 + 3.6 (구름 0.5·후처리 0.9·루멘 씬 라이팅 1.3) |
| 밤, 트럭 앞 (부지 생성, 몬스터 7 잠듦) | 12.6 / 15.0 | 79 | 6.2 + 2.7 |
| 밤, 축사 실내 작업등 아래 (2900,−1455) | **13.0 / 15.2** | 77 | 4.8 + 2.5 |
| 같은 자리, `r.ScreenPercentage 200`(≈2932×1008, 1080p 급 픽셀) | 13.5 / 16.0 | 74 | — |

밤 실내에서 껐다 켠 것(avg ms, 기준 13.0): 루멘 끔 12.0 · VSM 끔 13.1 · 둘 다 끔 12.0 · 루멘 소프트웨어 RT 12.1 ·
볼류메트릭 안개 끔 12.9 · 화면 비율 50% 12.9 · `sg.*` Medium 13.5 · High 13.3. 200% 에서도 12.0~13.7 안.
→ **CPU 바닥에 붙어 있다.** 어떤 렌더 기능도 1ms 넘게 벌어 주지 않는다.

게임 스레드(밤·트럭 앞, 한 프레임 덤프): 틱 함수 61개 ≈ 4.9ms — 몬스터 스켈레탈 7개 ≈ 1.6ms, 플레이어 `ABP_Unarmed` ≈ 1.5ms.
`IpNetDriver Socket RecvFrom` 23ms 스파이크 1회(지켜볼 것). 렌더 스레드 ≈ 5.7ms 씬 + 1.1ms 프레임 끝. 나머지는 에디터(Slate) 몫 —
에디터 밖(Standalone)에서는 바닥이 더 낮다.

**스로틀 함정(가장 중요)**: 에디터 창이 최소화됐거나 포그라운드가 아니면 엔진이 **정확히 333ms/프레임(3fps)** 로 돈다 —
`UEditorPerformanceSettings.bThrottleCPUWhenNotForeground` 기본값 true. 최소화된 창은 이 값을 꺼도 스로틀된다.
지금까지 메모·문서의 "이 머신 PIE 는 3~8fps" 는 전부 이것이었다(첫 측정에서 모든 조합이 333.3ms 로 재현).
프로젝트 설정 `Config/DefaultEditorSettings.ini` 에서 이 값을 **False** 로 두었다(백그라운드에서도 PIE 가 제 속도 —
자동화·터미널로 알트탭 중에도; 에디터 재시작 뒤 적용). 이 클래스는 `config=EditorSettings` 라 `DefaultEditorPerProjectUserSettings.ini` 에 넣으면
안 읽힌다(엔진 헤더 확인). 사용자 계층은 `%LOCALAPPDATA%\UnrealEngine\5.8\Saved\Config\WindowsEditor\EditorSettings.ini` — 거기 같은 키가 있으면 그쪽이 이긴다. 창을 **최소화**하면 여전히 3fps 다 — 재거나 자동화할 때는 창을 복원해 둔다.

## "왜 내 PIE 만 느리지" 점검표

1. **에디터 창이 최소화·가려짐?** → 복원한다. 위 스로틀. 파이썬 자동화도 마찬가지(`SetWindowPos`/`ShowWindow(9)` 로 복원).
2. **다른 세션이 컴파일 중?** `cl.exe`·UBT 가 6코어를 65% 넘게 먹는다 — CPU 바닥 프레임이 그대로 늘어난다. `/cl9 status` 로 잠금을 본다.
   빌드 중에는 재지 않는다.
3. **PIE 클라 수가 2?** 창 둘이 각각 렌더·틱한다. 성능을 볼 때는 1, 협동 검증 때만 2.
4. **셰이더 컴파일 중?** 에디터 우하단 "Compiling Shaders (N)" — 새 재질·조합이 처음 보일 때 히치. 끝날 때까지 기다린다.
5. **메모리 16GB**: Claude 세션·파이썬·에디터·PIE 가 같이 쓰면 스왑 — 빌드가 느려진 사고가 있었다(work-progress 09-14). `Get-CimInstance Win32_OperatingSystem` 로 여유를 본다.
6. **뷰포트 크기**: 이 표는 1466×504 다. 전체 화면(1919×1032)이면 픽셀 2.7배 — 그래도 200% 실측이 13.5ms 라 GPU 는 여유.
7. **`t.MaxFPS`·VSync**: 에디터 기본 `t.MaxFPS 0`·VSync 끔. 스무딩(22~62)이 켜져 있으면 62 위로 안 오른다 — 정상.

## 측정

손으로 볼 때(게임 콘솔 백틱): `stat unit`(Frame/Game/Draw/GPU) → 어느 스레드가 크나 · `stat gpu` · `ProfileGPU`(창이 뜬다 — 로그로만 받으려면
`r.ProfileGPU.ShowUI 0`) · `stat dumpframe -ms=0.3`(로그 `LogStats`) · `stat scenerendering` · `stat niagara` · `stat anim`.

도구로(재현 가능한 기록을 남길 때):

```
py -3 .claude/hooks/cl9_lock.py acquire pie --minutes 30        # 단일 명령으로(cd && 로 이으면 훅이 못 본다)
call_tool EditorAppToolset.StartPIE {bSimulate:false, playMode:"PlayMode_InViewPort", warmupSeconds:6}
# 장면 만들기: 콘솔 cl9.night(부지·몬스터), 자리 이동은 cfg 의 teleport
# (선택) Saved/Perf/perf_probe_cfg.json — out·label·teleport·yaw·configs·dumpframe·settle_s·sample_s
py -3 Tools/Unreal/run_python.py Tools/Unreal/perf_probe.py     # 등록 즉시 돌아온다. 조합당 ≈8.5초
#   결과 Saved/Perf/perf_probe_<시각>.json 의 status 가 done 이면 끝 (configs[].avg_ms/p95_ms/max_ms/fps_avg)
py -3 Tools/Unreal/perf_parse.py --list                          # 로그 표식 줄 번호
py -3 Tools/Unreal/perf_parse.py C0_baseline --after <줄> --top 12   # GPU 파이프라인 총합·Scene 자식 상위
call_tool EditorAppToolset.StopPIE
py -3 .claude/hooks/cl9_lock.py release
```

- **스톨(히치)을 잡을 때는 `Tools/Unreal/perf_scenario.py`** — 같은 방식으로 PIE 안에서 등록하고 `Saved/Perf/perf_scenario_cfg.json` 의 `steps`(시각별
  `cmd`·`mark`·`teleport`·`kite`(몬스터가 붙으면 뒤로 순간이동해 잡히지 않고 접근을 계속 받는다)·`look_at_nearest`·`rpc`·`profilegpu`·`dumpframe`)를 돌리며 프레임
  시계열·구간 통계·스파이크(시각·거리·몬스터 수)를 남긴다. 요약은 `Tools/Unreal/perf_scenario_summary.py <결과.json>`. 스톨 원인은 `steps` 첫머리에
  `t.HitchFrameTimeThreshold 30` + `stat dumphitches` 를 넣고 로그 `LogStats: Thread Hitch` 의 게임 스레드 스택으로 읽는다(2026-09-21 S1~S7 이 이 방식).
- 조합 C0~C9 = 기본·루멘 끔·VSM 끔·둘 다·루멘 SW·화면 50%·안개 끔·sg Medium·sg High·루멘 SW+VSM 끔. D0~D8 = 화면 200% 에서 같은 것.
  새 조합을 넣으면 `RESTORE` 목록에 그 CVar 를 같이 넣는다 — 도구는 끝날 때 전부 원복하고 스로틀 플래그도 되돌린다.
- 같은 자리·같은 뷰포트 크기로 잰 것만 비교한다. 결과 JSON 에 `viewport`·`pawn_location`·`scene`(액터·광원·스켈레탈·나이아가라 수)이 있다.
- 자동화 PIE 의 프레임 수치는 창을 복원한 상태에서만 믿는다(위 함정). 파이썬 틱 콜백 자체는 표본당 수십 µs 다.
- 사람 육안 항목은 [../testing/pie-scenarios.md](../testing/pie-scenarios.md) "U. 성능 기준선".

## 규칙 — 새 코드·에셋이 지킬 것

측정 없이 최적화하지 않는다. 대신 아래는 **처음부터** 지킨다 — 나중에 재면 이미 늦는 종류다.

### 첫 사용 동기 로드 금지 (2026-09-21 — "몬스터 나올 때 렉" 의 원인)
- 표(셰이크·사운드·파동·rules·몬스터·캐릭터 행)의 소프트 참조를 **게임플레이 도중 처음 쓰는 자리에서 `LoadSynchronous` 하지 않는다.**
  틱·RPC·상태 전이 안의 동기 로드는 그 프레임을 로드 시간만큼(수십~수백 ms) 세운다 — 블루프린트 클래스(셰이크·ABP)는 에디터에서
  컴파일까지 붙어 더 길다(실측: `CS_HurtShake` 클래스 577ms, `NS_CL9_Wave` ≈500ms).
- 대신 `UCL9GameInstance::Init` 이 표를 받은 자리에서 전부 올리고 **`WarmAssets`(UPROPERTY 배열)로 붙든다** — 로드만 하고 붙들지
  않으면 다음 GC(에디터 PIE 는 30~60ms 히치로 60초마다) 에 풀려 첫 사용이 다시 콜드가 된다. 지금 올리는 것: 셰이크 행의 클래스, 사운드
  행의 음원, 파동 행의 나이아가라, rules 의 `HitFlashMaterial`·`HitPuffSystem`, 몬스터 행의 `SkeletalMesh`·`AnimBP`, 캐릭터 행의
  `SkeletalMesh`·`AnimBP`·`ArmsMesh`·`ArmsAnimBP`. 새 표·새 소프트 참조 열을 만들면 **같은 자리에 한 줄 더한다**.
- 사용처의 `LoadSynchronous()` 는 그대로 둔다(이미 올라와 있으면 즉시 반환 — 안전망). 밤 시작(부지 생성·아이템 스폰)은 전환 순간이라
  동기 로드가 허용된다(실측 최대 38ms).
- 나이아가라는 에셋을 올려 둬도 **첫 활성화**가 에디터 스크립트 컴파일(`Niagara - System - CompileScript`, ≈400ms, 프로세스당 1회)을 부른다 — 그래서
  `UCL9WaveRenderer::OnWorldBeginPlay` 가 판 시작 때 행마다 한 번 켰다 끈다. 새 나이아가라 표를 만들면 같은 식으로 판 시작에서 한 번 켠다.
- 남는 1회성 비용: 첫 `StartCameraShake` 의 인스턴스 생성 ≈30ms.

### 틱
- `Tick` 은 매 프레임 할 일이 있을 때만 켠다(`bCanEverTick`). 시간 뒤에 한 번 할 일은 타이머, 상태가 바뀔 때 할 일은 OnRep·델리게이트.
  지금 틱하는 클래스 9개(Character·Cart·FlashItem·Item·Kitchen·Monster·NightLighting·SiteMap·TruckPad)는 각각 이유가 있다 — 새로 더할 때 이유를 주석에 적는다.
- 틱 안에서 `TActorIterator`·`GetAllActorsOfClass` 는 **플레이어(≤4)** 처럼 상한이 작은 것만. 아이템·몬스터·`ACL9ActorBase` 전체를 매 프레임 훑지 않는다 —
  사건이 날 때(줍기·스폰·죽음) 캐시하거나 오버랩 이벤트를 쓴다. `TObjectIterator` 는 틱에서 금지(초기화·치트 전용).
- 표현 전용 틱(플래시·소리·발소리)은 상태 시각에서 파생해 **값이 바뀔 때만** 컴포넌트를 건드린다(`SetIntensity`·`SetVisibility` 매 프레임 호출 금지 —
  `ACL9SiteMap::SetLampsOn` 처럼 같은 값이면 나간다).
- 물리 아이템은 재운다(`CL9Item` 변위 기준 잠들기). 잠든 몸체에 매 틱 힘을 주지 않는다.
- 몬스터 감각(`SenseTick`)은 마리당 프레임당 시야 트레이스 1회(플레이어 수만큼) — 종류가 늘어도 이 틀을 넘기지 않는다. 몬스터 스켈레탈 애니가
  게임 스레드의 가장 큰 단일 항목(마리당 ≈0.23ms)이다: 새 종의 ABP 는 `BlueprintUpdateAnimation` 을 얇게(변수 복사만), 계산은 C++ 게터.

### 질의(트레이스·오버랩)
- 프레임당 트레이스 수를 **셀 수 있어야** 한다: 발생 사건 × 광선 수. 파동 `OcclusionRays` 처럼 표 컬럼으로 두고 balance 가 정한다.
- 반복 질의는 사건 단위로(파동은 **발생 때만** 광선을 쏘고 링 차폐는 나이아가라 Collision 이 한다 — [wave.md](wave.md) "비용").
- 휘두르기(2026-09-22): 판정 구간의 틱마다 구 스윕 1 + 걸린 살아 있는 몬스터 수만큼 가림 선 트레이스 + 표면 선 트레이스 1 회 / 넉백: 스텝(틱)마다 선 트레이스 1 회 — [character.md](character.md) 판정·[monster.md](monster.md) "타격 넉백".
- 넓은 반경 오버랩(`OverlapMultiByObjectType`)은 작은 반경·오브젝트 타입 필터와 같이. 조준(`ResolveFocus`)의 `PickRadius` 구가 상한 예.

### 광원
- 동적 그림자 광원은 **손전등·헤드램프·달(방향광)** 셋뿐. 시설 조명(작업등·모닥불)은 `SetCastShadows(false)`([light.md](light.md) 광원 목록) — 새 소품 빛도 같다.
- 켜진 램프 수는 `LampDensity`(사이트 행)가 정한다 — 모듈에 빛 소품을 더하면 이 비율이 총량을 잡는다. 깜박임(`TickFlicker`)은 켜진 램프만 돈다.
- 새 광원 종류를 넣기 전에 `stat gpu` 의 `Lights`·`ShadowDepths` 를 전후로 본다(기준 0.2~0.5ms).

### 이펙트(나이아가라·파동·재질)
- 나이아가라 시스템은 **고정 바운드**·`MaxConcurrent`·`CullDistance` 가 있어야 한다(파동 행 필수 컬럼 — [wave.md](wave.md) "체크리스트").
  CPU Collision 파티클은 수 × 수명이 곧 비용 — 파티클 수는 에셋에서 정하고 문서에 적는다.
- 재질은 Substrate 단순 슬랩(정점 색 → BaseColor) 틀을 유지한다 — 레이어·반투명·포스트프로세스 재질은 이유가 있을 때만.
- 후처리 재질(피격 비네트 등)은 화면 전체 패스다 — 하나로 합치고 알파 0 이면 끈다.

### UI
- 위젯 `NativeTick` 에서 월드를 훑지 않는다. 예외는 조리대(`ACL9Kitchen`, 판에 몇 개)처럼 상한이 작은 것 — 주석에 상한을 적는다.
  스캔은 스캔 순간 한 번만 훑는다(`WasRecentlyRendered` 로 먼저 거른다).
- 문자열 서식은 값이 바뀔 때만(`LastFocusPrompt` 비교 틀). `FText::Format` 매 프레임 금지.
- 저프레임 보정은 실시간(`FPlatformTime::Seconds`)이 아니라 월드 시간으로 — 스로틀 3fps 는 이제 없다.

### 네트워크
- 복제 액터는 `SetNetUpdateFrequency(20)`([../conventions.md](../conventions.md) §6). RPC 는 사건당 1개, Reliable 남발 금지(파동 [wave.md](wave.md) 네트워크).
- `IpNetDriver Socket RecvFrom` 스파이크가 재현되면(위 기준선 23ms 1회) 리슨 서버 넷 틱을 의심한다 — `stat net` 으로 본다.

### 에셋
- 스켈레탈 메시 삼각형·본 수·머티리얼 슬롯 수는 임포트 스크립트 `manifest.json` 에 남긴다 — 새 종은 닭 A 기준(현재 표는 [animation.md](animation.md) 보유 에셋)을 넘기지 않는다.
- 텍스처 ≤ 2K, 소품 스태틱 메시는 Nanite 켜진 채(프로젝트 기본) — 단 반투명·마스크 재질은 Nanite 가 못 그리므로 그 슬롯만 예외.
- 레벨에 두는 것은 `Lvl_Henhouse` 의 하늘·안개·바닥뿐 — 나머지는 생성기(`ACL9SiteMap`)가 짓고 부순다. 레벨에 손으로 소품을 쌓지 않는다.

### 회귀 게이트
- 틱·광원·이펙트·트레이스·위젯 틱을 **더하는** 실행은 보고서 "PIE 확인 항목" 에 밤 실내 avg 를 적는다(위 도구, 같은 자리). +1ms 넘으면 decisions.md 에 이유.
- 실행 하네스(`/cl9 task`)의 gameplay·map·ui 역할 brief 에 이 문서 "규칙" 절 링크가 들어간다.

## GPU 가 병목이 되면 — 레버 순서(밤 실내 실측 절감)

| 순서 | 레버 | 절감(ms) | 값을 치르는 것 |
|---|---|---|---|
| 1 | 스케일러빌리티 `sg.*` 한 단계(Epic→High) | ≈0 (지금은 CPU 바닥) | 옵션 화면으로 플레이어에게 넘길 수 있다 — [settings.md](settings.md) |
| 2 | 루멘 하드웨어 RT → 소프트웨어 (`r.Lumen.HardwareRayTracing 0`) | 0.9 | RT 없는 GPU 도 같은 경로 — 출시 최소 사양과 묶인다 |
| 3 | 볼류메트릭 안개 밀도·`r.VolumetricFog.GridPixelSize` | 0.1 | 손전등 빔 두께([light.md](light.md) 렌더 세팅) |
| 4 | VSM 끔 (`r.Shadow.Virtual.Enable 0`) | 0~0.5 (낮 로비 3.5) | 손전등 그림자 품질 |
| 5 | 루멘 끔 | 1.0 | **밤 조명이 무너진다** — `SkyLeak` 이 실내 앰비언트의 전부. 마지막 수단이 아니라 금지에 가깝다 |
| — | 화면 비율 | 0 (픽셀 바운드 아님) | 200% 에서도 +0.5ms |

CPU 가 병목이면(지금): 에디터 밖(Standalone·`-game`)에서 다시 재고, `stat dumpframe` 의 큰 항목(애니·틱·넷)부터.

## 몬스터 출현·접근 실측 (2026-09-21 밤, `perf_scenario.py` 시나리오 S1~S5, 에디터 PIE 1클라 1466×504)

| 시나리오 | avg ms | 스파이크 |
|---|---|---|
| 밤 시작(`cl9.night`: 부지 생성 + 닭 5 + 아이템 24) | 9.7 | 최대 38ms — 히치 없음 |
| **첫 파동 링**(`cl9.wave monster_prox`, 프로세스 첫 회) | 링 있을 때 +1.0 | **166 + 358ms** — 나이아가라 시스템 첫 로드. 새 PIE 에서는 없음(프로세스당 1회) |
| 종별 첫 스폰(`cl9.addmonster` 닭·거위·돼지·양) | +0.1 | 35~41ms(메시·ABP 동기 로드) |
| 추격(카이팅으로 잡히지 않게) 5마리 / 10 / 20 | 10.5 / 11.3 / 12.8 | 마리당 ≈0.15ms. 10마리 동시 스폰 98ms |
| 추격 중 토글: HWRT 끔 / VSM 끔 / 스켈레탈 RT 끔 / 손전등 켬·끔 | 9.7 / 10.0 / 9.9 / 9.6·9.8 | 전부 1ms 안 — 렌더 기능은 원인 아님 |
| **잡히는 첫 순간**(닭이 붙어 잡음 → 피격 셰이크) | — | **694ms 히치** = 캐릭터 틱 → `Flush Async Loading` 577ms(셰이크 BP 클래스 로드+컴파일) + `AddCameraShake` 30ms |
| 첫 타격(`cl9.hitmonster`) | — | 400ms 급 1회(같은 종류 — 플래시 재질·퍼프·셰이크) |
| 그 밖의 30~63ms 히치 | — | 전부 GC(`ConditionalCollectGarbage`) — 에디터 정상 범위 |
| **고친 뒤**(S7, 2차 빌드, 새 에디터·새 PIE) — 첫 링 / 첫 스폰 / 첫 타격 / 잡힘 / 죽음·드롭 / 거위·돼지 첫 스폰 | 11.1~11.5 | **31 / 25 / 17 / 17 / 16 / 43·17ms** — 나이아가라 컴파일 0.30s 는 PIE 시작 로그(`LogNiagara: Compiling System`)로 옮겨졌다 |

결론: 지속 프레임 저하는 없고, **첫 사용 동기 로드 스톨**이 "몬스터 나올 때 렉" 이다 → 위 규칙으로 Init 에서 미리 올린다. 재현·검증은 `stat dumphitches`
(`t.HitchFrameTimeThreshold 30`) 를 켜고 스톨 프레임의 게임 스레드 스택을 로그(`LogStats: Thread Hitch`)에서 읽는 것이 가장 빠르다.

## 미결·지켜볼 것
- `IpNetDriver Socket RecvFrom` 23ms 스파이크 1회(밤·트럭 앞·리슨 서버 1클라). 재현되면 넷 틱 조사.
- `ABP_Unarmed` `BlueprintUpdateAnimation` ≈1.3ms — 플레이어 한 명 값으로는 크다. 4인이면 5ms. ABP 변수 복사 경로를 얇게 할 후보.
- 출시 최소 사양(하드웨어 RT 루멘 유지 여부·`r.RayTracing` 퍼뮤테이션) — Steam 게이트.
- `Intermediate` 11GB·정식 빌드 4~15분은 이 문서 밖(빌드 시간은 [../conventions.md](../conventions.md) §7).
