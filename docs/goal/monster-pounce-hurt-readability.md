<!-- 원본: /autoplan 계획 2026-09-19 (~/.claude/plans/giggly-wishing-penguin.md) -->
# 잡히는 순간을 보이게 — 예고 + 피격 읽기 계획

**상태**: 구현 완료 2026-09-20(실행 monster-pounce-hurt-readability — decisions 09-20 "잡기 예고(`Pounce`) 도입"·"넘어진 동안 무적"·"잡히는 순간 카메라는 `UCameraModifier`"·"피격 방향 호"·"소리는 상태 전이 감지" 항목; PIE `docs/testing/pie-scenarios.md` C3-예고·C3·C3-다마리). 코드는 2026-09-21 현재 **미커밋**(HEAD 의 `CL9MonsterDef.h` 에 `Pounce` 열이 없다). 본문의 초안 잔재·TODO 는 지우지 않고 정정 표기만 덧붙였다.

## 배경

사용자(2026-09-19): "몬스터한테 맞을 때 몬스터가 때리는 모션을 내가 못 봐. 그래서 '엥 뭐지, 왜 맞았지?' 느낌이야."

코드로 확인한 원인 세 가지:

1. **잡기는 예고 없이 즉시다.** `Source/CL9/Monsters/CL9Monster.cpp:602-606` — Chase 틱에서 `Dist2D <= CatchRadius(84)` 이면 그 틱에 `HostCatch`(`:904`) → `Target.OnDamage` → `EnterState(Catch)`. 잡는 클립(`AN_Chicken_Catch` 0.8초 = `CatchPauseSeconds`)은 **맞은 뒤에** 재생된다(`animation.md:71` "판정이 상태 진입 순간이라 예비동작은 없다"). "때리는 모션"은 맞기 전에 존재하지 않는다.
2. **잡을 때 닭은 화면 밖(발밑)이다.** 눈높이 156cm, 닭 중심 36cm, 잡기 거리 84cm → 수평선 아래 **55°**. 월드 카메라 FOV 90°(`FirstPersonFieldOfView 70` 은 팔 메시 전용, `CL9Character.cpp:95-97`)·16:9 → 세로 반각 29.4° 라 피치 0 에서 닭은 **약 210cm 안쪽부터 프레임 밖**(`ChaseSpeed` 396 이면 사라짐→잡힘 0.3초). 84cm 에서 보려면 −26° 이상 내려다봐야 하고, 옆·뒤에서 오면 볼 수 없다.
3. **맞은 뒤에도 누가 때렸는지 알 길이 없다.** `CL9Character.cpp:1805-1848 OnDamage(Amount, Source)` 는 `Source` 를 로그(`:1836`)에만 쓴다. 넘어짐 카메라(`:674-677`)는 눈높이만 30cm 로 내리고 시선은 그대로(`LookInput :294` 에 넘어짐 검사 없음 — 시선은 자유). 잡히는 순간 소리도 없다(`SoundTable` 에 `Catch` 행 없음). 피격 방향 표시는 계획에도 없다(`ui.md:47` "글자도 아이콘도 없다").

목표: **맞기 전에 읽고(예고), 맞는 순간 무엇이 어디서 잡았는지 보이고 들리게** 한다. 두 검토 음성은 예고를 분리하자고 했으나 **사용자가 게이트에서 "예고도 지금 한 번에" 를 택했다**(모델 이견 1 기각) — 예고 값은 Codex 지적을 반영해 다시 잡았다(아래 A 절).

## 전제 다시 보기
- **원래 요청 "때리는 모션이 안 보인다" 의 직접 답은 예고 모션 추가지만, 두 검토 음성(Codex·Claude)이 모두 "피드백 먼저, 예고는 분리 실험" 을 권했다.** 이유: 예고는 판정 시점을 옮기는 **규칙 변경**이고(닭 396 = 걷기 396 이라 닭이 0.35초 멈추면 걷는 사람이 공짜로 이탈), 예고 없이도 카메라·방향·소리로 "왜 맞았지" 는 풀린다. → **모델 이견(최종 게이트에서 사용자가 결정)**.
- 2026-09-15 결정(`decisions.md:1218` "몬스터 예비동작 반려 — 판정을 늦춰야 해 규칙 2 위반")과의 관계: 규칙 2(`animation.md:9`)는 "애님이 판정에 못 들어간다" 이고, 값(`PounceWindup`)이 정하는 판정 지연은 규칙 2 와 충돌하지 않는다. 그래도 번복 여부는 2단계에서 사용자가 정한다.
- "못 본다" 의 진짜 원인은 예고 부재보다 **기하(발밑)** 다 — 예고를 넣어도 닭은 프레임 밖이라 카메라(피치)와 소리가 먼저다.
- 아무것도 안 하면: 잡힘은 "갑자기 빨개지고 주저앉음" 이라 위협을 학습할 수 없고, 닭이 1호 위협이라 밤 전체의 긴장이 운처럼 느껴진다(사용자 통증).

## 구현 범위 — A(예고) + B·C·D(피드백), 실행 하나

### B. 맞는 순간 카메라가 범인을 본다 — 카메라 전용 효과(조작·조준 불변)
- **피격 사건 구조체** `FCL9HurtEvent { float ServerTime; FVector_NetQuantize From; TObjectPtr<AActor> Attacker; uint16 Seq; }` 를 `ACL9Character` 에 복제(`COND_OwnerOnly`). `OnDamage` 가 `ServerTime = KnockdownStartServerTime`, `From = Source ? Source->GetActorLocation() : 자기 위치`, `Seq++` 로 채운다. 방향의 원천은 **위치**(`From`)라 공격자가 파괴·이동해도 그 순간의 방향이 남고, `ServerTime == KnockdownStartServerTime` 이 "이번 넘어짐의 사건" 을 보증한다(옛 사건 잔존 금지 — Codex eng 3). `cl9.damage` 는 가장 가까운 몬스터를 Source 로(`CL9CheatManager.cpp:409` 의 `nullptr`).
- **카메라 모디파이어** `UCL9HurtLookModifier : UCameraModifier`(소유 클라 `PossessedBy/BeginPlay` 에서 `PlayerCameraManager->AddNewCameraModifier`): `ModifyCamera` 가 `KnockdownElapsed ∈ [0, KnockdownTime)` 동안 POV 에 **피치 오프셋**(목표 `HurtLookPitch` −25°, 진입 `HurtLookTime` 0.45초 EaseOut — 도달 보장, `FInterpTo` 아님)과 **제한된 요 오프셋**(`From` 방향까지, 최대 `HurtLookYawMax` 60°)을 더하고, `GetUp`(마지막 0.6초) 동안 0 으로 되돌린다. **컨트롤 회전은 건드리지 않는다** — `LookInput` 게이트 없음, 조준 질의(`GetControlRotation` :590·969·1061)·플레이어의 원래 시선 그대로, 입력 복귀 시점 모순 없음(Codex eng 1·6). 스태미나 셰이크와 같은 "POV 만" 원칙(`animation.md:99`).
- 보이는 범위의 정직한 한계: 가로 FOV 90°(반각 45°) + 요 오프셋 60° = 공격자가 시선에서 **105° 안**이면 화면에 걸친다. 그 밖(등 뒤)은 카메라가 아니라 방향 호가 답한다(Codex eng 2). 피치 −25° 는 DownedZ(바닥 30cm)·닭 중심 36cm·거리 84~110cm 에서 닭이 화면 중앙 아래쪽에 오는 값 — 다른 눈높이·높은 몬스터는 balance 가 `HurtLookPitch` 로 조절.
- 1인칭 팔은 카메라 **컴포넌트** 자식이라 POV 오프셋을 안 따라간다 — 넘어짐 0~0.5초의 `FPKnockdown`(손이 얼굴 앞) 이 25° 위에 보인다. PIE 육안 항목; 어색하면 모디파이어가 팔 메시에 같은 오프셋을 주는 한 줄(TASTE 아님, 구현 판단).
- 재피격(넘어진 동안 다른 닭): `Seq` 가 바뀌면 요 목표만 새 `From` 으로, 피치 진입은 다시 시작하지 않는다(덜컹 방지). 죽음(`KnockdownEnd = ∞`): 진입 뒤 유지, 관전 전환 시 모디파이어 비활성. 폰 교체·`UnPossessed`: 모디파이어 제거.
- 값: `CL9Rules` `HurtLookTime`(0.45)·`HurtLookPitch`(−25)·`HurtLookYawMax`(60) — balance 초안. `HurtLookSpeed` 는 없앤다(EaseOut 고정 시간). **로드 검사**: `HurtLookTime + GetUp 0.6 ≤ CharacterDef.KnockdownTime`(1.5) — 아니면 진입·복귀 보간이 겹쳐 카메라가 떤다(Claude eng). `Attacker` 포인터는 `IsValid()` 로만 읽고(파괴 직후 pending-kill), 방향은 `From` 만 쓴다.

### C. 피격 방향 호 (HUD) — 표에서 끌 수 있게
- `WBP_HUD` 가장자리 빨간 호 `HurtDirection` 1개: `UCL9HUDWidget::GetHurtDirectionAngle()`(카메라 요 − 공격자 방향) 로 `RenderTransform.Angle`, 불투명도 `GetHurtFlash()`. 이번 넘어짐의 `HurtEvent` 가 없으면(치트 null·관전) 숨김.
- **정정**: 리썰 컴퍼니에는 방향 표시가 없다(비네트·킥·소리뿐) — Claude 음성 지적. 장르 톤과 어긋날 수 있어 `CL9Rules.bHurtDirectionIndicator`(기본 true)로 끈다. **디자이너 Collapsed 존중**: 기존 `RefreshHurtVignette`(`CL9HUDWidget.cpp:1167`)는 `Collapsed` 를 덮어쓰므로 그대로 복사하지 않는다 — `NativeConstruct` 에서 초기 가시성이 `Collapsed` 면 `bDesignerHidden` 으로 기억하고 이후 절대 만지지 않는다(HP 점과 같은 규칙). 각도는 `FRotator::NormalizeAxis(공격자 요 − 카메라 요)`, 0° = 화면 위(앞), 시계 방향 양수. 취향 결정(게이트).

### D. 소리 — 잡는 순간과 다가오는 소리 (규칙 불변)
- `SoundTable` 행 `MonsterCatch`(덮치는 꽥+퍼덕임, 합성): 몬스터 `Catch` 진입 에지에서 모든 머신 재생 — 창(elapsed) 이 아니라 **상태 전이 감지**: `State == Catch ∧ (지난 틱 State != Catch ∨ StateStartServerTime 변화)`. 200ms 창은 늦게 온 복제를 놓친다(Codex eng 4). 첫 틱(`bPrimed`)만 기준값. Catch 가 관측 전에 Stun/Dead 로 덮이면(같은 틱 피격) 그 잡기 소리는 빠진다 — 이미 `Hurt` 가 나므로 허용(문서화). 지금은 `Hurt`(맞은 몸 소리)만 있고 잡는 쪽 소리가 없다.
- `SoundTable` 행 `MonsterNear`(짧은 꼬꼬댁): 이미 있는 근접 소나 사건(`HostTickProxNoise :1108`, `monster_prox` 330cm·0.65초 — wave-impl)에 얹는다. 소나는 호스트가 `HostReportNoise` → `Multicast_Noise`(Unreliable) 로 퍼뜨리고 각 머신의 `UCL9WaveRenderer::PlayWave` 가 링을 그리므로, **`FCL9WaveInfo` 에 `FName SoundRow` 열을 추가**(파동 표 행마다 '이 소음이 나면 낼 소리', 기본 None)하고 `PlayWave` 가 행 활성·MinInterval 검사 뒤·시각 게이트(가시성·CullDistance·Niagara 에셋) 앞에서 `Row.SoundRow` 가 있으면 `CL9Sound::PlayAt` 한다 — `if (Source == MonsterProx)` 같은 코드 분기 없음(Claude eng: 데이터 드리븐 §1). `monster_prox` 행의 `SoundRow = MonsterNear`. 다른 소음원(달리기·깨짐)에 소리를 붙이고 싶으면 행만 채운다 — 소리는 링이 안 그려져도(정전 아님·컬링 밖) 난다, 행이 꺼지면 사건 자체가 없다(문서화). 새 사건·새 복제 없음, 링과 같은 박자. 리슨 호스트 이중 재생은 기존 가드(호스트는 자기 멀티캐스트를 버린다)가 막는다. 잠든 계열은 소나가 없으니 소리도 없다. 2D 거리 소나라 위층 닭도 들린다 — 층 검사는 wave 몫(기존 미결).
- 둘 다 `Tools/Audio/synth_sfx.py` 합성 → `import_sfx.py` → `S_MonsterCatch`·`S_MonsterNear`.

### 하지 않는 것
- 요 강제 회전 전면 적용(가장자리 걸치기까지만, TASTE).
- 3인칭/보디캠, 닭을 얼굴 높이로 뛰게 하기(모델·리그·콜리전 재작업 — 별건).
- (정정 2026-09-20) "넘어진 동안 무적" 은 원래 여기에 TODO 로 있었으나 **사용자 결정으로 이번 실행에 포함**됐다 — 아래 "넘어짐 무적" 절.

### 넘어짐 무적 (사용자 결정 2026-09-20 — 실행 중 추가)
- `ACL9Character::OnDamage` 머리(권한·HP 검사 바로 뒤)에 `IsKnockedDown()` 게이트 — HP·넘어짐 시계·`HurtEvent` 전부 불변, 로그 한 줄. **`cl9.damage` 도 같은 `OnDamage` 를 타므로 치트도 게이트에 걸린다**(우회 없음).
- 몬스터 쪽은 **진입 검사에서 건너뛰기**: Chase 틱의 잡기·예고 조건에 `!Target->IsKnockedDown()` — 넘어진 타깃엔 Pounce 도 Catch 도 시작하지 않고 추격(예산·놓침 시계·경로)은 그대로 이어져 타깃이 서는 순간 다시 예고. 타깃 선택(`SenseTick`)에서 빼지 않은 이유·이미 예고 중이던 닭의 처리(덮치되 재검사에서 헛침)는 `docs/systems/monster.md` 잡기 = 공격.
- 다마리의 긴장은 연타가 아니라 **재예고·마릿수**로 — 일어서는 순간 옆에서 기다리던 닭이 예고에 들어간다. 표 스위치는 없다(규칙).

### A. 잡기 예고 상태 `Pounce` — "덮치기 전에 몸을 세운다, 뒤로 한 발 물러나면 피한다" (사용자 결정 D1·D2)
- 새 상태 `ECL9MonsterState::Pounce`(enum 끝에 추가 — 모든 `switch` 에 case). Chase 틱에서 `Dist2D <= PounceRadius` 이고 `CatchCooldownRemaining <= 0` 이고 `PounceMissRemaining <= 0` 이면 `EnterState(Pounce)`: 이동 0, 매 틱 타깃을 향해 회전, `PounceWindup` 대기(`ChaseBreak`·`ChaseBudget` 시계는 계속 흐른다 — Pounce 는 Chase 의 부분 상태).
- 예고 끝: **타깃의 현재 위치** 방향으로 `PounceLunge` 만큼 한 번에 이동(Visibility 채널 스윕 — 벽이면 거기까지, feel-hit-kill 의 죽음 발사 트레이스 재사용) → **재검사**: `Dist2D <= CatchRadius` 이고 몬스터→타깃 Visibility 트레이스가 막히지 않으면 `HostCatch`(기존 그대로), 아니면 **헛침** — `bPounceMissed`(복제 bool) 세우고 `PounceMissRemaining = PounceMissCooldown`, Chase 복귀(원격 잡기 금지 — Codex CEO 4).
- **값 — 사용자 D2 "걷기로 물러나도 피한다"**: `PounceWindup` 0.6(사람 반응 0.25 + 방향 전환 0.15 + 여유), `PounceRadius` 130(예고 시작 — Catch 84 + 닭 한 걸음), `PounceLunge` 110. 걷기(396)로 뒤로 물러나면 0.6초에 238cm → 130+238 = 368 > 84+110 = 194 → **헛침**. 제자리·옆걸음(√(130²+238²)=271 > 194)도 옆으로 빠지면 헛침 — 즉 "예고 뒤 어느 쪽이든 걸어서 벗어나면 산다, 멈춰 있으면 잡힌다". 달리기 중 뒤에서 온 닭(추격 396 < 654)은 애초에 못 따라잡는다 — 기존 A1·A4 그대로. **닭이 훨씬 순해진다**(사용자가 감수) — 긴장은 `PounceMissCooldown` 0.6 뒤 즉시 재예고 + 다마리로 만든다. 값은 전부 balance 초안, PIE 뒤 조정.
- 표 열(`docs/plan/data/monster/common.md`·`UCL9MonsterDef`): `PounceRadius`(cm) · `PounceWindup`(s) · `PounceLunge`(cm) · `PounceMissCooldown`(s). 로드 검사: `PounceRadius >= CatchRadius`, `PounceLunge >= 0`; `CatchRadius == 0`(거위) 또는 `PounceWindup == 0` 이면 Pounce 없음(옛 즉시 잡기). 종별 값은 balance(너구리·돼지·양은 닭 비율).
- 예고 중 `OnStrike/OnShove`: Stun 이 이긴다(예고 취소, `CatchCooldown` 그대로 → 경직 뒤 다시 예고) — 반격 창. 예고 중 타깃 소실(시야·죽음): Chase 의 놓침 규칙으로.
- 애님: `AN_Chicken_PounceWindup`(0.6초 원샷 — 몸을 뒤로 젖히고 날개를 벌리는 "덮치기 전 자세", `Tools/Blender/animate_creature_reacts.py` 표에 행, 헤드리스) → 덮침은 `Catch` 클립 그대로, 헛침은 `Catch` 클립 앞 0.3초(`bPounceMissed` 로 ABP 가 고른다). ABP `ABP_Chicken_A` 는 상태 진입 시각 평가기 체인에 가지 하나(리드 MCP). 다른 종은 자기 `Catch` 첫 프레임을 유지(클립 없이 동작).
- 소리·파동: `SoundTable` 행 `MonsterPounce`(꽥+퍼덕임) 를 Pounce 진입 전이에서 모든 머신(D 의 Catch 와 같은 감지). `WaveTable` 행 `monster_pounce`(`ECL9NoiseSource::MonsterPounce`, `HostReportNoise` 한 줄, `SoundRow` 는 비움 — 소리는 상태 전이가 낸다) — 링이 "뒤에서 온다" 를 바닥에 그린다.
- 다마리: 예고가 겹치면 각자 예고·각자 헛침. ~~넘어진 동안 다른 닭의 덮침은 지금처럼 맞는다(무적은 TODO — 사용자 결정 대기)~~ → **넘어진 동안 무적**(2026-09-20 사용자 결정, 위 "넘어짐 무적" 절) — 다른 닭은 옆에 서서 기다리다 일어서는 순간 예고.
- `WaveTable.monster_pounce` 행의 값(`FallbackRadius`·`Thickness`·`Color`·`Power`·`OcclusionRays`·`MinInterval`)은 **balance 초안**이다(사용자 결정) — balance 가 채우기 전까지 `docs/plan/data/wave.md` 의 "(balance 초안)" 자리 그대로. `RadiusFrom None` 이라 `FallbackRadius > 0` 이어야 행이 산다.

#### A 의 구현 계약 (Codex 기술 재검토 반영 — 구현자가 지킬 것)
- **전이**: `EnterState(Chase)` 는 추격 예산·시계를 리셋하므로 헛침 복귀는 그것을 쓰지 않는다 — Catch 가 지금 하는 직접 쓰기(`State = Chase; StateStartServerTime=…; ChaseRepathRemaining=0; ApplyState()`, `CL9Monster.cpp:626-629`)를 **`ResumeChase()` 헬퍼**로 뽑아 Catch·Pounce 헛침이 같이 쓴다(예산·놓침 시계 보존). Pounce 틱은 **덮치기 전에** Chase 의 탈출 조건(타깃 소실·`DeaggroRadius`·`HomeRange`·`ChaseBudget`)을 먼저 평가하고, Dead·Stun 은 예고를 취소한다. Rage 진입은 진행 중인 Pounce/Stun 과 두 쿨다운을 건드리지 않는다. **타깃 고정**: Pounce 진입 때 `PounceTarget`(TWeakObjectPtr)을 잠그고 `SenseTick` 의 재타깃팅은 예고 동안 무시 — 덮침은 그 타깃의 현재 위치로.
- **결과 복제**: 지속 bool 대신 **`PounceResultServerTime`(float, 복제)** + **`bPounceHit`(복제)** — 헛침이면 ABP 가 `GetServerTime() − PounceResultServerTime < 0.3` 동안 `Catch` 클립 앞부분을 평가기로 튼다(농부 휘두르기와 같은 시계 방식), 연속 헛침도 시각이 갱신되니 구분된다. 성공 소리(`MonsterCatch`)는 Catch 전이에서만, 헛침엔 `MonsterPounce` 뒤 소리 없음. 늦게 접속한 클라는 `bPrimed` 규칙으로 무음.
- **덮침 이동**: Visibility **선** 트레이스가 아니라 **몸체 구 스윕**(`SweepSingleByObjectType`, WorldStatic + WorldDynamic — 바닥·문 조각 프로필이 제각각이라 둘 다) 으로 `PounceLunge` 만큼 수평(Z 평탄) 이동, 자신·타깃·캐릭터·아이템·몬스터는 무시 목록에 넣는다(죽음 발사의 "4회 재시도·통과" 방식은 쓰지 않는다 — 닭 무리가 벽을 가릴 수 있다). 실제 도달 위치로 `HeadingYaw` 동기·경로 무효화. 재검사는 **도달 위치 기준** `Dist2D <= CatchRadius` **그리고** `|ΔZ| <= Radius + 캡슐 반높이`(위층 타깃 배제) **그리고** 몬스터 중심→타깃 중심 Visibility 선 트레이스가 막히지 않을 것.
- **회피 산수의 정직한 서술**: 예고 진입은 130cm 이하 어디서든 일어나므로 이상적 후퇴 소요는 최대 `(194−130)/396 = 0.16초` + 반응 0.25 → 0.6초 안에 여유가 있다. 다만 가속·지연·장애물·타깃 전환은 보장을 깎는다(PIE 로 잰다). 헛친 뒤 간격은 ≈258cm 라 같은 속도로 물러나는 사람에겐 **다시 예고할 수 없다** — "0.6초 뒤 재예고" 가 아니라 "간격이 다시 좁혀지면(멈추거나 돌면) 재예고". 예고 중 회전은 `MoveAlongPath` 의 회전 저항(`TurnRate`·`TurnDrag`)을 우회하므로 **예고 회전도 `TurnRate` 로 제한**한다(닭이 제자리에서 홱 돌지 않게).
- **시간·복제**: 20Hz 는 복제 주기이지 AI 틱이 아니다 — Pounce 진입·결과 시각을 찍은 뒤 `ForceNetUpdate()` 로 즉시 보내고, `net.PktLag 100` 에서 클라가 보는 예고 길이가 0.5초 이상인지 잰다. `PounceMissRemaining` 은 틱마다 한 번만 감소, `KillCooldown` 은 헛침·취소에 영향 없음. 로드 검사는 유한·비음수, `CatchRadius 0` 은 "잡지 않는 종" 유지.
- **다마리·성능**: 3마리 동시 예고·덮침, 예고 만료 순간의 피격, 타깃 사망·접속 끊김, 동적 문, 높이 차, 연속 헛침을 PIE 항목에; 스윕·재경로·소음 브로드캐스트가 몰릴 때 `stat game` 프레임.

### 성공 기준 (PIE 육안)
잡힌 뒤 "무엇이 어디서" 를 말할 수 있는가 · 예고를 보고 뒤로 걸어 실제로 헛치게 만들 수 있는가 · 3마리 동시에 예고가 읽히는가.

## 이미 있는 것 (재사용)
| 부분 문제 | 기존 코드 | 재사용 |
|---|---|---|
| 상태 에지에서 소리 | `CL9Sound::PlayAt` + `TickHitFeel` 상승 에지(`CL9Monster.cpp:265`) | `Catch` 에지 하나 추가 |
| 다가오는 소리의 박자 | `HostTickProxNoise`(`:1108`)·`monster_prox` 행·`Multicast_Noise`·`PlayWave` | `PlayWave(MonsterProx)` 에서 `MonsterNear` 재생 |
| 넘어짐 카메라 | `:669-678 DownedZ FInterpTo`(컴포넌트 Z) · 스태미나 셰이크(POV 만) | POV 오프셋은 `UCameraModifier` 로 — 컨트롤 회전 불변 |
| 피격 페이드 | `GetHurtFlash()`(`:1542`) | 방향 호 불투명도 |
| 표 열 | `CL9RulesDef`(`HitStopTime` 등 HitFeel 카테고리) | 4열 + bool 1개 |
| 소리 합성·임포트 | `Tools/Audio/synth_sfx.py`·`Tools/Unreal/import_sfx.py` | 파일 2개 |
| HUD 옵션 위젯 | `HurtVignette` BindWidgetOptional 패턴 | `HurtDirection` 도 Optional |
재구축 없음. 새 RPC·멀티캐스트 없음(`HurtEvent` 구조체 복제 1개).

## 12개월 뒤 이상적인 상태
```
지금                             이 계획                                12개월 뒤 이상
잡기 = 즉시 판정, 맞은 뒤 클립,    피격 사건(HurtEvent) 복제 공통층,     종별 공격이 예고→판정 틀 위에(2단계),
카메라 Z 만, 잡는 소리 없음,       카메라가 발밑을 봄, 방향 호(끌 수 있음),  피격 방향·범인·예고 소리는 공통층,
다가오는 소리 없음           --->  잡는 소리·다가오는 소리        --->     표로 예고 길이/사거리 튜닝
```
델타: `HurtEvent` 공통층과 "상태 에지 소리" 가 이 계획에서 생긴다. 2단계는 이 위에 상태 하나를 얹는다.

## 구현 대안 비교
```
대안 A: 피드백만 — 카메라 피치·방향 호·잡는 소리·다가오는 소리 (1단계 = 권장)
  노력: S (사람 ~1.5일 / 클로드 ~40분)   위험: 낮음   완전성: 7/10 ("왜 맞았지" 해결, "어떻게 피하지" 는 2단계)
  장점: 규칙 불변 · 새 인프라 없음 · 무엇이 불만을 줄였는지 분리해 잴 수 있다
  단점: 예고가 없으니 "피할 수 없다" 는 남는다 · 방향 호는 장르 톤 검토 필요
대안 B: A + 예고 상태(Pounce) 한 번에
  노력: M (사람 ~4일 / 클로드 ~2시간 + 클립)   위험: 중~높음 (밸런스 규칙 변경)   완전성: 9/10
  장점: 읽고 피하는 공격 · 종별 확장 틀
  단점: 닭 396 = 걷기 396 규칙과 정면 충돌 · 0.35초/Reach 200 초안은 "이미 도망치던 사람만 산다" · 무엇이 효과였는지 못 가른다(두 음성 공통 지적)
대안 C: 시점 자체를 바꿈(3인칭/보디캠/닭 점프)
  노력: XL   위험: 높음   완전성: 8/10 — 2026-09-01 1인칭 확정 번복·팔 클립 전부 재작업
```
**모델 권고: A 먼저, B 의 예고는 뒤에. → 게이트에서 사용자가 B(한 번에)를 택했다(User Challenge 1 기각, D2 = 걷기로 물러나도 피한다).** A 절의 값은 Codex 지적(0.35초·Reach 200 은 대응 불가)을 반영해 0.6초·Lunge 110 으로 다시 잡았다.

## 검토 모드: 선택적 확장
수용(blast radius 안·<1일 CC): `cl9.damage` 에 `LastAttacker` 채우기(치트 한 줄), `MonsterNear` 를 기존 소나 사건에 얹기(표 행 하나). 보류(TODOS): ~~넘어짐 중 무적~~, 종별 예고 클립, ~~2단계 Pounce~~. (정정 2026-09-21 — 넘어짐 중 무적은 2026-09-20 사용자 결정으로 이번 실행에 포함·구현됐다(위 "넘어짐 무적" 절, decisions 09-20); 2단계 Pounce 도 게이트에서 합쳐졌다(아래 "이번에 하지 않는 것"). 남은 보류는 종별 예고 클립만.)

## 구현 중 마주칠 결정 (미리 정한 것)
- ~~첫 시간(기초): `LastAttacker` 는 `TObjectPtr<AActor>` + `COND_OwnerOnly`(UE 복제는 `TWeakObjectPtr` 를 직접 안 받는다) — 파괴되면 null 이 온다. `KnockdownStartServerTime` 과 같은 `GetLifetimeReplicatedProps` 블록.~~ **초안 잔재** — 최종은 B 절의 `FCL9HurtEvent{ServerTime, From, Attacker, Seq}`(결정 7). 단, "`Attacker` 는 `IsValid()` 로만 읽고 방향은 `From`" 은 그대로다.
- ~~2~3시간(핵심 로직): 피치 보간이 `bUsePawnControlRotation` 카메라와 싸우지 않게 `SetControlRotation` 만 쓰고, `LookInput` 무시는 함수 머리에 게이트 한 줄.~~ **초안 잔재** — 최종은 `UCL9HurtLookModifier`(POV 오프셋만, 결정 6): **컨트롤 회전 불변·`LookInput` 게이트 없음**. 복제 순서 역전(사건이 시계보다 늦음)의 처리는 유효하다: 피치는 넘어짐 시계만으로 시작하고 요/호는 사건이 있을 때. (아래 "데이터 흐름" 그림과 "검증 (PIE)" 자동 항목의 `LastAttacker`·`LookInput 게이트` 도 같은 초안 표기다 — 최종 이름은 `HurtEvent`·`HasHurtDirection()`·`GetHurtDirectionAngle()`, 게이트 해제 시각은 없다.)
- 4~5시간(통합): `Catch` 에지 소리 — `TickHitFeel` 의 창은 `Stun|Dead` 라 `Catch` 는 별도 창. 잡힌 쪽 `Hurt`·잡는 쪽 `MonsterCatch` 가 같은 순간 겹치므로 Concurrency 4 안. `PlayWave` 는 `bPredicted`(호스트 자기 것)와 멀티캐스트 수신 둘 다 지나므로 리슨 호스트 이중 재생 가드가 이미 있는지 확인(wave-impl 은 호스트 멀티캐스트를 버린다 — 소리도 같은 가드 안에서).
- 6시간 이후(마무리): 2인 PIE 상대 화면(호는 소유자만, 소리는 전원), 늦게 접속한 클라 첫 틱 무음(`bPrimed`), 관전 카메라에선 호 숨김.

## 바꿀 파일
| 파일 | 변경 |
|---|---|
| `Source/CL9/Data/CL9Types.h` | `ECL9MonsterState::Pounce`(끝) · `ECL9NoiseSource::MonsterPounce` |
| `Source/CL9/Data/CL9MonsterDef.{h,cpp}` | `PounceRadius`·`PounceWindup`·`PounceLunge`·`PounceMissCooldown` + 로드 검사 |
| `Source/CL9/Monsters/CL9Monster.{h,cpp}` | `ResumeChase()` 헬퍼 · Chase→Pounce(타깃 고정·TurnRate 회전)→구 스윕 덮침→재검사→`HostCatch`/헛침 · `PounceResultServerTime`·`bPounceHit`(복제, `ForceNetUpdate`) · `PounceMissRemaining` · Stun/Dead 우선 · Pounce/Catch 전이 소리 · `HostReportNoise(MonsterPounce)` |
| `Tools/Blender/animate_creature_reacts.py` · `Tools/Unreal/reimport_creature_clips.py` | `AN_Chicken_PounceWindup` 0.6초 |
| `Source/CL9/Character/CL9Character.{h,cpp}` | `FCL9HurtEvent HurtEvent`(복제 OwnerOnly, `OnDamage` 가 채움) · `GetHurtEvent()`·`GetHurtDirectionAngle()`·`HasHurtDirection()`(이번 넘어짐 사건인지 검사) · 소유 클라에서 모디파이어 등록/해제 |
| `Source/CL9/GameMode/CL9HurtLookModifier.{h,cpp}`(신규) | `UCameraModifier` — POV 피치·요 오프셋(EaseOut 진입·GetUp 복귀), 컨트롤 회전 불변 |
| `Source/CL9/Data/CL9RulesDef.h` | `HurtLookTime`·`HurtLookPitch`·`HurtLookYawMax`·`bHurtDirectionIndicator` |
| `Source/CL9/Data/CL9SoundTable.h` | `CL9SoundNames::MonsterPounce`·`MonsterCatch`·`MonsterNear` |
| `Source/CL9/Data/CL9WaveTable.h` · `Source/CL9/FX/CL9WaveRenderer.cpp` | `FCL9WaveInfo.SoundRow`(FName) 열 · `PlayWave` 가 시각 게이트 앞에서 `SoundRow` 재생 |
| `Source/CL9/GameMode/CL9GameInstance.cpp` | `Init` 에서 `SoundTable` 행을 미리 `LoadSynchronous`(첫 재생 콜드 로드 끊김 방지 — Codex eng) |
| `Source/CL9/UI/CL9HUDWidget.{h,cpp}` | `HurtDirection` BindWidgetOptional + 각도·표시 |
| `Source/CL9/GameMode/CL9CheatManager.cpp` | `Damage()`(`:409`) → 가장 가까운 몬스터를 Source 로(없으면 nullptr 그대로) |
| `Tools/Audio/synth_sfx.py`·`Tools/Unreal/import_sfx.py` | `monster_pounce.wav`·`monster_catch.wav`·`monster_near.wav` |
| 에셋(리드) | `CL9Rules` 4행 · `MonsterTable` 4열(닭·너구리·돼지·양) · `SoundTable` 3행 · `WaveTable.monster_prox.SoundRow` + `monster_pounce` 행 · `ABP_Chicken_A` Pounce 가지 · `WBP_HUD` 호 이미지 |
| 문서 | `docs/systems/character.md`(피격 절) · `monster.md`(상태 기계 Pounce·잡기 절·타격의 표현 표) · `animation.md`(몬스터 원샷에 PounceWindup) · `plan/data/monster/common.md`·`chicken.md`·`raccoon.md`·`pig.md`·`sheep.md` · `ui.md` · `sound.md` · `wave.md`(소나 = 소리) · `plan/data/rules.md` · `pie-scenarios.md` C3(갱신)·C1(다가오는 소리) · `decisions.md`(2단계 보류 + 09-15 관계) |

## 데이터 흐름
```
호스트: Chase 틱 → HostCatch → OnDamage(Source) ─▶ LastAttacker(OwnerOnly)·KnockdownStartServerTime 복제 ─▶ 소유 클라: 피치 보간·LookInput 게이트·HUD 호
                        └▶ EnterState(Catch) ─▶ State/StateStartServerTime 복제 ─▶ 모든 머신: Catch 에지 → MonsterCatch 소리
호스트: HostTickProxNoise → HostReportNoise(MonsterProx) ─▶ Multicast_Noise(Unreliable) ─▶ 각 머신 PlayWave → 링 + MonsterNear 소리
  없음:  LastAttacker null(치트·파괴) → 피치만, 호 숨김.  비어 있음: 표 행 없음 → 소리 없음(조용히).  표 bool false → 호 숨김.
  오류:  복제 순서 역전 → 피치는 시계만으로, 요/호는 둘 다 있을 때.  Unreliable 유실 → 그 박자 소리 한 번 빠짐(다음 0.65초에 다시).
```

## 오류와 처리 표
```
  코드 경로                          | 잘못될 수 있는 것                     | 처리
  ----------------------------------|---------------------------------------|----------------------------------------
  OnDamage → HurtEvent              | Source 가 몬스터가 아님/치트 null       | null 허용 → 피치만, 방향은 자기 위치
  카메라 모디파이어 피치·요           | 공격자 파괴/멀어짐                     | 방향은 사건의 위치(From)라 영향 없음
  (시선 입력 게이트 없음)             | —                                       | 카메라 전용 효과라 입력을 막지 않는다
  TickHitFeel(Catch 에지)            | 레벨 배치 개체 StateStartServerTime 0    | 상태로 거른다(Catch 아님) — 기존 규칙
  PlayWave(MonsterProx) 소리         | Unreliable 유실·CullDistance 밖          | 링과 같이 빠짐(다음 박자)
  HUD GetHurtDirectionAngle          | 카메라/PlayerState 없음(관전)             | 0 + 숨김
  CL9Rules 새 열 0                   | HurtLookTime 0                          | 카메라 효과 끔(0 = 끔, 기존 규칙)
```
빠진 처리 없음.

## 실패 양상 표
```
  코드 경로           | 실패 양상                          | 처리?    | 시험(PIE) | 사용자가 보는 것            | 로그?
  --------------------|-----------------------------------|----------|-----------|---------------------------|--------
  피치 보간            | 값 과대로 멀미                     | 값       | C3        | 화면 홱 내려감              | N → balance 관찰
  요 제한 보간         | 시야 빼앗김 느낌                    | 값(60°)  | C3        | 보던 방향이 밀림            | N → TASTE
  방향 호              | 장르 톤 위화                       | bool     | C3        | 게임스러운 호               | N → 표에서 끔
  MonsterNear          | 8~12 마리 겹침 소음                 | Concurr. | C1 3마리   | 꼬꼬댁 뭉침                 | N → 값(간격 0.65)
  Catch 소리 + Hurt    | 동시 재생 겹침                      | Concurr.4| C3        | 두 소리 동시                | N
```
치명적 빈틈 없음(사용자에게 안 보이면서 로그도 없는 조합 없음).

## Codex 의견 (전략 검토) — 요지
1. "피격 원인 모름" 을 "회피 가능한 공격 체계" 로 확대하면서 필요성을 검증하지 않았다 — 최소 피드백과 공격 변경을 분리해 닭에서 먼저 비교하라. 2. "밸런스 불변" 은 틀렸다(닭 396 = 걷기 396, 닭만 멈추면 공짜 이탈). 3. 0.35초는 대응 시간이 아니다(남는 여유 0.17초). 4. 스윕 뒤 `HostCatch` 에 거리·차폐 재검사가 없어 원격 잡기 위험. 5. 강제 카메라 0.45초는 시선을 빼앗는다; 닭 여러 마리면 넘어짐 중 피해 차단이 없어 연타. 6. 대안 비교가 권장안에 유리(단독 실험 미비교). 7. 협동 추출의 가치(동료 개입 시간)와 성공 기준(원인 설명률·회피율·구조 행동) 부재. 8. 검증 안 된 공격 문법이 6개월 뒤 종별 설계의 제약이 된다. 6차원 전부 NO.

## Claude 독립 검토 (전략) — 요지
문제 설정은 맞고 시점도 적절(리프레임 불필요). 그러나 **A+B 번들이 리스크** — A 만으로 "왜 맞았지" 가 풀리는데 단계적 출시가 대안표에 없다. [High] "방향 호는 리썰 포함 관례" 는 틀림 — 리썰엔 방향 표시가 없다(비네트·킥·소리뿐). [Medium] PounceReach 수치 유도 없음. [Medium] 8~12마리 동시 상황이 검증에 없음. [Medium] 카메라 강제 회전의 "도움" 전제 미검증, 재피격 처리 누락. 6개월 후회: Pounce 를 다 만들었는데 A 만으로 충분했음이 드러남. 6차원: (1)NO (2)YES (3)NO (4)NO (5)NO (6)NO.

```
전략 검토 — 두 모델 합의표:
═══════════════════════════════════════════════════════════════
  항목                                Claude  Codex  합의
  ──────────────────────────────────── ─────── ─────── ─────────
  1. 전제가 타당한가?                  아니오  아니오  둘 다 아니오 → 수치 유도·리썰 인용 정정(반영)
  2. 맞는 문제인가?                    예      아니오  불일치 → "피격 원인" 은 맞고 "회피 체계" 확대가 문제 → 2단계 분리로 해소
  3. 범위가 맞는가?                    아니오  아니오  둘 다 아니오 → A/B 분리(반영)
  4. 대안을 충분히 봤는가?             아니오  아니오  둘 다 아니오 → 단계적 출시 대안 추가(반영)
  5. 경쟁작 대비 위험을 다뤘는가?       아니오  아니오  둘 다 아니오 → 리썰 사례 정정, 방향 호는 끌 수 있게(반영)
  6. 6개월 뒤 궤적이 건전한가?          아니오  아니오  둘 다 아니오 → 검증 뒤 공통화(2단계)
═══════════════════════════════════════════════════════════════
```
**리드 판정**: 두 음성이 독립적으로 같은 결론(피드백 먼저, 예고는 분리 실험)에 닿았다 — 계획을 위 2단계 구조로 고쳤다. 사용자의 원래 방향("때리는 모션")과 다르므로 **모델 이견 1** 로 게이트에 올린다. Codex 4(재검사)·5(카메라 비용·연타)·Claude 의 리썰 정정·다마리 시험은 반영했다.

## 검증 (PIE)
- **C3-예고(신설)**: 닭이 130cm 에서 0.6초 멈춰 몸을 세우고 `MonsterPounce` 가 난다(파동 링, 2인 PIE 상대 화면에도). ① 서 있으면 110cm 덮쳐 잡힘 ② 예고 순간 뒤로 걸으면 헛침(`PounceResultServerTime`·`bPounceHit=false`) → 계속 물러나면 재예고 없음, 멈추면 간격이 좁혀져 재예고 ③ 옆으로 걸어도 헛침 ④ 예고 중 팬으로 치면 경직이 예고를 끊는다 ⑤ 벽 너머(문틈)에선 덮치지 않는다(재검사). 자동: `GetState` Chase→Pounce→Catch/Chase, Pounce 길이 0.6±0.05, 헛침 케이스 `bPounceHit=false`·`KillCooldown` 변화 없음·예고 중 `TurnRate` 제한 회전; `net.PktLag 100` 에서 클라 체감 예고 ≥0.5초; 3마리 동시·예고 만료 순간 피격·타깃 사망·문·높이 차·연속 헛침.
- **C3(갱신)**: 닭에게 잡히면 ① 그 순간 `MonsterCatch`(잡는 소리)와 `Hurt`(맞는 소리)가 함께 나고, ② 0.45초 안에 카메라가 −25° 로 내려가 덮치는 닭의 `Catch` 클립이 보이며(뒤에서 잡혔으면 요가 60° 안에서 닭 쪽으로 걸치고 호가 뒤를 가리킨다), ③ 가장자리 빨간 호가 닭 방향을 가리키고 비네트와 함께 사라지고, ④ 일어나면 피치 0·시선 입력 복귀. `cl9.damage 1`(치트)도 가장 가까운 닭을 범인으로 같은 그림. 2인 PIE: 상대 화면엔 소리만(호·카메라는 본인만).
- **C1(갱신)**: 닭이 `TriggerRadius`(780cm) 안으로 추격해 오면 **`RepeatInterval` 1.2초**마다 `MonsterNear` 가 들린다(파동 링과 같은 박자), 잠든 닭은 조용. 3마리 동시(`cl9.addmonster chicken 3`)에서도 소리가 뭉치지 않는다(Concurrency 4). (정정 2026-09-20 docs: 이 문서의 "330cm·0.65초" 는 `monster_prox` 의 **링 반경과 수명**(330 ÷ 510)이지 박자가 아니다 — 박자는 `docs/plan/data/wave.md` 표 4 의 `RepeatInterval` 1.2초, 발동 거리는 `TriggerRadius` 780cm.)
- 자동(`scratchpad/pie_feel.py` 계열): `cl9.damage` 뒤 0.45초 카메라 피치 −25±3, `LastAttacker` 세팅, `GetHurtDirectionAngle` ≈ 닭 방향, `LookInput` 게이트 해제 시각, `au.DumpActiveSounds` 에 `S_MonsterCatch`·`S_MonsterNear`.
- 표 bool 끄기: `bHurtDirectionIndicator=false` 면 호가 안 뜬다(빌드 없이). 롤백 = 표 4행 0/false + `SoundRow` 비움(빌드 없이 옛 동작).
- **다마리**: `cl9.addmonster chicken 10` 으로 10마리 추격 — `MonsterNear` 가 Concurrency 4 로 잘리고 프레임이 안 떨어진다(`stat audio`), 두 마리에 연속으로 잡혀도(`Seq` 증가) 카메라가 덜컹 없이 요 목표만 바뀐다.
- **GetUp 복귀**: 넘어짐 0.9~1.5초에 POV 피치가 −25→0 으로 돌아오고 컨트롤 회전은 넘어지기 전 값 그대로(`GetControlRotation` 로그).
- **콜드 로드**: 첫 잡힘에서 프레임 끊김 없음(`Init` 프리로드).
- **디자이너 Collapsed**: `WBP_HUD` 에서 `HurtDirection` 을 Collapsed 로 두면 C++ 가 안 켠다.
- 사용자 육안(성공 기준, Codex 7): 잡힌 직후 "무엇이 어디서" 를 말할 수 있는가.

## 실행 방법
`/cl9 task monster-pounce-hurt-readability` 실행 하나: gameplay(Pounce 상태·재검사·HurtEvent·카메라 모디파이어·소리 전이·SoundRow·치트) · art(PounceWindup 클립 Blender 헤드리스 + 소리 3개) · ui(방향 호) · balance(몬스터 4열×4종 + CL9Rules 4행) · docs. 빌드 1회(enum·열·복제), PIE C3-예고·C3·C1·C4(예고 중 피격).

## 구조 (기술 검토)
```
호스트                                         소유 클라                                모든 머신
ACL9Monster ─HostCatch─▶ ACL9Character::OnDamage ─┬─ HurtEvent(OwnerOnly) ──▶ UCL9HurtLookModifier(POV 피치·요) ┐
                                                 │                            UCL9HUDWidget::HurtDirection(각도·페이드)  │ 컨트롤 회전 불변
                                                 └─ KnockdownStart/End 복제 ─▶ (기존) DownedZ·비네트·Hurt 셰이크·Hurt 소리 ┘
ACL9Monster::EnterState(Catch) ─State/StateStartServerTime 복제─▶ TickHitFeel: Catch 전이 감지 ─▶ CL9Sound::PlayAt(MonsterCatch)
ACL9Monster::HostTickProxNoise ─▶ GameState::HostReportNoise ─Multicast_Noise─▶ UCL9WaveRenderer::PlayWave ─▶ 링 + Row.SoundRow(MonsterNear)
새 결합: UCameraModifier(엔진 표준, 캐릭터가 등록·해제) · WaveTable 행 → SoundTable 행(데이터 참조, 코드 분기 없음). 새 RPC 없음. 단일 실패점 없음(어느 행이 비어도 옛 동작).
```

## Codex 의견 (기술 검토) — 요지
[High] 컨트롤 회전 강제·LookInput 차단은 조작을 빼앗고 조준 질의(:590·969·1061)에 영향, GetUp 피치 복귀가 입력과 싸움 → **카메라 전용 효과로**. [High] 요 60° 로는 화면 밖 공격자를 보장 못 함(FOV 90 → 105° 안만), `FInterpTo` 는 도달 보장 없음 → **EaseOut 고정 시간**. [High] `LastAttacker` 는 이번 넘어짐의 사건인지 보증 못 함, 같은 틱 연타·파괴 → **사건 구조체(ServerTime·위치·Seq)**. [High] `elapsed<0.2` 창은 늦은 복제를 놓치고 첫 에지 억제·Stun 덮임 → **전이 감지**. [Medium] `PlayWave` 의 시각 게이트가 소리를 막음, 2D 소나·Concurrency 공유·`PlayAt` 동기 로드 → **게이트 앞 재생·프리로드**. [Medium] 비네트 코드가 Collapsed 를 덮어씀, `cl9.damage` 는 `CL9CheatManager.cpp`, 네트워크·수명 시험 부족. 6차원 전부 NO.

## Claude 독립 검토 (기술) — 요지
구조(OwnerOnly 사건·Catch 에지 소리)는 선례(`SwingHitServerTime`)와 일치. [Medium] `PlayWave` 에 `if (Source == MonsterProx)` 하드코딩은 §1 데이터 드리븐 위반 → **`FCL9WaveInfo.SoundRow` 열**. [Medium] `TObjectPtr` 은 Destroy 직후 non-null(pending kill) → `IsValid`. [Medium] `HurtLookTime + 0.6 ≤ KnockdownTime` 불변식 미강제 → 로드 검사. [Medium] 8~12마리 시험이 3마리뿐, GetUp 복귀 미검증. 리슨 호스트 이중 재생은 이미 가드됨(확인). 권한: 새 RPC 불필요·조준 게이트 뒤라 안전. 판정: (1)NO (2)NO (3)YES (4)YES (5)NO (6)YES(조건부).

```
기술 검토 — 두 모델 합의표:
═══════════════════════════════════════════════════════════════
  항목                                Claude  Codex  합의
  ──────────────────────────────────── ─────── ─────── ─────────
  1. 구조가 건전한가?                  아니오  아니오  둘 다 아니오 → 카메라 모디파이어·HurtEvent·SoundRow 열로 개정(반영)
  2. 시험이 충분한가?                  아니오  아니오  둘 다 아니오 → 테스트 계획 21항목(반영)
  3. 성능 위험을 다뤘는가?             예      아니오  불일치 → 동기 로드는 Init 프리로드로(반영), Concurrency 는 #17 시험
  4. 보안 위험을 다뤘는가?             예      아니오  불일치 → Codex 의 "치트 RPC 가 non-Shipping 에서 전부 허용" 은 기존 치트 설계(범위 밖), 새 공격면 없음
  5. 오류 경로를 처리했는가?           아니오  아니오  둘 다 아니오 → 사건 동등성·IsValid·전이 감지·불변식(반영)
  6. 배포 위험이 감당되는가?           예      아니오  불일치 → 롤백 = 표 4행+SoundRow 비움(빌드 없이); 패키징 검증은 Steam 게이트 몫
═══════════════════════════════════════════════════════════════
```

## Codex 재검토 (기술, 사용자가 예고를 포함한 뒤) — 요지
"포함하되 구현 전에 고칠 것": `EnterState(Chase)` 가 예산을 리셋하니 헛침 복귀는 공용 재개 헬퍼로 · Pounce 의 탈출 조건을 덮치기 전에 평가, Dead/Stun 취소, Rage 는 보존, `SenseTick` 재타깃팅 잠금 · 지속 `bPounceMissed` 로는 연속 헛침·ABP 선택·늦은 접속을 못 다룬다 → 결과 시각+bool · "Visibility 선을 따라 스윕" 은 질의를 혼동 — 반경 있는 월드 블로킹 질의(WorldStatic+WorldDynamic), 무시 목록 명시, 죽음 발사의 4회 재시도 방식 금지, 수평화·`HeadingYaw` 동기·경로 무효화·수직 조건 · 회피 산수: 후퇴 소요 0.16초라 0.6초는 여유 있으나 가속·지연·재타깃팅이 깎는다, 헛친 뒤 258cm 라 "0.6초 뒤 재예고" 는 거짓, 예고 회전이 `TurnDrag` 를 우회 · 20Hz 는 복제 주기(AI 틱 아님) — 즉시 복제·네트워크 에뮬레이션 시험 · 쿨다운 감소 1회/틱·`KillCooldown` 보존·유한 검사. 6차원 NO(계약 미완) → 위 "A 의 구현 계약" 으로 전부 반영.

## 두 검토에서 겹친 주제
**주제: 강제 카메라의 조작 비용** — CEO(Codex 5)·Eng(Codex 1·2, Claude 5)에서 독립적으로 나옴 → 고신뢰 신호, 카메라 전용(POV) 효과로 개정. **주제: 다마리 연타** — CEO(Codex 5)·CEO(Claude)·Eng(Claude 3)에서 → ~~넘어짐 중 무적은 TODO(사용자)~~(정정 2026-09-21 — 2026-09-20 사용자 결정으로 이번 실행에 포함·구현됐다, 위 "넘어짐 무적" 절), 시험은 10마리로.

## 이번에 하지 않는 것 (기술 검토 재확인)
3인칭 · 종별 예고 클립 · 소나의 층 검사(wave 미결) · 치트 RPC 검증 강화(Shipping 에서 치트 자체가 빠진다). (2단계 Pounce 는 게이트에서 이번 실행에 합쳐졌고, 넘어짐 무적은 2026-09-20 사용자 결정으로 포함됐다.)

## 테스트 계획 파일
`C:\Users\user\.gstack\projects\Work\user-master-test-plan-20260919-112719.md` — 코드패스 21개 ↔ 검증 방법.

## 미룬 일
1. ~~**넘어짐 중 무적**(P2, S): `OnDamage` 에 `IsKnockedDown()` 게이트 — 닭 여러 마리 연타 방지. 기존 규칙 변경이라 사용자 결정. Depends: 없음.~~ → **이번 실행에 포함**(사용자 결정 2026-09-20, 치트도 게이트 — 위 "넘어짐 무적" 절).
2. **2단계 Pounce 예고**(P2, M): 위 2단계 절 — 1단계 플레이테스트 뒤.
3. **파동 소나 층 검사**(P3, S): 2D 거리라 위층 닭 소리·링 — wave 미결과 함께.
4. **종별 예고 클립**(P3, M): 2단계가 열리면.

<!-- AUTONOMOUS DECISION LOG -->
## 결정 기록

| # | 단계 | 결정 | 분류 | 원칙 | 근거 | 기각한 것 |
|---|-------|----------|----------------|-----------|-----------|----------|
| 1 | 전략 모드 | 모드 SELECTIVE EXPANSION | 기계적 | P2 | 치트 한 줄·표 행 하나 수용 | EXPANSION 전면 |
| 2 | 전략 대안 | 피드백 먼저, 예고는 2단계 | **모델 이견** | P1·P3 | 두 음성 합의: 피드백/규칙 변경 분리 | B 한 번에 |
| 3 | 전략 | 카메라 요 강제 → 제한된 요(60°) | 취향 | P5 | 시선 비용(Codex) vs 범인 보이기 | 전면 요 회전 |
| 4 | 전략 | 방향 호 유지하되 표 bool + 디자이너 Collapsed 존중 | 취향 | P1 | 리썰엔 없음(Claude) — 톤 판단은 사용자 | 호 삭제 |
| 5 | 전략 | `MonsterNear` 를 기존 소나 사건에 얹음 | 기계적 | P4 | 새 사건·복제 대신 재사용 | 새 소음 이벤트 |
| 6 | 기술 | 컨트롤 회전 대신 `UCameraModifier` POV 오프셋 | 기계적 | P5 | 조작·조준 불변, 입력 게이트 불필요 | SetControlRotation+LookInput 게이트 |
| 7 | 기술 | `LastAttacker` → `FCL9HurtEvent{ServerTime,From,Attacker,Seq}` | 기계적 | P1 | 이번 사건 보증·파괴 내성 | 액터 포인터 하나 |
| 8 | 기술 | Catch 소리 = 상태 전이 감지 | 기계적 | P1 | 늦은 복제·첫 에지 문제 | elapsed<0.2 창 |
| 9 | 기술 | `FCL9WaveInfo.SoundRow` 열 | 기계적 | P4·§1 | 코드 분기 대신 데이터 | `if Source==MonsterProx` |
| 10 | 기술 | `Init` 에서 SoundTable 프리로드 | 기계적 | P3 | 콜드 로드 끊김 | 첫 재생 동기 로드 |
| 11 | 기술 | 로드 검사 `HurtLookTime+0.6 ≤ KnockdownTime` | 기계적 | P1 | 보간 겹침 | 주석만 |
| 12 | 기술 | 넘어짐 중 무적 → TODO | 취향→미룸 | P3 | 기존 규칙 변경, 사용자 몫 | 이번에 넣기 |
| 13 | 기술 | Codex "치트 RPC 검증" 은 범위 밖 | 기계적 | P3 | 기존 치트 설계·Shipping 제외 | 검증 추가 |
| 14 | 게이트 | 모델 이견 1 기각 — 예고(Pounce)도 지금 한 번에 | **사용자 결정** | — | 사용자 원래 방향 유지 | 2단계 분리 |
| 15 | 게이트 | 걷기로 물러나도 피한다(Lunge 110) | **사용자 결정** | — | 닭이 순해지는 것을 감수 | 걷기론 못 피함(Lunge 190) |
| 16 | 기술 재검토 | `ResumeChase` 헬퍼·타깃 고정·결과 시각 복제·구 스윕·TurnRate 회전·ForceNetUpdate | 기계적 | P1·P5 | Codex 재검토 계약 | bool 하나·선 트레이스 |

## 검토 완료 요약
```
  | 검토 모드            | 선택적 확장                                                |
  | 사전 점검            | 원인 3개 코드 확인(즉시 판정·발밑 기하·Source 미복제)         |
  | 0단계                | 전제 도전 2개(09-15 반려 관계·기하), 대안 3, 시간 심문        |
  | 1 구조               | 지적 2 → 카메라 모디파이어·SoundRow 열                     |
  | 2 오류 처리          | 오류 경로 7, 빠진 것 0                               |
  | 3 보안               | 새 공격면 0 (치트 RPC 는 기존·범위 밖)             |
  | 4 데이터·경계        | 없음/비어 있음/오류 4 경로, 재피격·죽음·관전 처리                |
  | 5 코드 품질          | 지적 1 (하드코딩 분기 → 데이터 열)                          |
  | 6 시험               | 시험 항목 21, 육안 항목 2                          |
  | 7 성능               | 지적 1 (동기 로드 → 미리 로드)                               |
  | 8 관측·디버그        | 로드 검사 Error 로그·`au.DumpActiveSounds`·`stat audio`       |
  | 9 배포·롤백          | 롤백 = 표 4행+SoundRow(빌드 없이)                            |
  | 10 장기 궤적         | 되돌리기 쉬움 5/5, 남긴 빚: 넘어짐 중 무적·2단계 Pounce         |
  | 11 디자인            | 생략 (UI 범위 없음 — 호 이미지 1개)                      |
  | 하지 않는 것         | 작성(6개)                                          |
  | 이미 있는 것         | 작성                                                      |
  | 이상 상태와의 차이   | 작성                                                      |
  | 오류 처리 표         | 경로 7, 치명적 빈틈 0                                 |
  | 실패 양상            | 5개, 치명적 빈틈 0                                   |
  | 미룬 일              | 4개 (미룬 일 절)                          |
  | 외부 의견            | 실행(Codex + Claude, 전략·기술)                      |
  | 그림                 | 3 (데이터 흐름·구조·이상 상태)                    |
  | 미결 결정            | 전부 해결(게이트 승인)                    |
```

## GSTACK REVIEW REPORT

| 검토 | 명령 | 목적 | 횟수 | 상태 | 결과 |
|--------|---------|-----|------|--------|----------|
| 전략 검토 | `/plan-ceo-review` | 범위·전략 | 1 | ✅ 통과(2단계 분리 → 사용자가 예고 포함으로 결정) | 5/6 CONFIRMED-NO → 개정, 1 DISAGREE |
| Codex 검토 | `/codex review` | 독립 2차 의견 | 3 (전략·기술·재검토) | ✅ 반영 | 8 + 6 + 5 지적, 전부 답함 |
| 기술 검토 | `/plan-eng-review` | 구조·시험(필수) | 2 | ✅ 통과(개정판 기준) | 3/6 CONFIRMED-NO → 개정, 3 DISAGREE 답함 |
| 디자인 검토 | `/plan-design-review` | UI/UX | 0 | ⏭ 생략(UI 범위 없음) | — |
| 개발자 경험 검토 | `/plan-devex-review` | 개발자 경험 | 0 | ⏭ 생략(해당 없음) | — |

- **Codex 요약:** 전략 — 피드백/규칙 변경 분리·밸런스 불변 주장 철회·재검사·카메라 비용; 기술 — 카메라 전용 효과·사건 구조체·전이 감지·게이트 앞 재생·프리로드·Collapsed 존중·cl9.damage 파일 위치.
- **두 모델 교차:** 두 모델이 독립적으로 "피드백 먼저, 예고는 분리" 와 "강제 카메라의 조작 비용" 에서 겹쳤다(고신뢰). Claude 만 SoundRow 데이터 열·불변식·리썰 인용 정정을, Codex 만 사건 구조체·전이 감지·프리로드를 냈다.
- **판정:** 전략 + 기술 검토 통과 — 모델의 '예고 분리' 권고는 사용자가 기각(예고 포함), 기술 재검토 계약 반영. 구현 가능.

게이트 (2026-09-19): 권고대로 승인 — 카메라 요 걸치기 켬 · 방향 호 넣음(표 bool) · 넘어짐 중 무적은 미룸.

NO UNRESOLVED DECISIONS (미결 결정 없음)
