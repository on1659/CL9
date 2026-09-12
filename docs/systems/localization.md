# 문자열·다국어 시스템 (String Table `CL9`)

> 플레이어에게 보이는 문자열은 **전부 String Table `CL9`의 키**다. 코드·UMG·데이터 표 어디에도
> 한글을 직접 치지 않는다 — 한글 원문은 `Content/Strings/CL9.csv` 한 곳에만 있고, 언어를 바꾸면
> 그 키들이 한꺼번에 다른 언어로 바뀐다 (2026-09-12 사용자 — "한글을 박는게 아니라 string alias를
> 박아서 언어설정 바뀔때마다 싹다 바뀌어야").
>
> **새 문자열을 쓸 때마다 이 문서를 따른다.** 규칙 요지는 [../conventions.md](../conventions.md) § 6.

## 구조

**사람이 고치는 파일은 `Content/Strings/CL9_Strings.csv` 하나다** — 한 줄이 문구 하나, 열이 언어다
(2026-09-12 사용자 "나라별로 바꿀 수 있게"):

| No | Key | DevNotes | ko | en | ja | zh-Hans |
|---|---|---|---|---|---|---|
| 12 | `Hud.Money` | 공유 자금. {0}=금액. "이더"는 이 게임의 화폐 단위 | `{0} 이더` | `{0} ether` | | |

- `No` — 행 번호(기획서·대화에서 "12번 문구"로 가리키는 용도, 정렬 기준). 새 행은 마지막 번호+1.
- `Key` — 코드·WBP·데이터 행이 참조하는 이름 (아래 키 규칙). **바꾸지 않는다.**
- `DevNotes` — **어디서 쓰는지가 아니라 어떤 뜻·상황의 문구인지**, 서식 인자 `{0}`이 무엇인지. 번역자가 이것만 보고
  옮길 수 있어야 한다 (사용자 2026-09-12).
- `ko` — 원문(필수). 나머지 언어 열은 비워도 된다 — 빈 칸은 그 언어에서 ko로 보인다.
- 새 언어는 열을 하나 더 붙이고 `Config/Localization/CL9_*.ini`·`DefaultEditor.ini`·`DefaultGame.ini`의 문화권 목록에
  같은 이름(UE 문화권 코드: `ja`, `zh-Hans`, `de`…)을 넣으면 된다.

시트에서 나머지 파일은 스크립트가 만든다 — **`python Tools/Loc/sync_strings.py --build`** (에디터 켠 채로 가능, 약 3분):

| 무엇 | 어디 | 역할 |
|---|---|---|
| 런타임 String Table | `Content/Strings/CL9.csv` (`Key,SourceString,DevNotes`) | 시트의 `ko` 열에서 **생성** — 손대지 말 것. `Source/CL9/CL9.cpp` `FCL9GameModule::StartupModule`이 `LOCTABLE_FROMFILE_GAME("CL9", "CL9", "Strings/CL9.csv")`로 테이블 `CL9`(네임스페이스 `CL9`)로 등록 |
| 로컬라이제이션 타깃 | `Config/Localization/CL9_*.ini`, `Config/DefaultEditor.ini` | 수집·내보내기·가져오기·컴파일. 문화권 `ko`(원문)·`en`·`ja`·`zh-Hans` |
| 번역 | `Content/Localization/CL9/<culture>/CL9.po` → `.locres` | 시트의 언어 열을 `.po`에 채운 뒤 컴파일한 산출물 (커밋한다) |
| 런타임 경로 | `Config/DefaultGame.ini` `[Internationalization] LocalizationPaths`, `ProjectPackagingSettings` | `.locres` 경로, 패키징 문화권, CSV 스테이징(`DirectoriesToAlwaysStageAsUFS=Strings`) |
| 언어 전환 | `cl9.lang [ko\|en\|ja\|zh-Hans]` ([cheats.md](cheats.md)) | 지금 바꾸고 `GameUserSettings.ini`에 저장. 설정 화면 언어 항목의 대역 |

`sync_strings.py`는 시트를 검증한다 — 키·번호 중복, `ko` 빈칸, 언어별 `{n}` 인자 불일치, **코드의 `LOCTABLE("CL9", …)` 키가
시트에 없음(오타)** 이 있으면 멈추고 파일·줄 번호를 찍는다. 오타는 빌드를 통과하고 화면에 키 문자열로 새기 때문에 여기서 잡는다.
`--build` 없이 돌리면 `CL9.csv`만 다시 만든다(1초) — 원문만 고쳤을 때는 이것으로 충분하고, 에디터가 켜져 있으면
저장 즉시 다시 읽는다(`FStringTableRegistry`의 디렉터리 감시).

왜 CSV인가: String Table 에셋(`.uasset`)은 바이너리라 diff·리뷰가 안 되고 에디터 없이는 못 고친다. 소스 수집기
(`GatherTextFromSource`)가 `LOCTABLE_FROMFILE_GAME` 매크로를 따라가 CSV 행을 전부 긁고, 에디터의 텍스트 속성
피커에도 이 테이블이 그대로 뜬다. `Content/Strings/*`는 에디터 자동 리임포트 감시에서 뺐다
(`DefaultEditorPerProjectUserSettings.ini`) — 안 빼면 CSV를 CurveTable로 임포트하려다 실패하고 파일을 잡아 둔다.

## 키 규칙

`영역.이름` 꼴, 점(.)으로 구분, 각 토막은 PascalCase. **키는 한 번 번역되면 바꾸지 않는다**
(바꾸면 그 키의 번역이 전부 버려진다) — 새 뜻이면 새 키를 만든다.

| 영역 | 예 | 쓰는 곳 |
|---|---|---|
| `Prompt.*` | `Prompt.PickUp` `[E] 줍기` | 상호작용 프롬프트 (`GetInteractPrompt`) |
| `Hud.*` | `Hud.Money` `{0} 이더`, `Hud.PocketSep` `, ` | HUD 본체 (`UCL9HUDWidget`) |
| `Stat.*` | `Stat.RunSpeed` `달리기` | 장비 효과 이름 |
| `Shop.*` `OrderSelect.*` `Dawn.*` `Spectate.*` `ScanLabel.*` `Pause.*` `Settings.*` | `Shop.NoMoney` `잔액 부족` | 화면(WBP) 하나 = 영역 하나. WBP에 직접 놓는 정적 텍스트도 같은 영역 |
| `Input.*` | `Input.PickPlace.Name` `줍기 / 놓기` | 리바인딩 가능한 동작의 이름 — IA 에셋 `PlayerMappableKeySettings.DisplayName`이 참조 (`Input.<매핑 이름>.Name`, [input.md](input.md) 리바인딩) |
| `Key.*` | `Key.SpaceBar` `Space`, `Key.LeftMouseButton` `LMB` | 키 표기 — `Key.<FKey 이름>`. **시트에 없으면 엔진 영문 원문으로 폴백**(`CL9Input::GetKeyText`)하는 유일한 파생 키 영역. 엔진 ko 번역은 "스페이스"·"왼쪽 Shift"·"LMB"로 섞이고 에디터 미리보기는 Engine 타깃을 싣지 않아 PIE와 패키지가 달라서 우리 표기를 둔다. 문자 키(E·F)는 행이 없어도 영문 그대로 |
| 데이터 행 | `Item.egg.Name`, `Item.egg.cooked.Name`, `Item.egg.Desc`, `Monster.goose.Name`, `SegKind.floor.Name` | `<종류>.<Alias>[.<SubAlias>].Name` / `.Desc`. 종류는 Def 클래스 이름에서 `UCL9`/`Def`를 뗀 것(Item·Monster·Order·SegKind·Site·Buff·Character·Equipment·Module·Piece) |

- 서식 인자 `{0}`·`{1}`은 원문에 그대로 둔다 — `FText::Format`이 키 쪽 원문/번역 둘 다에서 채운다.
- **키 표기는 박지 않는다** (2026-09-12 사용자 "[E] 이런 식으로 박혀 있더라고… {0} 해서 단축키를 바꾸면 같이 적용"):
  `Prompt.PickUp` "[{0}] 줍기", `Spectate.Next` "다음 동료 ({0})" — `{0}`은 현재 바인딩된 키 표기이고 괄호·위치는
  문자열(번역자)이 정한다. 채우는 쪽은 `CL9Input::GetActionKeyText` 한 곳([input.md](input.md) 키 표기).
  인자 수와 순서는 코드와 CSV가 같아야 한다(다르면 컴파일 단계 `bValidateFormatPatterns`가 잡는다).
- 구분자(`" · "`, `", "`)와 화폐 표기(`이더`)처럼 언어마다 다를 수 있는 것도 키다.
- 언어 선택지의 이름(설정 화면 "한국어"/"English")은 `FCulture::GetNativeName()`이다 — 잘못 바꾼 사람이 자기
  언어를 읽을 수 있어야 하므로 번역하지 않는 예외([settings.md](settings.md)).
- 번역 대상이 **아닌** 것: 플레이어 이름, 숫자(`FText::AsNumber`), 줄바꿈 `\n` join, 액터 디버그 이름
  (`ACL9ActorBase::GetDisplayName`의 `GetName()` 폴백), 로그·치트 출력, `DevNotes`. 이것들만
  `FText::FromString`을 쓴다.

## 문자열 추가 — 세 자리

### 1. C++
```cpp
#include "Internationalization/StringTableRegistry.h"
Text->SetText(LOCTABLE("CL9", "Shop.NoMoney"));
Text->SetText(FText::Format(LOCTABLE("CL9", "Hud.Money"), Money));   // 원문 "{0} 이더"
```
`NSLOCTEXT`/`LOCTEXT`/`INVTEXT`는 쓰지 않는다 (`grep -rn NSLOCTEXT Source/CL9`가 0이어야 한다).
그리고 `Content/Strings/CL9_Strings.csv`에 `135,Shop.NoMoney,못 사는 이유 — 돈 부족,잔액 부족,Not enough ether,,` 한 줄을
넣고 `python Tools/Loc/sync_strings.py`(원문만) 또는 `--build`(번역까지)를 돌린다.
시트에 없는 키는 화면에 키 문자열 그대로 뜬다 — 눈에 보이는 버그라서 놓치지 않는다.

### 2. UMG (WBP 디자이너)
텍스트 속성 오른쪽 ▾ → **String Table** → 테이블 `CL9`, 키 선택. 텍스트 칸에 한글을 직접 치지 않는다.
C++이 런타임에 덮어쓰는 자리표시자도 같은 키를 고른다(디자이너 미리보기가 원문으로 보인다).
시트에 새 키를 넣고 `sync_strings.py`를 돌려야 피커에 뜬다.

### 3. 데이터 표 행 (`DisplayName` / `Description`)
컬럼은 FText 그대로이고 값이 `CL9` 테이블 참조여야 한다 — 에디터에서 2번과 같은 피커로 고른다.
`UCL9DefBase::Validate`가 **테이블 참조가 아닌 텍스트를 에러로 잡아 행을 Broken**으로 만든다
(`cl9.list`에 `✗ DisplayName is typed text…`). 행을 추가하는 순서: 시트에 `Item.<alias>.Name` 줄 추가 → `sync_strings.py` →
행의 DisplayName에서 그 키 선택. 스크립트로 일괄 처리할 때는
`unreal.TextLibrary.text_from_string_table("CL9", key)` 값을 `set_editor_property`로 넣는다
(`Tools/Unreal/convert_text_to_string_table.py` 참고 — 2026-09-12 기존 행·WBP 일괄 변환에 쓴 것).

## 번역 — 언어 열 채우기 / 갱신

**평소 작업은 한국어(`ko` 열)만 채운다.** 다른 언어 열은 비워 두고, 번역은 사용자가 정한 시점에 한 번에 몰아서
한다 — 문구를 넣거나 고칠 때 번역을 신경 쓰지 않는다 (2026-09-12 사용자 "기본은 한국어로만 작업하고, 번역은 신경
쓰지 말라"). 빈 언어 열은 그 언어에서 한국어로 보일 뿐 게임은 정상이다. 번역 패스를 돌릴 때는 아래 절차대로
빈 칸(과 원문이 바뀐 행)을 채우고 `--build` 한 번.


1. `CL9_Strings.csv`의 언어 열(`en`·`ja`·`zh-Hans`…)을 채운다. 서식 인자 `{0}`은 원문과 같은 것을 그대로 쓴다(순서는 바꿔도 된다).
2. `python Tools/Loc/sync_strings.py --build` — 내부에서 커맨드릿 네 단계를 돌린다:
   수집(`CL9_Gather.ini`: CSV·WBP·표 → manifest/archive) → 내보내기(`CL9_Export.ini` → `.po`) → 시트 값을 `.po`의 `msgstr`에 채움
   → 가져오기(`CL9_Import.ini`) → 컴파일(`CL9_Compile.ini` → `<culture>/CL9.locres`). 로컬라이제이션 대시보드에서 같은 단계를
   눌러도 되지만, 그러면 시트가 아니라 `.po`를 직접 고치는 셈이라 다음 sync에 덮인다 — **시트가 정본**이다.
3. 확인: PIE에서 `cl9.lang en` / `cl9.lang ko`.

- 수집 결과 `Content/Localization/CL9/CL9.manifest`에 `CL9` 네임스페이스 키만 있어야 정상이다.
  에셋 수집(`Content/UI/*`, `Content/Data/*`, `Content/Input/*`)에서 **테이블 참조가 아닌 텍스트가 잡히면 그게 규칙 위반**이다
  (자동 키가 GUID로 보인다) — 그 자리를 키로 바꾼다. `CL9_Conflicts.txt`가 비어 있어야 한다.
- `Content/Localization/CL9/**`(manifest·archive·po·locres)는 전부 생성물이지만 커밋한다 — 클론이 빌드 없이 바로 돌게.
- 2026-09-12 en·ja·zh-Hans 열 186개는 코드 작성자가 채운 **초안**이다 — 원어민 검수 전.

## 언어 전환 (런타임)

- 시작 언어: `GameUserSettings.ini [Internationalization] Culture` → 없으면 OS 언어 → 없으면 `ko`
  (`DefaultCulture=ko`). 엔진이 읽는 표준 키라 커스텀 코드가 없다.
- `cl9.lang en` → `FInternationalization::SetCurrentLanguageAndLocale("en")` + 그 키 저장. 열려 있는 위젯은
  텍스트 바인딩이 문화권 변경 콜백으로 다시 그려지고, 멤버에 FText를 캐시한 곳은 다음 갱신 때 바뀐다.
- **에디터/PIE에서는 다르다**: 에디터는 게임 번역(`Game` 로컬라이제이션)을 싣지 않고, 현재 언어를 바꾸면 에디터
  UI까지 바뀐다(`TextLocalizationManager.cpp` `EnableGameLocalizationPreview`, "This only works in the editor").
  그래서 `cl9.lang`은 `GIsEditor`면 **게임 언어 미리보기**(`FTextLocalizationManager::EnableGameLocalizationPreview`,
  에디터 설정 "Preview Game Language"와 같은 것)를 켜고 저장하지 않는다. 미리보기는 비동기 로드라 한두 프레임 뒤에
  바뀐다. 실제 저장·시작 언어 동작은 Standalone/패키지 빌드에서 본다.
- 설정 화면([settings.md](settings.md) 언어 항목)이 생기면 같은 두 줄을 부른다 — 치트는 그때 빼도 된다.

## 하지 말 것

- 코드·WBP·표에 한글(어떤 언어든) 원문을 직접 치기. String Table 에셋(`.uasset`) 만들기 — CSV 하나가 정본.
- 키를 이름 바꾸기·재사용하기. 문장을 이어 붙여 만들기(`"손: " + 이름`) — 어순이 언어마다 다르다. 항상 `{0}` 서식.
- `Content/Strings/CL9.csv`·`Content/Localization/CL9/`를 손으로 편집하기 — 전부 `CL9_Strings.csv`에서 생성된다.

## 참고

- 결정 기록: [../decisions.md](../decisions.md) 2026-09-12 "화면 문자열은 String Table 키" (같은 날 오후의
  "String Table 안 둔다"를 뒤집음)
- 데이터 컬럼: [../data/schema.md](../data/schema.md) `DisplayName`/`Description`
- 엔진 근거 (UE 5.8): `GatherTextFromSourceCommandlet.cpp:513`(CSV 수집), `STextPropertyEditableTextBox.cpp:385`
  (피커가 C++ 등록 테이블도 나열), `TextLocalizationManager.cpp:294`(GameUserSettings의 Culture 키)
