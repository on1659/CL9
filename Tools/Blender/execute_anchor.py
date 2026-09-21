"""처형 2인 클립의 액터 공간 앵커 표 — animate_creature_reacts.py(돼지·너구리 Execute)와 animate_farmer_executed.py(농부 Executed_*)가 같이 읽는다.
2026-09-21 execute-clips. 설계 정본은 docs/goal/execute-sequence-pig.md "Run 2 — two-rig animation", 값의 근거는 brief 의 앵커 표.

두 리그는 각자 자기 blend 에서 저작되므로(D7 — 한 장면에 붙이면 축·저장이 어긋난다) 정렬은 이 표로만 맞춘다:
- 원점 = 피해자 캡슐 중심의 바닥 투영, +X = 피해자 전방, Z 위, 단위 cm. 캡슐 반경 39·반높이 90, 눈 156, 어깨 ≈ 140.
- 몬스터 리그는 +X 전방·root 바닥이라 몬스터 클립의 x(cm)는 `to_victim_x` 로 피해자 공간이 된다(요 180 이면 부호가 뒤집힌다).
- 시각은 두 클립이 같은 초(클립 시작 = 처형 진입 = `ExecuteStartServerTime`)를 쓴다. 죽는 순간 = 클립 끝 = `ExecuteDuration`(표 값 — 초안 2.0,
  balance 가 바꾸면 여기 DURATION 만 고치고 `--clips Execute` 로 다시 굽는다).
- 카메라 블렌드(`CL9Rules.ExecuteCameraBlend` 0.3) 동안 피해자는 아직 몬스터 카메라를 못 본다 — 예비동작만 둔다.
"""
DURATION = 2.0                 # 초 — ExecuteDuration 초안. 두 종·농부 짝 모두 같은 길이
CAMERA_BLEND = 0.3             # CL9Rules.ExecuteCameraBlend — 이 동안은 예비동작
CAPSULE_RADIUS_CM = 39
CAPSULE_HALF_HEIGHT_CM = 90
EYE_CM = 156
SHOULDER_CM = 140

ANCHOR = {
    'Pig': {
        # 잡히는 순간 그 자리에서 처형(bExecuteOnCatch, 순간 이동 없음). 클립은 거리 CatchRadius 100 을 가정하되 실제는 0~100 가변(덮침 성공 ≈ 30~36
        # 겹침)이라 앞 추력은 max_thrust_cm 이내 — 더 파고들지 않는다.
        'monster_xy_cm': (100.0, 0.0), 'monster_yaw_deg': 180.0,
        'nose_cm': 94.0,                # 코끝 = 몬스터 중심 앞 94 → 피해자 공간 +6(가슴 앞)
        'max_thrust_cm': 15.0,
        'contact_s': 0.50,
        'monster': {'windup': (0.0, 0.3), 'strike': (0.3, 0.5), 'recover': (0.5, 0.9), 'shake': (1.0, 1.6)},
        'farmer': {'brace': 0.40, 'hit': 0.50, 'react': 0.62, 'land': 1.10, 'down': 1.40},
        'farmer_end': 'DOWN',           # AN_Farmer_Down 첫 프레임과 같은 자세 — 보통 죽음 시체와 같은 그림
    },
    'Raccoon': {
        # 등 뒤 ExecuteAnchorDistance 120 에 텔레포트 고정. 도약해 등에 앞발.
        'monster_xy_cm': (-120.0, 0.0), 'monster_yaw_deg': 0.0,
        'nose_cm': 85.0,                # 코끝 = 중심 앞 85 → 기본 자세에서 캡슐 표면(−39) 안 4cm
        'leap_cm': (42.0, 38.0),        # 도약 정점의 body 이동 (X, Z) — brief 초안 +50/+45·피치 −0.6 은 코끝이 피해자 머리 위(z 180)로 나가 줄였다
        'leap_pitch_rad': -0.45,        # 앞이 들리는 피치(음수 = 코 위) — 코끝이 목덜미(≈ z 150) 에 온다
        'paw_target_cm': (-15.0, 135.0),  # 접촉 때 앞발 자리(피해자 공간 X, Z) — 어깨뼈
        'contact_s': 0.60,
        'monster': {'crouch': (0.0, 0.3), 'leap': (0.3, 0.6), 'hold': (0.6, 0.75), 'land': (0.75, 1.0), 'sniff': (1.25, 1.8)},
        'farmer': {'hit': 0.60, 'react': 0.72, 'fall': 1.00, 'prone': 1.25},
        'farmer_end': 'PRONE',          # 신설 — 앞으로 엎드림(골반 피치 −π/2, 얼굴 옆으로)
    },
}


def to_victim(species, monster_x_cm, monster_y_cm=0.0):
    """몬스터 로컬 (x, y)(cm, +X 전방) → 피해자 액터 공간 (X, Y)(cm). 요는 0 또는 180 만 쓴다"""
    cx, cy = ANCHOR[species]['monster_xy_cm']
    flip = -1.0 if ANCHOR[species]['monster_yaw_deg'] == 180.0 else 1.0
    return cx + flip * monster_x_cm, cy + flip * monster_y_cm


def preview_times(species):
    """미리보기로 남길 시각(초) — 시작·예비 끝·접촉·직후·복귀·2차·끝. 접촉 프레임이 꼭 들어간다"""
    c = ANCHOR[species]['contact_s']
    return sorted({0.0, CAMERA_BLEND, c, round(c + 0.2, 2), 0.9, 1.1, 1.4, 1.7, DURATION})
