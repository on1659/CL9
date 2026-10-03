# 스테이지 만들기 — 가이드

> 스테이지 하나 = **`MapTable`의 행 하나 + 레벨 하나 + 명세 JSON 하나.** 명세(`Tools/Unreal/stages/<alias>.json`)를 쓰고
> `setup_stage.py`를 돌리면 행과 레벨이 생기고, `qa_farm_stage_pie.py`로 사람 손 없이 한 밤을 돌려 확인한다.
> 열의 뜻·값의 정본은 [../plan/data/site.md](../plan/data/site.md), 맵 생성은 [map.md](map.md), 스폰 규칙은 [raiding.md](raiding.md).
> 이 방식이 생긴 경위: [../goal/farm-stage.md](../goal/farm-stage.md) · [../decisions.md](../decisions.md) 2026-10-03.

지금 있는 스테이지: `henhouse`(양계장, `Lvl_Henhouse`) · `farm`(대농장, `Lvl_Farm`) · `pigfarm`(돼지 농장, `Lvl_PigFarm`) ·
`flat`(테스트 평지, 생성기 없음).

---

## 어떻게 이어져 있나

```
Lvl_X 를 연다
  → ACL9GameMode::StartPlay → ResolveSiteAlias()
       MapTable 에서 Level 열 == 지금 레벨인 첫 행 (없으면 게임 모드 BP 의 DefaultSiteAlias)
  → GameState.SetSiteAndDanger(alias) + SetSiteMap(seed)
       모든 머신이 그 행의 맵 열(BaseModuleCount·Theme·…)로 같은 판을 짓는다   — map.md
  → 밤 시작(로비 / cl9.night)
       밤 시계 = 행의 NightLength·DarkBase·DarkMin (0 이면 CL9Rules)          — site.md 밤 시계
       스폰   = 행의 Zones(깊이 띠마다) + MonsterSpawns/ItemSpawns(예전 규칙)  — site.md 구역
       항아리 = 납품 방(Special)마다 하나 — 몇 개 서느냐는 SpecialCopies
```

C++ 를 고칠 일이 없다. 스테이지마다 다른 것은 전부 행의 열이다.

---

## 순서

### 1. 성격부터 한 줄로 정한다

수치보다 먼저 "이 스테이지는 무엇이 다른가"를 정한다. 그 한 줄이 아래 손잡이 고르는 기준이 된다.

| 스테이지 | 한 줄 | 그래서 바꾼 손잡이 |
|---|---|---|
| `henhouse` | 기준판 — 작고 2층, 닭 위주 | (기준) |
| `farm` | 5배 넓은 판 — 깊이 갈수록 위험하고 값지다 | `BaseModuleCount` 200 · 구역 3 · 밤 480 · `SpecialCopies` 4 · `LoopChance` ↑ |
| `pigfarm` | 단층·어둡고 막다른 길이 많은 고기 판 — 돼지(잡히면 처형)를 피해 다니는 판 | `MaxFloors` 1 · `LampDensity` 0.2 · `LoopChance` 0.15 · `MainPathShare` 0.45 · 돼지·너구리 구역 |

### 2. 손잡이 고르기

| 하고 싶은 것 | 열 | 메모 |
|---|---|---|
| 판 크기 | `BaseModuleCount` | 1층 모듈 수. 일자 곡선(1일 0.85 ~ 10일 1.25)이 곱해진다. 양계장 40 |
| 층 | `MaxFloors` · `UpperFloorShare` · `StairCountMin/Max` | 위층은 아래층이 받쳐 주는 자리에만 선다 |
| 미로답게 / 트이게 | `LoopChance` (낮을수록 우회로가 적다) · `DeadEndShare` · `PassageShare` | 우회로가 적으면 처형형 몬스터가 무서워진다 |
| 깊이 | `MainPathShare` | 본줄기 비율 — 높을수록 입구에서 바닥까지 멀다 |
| 어둠 | `LampDensity` · 조명 3페이즈 | 켜지는 등의 비율. 조명 페이즈는 `inherit_from` 행에서 물려받는다 |
| 반납 지점 수 | `SpecialCopies` | 납품 방·주방을 몇 번 놓나. 판이 크면 늘린다 |
| 밤 길이 | `NightLength` · `DarkBase` · `DarkMin` | 0 이면 전역값. `DarkMin ≤ DarkBase ≤ NightLength` 가 아니면 행이 거부된다 |
| 어디에 무엇이 | `Zones` | 아래 3 |
| 값어치 | `ValueRate` · `BaseDanger` | **경제 문서([../plan/economy.md](../plan/economy.md))와 함께가 아니면 1.0 그대로 둔다** |
| 모듈 세트(겉모습) | `Theme` | 모듈 표·조각 표를 함께 고른다. 지금 세트는 `henhouse` 하나뿐 — 새 세트는 [asset-swap.md](asset-swap.md) |

**밤 길이 어림** — 양계장(본줄기 16모듈 · 245초)과 대농장(60모듈 · 480초)을 잇는 직선: `밤 ≈ 160 + 5.3 × 본줄기 모듈 수`,
본줄기 모듈 수 = `BaseModuleCount × MainPathShare`. 돼지 농장은 100 × 0.45 = 45 → 400초. 출발값일 뿐이고 플레이로 맞춘다.

### 3. 구역 짜기

깊이 = 지점의 홉 거리 ÷ 판에서 가장 먼 홉 (0 입구 ~ 1 가장 깊은 곳). 구역은 `[min, max)` 띠, 마지막 띠는 1 까지 포함한다.

- **띠는 0 부터 1 까지 빈틈없이 잇는다** — 빈 띠의 지점에는 아무것도 안 놓인다.
- 개수는 `[최소, 최대]` × 일자 배율이다. **1일차 몬스터 배율은 0.6** 이라 `2~3` 은 1일차에 1~2 마리가 된다 — 꼭 나와야 하는 종은 최소를 2 이상으로.
- 몬스터 별칭은 `MonsterTable`(`chicken` · `goose` · `pig` · `raccoon` · `sheep`), 재료는 `ItemTable`(`egg` · `meat`).
- 종의 역할: 닭 = 기본 추격, 양 = 닭과 같은 값(시험용), 거위 = 경보(잡지 않고 무리를 깨운다), 돼지 = 느리지만 잡히면 처형,
  너구리 = 안 보는 사람 뒤로 다가오는 스토커(처형). 상세는 [../plan/data/monster/](../plan/data/monster/).
- 주문서가 요구하는 재료가 있어야 한다 — 지금 주문은 계란·고기뿐이다([../plan/data/order.md](../plan/data/order.md)).
- 사이트 전체 `monster_spawns`/`item_spawns` 는 구역과 별개로 예전 규칙(몬스터는 깊은 60%, 재료는 전체)으로 더 놓인다. 구역만 쓸 거면 비운다.

### 4. 이름 문자열

표시 이름은 String Table 키여야 한다(직접 쓴 텍스트면 행이 레지스트리에서 **통째로 거부**된다 — 로그 `broken row — DisplayName is typed text`).

1. `Content/Strings/CL9_Strings.csv` 끝에 한 행: `<다음 번호>,Site.<alias>.Name,<설명>,<한국어 이름>,,,` (ko 열만 — 번역은 몰아서)
2. `python Tools/Loc/sync_strings.py` → `Content/Strings/CL9.csv` 재생성

### 5. 명세 JSON

`Tools/Unreal/stages/<alias>.json` — 기존 파일을 복사해서 고치는 게 빠르다.

```json
{
  "alias": "pigfarm",
  "name_key": "Site.pigfarm.Name",
  "level": "/Game/Maps/Lvl_PigFarm",
  "source_level": "/Game/Maps/Lvl_Henhouse",
  "inherit_from": "henhouse",
  "columns": { "base_module_count": 100, "max_floors": 1, "lamp_density": 0.2, "night_length": 400.0, "...": "..." },
  "monster_spawns": [],
  "item_spawns": [],
  "zones": [
    {"name": "outer", "depth": [0.0, 0.40], "monsters": [["chicken", 4, 6], ["pig", 1, 2]], "items": [["egg", 8, 10]]}
  ]
}
```

- `columns` 의 이름은 파이썬 이름이다 — C++ 열을 snake_case 로, bool 은 앞의 `b` 를 뗀다(`bBuildCeilings` → `build_ceilings`).
- 명세에 안 적은 열은 `inherit_from` 행에서 물려받는다(생성기·테마·조명·층·비율·밤 시계 — 목록은 스크립트의 `INHERIT`).
- `source_level` 은 레벨이 없을 때만 복제한다. 트럭 패드·푸드트럭·주방·카운터·조명·시작 손전등이 따라온다.
- **바깥 모습(어두운 저녁 하늘·흙바닥, 로비 조명)은 명세에 안 적어도 자동으로 입는다** — 모든 스테이지 공통이다([light.md](light.md) "바깥 모습").

### 6. 돌리기 (에디터를 닫은 상태 — 헤드리스)

```
py -3 .claude/hooks/cl9_lock.py acquire compile          ← 먼저 잡아야 .cmd 를 쓰고 띄울 수 있다(훅)
(.cmd 에 ping -n 15 대기 후 아래 명령을 적고 Start-Process 로 띄운 뒤)
py -3 .claude/hooks/cl9_lock.py acquire commandlet --pid <그 cmd PID>

UnrealEditor-Cmd.exe CL9.uproject -run=pythonscript ^
  -script="D:/Work/unreal/CL9/Tools/Unreal/setup_stage.py D:/Work/unreal/CL9/Tools/Unreal/stages/<alias>.json apply" -unattended -stdout
```

- 인자 없이(또는 `inspect`) 돌리면 바꾸지 않고 상태만 찍는다. 로그는 `[stage <alias>]` 로 찾는다(커맨드릿은 경고 이상만 찍혀서 경고로 남긴다).
- `apply` 가 하는 일: 레벨 복제(없을 때) → 행 쓰기·`MapTable` 저장 → 레벨의 바닥 판·`NavBounds` 를 10일차 판까지 덮게 늘림 →
  **레벨에 저장된 RecastNavMesh 삭제** → 바깥 모습(하늘 구·바닥 재질) → 레벨 저장. 다시 돌려도 같은 결과다(명세가 정본).
- 에디터가 켜져 있으면 콘솔에서 `py "…/setup_stage.py" "…/stages/<alias>.json" apply` 도 된다(같은 스크립트).

**내비메시를 지우는 이유** — 복제해 온 레벨의 저장된 내비 데이터는 원본 경계 기준 타일 용량을 들고 있어서, 런타임에 새 경계의
일부만 지어진다(대농장 첫 실측: 몬스터 11/26 이 내비 밖에서 못 움직였다). 지우면 런타임(Dynamic)이 지금 경계로 새로 짓는다.

### 7. 확인

1. **헤드리스 한 밤** — 로그만 본다:
   `UnrealEditor-Cmd.exe CL9.uproject /Game/Maps/Lvl_X -game -nullrhi -nosound -ExecCmds="cl9.night" -abslog=<경로>`
   - `Stage: level /Game/Maps/Lvl_X plays site '<alias>'` — 행이 골라졌다 (안 나오면 `Level` 열 확인)
   - `Site map built — site '<alias>' … N of N modules` — 판이 다 섰다
   - `Night 1 at '<alias>' started — <밤>s` · `zone <이름> (… M monster / L loot points)` · `K jars placed`
   - `broken row` 가 보이면 그 행은 레지스트리에서 빠진 것이다 — 메시지대로 고친다
2. **자동 PIE** — 몬스터가 정말 움직이는지·프레임:
   `UnrealEditor.exe CL9.uproject /Game/Maps/Lvl_X -ExecCmds="py D:/Work/unreal/CL9/Tools/Unreal/qa_farm_stage_pie.py"`
   → 에디터가 PIE → `cl9.night` → 밤 +20·50·90초 표본 → 스스로 닫힌다. 결과 `Saved/QA/stage_pie_Lvl_X.json`:
   `site`, `jars`, `items`, 표본마다 `on_navmesh_500`(내비 위 몬스터 수 — **몬스터 수와 같아야 한다**), `ever_moved_150`, `avg_frame_ms`,
   `nav_map`(10 m 간격 내비 지도 — `#` 가 판 전체를 덮어야 한다).
3. **사람이 한 판** — 숫자는 여기서 맞춘다. 고친 값은 명세 JSON 에 적고 6 을 다시 돌린다.

### 8. 문서

- [../plan/data/site.md](../plan/data/site.md) — 그 스테이지 절(성격 한 줄 + 값 표 + 근거)과 "지금 있는 것" 표의 행
- [../decisions.md](../decisions.md) — 왜 이 성격·이 값인지 (append-only)

---

## 함정

| 증상 | 원인 | 처리 |
|---|---|---|
| 로그에 `Stage: no site row names level …` | 행의 `Level` 이 비었거나 다른 레벨 | 명세 `level` 확인 후 apply |
| `broken row — DisplayName is typed text` | 이름을 텍스트로 넣었다 | 4 의 문자열 키 |
| `name_key … has no key` 로 apply 중단 | 문자열 행을 안 넣었거나 `sync_strings.py` 를 안 돌렸다 | 4 |
| 몬스터 일부가 영영 제자리 | 내비가 판 일부에만 있다 | QA `nav_map` 확인 → apply 다시(저장 내비 삭제·경계 확장) |
| 판이 목표보다 작게 선다 | 격자가 비좁아 자리를 못 찾는다 | 생성기의 채움 단계가 남은 예산을 메운다(2026-10-03). 그래도 작으면 `CL9.Map.LargeSite` 테스트를 그 손잡이로 돌려 본다 |
| 1일차에 꼭 나와야 할 종이 0 마리 | 일자 배율 0.6 에 반올림 | 최소를 2 이상으로 |
| `wallStub … stretched -50%` 경고 | 조각 팩 문제(모든 스테이지 공통, 기존) | 무시 — [asset-swap.md](asset-swap.md) |
