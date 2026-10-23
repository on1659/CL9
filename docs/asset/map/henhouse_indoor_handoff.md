# 양계장(실내 계사) 오브젝트 제작 — ChatGPT 전체 이관 브리프

> 2026-10-13 사용자 결정: **이미지 → Tripo 3D → Blender 재단 → FBX 납품까지 전부 ChatGPT가 한다.** 이 세션(Claude)의 역할은 여기서 끝이다.
> **ChatGPT에는 `henhouse_indoor_handoff_bundle.md`의 내용을 통째로 복사해 첫 메시지로 붙여 넣는다 — 그 파일 전체가 프롬프트 하나다.** 브리프 + 명세 전부가 들어 있어 첨부도 안내 문장도 필요 없다. 너무 길어 붙여 넣기가 막히면 파일로 올리고 "Follow the brief at the top. Start now."만 적는다.
> 원본 문서가 바뀌면 아래 명령으로 번들을 다시 만든다(번들은 생성물이라 손으로 고치지 않는다).
>
> ```bash
> cd /c/Work/CL9 && { sed -n '/^# HAND-OFF BRIEF/,$p' docs/asset/map/henhouse_indoor_handoff.md; n=1; for f in docs/asset/map/henhouse_indoor_objects.md docs/asset/map/henhouse_indoor_gpt_prompt.md docs/asset/tools/tripo.md docs/asset/common.md ArtSource/map/henhouse_indoor/README.md ArtSource/map/henhouse_structure/ceiling/export/manifest.json; do echo; echo; echo "=================================================================="; echo "# PART $n — $f"; echo "=================================================================="; echo; cat "$f"; n=$((n+1)); done; } > docs/asset/map/henhouse_indoor_handoff_bundle.md
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
