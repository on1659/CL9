# 수레 — `cart`

`CartOnly` 아이템 운반을 여는 물건. **장비로 사지만, 물건으로서는 여느 아이템과 같은 규칙을 탄다.**

컬럼 정의는 [common.md](common.md). **상점 행(가격·해금)은 [../equipment.md](../equipment.md)** —
`ItemDef` 컬럼이 그 장비를 이 문서의 아이템으로 연결한다.

---

## 운반

- 끌면 캐릭터 속도에 `CartMult` 0.75가 곱해진다 ([../character.md](../character.md))
- `CartOnly` 아이템(수레로만 옮길 수 있는 것)을 실을 수 있다
- 수레 자체는 품에 안 들어간다 — 끌거나 두거나

## 원정과 분실

- 원정에 끌고 갈 수 있고, **두고 오면 잃는다** — 밤이 끝나면 맵과 함께 사라진다
  ([../equipment.md](../equipment.md)). 다시 사는 값이 그 대가다.

## 미결

- 가격 ([../equipment.md](../equipment.md) 미결과 동일)
- 적재 규칙 (몇 개까지, 쏟아지는가) — ② 2f에서 정한다
- `CartOnly` 아이템 목록 — 아직 없다 (큰 재료가 생겨야)
