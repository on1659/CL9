# docs/goal 상태 대조 감사 (읽기 전용)

감사 기준일 2026-10-16, 대상은 `C:\Work\CL9\docs\goal\` 의 문서 48개(README.md 제외, 요청서의 "46개"와 달리 실제로는 README 를 뺀 48개였다)이다. 방법은 문서의 Status 줄·수용 기준·Results 를 읽고, 문서가 이름 붙인 산출물(C++ 심볼, `cl9.*` 명령, `CL9.*` 테스트, `Lvl_*`/`BP_*`/`WBP_*`/`AN_*`, Tools 스크립트, 커밋 해시)이 `C:\Work\CL9\Source`·`Content`·`Tools`·`.claude`·git 에 실제로 있는지 대조하는 것이다. `.uasset` 은 파일 존재만 보았고, 표 열 이름은 `Source\CL9\Data\*.h` 의 UPROPERTY 로 확인했다. PIE 로 본 수치(예: control-feel 의 Gate A 73/73)는 저장소만으로 재현할 수 없어 "일치"로 두되 수치 자체는 검증하지 않았다.

## 판정 표

| 문서 | 문서가 말하는 상태 | 실제 확인 결과 | 판정 |
|---|---|---|---|
| asset-production-loop.md | 상태 줄 없음(실행 지침). README 는 진행 기록을 production-progress.md 로 안내 | `C:\Work\CL9\docs\asset\production-progress.md` 존재, 지침 문서라 대조할 산출물 없음 | 일치 |
| build-broker.md | IMPLEMENTED 10-15, 단위 테스트 23개 | `Tools\Build\build_broker.py`·`build_ctl.py`·`build_request.py`·`test_build_broker.py` 존재, `def test_` 24개. 커밋 0e4c2eb·1b340a8 | 일치(테스트 개수만 23 대 24) |
| combat-feel-pass-1.md | DONE 09-22 | `SwingHitReach`·`SwingHitRadius`·`SwingRecovery`·`SwingBufferTime`·`HitKnockbackTime`·`SwingSurfaceHitServerTime` 존재, `SwingHitScale`·`GetSwingHitSphere` 는 코드에서 사라짐(문서가 말한 삭제와 일치) | 일치 |
| control-feel.md | 구현·측정 10-05, 미커밋 | `CL9.Feel.Ramp/Shapes/Spring` 테스트가 `Source\CL9\Character\CL9FeelTests.cpp`(미추적)에 있음 | 일치 |
| cooking-progress-feedback.md | 구현·PIE 검증 10-09, 미커밋 | `SM_Cook_HeatHaze_A`·`M_CL9_Cook_HeatHaze`·계란 상태 FBX 존재. 본문 이름 `SM_Kitchen_HeatHaze_A` 는 실제 파일명과 다름. 수용 기준 B1~B12 는 `[ ]` 로 남고 Results 쪽만 `[x]` | 일치(이름 표기 어긋남) |
| doc-audit-closeout.md | 완료 09-09, 8항 중 6항 확인 | 커밋 ee9eec7·81d1971 존재, `StaminaMinResume` 는 `CL9CharacterDef` 에 있고 검증 코드도 있음, 미체크 2항은 문서가 사유를 적어 둠 | 일치 |
| execute-sequence-pig.md | Run 1 착수·Run 2 착수(09-21), 수용 기준 거의 전부 `[ ]` | 처형 C++(`ExecuteRadius`·`bExecuteOnCatch` 등)와 클립 4개(`AN_Pig_Execute`·`AN_Raccoon_Execute`·`AN_Farmer_Executed_Pig/Raccoon`) 모두 존재·커밋됨(4a151e7, a2c4834, 7e00227). 문서는 "완료"를 선언하지 않음 | 문서가 뒤처짐(착수로만 적음) |
| farm-stage.md | 구현·검증 10-03, Not committed | `Lvl_Farm`·`SpecialCopies`·사이트 `Level` 열 존재. 그러나 Lvl_Farm·Lvl_PigFarm·사이트 표는 커밋 747ca8c(10-04)에 들어감 | 일치(미커밋 표기만 낡음) |
| game-feel.md | 1차 구현, §4 줍기 팔 클립·§5 몬스터 HP 핍 미제작 | `AN_Farmer_FPPickup`·`WBP_MonsterHp` 가 `Content` 에 없음. 커밋 2bd6641·52fe32d·e84b387·25f4e6c 존재 | 일치 |
| giant-stage.md | spec settled, "Open Questions gate implementation" | 문서 안에 Open Questions 절이 없고 "Gate answers (user, 2026-10-04)" 로 답이 끝남. `UCL9SiteDef::Districts`·`FCL9SiteDistrict`·`CL9.Map.Districts`·`DistrictSlots` 가 이미 있음(커밋 0b2b7cc, 컴파일 미확인 WIP). `Lvl_Mongjang`·`Tools\Unreal\stages\mongjang.json` 은 없음 | 문서가 뒤처짐(부분 구현·질문 해소 미반영) |
| hit-feedback.md | 구현 완료 09-15 | 커밋 3e20783, `CS_HitShake` 존재 | 일치 |
| kitchen-identity.md | 구현·PIE 검증 10-09, 미커밋 | `BP_CL9Kitchen`·`CL9Kitchen` 수정 상태로 추적됨, 이름 대조에서 누락 없음 | 일치 |
| lethal-layout.md | 구현·검증 10-15, 미커밋 | `CL9SiteLayout`·`CL9SiteLayoutTests.cpp`, `CL9.Map.Flow.Rules/.Determinism/.RotateCell` 3개 모두 존재 | 일치 |
| look-rework.md | 구현·검증 10-15 evening | 룩 값은 이후 `CL9LookDef`·`LookTable.uasset`·`M_CL9_PP_Posterize`·`M_CL9_Structure` 로 이동. 문서가 말한 `CL9Rules.LookMaxResolution`·`LookGrain`·`MF_CL9_SurfaceDetail` 은 코드·에셋에 없음. 수용 기준은 전부 `[ ]` | 일치(구현은 있으나 이름이 look-table 로 이사함) |
| look-table.md | 구현·검증 10-16 | `Source\CL9\Data\CL9LookDef.h/.cpp`·`Content\Data\LookTable.uasset` 존재(둘 다 미추적), 이름 누락 0 | 일치 |
| loop-rework.md | 컨셉 명세 초안, 미결 열림, 구현 착수 전 게이트 | 문서의 미결 11개가 모두 취소선·"2026-10-12 사용자" 답으로 닫혔고 7a~7l 이 구현됨 | 문서가 뒤처짐 |
| loop-7a-wipe-penalty.md | 완료 10-12, 정산 주기만 커밋 | `WipePenaltyRate`·`SettlementIntervalDays`·`CL9.Run.WipePenalty` 존재, 코드는 a5af9f2 에 커밋됨 | 일치 |
| loop-7b-yard-no-spawn.md | 완료 10-12 | 이름 누락은 로그 문자열뿐 | 일치 |
| loop-7c-kitchen-in-building.md | 완료 10-12 | 누락 0, `BP_CL9Kitchen` 존재 | 일치 |
| loop-7d-team-kill.md | 완료 10-12 | `KilledByPlayer` 존재 | 일치 |
| loop-7e-mid-round-spawn.md | 완료 10-12 | `MidSpawnInterval` 등 존재, 누락 0 | 일치 |
| loop-7f-out-game-currency.md | 완료 10-12, uncommitted | `UCL9WalletSubsystem`·`cl9.wallet`·`WalletIncomeRate` 존재. 코드는 a5af9f2 로 커밋됨 | 일치(미커밋 표기만 낡음) |
| loop-7g-imposter.md | 완료 10-12, uncommitted | `cl9.imposter`·`CL9.Run.Imposter` 존재. 코드는 a5af9f2 로 커밋됨, 10-14 굴림 시점 변경은 문서 수정분만 미커밋 | 일치(미커밋 표기만 낡음) |
| loop-7h-kill-contribution.md | 완료 10-12, uncommitted | 이름 누락 없음, 코드 a5af9f2 로 커밋, 문서도 추적됨 | 일치(미커밋 표기만 낡음) |
| loop-7i-company.md | 완료 10-12, uncommitted | `Lvl_Company.umap`·`ACL9DestinationBoard` 추적됨 | 일치(미커밋 표기만 낡음) |
| loop-7j-round-start.md | 완료 10-12, uncommitted | `ACL9YardTension` 추적됨 | 일치(미커밋 표기만 낡음) |
| loop-7k-skins.md | 완료(구조) 10-12, uncommitted | `UCL9SkinDef`·`ACL9SkinCloset`·`SkinTable.uasset`·`cl9.skin` 존재, 추적됨 | 일치(미커밋 표기만 낡음) |
| loop-7l-stage-stars.md | 완료 10-13 | `bStageStars`·`CL9.Run.StageStars` 존재 | 일치 |
| monster-catch-alarm-species.md | 완료 09-03 | 커밋 67391d3·88021e4·98b0c16 존재, `cl9.hitmonster` 등록됨 | 일치 |
| monster-pounce-hurt-readability.md | 구현 완료 09-20, "코드는 09-21 현재 미커밋(HEAD 에 Pounce 열 없음)" | HEAD 의 `CL9MonsterDef.h` 에 `Pounce` 가 6번 나옴(커밋 4a151e7, 2026-09-21 23:44). 코드·에셋·문서 커밋 모두 존재 | 일치(미커밋 서술이 사실과 반대로 낡음) |
| outdoor-backdrop.md | spec draft, 구현 완료 아님 | `Lvl_Outdoor`·`OutdoorSet`·`OutdoorDensity` 없음. 다만 바깥 작업이 진행 중임: `CL9LandscapeTools.h`, `setup_outdoor.py`, `Content\Maps\Outdoor\M_CL9_Landscape` 등(전부 미추적) | 일치(인접 진행분은 미반영) |
| performance-baseline-and-guidelines.md | implemented 09-21, 마지막 수용 항목만 `[ ]` | `Tools\Unreal\perf_probe.py`·`perf_parse.py`, `docs\systems\performance.md`, `Config\DefaultEditorSettings.ini` 의 `bThrottleCPUWhenNotForeground=False` 확인 | 일치 |
| round-start-truck-intro.md | spec settled, Run 둘 REPORTED, 미커밋, 수용 기준 `[ ]` | `ACL9PowerSwitch`(미추적)·`CL9.Map.PowerSwitch`·`CL9.Map.CellPathToCell`·`cl9.power` 존재 | 일치 |
| session-orchestration.md | IMPLEMENTED 09-13, 커밋 e0a44fc | `.claude\agents\cl9-*.md` 6개, `.claude\hooks\cl9_lock.py`·`cl9_scope_guard.py`·`cl9_roles.py`·`cl9_run.py`·`cl9_audit.py` 와 테스트, `.claude\skills\cl9`, `cl9-roles.json`, `docs\workflow\sessions.md` 존재. 본문의 `classify_path`·`check_lock` 등은 리뷰 기록에만 나오는 계획 이름이고 구현에는 없음 | 일치 |
| session-orchestration.walkthrough.md | 같음(09-21 정정 표기 있음) | 위와 같음 | 일치 |
| settings-and-key-rebinding.md | 구현 완료 09-12~13, `cl9.resetkeys` 는 만들지 않음 | `"cl9.resetkeys"` 코드에 없음(문서와 일치), `HandleResetKeysClicked` 존재, 커밋 2053d6b·e1b459e 존재 | 일치 |
| start-lobby.md | 혼자 호스트로 검증, 2인 미검증, 미커밋 | `Lvl_Start.umap`·`CL9TitleWidget`·`CL9GameOverWidget`·`WBP_Title`·`WBP_GameOver`·`ACL9DepartLever` 존재, `GameDefaultMap=Lvl_Start`(Config\DefaultEngine.ini:15), 대부분 미추적 | 일치 |
| steam-release.md | 승인 대기·미착수 | Source 에 세션 생성 코드(`CreateSession` 등) 없음, `Source\CL9\Tests` 없음, `Lvl_Title` 없음. 시작 레벨은 `Lvl_Start` 로 따로 생김 | 일치 |
| steam-release.ko.md | 승인 대기·미착수 | 위와 같음. 다만 영어본에 추가된 10-11·10-12 정정(voice-chat, voice-eos-redpoint 언급)이 한글판에는 0건 | 일치(영어본과 내용 어긋남) |
| string-table-localization.md | 구현 완료 09-12, 커밋 7fec481 | `Content\Strings\CL9.csv`, `Content\Localization\CL9\{ko,en,ja,zh-Hans}` 존재 | 일치 |
| swing-hitbox.md | 구현 완료 09-15, 09-20 `SwingHitScale` 추가 | 커밋 d4fe2ec 존재. 그러나 `SwingHitScale`·`DropReach` 는 코드에 없다(combat-feel-pass-1 이 09-22 에 폐기, decisions.md:1550). 이 문서에는 폐기 표기가 없음 | 문서가 뒤처짐(후속 번복 미표기) |
| truck-counter.md | 완료 10-13 | `BP_CL9Counter`·`TruckCounterClass`·`TruckCounterOffset` 존재 | 일치 |
| use-action-left-click.md | 완료(체크 6/6) | 수용 기준 6개 모두 `[x]`, `Server_Shove` 는 코드에서 사라짐 | 일치 |
| voice-chat.md | implemented 10-11, voice-eos-redpoint 로 전송층 대체 | `cl9.voice`·`cl9.voiceroute`·`VoiceSpectatorVolume`·`SA_CL9_Voice` 존재. 문서가 말한 엔진 VOIP(`UVOIPTalker`·`IOnlineVoice`)는 지금 Source 에 없음(대체 작업 반영) | 일치(전송층 이력) |
| voice-eos-redpoint.md | `Status: drafted — awaiting owner approval` | 본문이 "구현이 기본값 (b) 적용"이라 하고, 실제로 `UCL9GameInstance::BP_HostLobby/BP_FindAndJoinLobby/BP_LeaveLobby/NotifyLobbyReady`, `ACL9PlayerState::BP_ApplyVoicePlacement`, `SA_CL9_Voice`/`SA_CL9_VoiceSpeaker`, 감도 행 제거(CL9GameUserSettings.h:83)가 코드에 있음. Redpoint 플러그인은 `CL9.uproject`·`Plugins` 에 없고 `UCL9VoiceTalker` 도 없음(W0 대기) | 문서가 뒤처짐(Status 줄만 초안) |
| wave-landing-ping.md | Status 줄 없음, 수용 기준 1개만 `[ ]` | `Lvl_WaveTest.umap`(미추적)·`CL9WaveTable` 수정분 존재, 이름 누락은 영상 파일명뿐 | 일치(Status 줄 부재) |
| wave-urgency-ripple.md | Status 줄 없음, 수용 기준 전부 `[x]` | `CL9WaveTable.cpp`·`WaveTable.uasset` 수정분, `cl9.wave` 존재, 누락 0 | 일치(Status 줄 부재) |
| wire-cl9art-assets.md | 완료(체크 5/5) | 수용 기준 `[x]`, `bLitWhenFree` 는 문서가 스스로 폐기한 열(Related Files 107행에 잔재만) | 일치 |

## 불일치 상세

- **loop-rework.md** — 3행 `상태`가 "컨셉 명세 초안 … 미결이 열려 있어 구현 착수 전 게이트"인데, 같은 문서 125~160행의 미결 1~11이 전부 취소선과 "2026-10-12 사용자" 답으로 닫혀 있다. 7a~7l 이 구현되고 커밋 a5af9f2 에 코드가 들어갔으므로 문서가 한 단계 이상 뒤처졌다.
- **execute-sequence-pig.md** — 3~4행은 "Run 1 착수·Run 2 착수"이고 수용 기준(68~87행)은 거의 전부 `[ ]`이다. 하지만 `ExecuteRadius`·`bExecuteOnCatch` 가 `CL9MonsterDef.h:191~` 에 있고 클립 4개가 `Content\Monsters\PigA\AN_Pig_Execute.uasset` 등으로 커밋돼 있다(HEAD 추적 확인). "완료"라고 적지 않았을 뿐 구현은 끝나 있다.
- **giant-stage.md** — 3행은 "Open Questions gate implementation"이지만 문서에 그 절이 없고 160행 "Gate answers (user, 2026-10-04)" 로 답이 났다. `CL9SiteDef.h:306` `Districts`, `CL9SiteLayoutTests.cpp` 의 `CL9.Map.Districts`·`DistrictSlots` 는 있으나 커밋 0b2b7cc 메시지가 "gameplay 중단·컴파일 미확인"이라 밝히고, `mongjang.json`·`Lvl_Mongjang` 은 못 찾음. 즉 부분 구현이다.
- **voice-eos-redpoint.md** — 3행은 "drafted — awaiting owner approval"이나 55행·56행이 구현 결과를 서술하고, 헤더에 `BP_HostLobby` 등 브리지 함수가 실제로 있다(`CL9GameInstance.h:209~217`, `CL9PlayerState.h:207`). 플러그인(W0)과 귀로 듣는 검증은 못 찾았으므로 "C++ 브리지 구현, 연동 대기"가 정확하다.
- **monster-pounce-hurt-readability.md** — 4행이 "코드는 09-21 현재 미커밋(HEAD 의 CL9MonsterDef.h 에 Pounce 열이 없다)"인데, `git show HEAD:Source/CL9/Data/CL9MonsterDef.h` 에 `Pounce` 가 6번 나온다. 커밋 4a151e7(2026-09-21 23:44)이 이 코드를 넣었다.
- **swing-hitbox.md** — 3행과 README 가 "09-20 `SwingHitScale` 추가"를 현행처럼 적는다. 그러나 `SwingHitScale`·`GetSwingHitSphere` 는 코드에 없고 decisions.md:1550 이 09-22 에 폐기를 기록했다. 이 문서에는 combat-feel-pass-1 로의 번복 표기가 없다.
- **loop-7f~7k 의 "uncommitted"** — 코드와 규칙 셀은 커밋 a5af9f2(10-13)에 들어 있고 loop-7h 등 문서도 추적 중이다. README 행은 "내 몫 커밋 a5af9f2"로 갱신됐지만 각 문서 Status 줄은 그대로다. 7g 는 10-14 굴림 시점 변경분만 실제로 미커밋이다.
- **farm-stage.md** — "Not committed" 이지만 Lvl_Farm·사이트·모듈·구간 표는 커밋 747ca8c(10-04)에 있다.
- **build-broker.md** — Status "단위 테스트 23개"인데 `Tools\Build\test_build_broker.py` 의 `def test_` 는 24개이고 README 는 24개로 적는다.
- **look-rework.md** — Status 는 구현 완료지만 수용 기준 전부 `[ ]` 이고, 본문의 `CL9Rules.LookMaxResolution`·`LookGrain`·`MF_CL9_SurfaceDetail` 은 현재 코드·에셋에 없다(`CL9LookDef` 열 `MaxResolution`·`Grain`, 재질 정적 스위치 `SurfaceDetail` 로 이동). look-table.md 가 이 이동을 설명한다.
- **cooking-progress-feedback.md** — 58행의 `SM_Kitchen_HeatHaze_A` 는 실제 `Content\Items\Kitchen\SM_Cook_HeatHaze_A.uasset` 와 이름이 다르다.
- **outdoor-backdrop.md** — 문서 상태(초안)는 맞지만, `CL9LandscapeTools.h`·`Tools\Unreal\setup_outdoor.py`·`Content\Maps\Outdoor\M_CL9_Landscape` 등 바깥 지형 작업이 미추적 상태로 진행 중임이 문서에 없다.
- **steam-release.ko.md** — 영어본에만 있는 10-11·10-12 정정(voice-chat·voice-eos-redpoint 언급)이 한글판에 없다.
- **wave-landing-ping.md, wave-urgency-ripple.md** — 상단에 Status 줄이 없어 상태를 수용 기준 체크박스로만 읽을 수 있다.

## goal/README.md 색인과 문서 Status 가 어긋난 행

README 에 행 자체가 없는 문서(8개): combat-feel-pass-1, cooking-progress-feedback, look-rework, look-table, performance-baseline-and-guidelines, round-start-truck-intro, wave-landing-ping, wave-urgency-ripple. README 머리말은 "2026-09-21 기준 요약"이라고 하지만 10월 문서 행(loop-7*, lethal-layout 등)은 갱신돼 있어 이 8개만 빠져 있다.

행은 있으나 문서와 어긋난 것:

1. **build-broker** — README "단위 24개", 문서 "23개".
2. **execute-sequence-pig** — README "Run 2(2인 클립) 미착수", 문서는 "Run 2 착수 2026-09-21"이고 클립도 존재.
3. **giant-stage** — README "명세 초안(10-04), 미결 5개", 문서는 "spec settled" 이고 Gate answers 로 답이 끝남.
4. **voice-eos-redpoint** — README "C++ 쪽 구현·컴파일 검사 완료", 문서 Status 는 "drafted — awaiting owner approval".
5. **loop-rework** — README 와 문서가 둘 다 "초안, 미결 11개"로 같지만 둘 다 사실과 다름(미결 전부 닫힘, 7a~7l 구현).
6. **monster-pounce-hurt-readability** — README "코드 미커밋", 실제로는 4a151e7 로 커밋됨(문서 Status 도 같은 오류).
7. **swing-hitbox** — README "09-20 `SwingHitScale` 추가", 현재 코드에는 없음.
8. **loop-7k-skins** — README 행의 상태 칸에 "내 몫 커밋 a5af9f2(남의 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) 선언에 기대 단독 빌드 불가)" 처럼 치환이 중첩돼 문장이 깨져 있다. 또한 7d~7k 행은 "커밋 a5af9f2", 해당 문서 Status 는 "uncommitted"(7d·7e·7b·7c 는 Status 에 커밋 언급이 없다).
9. **loop-rework 행의 "7a~7k"** — 문서 제목 쪽은 7a~7k, README 는 7l 행까지 있어 범위 표기가 서로 다르다(문서 4행 "7a~7k").
10. **farm-stage** — README "미커밋", 실제로는 커밋 747ca8c 에 포함.
