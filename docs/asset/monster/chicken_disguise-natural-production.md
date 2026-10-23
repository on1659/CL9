# chicken_disguise 자연스러운 벗기·입기 제작 카드

> **이전 `animation_natural_v1` 제작 이력.** 외형 반려 후 보존한 버전이다.
> 현재 새 외형 작업은 [TRELLIS 닭·탈착 애니메이션 인계서](chicken_disguise-animation-handoff.md)를 따른다.
> 아래 동작·치수·납품 기록은 옛 파일에만 해당하며 새 메시의 검수 증거로 복사하지 않는다.

상태: 사용자 외형 반려 — 애니메이션 중단, 전신 외형 재작업 — 2026-10-11

이번 작업은 독립 Blender 제작이며 UE 연결은 범위 밖이다.

- 정본: `ArtSource/monster/chicken_disguise/reference/approved_chicken_disguise_20261011.png` (직접 열어 확인).
- 신규 원본: `ArtSource/monster/chicken_disguise/animation_natural_v1/`.
- 기존 v1~v4 및 열린 요리사 Blender 파일은 보존. 별도 background Blender 사용.
- 1BU=1m, +X 정면, +Z 위, 바닥 root. 기본 30fps, 900프레임.
- 제작 치수: 닭 실측 높이 약 218.6cm, 펼친 본체 약 335.4cm, 비교 요리사 약 127cm. 이미지 원근을 실측값으로 사용하지 않는다.
- 본체의 밝고 긴 단순 얼굴, 작은 검은 눈, 가는 세로 틈을 유지한다.
- 신체는 고정 길이 관절과 손발 IK, 손가락 제어를 사용한다. 전역 크기 변신 없음.
- 탈은 뒤쪽 봉제선이 열리는 한 벌의 속 빈 천. 형태 키/보조 제어로 펼침·주름·처짐을 재현하고 내부 여밈끈을 사용한다. 실제 Cloth 시뮬레이션 여부와 엔진 전달 여부를 별도 기록한다.
- 클립: 1–330 벗기 및 완료 유지, 331–900 진정·바닥 회수·재착용. 입기는 별도 동작.
- 검수: 고정 3/4 및 측면, 손-천 접촉, 지지점, 바닥 침범, 접힌 신체와 탈 내부, 영상 재읽기. 실제 결과는 산출물 `qa.md`에 기록한다.

아직 최종 자연스러움 합격 또는 게임 사용 가능 상태를 의미하지 않는다.

납품: animation_natural_v1/chicken_disguise_natural.blend 및 preview/의 off_on_review.mp4(30초), take_off.mp4(11초), put_on.mp4(19초), side_contact_review.mp4(30초). 모두 1920×1080, 30fps, 전체 프레임 디코딩 확인. 상세 검수와 제한은 같은 제작 폴더 qa.md / manifest.json / video_readback.json을 따른다.

당시 사용자 지시(2026-10-11): 애니메이션을 멈추고 채택 이미지 기준 닭·몬스터 전신 외형을 먼저 재작업한다. 새 그림은 ArtSource/monster/chicken_disguise/reference/fullbody_redraw_20261011_v1에 저장한 검토 후보이며, 새 채택으로 간주하지 않는다.
