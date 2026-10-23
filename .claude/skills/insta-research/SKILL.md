---
name: insta-research
description: |
  인스타그램 게시물 조사 — 로그인 없이 열리는 범위에서 특정 계정·해시태그의 게시물 캡션을 모아 주제와 관련 있는 것만 추리고,
  CL9 코드·표와 대조해 `C:\Work\CL9\docs\reference\` 아래 문서로 남긴다. 2026-10-17 손맛 조사(game-feel-social.md)의 절차를 그대로 옮긴 것이다.
  Triggers: /insta-research, 인스타 조사, 인스타에서 찾아봐
argument-hint: "[<계정 이름> | #<해시태그> ...] [--max <수집 상한>] [--against <대조할 CL9 문서>] [--topic <주제>]"
user-invocable: true
---

# /insta-research — 인스타그램 게시물 조사

2026-10-17 에 세션 f680dfeb 가 `graphic.magazine` 계정과 해시태그 `#gamefeel` `#타격감` `#gamejuice` 에서 캡션 48개를 모아
관련 6개를 추리고 CL9 와 대조해 [C:\Work\CL9\docs\reference\game-feel-social.md](C:\Work\CL9\docs\reference\game-feel-social.md) 를 쓴 절차다.
같은 날 세션 cc42bb21 이 한 번 더 인스타를 봤는데(2차), 그때는 사용자가 직접 로그인해 둔 탭을 썼다 — 끝의 "로그인된 탭이 있을 때" 절.
이 문서에서 **"미확인"** 이라고 적은 것은 그 두 기록에 없는 내용이다. 추측으로 채우지 않았다.

보고·문서는 한국어 완결 문장. 경로는 전부 절대 경로.

## 입력

| 인자 | 뜻 | 없으면 |
|---|---|---|
| 계정 이름 (`graphic.magazine` 처럼 `@` 없이) | 프로필 첫 화면의 게시물을 모은다 | 사용자가 준 게시물 링크의 계정 |
| `#해시태그` (여러 개, 한글 가능) | 해시태그 탐색 페이지의 상위 게시물을 모은다 | 사용자가 준 게시물의 해시태그 중 주제와 닿는 것 |
| `--max <n>` 수집 상한 | 캡션을 가져올 게시물 수의 상한 | 48 (2026-10-17 에 차단 없이 끝난 수. 그 이상은 미확인) |
| `--against <문서>` 대조할 CL9 문서 | "CL9 현재" 열의 근거로 삼을 문서·코드 | 주제에 맞는 `C:\Work\CL9\docs\systems\*.md` · `docs\plan\data\*.md` · `Source\CL9\` grep |
| `--topic <주제>` | 관련/무관을 가르는 기준 주제 | 사용자가 준 게시물의 주제 |

인자가 하나도 없으면 사용자가 방금 준 인스타 링크를 대상으로 한다. 링크도 없으면 묻는다.

## 절차

### 0. 전제 — 브라우저와 로그인

- 브라우저는 전역 지침대로 gstack `/browse` 다. 정본 실행 파일은 `C:\Users\Radar\.claude\skills\gstack\browse\dist\browse` 이고, 부를 때는
  `/browse` 스킬의 preamble(`C:\Users\Radar\.claude\skills\gstack\bin\gstack-skill-start --skill "browse" --model "claude"`)을 먼저 친다.
  Bash 에서 `B="$HOME/.claude/skills/gstack/browse/dist/browse"` 로 잡고 `"$B" goto <url>` · `"$B" text` · `"$B" js "<식>"` · `"$B" screenshot <경로>` · `"$B" closetab` 을 쓴다.
- 이 브라우저는 **로그인돼 있지 않은 상태**로 쓴다. 로그인 벽을 우회하지 않고, 벽 바깥에 보이는 것만 읽는다.
  이 브라우저가 어느 프로필·쿠키를 쓰는지는 기록에 없다(미확인). `/setup-browser-cookies` 는 쓰지 않았다.
- 접근 사이에 `sleep 2`(목록 페이지) · `sleep 1.5`(게시물 페이지)를 둔다. 이것 말고 레이트 제한을 피하는 조치는 없었고,
  48개(목록 8페이지 + 게시물 48)까지는 차단이 없었다. **그 이상에서 차단되는지는 미확인.**

### 1. 접근 확인 — 무엇이 보이는지 먼저 잰다

사용자가 준 링크 하나를 열어 로그인 없이 읽히는 범위를 확인하고 보고한다.

```bash
B="$HOME/.claude/skills/gstack/browse/dist/browse"; "$B" goto "<게시물 URL>" 2>&1 | tail -1; "$B" text 2>&1 | head -60; "$B" js "JSON.stringify({title:document.title, og:[...document.querySelectorAll('meta[property^=\"og:\"]')].map(m=>m.getAttribute('property')+'='+m.content)})" 2>&1 | tail -1
```

2026-10-17 에 확인된 범위(릴 한 편 기준):

| 열리는 것 | 안 열리는 것 |
|---|---|
| 캡션 전문·해시태그·작성자·게시 날짜·좋아요·댓글 수 (`og:title` `og:description`), 썸네일 1장 (`og:image`) | 영상 재생·프레임 (로그인 벽) |
| 프로필 첫 화면의 최근 게시물 **12개 링크** | 그 뒤 게시물 (스크롤해도 안 열림), 릴스 탭 (비어 있음) |
| 해시태그 탐색 `https://www.instagram.com/explore/tags/<태그>/` 의 상위 **12개 링크** (`utm_source=popular_topic_grid`) | 스토리 (로그인 필수 + 24시간 소멸) |

해시태그 페이지가 0개를 돌려주면 그 태그 페이지가 비어 있는 것이다(2026-10-17: `#게임손맛` `#히트스톱` `#juice` `#게임필` 이 0개).
상위 12개가 어떤 순서(인기순·최신순)인지는 미확인.

### 2. 수집 ① — 링크 모으기

프로필과 해시태그 페이지를 차례로 열어 `/reel/`·`/p/` 링크를 뽑아 세션 스크래치패드 `links.txt`(탭 구분: 출처 페이지 \t 경로)에 쌓는다.
한글 해시태그는 URL 인코딩한다(`#타격감` → `%ED%83%80%EA%B2%A9%EA%B0%90`).

```bash
B="$HOME/.claude/skills/gstack/browse/dist/browse"; S="<세션 스크래치패드 절대 경로>"; : > "$S/links.txt"
for u in "https://www.instagram.com/<계정>/" "https://www.instagram.com/explore/tags/<태그1>/" "https://www.instagram.com/explore/tags/<태그2>/"; do
  "$B" goto "$u" >/dev/null 2>&1; sleep 2
  "$B" js "[...document.querySelectorAll('a[href*=\"/reel/\"],a[href*=\"/p/\"]')].map(a=>a.getAttribute('href')).filter((v,i,s)=>s.indexOf(v)===i).join('\n')" 2>&1 | grep -E '^/' | sed "s|^|$u\t|" >> "$S/links.txt"
  echo "$u -> $(grep -c "$u" "$S/links.txt")"
done; "$B" closetab >/dev/null 2>&1; echo TOTAL $(cut -f2 "$S/links.txt" | sort -u | wc -l)
```

페이지마다 몇 개가 잡혔는지 보고한다. 합계가 `--max` 를 넘으면 사용자가 준 계정·해시태그 순서대로 앞에서 자른다.

### 3. 수집 ② — 캡션 가져오기

링크마다 게시물 페이지를 열어 `og:title` `og:description` `og:image` `og:video` 만 읽어 `posts.jsonl` 한 줄씩 쌓는다.
48개에 약 2분 걸리므로 **`run_in_background`** 로 돌리고(timeout 600000), 그 사이에 4·5단계의 CL9 쪽 조사를 먼저 한다.

```bash
B="$HOME/.claude/skills/gstack/browse/dist/browse"; S="<세션 스크래치패드 절대 경로>"; : > "$S/posts.jsonl"; n=0
for p in $(cut -f2 "$S/links.txt" | sort -u); do
  n=$((n+1)); "$B" goto "https://www.instagram.com$p" >/dev/null 2>&1; sleep 1.5
  "$B" js "JSON.stringify({path:'$p', title:(document.querySelector('meta[property=\"og:title\"]')||{}).content||'', desc:(document.querySelector('meta[property=\"og:description\"]')||{}).content||'', img:(document.querySelector('meta[property=\"og:image\"]')||{}).content||'', video:(document.querySelector('meta[property=\"og:video\"]')||{}).content||''})" 2>&1 | grep -E '^\{' >> "$S/posts.jsonl"
done; "$B" closetab >/dev/null 2>&1; echo FETCHED $n; wc -l "$S/posts.jsonl"; grep -c '"title":""' "$S/posts.jsonl"
```

끝나면 `title` 이 빈 줄 수(못 읽은 게시물)를 보고한다. `og:description` 은 `"<좋아요> likes, <댓글> comments - <계정> on <날짜>: \"<캡션>\""` 꼴이다.

읽기 좋게 `posts.md` 로 푼다(아래 스크립트를 스크래치패드에 `dump_posts.py` 로 두고 `py -3 -I dump_posts.py posts.jsonl posts.md`).
**콘솔에 바로 찍으면 cp949 인코딩 오류가 난다** — 파일로 쓰고 Read 로 읽는다(`PYTHONIOENCODING=utf-8` 을 붙여도 된다).

```python
import json, sys
src, dst = sys.argv[1], sys.argv[2]
out = []
for l in open(src, encoding="utf-8"):
    d = json.loads(l)
    out.append("## " + d["path"])
    out.append("img: " + ("yes" if d["img"] else "no") + " | video: " + ("yes" if d["video"] else "no"))
    out.append(d["desc"].replace("\n", " / "))
    out.append("")
open(dst, "w", encoding="utf-8").write("\n".join(out))
print(len(out) // 4)
```

`links.txt` `posts.jsonl` `posts.md` 는 **세션 스크래치패드에만** 둔다. 저장소(`C:\Work\CL9\`)에 옮기지 않는다 — 남의 글 전문을 옮겨 적지 않는 `docs\reference\` 규칙.

### 4. 추리기 — 관련/무관 판정

`posts.md` 를 읽고 캡션 내용이 `--topic` 주제를 실제로 다루는지로 가른다. 2026-10-17 에 적용된 기준:

| 판정 | 기준 | 그날의 예 |
|---|---|---|
| 관련 | 캡션이 주제의 기법·원리를 말한다 | 히트스톱·스쿼시·셰이크·파티클, 발 IK, 래그돌, "모든 행동에 반응", 임팩트 프레임 노티파이 |
| 관련 (내용 없음) | 제목만 있고 캡션에 내용이 없다 — 표에는 넣되 "캡션에 내용 없음(제목뿐)" 이라고 적는다 | "What is Game Juice?" |
| 무관 — 동음이의 | 같은 단어를 다른 뜻으로 쓴 글 | 주스(음료), 야구단의 "타격감" |
| 무관 — 홍보 | 주제어를 해시태그로만 달고 내용은 일반 게임 홍보 | 모바일 게임 플레이 영상 |
| 무관 — 같은 계정의 다른 주제 | 계정은 맞지만 주제가 다르다 | 레이마칭·렌더링 파이프라인·셰이더·복셀·AI 게임 (렌더링 교양) |

무관으로 뺀 것은 문서에 **이유만 한 줄**로 적는다(목록은 안 적는다). 영상은 못 봤으므로 "릴이 말한 것"은 캡션 문장 그대로이고, 캡션에 없는 내용을 영상이 보여줬을 거라고 쓰지 않는다.
관련 글이 기법 이름만 나열하면, 그 기법을 길게 설명한 **원전**(강연·책)을 "보강" 으로 덧붙여 2절 목록을 채운다(2026-10-17: Jonasson·Purho "Juice it or lose it", Nijman "The Art of Screenshake", Swink *Game Feel*).

### 5. 대조 — CL9 에 있는지 없는지

관련 글이 꼽은 기법 하나하나를 CL9 의 **코드·표·에셋**으로 확인한다. 문서만 보고 "있음" 이라고 쓰지 않는다.

- 코드: `Grep` — 예 `HitStopTime|HitFlashTime|HitPuffSystem` (`C:\Work\CL9\Source\CL9`), `SetRelativeScale3D|SetWorldScale3D` (스쿼시 유무), `NS_|NiagaraSystem` (파티클 종류).
- 에셋: `Glob` — 예 `C:\Work\CL9\Content\**\{AN_Farmer_FPPickup,WBP_MonsterHp,NS_CL9_HitPuff,CS_HitShake}*` (명세는 있는데 에셋이 없는 것을 가른다).
- 표·결정: `C:\Work\CL9\docs\plan\data\rules.md` · `docs\systems\<시스템>.md` · `docs\decisions.md` (수치와 의도적 반려 — 예 래그돌은 game-feel.md 가 "Rejected").
- 같은 날의 다른 결론과 관계를 머리말에 적는다(2026-10-17: combat-gap.md 가 "밍밍함의 주범은 구조" 라고 했으므로 이 표는 표현 층 후보일 뿐이라는 전제).

판정은 **있음 / 없음 / 부분 / 다름 / 유지(의도적 반려)** 중 하나이고, 근거 열에 파일을 적는다. 없음·부분은 번호를 매겨 3절 후보가 된다.

### 6. 문서 쓰기

`C:\Work\CL9\docs\reference\<주제>-social.md` 로 쓴다(2026-10-17: `game-feel-social.md`). 절 구조는 아래 "산출물 형식" 그대로.
쓴 뒤 `C:\Work\CL9\docs\reference\README.md` 의 "어디를 볼 것인가" 표에 한 줄을 넣는다
(형식: `| **<주제> — <무엇을 대조했나, 빈칸 n개>** (<날짜>) | [<파일>](<파일>) (인스타그램, 두 게임과 무관) | |`).

보고는 모은 수 → 관련 수와 뺀 이유 → 못 본 것 → 대조 결과(있음이 몇, 빈칸이 몇) → 가장 싼 후보 순서로. 커밋은 하지 않는다.
분석 턴이므로 끝에 "구현" 버튼을 붙이지 않는다(전역 CLAUDE.md 규칙).

## 산출물 형식 — 결과 문서의 절 구조

[C:\Work\CL9\docs\reference\game-feel-social.md](C:\Work\CL9\docs\reference\game-feel-social.md) 와 같게 한다.

```
# 참고 — SNS(인스타그램)의 <주제> 글을 CL9 에 대본 기록 (<날짜>)
  머리말: 사용자가 무엇을 보고 무엇을 시켰나, "로그인 없이 읽을 수 있었던 만큼만" 이라는 범위,
          결정은 여기 없다(decisions.md·rules.md 로 보냄), 같은 날의 다른 결론과의 관계(전제 인용 상자)

## 1. 무엇을 어떻게 모았나, 어디까지 닿았나
  | 경로 | 결과 |   — 프로필(로그인 없음) / 해시태그 탐색(태그별 개수, 0개인 태그) / 스토리(접근 불가) / 합계(모은 수 → 관련 수, 뺀 이유 한 줄)
  캡션 기준이라는 한 줄 + 영상은 YouTube Shorts·Threads 쪽이 로그인 없이 열린다는 안내
  ### 관련 게시물 N개
  | 계정·날짜 | 캡션 요지 (캡션 그대로, 영상은 못 봄) | 링크 |   — 링크는 게시물 ID 를 글자로 한 인스타 URL
  같은 계정의 무관한 글은 주제만 나열하고 "무관" 한 줄
  ### 보강 — 이 릴들이 요약하고 있는 원전
  강연·책 목록(제목·연도·무엇을 보여주나)

## 2. 기법별 CL9 대조표
  "CL9 현재는 <날짜> 코드·표 기준, 근거 열의 파일이 정본" 한 줄
  ### 2.1 <글이 직접 꼽은 것>   | 기법 | 릴/원전이 말하는 것 | CL9 현재 | 근거 | 판단 |
  ### 2.2 <원전의 나머지 항목>  같은 열
  ### 2.3 <그 밖에 수집 글이 짚은 것>  | 기법 | 글 | CL9 현재 | 근거 | 판단 |
  판단 열: 있음 / 없음·후보 ① / 부분 / 다름 / 유지(의도적 반려)

## 3. 적용 후보 (결정 아님 — 사용자가 고른다)
  "전부 판정·수치를 건드리지 않는 표현 층, 조절 수치는 CL9Rules 열로 뺀다" 한 줄
  | # | 후보 | 어디를 만지나 | 새 표 열(초안) | 크기 |
  우선순위 한 단락(가장 싼 것, 명세만 있고 미제작인 것)

## 4. 이 조사의 한계
  영상을 못 봤다 / 표본이 작다(해시태그 상위 12개뿐, 무관한 글이 섞임) / 캡션 원문은 스크래치패드에만 두고 저장소에 안 둔다
```

## 지켜야 할 것

- **로그인하지 않는다.** 로그인 벽·레이트 제한을 우회하지 않는다. 벽 바깥에 보이는 것만 읽고, 못 본 것은 문서에 "못 봤다" 고 적는다.
- **계정 정보를 입력하지 않는다.** 아이디·비밀번호·인증 코드는 어떤 칸에도 넣지 않는다. 사용자가 로그인하겠다고 하면 사용자가 직접 한다(아래 절).
- **캡션 인용은 짧게, 출처 링크를 단다.** 문서에는 요지와 핵심 문장 한두 줄만 큰따옴표로 넣고 게시물 링크를 붙인다. 캡션 전문·영상·썸네일은 저장소에 넣지 않는다.
- **개인 계정 정보를 모으지 않는다.** 문서에 적는 것은 계정 이름·게시 날짜·공개 캡션 요지뿐이다. 팔로워 수·팔로잉·프로필 소개·댓글 작성자는 모으지도 적지도 않는다.
  (2026-10-17 에는 프로필 페이지 텍스트에 팔로워 수가 보였지만 문서에 적지 않았다.)
- 수집 원문(`links.txt` `posts.jsonl` `posts.md`)은 세션 스크래치패드에만 둔다.
- 인스타 내부 API(`/api/v1/...`)를 부르지 않는다(2차 구간에서 한 번 시도했고 404 였다. 기록에 있는 사실일 뿐, 쓰는 절차가 아니다).

## 로그인된 탭이 있을 때 (2026-10-17 2차 — 선택 경로, 기본이 아니다)

2차 구간에서 사용자가 **앱 안의 브라우저 패인에 직접 인스타 로그인을 해 두었다**고 알려 와서, 그 탭(`mcp__Claude_Browser__*`, `tabId tab-1`)을 썼다.
사용자가 그렇게 말한 원문은 기록에 남아 있지 않다(어시스턴트의 "로그인을 해 두셨다는 말씀 확인했습니다" 만 있음 — 미확인).
사용자가 먼저 그렇게 말한 경우에만 이 경로를 쓰고, Claude 가 로그인하거나 로그인을 권하지 않는다. 그 탭은 사용자의 세션이므로 로그아웃·설정 변경을 하지 않는다.

그때 확인된 것:

| 할 수 있었던 것 | 안 됐던 것 |
|---|---|
| 릴 영상 재생 → `<video>` 를 canvas 에 시간별로 그려 **프레임 시트**를 만들고 `computer zoom` 으로 보기(스쿼시가 몇 프레임·얼마나 눌리는지 실측) | 키워드 검색 `https://www.instagram.com/explore/search/keyword/?q=<검색어>` — 결과가 적고 음악·장난감 광고 등 엉뚱한 글이 섞여 릴 한 편 외에 쓸 만한 게 없었다 |
| 검색 결과 격자에서 `/p/` 링크와 `img.alt`(캡션 일부) 읽기 | 해시태그 `explore/tags/<태그>/` 가 `#태그` 키워드 검색으로 리다이렉트되어 **0개** (로그인 없는 1차에서는 12개였다) |
| 게시물 페이지를 `fetch` 해 `<meta name="description">` 읽기 | 내부 API `/api/v1/fbsearch/web/top_serp/` 404 |

프레임 시트 함수는 세션 cc42bb21 기록(2026-10-17 15:15~15:16, `window.__sheet2`)에 있다. 요지: `video.pause(); muted=true` → `currentTime` 을 `seeked` 이벤트로 옮겨 가며 `drawImage` 로 canvas 격자에 그리고 시각을 글자로 얹는다 → 끝나면 `#__sh` 를 지운다.
이 경로로 본 내용도 문서에는 "로그인한 탭에서 영상을 봤다" 고 밝히고, 영상 프레임 자체는 저장소에 넣지 않는다.

## 미확인 목록 (기록에 없어 채우지 않은 것)

1. 레이트 제한 — 48개 넘게 긁었을 때 차단되는지, 차단되면 어떤 응답이 오는지. 1차는 2초·1.5초 간격 말고는 조치가 없었고 48개까지 차단이 없었다.
2. gstack `/browse` 헤드리스 브라우저가 쓰는 프로필·쿠키 상태(로그인 안 된 상태였다는 것만 결과로 확인).
3. 해시태그 탐색 페이지 상위 12개의 정렬 기준.
4. 사용자가 2차에서 로그인했다고 말한 원문과, 어느 계정으로 로그인했는지.
5. 썸네일(`og:image`)과 스크린샷(`ig_reel.png`) 활용 — 1차에서 한 장 받았지만 문서에 쓰지 않았다. 판정에 썼는지는 기록에 없다.
6. 48개 안에서 `title` 이 빈 게시물(못 읽은 것)이 나왔을 때의 처리 — 1차는 0개여서 겪지 않았다.
