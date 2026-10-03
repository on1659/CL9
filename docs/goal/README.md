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
