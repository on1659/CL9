# 음성 채팅 (마이크)

> 마이크 → **EOS Voice(EOS RTC)** → 말한 사람별 재생. 전송은 **Redpoint EOS Online Framework 무료판**(Blueprint 전용 — C++ 헤더가 없다)이 맡고,
> C++ 는 **누가 듣나**(호스트가 거는 뮤트), **어디서 들리나**(말하는 사람마다 하나의 자리), 말하기 방식·옵션을 정해 `BlueprintImplementableEvent` 로 블루프린트에 넘긴다.
> 기획(왜·규칙·범위 밖·남은 일)은 [../plan/voice.md](../plan/voice.md), 이번 전송층 교체 명세는 [../goal/voice-eos-redpoint.md](../goal/voice-eos-redpoint.md),
> 값은 [../plan/data/rules.md](../plan/data/rules.md) "말소리". 옛 엔진 내장 VOIP 길의 사실(E1~E13)은 [../goal/voice-chat.md](../goal/voice-chat.md)에 기록으로만 남는다.
> 규약 예외는 [../conventions.md](../conventions.md) § 9. **몬스터 청각은 이 작업에 없다**(아래 "몬스터 청각 없음").

**구현 상태(2026-10-12, 실행 `voice-eos-redpoint`, 전부 미커밋)**: C++ 쪽 구현 · 에디터 타깃 빌드 성공(62초) ·
`create_voice_assets.py` 실행으로 `SA_CL9_Voice`·`SA_CL9_VoiceSpeaker` 생성과 `SettingsTable.VoiceAttenuation`·`VoiceSpeakerAttenuation` 채움, `verify` 결과 `VOICE ASSETS VERIFY OK`(`failures: []`) ·
`BP_CL9PlayerState`(부모 `ACL9PlayerState`) 생성과 두 게임 모드의 `PlayerStateClass` 지정(브리지 그래프는 비어 있다) ·
`CL9Rules` 음성 셀 12개가 클래스에서 사라지고 `VoiceSpectatorVolume` 이 0.8 인 것 확인 · `CL9VoiceTalker.{h,cpp}` 삭제 ·
플러그인 없는 PIE(`Lvl_Henhouse` 리슨 2인)에서 자리·뮤트 로그가 나는 것 확인(아래 "치트와 로그" 끝).
아직 안 된 것: W0(EOS 개발자 포털·Redpoint 플러그인 설치·Developer Authentication Tool — 사용자 진행 중, 플러그인이 아직 `Plugins/` 에 없다) · `uproject`·ini·`Build.cs` 배선 ·
블루프린트 브리지 그래프(`BP_CL9PlayerState` 이벤트 셋, `BP_CL9GameInstance` 이벤트 — 리드가 에디터에서) · 볼륨 경로 결정(OQ3) · T0~T2 확인(귀로 듣는 T1·T2 는 플러그인과 BP 그래프가 선 뒤다).
이 문서에서 **"리드 확인 대기"** 는 Redpoint 를 설치하고 에디터에서 열어 봐야 알 수 있는 것이다 — 보고서에 없는 것을 추측해 쓰지 않았다.
검증 절차는 [../testing/pie-scenarios.md](../testing/pie-scenarios.md) Z절(T0~T2). 에셋·표 행·BP 연결은 아래 "의존하는 에셋·표 행".
**무엇이 경로를 고르나**(무전기 아이템·상점·UI)는 다음 명세다 — 이 문서의 입구는 `HostSetVoiceRoute` 하나다. 무전기의 스위치 켜고 끄기·떨어뜨리기·줍기도 그 명세의 몫이다 — 이번에는 **연결 = 켜짐**이다.

## 한눈에

```
호스트 (판정)                                                      모든 머신 (블루프린트 = Redpoint 노드)
 C++ 가 정한다                                                     ACL9PlayerState / UCL9GameInstance 의 BlueprintImplementableEvent
  ├ HostApplyVoicePlacement ── BP_ApplyVoicePlacement ───────────→ 화자의 Proximity Type · Anchor · 감쇠 에셋 (서버 설정 — 플러그인이 복제)
  ├ RebuildVoiceMutes → 듣는 사람 PS 의 목록(OwnerOnly) ──────────→ 듣는 머신이 BP_ApplyVoiceMute (듣는 쪽 로컬 뮤트)
  └ HostGame / JoinGame("lan") / LeaveGame ── BP_HostLobby … ────→ EOS 로비 = 음성의 managed 채널 (게임 연결은 Redpoint P2P)
 각 머신의 로컬: ApplyVoice ── BP_ApplyVoiceOptions ────────────→ 송신 라우팅 · 입력 볼륨 · 출력 볼륨
                 로비 입장 ── BP_ApplyVoiceAudioFeatures ───────→ 에코 제거 · 잡음 억제 · 자동 이득
 0.1초 폴링: 원격 PS 마다 BP_ReadVoiceTalking ──────────────────→ IsVoiceTalking → HUD
```

- **말하는 사람 하나에 소리 자리 하나**(핫라인 ①): 평소에는 화자의 폰, 핫라인 중에는 **받는 사람 무전기 자리(상대의 폰)에서만** 난다. 그동안 화자 곁 사람은 그 목소리를 듣지 못한다.
  핫라인은 **두 무전기 스피커의 양방향 연결**이다 — 내 말은 상대 자리 스피커로, 상대의 말은 내 자리 스피커로 나간다.
  핫라인은 뮤트를 만들지 않는다(사용자 결정 2026-10-11 오후·저녁·밤 — contract §13·§14·§15). 2026-10-12 사용자 결정으로 "화자 곁에서도 들린다" 가 "받는 사람 자리에서만 난다" 로 단순해졌다.
- **무전기는 사람과 무관하다** — 연결이 켜져 있으면 상대가 죽어 있어도 그 자리(시체 = 그 사람의 마지막 폰)에서 울린다. 폰이 아예 없으면(나감·소멸) 울릴 자리가 없다.
  말하는 쪽의 관전 규칙은 그대로다 — 죽은 사람의 말은 무전기로도 산 사람에게 들리지 않는다.
- 새 `Multicast_*` 는 없다. 새 Server RPC 는 `Server_SetVoiceRoute` 하나다. 데디케이티드 서버 분기는 없다.
- 음성은 EOS RTC 가 나른다 — 게임 연결(리슨 호스트)과 별개이고, 서버 시간 틱 비교(conventions § 9)의 대상이 아니다.
- 게임 연결은 Redpoint 의 P2P 연결 위에 선다(명세 R3). 음성 채널과 같은 로비에 묶인다.

## 전송과 세션 — EOS 로비

음성 채널은 **EOS 로비의 managed 채널**이다(명세 R2 — 로비를 만들면 채널이 함께 생기고 참가자는 기본으로 연결된다. 채널을 따로 만드는 unmanaged 방식은 전용 서버가 필요해 쓰지 않는다).
세션 없는 연결에는 **채널이 없어 음성이 없다.** `UCL9GameInstance` 가 로비를 세우고 허문다([session.md](session.md) "세션 API"). 로비 노드는 블루프린트이고 C++ 는 결정과 상태(`bLobbyJoined`)만 가진다.

| 길 | 로비 | 음성 |
|---|---|---|
| 방 만들기(`HostGame`, `cl9.host`) | 레벨을 열기 **전에** `BP_HostLobby(4)`. 블루프린트가 끝나면 `NotifyLobbyReady(bOk)` 를 부르고, 그 안에서 `OpenLevel(listen)` 을 연다 — **성공이든 실패든 레벨은 열린다**(로비가 없으면 음성만 없다) | 됨 |
| 로비 참가(`cl9.join lan`) | `BP_FindAndJoinLobby()` → `NotifyLobbyJoined(bOk, ConnectAddress)` → `ClientTravel`. 로비가 0개면 bOk=false, 경고 한 줄만 남기고 끝 | 됨 |
| 주소 접속(`cl9.join <ip>`) | 없음 — 로그 `Voice: no session — remote voice disabled on this connection` | **없음** (게임은 전과 같다) |
| 시작 레벨(메뉴 월드) | — | 없음(로비 밖) |
| Shipping | 로비를 세우는 진입부가 `#if !UE_BUILD_SHIPPING` — Steam 층(steam-release) 전까지 **로비도 음성도 없다** | 없음 |

- **`HasVoiceSession()` = `bLobbyJoined`** — "managed 채널이 있는 로비에 들어가 있다" 는 뜻이다. 옛 `NAME_GameSession` 이름 규칙은 없어졌다. 로그의 `GameSession` 은 오라클 문구를 지키려는 이름일 뿐이다.
- **이미 로비 안이면 먼저 떠난다**: `HostGame`·`JoinGame("lan")` 이 로비 안에서 불리면 `LeaveLobby()` 를 부르고, `NotifyLobbyLeft` 에서 이어서 만들기·찾기를 한다(`AfterLobbyLeft`).
- **늦게 온 알림 처리**: `CancelJoin` 뒤에 `NotifyLobbyJoined(true)` 가 오면 그 로비를 도로 떠난다. `LeaveGame` 뒤에 `NotifyLobbyReady(true)` 가 오면 로비를 떠나고 레벨은 열지 않는다.
  블루프린트의 비동기 노드는 중간에 멈출 수 없어서 C++ 가 표시만 내리고 결과가 오면 되돌린다.
- **블루프린트가 없을 때**(`IsFunctionImplementedInScript` 로 먼저 본다 — 완료 알림이 영영 오지 않으면 방이 안 열리기 때문이다):
  `BP_HostLobby` 가 없으면 경고 `Voice: '<클래스>' does not implement BP_HostLobby — hosting with no voice chat.` 후 바로 레벨을 열고,
  `BP_FindAndJoinLobby` 가 없으면 경고 후 false, `BP_LeaveLobby` 가 없으면 경고 후 즉시 떠난 것으로 친다. 그래서 플러그인이나 BP 가 없어도 `cl9.host` 는 동작한다.
- **`LeaveGame`·`CancelJoin` 은 로비를 떠난다** — 남겨 두면 다음 방의 음성 채널과 섞인다. 떠나는 일은 블루프린트가 비동기로 끝내고 `NotifyLobbyLeft` 로 알린다.
- 로비에 들어가면(`OnLobbyEntered`) 오디오 기능 셋을 켜고(아래 "오디오 기능") `ApplyVoice` 를 다시 부른다 — 로비 전에 부른 `ApplyVoice` 는 블루프린트가 받을 채널이 없었다.
- `bAllowJoinInProgress` 같은 판 중 참가 관문은 없다 — 판 중 참가를 막는 것은 게임 쪽 일이고 아직 없다.
- **개발용 로그인**: 로비를 세우려면 EOS 에 로그인돼 있어야 한다 — 아래 "개발용 로그인".

## 블루프린트 브리지

Free Edition 에는 C++ 헤더가 없다. C++ 가 결정을 내리고 `BlueprintImplementableEvent` 로 넘기면, 블루프린트가 Redpoint 노드로 옮긴다. 이름은 확정이고 구현은 리드가 에디터에서 만든다.
**Redpoint 의 실제 노드 이름·핀·설정 화면 항목은 W0 에서 플러그인을 열어 봐야 안다 — 리드 확인 대기.** 아래 "대응" 열은 명세가 읽은 Redpoint 문서(R4·R5) 기준의 이름이다.

| 클래스 | 이벤트 | 부르는 때 | 어느 머신 | 대응 (문서 기준) |
|---|---|---|---|---|
| `ACL9PlayerState`(구현은 `BP_CL9PlayerState`) | `BP_ApplyVoicePlacement(ECL9VoicePlacement Kind, AActor* Anchor, USoundAttenuation* Attenuation)` | `HostApplyVoicePlacement()` — 경로 변경(`OnRep_VoiceRoute`, 호스트는 `HostApplyVoiceRoute` 경유) · 죽음·부활 가장자리(`UpdateVoiceLifeState`) · 폰 교체(`OnVoicePawnSet` — 나와 핫라인 상대 둘 다) · `PostLogin` | **호스트만** | Voice Chat 컴포넌트의 Proximity Type · Anchor · Attenuation Settings (R4) |
| 같음 | `BP_ApplyVoiceMute(APlayerState* Speaker, bool bMuted)` | **듣는 사람의** PS 에서, 뮤트 목록이 바뀐 쌍마다(`ApplyVoiceMuteDiff`) | 듣는 사람 머신 | `SetPlayerMuted` — 듣는 쪽 로컬 뮤트 (R5) |
| 같음 | `bool BP_ReadVoiceTalking()` | `BeginPlay` 의 0.1초 반복 타이머(`PollVoiceTalking`). **로컬 PS 에서는 부르지 않는다** | 모든 머신, 원격 PS 마다 | `IsPlayerTalking` 폴링 — 채널에 2명 이상일 때만 판정 (R5). 구현이 없으면 늘 false |
| `UCL9GameInstance` | `BP_HostLobby(int32 MaxPlayers)` (protected) | `HostGame` (MaxPlayers=4). 끝나면 **반드시** `NotifyLobbyReady(bOk)` | 방 만드는 머신 | 음성을 켠 EOS 로비 만들기 |
| 같음 | `BP_FindAndJoinLobby()` (protected) | `JoinGame("lan")`. 끝나면 **반드시** `NotifyLobbyJoined(bOk, Addr)` — 0개면 false | 참가자 머신 | 로비 찾기·참가 → 접속 주소 |
| 같음 | `BP_LeaveLobby()` (protected) | `LeaveGame` · `CancelJoin`(접속 대기가 있을 때) · 로비 안에서 다시 만들기·참가. 끝나면 `NotifyLobbyLeft()` | 로컬 | 로비 떠나기 |
| 같음 | `BP_ApplyVoiceOptions(bool bEnabled, bool bPushToTalk, bool bTransmit, float InputVolume, float OutputVolume)` | `SendVoiceOptions` — `ApplyVoice` · R 누름/뗌 · `ClientEnableNetworkVoice`. 볼륨은 0..200 | 로컬 | `bTransmit` 참 → `TransmitToAllChannels`, 거짓 → `TransmitToNoChannels` (R5). 입력 볼륨 0~200 (R5). 출력 볼륨 노드는 **리드 확인 대기** |
| 같음 | `BP_ApplyVoiceAudioFeatures(bool bEcho, bool bNoise, bool bAGC)` | 로비 입장 직후 한 번 `(true, true, true)` | 로컬 | `EnableEchoCancellation` · `EnableNoiseSuppression` · `EnableAutoGainControl` (R5) |
| 같음 (BlueprintCallable) | `NotifyLobbyReady(bool bOk)` · `NotifyLobbyJoined(bool bOk, const FString& ConnectAddress)` · `NotifyLobbyLeft()` | 블루프린트가 위 로비 이벤트를 **성공 경로와 실패 경로 둘 다에서** 끝낼 때. 밖에서 로비가 닫혔을 때 `NotifyLobbyLeft` 를 불러도 된다 | — | — |

- `BP_SetVoiceInputDevice(FString)`(명세 결정 2 에 있던 마이크 장치 선택)는 **만들지 않았다** — 마이크 장치 선택은 범위 밖이다(Windows 기본 입력 하나).
- **`BP_CL9PlayerState`**(부모 `ACL9PlayerState`)는 만들어 `BP_CL9GameMode`·`BP_CL9GameMode_Flat` 의 `PlayerStateClass` 로 지정했다(2026-10-12 — 브리지 그래프는 아직 비어 있다). 지정하지 않으면 로그는 찍히지만 배치·뮤트·말하는 중은 아무 일도 하지 않는다.
  파이썬으로 BP 의 CDO 를 바꾼 뒤에는 **`compile_blueprint` 를 해야** 스폰에 반영된다 — 안 하면 인스턴스가 C++ 기본값으로 뜬다.
- `BP_HostLobby` 를 구현하고 Notify 를 빠뜨리면 **방이 영영 열리지 않는다** — 성공과 실패 어느 쪽에서도 `NotifyLobbyReady` 가 불려야 한다.
- 브리지 그래프의 실제 구현(노드 연결·핀)은 리드가 에디터에서 만든다 — 이 문서는 이름과 호출 시점만 정한다.

### `ECL9VoicePlacement` 와 Redpoint 설정

| 값 | Redpoint |
|---|---|
| `None` | Proximity None (2D) |
| `Pawn` | Possessed Pawn |
| `Anchor` | Custom + Anchor 액터. Anchor 가 null 이면 "Silenced when no Anchor Point" (R4, 2026-07-10 설정) |

## 경로 모델

```cpp
UENUM() enum class ECL9VoiceRoute : uint8 { Proximity, Direct };
USTRUCT() struct FCL9VoiceRoute { ECL9VoiceRoute Mode; TObjectPtr<ACL9PlayerState> Target; };  // Target = 핫라인 상대 (Direct 만)
```

`ACL9PlayerState::VoiceRoute` 는 **복제되고 호스트만 쓴다.** 핫라인은 **양방향 연결 하나**라서 연결된 두 사람의 `VoiceRoute` 가 **서로를 가리킨다**(A = `direct(B)`, B = `direct(A)`).
입구는 셋뿐이고, 이름·시그니처는 그대로이며 **하나의 호출이 두 사람을 함께 바꾼다.**

| 입구 | 누가 | 비고 |
|---|---|---|
| `ACL9PlayerState::HostSetVoiceRoute(Mode, Target)` | 호스트 코드 (`HasAuthority` 게이트) | **다음 명세의 아이템·UI 가 부르는 곳.** 무전기를 들었는지 같은 성립 조건은 호출하는 쪽이 판정한다. **`Direct` + Target = 나 ↔ Target 을 잇는다. `Proximity` = 내 연결을 끊는다(상대도 근접)** |
| `ACL9PlayerController::Server_SetVoiceRoute(Mode, Target)` | 소유 클라 → 호스트 | 이 기능의 **유일한 새 Server RPC**. `RequestVoiceRoute(Mode, Target)` 가 호스트면 직접·클라면 이 RPC 를 부른다. Validate 는 늘 통과한다(상대가 막 나간 경합 같은 선의의 요청까지 끊지 않으려고) — `_Implementation` 이 Target 이 null 이거나 이 판의 `PlayerArray` 에 있는 PlayerState 인지 보고, 아니면 경고 `Voice route: request from '<p>' names a player who is not in this game (left?) — route unchanged.` 만 남기고 바꾸지 않는다 |
| 치트 `cl9.voiceroute prox \| direct <n>` | 로컬 → 호스트 판정 | `n` 은 `cl9.damage` 와 같은 PlayerArray 번호 |

**연결 규칙** (사용자 결정 2026-10-11 밤 — contract §15-1):
- **한 사람은 핫라인 하나만 가진다.** 나나 Target 이 이미 다른 사람과 이어져 있으면 **새 연결이 두 사람의 옛 연결을 끊는다** — 옛 상대는 근접만 남는다(그 옛 상대의 `-> prox` 줄이 남는다).
- **끊기**(`Proximity`)는 나와 상대 둘 다 근접으로 돌린다. 연결이 없을 때 끊으면 `Voice route: '…' has no hotline to drop.` 한 줄뿐이다.
- **한쪽이 나가면(`ACL9GameMode::Logout`) 연결이 끊긴다** — 나가는 사람의 `HostSetVoiceRoute(Proximity)` 로 둘 다 근접이 된다(남은 쪽에 `-> prox` 줄).
- `Direct` 의 Target 은 **다른 사람**이어야 한다 — null·자기 자신이면 경고 한 줄(`Voice route: '…' — direct needs another player…`)과 함께 그대로 둔다.
- 같은 둘이 이미 이어져 있으면 `Voice route: '<a>' and '<b>' are already linked.` 한 줄뿐이다.
- **죽어도 연결은 그대로다**(경로 값은 지워지지 않는다). 죽은 사람의 말은 효력이 없고(Kind `None`), 죽은 사람의 **무전기는 계속 울린다** — 아래 "무전기는 사람과 무관하다".
- 경로가 바뀐 **사람마다** 모든 머신이 로그 한 줄을 찍는다: `Voice route: <p> -> prox` / `Voice route: <p> -> direct(<상대>)`(호스트는 `HostApplyVoiceRoute` 안에서, 클라이언트는 `OnRep_VoiceRoute`).
  그래서 **연결 한 번이면 두 줄**(+ 옛 상대가 있으면 그 `-> prox` 줄), 끊기도 두 줄이다.
- 두 PlayerState 는 따로 복제되므로 클라이언트에서 한 프레임쯤 **한쪽만 바뀐 상태**가 보일 수 있다 — 뒤 줄이 도착하면 같은 값으로 맞는다.

**무전기는 사람과 무관하다** (사용자 결정 2026-10-11 밤 — contract §15-2: "무전기는 무전기일 뿐 사람이랑 관련이 없음, 스위치 켜져 있으면 울리는 것"):
- 무전기가 울리는 자리 = **연결된 상대의 폰**(`ACL9PlayerState::GetHotlineSpeakerPawn()`)이다 — 죽어도 빙의가 풀리지 않으므로 상대가 **죽어 있어도 시체(그 사람의 마지막 폰) 자리**에서 울린다.
  되살아나 폰이 바뀌면 `OnVoicePawnSet` 이 새 폰으로 Anchor 를 다시 정한다.
- **폰이 아예 없으면(나감·소멸) 울릴 자리가 없다** — Anchor 가 null 이 되어 Redpoint 의 "Silenced when no Anchor Point" 를 따른다(로그 `anchor(none)`).
- **말하는 쪽의 관전 규칙은 그대로다**: 죽은 사람은 무전기를 쓰지 않는다 — 죽은 사람의 말은 무전기로도 산 사람에게 들리지 않는다.
- 스위치 켜고 끄기·떨어뜨리기·줍기는 무전기 아이템 명세의 몫이다. 이번에는 **연결 = 켜짐**이다.

### 누가 듣나 — 호스트의 뮤트 표

호스트가 청자마다 못 듣는 화자의 목록을 정한다. **경로는 뮤트를 만들지 않는다 — 남는 것은 관전 규칙(리썰 컴퍼니식)뿐이다:**
산 청자는 죽은 화자를 못 듣고, 죽은 청자는 누구든 듣는다. 호스트 PC 도 청자다.

| 화자 ↓ / 청자 → | 산 청자 (핫라인 상대 아님) | 산 청자 = 핫라인 상대 | 죽은 청자 |
|---|---|---|---|
| 산 · 근접(Prox) | 들림 | — | 들림 |
| 산 · 핫라인(Direct) | 들림 | 들림 | 들림 |
| 죽은 · Prox | **뮤트** | — | 들림 |
| 죽은 · Direct | **뮤트** | **뮤트** — 죽은 사람은 무전기로도 산 사람에게 안 들린다 | 들림 |

"들림" 은 뮤트가 아니라는 뜻이다. 핫라인 중에 화자 곁 사람이 그 목소리를 못 듣는 것은 뮤트가 아니라 **소리 자리**(아래 "어디서 들리나") 때문이다.

- `ACL9GameMode::RebuildVoiceMutes(Trigger, Leaving)` 가 표 전체를 한 번에 다시 짜고 **바뀐 쌍만** 고친다(그대로인 쌍은 건드리지 않는다). 사유(Trigger)는 로그용이다. 표 계산과 로그 문구는 옛 스택과 같다.
- 부르는 자리: `HandlePlayerDeath` 머리(`dead`) · `ReviveDead` 끝(`revive`) · `PostLogin`(`login`) · `Logout`(`logout`, 나가는 사람은 아직 `PlayerArray` 에 있어 `Leaving` 으로 뺀다).
  `PostLogin`·`Logout` 은 `Super` 를 먼저 부른다.
- 로그 `Voice mute: <청자> - <화자> (dead)` 가 걸림, `Voice mute: <청자> + <화자> (<사유>)` 가 풀림이다.

**적용 — 듣는 사람 PS 의 목록과 그 머신의 차이 적용:**
1. 호스트가 듣는 사람마다 `HostSetVoiceMutedSpeakers(목록)` 를 부른다. 순서만 다른 같은 집합이면 건드리지 않는다.
2. 목록은 그 사람 PS 의 `VoiceMutedSpeakers`(`COND_OwnerOnly`)로 **그 사람 머신에만** 복제된다.
3. 그 머신의 `OnRep_VoiceMutedSpeakers()` → `ApplyVoiceMuteDiff()` 가 이미 적용한 목록(`AppliedVoiceMutes`, 약참조)과 비교해 **바뀐 쌍마다** `BP_ApplyVoiceMute(Speaker, bMuted)` 를 부른다.
   아직 도착하지 않은 화자(null)는 도착해 OnRep 이 다시 뜰 때 건다.
4. 리슨 호스트 자신이 듣는 사람이면 OnRep 이 뜨지 않으므로 `HostSetVoiceMutedSpeakers` 가 바로 적용한다.

Redpoint 의 `SetPlayerMuted` 는 듣는 쪽 로컬이라 위처럼 듣는 사람 머신이 적용한다(명세 결정 4). 새 RPC·Multicast 는 없다.
`OnRep_VoiceMutedSpeakers` 가 이전 값 인자를 받지 않는 것은 UHT 가 `TObjectPtr` 를 UFunction 인자로 거부하기 때문이다.

### 어디서 들리나 — 화자마다 하나의 자리

호스트가 화자마다 **자리 하나**를 정해 `BP_ApplyVoicePlacement` 로 넘긴다(`ACL9PlayerState::HostApplyVoicePlacement()`, 호스트에서만 돈다). 듣는 머신이 화자마다 자리를 고르던 옛 방식("더 크게 들리는 쪽" 고르기, 0.25초 타이머)은 없다.

| 화자 상태 | Kind | Anchor | 감쇠 에셋 | 로그 |
|---|---|---|---|---|
| 죽음 | `None` | null | null | `none (-)` |
| 산 · 핫라인 없음 | `Pawn` | — (화자의 폰) | `SettingsTable.VoiceAttenuation` = `SA_CL9_Voice` | `pawn (SA_CL9_Voice)` |
| 산 · 핫라인 T | `Anchor` | T 의 `GetPawn()` — T 가 죽었어도 빙의가 풀리지 않아 시체다. 폰이 없으면 null | `SettingsTable.VoiceSpeakerAttenuation` = `SA_CL9_VoiceSpeaker` | `anchor(<T>) (SA_CL9_VoiceSpeaker)` · 폰이 없으면 `anchor(none) (…)` |

청자가 듣는 모습(위 표와 뮤트 표의 결과):

| 화자 S ↓ / 청자 L → | 산 L = S 의 핫라인 상대 | 산 L = 그 밖 | 죽은 L (상대여도) |
|---|---|---|---|
| 산 · 핫라인 없음 | — | S 의 폰 자리에서 3D (`SA_CL9_Voice`) | S 의 폰 자리에서 3D — 보는 카메라가 오디오 리스너 |
| 산 · 핫라인 (상대 T — **살았든 시체든**) | T 의 폰 자리, 곧 내 자리에서 (Inner 150 cm 안이라 거의 그대로) | **T 의 폰 자리에서만** — T 곁이면 들리고 **S 곁에서는 들리지 않는다** | T 의 폰(시체여도) 자리에서 3D — 보는 카메라 기준 |
| 죽은 S | 뮤트 | 뮤트 | 2D (`None`) — 죽은 사람끼리는 거리 없이 들린다 |

- 받는 사람이 자기 폰 자리에서 듣는 것이 옛 "핫라인 상대는 2D" 의 몫을 대신한다(받는 사람의 오디오 리스너가 자기 폰 곁이라 소리가 그 자리에서 난다). 별도의 2D 배치는 없다.
- 죽은 청자가 산 화자를 관전 카메라 자리에서 3D 로 듣는 것은 **공짜로 따라온다** — 언리얼 오디오 리스너가 보고 있는 카메라를 따라간다.
- **다시 정하는 때**: 경로 변경 · 폰 교체(`OnVoicePawnSet` — 스폰, `ReviveDead` 뒤 재스폰) · 생사 가장자리(`UpdateVoiceLifeState`, `OnRep_Hp` 로 판정) · `PostLogin`. 블루프린트에는 **매번** 넘기고(Redpoint 컴포넌트가 늦게 준비돼도 다음 가장자리에서 맞는다), 로그는 **바뀔 때만** 한 줄이다:
  `Voice: placement <p> -> none|pawn|anchor(<t>)|anchor(none) (<에셋 이름 또는 ->)`. 감쇠 에셋이 비어 있으면 `(no attenuation asset)` 이 찍힌다.
- Redpoint 의 Proximity 설정은 서버 전용이고 플러그인이 복제한다 — 그래서 호스트만 정한다.
- 배치 변경이 말하는 도중에 귀에 닿는지, 다음 발화부터인지는 **리드 확인 대기**(옛 엔진 길은 다음 발화부터였다. Redpoint 는 R4 에 "리스폰 뒤 앵커를 다시 계산한다" 만 있다).

### 감쇠 — 에셋 둘

Redpoint 는 숫자가 아니라 감쇠 **에셋**을 받는다(명세 결정 5). 옛 `CL9Rules` 감쇠 셀 다섯과 런타임 감쇠 객체(`MakeVoiceAttenuation`)는 없어졌다.

| 에셋 | 쓰임 | 가리키는 칸 |
|---|---|---|
| `/Game/Sounds/Master/SA_CL9_Voice` | 핫라인이 없는 산 화자의 목소리 | `SettingsTable.VoiceAttenuation` |
| `/Game/Sounds/Master/SA_CL9_VoiceSpeaker` | 핫라인 상대 자리의 무전기 스피커 | `SettingsTable.VoiceSpeakerAttenuation` |

- **값**(정본 [../plan/data/rules.md](../plan/data/rules.md) "음성 감쇠 에셋 값" 표, 초안): 두 에셋 모두 `Inner` 150 cm · `SilentAt` 1500 cm(감쇠 거리 `FalloffDistance` 1350) · `Linear`.
  귀로 확정하는 것은 Z3·Z6 이다.
- **고정 속성**(에셋 스크립트가 넣는다): 거리 감쇠 켬 · 공간화 켬(panning) · 구 모양 · 오클루전·공기 흡수(저역 통과)·청취 초점 끔 · 플러그인 배열 넷(공간화·오클루전·리버브·소스 데이터) 비움.
- 에셋은 `Tools/Unreal/create_voice_assets.py` 가 만들고 두 칸에 넣는다(멱등 — 값이 이미 맞으면 저장하지 않는다). 확인은 `py -3 Tools/Unreal/run_python.py "Tools/Unreal/create_voice_assets.py verify"` — **따옴표로 한 인자로** 넘겨야 한다.
  `verify` 가 두 에셋의 값과 두 칸을 대조해 `VOICE ASSETS VERIFY OK` 를 찍는다. 에셋 문서는 [../asset/README.md](../asset/README.md) "음성 채팅 에셋".
- **2026-10-12 리드가 실행했고 `verify` 가 `VOICE ASSETS VERIFY OK`(`failures: []`)였다.** 첫 실행은 에디터 기동 직후 `T_Icon_mic` 가 32×32 임시 크기로 읽혀 실패했다(텍스처 비동기 빌드) —
  다시 돌리니 통과했다. 에디터를 막 켠 직후에 실패하면 잠시 뒤 다시 돌린다. 에디터 없이 도는 검사 스크립트는 두지 않는다(사용자 결정).
- 값을 바꿀 때는 rules.md 표와 스크립트 상수를 함께 고친 뒤 스크립트를 다시 돌린다.

### 볼륨 경로 — 리드 확인 대기 (명세 열린 질문 3)

`ApplyVoice` 는 지금 남의 음성 볼륨을 **두 곳에 넣는다**: ① `SettingsTable.VoiceClass`(`SC_Voice`)의 믹스 오버라이드 ② `BP_ApplyVoiceOptions` 의 `OutputVolume`(EOS 출력).
Redpoint 가 음성을 언리얼 오디오 컴포넌트(`SC_Voice`)로 재생하면 슬라이더가 **제곱으로** 작용한다. W1 에서 재생 경로를 확인한 뒤 둘 중 하나를 뺀다 — 아직 모른다.
관전 중 배율 `VoiceSpectatorVolume`(0.8)도 같은 두 곳에 곱해진다. 결정되면 이 절과 [sound.md](sound.md)·[../plan/data/rules.md](../plan/data/rules.md) `VoiceSpectatorVolume` 행의 `SC_Voice` 문구를 맞춘다.

## 말하기 방식 — VA / PTT

송신은 `bTransmit` 하나로 정해진다: **`bTransmit` = 켜짐 ∧ (음성 감지 ∨ PTT 키 누름)** (`UCL9GameUserSettings::IsVoiceTransmitting`). 이 값이 `BP_ApplyVoiceOptions` 로 간다 —
참이면 `TransmitToAllChannels`, 거짓이면 `TransmitToNoChannels`(명세 결정 2).

- **음성 감지(VA, 기본)**: 켜져 있으면 늘 송신 상태로 둔다. 무엇이 실제로 나가는지는 EOS 가 정한다(DTX·음성 활동 판정). **감도 슬라이더는 없다** — Redpoint 문서에 음성 활동 문턱이나 입력 레벨 값이 없다(명세 R6).
- **눌러서 말하기(PTT)**: `IA_Talk`(R)를 누르는 동안만 송신한다(`Started` → `TalkInputStarted`, `Completed`·`Canceled` → `TalkInputStopped`). 누른 채 화면이 열려 `IMC_Gameplay` 가 떼이면 뗌 대신 `Canceled` 가 온다.
  키 상태는 컨트롤러의 `bTalkKeyHeld` 에 있고 `IsTalkKeyHeld()` 로 읽는다.
- `ACL9PlayerController::ApplyVoiceTalking()` 은 인자 없이 `SendVoiceOptions` 를 불러 현재 옵션과 송신 여부를 다시 넘긴다. 호출하는 곳: `TalkInputStarted/Stopped` · `ClientEnableNetworkVoice` 오버라이드.
- `ClientEnableNetworkVoice_Implementation` 오버라이드는 남겼다 — 엔진 기본 구현이 온라인 음성 송신을 켜고 끄는 것을 막고, 우리 옵션을 다시 넘긴다.
- 로비가 없으면 `bTransmit` 이 참이어도 블루프린트가 받을 채널이 없다 — 로비에 들어갈 때 `OnLobbyEntered` 가 `ApplyVoice` 를 다시 부른다.
- 엔진 `StartTalking/StopTalking`, `bRequiresPushToTalk` ini 줄은 쓰지 않는다.

## 옵션 4개와 `ApplyVoice`

모두 로컬·즉시·바꾸는 순간 저장(플레이어에게 이득이 되는 값이 아니다 — [../plan/settings.md](../plan/settings.md) 체크리스트 통과). 설정 화면의 모양은 [settings.md](settings.md) 오디오 절.
**"마이크 감도" 줄과 마이크 레벨 막대는 없다**(2026-10-12 — 명세 열린 질문 2 의 기본값 (b): Redpoint 문서에 음성 활동 문턱과 입력 레벨 값이 없다, R6).
설치한 판에 그런 노드가 있는지는 W0 에서 확인한다 — 리드 확인 대기. 있다면 되살리는 것은 새 결정이다.

| 필드 (`UCL9GameUserSettings`) | 기본 | 화면 | `BP_ApplyVoiceOptions` / 적용 |
|---|---|---|---|
| `bVoiceChatEnabled` | true | 토글 "음성 채팅 사용" | `bEnabled`. 꺼지면 `bTransmit` 도 거짓 |
| `VoiceChatMode` | `VoiceActivity` | 선택 "말하기 방식" (음성 감지 \| 눌러서 말하기) | `bPushToTalk`. `bTransmit` 에 반영 (위) |
| `MicGain` | 1.0 | 슬라이더 "마이크 증폭" 0~200% | `InputVolume` = `MicGain` × 100, 0..200 으로 자름 (EOS 입력 볼륨 — 0 만 무음, R5) |
| `VoiceChatVolume` | 1.0 | 슬라이더 "음성 채팅 볼륨" | `OutputVolume` = `VoiceChatVolume` × 100 (죽어 있으면 × `VoiceSpectatorVolume`), 0..200 으로 자름. `VoiceClass` 믹스 오버라이드도 걸린다 — 위 "볼륨 경로" |

`UCL9GameUserSettings::ApplyVoice(UWorld*)` 는 **멱등**이다. 하는 일 순서: ① `VoiceClass` 믹스 오버라이드(죽어 있으면 `× VoiceSpectatorVolume`) ② `SendVoiceOptions(World)`.
부르는 곳: 로컬 PC `BeginPlay`(`ApplyAudio` 옆) · 설정 화면의 각 Setter(저장 뒤 — 슬라이더는 끄는 동안 쓰기 + `ApplyVoice`, 놓을 때 저장) · **로비에 들어간 직후** · **내가 죽거나 살아났을 때**(관전 볼륨). R 누름/뗌과 `ClientEnableNetworkVoice` 는 `SendVoiceOptions` 만 부른다.
로그 한 줄(Z4):

`Voice: apply enabled=<0|1> mode=<VA|PTT> transmit=<0|1> gain=<f> volume=<f> input=<0..200> output=<0..200>`

`volume` 은 관전 배율이 곱해진 값(`VoiceChatVolume × scale`), `input`·`output` 은 자른 EOS 값이다.

## 오디오 기능 — 에코 제거 · 잡음 억제 · 자동 이득

EOS RTC 는 에코 제거·잡음 억제·자동 이득 조절이 **기본으로 켜져 있다**(명세 배경 — `eos_rtc_types.h:372-380`). 이것이 전송층을 바꾼 이유다(엔진 내장 VOIP 에는 노이즈 게이트와 침묵 감지뿐이다 — 에코 제거는 필수라는 사용자 결정 2026-10-12).
로비에 들어가면 `OnLobbyEntered` 가 `BP_ApplyVoiceAudioFeatures(true, true, true)` 를 한 번 부르고 로그를 남긴다:

`Voice: audio features echo=1 noise=1 agc=1`

- 플레이어가 바꾸는 옵션이 아니다. DTX·플랫폼 AEC 는 건드리지 않는다(R5 에 토글이 있지만 이번 범위가 아니다).
- 이전의 마이크 처리 셀 다섯(`VoiceSilenceAttackTime` 등)과 `cl9.voiceloop`(엔진 루프백)은 없어졌다. Redpoint 의 에코 테스트는 믿을 수 없다고 표시돼 있어(명세 결정 9) 확인은 **두 계정 두 창**(T1)과 **스피커 한 창**(T2, Z7)으로 한다.
- 효과는 사람 귀로만 확인된다(Z7). 로그는 "켰다" 까지만 보증한다.

## 몬스터 청각 없음

몬스터 청각 없음 — 몬스터 쪽에서 음성을 쓸지 나중에 결정(사용자 2026-10-11).

## HUD

말하는 사람 **목록**과 로컬 **마이크 아이콘**이다. 이름표(월드 투영)는 없다. 규칙은 [ui.md](ui.md) "게임 중 HUD" "말하는 사람" 행. **전송층이 바뀌어도 HUD 코드는 바뀌지 않았다** — `ACL9PlayerState::IsVoiceTalking()` 이 읽는 곳만 바뀌었다:

- 각 머신이 HUD 틱에서 `PlayerArray`(≤ 4)를 훑어 `ACL9PlayerState::IsVoiceTalking()` 을 묻는다. **복제 없음.**
  - **로컬 PS**(사용자 결정 2026-10-12): **눌러서 말하기(PTT)에서만** 송신 중(`IsVoiceTransmitting` = R 누름) ∧ 로비 안(`HasVoiceSession()`)일 때 참이다. **음성 감지(VA)에서는 로컬 아이콘을 띄우지 않는다**(거짓).
    Redpoint 의 말하는 중 판정은 채널에 혼자일 때 되지 않고(R5) VA 는 늘 송신 상태라, 송신 상태를 따르면 말과 무관하게 켜지기 때문이다. 말하기 방식은 `UCL9GameUserSettings::VoiceChatMode` 를 읽는다.
  - **원격 PS**: `PollVoiceTalking` 이 0.1초마다 `BP_ReadVoiceTalking()` 으로 갱신한 `bVoiceTalking`.
  - **이 머신에서 `cl9.talk on` 을 켜면 로컬 플레이어는 말하기 방식과 무관하게 늘 true**(HUD 확인용 — 가짜 플래그는 이 머신에만 있고, 로컬 분기보다 먼저 검사한다).
- 로컬이 말하면 `MicIcon`(우상단 아이콘), 남이 말하면 `TalkingBox` 에 `[슬롯 색 점][이름]` 한 줄. 집합이 바뀔 때만 다시 만든다. 로그 `HUD talking: +<이름>` / `HUD talking: -<이름>`(로컬도 찍는다).
- 원격 줄이 꺼지는 지연은 폴링 0.1초와 EOS 의 판정 지연의 합이다 — 옛 "마지막 패킷 뒤 약 1초" 는 없다. 실제 지연은 **측정 전**(Z8). 죽은 화자는 산 청자에게 뮤트라 줄도 없다.
- 마이크 레벨(`GetVoiceLevel()`)은 없다 — 레벨 막대와 함께 지웠다.

## 치트와 로그

치트는 [cheats.md](cheats.md): `cl9.voice`(이 머신의 음성 상태) · `cl9.voiceroute prox | direct <n>` · `cl9.talk on|off` · `cl9.join lan`(= 로비 찾기). `cl9.voiceloop` 는 2026-10-12 에 삭제했다.
`cl9.voice` 는 머리 줄 하나와 플레이어마다 한 줄을 찍는다:

```
cl9.voice: session yes|no, enabled <0|1>, mode VA|PTT, transmit <0|1>, local talking yes|no, net mode <n>.
cl9.voice:   [<n>] '<이름>' route prox|direct(<상대>) alive|dead talking yes|no muted-by [<이름>, …]
```

`muted-by` 는 이 머신이 아는 것만이다 — 뮤트 목록이 듣는 사람 머신에만 복제되므로 호스트는 전원을, 클라이언트는 자기 목록만 안다. 세 창에서 `route` 가 같아야 한다(Z6).
머리 줄의 `local talking` 은 `IsVoiceTalking()` 을 그대로 찍는다 — 음성 감지(VA)에서는 `cl9.talk on` 이 아니면 `no` 로 찍힌다(위 "HUD").
오라클이 읽는 로그(문구 고정 — contract §10):

| 로그 | 언제 | 시나리오 |
|---|---|---|
| `Voice: creating a lobby for 4 players.` | `HostGame` 이 `BP_HostLobby` 를 부르기 직전 | Z1 |
| `Voice: session created GameSession` / `Voice: session joined GameSession` / `Voice: session destroyed GameSession` | **블루프린트의 알림(`NotifyLobbyReady`·`NotifyLobbyJoined`·`NotifyLobbyLeft`) 시점**에 찍힌다 | Z1·Z2 |
| `Voice: session create failed — hosting with no voice chat.` | `NotifyLobbyReady(false)` — 경고, 레벨은 열린다 | Z1 |
| `Voice: '<클래스>' does not implement BP_HostLobby — hosting with no voice chat.` · `… BP_FindAndJoinLobby — use an address instead (cl9.join <ip>).` · `… BP_LeaveLobby — treating the lobby as left.` | 브리지 이벤트를 구현하지 않았을 때 — 경고 | Z1·Z2 |
| `JoinGame: searching for a lobby.` · `JoinGame: connecting to <addr> (lobby).` · `JoinGame: no lobby joined — is a host up (cl9.host)? An address still works (cl9.join <ip>).` | `cl9.join lan` 의 찾기·접속·실패(경고) | Z2 |
| `Voice: no session — remote voice disabled on this connection` | 주소 접속 | Z10 |
| `Voice: audio features echo=1 noise=1 agc=1` | 로비에 들어간 직후 한 번 | Z7 |
| `Voice: apply enabled=… mode=… transmit=… gain=… volume=… input=… output=…` | `ApplyVoice`·`SendVoiceOptions` 마다 | Z4 |
| `Voice: placement <p> -> none (-)` / `pawn (SA_CL9_Voice)` / `anchor(<t>) (SA_CL9_VoiceSpeaker)` / `anchor(none) (…)` | 호스트에서 소리 자리가 **바뀔 때** | Z3·Z5·Z6 |
| `Voice route: <p> -> prox` / `Voice route: <p> -> direct(<상대>)` | 경로가 바뀐 **사람마다** 한 줄 (모든 머신) — 연결 한 번 = 두 줄(+ 옛 상대의 `-> prox`), 끊기 = 두 줄. 같은 둘이 이미 이어져 있으면 `… are already linked.`, 연결이 없는데 끊으면 `… has no hotline to drop.` | Z6 |
| `Voice mute: <청자> - <화자> (dead)` / `Voice mute: <청자> + <화자> (<dead\|revive\|login\|logout>)` | 호스트 뮤트가 걸리고 풀릴 때 | Z5 |
| `HUD talking: +<이름>` / `HUD talking: -<이름>` | HUD 말하는 사람 집합이 바뀔 때 | Z8 |
| `cl9.voice: …` | `cl9.voice` 덤프 | Z1·Z6 |

사라진 로그: `Voice: lan search found <n>` · `Voice: local talker registered/unregistered` · `Voice: talker <p> settings …` · `Voice: talker <p> attenuation …`. 옛 로그를 읽는 문서는 이 표로 바꾼다.

**플러그인 없는 PIE 실측(2026-10-12, 리드)** — `Lvl_Henhouse` 리슨 2인, Redpoint 도 브리지 그래프도 없는 상태에서 오류 없이 다음이 나왔다:
- 접속 시 `Voice: placement <p> -> pawn (SA_CL9_Voice)`.
- `cl9.voiceroute direct 1` → 양쪽 모두 `placement … -> anchor(<상대>) (SA_CL9_VoiceSpeaker)`.
- `cl9.damage 99 1` → `Voice: placement <B> -> none (-)` · `Voice mute: <H> - <B> (dead)` · 관전이 된 클라이언트의 `Voice: apply … output=80`(기본 `VoiceChatVolume` 1.0 × `VoiceSpectatorVolume` 0.8).

이것은 C++ 가 결정을 내리는 단계까지만 보증한다. **소리가 실제로 바뀌는 것은 Redpoint 와 BP 그래프가 선 뒤**에야 확인된다(Z절 T1).

## 개발용 로그인 — Developer Authentication Tool

로비와 음성은 EOS 에 로그인돼 있어야 선다. Steam AppID 가 아직 없으므로(480 은 EOS 가 받지 않는다 — R8) **개발 중에는 Developer Authentication Tool 로 Epic 계정에 로그인한다**(명세 결정 7).
아래는 명세가 Redpoint 문서(R7, 2026-10-12 읽음)에서 가져온 사실이고, **실측은 W0 가 끝난 뒤 — 리드 확인 대기**다.

- 에디터 PIE 의 클라이언트 컨텍스트는 자격증명 `Context_1..N` 에 대응한다. **자격증명마다 서로 다른 Epic 계정**이어야 한다(조직의 개발자 계정 2~3개 — 사용자가 만든다). 기본 자격증명은 `Default`(`DevAuthToolCredentialName`), 도구 포트는 6300.
- 리슨 서버는 P2P 여야 하고 P2P 는 **플레이어마다 자기 EOS 계정**을 요구한다(R3) — 한 PC 에서 창 둘 = 계정 둘.
- **독립 `-game` 창의 자격증명 고르는 법은 문서에 없다.** W6 의 T1 이 명령행·ini 방법을 찾는다. 못 찾으면 PIE "Run Under One Process" 를 끄고 컨텍스트 둘로 대신하고, 어느 쪽이 됐는지 보고한다 — 리드 확인 대기.
- 주소 접속(`cl9.join <ip>`)은 EOS 없이 된다 — 음성 없는 시험용이다.
- Steam 로그인은 steam-release 의 몫이다(아래 "Steam 전환").

## 의존하는 에셋·표 행

이 목록이 빠지면 **코드는 조용히 지나가고 증상만 남는다**(대부분 로그는 돈다). 적용은 리드(에디터/커맨드릿).

| 무엇 | 빠지면 |
|---|---|
| 플러그인 `Plugins/OnlineSubsystemRedpointEOS`(무료판) + EOS 개발자 포털 제품·클라이언트(Peer2Peer 정책, 음성/RTC 켬)·Epic Account Services 앱(Online Presence·Friends·Country 권한) + Project Settings ID (명세 R9, W0) | 블루프린트 노드를 만들 수 없다. C++ 는 브리지 미구현 분기로 로비 없이 진행한다 — 위 "블루프린트가 없을 때" 경고 |
| `BP_CL9PlayerState`(부모 `ACL9PlayerState`) 를 `BP_CL9GameMode`·`BP_CL9GameMode_Flat` 의 `PlayerStateClass` 로 + PS 이벤트 셋 구현 — **생성·지정은 2026-10-12 끝, 이벤트 그래프는 비어 있다**(Redpoint 대기). CDO 를 파이썬으로 바꾼 뒤에는 `compile_blueprint` | 로그는 찍히지만 배치·뮤트·말하는 중이 무동작이다. `Voice: placement …` 는 나는데 소리가 안 변한다. 컴파일을 빼면 스폰한 인스턴스가 C++ 기본값을 쓴다 |
| `BP_CL9GameInstance` 그래프 — 로비 이벤트 셋 + `BP_ApplyVoiceOptions`·`BP_ApplyVoiceAudioFeatures` + 알림 호출(성공·실패 둘 다) | 알림을 빠뜨리면 방이 영영 열리지 않는다. 옵션 이벤트가 없으면 슬라이더·PTT 가 닿지 않는다 |
| `SA_CL9_Voice` · `SA_CL9_VoiceSpeaker`(`/Game/Sounds/Master/`) + `SettingsTable.VoiceAttenuation` · `VoiceSpeakerAttenuation` — `create_voice_assets.py` (**2026-10-12 리드 실행, `verify` OK**) | 칸이 비면 `Voice: placement … (no attenuation asset)` 가 찍히고 감쇠 에셋이 null 로 넘어간다 — Redpoint 가 그때 어떻게 하는지는 리드 확인 대기 |
| `SC_Voice`(`/Game/Sounds/Master/`, 부모 `SC_Master`) · `T_Icon_mic`(`/Game/UI/Icons/`) · `SettingsTable.VoiceClass` | 볼륨 믹스 오버라이드와 관전 배율이 `SC_Voice` 에 닿지 않는다(볼륨 경로 결정 전) / 아이콘 없음. `create_voice_assets.py` 가 세 칸(`VoiceClass`·`VoiceAttenuation`·`VoiceSpeakerAttenuation`)을 카멜 이름으로 채운다(2026-10-12 실행·`verify` OK) |
| `CL9Rules.VoiceSpectatorVolume` (소리 범주 열 **1개**) — 헤더 기본값과 같은 값으로 저장 | 에셋에 값이 없으면 헤더 기본값이 쓰인다(같아야 한다). 지운 12개 열은 클래스에서 사라졌고 `VoiceSpectatorVolume` 은 0.8 인 것을 2026-10-12 확인했다. 옛 열의 저장값은 다음 저장 때 버려진다 |
| `IA_Talk`(`/Game/Input/Actions/`) + `IMC_Gameplay`·`IMC_Spectate` 의 R 행 + `BP_CL9PlayerController.TalkAction` | PTT 방식에서 말할 수 없다. 키 바꾸기 화면에 "말하기" 행이 없다 |
| `WBP_HUD` 위젯 `TalkingBox`(VerticalBox)·`MicIcon`(Image) | 화면이 빈다(둘 다 `BindWidgetOptional` — 로그는 돈다) |
| String Table 키 8개 (`Settings.VoiceChatEnabled`·`Settings.VoiceChatMode*`·`Settings.MicGain`·`Settings.VoiceChatVolume`·`Input.Talk.Name`·`Hud.Talking`). `Settings.VoiceSensitivity`·`Settings.MicLevel` 은 **미사용(2026-10-12)** — 번역 정리 때 지운다 | 설정 화면·HUD 문구가 키로 보인다 |
| 배선 목표(적용은 리드, W0 이후 — **적용 전**): `DefaultEngine.ini` 의 엔진 VOIP 줄 `[OnlineSubsystem] bHasVoiceEnabled`·`[Voice] bEnabled`·`VoiPSoundClass` 와 `DefaultGame.ini [/Script/Engine.GameSession] bRequiresPushToTalk` 제거. `CL9.Build.cs` 에서 C++ 가 더는 쓰지 않는 `OnlineSubsystem`·`OnlineSubsystemUtils`(Public)·`SignalProcessing`(Private) 제거 — **단, Redpoint 가 OnlineSubsystem 의존을 요구하는지는 W0 에서 확인(리드 확인 대기)**. `CL9.uproject`: `OnlineSubsystemNull` 은 Redpoint 와 충돌하지 않으면 유지(주소 접속 시험용), `OnlineSubsystemSteam` 은 steam-release 까지 끈다 | 빌드·부팅이 막히거나, 남은 엔진 VOIP 줄이 Redpoint 와 겹친다 |

## 알려진 한계

- **talking 은 폴링이다** — 원격은 0.1초 주기이고 채널에 **2명 이상**일 때만 판정된다(R5). HUD 가 늦을 수 있다. 지연은 측정 전(Z8).
- **음성 감지(VA)에서는 로컬 마이크 아이콘을 띄우지 않는다**(사용자 결정 2026-10-12). Redpoint 는 채널에 혼자일 때 말하는 중을 판정하지 못하고(R5) VA 는 늘 송신 상태라, 송신 상태를 따르면 로비 안에서 말과 무관하게 켜진다.
  그래서 로컬 `IsVoiceTalking()` 은 **PTT 에서 R 을 누르고 로비 안일 때만**(`IsVoiceTransmitting ∧ HasVoiceSession`) 참이고, VA 에서는 거짓이다. `cl9.talk on` 은 방식과 무관하게 켠다. 내가 말하는 중인지를 VA 에서 아이콘으로 볼 수 없다.
- **시체 앵커**: 핫라인 앵커는 상대의 마지막 폰(시체)에 걸린다. 그 폰이 로비 부활 전에 소멸하면 Anchor 가 null 이 되어 "Silenced when no Anchor Point" 경로로 간다(`anchor(none)` 로그).
- **P2P**: 리슨이 Redpoint 의 P2P 연결 위로 바뀐다 — 접속 시간·틱 레이트는 한 번 재야 한다(미측정). 플레이어마다 자기 Epic 계정이 필요하다(R3). **인터넷이 필요하다.**
- **무료판은 최신 엔진만 지원한다**(R1) — UE 마이너 업데이트 전에 플러그인 업데이트를 확인한다. 무료판의 조건(연 수입 기준, 목록 요약에는 3만 USD 미만)은 설치 전 Fab 목록에서 확인한다 — 리드 확인 대기.
- **마이크 장치 선택이 없다** — Windows 기본 입력 하나. 감도 슬라이더와 레벨 막대도 없다.
- **`cl9.host flat` 으로는 방을 못 연다** — `flat` 사이트에 레벨이 없다. 음성 시험은 `cl9.host henhouse` 로 연다.
- **한 프로세스 다중 클라이언트 PIE**(T0)는 로그(`Voice mute:`·`Voice route:`)와 HUD 를 본다. 귀로 듣는 판정은 독립 창 둘(T1)에서 한다 — 명세 검증 구성이 그렇다.
- 핫라인의 두 `VoiceRoute` 는 따로 복제되어 클라이언트에서 한 프레임쯤 한쪽만 바뀐 상태가 보일 수 있다. 상대 폰이 아직 도착하지 않았으면 로그에 `direct(?)` 가 찍히고 도착하면 한 번 더 뜬다.
- 뮤트 전이·부활 해제는 2026-10-11 에 **옛 스택**의 로그로 확인했다(`Lvl_Henhouse` 리슨 2인 PIE, 증거 `.claude/runs/voice-chat/evidence/`). 표 계산은 그대로지만 **적용 단계가 바뀌었으니** A5(Z5)로 다시 본다.
- 판 중 참가를 거절하는 관문이 없다.
- 음질·감쇠 거리감·관전 볼륨·에코 제거 효과는 사람이 귀로 확정한다(Z절 표시).

## Steam 전환

**Redpoint 가 Steam → EOS Connect 를 맡는다**(R8 — 인증 그래프가 Steam 이 있으면 자동으로 고른다). Steam 층(steam-release)이 들어올 때 이 문서에 필요한 것은 셋뿐이다:

1. **AppID** — 자체 AppID 가 있어야 한다(480 은 EOS 가 받지 않는다). 등록과 Steamworks 쪽은 [../goal/steam-release.md](../goal/steam-release.md) 몫이다.
2. 그때까지의 개발 로그인은 위 Developer Authentication Tool 이다.
3. 세션은 이미 Redpoint EOS 로비가 맡는다 — steam-release W1/W2 의 세션 항목은 "Steam 인증 → EOS Connect 와 AppID 등록" 으로 좁아진다(그 문서의 2026-10-12 정정).
   `OnlineSubsystemSteam` 은 패키지에 남겨도 된다(R8 — 메차카멜레온이 둘 다 싣는다).

바뀌지 않는 것: `HostGame`·`JoinGame`·`LeaveGame`·`CancelJoin`·`IsJoining`·`HasVoiceSession` 의 이름과 시그니처([session.md](session.md) "개발용 전송이다" 규칙), 뮤트·배치·옵션·HUD·치트 전부.
실제 Steam 접속에서의 음성 확인은 둘째 계정·기기가 필요해 steam-release 의 검증 몫이다.

## 참고

- 기획·남은 일·사람 확인 대기 목록: [../plan/voice.md](../plan/voice.md)
- 이번 전송층 명세(결정 1~9·R1~R9·열린 질문): [../goal/voice-eos-redpoint.md](../goal/voice-eos-redpoint.md)
- 옛 명세와 엔진 사실(E1~E13, 기록): [../goal/voice-chat.md](../goal/voice-chat.md) · 이 실행의 역할 사이 계약 변경(§13·§14·§15·§16)은 [../decisions.md](../decisions.md) 2026-10-11
- 값·근거: [../plan/data/rules.md](../plan/data/rules.md) "말소리"
- 사운드 클래스·예외: [sound.md](sound.md) · 옵션 화면: [settings.md](settings.md) · 키: [input.md](input.md) · HUD: [ui.md](ui.md)
- 세션: [session.md](session.md) · 치트: [cheats.md](cheats.md) · 에셋 스크립트: [../asset/README.md](../asset/README.md) "음성 채팅 에셋"
- 검증: [../testing/pie-scenarios.md](../testing/pie-scenarios.md) Z절
