# goal 문서 — 색인

`/autogoal`·`/goaldoc`·`/autoplan` 이 만든 실행 명세다. **이력 문서**라 본문의 옛 문장은 지우지 않고 날짜 붙은 정정 표기를
덧붙인다. 결정의 정본은 [../decisions.md](../decisions.md), 현행 설계의 정본은 [../systems/](../systems/) — goal 과 다르면 그쪽이 맞다.
각 문서의 상태는 그 문서 상단 `Status:`/`**상태**:` 줄이 정본이고, 아래 표는 2026-10-16 기준 요약이다(2026-09-21 요약을 10-16 문서 감사 [../review/doc-audit-2026-10-16.md](../review/doc-audit-2026-10-16.md) 로 갱신).

| 문서 | 한 줄 | 상태(2026-10-16) |
|---|---|---|
| [asset-production-loop.md](asset-production-loop.md) | 아이템·몬스터·캐릭터 모델·애니메이션과 재사용 맵 파츠를 만들어 UE 에서 검수하는 아트 제작 루프(2026-09-11 범위 축소 — 맵 조립은 UE 작업으로) | 진행 기록은 [../asset/production-progress.md](../asset/production-progress.md) |
| [build-broker.md](build-broker.md) | 빌드 브로커 — 세션은 `build_request.py` 로 요청만, 큐·묶음·실행·오류 주인 분류·기록은 로컬 데몬이, 대기·시작 시각은 사용자가 제어, 아무 일 없으면 꺼짐 | 구현·검증 완료(10-15) — 단위 24개 + 동시 세션 E2E + 실제 `compile`·정식 `build`(에디터 종료·재기동) 통과 |
| [control-feel.md](control-feel.md) | 1인칭 조작 손맛 — 걷기·질주 곡선(가속·제동·질주 램프), 카메라(보브·롤·질주 FOV·착지 스프링), 빠진 소리, 자동 PIE 측정. 최고 속도·스태미나·점프 높이는 그대로 | 구현·측정 완료(10-05), Gate A 73/73, 미커밋 |
| [combat-feel-pass-1.md](combat-feel-pass-1.md) | 전투 손맛 1차 — 작은 넉백(`HitKnockbackTime`), 벽을 못 뚫는 조준 기준 구 스윕(`SwingHitReach`·`SwingHitRadius`, `SwingHitScale` 폐기), 팔 클립에 맞춘 회복(`SwingRecovery`), 한 칸 입력 버퍼(`SwingBufferTime`), 표면 타격음 | 완료(09-22) — C++·표·소리·PIE 측정(C4 ⑯); 2인 PIE·소리 3종 구별은 사용자 플레이로 |
| [doc-audit-closeout.md](doc-audit-closeout.md) | 2026-09-09 문서 감사 종결 — reference 주장 검증·`StaminaMinResume` 헤더 정정·HEAD 컴파일 복구 | 완료(09-09); 09-21 감사에서 8항 중 6항 확인 |
| [giant-stage.md](giant-stage.md) | 대농장의 5~10배 판(1000~2000모듈, 한 변 약 1.1km) — 생성기·내비·성능·밤 흐름 재설계 | 명세 확정(10-04, Gate answers); 부분 구현 — `UCL9SiteDef::Districts`·`CL9.Map.Districts` 테스트(0b2b7cc, WIP·컴파일 미확인, 테스트는 알려진 실패), `Lvl_Mongjang` 없음 |
| [outdoor-backdrop.md](outdoor-backdrop.md) | 맵 격자 밖 배경 레벨 — 공용 배경 서브레벨 + 판 둘레 자동 배치, 모델 배분 포함 | 명세 초안(10-04), 미결 6개 — 새 세션 `/autogoal` 대기 |
| [performance-baseline-and-guidelines.md](performance-baseline-and-guidelines.md) | 성능 기준선 — 측정으로 "지금 최적화가 필요한가"에 답하고, 측정이 지목한 것만 고치고, 반복 측정 도구(`Tools/Unreal/perf_probe.py`·`perf_parse.py`)와 규칙(`docs/systems/performance.md`)을 남김 | 구현(09-21); 마지막 수용 항목 1개 미체크 |
| [farm-stage.md](farm-stage.md) | 대농장 스테이지 — `Lvl_Farm`(양계장 5배 판), 사이트 행 `Level`·깊이 구역·밤 시계 덮어쓰기·`SpecialCopies` | 구현·검증(10-03), 커밋 747ca8c(10-04); 사람 손맛 확인 대기 |
| [execute-sequence-pig.md](execute-sequence-pig.md) | 처형 시퀀스 — 스토커(너구리)의 등 뒤 처형·돼지의 잡히면 처형, 처형 카메라 | Run 1 완료(09-20)·종 재배치·데스캠; Run 2 클립 4개 커밋(09-21·22, 7e00227·a2c4834), 2인 PIE 검증만 남음 |
| [game-feel.md](game-feel.md) | 타격감·사살감 1차 — 효과음·히트스톱·타격 퍼프·죽음 발사·드랍 호·발소리 | 1차 구현(09-18~20); 줍기 팔 클립·몬스터 HP 핍 미제작 |
| [hit-feedback.md](hit-feedback.md) | 타격감·피격감 — 빨간 오버레이·공격자 킥·피격 셰이크·비네트 | 구현 완료(09-15); `CS_HitShake` 09-18 사인파로 번복 |
| [knockdown-drop.md](knockdown-drop.md) | 넘어지면 퀵슬롯 아이템 0~2개를 무작위로 흘린다(`KnockdownDropWeight0/1/2` 40/45/15) — 결정 B, 2026-09-15 "죽을 때만" 번복 | 명세·구현(10-17), 컴파일 OK, 촬영 대기, 미커밋 |
| [burnt-bait.md](burnt-bait.md) | 바닥의 탄 요리 = 미끼 — 멎으면 `decoy` 파동, `DecoyRadius` 안의 사냥 중 아닌 몬스터가 와서 `DecoyEatTime` 먹는다(사람 무시) — 결정 F | 명세·구현(10-17), 컴파일 OK, 촬영 대기, 미커밋 |
| [kitchen-identity.md](kitchen-identity.md) | 조리대를 한눈에·지나가며 부엌으로 알아보게 — 부엌 세트 꾸밈·항상 켜진 불꽃·후드 표지·가스불/환풍기 소리·"부엌" 알림·"[G] 올려서 익히기" | 구현·PIE 검증(10-09), 사람 확인 A1·A5 대기, 미커밋 |
| [cooking-progress-feedback.md](cooking-progress-feedback.md) | 조리 진행을 보이고 들리게 — 버너 불꽃·접시 빛·열기 아지랑이(`SM_Cook_HeatHaze_A`)·재료 갈변·김→연기·지글지글 소리, 익은 계란은 계란후라이 모델 — 전부 이미 복제되는 상태에서 각 기계가 끌어냄 | 구현·PIE 검증(10-09), 미커밋; 영상 사용자 검토·스토브 놓기 문제 열림 |
| [lethal-layout.md](lethal-layout.md) | 맵 생성을 리썰(DunGen) 방식 배치 규칙으로 — 구간·모듈별 가중치와 깊이 곡선·큰 방 한 번·남는 문마다 곁가지·가지 끝 정리·인접 금지·계단 위치, 크기는 본줄기 길이 | 구현·검증(10-15) — CL9.Map.Flow 3개 통과, PIE 스폰 확인, 미커밋(섞인 파일) |
| [look-rework.md](look-rework.md) | 룩 개편 — 밤 실내를 리썰·CCL8 식으로: 검은 방과 따뜻한 빛 웅덩이, 벽 형태를 주는 그림자, 저해상도 포스터라이즈 프레임, 질감 있는 구조면, 꾸민 복도 — 게임플레이 판정은 손대지 않음 | 구현·검증(10-15 저녁, PIE·perf probe·봇 촬영), 미커밋; 값은 이후 look-table 로 이사(`CL9Rules.Look*`·`MF_CL9_SurfaceDetail` 이름은 옛것) |
| [look-table.md](look-table.md) | 룩 표 — 화면 필터·조명 프로필·램프 종류·HUD 밝기·구조면 디테일을 `LookTable`(`UCL9LookDef`) 하나에, PIE 중 편집이 즉시 반영 | 구현·검증(10-16 01:45), 미커밋(섞인 파일) |
| [data-panel.md](data-panel.md) | 데이터 패널 — 표마다 손으로 만든 HTML 페이지에서 데이터 애셋을 보고 고치고, PIE 중 바로 반영·저장(로컬 서버 `Tools/DataPanel`, 첫 페이지 룩 표) | 구현·검증(10-16 23:27), 미커밋 |
| [loop-rework.md](loop-rework.md) | 루프 개편 — 3판마다 회사 정산·구매, 이더 + 개인 소유 외부 재화(치장만), 판마다 0~1명 임포스터, 팀킬·처치 보상, 전멸은 판만 끝내고 이더 n% 차감, 마당 무스폰, 판 중간 스폰, 부엌은 건물 안 랜덤 | 컨셉 명세(10-11); 미결 11개 전부 10-12 사용자 답으로 닫힘, 7a~7l 구현·검증 완료(코드 a5af9f2) |
| [loop-7a-wipe-penalty.md](loop-7a-wipe-penalty.md) | 루프 개편 7a — 전멸은 판만 끝내고 누적 이더 30% 차감(`CL9Rules.WipePenaltyRate`), 정산 주기 3(`SettlementIntervalDays`·`QuotaCurve` 첫 키 3일차) | 완료(10-12) — PIE 1인·리슨 2인·테스트 통과, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) |
| [loop-7b-yard-no-spawn.md](loop-7b-yard-no-spawn.md) | 루프 개편 7b — 실외 모듈(`yard_open`)은 몬스터·재료 지점이 아니다(`PointPurposes` 0x8), 대농장·돼지 농장 outer 수를 남은 건물 안 지점 밀도로 다시 정함 | 완료(10-12) — PIE 3개 사이트 검증, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) |
| [loop-7c-kitchen-in-building.md](loop-7c-kitchen-in-building.md) | 루프 개편 7c — 부엌 모듈마다 조리대 하나를 호스트가 세운다(모듈 표 `PropSets[].Fixtures`), 인트로 카메라가 부엌에 닿으면 조리대를 본다 | 완료(10-12) — PIE 3개 사이트·요리·인트로 검증, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) |
| [loop-7d-team-kill.md](loop-7d-team-kill.md) | 루프 개편 7d — 휘두르기가 다른 플레이어도 맞힌다(Pawn 겹침 스윕, 피해 = 아이템 `HitDamage`), 죽인 플레이어를 `KilledByPlayer` 에 남긴다 | 완료(10-12) — 리슨 2인 PIE 검증, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) |
| [loop-7e-mid-round-spawn.md](loop-7e-mid-round-spawn.md) | 루프 개편 7e — 판 중간 스폰: 간격마다 한 마리, 모든 산 사람에게 안 보이고 먼 건물 안 지점, 총량 = 시작 수 × 비율(`CL9Rules.MidSpawn*`) | 완료(10-12) — PIE 검증, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) |
| [loop-7f-out-game-currency.md](loop-7f-out-game-currency.md) | 루프 개편 7f — 바깥 재화 "이더"(각자 기계의 `CL9Wallet`), 살아서 귀가하면 판 수입 × `WalletIncomeRate`, 새벽 화면·타이틀 표시, 판 안 화폐는 화면에서 "코인" | 완료(10-12) — PIE 1인·리슨 2인 검증, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) |
| [loop-7g-imposter.md](loop-7g-imposter.md) | 루프 개편 7g — 임포스터: 판 시작에 0~1명 비밀 배정(본인 머신에만 복제, 3인 10%·4인 25%), 판 끝 판정(성공·직접 타격 실패·처치·남겨짐 불인정), 이더 보상(성공 30·처치 20 초안), 새벽 공개, `cl9.imposter`. 굴림은 도착 판이면 도착 끝·전원 접속 뒤(10-14) | 완료(10-12) — 리슨 2인 PIE 검증, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가). 굴림 시점 변경 10-14 미커밋 |
| [loop-7h-kill-contribution.md](loop-7h-kill-contribution.md) | 루프 개편 7h — 처치 기여: 몬스터가 깨운 사람·시각을 기억, 창(30초 초안) 안에 그 몬스터가 남을 죽이면 깨운 사람의 기여 — 새벽 목록·임포스터 처치 인정·임포스터 유도 보상(10 초안) | 완료(10-12) — 리슨 2인 PIE 검증, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) |
| [loop-7i-company.md](loop-7i-company.md) | 루프 개편 7i — 회사: 정산일 로비의 레버가 판을 싣고 `Lvl_Company` 로(비심리스 ServerTravel + 판 저장), 회사에서 정산(미달 = 코인 30% 차감 후 계속)·장비 구매·목적지 판, 레버로 고른 사이트에 가서 판을 이어 감 | 완료(10-12) — 리슨 2인 PIE 왕복 검증, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) |
| [loop-7j-round-start.md](loop-7j-round-start.md) | 루프 개편 7j — 판 시작 연출: 레벨 첫 판의 트럭 도착 카메라(메시만 후진, 잠금·무적), 오늘의 주문 카드, 마당의 긴장(문 위 깜박이는 등·전원 전 짙은 안개·건물 안 울음·문간 실루엣), 판 시계는 도착부터 | 완료(10-12) — 리슨 2인 PIE 검증, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) |
| [loop-7k-skins.md](loop-7k-skins.md) | 루프 개편 7k — 스킨: `SkinTable`(대역 4벌, 몸 전체 교체 방식), 지갑 파일에 보유·입은 스킨, 빙의 때 호스트에 알려 복제·적용, 회사 옷장 화면(상점·마이페이지), `cl9.skin` | 완료(10-12, 구조) — 리슨 2인 PIE 검증, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가). 스킨 모델 대기 |
| [loop-7l-stage-stars.md](loop-7l-stage-stars.md) | 루프 개편 7l — 스테이지 별: 정산 기간 = 스테이지, 기간 수입 ÷ 목표로 별 0~3, 별로 다음 목적지 해금, 마지막 목적지 = 클리어(클리어 화면 → 시작 레벨). 실패로는 안 끝남. 스위치 `bStageStars` | 완료(10-13) — 리슨 2인 PIE 검증 |
| [round-start-truck-intro.md](round-start-truck-intro.md) | 판 시작·트럭 — 스테이지 로드 즉시 판 시작(주문은 시스템이 고름), 걸어 들어가는 트럭 안의 출발 레버 ON/OFF, 판 끝에 트럭 밖이면 남겨져 죽음, 캄캄한 건물과 불 켜진 전원 스위치(`ACL9PowerSwitch`)·플레이어별 카메라 비행 | 명세 확정(10-06); 실행 `round-truck-core`·`round-intro-camera` 보고됨, 미커밋; 사람 확인(A10, 2인) 대기, 후속 K10-F(10-10) |
| [truck-counter.md](truck-counter.md) | 판 레벨의 트럭 창구 — 트럭 옆면 바깥에 납품 창구(`BP_CL9Counter`)를 세워 판 레벨에서도 주문을 내고 코인을 번다(셀 `TruckCounterClass`·`TruckCounterOffset`) | 완료(10-13) — PIE·/cl9film 영상 검증 |
| [monster-catch-alarm-species.md](monster-catch-alarm-species.md) | 빌드 오더 ③ 3e~3g — 잡기·넘어짐·짐 흘림, 무리/사살 경보, 거위·너구리 행 | 완료(09-03); 너구리 값·Fox 대역은 폐기 |
| [monster-pounce-hurt-readability.md](monster-pounce-hurt-readability.md) | 잡히는 순간을 보이게 — 잡기 예고(`Pounce`)·넘어짐 무적·피격 카메라·피격 방향 호·소리 | 구현 완료(09-20), 코드 커밋 4a151e7(09-21) |
| [session-orchestration.md](session-orchestration.md) | 세션 조율 시스템 — 역할 레지스트리·에디터/빌드 잠금·훅·`/cl9` (영문 계획 + 리뷰 기록) | 구현 완료(09-13 검증, `e0a44fc`); 현행 규약은 [../workflow/sessions.md](../workflow/sessions.md) |
| [session-orchestration.walkthrough.md](session-orchestration.walkthrough.md) | 위 계획이 실행될 때 사용자 눈에 어떻게 보이는지의 시나리오(한글) | 같음 |
| [settings-and-key-rebinding.md](settings-and-key-rebinding.md) | 로컬 옵션·설정 화면 — 키 리바인딩·감도·볼륨·언어, `{0}` 키 라벨, 화면 히스토리 스택 | 구현 완료(09-12~13); `cl9.resetkeys` 는 만들지 않음 |
| [start-lobby.md](start-lobby.md) | 시작 레벨·타이틀 화면 → 스테이지 대기실(트럭 마당) → 출발 레버 → 전멸 시 게임 오버 화면 → 시작 레벨 복귀 | 구현·혼자 호스트 검증 완료(10-05), 2인 검증은 미완, 미커밋 |
| [steam-release.md](steam-release.md) · [steam-release.ko.md](steam-release.ko.md) | Steam 출시 계획 — Steamworks·세션 층·빌드 업로드 루프(영어본이 정본, 한글 번역판) | 승인 대기 — 미착수 |
| [string-table-localization.md](string-table-localization.md) | 모든 플레이어 문자열을 String Table `CL9` 키로 — 언어 전환 | 구현 완료(09-12) |
| [swing-hitbox.md](swing-hitbox.md) | 휘두르기를 애니메이션을 따르는 히트박스로, 놓기(G)는 손 자리 낙하 | 구현 완료(09-15); 09-20 `SwingHitScale` 추가 → 09-22 [combat-feel-pass-1](combat-feel-pass-1.md) 이 폐기(`SwingHitReach`·`SwingHitRadius` 구 스윕) |
| [use-action-left-click.md](use-action-left-click.md) | 좌클릭 = 든 아이템 사용(행 주도), 던지기는 G, 수레는 밀기 | 완료(체크 6/6, 09-12) |
| [voice-chat.md](voice-chat.md) | 인게임 마이크 — 기능·인터페이스만: 근접 3D·특정인 핫라인 라우팅 API, 음성 감지/PTT, 리썰식 관전 규칙, 음성 = 몬스터 소음, Null OSS 개발 세션(무전기 등 연출은 다음 명세). UE 5.8 엔진 사실 E1~E13 파일:줄 포함 | 구현·로그 검증(10-11, 미커밋); **전송층은 [voice-eos-redpoint.md](voice-eos-redpoint.md) 로 대체(10-12)** — 규칙·옵션·HUD·데이터 결정은 유효 |
| [voice-eos-redpoint.md](voice-eos-redpoint.md) | 음성 전송·재생층 교체 — 엔진 내장 VOIP → EOS Voice(EOS RTC) via Redpoint EOS Online Framework 무료판(Blueprint 전용), 에코 제거·잡음 억제·AGC, 세션 = EOS 로비, 핫라인 ①(연결 중엔 받는 사람 무전기 자리에서만 난다), 감쇠 에셋 둘, 감도 줄·레벨 막대 제거 | 명세(10-12); C++ 브리지(`BP_HostLobby` 등·`BP_ApplyVoicePlacement`) 구현·컴파일 검사 완료(미커밋); 플러그인은 `.uproject` 에 없음 — W0(계정·플러그인)·블루프린트 브리지·귀로 듣는 검증 대기 |
| [wave-landing-ping.md](wave-landing-ping.md) | 착지 파동 "핑" — `land` 행 파동이 벽을 만날 때까지(최대 12 m) 퍼지고, 사람·몬스터·아이템은 넘어가며, 벽에 자국을 남기고, 표 주도 2초 쿨다운; 어둠 속 실루엣 드러내기(10-16 추가) | Status 줄 없음 — 수용 기준 4/5 체크(10-16 촬영 `wave_ping_test_20261016b`), `Lvl_WaveTest` 미커밋 |
| [wave-urgency-ripple.md](wave-urgency-ripple.md) | 소음 파동을 물결로 — 밝은 마루 띠 + 뒤따르는 메아리 마루(CCL8 바닥 살 위에), 긴박도(몬스터 상태·몬스터 HP·캐릭터 HP·분노 게이지)에 따라 모양이 바뀜, 전부 표 셀 | Status 줄 없음 — 수용 기준 5/5 체크(10-15 PIE), `WaveTable` 수정분 미커밋 |
| [wire-cl9art-assets.md](wire-cl9art-assets.md) | 자체 제작 에셋(옛 `Content/CL9Art`) 전부를 표 행·BP·레벨에 연결 | 완료(체크 5/5, 09-12~13) |
