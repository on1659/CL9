# 참고 — Content Warning의 한 판: 산소·필름·체력·죽음·부활

> 2026-10-17. 설치본(Steam buildid 22999514, Unity 6000.0.67f1, Mono, Photon PUN)의 `Assembly-CSharp.dll`을 MCP 도구 `unity-analysis`로
> 디컴파일해 판 흐름·산소·사망·부활 코드를 읽고, 플레이어 프리팹(`resources.assets:24717`)과 `BigNumbers`(`resources.assets:22522`) 실값을 읽었다.
> **에셋을 가져오지 않는다** — 구조와 수치만 참고한다. 인계 문서 §5의 질문 6을 짧게 다룬다.
> 몬스터는 [contentwarning-monsters.md](contentwarning-monsters.md), 카메라·점수·도구는 [contentwarning-items.md](contentwarning-items.md).

## 결론

- **한 판의 시계는 산소 500초 하나다.** 지하(구세계)에서 잠수종 밖에 있는 동안만 줄고, 절반(250초)이 되면 목표가 "돌아가라"로 바뀌며, 0이면 죽는다.
  2차 몬스터 물결도 이 시계에 묶여 150~350초 사이에 나온다.
- **필름 90초가 두 번째 시계다.** 다 쓰면 역시 "돌아가라"로 바뀐다. 산소는 시간 압박, 필름은 **무엇을 찍을지 고르는 압박**이다.
- **죽음은 판 안에서 끝이 아니다.** 시체는 15점짜리 피사체로 남고, 제세동기로 체력 30에 살릴 수 있고, 죽은 사람은 산 사람에게 목소리가 안 들린다.
  모두 죽으면 5초 뒤 지상으로 돌아가고, 카메라를 못 가져오면 그날 영상은 없다.
- **죽음의 비용은 돈과 장비다.** 죽은 사람마다 보유금의 10%가 병원비로 빠지고, 죽은 뒤 3초에 인벤토리가 그 자리에 쏟아진다.

---

## 1. 하루의 흐름

| 단계 | 일 | 근거 |
|---|---|---|
| 지상(Surface) | 상점·모자·침대. 전원이 잠수종에 타고 문을 닫아야 내려간다 | `DivingBell.cs:58-99` (`collection.Count == players.Count`) |
| 지하 | Factory·Harbour·Mines 중 하나(할당량 주기마다 새로 고름). 몬스터 1차 물결 즉시, 2차 물결 150~350초 | `RoomStatsHolder.cs:135-167`, `RoundSpawner.cs:29-70` |
| 귀환 | 지하에서는 **문만 닫히면** 잠수종이 출발 가능 — 전원이 안 타도 된다 | `DivingBell.cs:73-83` |
| 추출 | 지상의 기계에 카메라를 넣어 녹화를 뽑는다. 실패하면 위로금(할당량 1/3 + 100달러) | `ExtractVideoMachine.cs:180-264` |
| 업로드·시청 | 점수 → 조회수·돈·할당량 ([items](contentwarning-items.md) 2절) | `UploadVideoStation.cs:216-223`, `UploadCompleteState.cs:34-59` |
| 잠 | 침대에서 잠이 차오르면(근처 5~10m 목소리가 0.8 미만이거나 전원이 2초 넘게 침대에 있을 때 초당 0.3) 다음 날. 할당량 주기는 **3일**, 3일째 밤에 못 채우면 **세이브 삭제**(게임 오버) | `Player.cs:504-519`, `SurfaceNetworkHandler.cs:97,588-620`, `RoomStatsHolder.cs:135-155` |

## 2. 시간 압박

| 장치 | 값 (실값) | 효과 | 근거 |
|---|---|---|---|
| 산소 | **500초** (`remainingOxygen`·`maxOxygen`, 플레이어 프리팹) | 지하에서 잠수종 밖일 때 초당 1 감소. 지상에서는 소모 없음 | `Player.cs:524-531,681-693`, `resources.assets:24717` |
| 산소 절반 | 250초 | 호스트가 목표를 `ReturnToTheDiveBellObjective`로 바꾼다 (화면 목표 표시) | `Player.cs:527-530` |
| 산소 0 | — | 본인 클라이언트가 `CallDie()` | `Player.cs:835-838` |
| 필름 0 | 카메라당 90초 | 같은 목표 전환 | `VideoCamera.cs:302-305` |
| 2차 물결 | `0.3~0.7 × 500` = 150~350초 | 몬스터마다 그 안의 무작위 시각에 안 보이는 곳에서 | `BigNumbers.cs:214-217` |
| 재배치 | 120초 뒤부터 100초마다 | 가장 먼 몬스터를 플레이어 근처로 | `Puppetmonster.cs:12-34` |

Infiltrator는 흉내 내는 사람의 산소를 매 프레임 복사해 HUD로 들키지 않게 한다(`Bot_Infiltrator.cs:233-236`).
산소가 공포 장치와 얽히는 점은 하나다 — **늦게까지 남을수록 2차 물결이 이미 다 나와 있고, `Puppetmonster`가 그만큼 여러 번 몬스터를 끌어온다.**

## 3. 체력·사망·부활

| 규칙 | 값 | 근거 |
|---|---|---|
| 최대 체력 | 100 | `Player.cs:303-305` |
| 자연 회복 | 코드에서 못 찾았다 — 회복 경로는 `CallHeal`(Hugger)뿐 | `Player.cs:913-935`, `ItemHugger.cs:41-46` |
| 사망 판정 | 체력 0 이하·산소 0·낙사(높이 ±100 밖)는 **본인** 클라이언트가 RPC로 알린다. 즉사 공격(CamCreep·Weeping·BlackHole)도 피해자 쪽이 `CallDie` | `Player.cs:894-911,943-993,804-825` |
| 죽은 뒤 | 3초 뒤 인벤토리 전부를 그 자리에 떨어뜨린다 | `Player.cs:1025-1039` |
| 시체 | 화면에 잡히면 15점 (코드 10) | [items](contentwarning-items.md) 2.3 |
| 목소리 | 죽은 지 3초 뒤 음성 그룹이 바뀐다 — **죽은 사람은 산 사람과 죽은 사람 모두를 듣지만 죽은 사람에게만 말한다.** 마이크 소음도 끊긴다 | `Player.cs:995-1002,845-848`, `NetworkVoiceHandler.cs:84-110` |
| 판 안 부활 | 제세동기 3회, **체력 30** | `Defib.cs:144-151`, `Player.cs:1005-1023` |
| 전멸 | 호스트가 전원 사망을 확인하면 5초 뒤 지상으로 | `PhotonGameLobbyHandler.cs:250-277` |
| 다음 날 | 죽은 사람도 지상에서 다시 시작. **죽은 사람 1명당 보유금의 10%** 병원비 | `SurfaceNetworkHandler.cs:156-201` |
| 카메라를 못 가져옴 | 카메라 업그레이드 초기화, 목표 `GoToBedFailed` (영상 없음) | `SurfaceNetworkHandler.cs:139-151` |

몬스터 공격이 사망과 얽히는 방식은 세 갈래다([monsters](contentwarning-monsters.md) 2절). HP를 깎는 공격은 대부분 1~2.5초 넘어뜨림을 함께 걸어
연타를 만들고(넘어진 대상은 `AbleToAttack`에서 빠지므로 같은 놈의 연타는 막힌다 — `Bot.cs:907-910`), 납치·매달림·묶임은 피해 없이 동료가
구해 주기를 기다리게 만들며(구조 갈고리·전기 충격봉), 즉사형은 조건(카메라에 찍힘·캡차 실패·카운트다운)이 화면에 드러나 있다.

---

## CL9에 대응하는 자리

CL9 현황은 [../combat-gap.md](../combat-gap.md)·[../scare-mechanics.md](../scare-mechanics.md)에서 가져왔다. 결정은 적지 않는다.

| 자리 | Content Warning | CL9 지금 |
|---|---|---|
| 판의 시계 | 산소 500초(절반에서 귀환 신호), 필름 90초 | 두 비교 문서에 기록 없음 |
| 판 중 증원 | 2차 물결 150~350초, 100초마다 재배치 | 7e 시야 밖 한 마리씩 |
| 잡혔을 때 | 넘어짐 + 피해, 또는 납치·매달림·묶임 | HP 1 + 넘어짐(무적, 짐 유지) |
| 죽음의 비용 | 인벤토리 전부 낙하, 보유금 10% | — |
| 동료 구조 | 갈고리로 떼어 내기, 제세동기 부활 30, Hugger 회복 | 없음 |
| 죽은 사람 | 피사체(15점)이자 죽은 사람끼리만 대화 | — |

---

## 근거와 한계

- 디컴파일 캐시 `C:\Users\Radar\AppData\Local\CL9\UnityAnalysis\contentwarning\decompiled\Assembly-CSharp`, 에셋 `resources.assets`(백엔드 AssetsTools).
- **미확인**: 맵 선택 규칙(`NewMapToPlay`), 지하에 남긴 카메라가 다음 날 어떻게 되는지(`PersistentObjectsHolder`는 카메라를 지상 저장에서 뺀다 — `PersistentObjectsHolder.cs:125,145`), 죽은 사람이 지상 어디서 다시 서는지,
  잠·저녁(`TimeOfDay`) 규칙, 상점 품목 갱신.
- 플레이 실측은 하지 않았다.
