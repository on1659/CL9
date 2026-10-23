# 제작 1묶음 컨셉 이미지 프롬프트 (2026-10-19)

사용자가 ChatGPT로 그려 주기로 한 다섯 종의 프롬프트다(결정: 돼지·양 재제작 #59, 신규 토끼·쥐·빈 돼지). 승인 기준은 톤 기준 넷(#50): ① 평상시 이미지에서 "귀엽다"나 "웃기다"가 나온다 ② 무서움은 정체가 드러나는 순간에만 ③ 정체는 같은 몸의 뒤틀림으로 읽힌다 ④ 평상 모델에 이음매(실밥) 한 곳이 보인다 — ③④는 탈이 있는 종(돼지)만, 토끼·쥐·양은 ①만 본다.

## 쓰는 법

- 종마다 **평상시** 한 장과(탈이 있는 돼지는) **정체가 드러나는 순간** 한 장을 한 쌍으로 본다. 후보는 종마다 2~3개씩 두었으니 마음에 드는 것만 그려도 된다.
- 모든 프롬프트 끝에 아래 **공통 조건** 문장을 그대로 붙인다(3D 생성 입력으로 바로 쓰기 위한 조건, [tools/tripo.md](C:\Work\CL9\docs\asset\tools\tripo.md)에서 확인된 것).
- 스타일 지시(로우폴리·평면 셰이딩)는 이미지 모델이 반만 듣는다. **형태·비율·색만 맞으면 된다.** 톤 정리는 Blender 재단에서 한다.
- 돌려줄 때: 파일을 `C:\Work\CL9\ArtSource\monster\<종 id>\concept\v1\` 에 `<종 id>_<후보 글자>_<normal|reveal>.png` 로 넣어 주거나, 그냥 채팅에 올려 주면 제가 그 자리에 넣고 카드에 출처·해시를 적는다. 종 id: 토끼 `rabbit_a`, 쥐 `rat_a`, 돼지 `pig_a`(재제작이라 `pig_b`로 새로 둘지는 카드에서), 빈 돼지 `pig_hollow`, 양 `sheep_a`.
- 보고 고를 때 말해 줄 것: 후보 글자 하나(예: "토끼 B"), 고칠 점 한 줄(예: "귀 더 길게"). 승인은 사용자의 말로만 생긴다.

### 공통 조건 (프롬프트 끝에 붙이기)

```
Single object, three-quarter front view, centered, uniform dark gray background, no ground plane, no floor shadow, no text, no watermark. Simple flat colors, few colors, wide color areas, high saturation, low-poly game asset look, readable silhouette.
```

## 1. 돼지 (재제작 — `pig_a`, 탈 있는 집 주인)

규칙 "들고 있는 것을 노린다". 평상시는 먹이통 앞에서 코를 킁킁거리는 순한 돼지, 정체는 입이 귀까지 찢어지며 몸보다 커지는 것. 실루엣은 낮고 긴 분홍 덩어리·짧은 다리·큰 코. **정체 이미지는 5절(빈 돼지) 후보로 그린다** — 아래 reveal 열은 한 쌍의 감만 잡는 참고다. 평상시 돼지의 **실밥 자리는 5절에서 고른 후보와 같아야** 나란히 놓았을 때 한 쌍이 된다(A 입꼬리→귀 뿌리, B 목둘레, C 턱 아래, D 주둥이 둘레). 먼저 5절에서 빈 돼지 후보를 고르고, 그 실밥 자리를 돼지 프롬프트의 [ ] 자리에 넣어 그린다.

| 후보 | 평상시 (normal) | 정체가 드러나는 순간 (reveal) |
|---|---|---|
| A 둥근 인형 | A plump round pink pig plush toy standing on four short stubby legs, oversized flat snout with two big round nostrils, tiny black bead eyes, small floppy ears, curly tail, one thin stitched seam line [실밥 자리는 5절 빈 돼지 후보에 맞춘다 — A: from both mouth corners back to the ear roots / B: around the neck / C: across under the chin / D: a ring around the snout], friendly and slightly dopey expression, chunky cartoon proportions. — 요지: 통통한 분홍 돼지 인형, 큰 납작 코, 등에 실밥 한 줄 | The same plump pink pig plush, but its mouth has split open from ear to ear far wider than its body, a cavernous pitch-black mouth interior with no visible teeth, the stitched seam on its back torn open, small bead eyes unchanged and calm, body otherwise identical, unsettling because only the mouth changed. — 요지: 같은 돼지, 입만 귀까지 찢어져 몸보다 크고 안은 새카맣다 |
| B 납작 코 강조 | A low, long, barrel-shaped pink pig with very short legs, an exaggeratedly large flat snout taking up most of the face, half-closed sleepy eyes, pink body in two flat tones, one thin stitched seam line [실밥 자리는 5절 빈 돼지 후보에 맞춘다 — A: from both mouth corners back to the ear roots / B: around the neck / C: across under the chin / D: a ring around the snout], cute and lazy, cartoon farm animal. — 요지: 코가 얼굴 대부분, 졸린 눈, 배에 실밥 | The same barrel-shaped pig, mouth stretched open impossibly wide below the snout, jaw hanging to the ground, dark hollow interior, seam on the belly burst open with stuffing-like dark fibers, eyes still half-closed and calm. — 요지: 턱이 바닥까지 늘어지고 배 실밥이 터짐 |
| C 작고 통통 | A small chubby piglet-like pig, big head small body, rosy cheeks, round snout, standing in a trough-side pose sniffing the ground, one thin stitched seam line [실밥 자리는 5절 빈 돼지 후보에 맞춘다 — A: from both mouth corners back to the ear roots / B: around the neck / C: across under the chin / D: a ring around the snout], pastel pink and cream, adorable. — 요지: 머리 큰 새끼돼지 비율, 이마에 탈 경계선 | The same chubby pig with its face split along the forehead seam, the lower half of the face opening into a huge gaping mouth wider than the body, interior deep black, upper face with the cute eyes still intact on top like a lid. — 요지: 이마 실밥이 뚜껑처럼 열리고 아래가 거대한 입 |

## 2. 양 (재제작 — `sheep_a`, 탈 없는 떠돌이, 실외)

규칙 "혼자 있지 마라" — 4~6마리가 혼자 남은 사람을 둘러싸고 15초 가둔다. 탈이 없으니 끝까지 귀엽게 크게. 무서움은 모습이 아니라 "무리가 한꺼번에 고개를 드는 0.5초"에서 나온다. 실루엣은 털 구름에 검은 얼굴·짧은 다리, 목에 방울. 실밥은 없다.

| 후보 | 평상시 (normal) | 고개 드는 순간 (참고, 선택) |
|---|---|---|
| A 털 구름 | A fluffy white sheep shaped like a round cloud of wool on four tiny dark legs, small black face with gentle wide-set eyes, little dark ears, a small brass bell on a red collar around its neck, grazing pose with head slightly down, soft cream wool in flat tones, extremely cute and harmless. — 요지: 구름 같은 흰 털뭉치, 검은 얼굴, 빨간 목줄에 방울 | The same cloud-like sheep standing perfectly still with its head raised straight up and turned toward the viewer, eyes wide and unblinking, bell hanging motionless, the body unchanged. — 요지: 같은 양이 고개를 번쩍 들고 이쪽을 본다 |
| B 큰 얼굴 | A cute cartoon sheep with an oversized round dark face taking up a third of the body, big shiny eyes, a huge puffy wool body, stubby legs, a bell on its neck, chunky toy-like proportions, white and charcoal flat colors. — 요지: 얼굴을 1.2~1.35배 키운 인형 비율(sheep_a_v2 방향) | The same big-faced sheep, head lifted high, mouth slightly open, eyes fixed forward, standing rigid. — 요지: 고개 든 채 굳음 |
| C 둥근 돌멩이 | A sheep that looks like a smooth rounded boulder of wool with a tiny dark face poking out at the front, almost no neck, four short legs, a small bell, flat pastel colors, chunky and comical. — 요지: 목 없는 돌멩이 같은 덩어리, 얼굴만 빼꼼 | The same boulder-sheep tilting the whole body back to raise its face, stiff pose, eyes wide. — 요지: 몸째 기울여 고개 듦 |

## 3. 토끼 (신규 — `rabbit_a`, 탈 없는 새 유형, 실내)

### 카드의 프롬프트 절 (꼬리말은 본문에 풀어 넣음)

후보 넷 × (평상시 · 무서운 순간) 한 쌍씩이다. 프롬프트는 이미지 모델용 영어이고, 아래에 한국어 요지 한 줄을 붙였다. 이미지는 아직 만들지 않았다.

**토끼는 탈이 없는 종이라 "정체가 드러나는 순간"이 따로 없다.** 톤 문장 5("탈이 없는 몬스터는 처음부터 끝까지 귀엽게 크게 만든다 — 무서움은 모습이 아니라 예고와 규칙에서 나온다", story-draft 6.1)를 따라, 무서운 쪽 이미지는 **같은 모델·같은 색**에서 자세와 눈만 바꾼 장면(뒷발 구르기 웅크림, 멈칫 응시)으로 만든다. 새 부위·이빨·상처·실밥은 넣지 않는다.

톤 기준 넷(결정 #50)을 이 종에 적용하는 법은 다음과 같다. ① 평시 귀여움 — 평상시 이미지에서 "귀엽다"나 "웃기다"가 나와야 한다. ② 무서움은 그 순간에만 — 무서운 기운은 무서운 쪽 이미지의 자세와 눈에만 있다. ③ 같은 몸 — 탈이 없으므로 "뒤틀림" 대신 같은 몸이 같은 동작(구르기·들이받기)을 위협적으로 하는 모습으로 읽는다. ④ 이음매 한 곳 — 해당 없다(탈 없는 종, 통합 4 2절 25번).

**공통 꼬리말** — 모든 프롬프트 끝의 `stylized low-poly game creature, chunky rounded shapes, flat-shaded large color areas, limited palette of three or four colors, one saturated accent color, cute and comedic, no realistic fur strands, no blood, no gore, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text` 자리에 이 문장을 그대로 붙인다:

```text
stylized low-poly game creature, chunky rounded shapes, flat-shaded large color areas, limited palette of three or four colors, one saturated accent color, cute and comedic, no realistic fur strands, no blood, no gore, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

#### 후보 A — 솜뭉치

평상시:

```text
A giant timid rabbit as tall as a person's waist, its body almost a perfect round ball of pure white, two huge upright ears about half the body height with bright hot-pink insides, a small hot-pink nose, very large glossy black eyes that look shy and nervous, oversized flat hind feet tucked under the round body, a tiny white cotton tail, sitting calmly with its nose twitching, stylized low-poly game creature, chunky rounded shapes, flat-shaded large color areas, limited palette of three or four colors, one saturated accent color, cute and comedic, no realistic fur strands, no blood, no gore, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

요지: 흰 공 같은 몸에 몸 높이 절반의 큰 귀(안쪽 진분홍)와 겁먹은 큰 눈을 한, 사람 허리 높이의 토끼가 얌전히 앉아 있다.

무서운 순간:

```text
The same giant white rabbit with exactly the same body, colors and proportions, now pressed low and flat to the ground like a coiled spring, both huge ears pinned straight back along its spine, one oversized hind foot raised high mid-stomp about to slam the floor, eyes wide and staring straight ahead with tiny pinpoint pupils, head lowered ready to headbutt, threatening pose only, no teeth, no wounds, no seams, stylized low-poly game creature, chunky rounded shapes, flat-shaded large color areas, limited palette of three or four colors, one saturated accent color, cute and comedic, no realistic fur strands, no blood, no gore, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

요지: 같은 흰 토끼가 몸을 납작하게 깔고 귀를 등에 붙인 채 뒷발 하나를 높이 들어 내려치려 하고, 작아진 눈동자로 정면을 노려본다.

톤 기준: ① 공 모양 몸과 큰 눈이 귀엽다. ② 무서움은 납작한 자세와 작은 눈동자에만 있다. ③ 같은 모델·같은 색이고 새 부위가 없으며, 뒷발 구르기(Thump)를 그대로 위협으로 쓴다. ④ 해당 없음.

#### 후보 B — 흰 장화

평상시:

```text
A giant gentle rabbit as tall as a person's waist, a soft pear-shaped body in dusty blue-gray, a bright white belly and comically huge bright white hind feet that look like big boots, tall stiff ears with saturated coral-pink insides, a round white muzzle, big dark worried eyes, standing on all fours and sniffing the air, stylized low-poly game creature, chunky rounded shapes, flat-shaded large color areas, limited palette of three or four colors, one saturated accent color, cute and comedic, no realistic fur strands, no blood, no gore, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

요지: 푸른빛 회색의 배 모양 몸에 흰 배와 장화처럼 큰 흰 뒷발, 산호색 귀 안쪽을 한 순한 큰 토끼가 네 발로 서서 냄새를 맡는다.

무서운 순간:

```text
The same blue-gray rabbit with the same white boots and the same proportions, crouched very low with its huge white hind boots planted wide behind it, both tall ears swung forward to point at the viewer like two horns, head tucked down for a charge, front paws pulled in, eyes narrowed into a hard stare, the whole body tense and leaning forward, threatening pose only, no teeth, no wounds, no seams, stylized low-poly game creature, chunky rounded shapes, flat-shaded large color areas, limited palette of three or four colors, one saturated accent color, cute and comedic, no realistic fur strands, no blood, no gore, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

요지: 같은 토끼가 흰 장화 같은 뒷발을 넓게 버티고 웅크린 채 두 귀를 뿔처럼 앞으로 겨누고, 머리를 숙여 들이받을 준비를 한다.

톤 기준: ① 큰 흰 장화 발이 우스꽝스럽다. ② 무서움은 앞으로 겨눈 귀와 굳은 눈에만 있다. ③ 같은 몸이 들이받기(Headbutt) 직전의 자세를 취한다. ④ 해당 없음.

#### 후보 C — 짝귀

평상시:

```text
A giant clumsy rabbit as tall as a person's waist, a round cream-white body with one large charcoal-gray patch on its back like a saddle, one ear standing straight up and the other ear flopped over one eye in a silly way, a hot-pink nose, round black eyes, chubby cheeks, sitting with a small tuft of grass in its mouth, stylized low-poly game creature, chunky rounded shapes, flat-shaded large color areas, limited palette of three or four colors, one saturated accent color, cute and comedic, no realistic fur strands, no blood, no gore, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

요지: 등에 짙은 회색 안장 무늬가 있는 크림색 둥근 토끼가 한쪽 귀는 세우고 한쪽 귀는 눈 위로 접은 채 풀을 물고 있다.

무서운 순간:

```text
The same cream-white rabbit with the same charcoal saddle patch, startled: both ears snapped bolt upright and rigid at the same height, the flopped ear now perfectly straight, the body frozen mid-hop with front paws lifted off the ground, staring directly at the viewer with wide unblinking eyes, a sudden total stillness, threatening pose only, no teeth, no wounds, no seams, stylized low-poly game creature, chunky rounded shapes, flat-shaded large color areas, limited palette of three or four colors, one saturated accent color, cute and comedic, no realistic fur strands, no blood, no gore, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

요지: 같은 토끼의 두 귀가 접혀 있던 쪽까지 꼿꼿이 서고, 앞발을 든 채 뛰다 멈춰 눈을 깜빡이지 않고 정면을 본다.

톤 기준: ① 짝귀가 우스꽝스럽다. ② 무서움은 두 귀가 같이 서는 순간과 멈춘 응시에만 있다. ③ 같은 몸이 Ears Up Startle·Hop Pause를 하고, 접힌 귀가 펴지는 변화가 실루엣으로 읽힌다. ④ 해당 없음.

#### 후보 D — 큰 발

평상시:

```text
A giant shy rabbit as tall as a person's waist, a bottom-heavy round body in light gray with darker gray ear tips and darker gray feet, enormous long hind feet longer than its head, ears with saturated pink insides, a small white muzzle, sleepy half-closed bashful eyes, lying loaf-shaped with its big feet sticking out in front, stylized low-poly game creature, chunky rounded shapes, flat-shaded large color areas, limited palette of three or four colors, one saturated accent color, cute and comedic, no realistic fur strands, no blood, no gore, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

요지: 밝은 회색에 귀 끝과 발이 짙은 회색인, 아래가 무거운 토끼가 머리보다 긴 뒷발을 앞으로 내놓고 식빵처럼 엎드려 졸린 눈을 하고 있다.

무서운 순간:

```text
The same light gray rabbit with the same dark gray feet and ear tips, body arched low and long, ears flat back, both enormous hind feet lifted together and frozen just before slamming the floor, eyes fully open and fixed on the viewer, a heavy stomping pose that suggests a loud thump, threatening pose only, no teeth, no wounds, no seams, stylized low-poly game creature, chunky rounded shapes, flat-shaded large color areas, limited palette of three or four colors, one saturated accent color, cute and comedic, no realistic fur strands, no blood, no gore, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

요지: 같은 회색 토끼가 몸을 길고 낮게 휘고 두 큰 뒷발을 함께 들어 바닥을 내려치기 직전에 멈춘 채, 크게 뜬 눈으로 정면을 본다.

톤 기준: ① 식빵 자세와 졸린 눈, 큰 발이 귀엽다. ② 무서움은 두 발 구르기 직전의 자세에만 있다. ③ 같은 몸이 뒷발 구르기를 과장하고, 쿵쿵 소리 예고(`ThumpSoundRadius`)와 그림이 맞는다. ④ 해당 없음.

#### 이미지 입력 조건 ([tools/tripo.md](C:\Work\CL9\docs\asset\tools\tripo.md) "입력 이미지 조건" 원문)

단일 오브젝트, 3/4 정면, 화면 중앙, 균일한 짙은 회색 배경, 바닥면 없음, 바닥 그림자 없음, 글자 없음. 이 조건을 프롬프트 끝에 그대로 적으면 Nano Banana가 지켰다.
스타일 지시(로우폴리·평면 셰이딩)는 이미지 모델이 반만 듣는다 — 형태와 색만 맞으면 된다. 톤은 Blender 재단이 맡는다.

## 4. 쥐 (신규 — `rat_a`, 탈 없는 떠돌이, 떼)

### 카드의 프롬프트 절 (꼬리말은 본문에 풀어 넣음)

후보 셋 × (평상시 · 떼가 쏟아지는 순간) 한 쌍씩이다. 프롬프트는 이미지 모델용 영어이고, 아래에 한국어 요지 한 줄을 붙였다. 이미지는 아직 만들지 않았다.

**쥐는 탈이 없는 종이라 "정체"가 없다.** 톤 문장 5를 따라, 무서운 쪽 이미지는 같은 개체가 **여럿이 한꺼번에 쏟아지는 장면**으로 만든다 — 무서움은 모습이 아니라 수와 기세에서 나온다. 한 마리의 생김새는 평상시와 같아야 하고, 이빨·핏발·상처를 더하지 않는다. 떼 이미지는 여러 오브젝트라 **3D 입력 조건을 지킬 수 없으므로 컨셉 검토 전용**이다. 3D 입력에는 평상시 한 마리 이미지만 쓴다.

톤 기준 넷(결정 #50)을 이 종에 적용하는 법은 다음과 같다. ① 평시 귀여움 — 한 마리가 생쥐 인형처럼 귀엽다. ② 무서움은 그 순간에만 — 떼가 쏟아지는 장면에만 기세가 있다. ③ 같은 몸 — 떼 속 개체가 평상시 한 마리와 똑같이 생겼다(뒤틀림 대신 "수"). ④ 이음매 한 곳 — 해당 없다(탈 없는 종). 인형처럼 그리되 바느질 자국이 생기지 않게 프롬프트에 `no stitches, no seams`를 넣는다.

**공통 꼬리말 — 평상시(3D 입력용)** — `stylized low-poly game creature, chunky rounded shapes, flat-shaded large color areas, limited palette of three colors, one saturated accent color, cute toy-like look with no stitches and no seams, no realistic fur strands, no blood, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text` 자리에 붙인다:

```text
stylized low-poly game creature, chunky rounded shapes, flat-shaded large color areas, limited palette of three colors, one saturated accent color, cute toy-like look with no stitches and no seams, no realistic fur strands, no blood, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

**공통 꼬리말 — 떼(컨셉 검토 전용)** — `stylized low-poly game creatures, chunky rounded shapes, flat-shaded large color areas, limited palette, every mouse identical to the others, no stitches, no seams, no blood, no gore, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text` 자리에 붙인다:

```text
stylized low-poly game creatures, chunky rounded shapes, flat-shaded large color areas, limited palette, every mouse identical to the others, no stitches, no seams, no blood, no gore, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

#### 후보 A — 펠트 공

평상시:

```text
A tiny toy-like mouse about 20 cm long, a round soft gray body shaped like a ball, enormous round ears as big as its head with bright hot-pink insides, a long thin hot-pink tail twice its body length curling into a little spiral at the tip, tiny hot-pink feet and nose, big shiny black eyes, standing on all fours with a cheerful curious look, stylized low-poly game creature, chunky rounded shapes, flat-shaded large color areas, limited palette of three colors, one saturated accent color, cute toy-like look with no stitches and no seams, no realistic fur strands, no blood, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

요지: 공처럼 둥근 회색 몸에 머리만 한 둥근 귀(안쪽 진분홍)와 끝이 말린 긴 분홍 꼬리를 한 20cm 생쥐 인형이 호기심 어린 얼굴로 서 있다.

떼가 쏟아지는 순간:

```text
A dense rushing stream of dozens of the same tiny round gray mice with huge pink-lined ears and long pink tails, pouring forward in one single wave toward the viewer, packed shoulder to shoulder, the rows of big round ears forming a rippling surface like water, pink tails whipping above the flow, all the eyes catching the light as tiny white highlights, overwhelming momentum, stylized low-poly game creatures, chunky rounded shapes, flat-shaded large color areas, limited palette, every mouse identical to the others, no stitches, no seams, no blood, no gore, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

요지: 같은 둥근 회색 생쥐 수십 마리가 한 물결로 쏟아지고, 줄지은 둥근 귀가 물결 면처럼 일렁이며 분홍 꼬리가 그 위로 휘날린다.

톤 기준: ① 공 같은 몸과 큰 귀가 귀엽다. ② 무서움은 수십 마리가 한꺼번에 몰려오는 장면에만 있다. ③ 떼 속 한 마리가 평상시와 같다. ④ 해당 없음.

#### 후보 B — 물방울

평상시:

```text
A tiny toy-like mouse about 20 cm long, a teardrop-shaped body in slate blue-gray with a pale gray belly, two very large ears shaped like round satellite dishes with saturated magenta insides, a long thin magenta tail, a pointed little magenta nose, round black eyes, sitting up on its hind legs with its front paws held together, stylized low-poly game creature, chunky rounded shapes, flat-shaded large color areas, limited palette of three colors, one saturated accent color, cute toy-like look with no stitches and no seams, no realistic fur strands, no blood, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

요지: 물방울 모양의 푸른 회색 몸에 접시처럼 큰 귀(안쪽 자홍)와 긴 자홍 꼬리를 한 생쥐가 앞발을 모으고 뒷발로 앉아 있다.

떼가 쏟아지는 순간:

```text
A tall cresting wave made of dozens of the same slate blue-gray mice with dish-shaped magenta-lined ears, surging toward the viewer, the front row rearing up with ears flared wide, the mice behind climbing over each other so the wave rises higher than a person's knee, thin magenta tails trailing behind like spray, stylized low-poly game creatures, chunky rounded shapes, flat-shaded large color areas, limited palette, every mouse identical to the others, no stitches, no seams, no blood, no gore, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

요지: 같은 푸른 회색 생쥐 수십 마리가 서로 타고 넘어 무릎 높이의 파도처럼 솟구치고, 앞줄이 귀를 활짝 편 채 일어선다.

톤 기준: ① 앞발을 모은 앉은 자세와 접시 귀가 귀엽다. ② 무서움은 떼가 파도처럼 솟는 장면에만 있다. ③ 떼 속 한 마리가 평상시와 같다. ④ 해당 없음.

#### 후보 C — 고리 꼬리

평상시:

```text
A tiny toy-like mouse about 20 cm long, a plump warm gray body with a white belly, large round ears with white rims and saturated pink insides, a long tail with three bold white rings that is easy to read from far away, a pink nose, round black eyes, mid-step with one front paw lifted, stylized low-poly game creature, chunky rounded shapes, flat-shaded large color areas, limited palette of three colors, one saturated accent color, cute toy-like look with no stitches and no seams, no realistic fur strands, no blood, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

요지: 따뜻한 회색에 흰 배, 흰 테두리 큰 귀, 흰 고리 셋이 또렷한 긴 꼬리를 한 통통한 생쥐가 앞발 하나를 든 채 걷는다.

떼가 쏟아지는 순간:

```text
Dozens of the same plump warm gray mice with white-ringed tails bursting out of a small dark round arch-shaped hole outline and fanning out across the frame, a narrow jet of mice at the hole spreading into a wide river, the white tail rings flickering everywhere and making the whole flow read as fast motion, stylized low-poly game creatures, chunky rounded shapes, flat-shaded large color areas, limited palette, every mouse identical to the others, no stitches, no seams, no blood, no gore, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

요지: 같은 생쥐 수십 마리가 작은 둥근 아치 구멍에서 가늘게 분출해 넓은 강처럼 부채꼴로 퍼지고, 흰 꼬리 고리가 사방에서 깜빡여 빠르기가 읽힌다.

톤 기준: ① 통통한 몸과 고리 꼬리가 귀엽다. ② 무서움은 구멍에서 쏟아지는 장면에만 있다. ③ 떼 속 한 마리가 평상시와 같고, 구멍에서 좁게 나와 퍼지는 모양이 실제 구멍(0~40cm)과 떼 폭(100cm)의 관계를 미리 보여 준다. ④ 해당 없음.

#### 이미지 입력 조건 ([tools/tripo.md](C:\Work\CL9\docs\asset\tools\tripo.md) "입력 이미지 조건" 원문)

단일 오브젝트, 3/4 정면, 화면 중앙, 균일한 짙은 회색 배경, 바닥면 없음, 바닥 그림자 없음, 글자 없음. 이 조건을 프롬프트 끝에 그대로 적으면 Nano Banana가 지켰다.
스타일 지시(로우폴리·평면 셰이딩)는 이미지 모델이 반만 듣는다 — 형태와 색만 맞으면 된다. 톤은 Blender 재단이 맡는다.

## 5. 빈 돼지 (신규 — `pig_hollow`, 돼지의 탈 터진 본체)

### 카드의 프롬프트 절 (꼬리말은 본문에 풀어 넣음)

빈 돼지는 **"정체" 쪽 이미지만** 만든다. 평상시 쪽은 돼지 본체의 몫이고, 승인 때는 정체 이미지를 평상시 돼지와 나란히 놓고 본다. 후보는 넷이고, 프롬프트는 이미지 모델용 영어, 아래에 한국어 요지 한 줄을 붙였다. 이미지는 아직 만들지 않았다.

**나란히 볼 평상시 참고:** 지금은 `C:\Work\CL9\ArtSource\monster\pig_a\preview\neutral.png`(실물)과 `C:\Work\CL9\ArtSource\monster\pig_a_v2\preview\v2_neutral.png`(귀여움 1단계, 승인 대기)다. `pig_a`가 재제작되므로, 새 돼지 컨셉이 나오면 그것과 다시 나란히 본다. 각 후보의 "평상 모델 이음매" 자리는 새 돼지 컨셉에 그대로 넣어야 한 쌍이 맞는다.

톤 기준 넷(결정 #50)은 다음과 같이 본다. ① 평시 귀여움 — 평상시 돼지 이미지가 맡는다(이 카드 밖). ② 무서움은 정체 순간에만 — 이 이미지에만 무서움이 있다. 피·살·이빨 줄 없이 입 안의 깊은 어둠과 비율이 틀어진 몸으로 무섭게 한다. ③ 같은 몸의 뒤틀림 — "먹는 입이 몸보다 커진다"(story-draft 6.3). 몸·다리·귀·꼬리·분홍은 평상시와 같아야 하고, 나란히 놓았을 때 "아, 저게 그거였구나"가 나와야 한다. ④ 평상 모델에 이음매 한 곳 — 후보마다 자리를 적었다.

**공통 꼬리말** — `stylized low-poly game monster, chunky rounded shapes, flat-shaded large color areas, limited palette of saturated bubblegum pink plus near-black for the mouth interior, no realistic flesh, no blood, no gore, no rows of teeth, no tongue detail, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text` 자리에 붙인다:

```text
stylized low-poly game monster, chunky rounded shapes, flat-shaded large color areas, limited palette of saturated bubblegum pink plus near-black for the mouth interior, no realistic flesh, no blood, no gore, no rows of teeth, no tongue detail, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

#### 후보 A — 귀까지 찢어진 입

```text
The same chubby saturated-pink low-poly pig with the same long low body, short stubby legs, big round snout, floppy folded ears and curly tail, but its mouth corners have torn open along a stitched seam line all the way back to the roots of its ears, the whole upper head tilted back so the open mouth seen from the front is wider and taller than the entire body, the inside of the mouth is a deep pitch-black hollow with no visible bottom, a few loose pink thread ends hanging at the torn corners, the body skin slightly slack as if empty inside, stylized low-poly game monster, chunky rounded shapes, flat-shaded large color areas, limited palette of saturated bubblegum pink plus near-black for the mouth interior, no realistic flesh, no blood, no gore, no rows of teeth, no tongue detail, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

요지: 같은 분홍 돼지의 입꼬리가 실밥을 따라 귀 뿌리까지 찢어지고, 머리가 뒤로 젖혀져 정면에서 입이 몸 전체보다 크며, 입 안은 바닥이 안 보이는 검은 구멍이다.

톤 기준: ② 무서움은 바닥 없는 입 안과 몸보다 큰 입에만 있다. ③ 먹는 입이 그대로 커졌고 나머지는 같은 돼지다. ④ **평상 모델 이음매: 양쪽 입꼬리에서 귀 뿌리까지 이어지는 가는 실밥 한 줄**(주인 6.3 조짐 3단계 "입꼬리가 귀까지 찢어져 보인다"와 같은 자리).

#### 후보 B — 자루 입

```text
The same chubby saturated-pink low-poly pig with the same short legs, hind body and curly tail, but the whole front half of its head has opened like the mouth of a drawstring sack: the snout and face are folded back over its shoulders as one loose pink flap, revealing a huge round opening larger than the pig's body, the opening is a deep dark tunnel going far back inside the empty pink skin, loose pink thread loops along the rim of the opening, stylized low-poly game monster, chunky rounded shapes, flat-shaded large color areas, limited palette of saturated bubblegum pink plus near-black for the mouth interior, no realistic flesh, no blood, no gore, no rows of teeth, no tongue detail, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

요지: 같은 분홍 돼지의 머리 앞쪽 전체가 자루 주둥이처럼 열려 얼굴이 어깨 위로 젖혀지고, 몸보다 큰 둥근 구멍이 빈 가죽 깊숙이 어두운 굴처럼 이어진다.

톤 기준: ② 무서움은 빈 가죽 속으로 이어지는 굴에만 있다. ③ 몸·다리·꼬리는 같은 돼지이고 머리가 입으로 뒤집혔다 — 다만 얼굴이 뒤로 접혀 "그 돼지"로 읽히는 정도가 넷 가운데 가장 약할 수 있다(추정). ④ **평상 모델 이음매: 목둘레를 한 바퀴 도는 실밥**(주인 6.3 조짐 3단계 "목의 실밥이 벌어지고"와 같은 자리).

#### 후보 C — 먹이통 턱

```text
The same chubby saturated-pink low-poly pig with the same long low body, short legs, floppy ears and curly tail, but its lower jaw has unhinged and dropped all the way down to the floor like a wide scoop or a feeding trough, the jaw sliding forward along the ground ahead of the body, the open mouth taller than the pig itself, the inside a deep dark hollow, the big round snout still on top, the torn seams under the chin showing loose pink threads, stylized low-poly game monster, chunky rounded shapes, flat-shaded large color areas, limited palette of saturated bubblegum pink plus near-black for the mouth interior, no realistic flesh, no blood, no gore, no rows of teeth, no tongue detail, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

요지: 같은 분홍 돼지의 아래턱이 빠져 먹이통 같은 넓은 삽처럼 바닥까지 내려가 앞으로 끌리고, 입이 돼지 키보다 높으며 안은 깊고 어둡다.

톤 기준: ② 무서움은 바닥을 쓸며 다가오는 턱과 어두운 입 안에만 있다. ③ 먹던 입이 먹이통이 됐다 — 돼지 농장의 먹이통(`Trough`)과 그림이 이어진다. 큰 코가 위에 남아 "그 돼지"로 읽힌다. ④ **평상 모델 이음매: 턱 아래를 가로지르는 실밥**.

#### 후보 D — 풍선 입

```text
The same chubby saturated-pink low-poly pig, but its mouth has ballooned outward into a huge rounded pink dome-shaped maw much bigger than the rest of the body, the rim of the mouth stretched thin and glossy, the pig's normal small body, short stubby legs, floppy ears and curly tail dangling comically under and behind the giant mouth, the inside of the dome a deep dark hole, a ring of stitch marks stretched wide around the mouth rim, stylized low-poly game monster, chunky rounded shapes, flat-shaded large color areas, limited palette of saturated bubblegum pink plus near-black for the mouth interior, no realistic flesh, no blood, no gore, no rows of teeth, no tongue detail, single object, three-quarter front view, centered in frame, uniform dark gray background, no floor plane, no ground shadow, no text
```

요지: 같은 분홍 돼지의 입이 몸보다 훨씬 큰 둥근 풍선처럼 부풀고, 원래의 작은 몸·짧은 다리·귀·꼬리가 그 아래·뒤에 매달려 있으며, 둥근 입 안은 깊고 어둡다.

톤 기준: ② 무서움은 몸보다 큰 입과 그 속 어둠에 있지만, 작은 몸이 매달린 모양은 우스운 쪽으로 기울 수 있다(추정 — 넷 가운데 가장 덜 무서울 수 있다). ③ 입만 커지고 몸은 그대로라 같은 돼지로 가장 잘 읽힌다. ④ **평상 모델 이음매: 주둥이(코와 입) 둘레를 도는 실밥 한 바퀴**.

#### 이미지 입력 조건 ([tools/tripo.md](C:\Work\CL9\docs\asset\tools\tripo.md) "입력 이미지 조건" 원문)

단일 오브젝트, 3/4 정면, 화면 중앙, 균일한 짙은 회색 배경, 바닥면 없음, 바닥 그림자 없음, 글자 없음. 이 조건을 프롬프트 끝에 그대로 적으면 Nano Banana가 지켰다.
스타일 지시(로우폴리·평면 셰이딩)는 이미지 모델이 반만 듣는다 — 형태와 색만 맞으면 된다. 톤은 Blender 재단이 맡는다.
