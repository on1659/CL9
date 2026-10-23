# 데이터 패널 — 데이터 애셋을 브라우저에서 고친다

> 브라우저 페이지에서 데이터 애셋 값을 보고 고친다. PIE 중이면 그 값을 쓰는 곳이 바로 따라오고, 저장은 페이지의 저장 버튼으로 한다.
> **표마다 손으로 만든 페이지가 따로 있다**(사용자 결정 2026-10-16·17 — "테이블마다 만들자"). 지금 20개 표 전부에 페이지가 있다.
> 열마다 **뜻(C++ 주석)·바꾸면 어떻게 되나·언제 반영되나**가 그 열 옆에 보인다(사용자 2026-10-17 "수치 바꾸면 어떤식으로 바뀌는지도 써줘").
> 에디터가 켜져 있어야 한다. 명세 [../goal/data-panel.md](../goal/data-panel.md).

## 쓰기

```
py -3 Tools/DataPanel/data_panel.py serve          # --open 을 붙이면 브라우저가 목록 페이지를 연다
```

데이터 패널과 사무실 서버(`http://127.0.0.1:8770/?project=CL9`)를 한 번에 켜고 끄려면 `C:\Work\CL9\StartServers.bat` · `C:\Work\CL9\StopServers.bat`
(2026-10-18). 켤 때 이미 떠 있는 포트는 건너뛰고, 끌 때는 그 포트를 쥔 프로세스를 끈다. 포트는 `CL9_PANEL_PORT`(기본 8766)와 사무실 `server.json` 의 `port` 를 그대로 읽는다.

`http://127.0.0.1:8766/` 첫 화면에 테마별로 표 카드가 있고, 표를 고르면 그 페이지(`/rules`, `/items` …)가 열린다.

**탭 세 겹**(사용자 2026-10-17 "테마별로 탭 좀 나눠주고"):
1. **테마 탭**(맨 위) — UI(룩 시안) · 빛(룩 표) · 규칙 · 사람 · 아이템·장비 · 몬스터 · 맵 · 진행·경제 · 연출·소리 · 시스템. 표의 테마와 순서는
   `tables.json` 의 `theme` 과 표 순서가 정한다(표를 더하면 그 행에 `theme` 만 적으면 된다).
2. **표 탭**(둘째 줄) — 고른 테마 안의 표들.
3. **묶음 탭**(페이지 안) — 행 표는 고른 행의 묶음(C++ Category)마다, 카드가 셋 이상인 페이지(룩·규칙·파동·진행 등)는 카드마다 탭 하나.
   마지막에 고른 탭·행은 브라우저가 기억한다.

**모델·모션 탭**(2026-10-17 사용자 "이거랑 model-gallery 라이브러리 하나로 합칠 수 있어? 사이트 하나로 조절") — `/gallery` 가
같은 머리글 아래에 모델 갤러리(`ArtSource/model-gallery.html`)를 연다. 주소 뒤 위치는 그대로 넘긴다(`/gallery#latest-motion-tests`).
갤러리는 **보기 전용**이고 지금처럼 `py -3 Tools/Assets/build_model_gallery.py` 가 만든다(아트 쪽 스크립트 — 패널은 고치지 않는다).
서버가 `ArtSource` 폴더를 `/art/…` 로 읽기 전용으로 내보내므로(폴더 밖 경로는 404, 영상 앞뒤 넘기기용 Range 지원) 따로 띄우던
`python -m http.server 8767 --directory ArtSource` 는 더 필요 없다. `tables.json` 에서는 `"kind": "page"` 행(`src` 가 열 주소)이고,
표가 아니므로 읽기·쓰기 API 는 막혀 있다.

**기획자가 읽는 문구**(사용자 2026-10-17 "기획자들도 읽기 쉬운 네이밍과 설명이 필요해") — 이름·설명·반영 문구에는 코드 이름(클래스·함수·변수·파일),
엔진 내부 말(틱·컴포넌트·레지스트리 등)을 쓰지 않는다. 값을 올리고 내릴 때 플레이에서 달라지는 것을 쓰고, 다른 열은 그 열의 한글 이름을 따옴표로
부르며, 위험한 값은 "주의:" 문장으로 남긴다. 탭·카드 제목의 영어 괄호(C++ Category)는 화면에서 뗀다. C++ 주석은 개발자용이라 머리글의
**"개발 메모"** 를 켜야 보인다(기본 끔). 문체 지침 정본: 이 문서의 "열 효과 CSV" 절.

열마다 **반영 배지** 색이 언제 먹는지를 말한다 — 초록 바로 · 파랑 새로 나올 때부터 · 보라 다음 판부터 · 주황 맵을 새로 만들 때부터 ·
노랑 플레이를 다시 켤 때부터 · 회색 지금은 안 쓰임.

- 값을 바꾸면 바로 에디터에 들어간다. 슬라이더는 끄는 동안 계속 보내고(요청은 하나씩, 밀린 값은 최신 것만), 숫자 칸은 Enter 나
  칸을 벗어날 때 보낸다.
- **C++ 검사를 그대로 탄다.** 파이썬 쓰기는 `ClampMin`·`Validate` 를 우회하므로, 패널은 값을 넣어 본 뒤 그 행의 C++ `Validate` 를
  부르고(`UCL9DefBase::ValidateForPanel`) 오류가 있으면 **그 편집을 넣지 않고** 오류 문구를 띄운다. 예: 규칙 표 `throw_charge_time` 0 →
  "Throw needs … a positive ThrowChargeTime", 몬스터 `kill_cooldown` 만 바꾸기 → "KillCooldown must equal RageTime".
  둘이 같아야 하는 열은 페이지가 묶어서 같이 바꾼다(몬스터 `rage_time` ↔ `kill_cooldown`).
- 이름 칸은 위에 한글 이름, 아래에 변수명이다(한글 이름은 effects CSV 의 `label`).
- 열 옆 설명 세 줄: 위는 C++ 주석(뜻), **바꾸면** 은 값을 올리고 내릴 때 게임에서 달라지는 것, **반영** 은 바로 · 새 스폰부터 ·
  다음 판부터 · 다음 맵 빌드부터 · 다음 PIE부터 · 영향 없음. 설명 위에 마우스를 올리면 근거(C++ 파일:줄)가 보인다.
  **영향 없음** 은 C++ 에서 그 값을 읽는 곳이 없다는 뜻이다(문서에는 있지만 아직 코드가 안 쓰는 열 — 예 규칙 표 `shake_full_time`).
- 맵 생성 열(사이트·모듈·조각·구간 종류 표)은 다음 맵 빌드부터 먹는다. 바로 보려면 PIE 콘솔에 `cl9.seed N`(호스트가 맵을 다시 짓는다).
- 위쪽 줄에 에디터 연결 여부, PIE 실행 여부, 잠금 상태가 보인다 — 다른 세션의 배타 잠금이 있으면 빨간 pill("촬영 중 — 세션 xxxx, 12:40 까지",
  용도에 따라 빌드 중·컴파일 중·커맨드릿 중), 에디터 임대 수("임대 N", 마우스를 올리면 누구인지), 에디터 자리 줄이 있으면 "줄 N명".
  **잠금은 쓰기를 막는다**(2026-10-18 사용자 "이거 조율해야지" — 10-17 "알림일 뿐" 번복, 명세
  [C:\Work\CL9\docs\goal\editor-ops.md](C:\Work\CL9\docs\goal\editor-ops.md) 3 절). 다른 세션의 배타 잠금(pie·build·compile·commandlet)이
  살아 있으면 서버가 쓰기·저장·되돌리기 둘(`/api/write`·`/api/save`·`/api/revert`·`/api/undo_save`)을 HTTP 423 으로 거부하고(본문 `blocked`·`holder`·`error`),
  페이지는 칸과 저장 단추를 잠근다. 머리글의 **"그래도 쓰기"** 를 켜면 칸이 풀리고 요청에 `force: true` 가 붙어 막기를 건너뛴다 —
  그 탭에서만 켜지고(새로 고치면 꺼진다), 켜고 쓴 사실은 잠금 도구(`record_override`)에 표·보유마다 한 번 기록되어 운영 담당이 보고서에 올린다.
- **패널 임대** — 쓰기·저장마다 고정 세션 ID `panel` 로 `editor` 임대를 받거나 갱신한다(`lock_values.csv` 의 `panel_lease_minutes`, 기본 5분).
  임대가 살아 있는 동안 다른 세션은 배타 잠금을 못 잡는다 — 마지막 쓰기 뒤 5분이면 빠져 촬영을 오래 막지 않는다. 임대를 못 받아도 쓰기는 하고
  사유를 응답 `warn` 으로 띄운다. 서버를 끄면 임대를 놓는다(다른 세션이 배타 잠금을 쥐고 있으면 잠금 도구가 놓기를 거부하므로 만료로 빠진다).
  `/api/status` 의 `lock_json` 이 잠금 도구 `status --json` 그대로다(머리글이 이것을 읽는다, 기존 `lock` 문자열도 그대로).
- **`?embed=1`** — 사무실 페이지의 데이터 탭에 끼울 때 쓴다. 머리글의 브랜드·테마 줄을 빼고 표 탭(모든 표)부터 그리며, 사무실 탭(`office` 행)은 안 그린다
  (사무실 안에서 다시 사무실을 열면 겹친다). 페이지 안 링크로 옮겨 가도 `embed=1` 이 이어진다.
- **테마**(2026-10-18) — 기본은 사무실 팔레트(연보라 바탕·종이·굵은 잉크 테두리·Fable 보라·노랑 단추, 본문 Pretendard·간판 Do Hyeon·숫자 Pixelify Sans)이고, 예전 "밤 남색 + 작업등 호박색" 은 **밤 테마**(`?theme=night`, `body.night`)로 남았다.
  사무실 데이터 탭에 끼워져 있으면 사무실이 밤일 때 `&theme=night` 로 열리고, 시각이 바뀌면 사무실이 `postMessage({type: "agentoffice:theme"})` 로 테마만 바꾼다(주소를 바꾸지 않아 편집 중인 칸이 그대로다). 폭 560px 이하(사무실 사이드바)에서는 행 목록·열 묶음이 세로로 쌓인다.
- 저장값과 다른 칸에는 왼쪽에 주황 줄이 붙고, 칸 위에 마우스를 올리면 저장값이 보인다. "저장값" 은 서버를 켠 뒤 처음 읽은 값이거나
  마지막으로 저장한 값이다.
- **저장** — 표 애셋을 저장한다(룩 표는 구조면 재질 인스턴스 중 더티인 것도 같이). PIE 중에도 된다. 누르면 먼저 입력 중인 칸을 닫고
  밀린 편집을 다 보낸 뒤 저장한다 — 슬라이더를 놓자마자 눌러도 마지막 값이 들어간다(2026-10-17 사용자 "슬라이더 고쳐서 저장 누르면 바로 저장되게").
- **직전 저장으로 되돌리기**(저장 옆) — 저장할 때마다 그 직전의 저장 상태를 `Saved/DataPanel/history/<표>.json` 에 쌓는다(표마다 최근 10개,
  서버를 다시 켜도 남는다). 누르면 가장 최근 기록의 상태를 다시 써 넣고(C++ 검사·적용 함수 그대로) 바로 저장한 뒤 그 기록을 뺀다 — 여러 번 누르면
  한 저장씩 거슬러 간다. 버튼에 그 저장 시각이 보인다. 사용자 2026-10-17 "그거로 수정하고 이상하면 원래대로". 첫 기록은 서버가 처음 읽은 디스크 값이다.
- **저장 안 한 변경 취소** — 마지막 저장 뒤에 바꾼 칸 전부를 저장 상태로 다시 써 넣는다(실시간으로 화면도 돌아온다). 저장은 하지 않는다.
- **지금 맵 것만**(머리글, 기본 켬) — 지금 맵을 알아내 그 맵에 맞는 행만 보인다(사용자 2026-10-17 "현재 에디터에서 플레이 중인 맵을 검사해서
  그 맵에 맞는 데이터만"). 지금 맵은 PIE 중이면 GameState 의 `CurrentSiteAlias`, 아니면 에디터에 열린 레벨과 `level` 이 같은 사이트 행이다.
  숨기는 것은 **다른 사이트에만 쓰이는 행**뿐이고 어느 맵에서나 쓰는 행은 그대로다:

  | 표 | 보이는 행 |
  |---|---|
  | 사이트 | 지금 사이트 |
  | 몬스터 | 지금 사이트(구역·지구 포함)에 스폰되는 것 + 어느 사이트에도 스폰 목록에 없는 것 |
  | 아이템 | 지금 사이트에 스폰되는 것과 그것을 조리·태우면 나오는 것 + 어느 사이트에도 스폰 목록에 없는 것(장비·빛·수레) |
  | 모듈 · 조각 | 지금 사이트 테마(`theme`)의 행 + 테마가 빈 공용 행 (사이트 테마가 비면 전부) |
  | 룩 표 | 지금 사이트의 조명 프로필 카드 |
  | 그 밖의 표 | 전부(전역 값) |

  숨긴 행은 행 목록 끝의 "다른 맵 행 N개 보기" 로 펼친다. 맞는 사이트 행이 없으면(예: 사이트가 아닌 테스트 레벨) "지금 맵 모름 — 전체" 다.
- 디테일 패널에서 고친 값은 브라우저 창으로 돌아올 때 다시 읽힌다.
- **하지 않는 것** — 배열·맵 행의 추가·삭제·이름 변경(이름은 다른 표·코드가 가리키는 키), 표시 이름·설명(문자열 표 키 —
  `CL9_Strings.csv` 가 정본). 키 열(`alias`·`sub_alias`·`name`)과 `Text` 열은 읽기만 한다. 그런 일은 디테일 패널에서 한다.
- 행이 많은 표는 왼쪽에 행 목록이 있고 고른 행의 열이 오른쪽에 묶음별로 나온다. 마지막에 고른 행은 브라우저가 기억한다.
  묶음에 없는 열은 "그 밖의 열" 에 나온다 — 빠지는 열이 없다.

## 표 목록

| 페이지 | 애셋 | 언제 먹나 (요지 — 열마다는 페이지에) |
|---|---|---|
| `/look` 룩 표 | `/Game/Data/LookTable` | 바로 — 적용 함수 `ApplyNow`·`ApplyBrightness` 를 부른다 |
| `/rules` 규칙 표 | `/Game/Data/CL9Rules` | 대부분 바로 · 메시·트럭·퀵슬롯 칸 수는 새 스폰부터 · 판 길이·어둠·판 중 스폰·임포스터 굴림은 다음 판 |
| `/items` 아이템 | `ItemTable` | 물리·파손·조리·사용·수레는 바로 · 메시·반경·값 굴림·기믹은 새 스폰부터 |
| `/monsters` 몬스터 | `MonsterTable` | AI 는 바로 · 메시·애님·반경·체력 굴림은 새 스폰부터 |
| `/characters` 캐릭터 | `CharacterTable` | 속도·스태미나·마찰은 바로 · 가속·점프·캡슐·메시는 새 스폰부터 |
| `/equipment` 장비 | `EquipmentTable` | 바로 |
| `/orders` 주문 | `OrderTable` | 재료·벌칙은 바로 · 보상·마감은 수락할 때 고정 · 제시 조건은 다음 로비 |
| `/buffs` 버프 | `BuffTable` | 새로 먹을 때부터 |
| `/skins` 스킨 | `SkinTable` | 가격·기본값은 바로 · 메시·색은 다시 입거나 새 스폰부터 |
| `/sites` 사이트 | `MapTable` | 조명 프로필·판매 배율은 바로 · 판 시계·스폰은 다음 판 · 맵 생성 열은 다음 맵 빌드 |
| `/modules` 모듈 | `ModuleTable` | 다음 맵 빌드 |
| `/pieces` 조각 | `PieceTable` | 다음 맵 빌드 |
| `/segkinds` 구간 종류 | `SegKindTable` | 다음 맵 빌드 |
| `/progression` 진행 | `CL9Progression` | 섞임 — 곡선마다 다르다 |
| `/wave` 파동 | `WaveTable` | 새 링부터 · 점 풀 크기·꺼진 행 목록은 다음 PIE |
| `/sound` 소리 | `SoundTable` | 다음 재생부터 (조리대 루프는 새 스폰부터) |
| `/shake` 흔들림 | `ShakeTable` | 바로 |
| `/settings` 설정 | `SettingsTable` | **읽기 전용** — 사운드 믹스·클래스 참조 |
| `/ui` UI 표 | `/Game/UI/UITable` | 다음에 그 화면을 열 때 |
| `/deflist` def 목록 | `CL9DefList` | **읽기 전용** — 저장하면 스스로 폴더를 훑어 채운다 |

정본은 `Tools/DataPanel/tables.json` 의 `live` 문구다(페이지 위쪽에 그대로 보인다).

## 구조

```
브라우저 페이지 (pages/<표>.html + panel.js)
   │  HTTP JSON  (127.0.0.1:8766, Host 확인)
data_panel.py ── 표 목록 tables.json · 열 효과 effects/<표>.csv · 헤더 열 목록 · 저장값 기억 · 변경 계산
   │  remote_execution (요청마다 연결을 열고 닫는다, 명령 포트 6777)
에디터 파이썬 ── panel_bridge.py: 읽기 · 경로로 쓰기 · C++ Validate · 표별 적용 함수 · 저장
```

| 파일 | 하는 일 |
|---|---|
| `Tools/DataPanel/data_panel.py` | HTTP 서버. 표 목록·페이지 제공, `/api/read`·`/api/write`·`/api/save`·`/api/revert`·`/api/undo_save`·`/api/status`, 쓰기 앞 잠금 검사(`write_gate`)·`panel` 임대 |
| `Tools/DataPanel/panel_bridge.py` | 에디터 안에서 import 된다. 서버를 켠 뒤·이 파일이 바뀐 뒤 한 번 다시 읽힌다(`importlib.reload`) |
| `Tools/DataPanel/cpp_schema.py` | C++ 헤더(`Source/CL9/Data`·`UI`)에서 파이썬에 안 열린 타입의 열 목록을 뽑는다 — 서버가 켤 때 `Saved/DataPanel/cpp_schema.json` 에 쓴다 |
| `Tools/DataPanel/tables.json` | 표 목록 — 키 · 제목 · 애셋 경로 · 페이지 · 적용 함수 키 · 실시간 정도 · 열 문서 |
| `Tools/DataPanel/effects/<표>.csv` | 열마다 `path,effect,timing,evidence` — 바꾸면 어떻게 되나·언제 반영되나·근거. **엑셀로 고친다**(UTF-8 BOM) |
| `Tools/DataPanel/pages/panel.js`·`panel.css` | 표 페이지가 같이 쓰는 머리글(테마·표 탭)·첫 화면·탭·위젯·배치 도구·저장 줄·상태 줄. 색·글꼴은 `panel.css` 맨 위 "공용 바탕" 덩어리의 변수 — 사무실 `C:\Work\Tools\AgentOffice\office\page\base.css` 와 글자 그대로 같은 덩어리라 고칠 때는 둘을 같이 고친다(기본 사무실 팔레트, 밤 테마 `body.night` 는 밤 남색·작업등 호박색, 인터넷이 없으면 맑은 고딕) |
| `Tools/DataPanel/pages/<표>.html` | 표 페이지 20개 |
| `Source/CL9/Data/CL9DefBase.*` | `ValidateForPanel`·`GetPanelCurveKeys`·`SetPanelCurveKeys` — 패널이 파이썬에서 부르는 C++ |

### 왜 이렇게 했나

- **포트를 두 번 잡지 않는다** — 파이썬 HTTP 서버의 기본 `allow_reuse_address` 는 Windows 에서 같은 포트에 서버 둘을 띄우게 둔다
  (2026-10-17: 옛 서버가 계속 답해 새 코드가 안 보였다). `PanelServer` 가 이를 끄므로 이미 떠 있으면 "이미 떠 있다" 를 알리고 끝난다 —
  코드를 고친 뒤에는 떠 있는 서버를 끄고 다시 켠다.
- **MCP(MCP 포트 — 설정값, 기본 8100)가 아니라 remote_execution** — MCP 의 ObjectTools 는 값을 넣을 수는 있어도 UFUNCTION(`ApplyNow`)을 못 부른다.
- **요청마다 연결을 열고 닫는다** — 에디터의 remote execution 은 명령 연결을 하나만 받고, 새로 열면 옛 연결을 밀어낸다
  (`FPythonScriptRemoteExecution::OpenCommandConnection`). 연결을 쥐고 있으면 다른 세션의 `Tools/Unreal/run_python.py` 가 끊긴다.
  명령 포트도 따로(6777, `run_python.py` 는 6776) 써서 TCP 바인드가 겹치지 않게 했다. 한 번 왕복은 0.1~0.6초다(모듈 표 125KB 가 가장 길다).
- **에디터가 둘이면 MCP 포트를 쥔 쪽** — 나중에 뜬 에디터는 포트를 못 잡은 유령이다([../workflow/sessions.md](../workflow/sessions.md)).
  서버가 노드마다 PID 를 한 번 물어 `netstat` 의 MCP 포트 주인과 맞춘다. 포트는 `editor_proc.mcp_port()` 로 읽는다.
  2026-10-18: 8000 에서 설정값(기본 8100)으로 바꿨다. 이유는 8000 이 다른 앱과 겹쳤기 때문이다. 정본은
  `C:\Work\CL9\Config\DefaultEditorPerProjectUserSettings.ini` 의 `ServerPortNumber` 다.
- **열 목록은 두 곳에서** — 파이썬에 열린 타입(`UCL9RulesDef`, `UCL9ItemDef` … `BlueprintReadOnly` 열이 있는 것)은 리플렉션 독스트링의
  `**Editor Properties:**` 목록(파이썬 snake 이름·타입·C++ 주석). 안 열린 타입(`UCL9DefTable`, `UCL9WaveTable`, `FCL9WaveInfo` 처럼
  `UCLASS()`·`USTRUCT()` 에 `BlueprintType` 이 없고 열이 `EditAnywhere` 뿐인 것)은 독스트링에 열이 없고 `get_editor_property` 도
  **C++ 이름 그대로**(`PoolSize`, `bEnabled`)만 받는다(2026-10-17 실측: `pool_size` 는 "Failed to find property"). 그래서 헤더에서 뽑은
  목록(`cpp_schema.py`)을 쓴다. 페이지·CSV 는 둘을 snake 로 맞춰 본다(`MaxConcurrent` = `max_concurrent`).
- **안 열린 열거형은 못 읽는다** — `UENUM()` 만 있는 열거형은 "Cannot pythonize" 로 실패한다. `ECL9UILayer` 는 `UENUM(BlueprintType)` 으로
  바꿨다(2026-10-17). 다른 표에 이런 열이 생기면 같은 한 단어를 더한다.
- **곡선(`FRuntimeFloatCurve`)은 C++ 함수로** — 본체 `EditorCurveData` 가 protected 라 파이썬이 못 읽는다. def 행의
  `GetPanelCurveKeys`·`SetPanelCurveKeys`(열 이름은 C++ 이름 `DangerCurve`)로 (시간, 값) 목록을 주고받는다. 보간 방식은 옛 첫 키를 따른다.
- **경로로 쓴다** — `{"path": ["Rows", 0, "chase_speed"], "value": 555}`. 파이썬은 구조체·배열·맵을 사본으로 주므로 브리지가 끝값을 바꾼 뒤
  지나온 사본을 위로 다시 써 넣는다. 인라인 행(오브젝트)은 그 자리에서 바뀌므로 그 위로는 다시 쓰지 않는다. 맵 키는 이름이다
  (열거형은 `RUN`, Name 은 문자열).
- **시험 넣기 → Validate → 진짜 넣기** — 편집을 모두 넣어 보고 지나온 def 행마다 `ValidateForPanel` 을 부른 뒤 원래 값으로 되돌린다.
  오류가 없으면 순서대로(룩 표는 밝기 편집 먼저) 다시 넣고 적용 함수를 부른다. 오류가 있으면 아무것도 남기지 않고 `rejected` 를 돌려준다.
  레지스트리의 표 사이 참조 검사(`Check*References`)는 게임 시작 때만 돈다 — 패널은 그것까지는 하지 않는다.
- **저장은 `EditorLoadingAndSavingUtils.save_packages`** — `EditorAssetLibrary.save_loaded_asset` 은 PIE 중에 "The Editor is currently
  in a play mode" 로 거부한다(2026-10-16 실측). 경로를 명시한 패키지만 저장한다(`save_assets([])` 금지 규약 그대로).
- **답은 파일로** — 브리지는 답 JSON 을 `Saved/DataPanel/reply.json` 에 쓰고 표식 줄 하나만 print 한다. 출력이 에디터 로그에도 남아서,
  표 전체를 print 하면 슬라이더 한 번마다 로그가 수십 KB 씩 는다.

### 값 JSON

| UE 타입 | JSON |
|---|---|
| float · int · bool · String | 그대로 (float 는 유효숫자 6자리) |
| Name · Text | 문자열 |
| LinearColor | `[r, g, b, a]` 선형값 — 페이지의 색 고르개는 sRGB 로 바꿔 보여 준다 |
| Color | `[r, g, b, a]` 0–255 |
| 오브젝트·소프트 오브젝트 참조 | 애셋 경로 또는 `null` (클래스는 `…_C`) |
| 구조체 | 필드 이름을 키로 한 객체 (벡터·회전은 성분 칸) |
| 인라인 행(DefTable `Rows`, 기믹) | 객체 + `__class`(실제 행 클래스) |
| 배열 | 목록 |
| 맵 | 키 이름 → 값 |
| 열거형 | 이름 (페이지는 고르기 칸 — 선택지는 브리지가 함께 보낸다) |
| 곡선 | `{"__curve": true, "keys": [[시간, 값], …], "external_curve": 경로}` |

## 사무실 페이지와 API

2026-10-18 공용 도구로 이동: 사무실 페이지·API·루프 도구는 `C:\Work\Tools\AgentOffice\`(쓰는 법 [C:\Work\Tools\AgentOffice\README.md](C:\Work\Tools\AgentOffice\README.md))에 있고, 이 서버에는 사무실 코드가 없다.
주소는 `http://127.0.0.1:8770/?project=CL9` 이고, 데이터 패널의 "사무실" 탭(`tables.json` 의 `office` 행, `kind: page`)은 `pages/frame.html` 로 그 주소를 틀 안에 연다(새 창으로 열기 링크 포함).

## 열 효과 CSV (`effects/<표>.csv`)

| 열 | 뜻 |
|---|---|
| `path` | 경로 패턴 — 구조체는 점, 배열 칸은 `[]`, 맵 키는 `{}`, 이름은 snake. 예 `rows[].hp_max` · `profiles[].night.fog_density` · `rows{}.radius_scale` |
| `label` | 열의 **한글 이름** — 페이지의 이름 칸 위에 굵게 보이고 변수명은 그 아래 작게(사용자 2026-10-17 "변수명 그대로 적는 건 밑에, 위에는 한글로"). 짧은 명사구, 단위는 넣지 않는다. 페이지가 `specs.label` 로 따로 정한 이름이 있으면 그것이 먼저다 |
| `effect` | 바꾸면 어떻게 되나 — **기획자가 읽는** 한국어 완결 문장 1~3개(올리면/내리면, 켜면/끄면, 다른 것을 넣으면). 단위는 풀어서(`cm(100 = 1m)`), 다른 열은 한글 이름을 따옴표로, 위험한 값은 "주의: …". 코드 이름·엔진 내부 말·개발 이력은 쓰지 않는다 |
| `timing` | `바로` · `새 스폰부터` · `다음 판부터` · `다음 맵 빌드부터` · `다음 PIE부터` · `영향 없음` 중 하나로 시작, " — " 뒤에 쉬운 말 한 구절. 첫 낱말은 기계용이고 화면 배지는 기획자 말로 보인다(바로 · 새로 나올 때부터 · 다음 판부터 · 맵을 새로 만들 때부터 · 플레이를 다시 켤 때부터 · 지금은 안 쓰임 — `panel.js` `TIMING`) |
| `evidence` | 그 값을 읽는 C++ 위치 `Source/CL9/…:줄` — 없으면 비우고 timing 은 `영향 없음` |

2026-10-17 에 열마다 C++ 에서 그 값을 읽는 코드를 열어 보고 썼다(추측 금지 — 확인 못 한 열은 "확인하지 못했다"). 열을 더하거나 코드가 바뀌면
그 줄만 고친다. 서버는 페이지를 열 때마다 다시 읽으므로 서버를 다시 켤 필요가 없다.

## 룩 표의 실시간 정도

브리지의 적용 함수(`APPLY['look']`)가 편집을 둘로 나눈다. 밝기 손잡이(`profiles[*].brightness`, `night_dark`, `night_bright`)를
먼저 넣고 `ApplyBrightness()` 를 부르고(프로필 Night 숫자를 끝점 사이로 다시 쓴다), 나머지를 넣고 `ApplyNow()` 를 부른다(구조면을 재질에
밀어 넣고 판 `Revision` 을 올린다). 순서가 이래야 Night 를 손으로 고친 값(되돌리기 포함)이 밝기 재계산에 덮이지 않는다.
무엇이 언제 다시 적용되는지는 [../plan/data/look.md](../plan/data/look.md) "실시간 모드" 표가 정본이다.

다른 표에는 적용 함수가 없다 — 데이터 애셋은 PIE 와 같은 메모리 객체라, C++ 가 쓸 때마다 다시 읽는 값은 그대로 따라온다.

## 표 페이지 추가·고치기

1. **`tables.json` 에 한 행** — 키는 페이지 주소가 된다. `theme` 은 테마 탭 이름(같은 테마 표끼리 붙여 둔다), `live` 는 페이지
   위쪽에 그대로 보인다 — 정직하게 쓴다.
2. **필드 이름 확인** — 서버를 띄운 채 `curl "http://127.0.0.1:8766/api/read?table=<키>"` 의 `schemas`·`values`.
3. **`pages/<키>.html`** — 표 모양에 맞는 틀 하나를 고른다.
   - 행 표(`UCL9DefTable`): `Panel.table('<키>', {title, note, groups: [{title, fields: ['열', …], note, specs}]})` — 왼쪽 행 목록 +
     묶음별 열. 공통 def 열(alias·표시 이름·아이콘·메시·액터 클래스·기믹) 묶음이 끝에 붙는다. 예 `items.html`.
   - 구조가 제각각인 애셋: `Panel.whole('<키>', {topTitle, note, order, titles, readOnly})` — 최상위 열마다 카드, 구조체·오브젝트 배열·맵은
     행 목록 카드. 예 `wave.html`.
   - 한 행 def 를 손으로 배치: `Panel.start('<키>', (P, v, root) => { … })` 안에서 `P.card` · `P.fields` · `P.grid` · `P.gridT` ·
     `P.groups` · `P.auto` · `P.rows`. 예 `look.html`·`rules.html`.

   위젯 종류는 값 타입으로 정해진다(숫자 → 슬라이더+칸, bool → 체크, 색 → 고르개+RGBA, 열거형 → 고르기, 곡선 → 키 목록+그래프,
   `/` 로 시작하는 문자열·`null` → 애셋 경로, 다른 문자열 → 글 칸). 열 옵션 `specs`: `{min, max, step}`(슬라이더 범위만 — 값은 애셋에),
   `max: 'x4'`(처음 값의 4배), `kind: 'ro'`(읽기 전용), `link: ['열']`(같은 행의 그 열도 같은 값으로, 한 요청에 묶어 보낸다).
4. **열 효과 CSV `effects/<키>.csv`** — 위 형식으로 열마다 한 줄. 열을 읽는 C++ 를 열어 보고 쓴다.
5. **실시간 적용이 필요하면 `panel_bridge.py` 의 `APPLY` 에 한 줄** — `(먼저 넣을 편집인가, 먼저 부를 함수, 뒤에 부를 함수)`.
   다른 애셋도 같이 저장해야 하면 `EXTRA_SAVES` 에 그 애셋 목록을 돌려주는 함수를 붙인다. 대부분의 표는 필요 없다.
6. **파이썬에 안 열린 타입이면** — 헤더가 `Source/CL9/Data`·`UI` 밖에 있으면 `cpp_schema.build` 의 폴더 목록에 더한다. 안 열린 열거형은
   `UENUM(BlueprintType)` 로.
7. **확인** — PIE 확인은 `py -3 .claude/hooks/cl9_lock.py acquire pie` 를 단독 명령으로 잡고 한다. 페이지에서 값을 바꿔 그 값을 쓰는 곳이
   따라오는지, 저장 뒤 파일 시각이 바뀌는지, 되돌리기가 원래로 돌아오는지 본다. 시험 값은 되돌려 저장한다.
