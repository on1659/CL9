# 설정 시스템 (로컬 옵션)

> 각 플레이어의 머신에 저장되는 개인 설정. **판 세이브([save.md](save.md))와 완전히 분리** —
> 설정은 로컬, 진행은 호스트의 것이다. 무엇을 옵션으로 여는지는 [../plan/settings.md](../plan/settings.md)가
> 정하고, 이 문서는 저장·적용·화면·확장 방법이다 (2026-09-12 구현 — [../decisions.md](../decisions.md)).

## 원칙

- **게임플레이 값을 노출하지 않는다.** 난이도·밸런스·시야 이득이 되는 옵션 금지 — plan/settings.md 체크리스트.
- 설정 UI는 `WBP_Main` 하위 Screen 행 `Settings` ([ui_system.md](ui_system.md)) — 메뉴 [설정]에서 연다
  (타이틀 화면이 생기면 거기서도).
- **바꾸는 순간 적용하고 그 자리에서 저장한다.** 화면 닫힘에 저장을 걸지 않는다 — `PlayOutro`는 비가상이고
  `NativeDestruct`는 페이드 뒤에 오며, 새벽 멀티캐스트·`Client_OpenScreen`이 Screen 배타로 설정 화면을 언제든
  밀어낼 수 있다.

## 저장 구조

| 무엇 | 어디 | 파일 |
|---|---|---|
| 키 리바인딩 | 엔진 **Enhanced Input User Settings** (`UEnhancedInputUserSettings`, 5.8 기본 켜짐, 서브클래스 없음) | `Saved/SaveGames/EnhancedInputUserSettings.sav` |
| 감도·Y반전·볼륨 6개 | `UCL9GameUserSettings : UGameUserSettings` (`config` 필드, `DefaultEngine.ini [/Script/Engine.Engine] GameUserSettingsClassName`) | `Saved/Config/.../GameUserSettings.ini` 섹션 `[/Script/CL9.CL9GameUserSettings]` |
| 언어 | 엔진 표준 키 (커스텀 코드 없음) | 같은 파일 `[Internationalization] Culture` |
| 그래픽(해상도·창·품질) | `UGameUserSettings` 표준 항목 — 화면에는 아직 없음 | 같은 파일 |
| 판 진행 | ❌ 여기 아님 — [save.md](save.md) | |

- `GameUserSettingsClassName`을 바꾸면 비-GlobalConfig 프로퍼티의 ini 섹션이 클래스 경로로 바뀐다 —
  개발 머신마다 **해상도 값이 한 번 초기화**된다(1회성, 정상).
- 저장 함수는 `UGameUserSettings::SaveSettings()`(=SaveConfig)만 부른다. **`ApplySettings()`·
  `ApplyNonResolutionSettings()`는 부르지 않는다** — Scalability를 프로세스 전체에 적용해 PIE에서 에디터
  뷰포트 품질까지 바꾸고 해상도 요청을 낸다. 우리 적용은 `ApplyAudio`·`ApplyLanguage`와 LookInput의 곱뿐이다.
- `SetToDefaults` 오버라이드는 `Super` 호출 + 필드 초기화만 — `UEngine::CreateGameUserSettings`가 생성 직후
  부르므로 상태에 기대면 안 된다.

## 항목과 적용 경로

### 키 (Enhanced Input User Settings)

전부 [input.md](input.md) "리바인딩" 절 — 등록 시점(`ACL9PlayerController::BeginPlay`), 매핑 이름, 슬롯 규칙,
게임패드 제외, 캡처·충돌 규칙. 화면은 아래 "설정 화면"에서 그 API를 부를 뿐 규칙을 갖지 않는다.

- 행은 `IMC_Gameplay` 매핑 순서대로 자동 나열된다 — 액션을 더하거나 빼도 화면 코드는 바뀌지 않는다
  (2026-09-13 퀵슬롯: "놓기 G"·"1번 칸"~"4번 칸" 행이 IMC에 넣은 자리에 그대로 뜬다).
- **마우스 휠(`IA_SlotScroll`, 퀵슬롯 칸 전환)은 설정에 뜨지 않고 바꿀 수도 없다.** IMC 행이 `IgnoreSettings`라
  `IsPlayerMappable()`이 false → 화면이 행을 만들지 않고, 캡처가 마우스 축/휠을 거부하므로 다른 동작에 휠을 넣을
  수도 없다. 휠 방향(아래 = 다음 칸)도 고정이다.

### 마우스

| 항목 | 필드 | 기본 | 적용 |
|---|---|---|---|
| 감도 | `MouseSensitivity` | 1.0 (화면 표시 100%) | `ACL9Character::LookInput`이 매번 읽어 축 값에 곱한다. `DefaultInput.ini`의 `AxisConfig` 0.07 위에 얹는 배율 |
| Y축 반전 | `bInvertY` | false | 같은 곳에서 Pitch 부호 반전 |

슬라이더 범위는 위젯 `UPROPERTY SensitivityRange`(기본 0.2~3.0, conventions § 1), 선형, 스텝 0.05, 정수 퍼센트로
표시. 로그 스케일·[−][+] 버튼은 플레이해보고 결정.

마우스 휠은 이 섹션에도 키 섹션에도 없다 — 퀵슬롯 칸 전환 전용이고 방향·기능 모두 고정이다(위 "키").

### 오디오

| 슬라이더 | 필드 | 대상 클래스 | 기본 |
|---|---|---|---|
| 전체 | `MasterVolume` | `SC_Master` | 1.0 |
| 효과음 | `EffectsVolume` | `SC_Effects` | 1.0 |
| UI | `UIVolume` | `SC_UI` | 1.0 |
| 배경음 | `AmbienceVolume` | `SC_Ambience` | 1.0 |
| 음악 | `MusicVolume` | `SC_Music` | 1.0 |
| 음성 | `VOVolume` | `SC_VO` | 1.0 |

- **슬라이더 기본은 전부 1.0.** 설계 게인(효과음 0.8·배경음 0.5·음악 0.7 — [sound.md](sound.md) 클래스 트리)은
  `SoundClass.Properties.Volume`에 있고 슬라이더는 그 위에 곱해진다. 80%로 보이는 기본값은 플레이어가 100%로
  올리게 만들어 믹스를 깨뜨린다 — "게임플레이 값 미노출"과 같은 정신.
- 적용: `UCL9GameUserSettings::ApplyAudio(UWorld*)` — 클래스마다 `SetSoundMixClassOverride(Mix, Class, Volume,
  1.0, 0.1s, bApplyToChildren=true)` 뒤 `PushSoundMixModifier(Mix)`. **`GameInstance::Init`에서는 못 부른다**
  (월드·오디오 디바이스가 없으면 조용히 return) — `ACL9PlayerController::BeginPlay`(로컬, `InitForPlayer` 옆)에서
  한 번, 슬라이더를 움직일 때마다 다시.
- 에셋 참조는 `UCL9SettingsTable`(DataAsset `Content/Data/SettingsTable`, `BP_CL9GameInstance`가 주입 —
  UITable과 같은 방식): `SoundMix` 1개 + SoundClass 6개 + 선택적 `PreviewSound`(확인음 에셋이 생기면 슬라이더를
  놓을 때 재생 — 지금은 비어 있음). conventions § 5 경로 예외는 `MasterSoundMix` 하나뿐이라 SoundClass는
  에셋 참조여야 한다.
- 지금은 재생되는 사운드가 하나도 없다 — 슬라이더는 믹스 오버라이드 값만 바꾼다. 앞으로 만드는 사운드 에셋은
  반드시 위 클래스 중 하나를 가져야 슬라이더가 닿는다 ([sound.md](sound.md)).

### 언어

- 선택지 `ko`·`en` (위젯 `UPROPERTY Languages`). 라벨은 `FCulture::GetNativeName()`("한국어"/"English") —
  String Table 예외([localization.md](localization.md)): 잘못 바꾼 사람이 자기 언어를 읽을 수 있어야 한다.
- 적용은 `UCL9GameUserSettings::ApplyLanguage(Culture)` 한 곳 — `cl9.lang` 치트 본문을 옮긴 것이다.
  패키지: `SetCurrentLanguageAndLocale` + `[Internationalization] Culture` 저장. **에디터/PIE**: 게임 언어
  미리보기(`EnableGameLocalizationPreview`)만 켜고 저장하지 않는다 — "바꿨는데 다음 PIE에 안 남는다"가 정상.
- 화면 자신의 문구는 FText 지연 재빌드가 다시 그린다 — 행을 다시 만들지 않는다.

## 설정 화면 (`UCL9SettingsWidget` / `WBP_Settings`)

```
[설정]                                            [X]
─ 키 ──────────────────────────────  [키 초기화]
  앞으로          [ W ]
  …              (IMC_Gameplay → IMC_Spectate 매핑 순서)
─ 마우스 ─────────────────────────────────────────
  감도            ────●──── 100%
  Y축 반전        [ ]
─ 오디오 ─────────────────────────────────────────
  전체 · 효과음 · UI · 배경음 · 음악 · 음성   슬라이더 6개
─ 언어 ───────────────────────────────────────────
  [ 한국어 | English ]
힌트 줄: 맞바꿈·거부 안내 (다음 캡처 시작 때 지움)
```

- 행은 C++가 런타임에 만든다 (트럭 패널 카드와 같은 방식). 빌더 4종: `AddSlider(라벨, 값, 범위, Setter)`
  `AddToggle(라벨, 값, Setter)` `AddChoice(라벨, 선택지들, 현재, Setter)`(선택지마다 버튼 하나인 세그먼트, 현재 값 강조)
  `AddKey(매핑)`. 버튼 페이로드는 릴레이 UObject(트럭 패널 `UCL9ShopCardRelay`와 같은 이유).
- [X]는 `UCL9Widget`의 공통 `CloseButton`(`BindWidgetOptional`) — `CloseSelf` = 히스토리 한 단계 뒤로
  ([ui_system.md](ui_system.md)). M도 같다. 설정 → 메뉴 → 게임.
- 리셋은 키 섹션의 [키 초기화] 하나(`ResetKeyProfileIdToDefault` + 저장). 감도·볼륨·언어는 손으로 되돌리는 데
  10초라 리셋 대상이 아니다. 전체 [기본값으로]·행 우클릭 리셋은 반려.
- 키 행·캡처·충돌 동작의 규칙은 [input.md](input.md) 리바인딩 절.

## 옵션 추가

plan/settings.md 체크리스트를 통과했으면 세 곳이다:

1. `UCL9GameUserSettings`에 `config` 필드 1개 + 기본값(`SetToDefaults`에도).
2. `UCL9SettingsWidget::Rebuild()`의 해당 섹션에 빌더 호출 1줄 — 라벨은 `LOCTABLE("CL9", "Settings.<이름>")`
   리터럴로 (그래야 `sync_strings.py`가 시트 누락을 잡는다). Setter 안에서 적용 + `SaveSettings()`.
3. `Content/Strings/CL9_Strings.csv`에 `Settings.<이름>` 행 → `python Tools/Loc/sync_strings.py`.

볼륨이면 `UCL9SettingsTable`에 SoundClass 참조 1개와 `ApplyAudio`의 한 줄이 더 붙는다. 새 섹션이면 헤더 키
`Settings.Section.<이름>` 하나. 화면·표·C++ 구조는 늘리지 않는다.

## 개발 중 알아둘 것

- 리바인딩은 `Saved/SaveGames/EnhancedInputUserSettings.sav`에 남는다 — 에디터를 껐다 켜도, 다음 PIE에도
  그대로다. 프롬프트가 `[{0}]`로 현재 키를 찍으므로 "왜 `[F] 줍기`지?"가 생기면 설정 화면의 **[키 초기화]**
  버튼(`HandleResetKeysClicked`) 또는 파일 삭제 — 콘솔 명령은 없다.
- **다중 PIE 인스턴스(리슨 서버+클라)는 같은 파일·같은 객체를 공유한다** — EI 저장 슬롯은 로컬 플레이어 인덱스 0
  하나이고, `UGameUserSettings`는 엔진당 객체 하나다. 한 창에서 바꾸면 다른 창도 다음 PIE부터 같이 바뀐다.
- 언어 행은 PIE에서 미리보기만이다(위 "언어").

## 참고

- 기획(무엇을 열고 무엇을 안 여는가): [../plan/settings.md](../plan/settings.md)
- 키 규칙·캡처·충돌: [input.md](input.md) 리바인딩 · 문구 서식 `{0}`: [localization.md](localization.md)
- 볼륨 믹스: [sound.md](sound.md) "볼륨 관리" · 감마 제한의 근거(그래픽 섹션이 생기면): [light.md](light.md)
- 설정 화면 위치: [ui.md](ui.md) 메뉴 → [설정] · 화면 스택: [ui_system.md](ui_system.md)
