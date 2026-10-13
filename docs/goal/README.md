# goal 문서 — 색인

`/autogoal`·`/goaldoc`·`/autoplan` 이 만든 실행 명세다. **이력 문서**라 본문의 옛 문장은 지우지 않고 날짜 붙은 정정 표기를
덧붙인다. 결정의 정본은 [../decisions.md](../decisions.md), 현행 설계의 정본은 [../systems/](../systems/) — goal 과 다르면 그쪽이 맞다.
각 문서의 상태는 그 문서 상단 `Status:`/`**상태**:` 줄이 정본이고, 아래 표는 2026-09-21 기준 요약이다.

| 문서 | 한 줄 | 상태(2026-09-21) |
|---|---|---|
| [asset-production-loop.md](asset-production-loop.md) | 아이템·몬스터·캐릭터 모델·애니메이션과 재사용 맵 파츠를 만들어 UE 에서 검수하는 아트 제작 루프(2026-09-11 범위 축소 — 맵 조립은 UE 작업으로) | 진행 기록은 [../asset/production-progress.md](../asset/production-progress.md) |
| [doc-audit-closeout.md](doc-audit-closeout.md) | 2026-09-09 문서 감사 종결 — reference 주장 검증·`StaminaMinResume` 헤더 정정·HEAD 컴파일 복구 | 완료(09-09); 09-21 감사에서 8항 중 6항 확인 |
| [giant-stage.md](giant-stage.md) | 대농장의 5~10배 판(1000~2000모듈, 한 변 약 1.1km) — 생성기·내비·성능·밤 흐름 재설계 | 명세 초안(10-04), 미결 5개 — 새 세션 `/autogoal`(HEAVY 예상) |
| [outdoor-backdrop.md](outdoor-backdrop.md) | 맵 격자 밖 배경 레벨 — 공용 배경 서브레벨 + 판 둘레 자동 배치, 모델 배분 포함 | 명세 초안(10-04), 미결 6개 — 새 세션 `/autogoal` 대기 |
| [farm-stage.md](farm-stage.md) | 대농장 스테이지 — `Lvl_Farm`(양계장 5배 판), 사이트 행 `Level`·깊이 구역·밤 시계 덮어쓰기·`SpecialCopies` | 구현·검증(10-03), 미커밋 |
| [execute-sequence-pig.md](execute-sequence-pig.md) | 처형 시퀀스 — 스토커(너구리)의 등 뒤 처형·돼지의 잡히면 처형, 처형 카메라 | Run 1 완료(09-20)·종 재배치·데스캠; Run 2(2인 클립) 미착수 |
| [game-feel.md](game-feel.md) | 타격감·사살감 1차 — 효과음·히트스톱·타격 퍼프·죽음 발사·드랍 호·발소리 | 1차 구현(09-18~20); 줍기 팔 클립·몬스터 HP 핍 미제작 |
| [hit-feedback.md](hit-feedback.md) | 타격감·피격감 — 빨간 오버레이·공격자 킥·피격 셰이크·비네트 | 구현 완료(09-15); `CS_HitShake` 09-18 사인파로 번복 |
| [loop-rework.md](loop-rework.md) | 루프 개편 — 3판마다 회사 정산·구매, 이더 + 개인 소유 외부 재화(치장만), 판마다 0~1명 임포스터, 팀킬·처치 보상, 전멸은 판만 끝내고 이더 n% 차감, 마당 무스폰, 판 중간 스폰, 부엌은 건물 안 랜덤 | 컨셉 명세 초안(10-11), 미결 11개 — 구현 전 게이트 |
| [loop-7a-wipe-penalty.md](loop-7a-wipe-penalty.md) | 루프 개편 7a — 전멸은 판만 끝내고 누적 이더 30% 차감(`CL9Rules.WipePenaltyRate`), 정산 주기 3(`SettlementIntervalDays`·`QuotaCurve` 첫 키 3일차) | 완료(10-12) — PIE 1인·리슨 2인·테스트 통과, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) |
| [loop-7b-yard-no-spawn.md](loop-7b-yard-no-spawn.md) | 루프 개편 7b — 실외 모듈(`yard_open`)은 몬스터·재료 지점이 아니다(`PointPurposes` 0x8), 대농장·돼지 농장 outer 수를 남은 건물 안 지점 밀도로 다시 정함 | 완료(10-12) — PIE 3개 사이트 검증, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) |
| [loop-7c-kitchen-in-building.md](loop-7c-kitchen-in-building.md) | 루프 개편 7c — 부엌 모듈마다 조리대 하나를 호스트가 세운다(모듈 표 `PropSets[].Fixtures`), 인트로 카메라가 부엌에 닿으면 조리대를 본다 | 완료(10-12) — PIE 3개 사이트·요리·인트로 검증, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) |
| [loop-7d-team-kill.md](loop-7d-team-kill.md) | 루프 개편 7d — 휘두르기가 다른 플레이어도 맞힌다(Pawn 겹침 스윕, 피해 = 아이템 `HitDamage`), 죽인 플레이어를 `KilledByPlayer` 에 남긴다 | 완료(10-12) — 리슨 2인 PIE 검증, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) |
| [loop-7e-mid-round-spawn.md](loop-7e-mid-round-spawn.md) | 루프 개편 7e — 판 중간 스폰: 간격마다 한 마리, 모든 산 사람에게 안 보이고 먼 건물 안 지점, 총량 = 시작 수 × 비율(`CL9Rules.MidSpawn*`) | 완료(10-12) — PIE 검증, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) |
| [loop-7f-out-game-currency.md](loop-7f-out-game-currency.md) | 루프 개편 7f — 바깥 재화 "이더"(각자 기계의 `CL9Wallet`), 살아서 귀가하면 판 수입 × `WalletIncomeRate`, 새벽 화면·타이틀 표시, 판 안 화폐는 화면에서 "코인" | 완료(10-12) — PIE 1인·리슨 2인 검증, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) |
| [loop-7g-imposter.md](loop-7g-imposter.md) | 루프 개편 7g — 임포스터: 판 시작에 0~1명 비밀 배정(본인 머신에만 복제, 3인 10%·4인 25%), 판 끝 판정(성공·직접 타격 실패·처치·남겨짐 불인정), 이더 보상(성공 30·처치 20 초안), 새벽 공개, `cl9.imposter` | 완료(10-12) — 리슨 2인 PIE 검증, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) |
| [loop-7h-kill-contribution.md](loop-7h-kill-contribution.md) | 루프 개편 7h — 처치 기여: 몬스터가 깨운 사람·시각을 기억, 창(30초 초안) 안에 그 몬스터가 남을 죽이면 깨운 사람의 기여 — 새벽 목록·임포스터 처치 인정·임포스터 유도 보상(10 초안) | 완료(10-12) — 리슨 2인 PIE 검증, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) |
| [loop-7i-company.md](loop-7i-company.md) | 루프 개편 7i — 회사: 정산일 로비의 레버가 판을 싣고 `Lvl_Company` 로(비심리스 ServerTravel + 판 저장), 회사에서 정산(미달 = 코인 30% 차감 후 계속)·장비 구매·목적지 판, 레버로 고른 사이트에 가서 판을 이어 감 | 완료(10-12) — 리슨 2인 PIE 왕복 검증, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) |
| [loop-7j-round-start.md](loop-7j-round-start.md) | 루프 개편 7j — 판 시작 연출: 레벨 첫 판의 트럭 도착 카메라(메시만 후진, 잠금·무적), 오늘의 주문 카드, 마당의 긴장(문 위 깜박이는 등·전원 전 짙은 안개·건물 안 울음·문간 실루엣), 판 시계는 도착부터 | 완료(10-12) — 리슨 2인 PIE 검증, 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) |
| [loop-7k-skins.md](loop-7k-skins.md) | 루프 개편 7k — 스킨: `SkinTable`(대역 4벌, 몸 전체 교체 방식), 지갑 파일에 보유·입은 스킨, 빙의 때 호스트에 알려 복제·적용, 회사 옷장 화면(상점·마이페이지), `cl9.skin` | 완료(10-12, 구조) — 리슨 2인 PIE 검증, 내 몫 커밋 a5af9f2(남의 내 몫 커밋 a5af9f2(남의 미커밋 선언에 기대 단독 빌드 불가) 선언에 기대 단독 빌드 불가). 스킨 모델 대기 |
| [loop-7l-stage-stars.md](loop-7l-stage-stars.md) | 루프 개편 7l — 스테이지 별: 정산 기간 = 스테이지, 기간 수입 ÷ 목표로 별 0~3, 별로 다음 목적지 해금, 마지막 목적지 = 클리어(클리어 화면 → 시작 레벨). 실패로는 안 끝남. 스위치 `bStageStars` | 완료(10-13) — 리슨 2인 PIE 검증 |
| [truck-counter.md](truck-counter.md) | 판 레벨의 트럭 창구 — 트럭 옆면 바깥에 납품 창구(`BP_CL9Counter`)를 세워 판 레벨에서도 주문을 내고 코인을 번다(셀 `TruckCounterClass`·`TruckCounterOffset`) | 완료(10-13) — PIE·/cl9film 영상 검증 |
| [monster-catch-alarm-species.md](monster-catch-alarm-species.md) | 빌드 오더 ③ 3e~3g — 잡기·넘어짐·짐 흘림, 무리/사살 경보, 거위·너구리 행 | 완료(09-03); 너구리 값·Fox 대역은 폐기 |
| [monster-pounce-hurt-readability.md](monster-pounce-hurt-readability.md) | 잡히는 순간을 보이게 — 잡기 예고(`Pounce`)·넘어짐 무적·피격 카메라·피격 방향 호·소리 | 구현 완료(09-20, 코드 미커밋) |
| [session-orchestration.md](session-orchestration.md) | 세션 조율 시스템 — 역할 레지스트리·에디터/빌드 잠금·훅·`/cl9` (영문 계획 + 리뷰 기록) | 구현 완료(09-13 검증, `e0a44fc`); 현행 규약은 [../workflow/sessions.md](../workflow/sessions.md) |
| [session-orchestration.walkthrough.md](session-orchestration.walkthrough.md) | 위 계획이 실행될 때 사용자 눈에 어떻게 보이는지의 시나리오(한글) | 같음 |
| [settings-and-key-rebinding.md](settings-and-key-rebinding.md) | 로컬 옵션·설정 화면 — 키 리바인딩·감도·볼륨·언어, `{0}` 키 라벨, 화면 히스토리 스택 | 구현 완료(09-12~13); `cl9.resetkeys` 는 만들지 않음 |
| [steam-release.md](steam-release.md) · [steam-release.ko.md](steam-release.ko.md) | Steam 출시 계획 — Steamworks·세션 층·빌드 업로드 루프(영어본이 정본, 한글 번역판) | 승인 대기 — 미착수 |
| [string-table-localization.md](string-table-localization.md) | 모든 플레이어 문자열을 String Table `CL9` 키로 — 언어 전환 | 구현 완료(09-12) |
| [swing-hitbox.md](swing-hitbox.md) | 휘두르기를 애니메이션을 따르는 히트박스로, 놓기(G)는 손 자리 낙하 | 구현 완료(09-15); 09-20 `SwingHitScale` 추가 |
| [use-action-left-click.md](use-action-left-click.md) | 좌클릭 = 든 아이템 사용(행 주도), 던지기는 G, 수레는 밀기 | 완료(체크 6/6, 09-12) |
| [wire-cl9art-assets.md](wire-cl9art-assets.md) | 자체 제작 에셋(옛 `Content/CL9Art`) 전부를 표 행·BP·레벨에 연결 | 완료(체크 5/5, 09-12~13) |
