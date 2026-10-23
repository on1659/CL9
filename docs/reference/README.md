# 참고 게임 분석 — 색인

사용자가 참고 게임으로 지목한 **R.E.P.O.**와 **Lethal Company**의 설치본을 직접 뜯어
"무엇이 있고, 어떻게 동작하고, 어떻게 관리되는가"를 영역별로 정리한 기록이다.
앞으로 CL9의 같은 영역을 만들 때 옆에 놓고 보라고 만든 문서다.

> **에셋을 가져오지 않는다.** 구조와 수치만 참고한다. 이 폴더의 어떤 문서도 상대 게임의
> 코드·메시·텍스처를 옮겨 적지 않는다. CL9의 결정은 이 폴더가 아니라
> [../plan/data/](../plan/data/)와 [../decisions.md](../decisions.md)에 있다.

```
docs/reference/
  README.md      이 문서
  takeaways.md   CL9에 무엇을 가져올 것인가 — 두 게임 종합
  repo/          R.E.P.O. 영역별 분석 7개 (조작감·프런트엔드 2026-10-05 추가)
  lethal/        Lethal Company 영역별 분석 7개 (조작감·프런트엔드 2026-10-05 추가)
  image/         참고 이미지 (게임 분석과 별개)
```

---

## 어디를 볼 것인가

| 알고 싶은 것 | R.E.P.O. | Lethal Company |
|---|---|---|
| 맵이 어떻게 만들어지나 | [repo/repo-map.md](repo/repo-map.md) | [lethal/lethal-map.md](lethal/lethal-map.md) |
| 아이템·장비·상점 | [repo/repo-items.md](repo/repo-items.md) | [lethal/lethal-items.md](lethal/lethal-items.md) |
| **전투·조작감: 입력→판정→반응** | [근접 물리와 CL9 비교](lethal/lethal-combat-feel.md) | [삽 직접 분석과 CL9 차이](lethal/lethal-combat-feel.md) |
| **전투가 밍밍한 이유: 결정·비용·위험·감지·놀람 장치의 구조 비교** (2026-10-17) | [combat-gap.md](combat-gap.md) (두 게임 종합) | |
| **무섭게 만드는 장치의 실값: 공포 게이지·스팅어·돌아보기·등장 주기** (2026-10-17) | [scare-mechanics.md](scare-mechanics.md) (두 게임 종합) | |
| **몬스터마다 풀어야 할 규칙 — 네 게임 종별 패턴(텔·규칙·결과·대응·숙련·처치 가능)과 CL9 5종 규칙 카드 초안** (2026-10-17 저녁) | [monster-rules-comparison.md](monster-rules-comparison.md) (리썰·리포·Content Warning·밸하임 종합) | |
| **SNS 의 손맛(game feel) 글 — 히트스톱·스쿼시·셰이크·파티클을 CL9 에 대조, 빈칸 6개** (2026-10-17) | [game-feel-social.md](game-feel-social.md) (인스타그램, 두 게임과 무관) | |
| **SNS 의 전투·타격 글(보스 공격 예고·몬스터 개성·근접 전투) — CL9 에 대조, 새 빈칸 없음** (2026-10-19) | [combat-social.md](combat-social.md) (인스타그램, 두 게임과 무관) | |
| **1인칭 조작감: 이동 곡선·스태미나·시점·카메라·소리 실측 수치** (2026-10-05) | [repo/repo-control-feel.md](repo/repo-control-feel.md) | [lethal/lethal-control-feel.md](lethal/lethal-control-feel.md) |
| **프런트엔드·세션 흐름: 부팅·메뉴·호스트·로비·사망·게임오버·복귀** (2026-10-05) | [repo/repo-frontend.md](repo/repo-frontend.md) | [lethal/lethal-frontend.md](lethal/lethal-frontend.md) |
| 훔쳐 나르는 물건과 운반 물리 | [repo/repo-valuables.md](repo/repo-valuables.md) | (lethal-items.md 안) |
| 몬스터와 함정 | [repo/repo-monsters.md](repo/repo-monsters.md) | [lethal/lethal-monsters.md](lethal/lethal-monsters.md) |
| 스테이지 루프·쿼터·저장·멀티 | [repo/repo-run.md](repo/repo-run.md) | [lethal/lethal-run.md](lethal/lethal-run.md) |
| **CL9에 무엇을 가져올 것인가** | [takeaways.md](takeaways.md) (두 게임 종합) | |

추가 분석 게임 (2026-10-17, 전투·공포 보강용 — `Tools/UnityAnalysis` MCP 로 디컴파일·에셋 실값):

| 알고 싶은 것 | Content Warning (놀람·공포) | Valheim (전투 결정) |
|---|---|---|
| 몬스터 감지·등장·공격 종류(HP 인가 통제권 상실인가)·소음 모델·점프스케어 조건 | [contentwarning/contentwarning-monsters.md](contentwarning/contentwarning-monsters.md) | [valheim/valheim-monsters.md](valheim/valheim-monsters.md) (감지·경계·은신·레벨·드롭) |
| 도구가 몬스터에게 하는 일·손전등·**공포가 보상인 구조(카메라 점수)** | [contentwarning/contentwarning-items.md](contentwarning/contentwarning-items.md) | [valheim/valheim-combat.md](valheim/valheim-combat.md) (공격 체인·스태미나·스태거·막기·패리·무기 역할·내성) |
| 한 판 루프·산소·사망·부활 | [contentwarning/contentwarning-run.md](contentwarning/contentwarning-run.md) | |

eference\mimesis\mimesis-monsters.md](C:\Work\CL9\docs
eference\mimesis\mimesis-monsters.md) · 도구·손전등·구출·오염도 게이지와 화면 효과 [C:\Work\CL9\docs
eference\mimesis\mimesis-items.md](C:\Work\CL9\docs
eference\mimesis\mimesis-items.md) · 정비소→트램→던전 루프·죽음·빙의·난이도 [C:\Work\CL9\docs
eference\mimesis\mimesis-run.md](C:\Work\CL9\docs
eference\mimesis\mimesis-run.md)

| 알고 싶은 것 | MIMESIS (변장·의심 — 2026-10-17 저녁) |
|---|---|
| 몬스터 15종 규칙·**변장(미메시스) 시스템**(목소리 녹음·신뢰도·오염도)·감지(반지름, 빛·발소리 없음) | [mimesis/mimesis-monsters.md](mimesis/mimesis-monsters.md) |
| 도구(방망이·샷건·탐지기)·손전등 배터리·오염도 게이지·화면 효과 | [mimesis/mimesis-items.md](mimesis/mimesis-items.md) |
| 정비소 → 트램 → 던전 루프, 시간 압박 셋, 관전·빙의, 난이도 표 | [mimesis/mimesis-run.md](mimesis/mimesis-run.md) |

CL9 쪽 대응 문서로 건너뛰려면 [takeaways.md](takeaways.md)가 빠르다. 주제별로
"저쪽은 이렇게 했고, 우리 표의 어느 칸이 그 자리인가"를 묶어 두었다.

---

## 두 게임은 어떻게 다른가

같은 장르(협동 추출)인데 맵을 만드는 방식이 정반대다. 이 차이가 나머지 설계를 전부 끌고 간다.

| | Lethal Company | R.E.P.O. |
|---|---|---|
| 생성기 | 기성품 `DunGen.dll` | 자체 `LevelGenerator` |
| 배치 | 자유 타일 그래프, 출입구가 임의 위치 | **정사각 격자**, 변 중앙 사방 소켓 |
| 충돌 | 붙여 보고 바운드 충돌 검사 | 격자 칸 점유 검사 |
| 크기 조절 | 달마다 `factorySizeMultiplier` (0.35~2.2) | 완료한 레벨 수로 모듈 개수 (5~15) |
| 다양성의 출처 | 타일 종류 자체가 많다 (60~80) | 타일 311개 + **소켓 연결에 따른 소품 교체** |
| 층 | 개념 없음, 계단이 타일 안에 | 개념 없음, 미니맵만 4m 단위로 나눠 그림 |
| 목적지 | **달 14곳**을 골라 간다 | 테마 4개 중 무작위, 고를 수 없다 |
| 목표 | 며칠 단위 **쿼터(금액)** | 레벨마다 **목표 금액** |
| 네트워크 | Netcode for GameObjects, 시드 공유 | Photon, 호스트가 결과를 룸 오브젝트로 복제 |
| CL9 적합도 | 이식이 어렵다 | **CL9이 이미 격자를 갖고 있다** |

**둘 다 트럭·우주선이 레벨 밖에 있고 플레이어가 걸어 들어간다.** CL9의 "트럭은 밖 고정"
결정과 같다. 다만 판매·정산 지점의 위치는 다르다 — 리썰컴퍼니는 물건을 우주선에 실어 회사
건물까지 **가져가서** 팔고, R.E.P.O.는 레벨 **안**의 추출 지점 여러 곳에 날라다 넣는다.

---

## 어떻게 조사했나

두 게임 모두 유니티이고, 도구 없이도 꽤 많이 읽힌다. 다시 조사할 일이 생기면 이 순서로 한다.

1. **`*_Data/Managed/`의 DLL 목록**만 봐도 엔진 구조가 드러난다. 리썰컴퍼니에는 `DunGen.dll`이
   있고 R.E.P.O.에는 없다 — 그것만으로 생성 방식이 갈린다.
2. **`Assembly-CSharp.dll`을 디컴파일한다.** `ilspycmd -p -o <출력> -r <Managed> <dll>`이면
   클래스당 `.cs` 하나로 프로젝트가 통째로 나온다. R.E.P.O. 963개, 리썰컴퍼니 386개였다.
   이 폴더의 동작 설명은 대부분 여기서 나왔다.
3. **에셋의 실제 수치는 `UnityPy` + `TypeTreeGeneratorAPI`로 읽는다.** IL2CPP가 아니라 Mono
   빌드라 타입트리를 DLL에서 생성할 수 있고, 그러면 `MonoBehaviour`(ScriptableObject 포함)의
   필드가 이름 그대로 나온다. 코드의 필드 초기값과 에셋의 실제 값은 다르므로 **밸런스 수치는
   반드시 이쪽을 봐야 한다.** 리썰컴퍼니에서 아이템 58종·몬스터 33종·달 14곳의 실측값을 이렇게 얻었다.
4. **에셋 이름만 훑어도 설계도가 나온다.** R.E.P.O.의 `Module - {테마} - {타입} - {티어} - {이름}`
   311개를 집계해 테마별·타입별 개수를 얻었다. 명명 규칙이 곧 구조다.

2026-10-17부터 이 절차는 MCP 도구 `unity-analysis`로 굳어 있다. 디컴파일, 클래스 검색, 에셋 실제 값 읽기,
코드값과 에셋값 비교를 도구로 부르면 근거(파일·줄 번호·`path_id`)가 함께 나온다. 사용법은
`C:\Work\CL9\Tools\UnityAnalysis\README.md`에 있다.

**확인과 추정을 섞지 않는다.** 각 문서는 코드에서 직접 읽은 것만 단정하고, 나머지에는
"(추정)"을 붙였다. 이전 세션이 DLL 문자열만 보고 내렸던 결론 중 몇 개는 이번에 코드로
확인하면서 **정정**됐고, 해당 문장에 그렇게 적어 두었다.

### 조사한 설치본

| 게임 | 경로 |
|---|---|
| R.E.P.O. | `D:\SteamLibrary\steamapps\common\REPO` (이 노트북에서는 `C:\Program Files (x86)\Steam\steamapps\common\REPO`) |
| Lethal Company | `C:\Program Files (x86)\Steam\steamapps\common\Lethal Company` |

두 설치본 모두 Thunderstore 모드 매니저의 흔적(`winhttp.dll`, `doorstop_config.ini`)이 있지만
게임 폴더 안에 `BepInEx`가 없다. 모드는 런타임에 패치할 뿐 `.assets`를 고쳐 쓰지 않으므로,
실측값은 게임이 배포한 데이터로 본다.
