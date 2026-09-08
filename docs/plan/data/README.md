# 세부 기획 — 데이터 표

수치와 종류 정의는 전부 여기 있다. **에디터에서 바꾸고 빌드 없이 확인한다.**

| 표 | 무엇 |
|---|---|
| [rules.md](rules.md) | 게임 전체 규칙 — 이동 · 스태미나 · 소리 · 죽음 · 밤 |
| [item/](item/common.md) | 아이템 — 공통 컬럼([common](item/common.md)) + [계란](item/egg.md)·[고기](item/meat.md)·[수레](item/cart.md)·[손전등](item/flashlight.md)·[헤드램프](item/headlamp.md) |
| [monster/](monster/common.md) | 몬스터 — 공통 컬럼([common](monster/common.md)) + [닭](monster/chicken.md)·[거위](monster/goose.md)·[너구리](monster/raccoon.md)·[돼지](monster/pig.md)·[양](monster/sheep.md) |
| [character.md](character.md) | 플레이어 외형 |
| [site.md](site.md) | 목적지 — 양계장 · (나중에) 벌집 |
| [order.md](order.md) | 주문서 (레시피 쿼터) |
| [equipment.md](equipment.md) | 살 수 있는 장비 |
| [progression.md](progression.md) | 일자 진행 곡선 |
| [buff.md](buff.md) | 먹어서 걸리는 상태 변화 — 아이템 행이 별칭으로 가리킨다 |
| [segkind.md](segkind.md) | 맵 조각 11종 |
| [module.md](module.md) | 맵 모듈 — 생성기가 조립하는 단위 |
| [sequence.md](sequence.md) | 연출 시퀀스 — 전이 지점(밤 시작·정전·전멸·새벽). **표는 아직 없다** |

---

## 공통 뼈대

종류를 정의하는 표는 전부 이 부모를 상속한다.

```cpp
UCL9DefBase : UPrimaryDataAsset
```

| 컬럼 | 타입 | 뜻 |
|---|---|---|
| `Alias` | `FName` | 식별자. `egg` · `chicken` · `henhouse` |
| `SubAlias` | `FName` | 하위 분류 (선택). `gold` · `cooked` |
| `DisplayName` | `FText` | 화면 표시명 (현지화 대상) |
| `Description` | `FText` | 상세 설명 (선택) |
| `Icon` | `TSoftObjectPtr<UTexture2D>` | UI 2D 이미지 |
| `Mesh` | `TSoftObjectPtr<UStaticMesh>` | 월드 메시 |
| `ActorClass` | `TSoftClassPtr<AActor>` | 전용 BP. **비우면 종류별 공용 BP를 쓴다** |
| `Gimmicks` | `TArray<TObjectPtr<UCL9Gimmick>>` `Instanced` | 이 종류에만 있는 특수 동작 |

### 규칙 넷

**1. `Alias` + `SubAlias`가 조회 키다.** 파일 이름이 아니다. 에셋을 옮기거나 이름을 바꿔도
데이터가 안 깨진다.

**2. `ActorClass`는 비워두는 게 기본이다.** 계란이 100종이어도 BP는 하나면 된다. 메시와 수치가
다를 뿐 동작이 같으면 전용 BP가 필요 없다.

**3. `Mesh`·`Icon`은 소프트 참조다.** 하드 참조로 두면 표를 여는 순간 모든 메시가 메모리에
올라온다.

**4. 기믹은 판정과 연출을 구분한다.** 호스트가 판정하는 구조라 기믹이 액터에서 직접 상태를
바꾸면 클라와 갈라진다. **연출 기믹은 컴포넌트로, 판정 기믹은 표의 값으로** 두고 호스트
판정 코드가 읽는다.

---

## 왜 DataTable이 아니라 DataAsset인가

기믹을 유니티 컴포넌트처럼 붙이려면 `Instanced` 프로퍼티가 필요한데, **이게 DataTable 행에서는
동작하지 않는다.** 행은 구조체(`FTableRowBase`)이고, 인스턴스로 만들어진 오브젝트는 자기를
소유할 UObject가 있어야 하는데 구조체는 그 역할을 못 한다.

그래서 행은 구조체가 아니라 **UObject**다. 배치는 2026-09-02에 엑셀형으로 확정했다
(decisions.md): **종류별 테이블 에셋 하나(`UCL9DefTable`)가 인라인 행들을 품는다** —
`DA_ItemTable`을 열면 전 아이템 행이 한 화면에 보이고, 행 추가는 Rows 배열의 `+`다.
행이 UObject라 Instanced 기믹은 그대로 동작한다. 독립 에셋 행도 여전히 읽힌다
(단일 규칙 에셋 등).

곡선이 필요한 값(일자별 배율)만 `UCurveFloat` 에셋을 쓴다 — 에디터에서 그래프로 만진다.

---

## 조회

```cpp
UCL9DefRegistry             // 게임 인스턴스가 들고 있는다
  FindItem(Alias, SubAlias)      → UCL9ItemDef*
  FindMonster(Alias, SubAlias)   → UCL9MonsterDef*
  FindSite / FindOrder / FindEquipment / FindCharacter / FindSequence
```

에셋 목록은 전용 목록 에셋에 등록한다. **코드에 에셋 경로를 쓰지 않는다** — 하드코딩된 경로가
아니라 등록된 목록을 순회한다.

### 읽는 경로

정적 표는 **각 머신이 로컬 레지스트리로 읽는다** — 모두 같은 에셋을 갖고 있고,
`CL9TuningHash`가 접속 시 일치를 검증한다. GameState는 런타임 진행 상태만 나른다
([../../conventions.md](../../conventions.md) § 5).

---

## 값이 서로 지켜야 하는 관계

수치가 에셋으로 나오면서 컴파일 타임 `static_assert`가 못 돈다. 대신 **로드할 때 같은 검사를
코드로 돌리고, 어긋나면 어느 줄이 깨졌는지 로그에 찍는다.**

검사하는 부등식 목록은 [../economy.md](../economy.md).

멀티는 `CL9TuningHash`가 접속할 때 호스트와 클라의 값을 대조한다. 표를 고친 사람은 접속이 막힌다.

---

## 단위

**`u`가 정본이고 `cm = u × 3`이다.** 시간과 비율에는 배율을 걸지 않는다.

| | u | cm |
|---|---|---|
| 플레이어 반경 | 13 | 39 |
| 눈높이 | 52 | 156 |
| 걷기 속도 | 132 | 396 |
| 달리기 속도 | 218 | 654 |
| 통로 폭 | 85 | 255 |
| 존 이격 (참고 — 맵 세팅의 몫) | 350 | 1050 |

표에는 **cm로 적는다** — 엔진이 쓰는 단위다.
