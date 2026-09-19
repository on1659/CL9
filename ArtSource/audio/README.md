# ArtSource/audio — 효과음 원본

`sfx/*.wav` 9개(pickup·place·swing_whoosh·hit_pan·hurt·monster_death·footstep_1/2/3)는 **전부 파이썬 합성 임시음**이다
(2026-09-18, 실음원 0개라 사용자 결정 — `docs/goal/game-feel.md` §1). 44.1 kHz·16-bit·모노, 피크 −3 dBFS.

- 재생성: `py -3 Tools/Audio/synth_sfx.py` — 결정적·멱등(고정 시드, 두 번 돌리면 바이트가 같다 — 끝에 `IDEMPOTENT OK`).
- 임포트(리드, 에디터 Cmd 바): `py "D:/Work/unreal/CL9/Tools/Unreal/import_sfx.py"` — `/Game/Sounds/SFX/**/S_*` + 발소리 큐 `S_Footstep`,
  클래스 `SC_Effects`, Concurrency `SCC_Effects4`. 멱등(있으면 재임포트).
- 실음원으로 바꿀 때: 같은 이름의 WAV 를 `sfx/` 에 덮어쓴다 → `import_sfx.py` 를 다시 돌린다 → `SoundTable` 행은 그대로(에셋 이름이 같다).
  합성 스크립트를 다시 돌리면 실음원이 지워지니, 교체한 파일은 `synth_sfx.py` 의 `SOUNDS` 목록에서 뺀다.
