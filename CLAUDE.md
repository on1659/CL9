# CL9 (UE 5.8)

"양계장 습격" — 동물이 주인인 장소에서 재료를 훔쳐다 푸드트럭에서 요리해 파는 1~4인 협동
추출 게임의 Unreal Engine 5.8 네이티브 구현. 표적은 Steam 리슨 서버 협동 멀티.
게임 기획은 `docs/plan/`, UE 설계는 `docs/conventions.md`+`docs/systems/`에 있다.
프로젝트는 UE 5.8 1인칭 템플릿에서 생성됐고, 그 위에 CL9 게임 코드가 빌드 오더 순서대로
올라가는 중이다 — 현재 어디까지 왔는지는 git log + `docs/plan/build-order.md`가 정본이다.
템플릿 마네킹·FPP 팔 + Dreamscape Farmlands 동물·환경 팩 보유 — systems/animation.md "보유 에셋".

## 빌드

`GenerateProjectFiles.bat` — 엔진 경로는 그 안에 고정돼 있다. **이 머신의 레지스트리에는
5.4/5.6만 등록돼 있어서 .uproject의 "5.8" 연결이 해석되지 않는다 — 반드시
`D:\Program Files\Epic Games\UE_5.8` 엔진으로 빌드한다.** 에디터 빌드 타깃은 `CL9Editor`다.

## 문서 구조

| 무엇을 만지나 | 읽을 문서 |
|---|---|
| 무엇을 만드는가 (게임 기획) | [docs/plan/README.md](docs/plan/README.md) |
| 게임 개요 | [docs/plan/overview.md](docs/plan/overview.md) |
| UE 설계 규약 | [docs/conventions.md](docs/conventions.md) |
| 모델·애니메이션 제작 | [docs/asset/README.md](docs/asset/README.md) |
| 결정의 변경·번복 | [docs/decisions.md](docs/decisions.md) |

## 하지 말 것

- 규칙·결정을 CLAUDE.md 본문에 쌓지 않는다 — 전문은 `docs/`에 두고, 여기에는 요지와
  포인터만 남긴다 (CCL8 저장소의 2026-08-30 문서 구조 결정 승계).
