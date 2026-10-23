# HAND-OFF BRIEF/,$p' docs/asset/map/henhouse_indoor_handoff.md; n=1; for f in docs/asset/map/henhouse_indoor_objects.md docs/asset/map/henhouse_indoor_gpt_prompt.md docs/asset/tools/tripo.md docs/asset/common.md ArtSource/map/henhouse_indoor/README.md ArtSource/map/henhouse_structure/ceiling/export/manifest.json; do echo; echo; echo "=================================================================="; echo "# PART $n — $f"; echo "=================================================================="; echo; cat "$f"; n=$((n+1)); done; } > docs/asset/map/henhouse_indoor_handoff_bundle.md
> ```
>
> 돌아오는 것은 `C:\Work\CL9\ArtSource\map\henhouse_indoor\<id>\` 폴더들 + `PRODUCTION_LOG.md`. UE 임포트와 `PropSets` 연결만 이쪽에서 한다.

---

# HAND-OFF BRIEF — you own the whole prop pipeline for one game location

You are taking over, end to end, the production of ~97 static props for the "indoor industrial
poultry house" location of a stylized co-op horror-comedy game (Unreal Engine 5.8). I am not
going to review step by step. You run the pipeline, check your own work against the specs in
this document, and hand back finished folders. Ask me only when something is genuinely blocked
(a credit purchase, a tool you cannot reach, a spec contradiction). Everything else you decide
and record.

This single document contains everything. After this brief come six parts (mostly in Korean —
that is fine, read them fully):

- PART 1 — henhouse_indoor_objects.md — THE SPEC. Every object: id, mesh name, size in cm,
  pivot, where it may stand in a randomly generated room ("band" rules), whether it blocks
  movement (bSolid), color variants, tone rules, triangle budgets, the Tripo process.
- PART 2 — henhouse_indoor_gpt_prompt.md — step-1 brief: the 97 reference images, per-object
  prompts and a QA list. Execute it as written.
- PART 3 — tripo.md — step-2: Tripo Studio settings that work, and the one that failed.
- PART 4 — common.md — Blender conventions: units, axes, naming, materials, budgets.
- PART 5 — henhouse_indoor/README.md — delivery folder layout and file names.
- PART 6 — manifest.json (example) — the exact manifest format each delivered mesh needs.

## The pipeline you run, per object
1. IMAGE  — generate the reference image(s) per PART 2. Save as
            <id>/concept/<id>_concept_v01.png (+ _front/_side/_back where marked "(views)").
2. 3D     — Tripo Studio (studio.tripo3d.ai, the user's paid account), image-to-3D.
            Settings: HD model, H3.1, triangles, polygon limit left at maximum (do NOT cap it in
            Tripo), texture 2K, PBR off, "remove lighting" on, 8K/part-generation trials off.
            About 40 credits per run. Text-to-3D is forbidden — it failed (PART 3).
            Save the raw GLB as <id>/tripo/v01/<id>_tripo.glb and fill <id>/tripo/job.md.
3. RECUT  — Blender. Decimate to the budget in PART 1 (walls 200–1,500; big props
            2,000–6,000; medium 500–2,000; small 100–600 triangles). Throw the Tripo texture away
            and repaint with FLAT VERTEX COLORS from the palette (max 3 colors + 1 accent per
            object), flat shading, visible facets. Pivot = bottom center, front = +X, up = +Z,
            1 Blender unit = 1 m, dimensions exactly as PART 1 (cm). No non-uniform or negative
            scale left on the object. Closed convex collision hull(s) named UCX_<MeshName>_00.
            Save <id>/blender/<id>.blend.
4. EXPORT — FBX: selected objects only, scale 1, +X forward / Z up, apply unit, mesh + UCX
            only, no animation, no lights/cameras. Name SM_Henhouse_<Name>_<Var>.fbx
            (e.g. SM_Henhouse_CageRow_A.fbx; variants B/C share the same geometry contract).
            Write <id>/export/manifest.json in the PART 6 format (dimensions_cm, triangles,
            vertices, materials, fbx_sha256, pivot, axis, collision_hulls...).
5. PREVIEW — three renders of the exported mesh: neutral light, single warm work-lamp,
            single flashlight beam from camera. Save in <id>/preview/.

## Non-negotiables (violating these makes the asset unusable)
- Shell pieces (PART 1 §4.A: walls, floors, ceilings, door frame, rails, treads, roof) are
  NOT made in Tripo. Model them in Blender to the exact grid sizes (multiples/divisors of
  500 cm, seamless edges, must tile at 0/90/180/270°). Tripo images are texture reference only.
- Solid props must be ≤ 120 cm deep (wall band); corridor props ≤ 50 cm deep. Flat ground items
  (straw mats, stains, drains, spills) ≤ 15 cm tall and marked bSolid=false in the manifest.
- Never white or bright cream on large surfaces (the game's eggs are that color).
- No text, numbers or logos on any mesh or texture.
- Colors are vertex colors from the palette; no baked lighting, no PBR textures in delivery.
- Pivot bottom-center, +X front, exact cm sizes. A wrong pivot or scale breaks the random
  placement system.

## Purchased packs — do not duplicate what they provide (added 2026-10-14)
Two Fab packs are purchased and already imported into the Unreal project at
C:\Work\CL9\Content\Peb\StylizedWarehouse (Chhhrrriiisss) and C:\Work\CL9\Content\Peb\StylizedFactory
(DZENDOR "Stylized Abandoned Factory – Workshop"). You may list the .uasset names there; never modify
anything under Content\Peb except the selected relocation authorized below. PART 1 §6 maps object ids to pack meshes. Rules:
- SKIP every object id mapped to a pack mesh in PART 1 §6: no image, no Tripo, no recut. Mark it
  "pack: <mesh name>" in PRODUCTION_LOG.md and move on.
- The six ids first thought to be in the packs but NOT there — lantern, hose_reel, tarp_pile, poster,
  pipe_bundle, shelf — are made as briefed.
- Still make all shell pieces (pack walls are 1/2/4 m and floors 4 m; they do not fit our 500 cm
  grid), all room-identity props (§4.B) and all farm-specific props, even when a pack has something
  similar — identity props must match the palette exactly.
- Keep your palette and recut rules unchanged. We will tint the pack materials toward the same
  palette in Unreal; you do not need to match the packs' look.

### Latest user clarification — selected relocation, 2026-10-14
The user explicitly asked to move assets into the project's folder convention and confirmed
the scope as **사용할것들** (only the assets used). This authorizes relocating the selected
meshes and their necessary materials/textures from Peb into `/Game/Maps/Henhouse/Props`.
Material and texture subfolders retain the pack name for provenance. All unselected pack
packages remain protected. Use Unreal asset operations and preserve required redirectors.
The exact25-ID user skip list in `ArtSource/map/henhouse_indoor/PACK_COVERAGE.json` overrides
the general room-identity exception above: rack and grading-room general furniture are
pack-supplied when explicitly listed. There are72 custom primary IDs remaining in scope.
The migration record and current production checkpoint are in `PRODUCTION_LOG.md`; source
assignment is separate from material tinting, instance posing, placement and gameplay QA.

## Order of work
Start with [cage_row] and run it through ALL five steps before touching anything else. Report
the first full round trip (image → GLB → recut → FBX → previews) with its triangle counts and
the three previews. That round trip fixes the process for the rest. Then continue in the
production order of PART 1 §5 (shell 7 → big identity props 5 → floor mats 4 + wall fixtures 5
→ remaining B/C → D/E/F).

## What you hand back
- The folder tree exactly as in PART 5: <id>/{prompt.md, concept/, tripo/job.md + v01/,
  blender/, export/ (FBX + manifest.json), preview/} for every object.
- A top-level PRODUCTION_LOG.md: per object — status, Tripo credits spent, triangle count,
  deviations from the spec and why, open problems.
- Nothing else is expected from you: Unreal import and PropSets wiring are done on our side.

## Do not
- Do not change ids, mesh names, sizes or band rules. If the spec is wrong, say so in the log
  and proceed with the spec as written.
- Do not spend Tripo credits on retries without recording why in job.md.
- Do not deliver Tripo output without the Blender recut.

Reply first with: (a) one line confirming you read all six parts, (b) any blocking question.
Then start [cage_row].


==================================================================
# PART 1 — docs/asset/map/henhouse_indoor_objects.md
==================================================================

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


==================================================================
# PART 2 — docs/asset/map/henhouse_indoor_gpt_prompt.md
==================================================================

# ChatGPT 이관 브리프 — 양계장(실내 계사) 오브젝트 이미지 97장

> 2026-10-13. **이미지 제작 일은 통째로 ChatGPT에 넘긴다**(사용자 "아예 이관"). 한 장씩 받아 오는 게 아니라, ChatGPT가 이 브리프 하나로
> 97장을 끝까지 만들고 이름 붙이고 목록까지 내놓는다. 우리 쪽으로 돌아오는 것은 완성된 폴더 하나다.
> 아래 코드 블록을 통째로 복사해 ChatGPT 대화 첫 메시지로 보낸다. 오브젝트별 설명의 정본은 [henhouse_indoor_prompts.md](henhouse_indoor_prompts.md) §3이고
> 이 파일은 그것을 브리프로 합친 것이다. 표가 바뀌면 여기도 고친다.
> 받은 폴더는 `ArtSource/map/henhouse_indoor/<id>/concept/`에 id별로 나눠 넣는다. 그 다음 단계(Tripo 이미지→3D, Blender 재단)는 ChatGPT가 못 하므로 여기서 한다.

```
# BRIEF — reference images for a 3D game asset pipeline (full hand-off)

You are taking over the whole job of producing concept reference images for 97 game props.
This is not a back-and-forth: work through the entire list yourself, keep every image
consistent, name the files, and deliver a complete set with a manifest. Only ask me a question
if a description is impossible to draw; otherwise decide yourself and note the decision in the
manifest.

## What this is for
The game is a stylized co-op horror-comedy. Players sneak into an abandoned industrial poultry
house at night to steal eggs while chickens (some of which are not really chickens) roam the
halls. The building is a maintained industrial facility whose people left suddenly: long cage
halls, feed store, incubator room, office, loading dock. The map is assembled from random room
modules, so every prop must read clearly on its own, from any side, in a dark room lit only by a
flashlight. These images are NOT final art: each one is fed into an image-to-3D generator
(Tripo) which converts it into a mesh. That is why the framing rules below are strict — the
3D tool needs one clean object on a plain background.

## Deliverables
1. One PNG per object, 97 in total, named exactly  <id>_concept_v01.png  (id from the list).
   Wide objects 16:9 landscape, compact objects 1:1 square, as marked per line.
2. For the objects marked (views) in the list, three extra PNGs of the same object with the same
   framing: <id>_front_v01.png, <id>_side_v01.png, <id>_back_v01.png.
3. A manifest (plain text or CSV) listing: id, file name(s), aspect, and a one-line note on any
   liberty you took with the description.
4. A short final report: which objects you consider weak or ambiguous, and why.

## Working method
- Go in list order, one object per reply, and start each reply with the id in brackets, e.g.
  [cage_row]. Continue automatically to the next object; if you cannot continue on your own,
  stop and I will say "next".
- Before you consider an image done, check it against the QA list below. If it fails, redo it
  before moving on — do not deliver a failing image and move on.
- Keep style identical across all 97. If you drift, go back to the first images as the anchor.
- Commands I may use later: "redo <id>" (regenerate that one), "views <id>" (three views of
  that one), "continue from <n>" (resume from list number n).

## QA list — every image must pass all of these
- Exactly one object, fully inside the frame, centered, nothing cropped.
- Plain uniform dark grey background (#3A3A3A). No floor, no ground plane, no cast shadow, no
  environment, no other props, no people, no animals.
- No text, letters, numbers, logos or labels anywhere (including on signs, paper, calendars).
- Three-quarter front view from slightly above eye level; "flat" items from a high angle.
- At most three flat colors plus one accent, from the palette; no white or bright cream on
  large surfaces.
- Stylized low-poly look: simplified chunky forms, large flat color areas; no photorealistic
  texture, no fine detail.

## Fixed style rules — apply to every image, do not restate them

STYLE
- Stylized low-poly game prop look: chunky simplified forms, large flat color areas, faceted
  surfaces. No photorealism, no fine texture detail, no decals, no text, no logos, no numbers.
- Exaggerate the functional part of each object (doors, spouts, handles, lamp shades) and keep the
  body a simple solid mass. Thin wires and handles are drawn 2-3x thicker than real.
- At most three flat colors per object plus one accent. Palette:
  wood planks = faded ochre #C9A86A with dark brown-grey #6B5A48 shadows
  structural steel = blue-grey #4A5A66 (dark #2C3640); rust orange #B5562A only on edges/bottoms
  wire mesh = silver grey #8E959A
  straw / feed = dull yellow #D9B44A (dark #9C7A2E)
  concrete = #7A7A74 (dark #5C5C57)
  safety yellow = #E8C020 (only floor lines, warning signs, electrical boxes)
  plastic = teal #2E8B7A or red #C9403A (one per object)
  paper / cloth = cream #EFE6CF, small areas only
- Never use white or bright cream on large surfaces.
- Clean, maintained industrial farm feeling: "people left yesterday", not ruins. No cobwebs,
  no blood, no dirt grime.

FRAMING (critical — each image is fed to an image-to-3D tool)
- Exactly one object, fully visible, centered, nothing cropped.
- Three-quarter front view from slightly above eye level. Flat ground items ("flat" in the
  description) are seen from a high three-quarter angle instead.
- Plain uniform dark grey background (#3A3A3A). No ground plane, no floor, no cast shadow, no
  environment, no other props, no people, no animals.
- Soft even lighting, no dramatic highlights or rim light.
- Aspect ratio as given per object: 16:9 landscape or 1:1 square.
- Sizes in the descriptions are proportions only — never draw dimensions or labels.
- "Same X as <id>" means: reuse that object's shape and colors exactly and change only what the
  line says. These pairs must look like the same prop in two states.

## OBJECT LIST  (97 objects; "(views)" = also deliver front/side/back)

ROOM-IDENTITY PROPS
1. [cage_row] 16:9 (views) — Industrial poultry battery cage row: three tiers of wire mesh cages on a blue-grey steel frame, three cages per tier, chunky exaggerated cage doors, a shallow feed trough running along the front of each tier, dull yellow straw in the bottom of each cage. 5 m long, 1 m deep, 2.2 m tall.
2. [cage_row_open] 16:9 — Same battery cage row as cage_row but every cage door swung open and hanging, straw spilling out of the lower tier.
3. [nest_box] 16:9 (views) — Wooden laying nest box unit with three side-by-side compartments, each filled with straw, a raised front lip on each compartment, a sloped plank roof on top, faded ochre wood with dark edges. 2.5 m long, 0.6 m deep, 1.2 m tall.
4. [nest_box_used] 16:9 — Same nest box unit as nest_box but the straw is pressed flat into shallow bowls as if sat on, a few straw strands hanging over the front lips.
5. [shutter] 16:9 (views) — Closed industrial roll-up loading dock shutter door: six wide horizontal steel slats, blue-grey, a thick bottom rail, side guide rails. 5 m wide, 0.3 m deep, 3 m tall.
6. [pallet] 1:1 — Wooden shipping pallet lying flat, thick planks, faded ochre wood. 1.2 m by 1 m, 0.15 m tall.
7. [barrel_fire] 1:1 — Rusted steel drum used as a fire barrel, blue-grey with rust orange around the top rim and bottom, a few rectangular holes cut in the upper side, no fire, no smoke. 0.6 m diameter, 0.9 m tall.
8. [corner_mirror] 1:1 — Convex safety mirror on a short wall bracket: round domed mirror disc with a thick orange rim, small steel arm behind it. 0.4 m diameter.
9. [handrail] 16:9 — Simple steel pipe handrail section with two posts and one top rail, blue-grey, chunky pipe. 2.5 m long, 1 m tall.
10. [water_line] 16:9 — Poultry nipple drinker line: one long horizontal pipe at knee height on two short brackets, small drinker nipples hanging underneath at regular spacing, blue-grey pipe. 5 m long, 0.6 m tall.
11. [silo_base] 1:1 (views) — Bottom section of a feed silo indoors: an inverted cone funnel body on four steel legs, a round discharge spout at the bottom, a vertical pipe rising from the top edge, blue-grey steel with rust on the lower cone. 1.2 m square footprint, 2.5 m tall.
12. [sign_board] 16:9 — Hanging directional sign board: a flat rectangular panel suspended from two short chains, showing a single large arrow shape as a flat color block, no letters. 1.2 m wide, 0.4 m tall.
13. [brooder] 16:9 (views) — Two chick brooder boxes joined as one unit: wooden boxes with hinged lids half open, a heat lamp shade on a short arm over each box, straw inside, faded ochre wood with blue-grey lamp shades. 2.5 m long, 1 m deep, 1.2 m tall.
14. [tool_board] 16:9 — Wall-mounted tool board: a flat wooden panel with a shovel, a rake, a pitchfork and a hoe hanging on hooks, tool heads exaggerated, faded ochre wood, blue-grey tool heads. 2.5 m wide, 1.5 m tall, 0.15 m deep. Front view.
15. [rack] 16:9 (views) — Industrial metal storage rack with three shelves, a few wooden crates and sacks sitting on the shelves as simple blocks, blue-grey steel frame. 2.5 m long, 0.6 m deep, 2.2 m tall.
16. [desk] 16:9 (views) — Plain farm office desk: thick wooden top on a blue-grey steel frame, a closed ledger book and a chunky old telephone sitting on top as part of the object. 1.6 m wide, 0.8 m deep, 0.75 m tall.
17. [chair] 1:1 — Simple wooden office chair with a steel frame, upright, chunky legs. 0.5 m by 0.5 m, 0.9 m tall.
18. [notice_board] 16:9 — Wall notice board: a cork panel in a wooden frame with a few blank paper sheets pinned on it as flat cream rectangles, no writing. 1.2 m wide, 0.9 m tall, 0.05 m deep. Front view.
19. [grading_table] 16:9 (views) — Egg grading table: a long steel table with a row of rollers along its top and an empty teal plastic tray at one end, blue-grey steel. 2.5 m long, 0.8 m deep, 0.9 m tall.
20. [hood] 16:9 — Stainless kitchen extractor hood, a wide tapered box, blue-grey, meant to hang from a ceiling. 2.5 m wide, 0.6 m deep, 0.6 m tall.
21. [floor_sign] 1:1 — Small square wall plate with one thick horizontal teal stripe, no numbers, blue-grey plate. 0.4 m square. Front view.

SHARED PROPS
22. [crate] 1:1 — Wooden storage crate with corner battens, faded ochre wood, dark edges. 0.6 m by 0.6 m, 0.5 m tall.
23. [crate_stack] 1:1 — Two wooden crates stacked, the top one slightly offset, same crate as crate. 1 m tall.
24. [sack_up] 1:1 — Standing feed sack, full and rounded, the top gathered and tied with an exaggerated knot, cream cloth with a dull yellow band. 0.5 m wide, 0.8 m tall.
25. [sack_pile] 16:9 — Three feed sacks lying in a loose heap as one mass, cream cloth with dull yellow bands. 1.2 m by 0.8 m, 0.6 m tall.
26. [sack_spill] 16:9 — A feed sack torn open on its side with a mound of dull yellow grain spilling out of it, sack and grain as one object. 1 m by 0.8 m, 0.35 m tall.
27. [drum] 1:1 — Steel storage drum, closed top, two raised ribs around the body, dark blue. 0.6 m diameter, 0.9 m tall.
28. [bucket] 1:1 — Galvanized metal bucket with a thick wire handle standing up, blue-grey. 0.3 m diameter, 0.35 m tall.
29. [trough] 16:9 — Long shallow animal water trough, a simple steel channel on two short legs, blue-grey. 2.5 m long, 0.4 m wide, 0.4 m tall.
30. [egg_tray_stack] 1:1 — A stack of eight empty teal plastic egg trays, dimpled tops, slightly uneven stack. 0.3 m square, 0.4 m tall.
31. [basket] 1:1 — Red plastic crate basket with grid holes drawn as flat darker squares, empty. 0.5 m by 0.35 m, 0.3 m tall.
32. [handcart] 1:1 (views) — Two-wheeled hand truck standing upright, blue-grey steel frame, thick rubber wheels, empty. 0.6 m wide, 0.4 m deep, 1.2 m tall.
33. [handcart_loaded] 1:1 — Same hand truck as handcart but with two brown cardboard egg boxes strapped on the plate.
34. [ladder] 1:1 — Wooden straight ladder leaning at a slight angle, thick rungs, faded ochre wood. 0.4 m wide, 2.5 m tall.
35. [broom] 1:1 — Push broom and a dustpan leaning together, wooden handle, blue-grey dustpan, drawn as one object. 0.3 m wide, 1.4 m tall.
36. [shovel] 1:1 — Farm shovel standing upright, exaggerated wide blade, wooden handle, blue-grey blade. 1.5 m tall.
37. [rake] 1:1 — Farm rake standing upright, exaggerated wide head, wooden handle. 1.5 m tall.
38. [pitchfork] 1:1 — Pitchfork standing upright, four thick tines, wooden handle. 1.5 m tall.
39. [hose_reel] 1:1 — Wall-mounted hose reel with a coiled green hose, blue-grey bracket, the coil drawn as a solid ring. 0.5 m wide, 0.3 m deep, 0.6 m tall. Front view.
40. [extinguisher] 1:1 — Red fire extinguisher on a small wall hook, chunky nozzle. 0.6 m tall. Front view.
41. [breaker_box] 1:1 — Wall electrical breaker box, closed grey door with a safety yellow stripe across it, blue-grey body. 0.6 m wide, 0.8 m tall, 0.15 m deep. Front view.
42. [breaker_box_open] 1:1 — Same breaker box as breaker_box with the door open showing a row of switches as flat color blocks.
43. [switch_box] 1:1 — Small wall light switch box with one big rocker switch, blue-grey. 0.15 m square. Front view.
44. [valve] 1:1 — Pipe valve with a red hand wheel on a short pipe stub, blue-grey pipe. 0.3 m across.
45. [wall_fan] 1:1 — Square wall ventilation fan: a steel box frame with four wide blades inside, blue-grey, blades stopped. 0.6 m square, 0.15 m deep. Front view.
46. [tube_light] 16:9 — Ceiling fluorescent tube light fixture, long thin box with a single tube, switched off, blue-grey. 1.2 m long, 0.15 m wide, 0.08 m tall.
47. [work_lamp] 1:1 — Clamp work lamp: a round metal shade with a bulb inside, a spring clamp at the base, blue-grey shade, lamp switched off. 0.3 m diameter, 0.4 m tall.
48. [lantern] 1:1 — Old metal storm lantern, switched off, thick handle loop on top, blue-grey. 0.35 m tall.
49. [shelf] 16:9 — Single wall-mounted wooden shelf on two brackets, faded ochre wood. 1.2 m long, 0.3 m deep. Front view.
50. [jars] 1:1 — A row of four glass jars and tins with cream labels, standing together as one object, lids in dull yellow and blue-grey. 0.6 m wide, 0.25 m tall.
51. [spill_feed] 1:1 — A flat irregular mound of spilled dull yellow grain, drawn alone as a thin flat shape. 1.5 m across, flat.
52. [straw_mat] 1:1 — A flat irregular spread of loose straw, dull yellow with darker strands, drawn alone as a thin flat shape. 2 m across, flat.
53. [drain] 1:1 — Round floor drain grate, blue-grey with a slot pattern as flat dark lines. 0.6 m across, flat.
54. [stain] 1:1 — A flat dark water stain shape, single darker grey color. 1.2 m across, flat.
55. [tarp_pile] 1:1 — A lumpy pile of unknown goods covered with a green tarpaulin tied down with a rope, drawn as one soft mass. 1.2 m by 1 m, 0.7 m tall.
56. [pallet_up] 1:1 — Wooden shipping pallet standing on its edge, leaning slightly. 1.2 m wide, 1 m tall.
57. [tires] 1:1 — Three old truck tires stacked, dark grey rubber. 0.7 m diameter, 0.6 m tall.
58. [sign_warn] 1:1 — Triangular warning sign plate, safety yellow with a thick black border and a single black exclamation shape, flat. 0.4 m. Front view.
59. [poster] 1:1 — A paper poster, cream paper with three colored rectangles and bars suggesting a hygiene chart, no letters. 0.4 m by 0.6 m. Front view.
60. [clock] 1:1 — Round wall clock with thick hands stopped at ten past ten, cream face, blue-grey rim. 0.3 m. Front view.
61. [calendar] 1:1 — Hanging wall calendar, cream pages with a grid of flat squares, one square marked red, no digits. 0.3 m by 0.4 m. Front view.
62. [coat_hook] 1:1 — Wall coat hook rail with one grey work overall hanging from it, the overall drawn as a soft chunky mass. 0.6 m wide, 1.7 m tall.
63. [boots] 1:1 — A pair of rubber work boots standing together, dark green. 0.3 m, 0.35 m tall.
64. [footbath] 1:1 — Flat rectangular disinfectant foot mat, blue-grey tray with a dark liquid surface. 0.9 m by 0.6 m, flat.
65. [radio] 1:1 — Old portable radio with a big round dial and a thick antenna, cream body with a blue-grey grille. 0.3 m wide.
66. [pipe_bundle] 16:9 — A bundle of five steel pipes lying down tied with two straps, blue-grey. 2.5 m long, 0.3 m across.
67. [cage_single] 1:1 — A single empty wire mesh poultry cage with its door hanging open, blue-grey frame, silver mesh. 0.6 m by 0.5 m, 0.5 m tall.
68. [egg_box_stack] 1:1 — A stack of closed brown cardboard egg boxes, three high, slightly uneven. 0.6 m by 0.4 m, 0.6 m tall.

TRACES OF THE PEOPLE WHO LEFT
69. [note_paper] 1:1 — A single sheet of cream paper pinned flat, handwriting suggested only by uneven dark grey lines, no readable letters. 0.2 m by 0.28 m. Front view.
70. [chalk_ledge] 1:1 — A tiny wooden wall ledge with two stubs of white chalk lying on it. 0.3 m wide.
71. [coop_door_open] 1:1 (views) — A small wooden coop door in its frame, swung wide open and sagging on one hinge, wire mesh panel in the door, faded ochre wood. 0.9 m wide, 1.8 m tall.
72. [chair_fallen] 1:1 — The same wooden office chair as chair, knocked over lying on its side.
73. [desk_left] 1:1 — A tin mug and an open ledger book lying together, drawn as one small object, cream pages with grey lines, no letters. 0.6 m wide.
74. [keys] 1:1 — A ring of three old keys lying flat, blue-grey metal, seen from above. 0.15 m.
75. [shovel_dirty] 1:1 — Same shovel as shovel but leaning, with dark soil caked on the blade.

TRACES OF THE NEW OWNER
76. [big_nest] 1:1 — A large round nest built from straw, a thick raised rim, empty inside, dull yellow straw with darker strands. 1.6 m across, 0.45 m tall.
77. [straw_drag] 16:9 — A flat spread of straw with long parallel drag marks all pointing one direction, dull yellow, drawn alone as a thin flat shape. 2.5 m by 1.2 m, flat.
78. [feathers] 1:1 — A small loose cluster of brown and orange feathers lying flat, drawn alone. 0.8 m across, flat.
79. [stitch_cloth] 1:1 — A torn scrap of pale cloth lying flat with thick dark stitches across it, like a seam from a stuffed toy. 0.4 m by 0.3 m, flat.

WHERE THE CHICKENS COME FROM
80. [duct_grill] 1:1 — A square wall ventilation grill with one corner of the louvre panel bent outward and hanging loose, dark opening behind it, blue-grey. 0.8 m square, 0.15 m deep. Front view.
81. [floor_hatch] 1:1 — A square steel floor hatch cover pushed aside from its opening, showing a dark gap, blue-grey. 0.9 m square, flat.

SHELL PIECES (texture reference only)
82. [wall_500] 16:9 — A flat wall panel of four wide horizontal wooden planks, faded ochre with dark seams, seen straight on, no frame. 5 m wide, 4 m tall.
83. [wall_500_mesh] 16:9 — Same plank wall as wall_500 but the top quarter is a band of silver wire mesh instead of planks.
84. [wall_500_pipe] 16:9 — Same plank wall as wall_500 with one horizontal blue-grey pipe running across it at mid height on small brackets.
85. [wall_500_chalk] 16:9 — Same plank wall as wall_500 with a cluster of white chalk tally marks drawn at eye level on the left half, no letters.
86. [floor_500] 1:1 — A flat square of grey concrete floor seen from directly above, subtle two-tone patches, no objects.
87. [floor_500_straw] 1:1 — Same concrete square as floor_500 with scattered straw bits in a loose arc, dull yellow.
88. [ceiling_500] 1:1 — A flat square ceiling panel seen from below with two parallel wooden rafters, faded ochre.
89. [ceiling_500_duct] 1:1 — Same ceiling panel as ceiling_500 with one round blue-grey air duct running across it.
90. [dock_floor_500] 1:1 — A flat square of grey concrete floor seen from above with a safety yellow stripe along one edge.
91. [dock_rail_250] 16:9 — Industrial loading dock barrier: a solid blue-grey panel on the lower half and steel wire mesh on the upper half, two posts. 2.5 m wide, 2.9 m tall. Front view.
92. [door_frame_250] 1:1 — A steel door frame, blue-grey, open in the middle, a blank name plate slot above the opening. 2.5 m wide. Front view.
93. [flap_door] 1:1 — A doorway filled with two hanging rubber flap panels, dark grey with a safety yellow stripe at the bottom. Front view.
94. [tread] 16:9 — One steel grating stair tread, blue-grey, anti-slip ridges as flat lines. 2.5 m long, 0.26 m deep.
95. [pen_fence_250] 16:9 — A chest-high wire mesh pen fence panel between two wooden posts, silver mesh, faded ochre posts. 2.5 m wide, 1.3 m tall. Front view.
96. [roof_500] 1:1 — A flat square of corrugated metal roof seen from above, blue-grey.
97. [pillar] 1:1 — A square steel H-beam column, blue-grey, with a bolted base plate. 0.4 m square, 4 m tall.

## Start
Reply first with one line confirming you understood the deliverables and the QA list, then
begin with object 1 and keep going. When all 97 (plus the view sets) are done, post the
manifest and the final report.
```


==================================================================
# PART 3 — docs/asset/tools/tripo.md
==================================================================

# Tripo Studio — 사용 기록과 권장 설정

> 2026-10-13 첫 사용. 계정 1992radar(사용자 결제), 크레딧 25,160으로 시작. Tripo는 **MCP가 아니라 브라우저 창**으로 쓴다 —
> Blender MCP의 Tripo 생성기는 MCP for Blender Premium 전용이라 꺼져 있고, 별도 Tripo MCP 서버는 등록돼 있지 않다.
> 세션 안에서는 내장 브라우저(`studio.tripo3d.ai`에 로그인된 탭)를 조작한다.
> 공정 전체와 오브젝트 목록은 [../map/henhouse_indoor_objects.md](../map/henhouse_indoor_objects.md), 프롬프트는
> [../map/henhouse_indoor_prompts.md](../map/henhouse_indoor_prompts.md), 파일 자리는 `ArtSource/map/henhouse_indoor/README.md`.

## 결론부터

- **텍스트 → 3D는 쓰지 않는다.** 첫 시험(케이지 열, 텍스트 + 폴리곤 6,000 제한)이 너덜너덜한 판 하나로 나왔다(작업 `8490100f`, 40 크레딧).
  스타일 지시("flat shading, visible triangles")가 형태 설명을 눌렀고, Tripo의 폴리곤 제한은 "저폴리로 설계"가 아니라 "고폴리를 깎기"라 철망 같은 얇은 반복 구조가 먼저 사라진다.
- **이미지 → 3D로 간다.** 이미지는 Tripo 안의 이미지 생성(무료 할당)이나 ChatGPT에서 만든다. 2026-10-13 시험에서 Tripo 이미지 생성(Nano Banana, 16:9, 4장)은
  네 장 모두 프롬프트대로 세 단 케이지 열이 나왔고 배경이 균일해 3D 입력으로 바로 쓸 수 있었다. 사용자는 이미지는 ChatGPT로 그리기로 했다(같은 날).
- **폴리곤 제한은 Tripo에서 걸지 않는다.** 2,000,000 그대로 두고 감면·정점색은 Blender 재단 공정에서 한다(objects.md §2.3).

## 화면과 경로

| 화면 | URL | 무엇 |
|---|---|---|
| 모델 생성 | `/ko/workspace/generate` | 텍스트·이미지·다중 시점 → 3D. 왼쪽 패널에서 입력 모드 아이콘 4개(이미지·?·다중 시점·텍스트 연필) |
| 이미지 생성 | `/ko/workspace/generate-image` | 텍스트 → 이미지. 생성된 장마다 "3D 생성"·"멀티 뷰 생성"·"오려내기"·"4K" 버튼 |
| 에셋 | `/ko/assets` | 생성 이력. 오른쪽 패널에도 최근 항목 썸네일 |
| DCC Bridge | 헤더 버튼 | Blender 등으로 바로 보내는 플러그인. **미검증** |

## 모델 생성 옵션과 비용 (2026-10-13 관찰)

| 항목 | 선택지 | 권장 | 비고 |
|---|---|---|---|
| 탭 | HD 모델 / 스마트 메시 | HD 모델 | 스마트 메시는 쿼드 리토폴로지(P2.0) |
| AI 모델 | H3.1 최고 품질 | 그대로 | "대기 시간이 길어집니다" — 실측 약 4분 30초 |
| 울트라 메시 품질 | 켬/끔 | 켬 | |
| 텍스처 | 켬/끔, 2K/4K/8K | 켬, **2K** | 8K는 "체험 ×1" 구독 특전 — 아낀다. 2K로 바꾸면 8K 스위치가 같이 꺼진다 |
| 조명 제거 | 켬/끔 | 켬 | 베이크된 그림자를 뺀다 |
| PBR | 켬/끔 | **끔** | 어차피 팔레트 정점색으로 다시 칠한다 |
| 토폴로지 | 쿼드 / 삼각형 | 삼각형 | |
| 폴리곤 수 | 500 ~ 2,000,000 | **2,000,000(제한 없음)** | 6,000으로 걸었더니 형태가 뭉개졌다 |
| 부분별 생성 | 체험 ×1 | 끔 | |
| 개인정보 보호 | 공유 전용 | 그대로 | |

비용(크레딧): 8K 켬 65 → 2K 45 → PBR 끔 **40**. 생성 뒤 "무료 재시도" 버튼이 뜬다(같은 입력 재생성으로 보임, 미검증).

## 이미지 생성 옵션과 비용

| 항목 | 값 |
|---|---|
| 모델 | Nano Banana (GPT Image 2.5 선택지도 있음) |
| 비율 | 1:1 · 16:9 · 9:16 · 4:3 · 3:4 — 가로로 긴 소품은 16:9 |
| 장 수 | 1~4 |
| 4K 해상도 | 켜면 10 크레딧 |
| 비용 | **무료 할당**(2026-10-13 "Free 4/1000") — 4장 생성해도 크레딧 변동 없음 |

## 입력 이미지 조건 (시험으로 확인)

단일 오브젝트, 3/4 정면, 화면 중앙, 균일한 짙은 회색 배경, 바닥면 없음, 바닥 그림자 없음, 글자 없음. 이 조건을 프롬프트 끝에 그대로 적으면 Nano Banana가 지켰다.
스타일 지시(로우폴리·평면 셰이딩)는 이미지 모델이 반만 듣는다 — 형태와 색만 맞으면 된다. 톤은 Blender 재단이 맡는다.

## 세션에서 브라우저로 조작할 때

- 텍스트 상자는 `type`이 안 먹을 때가 있다 — `form_input`으로 값을 넣는다.
- 지오메트리 옵션은 "지오메트리 및 텍스처" 행을 눌러 여는 팝오버 안에 있다. 팝오버 안 요소는 `read_page`로 ref를 얻어 누른다.
- 생성 버튼은 패널 맨 아래 노란 버튼. 누르면 URL이 `/generate/<작업 id>`로 바뀐다.
- 창이 좁으면 3D 뷰어가 잘린다 — 확인할 때 1600×900으로 넓히고 끝나면 desktop으로 되돌린다.
- 생성·크레딧 소모는 매번 사용자 확인 뒤에 누른다.

## 미검증·다음에 확인할 것

1. 내보내기 형식(GLB/FBX)과 좌표·스케일 — 2,000,000 폴리곤 결과를 Blender로 가져와 감면하는 첫 왕복.
2. DCC Bridge로 Blender에 직접 받는 길.
3. "무료 재시도"가 설정 변경을 허용하는지.
4. 다중 시점 입력(정면·측면·뒷면 3장)이 단일 3/4 이미지보다 나은지 — 케이지 열처럼 길고 반복적인 것에서.


==================================================================
# PART 4 — docs/asset/common.md
==================================================================

# 공통 제작 규격

시각 방향은 [리썰·R.E.P.O. 적용 기준](../reference-direction.md)을 먼저 확인한다.
어두운 공간에서 읽히는 큰 형태와 과장된 움직임을 기준으로, 아래 기술 규격을 적용한다.

## 크기와 좌표 — 준수

- 신규 Blender 씬: Metric, Unit Scale **1.0**, **1 Blender unit = 1m**. 제작 카드에는 cm로 기록한다.
  예: 500cm 벽은 Blender Dimensions 5m. cm 표시만 바꾸는 것과 실제 스케일 변경을 혼동하지 않는다.
- 신규 모델의 작업 정면은 **+X**, 위는 **+Z**. UE에서 액터의 +X가 전방이다.
  기존 스켈레톤을 이어 쓰는 모델은 해당 리그의 축·레스트 포즈가 우선이다.
- 내보내는 메시/리그 오브젝트는 지정 원점에 놓는다. 비균일·음수 Scale을 남기지 않는다.
  Rotation/Scale 적용은 **리깅 전에** 한다. 완성된 리그나 애니메이션에 일괄 적용하지 않는다.
- 작업용 1m 큐브, +X 화살표, 플레이어 캡슐 가이드를 `REF` 컬렉션에 둔다. 내보내기에서는 제외한다.
- 원점은 분류별 문서를 따른다. 원점·바운드·충돌 크기는 서로 다른 값이다.
  1m 기준 큐브가 UE에서 100cm인지 확인하기 전에는 배율을 확정하지 않는다.

## 이름과 씬 정리 — 신규 제작 기본

| 대상 | 예시 |
|---|---|
| 제작 카드/원본 ID | `chicken_a`, `egg_raw`, `henhouse_wall_500_a` |
| Static Mesh | `SM_Egg_Raw`, `SM_Henhouse_Wall_500_A` |
| Skeletal Mesh / Skeleton | `SK_Chicken_A`, `SKEL_Chicken` |
| 애니메이션 | `AN_Chicken_Walk`, `AN_Chicken_HitReact` |
| Material / Instance | `M_CL9_Wood`, `MI_Henhouse_Wood_A` |
| Texture | `T_Chicken_A_BC`, `_N`, `_ORM`, `_E` |

파일/오브젝트명은 영문·숫자·밑줄로 통일하고 `Cube.001` 같은 자동 이름을 납품하지 않는다.
컬렉션은 `EXPORT`(모델·내보낼 뼈), `RIG_CONTROLS`, `COLLISION`, `REF`, `PREVIEW`로 나눈다.
분류는 내보내기 자동화를 이미 구현했다는 뜻이 아니다. 실제 선택 대상을 확인한다.
공개된 UE 에셋명은 유지해 재임포트하고, v002 같은 작업 버전은 제작 카드에 기록한다.

## 메시·UV·재질

- 보이지 않는 내부 면·중복 정점·뒤집힌 노멀·불필요한 미세 베벨을 정리한다.
  리깅 모델은 관절 주변 변형을 우선하고, 내보내기 사본에서 삼각분할 결과를 확인한다.
- UV0는 표면용. 겹침/미러는 의도한 경우만 사용한다. 베이크 영역은 여백을 두고 실제 밉맵에서 확인한다.
  정적 조명 베이크를 쓰는 메시만 별도 비중첩 Lightmap UV 필요 여부를 UE에서 확정한다.
- Blender 전용 셰이더 노드는 자동 이식 대상으로 보지 않는다. 색·노멀·거칠기 등을 이미지로
  베이크하거나 UE 머티리얼로 재구성한다. 텍스처는 PNG/TGA, 경로는 상대 경로로 보관한다.
- 기본 패킹: BC=Base Color, N=Normal, ORM의 R=AO/G=Roughness/B=Metallic, E=Emissive.
  BC는 sRGB, N·ORM은 선형 데이터. 노멀은 UE용 DirectX 방향으로 맞추고 베이크 방식에 따라
  Green 반전이 필요한지 확인한다. 색 공간과 채널 규칙은 납품 카드에 기록한다.
- 불빛/그림자를 Base Color에 강하게 구워 넣지 않는다. 발광 표면과 실제 광원은 분리한다.
  광원 범위·정전 반응·발각 효과는 엔진의 데이터/로직이 담당한다.
- 기본은 Opaque. 철망·볏짚의 Masked 사용은 겹침 비용을 확인한다. 털·깃털 한 가닥씩을
  만들기보다 큰 덩어리로 표현한다. Two Sided를 모든 재질에 일괄 적용하지 않는다.

## 성능 예산 — 측정 전 초안

아래는 엔진 한계나 확정 성능 보증이 아니라 **LOD0 제작 목표**다. 초과 시 이유와 실측을 카드에 남긴다.

| 대상 | 삼각형 목표 | 재질 슬롯 목표 | 텍스처 기본 |
|---|---:|---:|---|
| 작은 재료·손전등 | 500~3,000 | 1~2 | 512~1K |
| 수레·조리대·큰 소품 | 2,000~8,000 | 1~3 | 1K~2K |
| 반복 벽·울타리 조각 | 500~4,000 | 1~2 | 공유 타일/트림 1K~2K |
| 동물 한 마리 | 5,000~15,000 | 1~2 | 1K~2K |
| 플레이어 풀바디 | 15,000~30,000 | 2~3 | 2K |
| 1인칭 팔 한 쌍 | 8,000~20,000 | 1~2 | 2K |

LOD1 약 50%, LOD2 약 25%를 출발점으로 삼되 눈·부리·손가락·관절 실루엣을 지킨다.
작은 소품은 거리 컬링이 더 적절할 수 있다. Nanite 사용 여부는 임포트/성능 검수에서 결정한다.
최대 동물 수, 4인 캐릭터, 반복 조각이 함께 보이는 UE 장면에서 프레임 비용을 확인한다.

## 스켈레톤·애니메이션 기본

첫 왕복 [004](../iterations/004.md): Unit Scale=1, FBX +X/+Z, UE convert_scene/unit 및
force_front_x_axis=True, import_uniform_scale=1로 닭/너구리 크기 오차 <0.00002cm 확인.
전 프레임 발 위치는 UE Y축 반전 반영 시 원본과 <0.01cm 일치한다. 캐릭터는 별도 검수한다.
UE armature wrapper/단위 배율을 임의 삭제·적용하지 않는다. VertexColor 재질의 첫 출력 이름은
빈 문자열이며 연결 반환값·실제 입력 노드를 확인한다. SkeletalMesh usage도 설정한다.

- 새 동물 리그는 단일 `root` 아래에 변형 뼈를 둔다. `root`는 바닥 원점이며 이동 클립에서 고정한다.
  좌우 뼈는 `_l`/`_r`; 컨트롤/IK 뼈와 내보낼 뼈를 구분한다. 기존 리그 이름은 바꾸지 않는다.
- 신규 스킨은 정점당 영향 뼈 최대 4개를 제작 목표로 삼고 가중치 합 1, 미할당 정점 0을 검사한다.
- 기본 작업/베이크 **30fps**, 클립 하나당 Action 하나. 60fps는 손 동작 등 필요할 때 카드에 명시한다.
  시간은 초가 기준이다. 30fps에서 0.55초처럼 정수 프레임이 아닌 길이는 서브프레임 또는 UE 재생 배율로 맞춘다.
- 제자리 이동(in-place), Root Motion 없음이 기본. IK/제약 결과를 변형 뼈에 베이크한다.
  루프의 첫/끝 포즈와 발 접지, 속도 연속성을 확인하며 경계 프레임 중복으로 멈칫하지 않게 한다.
- 걷기/뛰기는 1배속 기준 cm/s와 보폭·주기를 함께 납품한다. UE는 현재 속도/기준 속도로 재생한다.
  애니메이션이 이동·공격·데미지 판정을 만들지 않는다. [애니메이션 시스템](../../systems/animation.md)이 우선한다.


==================================================================
# PART 5 — ArtSource/map/henhouse_indoor/README.md
==================================================================

# ArtSource/map/henhouse_indoor — 양계장(실내 계사) 오브젝트 작업 폴더

2026-10-13 시작. 계속 쓰는 파이프라인이라 오브젝트마다 같은 구조를 지킨다.
정본 문서: 목록·톤·배치 규칙 [docs/asset/map/henhouse_indoor_objects.md](../../../docs/asset/map/henhouse_indoor_objects.md),
이미지 프롬프트 [docs/asset/map/henhouse_indoor_prompts.md](../../../docs/asset/map/henhouse_indoor_prompts.md),
Tripo 설정 [docs/asset/tools/tripo.md](../../../docs/asset/tools/tripo.md).

## 공정

```
ChatGPT 이미지  →  Tripo 이미지→3D  →  Blender 재단  →  FBX 내보내기  →  UE 임포트·PropSets
  concept/           tripo/             blender/          export/            (Content/Maps/Henhouse/…)
```

1. **concept/** — ChatGPT가 그린 입력 이미지. `prompt.md`에 보낸 프롬프트와 날짜를 적는다.
2. **tripo/** — Tripo가 내보낸 원본(GLB/FBX)과 `job.md`(작업 id·설정·크레딧·어느 concept 파일을 썼나). 조건을 바꿔 다시 뽑으면 `v02/`.
3. **blender/** — 재단 작업 파일 `<id>.blend`. 감면·팔레트 정점색·피벗(바닥 중앙)·정면 +X·치수 맞춤·UCX 충돌.
4. **export/** — 납품 FBX `SM_Henhouse_<Name>_<Var>.fbx`와 `manifest.json`(기존 ArtSource 규약과 같은 형식 — `henhouse_structure/ceiling/export/manifest.json` 참고).
5. **preview/** — Blender·UE 검수 스크린샷(중립광·작업등·손전등 세 장).

## 폴더 하나의 모양

```
<id>/                      ← prompts.md 표의 id 그대로 (snake_case)
  prompt.md                ← 보낸 프롬프트·날짜·비고
  concept/
    <id>_concept_v01.png   ← 3/4 정면 한 장. 시점 셋이면 _front/_side/_back
  tripo/
    job.md                 ← 작업 id, 설정, 크레딧, 입력 파일, 결과 평
    v01/<id>_tripo.glb
  blender/
    <id>.blend
  export/
    SM_Henhouse_<Name>_A.fbx
    manifest.json
  preview/
    <id>_neutral.png · <id>_worklamp.png · <id>_flashlight.png
```

`_template/`를 복사해서 시작한다. 비어 있는 폴더는 `.gitkeep`으로 유지한다.

## 규칙

- 폴더 id와 메시 이름은 1:1이다 — `cage_row` ↔ `SM_Henhouse_CageRow_A`. 변형(A/B/C)은 같은 폴더 안에서 export 파일 이름으로만 갈린다.
- 원본(concept·tripo)은 지우지 않는다. 다시 뽑으면 버전을 올린다.
- Tripo에서 받은 GLB는 수십 MB가 될 수 있다 — `tripo/`에는 채택한 버전만 두고 나머지는 Tripo 에셋 목록에서 다시 받는다.
- 재단 결과의 삼각형 수·치수·피벗은 `manifest.json`에 적어야 UE 쪽 검수가 읽는다.
- 상태는 이 README에 적지 않는다. 진행은 `docs/asset/production-progress.md`, 결정은 `docs/decisions.md`.

## 누가 하나 (2026-10-13)

**이미지·Tripo·Blender 재단·FBX 납품은 전부 ChatGPT가 한다** — 브리프 [docs/asset/map/henhouse_indoor_handoff.md](../../../docs/asset/map/henhouse_indoor_handoff.md).
이쪽은 돌아온 폴더를 UE에 임포트하고 `PropSets`에 연결한다. 진행 기록은 ChatGPT가 쓰는 `PRODUCTION_LOG.md`(이 폴더 루트)가 정본이다.

## 오브젝트 폴더 목록

prompts.md §3의 id 전부가 후보다. 폴더는 **착수할 때** 만든다 — 지금 있는 것:

| id | 상태 |
|---|---|
| `cage_row` | concept: Tripo 이미지 생성 4장 시험(2026-10-13, 저장 안 함). 텍스트→3D 실패 1회(작업 8490100f). ChatGPT 이미지 대기 |


==================================================================
# PART 6 — ArtSource/map/henhouse_structure/ceiling/export/manifest.json
==================================================================

{
  "source": "Original CL9 geometry through Blender MCP",
  "source_blend": "ceiling.blend",
  "pivot": "floor center",
  "axis": "+X forward / +Z up",
  "materials": {
    "M_CL9_Ceiling_Wood": {
      "vertex_color": true,
      "roughness": 0.91,
      "metallic": 0
    }
  },
  "assets": {
    "henhouse_ceiling": {
      "object": "SM_Henhouse_Ceiling_500_A",
      "dimensions_cm": [
        500.0,
        500.0000476837158,
        30.000001192092896
      ],
      "triangles": 1000,
      "vertices": 516,
      "uv_layers": 1,
      "materials": [
        "M_CL9_Ceiling_Wood"
      ],
      "runtime_radius_cm": null,
      "closed_surfaces": true,
      "fbx_sha256": "be25dc5b1572c436ecb77af766a46223d18f1be09464c77f988f07f955b89d4e",
      "anchors_cm": {
        "EdgeLeft": [
          -250.0,
          -2.384185791015625e-05,
          0.0
        ],
        "EdgeRight": [
          250.0,
          -2.384185791015625e-05,
          0.0
        ],
        "EdgeFront": [
          0.0,
          -250.00003051757812,
          0.0
        ],
        "EdgeBack": [
          0.0,
          249.99996948242188,
          0.0
        ]
      },
      "rig": "N/A static structure",
      "bounds_center_cm": [
        0.0,
        0.0,
        15.000000953674316
      ],
      "collision_hulls": 1,
      "repair": "조이스트5/테두리2의 정확한 합집합; 판재10/상부판/UCX 보존"
    },
    "henhouse_ceiling_stretch100": {
      "object": "SM_Henhouse_Ceiling_Stretch100_A",
      "dimensions_cm": [
        100.0,
        100.00001192092896,
        100.0
      ],
      "triangles": 1000,
      "vertices": 516,
      "uv_layers": 1,
      "materials": [
        "M_CL9_Ceiling_Wood"
      ],
      "runtime_radius_cm": null,
      "closed_surfaces": true,
      "fbx_sha256": "b03886e1e1c0d8eeb8fbfd9210eb402548d556f1e0710680d6fe375df179f949",
      "anchors_cm": {},
      "rig": "N/A static structure",
      "bounds_center_cm": [
        0,
        0,
        0
      ],
      "collision_hulls": 1,
      "pivot_override": "bounds center",
      "restore_scale": [
        5,
        5,
        0.3
      ],
      "consumer": "CL9SiteMap Stretch: scale size/100, centered pivot",
      "repair": "조이스트5/테두리2의 정확한 합집합; 판재10/상부판/UCX 보존"
    }
  }
}
