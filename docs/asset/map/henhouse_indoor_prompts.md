# 양계장(실내 계사) 오브젝트 — ChatGPT 이미지 프롬프트 모음

> 2026-10-13. [henhouse_indoor_objects.md](henhouse_indoor_objects.md)의 오브젝트마다 **이미지 한 장을 ChatGPT로 그리고**, 그 이미지를
> Tripo 이미지→3D 입력으로 쓴다([../tools/tripo.md](../tools/tripo.md)). 이 문서는 그 이미지 프롬프트의 정본이다.
> 오브젝트 ID는 `ArtSource/map/henhouse_indoor/<id>/` 폴더 이름과 같다. 결과 이미지는 `<id>/concept/<id>_concept_v01.png`로 저장한다.
>
> 2026-10-13 시험(Tripo 이미지 생성, 케이지 열)에서 §1 조건을 프롬프트 끝에 그대로 적었을 때 네 장 모두 입력으로 쓸 수 있는 그림이 나왔다.
> 스타일 지시는 이미지 모델이 반만 듣는다 — **형태·비례·색만 맞으면 된다.** 로우폴리 톤은 Blender 재단이 맡는다.

---

## 1. ChatGPT에 먼저 붙여 넣는 머리글

아래 블록을 대화 첫 메시지로 넣고, 그 뒤에 §3의 오브젝트 행을 하나씩 보낸다. 한 대화에서 이어 가면 톤이 유지된다.

```
You are generating reference images for a 3D game asset pipeline. Every image you make in this
conversation must follow these fixed rules. Do not restate them; just apply them.

STYLE
- Stylized low-poly game prop look: chunky simplified forms, large flat color areas, faceted
  surfaces. No photorealism, no fine texture detail, no decals, no text, no logos.
- Exaggerate the functional part of each object (doors, spouts, handles, lamp shades) and keep the
  body a simple solid mass. Thin wires and handles are drawn 2-3x thicker than real.
- At most three flat colors per object plus one accent. Palette:
  wood planks = faded ochre #C9A86A with dark brown-grey #6B5A48 shadows
  structural steel = blue-grey #4A5A66 (dark #2C3640), rust orange #B5562A only on edges and bottoms
  wire mesh = silver grey #8E959A
  straw / feed = dull yellow #D9B44A (dark #9C7A2E)
  concrete = #7A7A74 (dark #5C5C57)
  safety yellow = #E8C020 (only for floor lines, warning signs, electrical boxes)
  plastic = teal #2E8B7A or red #C9403A (one per object)
  paper / cloth = cream #EFE6CF, small areas only
- Never use white or bright cream on large surfaces (the game's eggs are that color).
- Clean, maintained industrial farm feeling: "people left yesterday", not ruins. No cobwebs, no blood.

FRAMING (critical — the image is fed to an image-to-3D tool)
- Exactly one object, fully visible, centered, nothing cropped.
- Three-quarter front view from slightly above eye level.
- Plain uniform dark grey background (#3A3A3A). No ground plane, no floor, no cast shadow, no
  environment, no props around it, no people, no animals.
- Soft even lighting, no dramatic highlights.
- Landscape 16:9 for wide objects, square 1:1 for compact objects (each request says which).

OUTPUT
- One image per request unless I ask for views. If I ask for "views", give three separate images:
  front, side, back, same object, same framing.
Reply "ready" and wait for the first object.
```

## 2. 오브젝트 한 줄 보내는 형식

```
[<id>] <비율> — <설명>
```

예: `[cage_row] 16:9 — Industrial poultry battery cage row ...`

ID는 §3 표 첫 칸이다. 큰 소품과 좌우 비대칭 소품(책상·손수레·셔터)은 한 장 뽑아 본 뒤 괜찮으면 "views"로 세 장을 더 받아 Tripo 다중 시점 입력에 쓴다.

---

## 3. 오브젝트별 프롬프트

비율·치수는 objects.md §4와 같다. 설명은 영문 그대로 보낸다. 치수는 비례를 잡으라고 주는 것이고 글자로 그리라는 뜻이 아니다.

### 3.B 방의 정체 소품

| id | 비율 | 프롬프트 |
|---|---|---|
| `cage_row` | 16:9 | Industrial poultry battery cage row: three tiers of wire mesh cages on a blue-grey steel frame, three cages per tier, chunky exaggerated cage doors, a shallow feed trough running along the front of each tier, dull yellow straw in the bottom of each cage. Proportions 5 m long, 1 m deep, 2.2 m tall. |
| `cage_row_open` | 16:9 | Same battery cage row as cage_row but every cage door swung open and hanging, straw spilling out of the lower tier. |
| `nest_box` | 16:9 | Wooden laying nest box unit with three side-by-side compartments, each filled with straw, a raised front lip on each compartment, a sloped plank roof on top, faded ochre wood with dark edges. Proportions 2.5 m long, 0.6 m deep, 1.2 m tall. |
| `nest_box_used` | 16:9 | Same nest box unit as nest_box but the straw is pressed flat into shallow bowls as if sat on, a few straw strands hanging over the front lips. |
| `shutter` | 16:9 | Closed industrial roll-up loading dock shutter door: six wide horizontal steel slats, blue-grey, a thick bottom rail, side guide rails. Proportions 5 m wide, 0.3 m deep, 3 m tall. |
| `pallet` | 1:1 | Wooden shipping pallet lying flat, thick planks, faded ochre wood. Proportions 1.2 m by 1 m, 0.15 m tall. |
| `barrel_fire` | 1:1 | Rusted steel drum used as a fire barrel, blue-grey with rust orange around the top rim and bottom, a few rectangular holes cut in the upper side, no fire, no smoke. Proportions 0.6 m diameter, 0.9 m tall. |
| `corner_mirror` | 1:1 | Convex safety mirror on a short wall bracket: round domed mirror disc with a thick orange rim, small steel arm behind it. Proportions 0.4 m diameter. |
| `handrail` | 16:9 | Simple steel pipe handrail section with two posts and one top rail, blue-grey, chunky pipe. Proportions 2.5 m long, 1 m tall. |
| `water_line` | 16:9 | Poultry nipple drinker line: one long horizontal pipe at knee height on two short brackets, small drinker nipples hanging underneath at regular spacing, blue-grey pipe. Proportions 5 m long, 0.6 m tall. |
| `silo_base` | 1:1 | Bottom section of a feed silo indoors: an inverted cone funnel body on four steel legs, a round discharge spout at the bottom, a vertical pipe rising from the top edge, blue-grey steel with rust on the lower cone. Proportions 1.2 m square footprint, 2.5 m tall. |
| `sign_board` | 16:9 | Hanging directional sign board: a flat rectangular panel suspended from two short chains, showing a single large arrow shape as a flat color block, no letters. Proportions 1.2 m wide, 0.4 m tall. |
| `brooder` | 16:9 | Two chick brooder boxes joined as one unit: wooden boxes with hinged lids half open, a heat lamp shade on a short arm over each box, straw inside, faded ochre wood with a blue-grey lamp shade. Proportions 2.5 m long, 1 m deep, 1.2 m tall. |
| `tool_board` | 16:9 | Wall-mounted tool board: a flat wooden panel with a shovel, a rake, a pitchfork and a hoe hanging on hooks, tool heads exaggerated, faded ochre wood, blue-grey tool heads. Proportions 2.5 m wide, 1.5 m tall, 0.15 m deep. Front view is fine. |
| `rack` | 16:9 | Industrial metal storage rack with three shelves, a few wooden crates and sacks sitting on the shelves as simple blocks, blue-grey steel frame. Proportions 2.5 m long, 0.6 m deep, 2.2 m tall. |
| `desk` | 16:9 | Plain farm office desk: thick wooden top on a blue-grey steel frame, a closed ledger book and a chunky old telephone sitting on top as part of the object. Proportions 1.6 m wide, 0.8 m deep, 0.75 m tall. |
| `chair` | 1:1 | Simple wooden office chair with a steel frame, upright, chunky legs. Proportions 0.5 m by 0.5 m, 0.9 m tall. |
| `notice_board` | 16:9 | Wall notice board: a cork panel in a wooden frame with a few blank paper sheets pinned on it as flat cream rectangles, no writing. Proportions 1.2 m wide, 0.9 m tall, 0.05 m deep. Front view. |
| `grading_table` | 16:9 | Egg grading table: a long steel table with a row of rollers along its top and an empty plastic tray at one end, blue-grey steel, teal tray. Proportions 2.5 m long, 0.8 m deep, 0.9 m tall. |
| `hood` | 16:9 | Stainless kitchen extractor hood, a wide tapered box, blue-grey, meant to hang from a ceiling. Proportions 2.5 m wide, 0.6 m deep, 0.6 m tall. |
| `floor_sign` | 1:1 | Small square wall plate with one thick horizontal color stripe (teal), no numbers, blue-grey plate. Proportions 0.4 m square. Front view. |

### 3.C 공용 소품

| id | 비율 | 프롬프트 |
|---|---|---|
| `crate` | 1:1 | Wooden storage crate with corner battens, faded ochre wood, dark edges. Proportions 0.6 m by 0.6 m, 0.5 m tall. |
| `crate_stack` | 1:1 | Two wooden crates stacked, the top one slightly offset, same crate as crate. 1 m tall. |
| `sack_up` | 1:1 | Standing feed sack, full and rounded, the top gathered and tied with an exaggerated knot, cream cloth with a dull yellow band. Proportions 0.5 m wide, 0.8 m tall. |
| `sack_pile` | 16:9 | Three feed sacks lying in a loose heap as one mass, cream cloth with dull yellow bands. Proportions 1.2 m by 0.8 m, 0.6 m tall. |
| `sack_spill` | 16:9 | A feed sack torn open on its side with a mound of dull yellow grain spilling out of it onto the ground, sack and grain as one object. Proportions 1 m by 0.8 m, 0.35 m tall. |
| `drum` | 1:1 | Steel storage drum, closed top, two raised ribs around the body, dark blue. Proportions 0.6 m diameter, 0.9 m tall. |
| `bucket` | 1:1 | Galvanized metal bucket with a thick wire handle standing up, blue-grey. Proportions 0.3 m diameter, 0.35 m tall. |
| `trough` | 16:9 | Long shallow animal water trough, a simple steel channel on two short legs, blue-grey. Proportions 2.5 m long, 0.4 m wide, 0.4 m tall. |
| `egg_tray_stack` | 1:1 | A stack of eight empty teal plastic egg trays, dimpled tops, slightly uneven stack. Proportions 0.3 m square, 0.4 m tall. |
| `basket` | 1:1 | Red plastic crate basket with grid holes drawn as flat darker squares, empty. Proportions 0.5 m by 0.35 m, 0.3 m tall. |
| `handcart` | 1:1 | Two-wheeled hand truck standing upright, blue-grey steel frame, thick rubber wheels, empty. Proportions 0.6 m wide, 0.4 m deep, 1.2 m tall. |
| `handcart_loaded` | 1:1 | Same hand truck as handcart but with two brown cardboard egg boxes strapped on the plate. |
| `ladder` | 1:1 | Wooden straight ladder leaning at a slight angle, thick rungs, faded ochre wood. Proportions 0.4 m wide, 2.5 m tall. |
| `broom` | 1:1 | Push broom and a dustpan leaning together against nothing, wooden handle, blue-grey dustpan, drawn as one object. Proportions 0.3 m wide, 1.4 m tall. |
| `shovel` | 1:1 | Farm shovel standing upright, exaggerated wide blade, wooden handle, blue-grey blade. 1.5 m tall. |
| `rake` | 1:1 | Farm rake standing upright, exaggerated wide head, wooden handle. 1.5 m tall. |
| `pitchfork` | 1:1 | Pitchfork standing upright, four thick tines, wooden handle. 1.5 m tall. |
| `hose_reel` | 1:1 | Wall-mounted hose reel with a coiled green hose, blue-grey bracket, the coil drawn as a solid ring. Proportions 0.5 m wide, 0.3 m deep, 0.6 m tall. Front view. |
| `extinguisher` | 1:1 | Red fire extinguisher on a small wall hook, chunky nozzle. 0.6 m tall. Front view. |
| `breaker_box` | 1:1 | Wall electrical breaker box, closed grey door with a safety yellow stripe across it, blue-grey body. Proportions 0.6 m wide, 0.8 m tall, 0.15 m deep. Front view. |
| `breaker_box_open` | 1:1 | Same breaker box as breaker_box with the door open showing a row of switches as flat color blocks. |
| `switch_box` | 1:1 | Small wall light switch box with one big rocker switch, blue-grey. 0.15 m square. Front view. |
| `valve` | 1:1 | Pipe valve with a red hand wheel on a short pipe stub, blue-grey pipe. 0.3 m across. |
| `wall_fan` | 1:1 | Square wall ventilation fan: a steel box frame with four wide blades inside, blue-grey, blades stopped. 0.6 m square, 0.15 m deep. Front view. |
| `tube_light` | 16:9 | Ceiling fluorescent tube light fixture, long thin box with a single tube, switched off, blue-grey. Proportions 1.2 m long, 0.15 m wide, 0.08 m tall. |
| `work_lamp` | 1:1 | Clamp work lamp: a round metal shade with a bulb inside, a spring clamp at the base, blue-grey shade, lamp switched off. Proportions 0.3 m diameter, 0.4 m tall. |
| `lantern` | 1:1 | Old metal storm lantern, switched off, thick handle loop on top, blue-grey. 0.35 m tall. |
| `shelf` | 16:9 | Single wall-mounted wooden shelf on two brackets, faded ochre wood. Proportions 1.2 m long, 0.3 m deep. Front view. |
| `jars` | 1:1 | A row of four glass jars and tins with cream labels, standing together as one object, lids in dull yellow and blue-grey. Proportions 0.6 m wide, 0.25 m tall. |
| `spill_feed` | 1:1 | A flat irregular mound of spilled dull yellow grain on the ground, drawn alone as a thin flat shape, seen from a high three-quarter angle. 1.5 m across, very flat. |
| `straw_mat` | 1:1 | A flat irregular spread of loose straw on the ground, dull yellow with darker strands, drawn alone as a thin flat shape, seen from a high three-quarter angle. 2 m across, very flat. |
| `drain` | 1:1 | Round floor drain grate, blue-grey with slot pattern as flat dark lines, seen from a high angle. 0.6 m across, flat. |
| `stain` | 1:1 | A flat dark water stain shape on the ground, single darker grey color, seen from a high angle. 1.2 m across, flat. |
| `tarp_pile` | 1:1 | A lumpy pile of unknown goods covered with a green tarpaulin tied down with a rope, drawn as one soft mass. Proportions 1.2 m by 1 m, 0.7 m tall. |
| `pallet_up` | 1:1 | Wooden shipping pallet standing on its edge, leaning slightly. 1.2 m wide, 1 m tall. |
| `tires` | 1:1 | Three old truck tires stacked, dark grey rubber. 0.7 m diameter, 0.6 m tall. |
| `sign_warn` | 1:1 | Triangular warning sign plate, safety yellow with a thick black border and a single black exclamation shape, flat. 0.4 m. Front view. |
| `poster` | 1:1 | A paper poster pinned to nothing, cream paper with three colored rectangles and bars suggesting a hygiene chart, no letters. 0.4 m by 0.6 m. Front view. |
| `clock` | 1:1 | Round wall clock with thick hands stopped at ten past ten, cream face, blue-grey rim. 0.3 m. Front view. |
| `calendar` | 1:1 | Hanging wall calendar, cream pages with a grid of flat squares, one square marked red, no digits. 0.3 m by 0.4 m. Front view. |
| `coat_hook` | 1:1 | Wall coat hook rail with one grey work overall hanging from it, the overall drawn as a soft chunky mass. 0.6 m wide, 1.7 m tall. |
| `boots` | 1:1 | A pair of rubber work boots standing together, dark green. 0.3 m, 0.35 m tall. |
| `footbath` | 1:1 | Flat rectangular disinfectant foot mat, blue-grey tray with a dark liquid surface, seen from a high three-quarter angle. 0.9 m by 0.6 m, very flat. |
| `radio` | 1:1 | Old portable radio with a big round dial and a thick antenna, cream body with a blue-grey grille. 0.3 m wide. |
| `pipe_bundle` | 16:9 | A bundle of five steel pipes lying on the ground tied with two straps, blue-grey. 2.5 m long, 0.3 m across. |
| `cage_single` | 1:1 | A single empty wire mesh poultry cage with its door hanging open, blue-grey frame, silver mesh. 0.6 m by 0.5 m, 0.5 m tall. |
| `egg_box_stack` | 1:1 | A stack of closed brown cardboard egg boxes, three high, slightly uneven. 0.6 m by 0.4 m, 0.6 m tall. |

### 3.D 사람이 떠난 흔적

| id | 비율 | 프롬프트 |
|---|---|---|
| `note_paper` | 1:1 | A single sheet of cream paper pinned flat, with handwriting suggested only by uneven dark grey lines, no readable letters. 0.2 m by 0.28 m. Front view. |
| `chalk_ledge` | 1:1 | A tiny wooden wall ledge with two stubs of white chalk lying on it. 0.3 m wide. |
| `coop_door_open` | 1:1 | A small wooden coop door in its frame, swung wide open and sagging on one hinge, wire mesh panel in the door, faded ochre wood. 0.9 m wide, 1.8 m tall. |
| `chair_fallen` | 1:1 | The same wooden office chair as chair, knocked over lying on its side. |
| `desk_left` | 1:1 | A tin mug and an open ledger book lying together on a flat surface, drawn as one small object, cream pages with grey lines, no letters. 0.6 m wide. |
| `keys` | 1:1 | A ring of three old keys lying flat, blue-grey metal, seen from above. 0.15 m. |
| `shovel_dirty` | 1:1 | Same shovel as shovel but leaning, with dark soil caked on the blade. |

### 3.E 주인이 차지한 흔적

| id | 비율 | 프롬프트 |
|---|---|---|
| `big_nest` | 1:1 | A large round nest built from straw on the ground, a thick raised rim, empty inside, dull yellow straw with darker strands, seen from a high three-quarter angle. 1.6 m across, 0.45 m tall. |
| `straw_drag` | 16:9 | A flat spread of straw on the ground with long parallel drag marks all pointing one direction, dull yellow, drawn alone as a thin flat shape, high angle. 2.5 m by 1.2 m, flat. |
| `feathers` | 1:1 | A small loose cluster of brown and orange feathers lying flat on the ground, drawn alone, high angle. 0.8 m across, flat. |
| `stitch_cloth` | 1:1 | A torn scrap of pale cloth lying flat with thick dark stitches across it, like a seam from a stuffed toy. 0.4 m by 0.3 m, flat. |

### 3.F 닭이 나오는 자리

| id | 비율 | 프롬프트 |
|---|---|---|
| `duct_grill` | 1:1 | A square wall ventilation grill with one corner of the louvre panel bent outward and hanging loose, dark opening behind it, blue-grey. 0.8 m square, 0.15 m deep. Front view. |
| `floor_hatch` | 1:1 | A square steel floor hatch cover pushed aside from its opening, showing a dark gap, blue-grey, seen from a high three-quarter angle. 0.9 m square, flat. |

### 3.A 껍데기 (참고용 — 모델은 Blender 규격 제작)

벽·바닥·천장은 Tripo로 만들지 않지만, 질감·색 참고로 한 장씩 뽑아 두면 Blender 작업이 빠르다.

| id | 비율 | 프롬프트 |
|---|---|---|
| `wall_500` | 16:9 | A flat wall panel of four wide horizontal wooden planks, faded ochre with dark seams, seen straight on, no frame. 5 m wide, 4 m tall. |
| `wall_500_mesh` | 16:9 | Same plank wall as wall_500 but the top quarter is a band of silver wire mesh instead of planks. |
| `wall_500_pipe` | 16:9 | Same plank wall as wall_500 with one horizontal blue-grey pipe running across it at mid height on small brackets. |
| `wall_500_chalk` | 16:9 | Same plank wall as wall_500 with a cluster of white chalk tally marks drawn at eye level on the left half, no letters. |
| `floor_500` | 1:1 | A flat square of grey concrete floor seen from directly above, subtle two-tone patches, no objects. |
| `floor_500_straw` | 1:1 | Same concrete square as floor_500 with scattered straw bits in a loose arc, dull yellow. |
| `ceiling_500` | 1:1 | A flat square ceiling panel seen from below with two parallel wooden rafters, faded ochre. |
| `ceiling_500_duct` | 1:1 | Same ceiling panel as ceiling_500 with one round blue-grey air duct running across it. |
| `dock_floor_500` | 1:1 | A flat square of grey concrete floor seen from above with a safety yellow stripe along one edge. |
| `dock_rail_250` | 16:9 | Industrial loading dock barrier: a solid blue-grey panel on the lower half and steel wire mesh on the upper half, two posts. 2.5 m wide, 2.9 m tall. Front view. |
| `door_frame_250` | 1:1 | A steel door frame, blue-grey, open in the middle, a blank name plate slot above the opening. 2.5 m wide. Front view. |
| `flap_door` | 1:1 | A doorway filled with two hanging rubber flap panels, dark grey with a safety yellow stripe at the bottom. Front view. |
| `tread` | 16:9 | One steel grating stair tread, blue-grey, anti-slip ridges as flat lines. 2.5 m long, 0.26 m deep. |
| `pen_fence_250` | 16:9 | A chest-high wire mesh pen fence panel between two wooden posts, silver mesh, faded ochre posts. 2.5 m wide, 1.3 m tall. Front view. |
| `roof_500` | 1:1 | A flat square of corrugated metal roof seen from above, blue-grey. |
| `pillar` | 1:1 | A square steel H-beam column, blue-grey, with a bolted base plate. 0.4 m square, 4 m tall. |

---

## 4. 받은 이미지 저장 규칙

- 파일 이름 `<id>_concept_v01.png`. 다시 뽑으면 `v02`. 세 시점은 `<id>_front_v01.png` · `<id>_side_v01.png` · `<id>_back_v01.png`.
- 자리는 `ArtSource/map/henhouse_indoor/<id>/concept/`. 그 옆 `prompt.md`에 실제로 보낸 프롬프트와 ChatGPT가 만든 날짜를 적는다 — 이 표와 달라졌으면 표를 고친다.
- 3D로 보낸 장은 `tripo/` 폴더의 `job.md`에 어느 파일을 썼는지 적는다.
