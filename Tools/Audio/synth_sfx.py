"""임시 효과음 합성 — ArtSource/audio/sfx/*.wav (44.1 kHz · 16-bit · 모노).

정본: docs/goal/game-feel.md §1. 이 프로젝트에는 음원 에셋이 없어서(2026-09-18 사용자 결정) 게임 느낌 실행이 쓸
효과음 9개를 파이썬으로 합성한다. 나중에 실음원이 들어오면 같은 이름의 WAV 만 갈아끼우고
Tools/Unreal/import_sfx.py 를 다시 돌린다 — SoundTable 행은 그대로다(ArtSource/audio/README.md).

백엔드: **순수 파이썬**(`wave`·`struct`·`math`·`random`) 이다 — numpy 를 쓰지 않는다.
  numpy 가 있는 머신과 없는 머신에서 바이트가 같아야 하고(결정성), 전체 샘플이 9만 개 남짓이라
  순수 파이썬으로도 1초 안에 끝난다. 난수는 파일마다 고정 시드의 `random.Random` 이라 두 번 돌리면 바이트가 같다.

파일과 성격(brief 고정):
  pickup.wav         0.12초  짧고 밝은 "톡" — 사인 피치 위로 슬라이드 + 시작 노이즈 트랜지언트
  place.wav          0.15초  둔한 "툭" — 낮은 사인 감쇠 + 짧은 저역 노이즈
  swing_whoosh.wav   0.25초  바람 — 대역 필터 노이즈, 중심 주파수가 올라갔다 내려오고 진폭은 산 모양
  hit_pan.wav        0.35초  프라이팬 "땅" — 비조화 부분음 4개(1.0/2.76/5.4/8.9) 급감쇠 + 5 ms 노이즈 클릭
  hurt.wav           0.20초  몸에 맞는 "퍽" — 저역 노이즈 버스트 + 60~90 Hz 사인 감쇠
  monster_death.wav  0.50초  꽥+쿵 — 앞 0.2초 톱니파 피치 급강하, 뒤 0.3초 저역 쿵
  footstep_1/2/3.wav 0.10초  흙바닥 발소리 — 필터 노이즈 버스트, 셋은 필터 중심·길이만 다르다
공통: 시작·끝 2 ms 페이드(클릭 방지), 피크 -3 dBFS 정규화.

실행:  py -3 Tools/Audio/synth_sfx.py
  끝에 파일별 길이·피크 dBFS·SHA-256 표를 찍고, 메모리에서 두 번 렌더한 결과와 디스크에 쓴 바이트가 같은지
  스스로 검증한다(IDEMPOTENT OK / FAIL). 출력은 ASCII 만 쓴다(콘솔 cp949).
"""
import hashlib
import io
import math
import random
import struct
import sys
import wave
from pathlib import Path

SAMPLE_RATE = 44100
FADE_SECONDS = 0.002
PEAK_DBFS = -3.0
OUT_DIR = Path(__file__).resolve().parents[2] / "ArtSource" / "audio" / "sfx"


# ---------------------------------------------------------------- 기본 도구

def seconds_to_samples(seconds):
    return int(round(seconds * SAMPLE_RATE))


def silence(seconds):
    return [0.0] * seconds_to_samples(seconds)


def mix_into(target, source, offset_seconds=0.0, gain=1.0):
    """source 를 target 의 offset 위치에 더한다(길이가 넘치면 잘라 낸다)."""
    start = seconds_to_samples(offset_seconds)
    for i, value in enumerate(source):
        j = start + i
        if j >= len(target):
            break
        target[j] += value * gain
    return target


def sine_sweep(seconds, freq_start, freq_end, curve=1.0):
    """위상 누적 사인 — 주파수가 freq_start 에서 freq_end 로 미끄러진다(curve>1 이면 앞쪽에서 빨리 변한다)."""
    n = seconds_to_samples(seconds)
    out = []
    phase = 0.0
    for i in range(n):
        t = (i / n) ** curve
        freq = freq_start + (freq_end - freq_start) * t
        out.append(math.sin(phase))
        phase += 2.0 * math.pi * freq / SAMPLE_RATE
    return out


def sawtooth_sweep(seconds, freq_start, freq_end, harmonics=12, vibrato_hz=0.0, vibrato_depth=0.0):
    """가산 톱니파(배음 1/k) 피치 슬라이드 — 나이브 톱니파의 앨리어싱을 피한다."""
    n = seconds_to_samples(seconds)
    out = []
    phase = 0.0
    for i in range(n):
        t = i / n
        freq = freq_start + (freq_end - freq_start) * t
        if vibrato_hz > 0.0:
            freq *= 1.0 + vibrato_depth * math.sin(2.0 * math.pi * vibrato_hz * i / SAMPLE_RATE)
        value = 0.0
        for k in range(1, harmonics + 1):
            if freq * k >= SAMPLE_RATE * 0.45:
                break
            value += math.sin(phase * k) / k
        out.append(value * (2.0 / math.pi))
        phase += 2.0 * math.pi * freq / SAMPLE_RATE
    return out


def noise(seconds, seed):
    rng = random.Random(seed)
    return [rng.uniform(-1.0, 1.0) for _ in range(seconds_to_samples(seconds))]


def exp_decay(seconds, tau, attack_seconds=0.0):
    """지수 감쇠 포락선(tau 초에 1/e). attack 은 선형으로 올라간다."""
    n = seconds_to_samples(seconds)
    attack = max(1, seconds_to_samples(attack_seconds)) if attack_seconds > 0.0 else 0
    out = []
    for i in range(n):
        t = i / SAMPLE_RATE
        value = math.exp(-t / tau)
        if attack and i < attack:
            value *= i / attack
        out.append(value)
    return out


def hill(seconds, power=2.0):
    """산 모양 포락선 sin(pi t/T)^power — 가운데가 1."""
    n = seconds_to_samples(seconds)
    return [math.sin(math.pi * i / n) ** power for i in range(n)]


def apply(signal, envelope):
    return [s * e for s, e in zip(signal, envelope)]


def biquad_coeffs(kind, freq, q):
    """RBJ 쿡북 계수 — kind 는 'lowpass' | 'highpass' | 'bandpass'(0 dB 피크)."""
    w0 = 2.0 * math.pi * freq / SAMPLE_RATE
    cos_w0 = math.cos(w0)
    alpha = math.sin(w0) / (2.0 * q)
    if kind == "lowpass":
        b0, b1, b2 = (1.0 - cos_w0) / 2.0, 1.0 - cos_w0, (1.0 - cos_w0) / 2.0
    elif kind == "highpass":
        b0, b1, b2 = (1.0 + cos_w0) / 2.0, -(1.0 + cos_w0), (1.0 + cos_w0) / 2.0
    elif kind == "bandpass":
        b0, b1, b2 = alpha, 0.0, -alpha
    else:
        raise ValueError(kind)
    a0, a1, a2 = 1.0 + alpha, -2.0 * cos_w0, 1.0 - alpha
    return b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0


def biquad(signal, kind, freq, q, freq_of_t=None):
    """2차 IIR 필터. freq_of_t(0..1) 를 주면 시간에 따라 중심 주파수를 바꾼다(64 샘플마다 재계산)."""
    coeffs = biquad_coeffs(kind, freq, q)
    x1 = x2 = y1 = y2 = 0.0
    out = []
    n = len(signal)
    for i, x in enumerate(signal):
        if freq_of_t is not None and i % 64 == 0:
            coeffs = biquad_coeffs(kind, freq_of_t(i / n), q)
        b0, b1, b2, a1, a2 = coeffs
        y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2
        x2, x1 = x1, x
        y2, y1 = y1, y
        out.append(y)
    return out


def finish(signal):
    """2 ms 페이드 인/아웃 → 피크 -3 dBFS 정규화 → 16-bit 정수 리스트."""
    fade = seconds_to_samples(FADE_SECONDS)
    n = len(signal)
    for i in range(min(fade, n)):
        signal[i] *= i / fade
        signal[n - 1 - i] *= i / fade
    peak = max(abs(s) for s in signal)
    target = 10.0 ** (PEAK_DBFS / 20.0)
    gain = target / peak if peak > 0.0 else 0.0
    return [max(-32768, min(32767, int(round(s * gain * 32767.0)))) for s in signal]


def to_wav_bytes(samples):
    buffer = io.BytesIO()
    with wave.open(buffer, "wb") as handle:
        handle.setnchannels(1)
        handle.setsampwidth(2)
        handle.setframerate(SAMPLE_RATE)
        handle.writeframes(struct.pack("<%dh" % len(samples), *samples))
    return buffer.getvalue()


# ---------------------------------------------------------------- 소리 하나씩

def synth_pickup():
    """짧고 밝은 "톡" — 900→1900 Hz 위로 슬라이드하는 사인이 빠르게 죽고, 시작에 4 ms 고역 노이즈."""
    seconds = 0.12
    tone = apply(sine_sweep(seconds, 900.0, 1900.0, curve=0.6), exp_decay(seconds, 0.035, 0.001))
    click = apply(biquad(noise(0.004, seed=101), "highpass", 3000.0, 0.7), exp_decay(0.004, 0.0015))
    return mix_into(tone, click, 0.0, 0.5)


def synth_place():
    """둔한 "툭" — 150→85 Hz 사인 감쇠 + 12 ms 저역 노이즈."""
    seconds = 0.15
    thump = apply(sine_sweep(seconds, 150.0, 85.0, curve=0.5), exp_decay(seconds, 0.045, 0.001))
    knock = apply(biquad(noise(0.012, seed=102), "lowpass", 900.0, 0.8), exp_decay(0.012, 0.004))
    return mix_into(thump, knock, 0.0, 0.7)


def synth_swing_whoosh():
    """바람 — 대역 필터 노이즈. 중심 주파수 500→2000→500 Hz(산 모양), 진폭도 산 모양."""
    seconds = 0.25

    def center(t):
        return 500.0 + 1500.0 * math.sin(math.pi * t) ** 2

    wind = biquad(noise(seconds, seed=103), "bandpass", 500.0, 1.6, freq_of_t=center)
    return apply(wind, hill(seconds, 1.6))


def synth_hit_pan():
    """프라이팬 "땅" — 비조화 부분음 4개가 급감쇠하고, 시작에 5 ms 노이즈 클릭(금속감)."""
    seconds = 0.35
    base = 1050.0
    partials = [(1.0, 1.0, 0.14), (2.76, 0.55, 0.09), (5.4, 0.35, 0.06), (8.9, 0.2, 0.035)]  # (비율, 세기, tau)
    n = seconds_to_samples(seconds)
    out = [0.0] * n
    for ratio, gain, tau in partials:
        freq = base * ratio
        for i in range(n):
            t = i / SAMPLE_RATE
            out[i] += gain * math.sin(2.0 * math.pi * freq * t) * math.exp(-t / tau)
    click = apply(biquad(noise(0.005, seed=104), "highpass", 2500.0, 0.7), exp_decay(0.005, 0.0015))
    return mix_into(out, click, 0.0, 1.2)


def synth_hurt():
    """몸에 맞는 "퍽" — 저역 노이즈 버스트 + 90→60 Hz 사인 감쇠."""
    seconds = 0.2
    body = apply(sine_sweep(seconds, 90.0, 60.0, curve=0.7), exp_decay(seconds, 0.06, 0.002))
    burst = apply(biquad(noise(0.08, seed=105), "lowpass", 350.0, 0.9), exp_decay(0.08, 0.025, 0.002))
    return mix_into(body, burst, 0.0, 1.4)


def synth_monster_death():
    """꽥+쿵 — 앞 0.2초 톱니파 900→220 Hz 급강하(비브라토 살짝), 뒤 0.3초 저역 쿵."""
    seconds = 0.5
    out = silence(seconds)
    squawk = apply(sawtooth_sweep(0.2, 900.0, 220.0, harmonics=10, vibrato_hz=38.0, vibrato_depth=0.06),
                   exp_decay(0.2, 0.09, 0.008))
    squawk = biquad(squawk, "lowpass", 3500.0, 0.7)
    mix_into(out, squawk, 0.0, 0.8)
    thud = apply(sine_sweep(0.3, 80.0, 50.0, curve=0.6), exp_decay(0.3, 0.08, 0.002))
    thud_noise = apply(biquad(noise(0.06, seed=106), "lowpass", 300.0, 0.9), exp_decay(0.06, 0.02, 0.002))
    mix_into(out, thud, 0.2, 1.0)
    mix_into(out, thud_noise, 0.2, 1.2)
    return out


def synth_footstep(seed, seconds, center_hz):
    """흙바닥 발소리 — 대역 필터 노이즈 버스트(뒤꿈치 클릭 + 부드러운 몸통). 셋은 필터 중심·길이만 다르다."""
    body = apply(biquad(noise(seconds, seed=seed), "bandpass", center_hz, 1.1), exp_decay(seconds, seconds * 0.28, 0.003))
    heel = apply(biquad(noise(0.006, seed=seed + 50), "bandpass", center_hz * 2.2, 1.5), exp_decay(0.006, 0.002))
    return mix_into(body, heel, 0.0, 0.6)


SOUNDS = [
    ("pickup.wav", synth_pickup),
    ("place.wav", synth_place),
    ("swing_whoosh.wav", synth_swing_whoosh),
    ("hit_pan.wav", synth_hit_pan),
    ("hurt.wav", synth_hurt),
    ("monster_death.wav", synth_monster_death),
    ("footstep_1.wav", lambda: synth_footstep(201, 0.10, 900.0)),
    ("footstep_2.wav", lambda: synth_footstep(202, 0.09, 1250.0)),
    ("footstep_3.wav", lambda: synth_footstep(203, 0.11, 700.0)),
]


# ---------------------------------------------------------------- 실행

def render(builder):
    samples = finish(builder())
    return samples, to_wav_bytes(samples)


def peak_dbfs(samples):
    peak = max(abs(s) for s in samples)
    return 20.0 * math.log10(peak / 32768.0) if peak else -999.0


def main():
    try:
        sys.stdout.reconfigure(encoding="utf-8")
    except AttributeError:
        pass
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    print("backend: pure python (wave/struct/math/random), %d Hz, 16-bit mono, peak %.1f dBFS" % (SAMPLE_RATE, PEAK_DBFS))
    print("out dir: %s" % OUT_DIR)
    print("%-18s %8s %10s %-10s %s" % ("file", "length", "peak dBFS", "status", "sha256"))
    all_ok = True
    for name, builder in SOUNDS:
        samples, data = render(builder)
        _, data_again = render(builder)          # 같은 프로세스에서 두 번 렌더 — 결정성 1차 검증
        path = OUT_DIR / name
        previous = path.read_bytes() if path.exists() else None
        if previous != data:
            path.write_bytes(data)
        written = path.read_bytes()              # 디스크에 쓴 바이트 = 렌더 바이트 — 멱등 검증
        ok = data == data_again == written
        all_ok = all_ok and ok
        status = "unchanged" if previous == data else ("written" if previous is None else "updated")
        print("%-18s %6.3f s %8.2f dB %-10s %s%s" % (
            name, len(samples) / SAMPLE_RATE, peak_dbfs(samples), status, hashlib.sha256(written).hexdigest()[:16],
            "" if ok else "  MISMATCH"))
    print("IDEMPOTENT OK" if all_ok else "IDEMPOTENT FAIL")
    return 0 if all_ok else 1


if __name__ == "__main__":
    sys.exit(main())
