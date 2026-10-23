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
