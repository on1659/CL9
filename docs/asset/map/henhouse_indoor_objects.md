# 양계장(실내 계사) 맵 오브젝트 전체 목록 — Tripo 제작·랜덤 배치용

> 2026-10-13 작성. 사용자 결정: **양계장은 실내 산업형 계사로 다시 만든다.** 기존 양계장 조각·소품
> (`henhouse_sample_01` 이하, Farmlands 잔재)은 이 목록으로 **통째로 대체**한다. 1차는 정적 오브젝트만이고,
> 빛이 붙는 것(작업등·화로)도 몸체만 먼저 만든다. 닭은 한 마리가 아니라 여러 마리이며, 리썰 컴퍼니처럼
> 시간에 따라 더 나오는 규칙을 들일 가능성을 열어 두고 "나오는 자리"가 읽히는 오브젝트를 포함한다.
>
> 정본 관계: 조각 역할은 [../../systems/asset-swap.md](../../systems/asset-swap.md), 모듈 19종과 `PropSets` 규칙은
> [../../plan/data/module.md](../../plan/data/module.md), 조각 종류는 [../../plan/data/segkind.md](../../plan/data/segkind.md),
> 톤 규칙은 [../design-concept.md](../design-concept.md), 공통 제작 규격은 [../common.md](../common.md),
> 이야기는 [../../plan/story.md](../../plan/story.md). 이 문서는 그 위에 **무엇을 몇 개 어떤 치수로 어디에 놓나**를 적는다.

---

## 1. 이 곳이 어떤 곳인가 — 배경이 말해야 하는 것

산업형 계사다. 창이 거의 없는 긴 건물 안에 케이지 열이 늘어서고, 사료 창고·집란실·부화실·사무실·도구방이
복도로 이어진다. 트럭은 바깥이 아니라 **하역장**(지금의 마당 모듈 자리)에 붙는다. 지붕은 이미 지상 발자국 전체를
덮고 있으므로(`bBuildRoofs`), 마당 모듈은 "지붕 아래 열린 하역 공간"으로 읽히게만 바꾸면 된다.

배경이 말해야 하는 것은 넷이고, 오브젝트는 전부 이 넷 중 하나에 속한다.

| 겹 | 말하는 것 | 느낌 |
|---|---|---|
| A 껍데기 | 큰 시설이다 | 긴 벽, 배관·덕트가 천장을 따라간다, 콘크리트 바닥 |
| B 방의 정체 | 이 방은 뭐 하는 방이다 | 방마다 한눈에 읽히는 큰 물건 하나 |
| C 사람이 떠난 흔적 | 어제까지 사람이 있었다 | 급히 두고 간 물건, 잠그지 않은 문, 벽의 분필 정 자 |
| D 주인이 차지한 흔적 | 지금은 닭(탈을 쓴 것)의 집이다 | 둥지로 모인 알, 짚에 밀린 자국, 실밥, 깃털 |

폐허·거미줄·핏자국은 쓰지 않는다. "유지되던 시설에 사람만 없다"가 기준이다.

---

## 2. 디자인 톤 — 전부 같은 규칙으로

[design-concept.md](../design-concept.md)의 다섯 규칙을 배경에 그대로 적용한다. 요약하면 **삼각면 로우폴리, 넓은 색면,
높은 채도, Flat Shading, 실루엣 우선**이다.

### 2.1 형태

- 두툼하고 둔탁하게. 얇은 철사·가는 손잡이는 실제보다 2~3배 굵게 만든다. 손전등 밖에서는 실루엣만 보이므로 가는 것은 사라진다.
- 과장은 "기능 부위"에 준다. 케이지는 문짝, 사료통은 주둥이, 작업등은 갓. 나머지 몸통은 덩어리로 둔다.
- 디테일은 깎아 넣지 않고 색면으로 나눈다. 판자 틈·철망 칸은 지오메트리가 아니라 색 띠 또는 마스크 텍스처 한 장이다.
- 작은 소품도 하나의 덩어리 실루엣이 먼저 읽혀야 한다. 양동이는 "원통 + 손잡이 고리", 빗자루는 "막대 + 쐐기".

### 2.2 팔레트 (초안 — 첫 샘플 뒤 굳힌다)

| 재질 | 기본색 | 어두운 면 | 쓰는 곳 |
|---|---|---|---|
| 판자 목재 | `#C9A86A` 바랜 황토 | `#6B5A48` 회갈 | 벽, 둥지 상자, 작업대, 선반 |
| 구조 금속 | `#4A5A66` 청회 | `#2C3640` | 케이지 프레임, 기둥, 덕트, 난간, 셔터 |
| 녹 | `#B5562A` 주황갈 | — | 금속의 모서리·아래쪽 띠에만. 면 전체에 바르지 않는다 |
| 철망 | `#8E959A` 회은 | — | 케이지 면, 우리 울타리, 환풍 그릴 |
| 볏짚·사료 | `#D9B44A` 탁한 노랑 | `#9C7A2E` | 짚 깔개, 포대 속, 사료 더미 |
| 콘크리트 | `#7A7A74` | `#5C5C57` | 바닥, 하역장, 기초 |
| 안전 노랑 | `#E8C020` | — | 하역장 바닥 선, 경고 표지, 전기 박스 띠 |
| 플라스틱 | `#2E8B7A` 청록 · `#C9403A` 빨강 | — | 계란 트레이, 바구니, 양동이. 한 방에 한 색만 |
| 종이·천 | `#EFE6CF` 크림 | — | 종이, 포대 겉, 천 조각. **작은 면에만** |

규칙 둘. 첫째, **큰 면의 흰색·밝은 크림은 금지**다. 계란이 그 색이라 배경이 계란과 겹치면 안 된다.
둘째, 한 오브젝트에 색은 셋까지다. 넷째 색이 필요하면 형태를 줄인다.

### 2.3 Tripo 생성 규칙

Tripo 출력은 기본이 하이폴리 PBR이라 그대로 두면 캐릭터·몬스터와 톤이 갈린다. 그래서 공정은 "생성 → 재단"이다.

1. **프롬프트 공통 접두어**를 모든 오브젝트에 붙인다.
   `low-poly faceted game prop, flat shading, visible triangles, 3 flat colors max, no text, no logo, no ground plane, single object, centered, upright, front facing +X`
   뒤에 §4 표의 "프롬프트" 칸을 잇는다.
2. **재단(Blender)**: Decimate로 삼각형 예산(§2.4)까지 줄이고, 텍스처를 버리고 **팔레트 정점색**으로 다시 칠한다. 팔레트 텍스처 한 장(16×16)에 UV를 찍는 방식도 같은 결과면 허용한다.
3. **규격**: 바닥 중앙 피벗, 정면 +X, 1 Blender unit = 1 m, 치수는 §4 표에 맞춘다. 비균일 스케일을 남기지 않는다.
4. **이름**: `SM_Henhouse_<이름>_<변형>` (예: `SM_Henhouse_CageRow_A`). 충돌은 닫힌 볼록 `UCX_<메시명>_00`.
5. **껍데기(§4.A)는 Tripo로 만들지 않는다.** 500cm 약수·연결면·회전 반복 계약을 Tripo가 못 지킨다. Blender 규격 제작이다. Tripo는 질감 참고용으로만 쓴다.
6. **입력은 텍스트가 아니라 이미지다 (2026-10-13 시험).** 텍스트→3D는 폴리곤 제한과 스타일 지시가 겹쳐 형태가 뭉개졌다. 이미지는 ChatGPT로 그리고(프롬프트 정본
   [henhouse_indoor_prompts.md](henhouse_indoor_prompts.md)), Tripo는 폴리곤 제한 없이 이미지→3D만 맡긴다(설정·비용 [../tools/tripo.md](../tools/tripo.md)).
   파일 자리는 `ArtSource/map/henhouse_indoor/<id>/`(README 참조).

### 2.4 삼각형 예산

| 분류 | LOD0 목표 |
|---|---|
| 벽·바닥·천장 조각 | 200~1,500 |
| 큰 소품(케이지 열·사료 사일로·작업대) | 2,000~6,000 |
| 중간 소품(상자·드럼통·선반) | 500~2,000 |
| 작은 소품(양동이·종이·분필) | 100~600 |

---

## 3. 랜덤 생성에 맞는 배치 규칙 — 이것을 어기면 어색해진다

맵은 시드마다 다르게 조립된다. 손배치가 없으므로 **소품은 모듈 로컬 좌표의 `PropSets` 행**으로만 선다.
그래서 "어디에 둘 수 있나"가 오브젝트의 치수만큼 중요하다.

### 3.1 좌표와 금지 구역

- 모듈 로컬 cm, 원점은 최소 모서리, Z는 바닥 윗면. 칸은 500×500이고 칸 중심은 `(250+500i, 250+500j)`다.
- **칸 중심 반경 130cm는 몸 있는 소품 금지.** 몬스터·재료 스폰 지터가 그 안에서 뽑힌다. 어기면 배치기가 경고한다.
- 따라서 **벽 띠**는 벽 안쪽 **0~120cm**다. 몸 있는 소품(`bSolid=true`)은 전부 이 띠 안에 선다. 깊이 120cm를 넘는 소품은 만들지 않는다.
- **칸 중심에 둘 수 있는 것은 "몸 없는 깔개"뿐**이다 (`bSolid=false`, 높이 ≤ 15cm). 짚 깔개·바닥 얼룩·밀린 자국이 이것이고, 알이 그 위에 떨어진 것처럼 보이게 하는 장치다.

### 3.2 문 자리와 변 규칙

- 모듈의 `Sockets`에 있는 변은 **열릴 수 있는 변**이다. 그 변의 경계 칸은 전부 문 후보라 소품을 둘 수 없다.
- 열릴 수 있는 변에 붙는 소품은 `SideRule=SideClosed`로만 둔다. 벽이 섰을 때만 켜진다.
- `Sockets`에 없는 변은 **항상 벽**이다. 큰 소품·방의 정체 소품은 이 변에 둔다. 모듈별 항상 벽인 변은 모듈 표에서 읽는다.
- 코너에 두는 소품은 두 변이 모두 항상 벽인 코너에만 둔다. 한 변만 보장되면 한 변에만 걸친다.

### 3.3 통로

- 닭은 통로 중앙으로 뭉툭하게 움직이고, 플레이어는 두 손 짐을 든다. 복도(1칸 폭 500)의 벽 소품은 **깊이 50cm 이하**로 제한한다. 중앙 400cm가 비어 있어야 한다.
- 복도에는 몸 있는 소품을 한 변에만 둔다. 양쪽을 다 채우면 짐 든 사람과 닭이 못 비킨다.
- 계단 통로·계단실에는 디딤판 위와 착지 칸에 아무것도 두지 않는다.

### 3.4 회전과 대칭

- 모듈은 0/90/180/270도로 돌아 선다. 소품은 "앞"이 벽을 등지게 `Side`로 묶는다. 앞뒤가 없는 소품(드럼통, 상자)이 가장 안전하다.
- 벽 부착물(선반·배전반·그릴·포스터)은 벽 두께 안으로 파고들지 않게 두께 ≤ 15cm, 벽에서 2cm 띄운다.

### 3.5 다양성과 절제

- 모듈마다 `PropSets` **2~3벌**을 준비한다. 세트는 자리 좌표 해시로 뽑히므로 같은 방이 연달아 서도 다르게 보인다.
- 방의 정체 소품(§4.B)은 **그 모듈의 모든 세트에 들어간다.** 길 기억의 기준이라서다. 세트 사이에서 바뀌는 것은 공용 소품(§4.C)의 조합뿐이다.
- 한 방에 몸 있는 소품은 **4~7개**, 깔개는 2~4개로 묶는다. 잡동사니로 채우지 않는다.
- 떠난 흔적(§4.D)과 주인 흔적(§4.E)은 **한 판에 한 번 나오는 것이 한 종류씩 있어야** 희소성이 생긴다. 이것은 `SpecialCopies` 1인 Special 모듈이나 `DeadEnd` 모듈 한 종류의 세트 한 벌에만 넣는다.

### 3.6 껍데기는 변형으로 흔든다

벽·바닥·천장은 조각 표의 **변형 풀**(A/B/C)이 시드로 섞인다. 분필 자국·긁힌 자국·덕트·배관·얼룩은 소품이 아니라 **벽·천장·바닥 변형**으로 만든다.
그러면 문 자리·회전과 충돌하지 않고, 소품 수를 늘리지 않고도 반복이 깨진다. 변형은 같은 치수·연결면을 공유해야 한다.

### 3.7 닭이 나오는 자리 (리젠 원점)

판 중간 스폰은 이미 있다 — 60초마다 한 마리, 산 사람 눈에 안 보이고 먼 `Monster` 지점에, 시작 수의 50%까지
([../../goal/loop-7e-mid-round-spawn.md](../../goal/loop-7e-mid-round-spawn.md), 2026-10-12). 리썰의 시간 곡선을 들일지는 미결이다.
어느 쪽이든 **"닭이 여기서 나왔다"가 읽히는 물건**이 Monster 스폰 지점이 있는 모듈에 있어야 한다. §4.F가 그것이다.
세 층 결정(2026-10-13, story.md)으로 중간에 나오는 것은 닭이다(어느 닭이든 게이지가 차면 터진다) — 나오는 자리가 흉할 필요는 없다.

---

## 4. 오브젝트 목록

표 공통 기호. 치수는 길이×깊이×높이 cm, 피벗은 바닥 중앙이 기본이다. "띠"는 어디에 설 수 있나다.
벽=벽 띠 0~120, 복도=벽 띠 0~50, 중앙=칸 중심 허용(깔개), 벽면=벽 부착, 천장=천장 부착.
`S`는 `bSolid`. 변형 수는 같은 규격의 색·낡음 변형이다.

### 4.A 껍데기 — 조각 표 역할 (Blender 규격 제작)

조각 역할 이름은 asset-swap.md의 현행 역할을 그대로 쓴다. 이 표의 메시가 기존 것을 대체한다.

| # | 역할 | 메시 이름 | 치수 | 변형 | 무엇 | 톤·프롬프트 메모 |
|---|---|---|---|---|---|---|
| A1 | `wall`(`barn`) | `SM_Henhouse_Wall_500` | 500×40×H | A 판자 / B 판자+위쪽 철망 띠 / C 판자+배관 가로줄 / D 분필 정 자 자국 | 실내 벽 한 장 | 넓은 판자 3~4장으로 색면 분할. D는 한 판에 한 장만 나오게 Rarity 최저 |
| A2 | `wallQuarter`(`barnStub`) | `SM_Henhouse_Wall_125` | 125×40×H | A | 문 양옆 토막 | A1-A와 같은 판자 리듬 |
| A3 | `door`(문틀, 아직 없는 행) | `SM_Henhouse_DoorFrame_250` | 250×40×H | A | 문틀. 좌·우·상단 셋으로 나눠 개구부 비움 | 금속 프레임, 상단에 방 번호판 자리(비움) |
| A4 | `door`(`flapIn`) | `SM_Henhouse_FlapDoor` | 폭 개구부 맞춤×8×220 | A | 일방향 비상구 문짝 | 고무 플랩 2장, 안전 노랑 띠 |
| A5 | `floor` | `SM_Henhouse_Floor_500` | 500×500×30 | A 콘크리트 / B 콘크리트+배수 홈 / C 짚 부스러기 얼룩 | 실내 바닥 판, `Grid` | 얼룩은 색면 2단, 디테일 없음 |
| A6 | `ceiling` | `SM_Henhouse_Ceiling_500` | 500×500×30 | A 서까래 / B 덕트 한 줄 / C 전선 트레이 | 천장 판, `Grid` | 덕트·트레이는 판에 붙은 돌출 ≤ 60cm |
| A7 | `yard`(하역장 바닥) | `SM_Henhouse_Dock_Floor_500` | 500×500×30 | A 콘크리트 / B 노란 선 | 하역장 바닥, `Grid` | 노란 선은 가장자리 띠. 트럭 쪽 "바깥" 느낌은 색온도로, 재질은 실내와 같은 콘크리트 |
| A8 | `fence`(`wall`, 하역장 칸막이) | `SM_Henhouse_Dock_Rail_250` | 250×20×288 | A 철제 난간+그물 / B 적재 칸막이 판 | 하역장 모듈의 "벽" | 높이 288은 seg kind `wall` 유지. 하부 120cm 판, 상부 그물 |
| A9 | `fence` 토막(`wallStub`) | `SM_Henhouse_Dock_Rail_125` | 125×20×288 | A | 하역장 문 옆 토막 | 지금 없어서 눌리는 것을 해결 |
| A10 | `tread` | `SM_Henhouse_Tread` | 250×264×(단 높이) | A | 계단 디딤판, 격자 철판 | 미끄럼 방지 홈은 색 띠 |
| A11 | `penFence`(`pen`) | `SM_Henhouse_PenFence_250` | 250×20×130 | A | 우리 철망 울타리 | 기존 치수 유지, 톤만 맞춤 |
| A12 | `roof` | `SM_Henhouse_Roof_500` | 500×500×20 | A | 공통 외부 지붕 판 | 보이지 않는 위치. 단색 금속 |
| A13 | `roofFascia` | `SM_Henhouse_RoofFascia_500` | 500×10×100 | A | 외곽 빈 띠 마감 | 〃 |
| A14 | (후보) `pillar` | `SM_Henhouse_Pillar` | 40×40×H | A | 방 코너 기둥. 생성기가 아직 안 부른다 — 소품(벽 띠 코너)으로 먼저 쓴다 | H형강 실루엣 |

**H(벽 높이) = 630cm** (사용자 2026-10-14 결정) — seg kind `barn`과 같아 벽이 한 줄로 선다. 층고 610보다 20cm 높아 위층과 겹치는 것은 지금과 같다 (§6).

### 4.B 방의 정체 소품 — 모듈 19종마다 하나

그 모듈의 **모든 세트**에 들어가는 고정 소품이다. 복도 계열은 정체 소품 대신 변형으로 구분한다.

| 모듈 | 타입 | 정체 소품 | 메시 | 치수 | 띠 | S | 프롬프트 메모 |
|---|---|---|---|---|---|---|---|
| `yard_gate` | Start | 닫힌 셔터 문 + 적재 팔레트 2장 | `SM_Henhouse_Shutter_500`, `SM_Henhouse_Pallet` | 500×30×300 / 120×100×15 | 벽(항상 벽인 변) / 벽 | ✔ / ✔ | 셔터는 가로 띠 6줄 색면. 팔레트는 트럭 방향 |
| `yard_open` | Normal | 드럼통 화로(모닥불 대체) | `SM_Henhouse_BarrelFire` | 60×60×90 | 벽 | ✔ | 몸체만. 불·빛은 2차. 위쪽 녹 띠 |
| `corridor`·`corridor_3`·`corridor_4` | Passage | (정체 소품 없음) | — | — | — | — | 벽 변형 B/C + 천장 변형 B로 구분 |
| `corner` | Passage | 벽 모서리 안전 거울 | `SM_Henhouse_CornerMirror` | 40×20×40 | 벽면 | ✘ | 볼록 거울 원판. 코너 기억 표식 |
| `stair_pass` | Passage | 난간 한 쪽 | `SM_Henhouse_Handrail_250` | 250×10×100 | 벽 | ✔ | 파이프 난간 |
| `coop_hall` | Normal | **케이지 열** | `SM_Henhouse_CageRow_500` | 500×100×220 | 벽(항상 벽인 변) | ✔ | 3단 케이지, 문짝 과장, 앞에 사료 홈통. 이 판의 얼굴 |
| `pen` | Normal | 바닥 우리 + 급수 라인 | `SM_Henhouse_WaterLine_500` | 500×20×60 | 벽 | ✔ | 니플 급수 파이프. 우리 울타리는 A11 |
| `feed_store` | Normal | **사료 사일로 하부 + 배급 레일** | `SM_Henhouse_SiloBase` | 120×120×250 | 벽 코너 | ✔ | 깔때기 몸통, 아래 배출구. 천장으로 이어지는 파이프는 높이 H까지 |
| `cross` | Normal | 중앙 기둥 4개 → 벽 띠 기둥 A14 + 매달린 안내판 | `SM_Henhouse_SignBoard` | 120×5×40 | 천장 | ✘ | 화살표 안내판. 방향은 글자 없이 화살표 색면 |
| `incubator` | DeadEnd | **보온 상자 열** | `SM_Henhouse_Brooder_250` | 250×100×120 | 벽 | ✔ | 뚜껑 반쯤 열린 상자 2개 묶음, 위 열등 갓(빛은 2차) |
| `tool_room` | DeadEnd | 공구 벽걸이 판 | `SM_Henhouse_ToolBoard_250` | 250×15×150 | 벽면 | ✘ | 삽·갈퀴·쇠스랑 실루엣이 걸린 판. 개별 공구는 C에 따로 |
| `storage` | DeadEnd | 랙 선반(3단) | `SM_Henhouse_Rack_250` | 250×60×220 | 벽 | ✔ | 금속 랙, 선반마다 상자 색면 |
| `office` | DeadEnd | 책상 + 의자 + 벽 게시판 | `SM_Henhouse_Desk`, `SM_Henhouse_Chair`, `SM_Henhouse_NoticeBoard` | 160×80×75 / 50×50×90 / 120×5×90 | 벽 / 벽 / 벽면 | ✔ / ✔ / ✘ | 책상 위 장부·전화기는 한 덩어리로 붙임 |
| `coop_small` | DeadEnd | **둥지 상자 열(3칸)** | `SM_Henhouse_NestBox_250` | 250×60×120 | 벽 | ✔ | 칸마다 짚, 앞턱 과장. 스토리의 "둥지가 줄지어" |
| `delivery` | Special | 반납 항아리(기존 `return_jar`) + 계란 선별대 | `SM_Henhouse_GradingTable` | 250×80×90 | 벽 | ✔ | 롤러 선별대, 끝에 트레이 |
| `kitchen` | Special | 조리대는 고정물(`BP_CL9Kitchen`). 배경은 벽 후드 | `SM_Henhouse_Hood_250` | 250×60×60 | 천장 | ✘ | 스테인리스 후드. 색은 구조 금속 |
| `stair_up` | Stair | 계단실 난간 + 층 번호판 | `SM_Henhouse_FloorSign` | 40×5×40 | 벽면 | ✘ | 숫자 대신 색 띠(1층 청록, 2층 빨강) |

### 4.C 공용 소품 풀 — 벽 띠를 채우는 작은 것들

어느 모듈의 세트에도 섞일 수 있는 것들이다. 한 세트에 3~5개를 고른다. **방 성격과 맞는 것만** 고른다(사무실에 사료 포대를 두지 않는다).

| # | 이름 | 메시 | 치수 | 띠 | S | 변형 | 어울리는 모듈 | 프롬프트 메모 |
|---|---|---|---|---|---|---|---|---|
| C1 | 나무 상자 | `SM_Henhouse_Crate` | 60×60×50 | 벽·복도 | ✔ | A B C(쌓임 2단) | storage, feed_store, dock | 판자 상자, 모서리 띠 |
| C2 | 사료 포대(선 것) | `SM_Henhouse_Sack_Up` | 50×35×80 | 벽 | ✔ | A B | feed_store, pen | 포대 입구 묶음 과장 |
| C3 | 사료 포대(눕힌 것·더미) | `SM_Henhouse_Sack_Pile` | 120×80×60 | 벽 | ✔ | A | feed_store | 3자루 한 덩어리 |
| C4 | 터진 포대 + 흘러나온 사료 | `SM_Henhouse_Sack_Spill` | 100×80×35 | 벽·중앙(깔개 부분만) | ✔(포대)/✘(흘림) | A | feed_store | 흘림 부분은 별 메시 C25 |
| C5 | 드럼통 | `SM_Henhouse_Drum` | 60×60×90 | 벽·복도(깊이 50 초과 → 복도 금지) | ✔ | A 파랑 B 녹 | storage, dock | 가로 띠 2줄 |
| C6 | 양동이 | `SM_Henhouse_Bucket` | 30×30×35 | 벽·복도 | ✔ | A | pen, tool_room, kitchen | 손잡이 고리 굵게 |
| C7 | 물통(여물통) | `SM_Henhouse_Trough_250` | 250×40×40 | 벽 | ✔ | A | pen, coop_hall | 기존 `trough` 종류 대체 |
| C8 | 계란 트레이 더미 | `SM_Henhouse_EggTray_Stack` | 30×30×40 | 벽·복도 | ✔ | A 청록 B 회색 | delivery, coop_small, office | 트레이 8장 쌓임, 빈 것 |
| C9 | 플라스틱 바구니 | `SM_Henhouse_Basket` | 50×35×30 | 벽·복도 | ✔ | A 빨강 B 청록 | delivery, coop_small | 구멍은 색면 |
| C10 | 손수레(세워 둔 것) | `SM_Henhouse_Handcart` | 60×40×120 | 벽 | ✔ | A | dock, feed_store | 두 바퀴, 세워 기댄 자세 |
| C11 | 사다리(벽에 기댐) | `SM_Henhouse_Ladder` | 40×30×250 | 벽 | ✔ | A | tool_room, storage, stair_pass | 발판 굵게 |
| C12 | 빗자루 + 쓰레받기 | `SM_Henhouse_Broom` | 30×30×140 | 벽 | ✔ | A | 어디든 | 벽에 기댄 자세 |
| C13 | 삽 / 갈퀴 / 쇠스랑 | `SM_Henhouse_Shovel`·`_Rake`·`_Pitchfork` | 25×25×150 | 벽 | ✔ | 각 A | tool_room, pen | 머리 부분 과장 |
| C14 | 호스 릴 | `SM_Henhouse_HoseReel` | 50×30×60 | 벽면 | ✘ | A | pen, kitchen, corridor | 벽걸이, 호스 감긴 색면 |
| C15 | 소화기 | `SM_Henhouse_Extinguisher` | 20×20×60 | 벽면 | ✘ | A | corridor, cross, office | 빨강 원통. 복도 표식 |
| C16 | 전기 배전반 | `SM_Henhouse_Breaker_Box` | 60×15×80 | 벽면 | ✘ | A 닫힘 B 열림 | corridor, cross, stair_up | 안전 노랑 띠. 정전 연출의 시각 근거 |
| C17 | 벽 스위치 박스 | `SM_Henhouse_Switch` | 15×8×15 | 벽면 | ✘ | A | 어디든 | 문 옆 높이 120 |
| C18 | 배관 밸브 휠 | `SM_Henhouse_Valve` | 30×20×30 | 벽면 | ✘ | A | pen, feed_store | 빨간 휠 |
| C19 | 환풍기(벽) | `SM_Henhouse_WallFan` | 60×15×60 | 벽면(높이 H−80) | ✘ | A | coop_hall, pen | 날개 4장, 정지 |
| C20 | 형광등(천장, 꺼짐) | `SM_Henhouse_TubeLight` | 120×15×8 | 천장 | ✘ | A | 어디든 | 빛 없음. 꺼진 시설 느낌. 켜지는 조명은 C21 |
| C21 | 작업등 몸체 | `SM_Henhouse_WorkLamp` | 30×30×40 | 벽면·벽 | ✘ | A | 어디든 | 갓 + 집게. 빛은 `LightIntensity`로 2차 |
| C22 | 랜턴(꺼짐) | `SM_Henhouse_Lantern` | 20×20×35 | 벽·상자 위 | ✔ | A | office, tool_room | 금속 몸체 |
| C23 | 선반(벽걸이 1단) | `SM_Henhouse_Shelf_120` | 120×30×20 | 벽면 | ✘ | A | office, kitchen, tool_room | 위에 병·통은 C24 |
| C24 | 병·통 묶음 | `SM_Henhouse_Jars` | 60×20×25 | 선반 위 | ✘ | A | office, kitchen | 3~4개 한 덩어리 |
| C25 | 흘린 사료 깔개 | `SM_Henhouse_Spill_Feed` | 150×150×6 | 중앙·벽 | ✘ | A B | feed_store, pen | 몸 없는 깔개. 색 2단 |
| C26 | 짚 깔개 | `SM_Henhouse_Straw_Mat` | 200×200×8 | 중앙 | ✘ | A B C | coop_small, coop_hall, pen, incubator | **알이 떨어지는 자리.** 가장 많이 쓰는 깔개 |
| C27 | 바닥 배수구 | `SM_Henhouse_Drain` | 60×60×3 | 중앙·벽 | ✘ | A | pen, kitchen, dock | 격자 원판 |
| C28 | 바닥 얼룩 깔개 | `SM_Henhouse_Stain` | 120×120×2 | 중앙 | ✘ | A B | 어디든 | 물 얼룩. 어두운 1색 |
| C29 | 방수포 덮인 더미 | `SM_Henhouse_Tarp_Pile` | 120×100×70 | 벽 | ✔ | A 녹 B 파랑 | storage, dock | 밑에 뭐가 있는지 모르는 덩어리 |
| C30 | 팔레트(빈 것, 세움) | `SM_Henhouse_Pallet_Up` | 120×15×100 | 벽 | ✔ | A | dock, storage | 벽에 기댐 |
| C31 | 타이어 더미 | `SM_Henhouse_Tires` | 70×70×60 | 벽 | ✔ | A | dock | 3개 쌓임 |
| C32 | 안전 표지판 | `SM_Henhouse_Sign_Warn` | 40×3×40 | 벽면 | ✘ | A 삼각 B 사각 | corridor, cross, dock | 글자 없이 픽토그램 색면 |
| C33 | 포스터(위생 수칙) | `SM_Henhouse_Poster` | 40×2×60 | 벽면 | ✘ | A B | office, corridor | 글자 없이 색 블록 |
| C34 | 벽시계(멈춤) | `SM_Henhouse_Clock` | 30×5×30 | 벽면 | ✘ | A | office, kitchen | 바늘은 고정 |
| C35 | 달력(멈춘 날짜) | `SM_Henhouse_Calendar` | 30×2×40 | 벽면 | ✘ | A | office | 떠난 날 표시 — 떠난 흔적과 겹치므로 D와 같이 쓰지 않는 세트 |
| C36 | 작업복 걸이 + 작업복 | `SM_Henhouse_Coat_Hook` | 60×25×170 | 벽면 | ✘ | A | office, dock | 걸린 작업복 덩어리 |
| C37 | 장화 한 켤레 | `SM_Henhouse_Boots` | 30×30×35 | 벽 | ✔ | A | dock, office, pen | 두 짝 한 덩어리 |
| C38 | 소독 발판(매트) | `SM_Henhouse_Footbath` | 90×60×8 | 문 앞 안쪽 (문 자리 밖) | ✘ | A | corridor 끝, dock | 깔개. 문 자리에 걸치지 않게 벽 띠 |
| C39 | 라디오 | `SM_Henhouse_Radio` | 30×15×20 | 선반·책상 위 | ✘ | A | office | 트럭 라디오와 호응 |
| C40 | 파이프 다발(바닥) | `SM_Henhouse_Pipe_Bundle` | 250×30×30 | 벽 | ✔ | A | storage, tool_room | 복도엔 쓰지 않음 |
| C41 | 케이지(낱개, 빈 것) | `SM_Henhouse_Cage_Single` | 60×50×50 | 벽·상자 위 | ✔ | A | storage, coop_hall | 문 열림 |
| C42 | 계란 상자(종이, 닫힘) 더미 | `SM_Henhouse_EggBox_Stack` | 60×40×60 | 벽 | ✔ | A | dock, delivery | 크림색 금지 → 회갈 종이 |

### 4.D 사람이 떠난 흔적 — 한 판에 한 번

스토리의 "들어가면" 장면이다. 하나씩만 나와야 효과가 있으므로 **한 모듈의 한 세트에만** 넣는다.

| # | 이름 | 메시 | 치수 | 띠 | S | 놓는 곳 | 이야기와의 연결 |
|---|---|---|---|---|---|---|---|
| D1 | 분필 정 자 벽 + 붙은 종이 | 벽 변형 A1-D + `SM_Henhouse_Note_Paper` | 종이 20×1×28 | 벽면 | ✘ | `coop_small` 세트 1 | 농부의 기록. 종이는 글자 없이 줄 색면 |
| D2 | 분필 토막 + 선반 | `SM_Henhouse_Chalk_Ledge` | 30×8×5 | 벽면(D1 아래) | ✘ | D1과 같은 세트 | 두고 간 분필 |
| D3 | 열린 채 굳은 닭장 문 | `SM_Henhouse_Coop_Door_Open` | 90×60×180 | 벽 | ✔ | `coop_hall` 세트 1 | "닭장 문이 안 닫힌다" |
| D4 | 넘어진 의자 | `SM_Henhouse_Chair_Fallen` | 90×50×50 | 벽 | ✔ | `office` 세트 1 | 급히 떠남 |
| D5 | 책상 위 식은 잔 + 펼친 장부 | `SM_Henhouse_Desk_Left` | 60×40×15 | 책상 위 | ✘ | `office` 세트 1 | 한 덩어리 |
| D6 | 떨어진 열쇠 꾸러미 | `SM_Henhouse_Keys` | 15×10×3 | 벽 띠 바닥 | ✘ | `yard_gate` | 잠그지 않은 문 |
| D7 | 반쯤 실은 손수레(계란 상자 2개) | `SM_Henhouse_Handcart_Loaded` | 110×60×100 | 벽 | ✔ | `delivery` | 일하다 멈춘 자리 |
| D8 | 벽에 기댄 삽(흙 묻음) | `SM_Henhouse_Shovel_Dirty` | 25×25×150 | 벽 | ✔ | `tool_room` | 쫓아 보려던 흔적 |

### 4.E 주인이 차지한 흔적 — 닭(탈을 쓴 것)의 집

| # | 이름 | 메시 | 치수 | 띠 | S | 놓는 곳 | 이야기와의 연결 |
|---|---|---|---|---|---|---|---|
| E1 | 큰 둥지(알 모은 것) | `SM_Henhouse_BigNest` | 160×160×45 | 벽 코너(항상 벽인 두 변) | ✔ | `coop_small`, `coop_hall` | "흩어진 알을 전부 둥지로 밀어 넣는다". 알 아이템과 겹치지 않게 **빈 둥지**로 만들고 알은 스폰이 채운다 |
| E2 | 밀린 자국 짚 깔개 | `SM_Henhouse_Straw_Drag` | 250×120×8 | 중앙 (E1 쪽으로 향함) | ✘ | E1과 같은 세트 | 짚에 난 자국. 방향은 E1 코너를 향하게 Side 고정 |
| E3 | 깃털 뭉치 깔개 | `SM_Henhouse_Feathers` | 80×80×3 | 중앙·벽 | ✘ | coop_hall, pen, corridor | 흰색 금지 → 회갈·주황 깃털 |
| E4 | 실밥 천 조각 | `SM_Henhouse_Stitch_Cloth` | 40×30×2 | 벽 띠 바닥·둥지 턱 | ✘ | coop_small, incubator | 꿰맨 자국 있는 천. 몬스터 실밥과 같은 색 |
| E5 | 긁힌 자국 벽 | 벽 변형 (A1-E 추가 후보) | 500×40×H | — | — | coop_hall 근처 | 발톱 자국 3줄 색면. 변형 풀에 Rarity 낮게 |
| E6 | 케이지 문 전부 열린 열 | `SM_Henhouse_CageRow_Open` | 500×100×220 | 벽 | ✔ | coop_hall 세트 2 | B의 케이지 열 변형. "닭이 나왔다" |
| E7 | 둥지 상자 열(알 자국 있는 것) | `SM_Henhouse_NestBox_Used` | 250×60×120 | 벽 | ✔ | coop_small 세트 2 | B 둥지 열 변형. 짚이 눌림 |

### 4.F 닭이 나오는 자리 — 리젠 원점 (§3.7)

Monster 스폰 지점이 있는 모듈(coop_hall, pen, coop_small, incubator, cross, corridor 계열)에 적어도 하나는 선다.

| # | 이름 | 메시 | 치수 | 띠 | S | 메모 |
|---|---|---|---|---|---|---|
| F1 | 벽 덕트 그릴(열림) | `SM_Henhouse_Duct_Grill` | 80×15×80 | 벽면(바닥에서 20) | ✘ | 리썰 벤트 대응. 그릴 한쪽이 떨어져 있음 |
| F2 | 바닥 구멍 덮개(밀린 것) | `SM_Henhouse_Floor_Hatch` | 90×90×6 | 벽 띠 바닥 | ✘ | 덮개가 비껴 있음 |
| F3 | 케이지 열 뒤 어두운 틈 | B의 `CageRow` 뒤판 없음 | — | — | — | 케이지 열을 벽에서 30cm 띄워 세우면 뒤에 틈이 생긴다. 별 메시 없음 |
| F4 | 천장 개구부(판 빠짐) | 천장 변형 A6-D | 500×500×30 | 천장 | — | 판 하나가 빠져 위가 검다. Rarity 낮게 |

### 4.G 2층용 추가

2층은 1층과 같은 모듈 세트를 쓴다. 다만 **층을 읽히게** 하는 것 둘만 더한다.

| # | 이름 | 메시 | 치수 | 띠 | S | 메모 |
|---|---|---|---|---|---|---|
| G1 | 난간(계단실 상부) | B의 `Handrail_250` 재사용 | — | — | ✔ | `stair_up` 세트에 2장 |
| G2 | 층 번호 색 띠 | B의 `FloorSign` | — | 벽면 | ✘ | 1층 청록, 2층 빨강. 글자 없음 |

### 4.H 수납 가구·숨는 통·큰 홀 소품 (2026-10-22 추가 — `docs/goal/map-hidden-loot.md`)

여는 가구는 본체와 따로 움직이는 조각으로 내보내고, 치수는 게임 아이템 크기(달걀 30×30×40, 고기 67×35×34)에서 정했다.
메시 이름·움직이는 조각·소켓 이름은 [../../goal/map-hidden-loot.md](../../goal/map-hidden-loot.md) "인터페이스" 절, 치수·삼각형·검수는 카드 [henhouse_storage_furniture.md](henhouse_storage_furniture.md)가 정본이다.

| # | 이름 | 메시 | 띠 | 어울리는 모듈 |
|---|---|---|---|---|
| H1 | 서랍장(서랍 3) | `SM_Henhouse_Dresser_A` + `_Drawer_A` | 벽 | office, storage |
| H2 | 찬장(문짝 2) | `SM_Henhouse_Cupboard_A` + `_DoorL_A`·`_DoorR_A` | 벽 | kitchen, office |
| H3 | 낡은 냉장고 | `SM_Henhouse_Fridge_A` + `_Door_A` | 벽 | kitchen, delivery |
| H4 | 철제 사물함 | `SM_Henhouse_Locker_A` + `_DoorL_A`·`_DoorR_A` | 벽 | tool_room, 큰 홀 |
| H5 | 공구함 | `SM_Henhouse_Toolbox_A` + `_Lid_A` | 벽 | tool_room |
| H6 | 뚜껑 있는 나무 상자 | `SM_Henhouse_Chest_A` + `_Lid_A` | 벽 | storage, feed_store, 큰 홀 |
| H7 | 숨는 통 | `SM_Henhouse_HideBarrel_A` + `_Lid_A` | 벽 | 큰 홀, 막다른 방 |
| I1 | 팔레트 랙(큰 홀 정체) | `SM_Henhouse_PalletRack_A` | 벽(깊이 110) | 큰 홀 |
| I2 | 짐 더미(큰 홀 정체) | `SM_Henhouse_CargoPile_A` | 홀 가운데(칸 모서리) | 큰 홀 |

C8 계란 트레이 더미는 2026-10-22에 Blender로 직접 만들어 `SM_Henhouse_EggTray_Stack_A`로 납품했다(Tripo가 세 번 시간 초과). 이것으로 구매 팩이 대신하지 않는 자체 제작 소품에 메시가 없는 것은 없다.

---

## 5. 제작 순서

1. **A 껍데기 7종**(A1 벽 A/B, A5 바닥 A, A6 천장 A, A3 문틀, A7 하역장 바닥, A8 난간). 이것이 서야 나머지가 검수된다. Blender 규격 제작.
2. **B 정체 소품 중 큰 것 5개**: 케이지 열, 둥지 상자 열, 사료 사일로, 보온 상자, 랙 선반. Tripo 첫 배치. 여기서 §2.3 재단 공정을 굳힌다.
3. **C 공용 소품 중 깔개 4종**(C25·C26·C27·C28)과 **벽면 5종**(C15·C16·C19·C20·C21). 깔개가 있어야 칸 중심이 비어 보이지 않는다.
4. 나머지 B·C.
5. **D·E·F**. 희소 세트 구성과 함께.
6. 2차: 빛(C21 작업등, `yard_open` 화로, incubator 열등), 문짝 회전, 소리.

각 단계 끝에 같은 시드(873)로 PIE를 띄워 §3 규칙 위반 경고가 0인지와, 통로 끝→코너→방의 세 장면(중립광·작업등·손전등)을 남긴다.

---

## 6. 구매 팩과의 분담 (2026-10-14 사용자 결정 — 구매 전제로 작업)

Fab 팩 둘을 산다. 팩이 주는 것은 만들지 않고, 팩에 없는 것만 ChatGPT 이관분이 만든다. 팩 메시는 UE 안에서 재질 인스턴스 틴트로 §2.2 팔레트에 맞춘다.

| 팩 | 링크 | 가져오는 것 (이 문서 id) |
|---|---|---|
| Stylized Warehouse (Chhhrrriiisss) — `Content/Peb/StylizedWarehouse` (2026-10-14 받음, 메시 108) | https://www.fab.com/listings/332083b7-11da-49ee-b1a8-44ec9be94487 | `shutter`→`SM_Prop_RollerDoor`(+모터·스위치), `pallet`/`pallet_up`→`SM_Prop_Pallet`, `switch_box`→`SM_Prop_SwitchBox`, `sign_warn`(대안)→`SM_Prop_Sign_*`, 볼라드 7종·드럼 8종·상자·자루·지게차·팔레트잭·트롤리(목록 밖 추가), 바닥 노란 선 타일 16종(하역장 바닥 참고) |
| Stylized Abandoned Factory – Workshop (DZENDOR) — `Content/Peb/StylizedFactory` (2026-10-14 받음, 메시 100) | https://www.fab.com/listings/f59f81a0-9ba0-4b79-85d7-bea26de85acd | `crate`/`crate_stack`→`SM_Crate_Wood`/`SM_Crate_Stack`, `drum`→`SM_Barrel_Oil(_Rusted)`, `handcart`→`SM_Hand_Truck`, `tires`→`SM_Tire_Stack`, `bucket`→`SM_Bucket_Metal`, `sign_warn`→`SM_Sign_Warning`, `valve`→`SM_Valve_Gate`, `wall_fan`→`SM_Fan_Exhaust`, `breaker_box`→`SM_Fuse_Box`/`SM_Cabinet_Electric`, `extinguisher`→`SM_Fire_Extinguisher`, `clock`→`SM_Clock_Wall`, `tube_light`→`SM_Lamp_Fluorescent`, `work_lamp`→`SM_Lamp_Work_Tripod`(+`SM_Lamp_Hanging`·`SM_Lamp_Wall_Cage`), `rack`→`SM_Shelf_Rack(_B)`, `desk`→`SM_Desk_Metal`, `chair`/`chair_fallen`→`SM_Chair_Metal`, `ladder`→`SM_Ladder_Steel_3m`, `handrail`→`SM_Railing_Steel_2m`, 덕트·배관·케이블 트레이(변형 소재) |

**팩에 없어서 그대로 만드는 것** — 껍데기 전부(판자 벽·바닥·천장·문틀·난간은 팩 치수가 500 격자와 안 맞음 — 팩 벽은 1/2/4m, 바닥 4m), `lantern`·`hose_reel`·`tarp_pile`·`poster`·`pipe_bundle`·`shelf`(처음 팩에 있다고 적었던 여섯 — 실물 목록에 없음, 2026-10-14 확인),
`cage_row`·`cage_row_open`·`cage_single`·`nest_box`·`nest_box_used`·`brooder`·`silo_base`·`water_line`·`grading_table`·`hood`·`trough`·`egg_tray_stack`·`egg_box_stack`·`basket`·`sack_*`·`spill_feed`·`straw_mat`·`stain`·`drain`·`footbath`·`boots`·`coat_hook`·`jars`·`radio`·`calendar`·`notice_board`·`sign_board`·`corner_mirror`·`floor_sign`·`barrel_fire`·`broom`·`shovel`·`rake`·`pitchfork`·`tool_board`, D·E·F 전부.

팩 벽·바닥은 생성기에 넣지 않는다(치수·연결면이 우리 계약과 다름). 팩에 같은 물건이 있어도 **정체 소품(§4.B)은 자체 제작**이다 — 길 기억의 기준이라 톤이 정확해야 한다.

## 7. 미결

1. **벽 높이 H — 닫힘(2026-10-14): 630cm.** A1~A4·C19·F1 높이는 이 값으로 정한다.
2. **문 폭.** 문틀 A3을 250으로 적었지만 정본은 "조각 바운드에서 읽는다"다. 새 문틀을 만들면 그 폭이 곧 개구부가 되므로 캐릭터·수레 통과를 같이 확인한다.
3. **Tripo 재단 공정의 자동화 범위.** 감면·정점색 변환을 스크립트로 할지, 첫 배치는 손으로 할지.
4. **리젠 규칙 채택 여부.** 채택하면 F1·F2·F4가 스폰 원점이 되고 `Monster` 지점 배치가 바뀐다.
5. **스토리 문장.** `story.md`의 양계장 "동네에서 제일 작은 집, 닭 서른 마리"를 산업형 계사에 맞게 고친다.
6. **`pen` 모듈의 울타리 변 규칙.** module.md 미결(PerimeterKind 열)이 그대로 남아 있다. A11은 만들어 두되 생성기 연결은 그 결정 뒤다.
