# ArtSource/audio — 효과음 원본

`sfx/*.wav` 13개는 **전부 파이썬 합성 임시음**이다(2026-09-18, 실음원 0개라 사용자 결정 — `docs/goal/game-feel.md` §1).
44.1 kHz·16-bit·모노, 피크는 아래 표(적지 않은 것은 −3 dBFS).

| 파일 | 길이 | 성격 | 에셋 · SoundTable 행 |
|---|---|---|---|
| pickup.wav | 0.12 s | 짧고 밝은 "톡" | `S_Pickup` · `Pickup` |
| place.wav | 0.15 s | 둔한 "툭" | `S_Place` · `Place` |
| swing_whoosh.wav | 0.25 s | 바람 | `S_SwingWhoosh` · `Swing` |
| hit_pan.wav | 0.35 s | 프라이팬 "땅" | `S_HitPan` · `Hit` |
| hurt.wav | 0.20 s | 몸에 맞는 "퍽" | `S_Hurt` · `Hurt` |
| monster_death.wav | 0.50 s | 꽥+쿵 | `S_MonsterDeath` · `MonsterDeath` |
| footstep_1/2/3.wav | 0.10 s | 흙바닥 발소리 셋 | `S_Footstep_1/2/3` → 큐 `S_Footstep` · `Footstep` |
| monster_pounce.wav | 0.45 s | 꽥+퍼덕임 — 잡기 예고(Pounce) 시작. 피크 −5 dBFS | `S_MonsterPounce` · `MonsterPounce` |
| monster_catch.wav | 0.50 s | 덮치는 꽥+퍼덕임 — 잡는 순간. pounce 보다 낮고 거칠고 크게(−3 dBFS) + 쿵 | `S_MonsterCatch` · `MonsterCatch` |
| monster_near.wav | 0.16 s | 짧은 꼬꼬댁 — 근접 소나(`monster_prox` 0.65초마다). 피크 −10 dBFS, 반복돼도 안 거슬리게 | `S_MonsterNear` · `MonsterNear` |
| hit_surface.wav | 0.20 s | 프라이팬이 벽·바닥에 닿는 둔한 "퍽/툭" — 나무·흙 벽 울림 + 25 ms 짧은 금속 틱. 피크 −6 dBFS(명중 `hit_pan` 보다 작게) | `S_HitSurface` · `HitSurface` |

몬스터 3개는 2026-09-20 monster-pounce-hurt-readability(`docs/goal/monster-pounce-hurt-readability.md` A·D)에서 추가했다. 세 소리의 크기 차이는
합성 피크로 둔다(예고 < 잡기, 근접은 깔리는 소리) — 임포트 스크립트는 볼륨을 따로 만지지 않는다.
`hit_surface.wav` 는 2026-09-22 combat-feel-pass-1(`docs/goal/combat-feel-pass-1.md` D-4)에서 추가했다 — 허공(`swing_whoosh`)·벽(`hit_surface`)·몬스터(`hit_pan`)
세 소리가 귀로 갈리는 게 목적이라 벽은 짧고 둔하게, 금속 링은 명중의 1/5 길이로만 둔다.

- 재생성: `py -3 Tools/Audio/synth_sfx.py` — 결정적·멱등(고정 시드, 두 번 돌리면 바이트가 같다 — 끝에 `IDEMPOTENT OK`).
- 임포트(에디터 Cmd 바): `py "D:/Work/unreal/CL9/Tools/Unreal/import_sfx.py"` — `/Game/Sounds/SFX/**/S_*` + 발소리 큐 `S_Footstep`,
  클래스 `SC_Effects`, Concurrency `SCC_Effects4`. 멱등(있으면 재임포트). **12개 임포트됨**(2026-09-21 `Content/Sounds/SFX/` 확인 —
  Carry 3 · Character 5(발소리 3 + 큐 + `S_Hurt`) · Monster 5, 몬스터 3개 `S_MonsterPounce`·`S_MonsterCatch`·`S_MonsterNear` 는 2026-09-20).
  `S_HitSurface`(Carry, 2026-09-22)는 스크립트에 추가만 된 상태 — 리드가 임포트 스크립트를 다시 돌려야 생긴다.
  `SoundTable` 의 `MonsterPounce`·`MonsterCatch`·`MonsterNear` 행은 2026-09-20 PIE 기록(`au.DumpActiveSounds` 에 셋 다 — decisions.md)으로는
  들어간 것으로 보이나 `.uasset` 이라 에디터에서 확인한다.
- 실음원으로 바꿀 때: 같은 이름의 WAV 를 `sfx/` 에 덮어쓴다 → `import_sfx.py` 를 다시 돌린다 → `SoundTable` 행은 그대로(에셋 이름이 같다).
  합성 스크립트를 다시 돌리면 실음원이 지워지니, 교체한 파일은 `synth_sfx.py` 의 `SOUNDS` 목록에서 뺀다.
