---
name: cl9film
description: |
  CL9 게임플레이 영상 촬영 — 기능을 만든 뒤 봇이 PIE 에서 실제로 플레이하는 장면을 녹화해 자막을 얹은 MP4 로 보낸다.
  한 판 흐름(도착 → 전원 → 재료 → 조리 → 납품 → 귀가) 위에 이번 기능 장면을 끼워 넣는다. 정본 도구: Tools/Film/.
  Triggers: /cl9film, 게임플레이 영상, 플레이 영상 찍어, 영상 찍어서 올려줘(기능 시연)
argument-hint: "[<보여줄 기능 설명> | round]"
user-invocable: true
---

# /cl9film — 게임플레이 영상 촬영

사용자 결정(2026-10-13): **봇이 플레이**한다 · 영상은 **한 판 흐름 + 이번에 만든 기능 장면** · 치트는 **필요한 곳에만 쓰고 자막에 밝힌다**.
보고·자막은 한국어 완결 문장. 플레이 구간은 "판"(코드 이름 Night 그대로).

인자가 `round` 면 한 판 템플릿(`Tools/Film/shots/round_farm.json`)을 그대로 찍는다. 기능 설명이면 아래 절차로 샷 목록을 짜서 찍는다.
인자가 없으면 이번 대화에서 막 만든 기능을 대상으로 한다 — 무엇인지 분명하지 않으면 묻는다.

## 절차

0. **전제**
   - 소스를 바꿨으면 빌드가 최신이어야 한다(`docs/workflow/sessions.md` 빌드 절차). 영상은 에디터에 올라간 바이너리를 찍는다.
   - 다른 세션이 PIE·빌드 중이면 기다린다: `py -3 .claude/hooks/cl9_lock.py status`.
   - 잠금(단독 명령): `py -3 .claude/hooks/cl9_lock.py acquire pie` — 진행기는 잠금을 잡지 않는다.
1. **샷 목록** — 한 판 템플릿을 복사해 `Saved/Film/shots/<기능>.json` 으로 두고(다시 쓸 만하면 `Tools/Film/shots/`), 이번 기능 장면을
   그 기능이 판 안에서 실제로 일어나는 자리에 끼운다. 장면마다 앞에 `mark` 로 자막을 단다(아래 "자막").
   - 기능에 닿는 길이 길면(정산일·회사·클리어 등) 치트로 당기고 자막에 "(시험용 치트로 …)" 라고 적는다.
   - 레벨을 건너면(회사·다른 사이트) 다음 단계는 `rebind` — 새 세계의 캐릭터를 다시 잡는다.
   - 규칙 셀을 촬영 동안만 바꾸려면 샷 목록 `rules`(예: `{"night_length": 60}`) — 끝나면 진행기가 되돌린다.
2. **찍기** — `py -3 Tools/Film/film.py --shots <샷 목록> --name <기능_날짜>` (Bash timeout 600000, 길면 run_in_background).
   준비(레벨·인원·셀·PIE 창 1280x720) → StartPIE(호스트는 별도 창) → 녹화 → 봇 → 정지 컷 → StopPIE·되돌리기 → 자막 → 인코딩.
   봇이 실패하면 영상은 만들지 않고 `Saved/Film/<이름>/status.json`(마지막 단계·오류)과 `sheet.jpg` 를 남긴다 → 샷 목록을 고쳐 다시.
3. **검토** — `Saved/Film/<이름>/sheet.jpg`(자막 표마다 2초 뒤 한 장)를 Read 로 보고 **자막이 화면과 맞는지** 확인한다.
   보이지 않는 것을 자막으로 말하지 않는다(예: 실루엣이 벽에 가렸으면 다시 찍거나 자막을 바꾼다). 맞지 않으면 1로.
4. **보내기** — `SendUserFile` 로 `Saved/Video/<이름>.mp4`(status normal, 한 줄 설명). 보고에는 무엇을 찍었는지, 치트를 쓴 곳,
   빨리 감은 곳, 봇이라서 어색한 점을 적는다.
5. **정리** — `py -3 .claude/hooks/cl9_lock.py release`. 메모리 work-progress 에 영상 경로.

## 봇 동작 (`Tools/Film/bot.py`)

샷 목록 `steps` 의 한 줄 = `{"do": <동작>, ...인자}`. 봇은 **호스트 캐릭터만** 움직인다.

| do | 인자 | 하는 일 |
|---|---|---|
| `mark` | `kind` S·F·E, `lines` [1~2줄], `speed`(F) | 자막 표. S = 제 속도, F = 다음 표까지 빨리 감기(자막 끝에 "(N배속)" 자동), E = 끝 |
| `wait` | `seconds` | 기다린다 |
| `wait_move` | `timeout` | 조작 잠금(트럭 도착·전원 인트로)이 풀릴 때까지 |
| `walk` | `to`, `within`(cm), `run`, `timeout` | 내비 경로를 따라 이동 입력으로 걷는다(가는 쪽을 본다). 경로 끝에서는 대상 쪽으로 붙어 선다 |
| `face` | `to`(기본 직전 대상), `seconds` | 시선을 대상으로 부드럽게 돌린다 — 줍기·상호작용 전에 꼭 |
| `press` | `expect`(조준 대상 클래스 이름 일부), `times`, `gap` | E (`Server_PickPlace`) — 줍기·스위치·레버·판 |
| `drop` | | G (`Server_Drop`) — 놓기·조리대에 올리기 |
| `swing` | `hold` | 휘두르기(`Server_UseStart/Release`) |
| `slot` | `n` | 퀵슬롯 고르기 |
| `wait_cooked` | `timeout` | 조리대 위가 익을 때까지(계란 12초) |
| `cheat` | `cmd` | `cl9.*` 콘솔 치트(`docs/systems/cheats.md`) — 자막에 밝힌다 |
| `teleport` | `to`, `offset`, `who` host·client | 순간이동 — 참가자를 세울 때. 영상에 보이면 자막에 밝힌다 |
| `rebind` | `timeout` | 레벨을 건넌 뒤 새 세계의 컨트롤러·캐릭터를 다시 잡는다 |

대상 `to`: `{"class": "CL9Kitchen"}`(클래스 — 걸어서 닿는 가장 가까운 것) · `{"item": "egg", "sub": ""}`(바닥의 날것, `"cooked"` 면 익은 것)
· `{"kitchen_item": true}`(조리대 위의 것) · `{"truck_inside": true}`(트럭 짐칸 가운데) · `{"loc": [x, y, z]}` · `{"last": true}`(직전 대상).
고정물 클래스: `CL9PowerSwitch` `CL9Kitchen` `CL9DepartLever` `CL9Truck` `CL9Jar` `CL9DestinationBoard` `CL9SkinCloset` `CL9Counter`(판 레벨은 트럭 창구).

## 자막

- 화면에 보이는 것만 말한다. 장면 하나에 1~2줄, 한 줄이 1240px(약 55자)를 넘으면 합성이 멈춘다 — 둘로 나눈다.
- 첫 표에 "플레이는 봇: 이동·시선은 스틱 입력, 줍기·조리·레버는 실제 조작 함수" 를 밝힌다(템플릿에 있다).
- 치트·순간이동을 쓴 장면은 "(시험용 치트로 코인 1000)" 처럼 그 자리에 적는다. 빨리 감기는 F 표가 알아서 적는다.
- 부제(영상 머리 카드)에는 촬영 방식 — "봇 플레이 · 실시간 녹화" (프레임 단위 캡처와 구분, `docs/asset/workflow.md`).

## 알려진 함정 (2026-10-13 실측)

- **양계장(Lvl_Henhouse)은 저장된 내비 범위가 건물보다 작다** — 조리대까지 경로가 없어 봇이 벽에 막힌다. 한 판 템플릿은 **대농장**.
- 납품은 **트럭 창구**(트럭 옆면 바깥의 `CL9Counter`, 2026-10-13 — `docs/goal/truck-counter.md`). 창구는 마당 내비메시 바깥이라
  `walk` 가 목표에서 가장 가까운 내비 지점까지 경로로 간 뒤 곧장 걷는다(로그 `goal off navmesh`).
- 대농장·돼지 농장은 사이트 행이 판 길이를 정해 `NightLength` 셀이 안 먹는다 — 판을 끝내려면 레버(트럭 안, 5초 안에 두 번) 또는 `cl9.night`.
- 레버는 짐칸 가운데에서 손이 안 닿는다 — `walk` 로 레버 앞(within 90)까지 간 뒤 `face`·`press`.
- 봇은 호스트만 움직인다. 2인 영상의 참가자는 서 있거나 `teleport who=client` 로만 옮긴다.
- 녹화는 PIE 창 1280x720 에서 약 20 fps(창이 크면 13 fps 로 떨어진다 — 진행기가 촬영 동안 창 크기를 바꿨다 되돌린다).
- 촬영 중에 에디터에서 다른 일을 하면(레벨 저장 등) 같이 찍히거나 멈춘다 — 잠금이 막지만 사람 손은 못 막는다.

## 파일

| 파일 | 역할 |
|---|---|
| `Tools/Film/film.py` | 진행기(한 명령) |
| `Tools/Film/bot.py` | 에디터 안 봇(샷 목록 실행) |
| `Tools/Film/editor_ops.py` | 촬영 준비·되돌리기(레벨·인원·셀·창 크기) |
| `Tools/Film/rec.py` · `grab.py` | PIE 창 실시간 녹화 · 한 장 |
| `Tools/Film/compose.py` · `encode.py` · `blender_encode.py` | 자막 합성 · MP4(ffmpeg, 없으면 Blender 5.2) |
| `Tools/Film/mcp.py` | 에디터 MCP 호출(StartPIE·StopPIE) |
| `Tools/Film/shots/round_farm.json` | 한 판 템플릿(대농장) — 전원 → 계란 → 조리 → 트럭 창구 납품 → 레버. `round_henhouse.json` 은 내비 문제로 조리대에서 막힌다 |
