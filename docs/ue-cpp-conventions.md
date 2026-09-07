# UE C++ 코딩 규칙

CLAUDE.md용 — Unreal Engine C++ 프로젝트에 적용.
전문(에픽 공식 한글판)은 `C:\Users\user\rules\reference\coding-standard-ko.md`를 참조해라.

---

## 클래스 체계

- 클래스는 작성자가 아니라 읽는 사람 기준으로 구성해라. 퍼블릭 구현을 먼저 선언하고 프라이빗 구현을 뒤에 둬라.

## 저작권 고지

- 공개 배포용 소스 파일(`.h`, `.cpp`, `.xaml`)은 첫 줄에 `// Copyright Epic Games, Inc. All Rights Reserved.`를 이 형식 그대로 넣어라. 형식이 어긋나면 CIS가 실패한다.

## 명명 규칙

- PascalCase를 사용해라. 단어 사이 언더스코어 없음. 코드(식별자)는 미국 영어 철자법·문법을 따라라.
  **코멘트는 예외 — 이 프로젝트는 한글로 쓴다** (2026-09-06 결정, decisions.md).
- 타입명에 아래 접두사를 붙여라. 언리얼 헤더 툴이 요구하므로 빠뜨리지 마라.

  | 접두사 | 대상 |
  |--------|------|
  | `T` | 템플릿 클래스 |
  | `U` | `UObject` 파생 클래스 |
  | `A` | `AActor` 파생 클래스 |
  | `S` | `SWidget` 파생 클래스 |
  | `I` | 추상적 인터페이스인 클래스 |
  | `C` | 컨셉(concept) 역할의 구조체 타입 |
  | `E` | 열거형 |
  | `F` | 그 외 대부분의 클래스 |
  | `b` | bool 변수 (소문자 b) |

- typedef에도 원본 타입에 맞는 접두사를 붙여라 (구조체 → `F`, `UObject` → `U`). 특정 템플릿 인스턴스화의 typedef는 더 이상 템플릿이 아니므로 그에 맞는 접두사를 붙여라: `typedef TArray<FMytype> FArrayOfMyTypes;`
- C#에서는 접두사를 생략해라. 타입 템플릿 파라미터와 그에 기반한 중첩 타입 에일리어스는 접두사 규칙에서 제외한다.
- 템플릿 파라미터와 에일리어스가 충돌하면 `In` 접두사로 구분해라 (`InElementType` → `using ElementType = InElementType;`).
- 매크로는 전부 대문자 + 언더스코어로 쓰고 `UE_` 접두사를 붙여라: `#define UE_AUDIT_SPRITER_IMPORT`
- 타입·변수명은 명사로, 메서드명은 메서드의 이펙트(이펙트가 없으면 반환 값)를 설명하는 동사로 지어라. 설명적인 용어 뒤에는 `Type` 접미사를 써라.
- bool을 반환하는 함수는 true/false 질문형으로 지어라: `IsVisible()`, `ShouldClearBuffer()`.
- 프로시저(반환 값이 없는 함수)는 강한 동사 뒤에 오브젝트를 붙여 써라. `Handle`, `Process` 같은 모호한 동사로 시작하지 마라.
- 참조로 전달되고 함수가 값을 기록하는 파라미터에는 `Out` 접두사를, 그게 bool이면 접두사 앞에 `b`를 붙여라: `bOutResult`.
- 과도한 약어를 쓰지 마라. 이름의 범위가 넓을수록 더 서술적으로 지어라. 변수는 각각 별도 줄에 선언해라.
- 값을 반환하는 함수는 이름만 보고 무엇을 반환하는지 알 수 있게 지어라.
  ```cpp
  bool CheckTea(FTea Tea);    // Bad — true일 경우 무슨 의미인지 알 수 없다
  bool IsTeaFresh(FTea Tea);  // Good
  ```

## 포용적 단어 선택

- `blacklist`/`whitelist`, `master`/`slave` 같은 단어 대신 대체어를 써라 (deny list / allow list, primary / secondary 등). 사람이 아닌 것은 항상 `it`, `its`로, 가상의 인물은 단수형이라도 `they`, `them`, `their`로 지칭해라.

## 포터블 C++ 코드

- 크기 의존 코드에는 플랫폼 독립 타입을 써라: `int32`, `uint32`, `uint8`, `int64`, `float`, `double`.
- `int`는 최소 너비 32비트만 보장된다. 정수 너비가 중요치 않은 경우에만 써라.
- 시리얼라이즈·리플리케이트 포맷에는 반드시 명시적으로 크기가 정해진 타입을 써라.
- `bool` 타입을 쓰고 부울 크기를 추정하지 마라. `BOOL`은 컴파일되지 않는다. `TCHAR`, `PTRINT`의 크기도 추정하지 마라.

## 표준 라이브러리 사용

- 표준 컨테이너와 스트링은 interop 코드를 제외하고 쓰지 마라. 동일한 API에서 UE 언어와 표준 라이브러리 언어를 섞지 마라.
- `<atomic>`, `<type_traits>`, `<initializer_list>`, `<limits>`, `<cmath>`는 사용해라. `<regex>`는 에디터 전용 코드 안에 캡슐화해서만 써라. `memcpy()`/`memset()`은 명확한 퍼포먼스 이점이 있을 때만 `FMemory::Memcpy`/`FMemory::Memset` 대신 써라.

## 코멘트

- 코드가 스스로 설명하도록 작성해라. 코드를 반복하거나 코드와 모순되는 코멘트를 달지 마라.
  ```cpp
  // Bad
  t = s + l - b;

  // Good
  TotalLeaves = SmallLeaves + LargeLeaves - SmallAndLargeLeaves;
  ```
- 나쁜 코드에 긴 코멘트를 달지 말고 코드를 다시 작성해라.
- 코드는 구현을, 코멘트는 의도를 설명한다. 코드 한 줄의 의도를 바꾸면 코멘트도 갱신해라.
- 메서드 코멘트는 메서드가 퍼블릭으로 선언되는 곳에 딱 한 번만 작성해라. 호출자와 무관한 구현 세부 사항은 구현 안에 달아라.
- 클래스 코멘트에는 이 클래스가 해결하는 문제와 생성한 이유를 써라.
- 파라미터 코멘트에는 측정 단위, 예상되는 값 범위, '불가능한' 값, 상태/오류 코드의 의미를 써라.
- `@warning`, `@note`, `@see`, `@deprecated`, `@return`은 각각 별도 줄에 선언해라. 함수의 목적이 오로지 그 값을 반환하는 것이면 `@return`을 쓰지 마라.

## Const 정확도

- 함수가 수정하지 않는 실행인자는 `const` 포인터·참조로 전달해라. 오브젝트를 수정하지 않는 메서드는 `const`로 선언해라. 컨테이너를 수정하지 않는 루프에는 `const` 이터레이션을 써라.
- by-value 함수 파라미터와 로컬에도 `const`를 붙여라.
  예외: 컨테이너 안으로 이동하는 pass-by-value 파라미터 (`MemberArray = MoveTemp(InNewArray);`).
- 포인터 자체를 const로 만들 때는 끝에 붙여라 (`T* const Ptr`). 레퍼런스는 재할당이 불가하므로 `T& const`는 틀린 표기다.
- 반환 타입에는 `const`를 쓰지 마라. 이동 시맨틱이 제한되고 기본 타입에는 컴파일 경고가 난다. 이 규칙은 반환 타입 자체에만 적용되며 포인터의 타깃 타입과 반환 레퍼런스에는 해당하지 않는다.
  ```cpp
  // Bad
  const TArray<FThing> GetSomeArray();
  const TArray<FThing>* const GetSomeArray();

  // Good
  const TArray<FThing>& GetSomeArray();
  const TArray<FThing>* GetSomeArray();
  ```

## 최신 C++ 언어 문법

- `static_assert`는 컴파일 시간 어서트에 써라. `override`와 `final` 키워드를 적극 사용해라.
- `NULL` 매크로를 쓰지 마라. 모든 경우 `nullptr`를 써라. C++/CX 빌드 호환이 필요하면 `decltype(nullptr)` 대신 `TYPE_OF_NULLPTR`를 써라.
- `auto`는 아래 세 경우에만 허용한다. 나머지는 타입을 명시해라. C#의 `var`도 같다.
  1. 람다를 변수에 바인딩할 때
  2. 이터레이터 타입이 매우 장황해서 가독성을 해칠 때
  3. 표현식 타입을 쉽게 식별할 수 없는 템플릿 코드
- `auto`를 쓸 때도 `const`, `&`, `*`를 타입 이름과 마찬가지로 정확히 붙여라. C++20 구조체 바인딩은 실질적으로 variadic `auto`이므로 쓰지 마라.
- 범위 기반 For를 선호해라. `TMap`은 이터레이터의 `Key()`/`Value()`가 아니라 `TPair`의 `Key`/`Value` 필드다.
  ```cpp
  // Bad
  for (auto It = MyMap.CreateIterator(); It; ++It) { ... It.Key() ... }

  // Good
  for (TPair<FString, int32>& Kvp : MyMap) { ... Kvp.Key ... }
  ```
- 람다는 두 구문 정도 길이로 유지해라. 사소하지 않은 람다는 일반 함수처럼 문서화해라.
- 자동 캡처(`[&]`, `[=]`) 대신 명시적 캡처를 써라. 지연 실행 시 포인터 캡처는 허상 참조를 만들고, 잘못 캡처된 UObject 포인터는 가비지 컬렉터에 보이지 않으며, `[=]`도 멤버 변수를 참조하면 `this`를 묵시적으로 캡처한다.
- 대규모 람다이거나 다른 함수 호출 결과를 반환할 때는 명시적 반환 타입을 써라.
- 열거형은 `enum class`로 선언해라. 네임스페이스 enum 금지.
  ```cpp
  // Bad
  UENUM() namespace EThing { enum Type { Thing1, Thing2 }; }

  // Good
  UENUM() enum class EThing : uint8 { Thing1, Thing2 };
  ```
- 블루프린트에 노출하는 열거형은 `uint8` 기반으로 선언해라.
- `UPROPERTY`에는 `TEnumAsByte<EThing::Type>` 대신 `EThing`을 직접 써라. 열거형 프로퍼티는 바이트가 아닌 어떤 크기도 될 수 있다.
- 비트 플래그 enum에는 `ENUM_CLASS_FLAGS(EnumType)`를 쓰고, truth 컨텍스트 대신 `None`과 비교해라.
  ```cpp
  // Bad
  if (Flags & EFlags::Flag1)

  // Good
  if ((Flags & EFlags::Flag1) != EFlags::None)
  ```
- 컨테이너·스트링을 값으로 전달·반환할 때 이동 시맨틱이 자동으로 쓰인다. 명시적으로는 `std::move`의 UE 대응인 `MoveTemp`를 써라.
- 디폴트 멤버 이니셜라이저는 엔진 코드보다 게임 코드에 적합하다. 디폴트값을 바꾸면 모든 종속 파일을 리빌드해야 하고, 베이스 클래스·UObject 서브오브젝트·전방 선언 타입 포인터는 이 방식으로 초기화할 수 없다.

## 서드 파티 코드

- 엔진이 사용하는 라이브러리 코드를 수정하면 `//@UE5` 코멘트와 변경 이유를 기록해라.
- 엔진에 포함되는 서드 파티 코드는 `// @third party code - BEGIN <이름>` / `// @third party code - END <이름>`으로 감싸라.

## 코드 포맷

- 중괄호는 항상 새 줄에 열어라. 단일 구문 블록과 if-else의 모든 실행 블록에도 중괄호를 넣어라.
  ```cpp
  // Bad
  int32 GetSize() const { return Size; }

  // Good
  int32 GetSize() const
  {
      return Size;
  }
  ```
- 여러 갈래의 if 문에서 각 `else if`는 첫 번째 `if`와 같은 양만큼 들여써라.
- 줄 시작 부분의 공백에는 스페이스가 아니라 탭을 쓰고, 탭 크기는 4자로 설정해라. C# 코드에도 탭을 써라 (Visual Studio 디폴트는 스페이스이므로 세팅을 바꿔라).
- switch에서 다음 케이스로 넘어갈 때는 `break`를 넣거나 `// falls through`를 명시해라. 동일한 코드를 갖는 빈 케이스는 예외다.
- switch에는 항상 `default` 케이스를 만들고 그 안에도 `break`를 넣어라.
- 함수명과 실행인자 목록에 선행되는 괄호 사이에 스페이스를 넣지 마라.
- 포인터·레퍼런스 기호는 그 오른쪽에 스페이스를 한 칸만 둬라.
  ```cpp
  FShaderType* Ptr;   // Good
  FShaderType *Ptr;   // Bad
  FShaderType * Ptr;  // Bad
  ```

## 네임스페이스

- `UCLASS`, `USTRUCT` 등을 정의할 때는 네임스페이스를 쓸 수 없다. UnrealHeaderTool이 지원하지 않는다.
- `UCLASS`, `USTRUCT` 등이 아닌 새 API는 `UE::` 네임스페이스에 넣어라. 중첩 네임스페이스(예: `UE::Audio::`)가 이상적이다.
- 공개 API의 일부가 아닌 구현 세부 정보는 Private 네임스페이스(예: `UE::Audio::Private::`)에 넣어라.
- 전역 범위에는 `.cpp` 파일에서도 `using` 선언을 넣지 마라 (unity 빌드가 깨진다). 다른 네임스페이스 안이나 함수 바디 안에는 넣어도 된다.
- 전방 선언된 타입은 각각의 네임스페이스 안에서 선언해라. 그러지 않으면 링크 오류가 난다.
- 매크로는 네임스페이스에 넣을 수 없으므로 `UE_` 접두사를 붙여라.

## 물리적 종속성

- 파일명에 클래스 접두사(`U`, `A` 등)를 붙이지 마라 (`Scene.cpp`, not `UScene.cpp`).
- 모든 헤더 파일에 `#pragma once`를 써라.
- 물리적 결합을 최소화해라. 특히 다른 헤더에서 표준 라이브러리 헤더를 include하지 마라.
- 헤더 include 대신 전방 선언(forward declaration)이 가능하면 그렇게 해라.
- `Core.h` 대신 정의가 필요한 특정 헤더를 include해라. 간접 include에 의존하지 말고 필요한 것을 전부 직접 include해라.
- 다른 모듈이 필요로 하는 정의는 `Public` 디렉터리에, 나머지는 `Private` 디렉터리에 둬라.
- 큰 함수는 논리적 하위 함수로 나눠라. 인라인 함수는 사소한 접근자이거나 프로파일링으로 이점이 확인된 경우에만 써라. `FORCEINLINE` 사용에 보수적이어라.

## 캡슐화

- 클래스 멤버는 public/protected 인터페이스의 일부가 아니면 거의 항상 `private`으로 선언해라.
- 파생 클래스에서만 쓰는 필드는 `private`으로 만들고 보호된 접근자를 제공해라.
- 더 이상 파생시킬 클래스가 아니면 `final`을 써라.

## 일반적인 스타일 문제

- 종속성 거리를 최소화해라. 변수 값은 사용하기 직전에 설정해라. 메서드는 가급적 하위 메서드로 분할해라.
- 컴파일러 경고를 모두 해결해라. 최후의 수단으로만 `#pragma`로 억제해라.
- `.cpp`와 `.h` 파일 끝에 빈 줄을 하나 만들어라 (gcc 대응).
- 디버그 코드는 전반적으로 유용하고 잘 다듬어진 상태가 아니면 체크인하지 마라.
- 문자열 리터럴에는 항상 `TEXT()` 매크로를 써라.
- 루프에서 반복되는 공통 하위 표현식은 밖으로 꺼내라. 스트링 리터럴에서의 `FName` 생성 같은 전역 함수 호출은 statics로 중복 연산을 피해라.
- 핫 리로드를 염두에 둬라. 리로드 중 변할 수 있는 함수에는 인라인·템플릿을 쓰지 마라.
- 변수 섀도잉을 허용하지 마라.
- 복잡한 조건식은 이름 붙인 중간 변수로 분리해라.
  ```cpp
  // Bad
  if ((Blah->BlahP->WindowExists->Etc && Stuff) && !(bPlayerExists && bGameStarted && IsTuesday()))

  // Good
  const bool bIsLegalWindow = Blah->BlahP->WindowExists->Etc && Stuff;
  const bool bIsPlayerDead = bPlayerExists && bGameStarted && IsTuesday();
  if (bIsLegalWindow && !bIsPlayerDead)
  ```
- 함수 호출에 익명 리터럴을 쓰지 마라. 명명된 상수로 의미를 설명해라: `Trigger(TEXT("Soldier"), 5, true)` → `Trigger(ObjectName, CooldownInSeconds, bVulnerableDuringCooldown)`
- 헤더에 특수한 스태틱 변수를 정의하지 마라. `extern SOMEMODULE_API`로 선언하고 `.cpp`에서 정의해라.
- 동작 변경 없는 대규모 형식 변경(공백 정리, 변수 일괄 이름 변경)은 동작 변경 커밋과 섞지 마라.

## API 디자인 가이드라인

- 플래그 용도의 `bool` 파라미터를 쓰지 마라. `enum class`로 대체해라.
  ```cpp
  // Bad
  FCup* MakeCupOfTea(FTea* Tea, bool bAddSugar = false, bool bAddMilk = false, bool bAddHoney = false);
  FCup* Cup = MakeCupOfTea(Tea, false, true, true);

  // Good
  enum class ETeaFlags { None, Milk = 0x01, Sugar = 0x02, Honey = 0x04, Lemon = 0x08 };
  ENUM_CLASS_FLAGS(ETeaFlags)
  FCup* MakeCupOfTea(FTea* Tea, ETeaFlags Flags = ETeaFlags::None);
  FCup* Cup = MakeCupOfTea(Tea, ETeaFlags::Milk | ETeaFlags::Honey);
  ```
  예외: setter처럼 완전한 상태를 전달할 때 (`void FWidget::SetEnabled(bool bEnabled)`).
- 파라미터가 많은 함수는 전용 struct로 묶어라: `MakeTeaForParty(const FTeaPartyParams& Params)`
- `bool`과 `FString`으로 함수를 오버로드하지 마라. `Func(TEXT("String"))`이 bool 오버로드를 호출한다.
- 인터페이스 클래스(`I` 접두사)는 항상 추상형이어야 하고 멤버 변수를 가지지 마라. 순수 가상이 아닌 메서드는 포함할 수 있고, 인라인 구현되면 비가상·정적 메서드도 포함할 수 있다.
- 파생 클래스에서 가상 함수를 오버라이드할 때 `virtual`과 `override`를 모두 써라.

## 플랫폼별 코드

- 플랫폼별 코드는 `Engine/Platforms/[PLATFORM]/...` 하위의 플랫폼별 소스 파일로 추상화해라. `[PLATFORM]` 디렉터리 밖에서 `PLATFORM_XBOXONE` 같은 형태를 쓰지 말고, 하드웨어 추상화 레이어에 static 함수를 추가하거나 `Platform.h`에 디폴트값을 둔 `#define`(예: `PLATFORM_USE_PTHREADS`)을 써라.

## 컨테이너

공식 코딩 표준 페이지 밖(컨테이너·TArray 문서)에서 온 규칙이다.

- `TArray`는 값 타입으로 취급해라. `new`/`delete`로 생성하지 마라.
- 복잡한 타입 삽입에는 `Add` 대신 `Emplace`를 써라 (불필요한 임시 변수 방지).
- 순서가 중요하지 않은 제거에는 `RemoveSwap`, `RemoveAtSwap`, `RemoveAllSwap`을 써라.
- 컨테이너 이동에는 `MoveTemp()`를 써라. 이동 후 원본은 빈 상태가 보장된다.
- `AddUninitialized`, `InsertUninitialized`, `SetNumUninitialized`는 `FVector`, `FMatrix` 같은 자명한 타입에만 써라. 생성자가 필요한 복잡한 타입에 쓰면 미정의 동작이 발생한다.
- 다수의 원소를 추가하기 전 `Reserve()`로 미리 할당해라. 빈 슬롯 제거는 `Compact()`, 남은 슬랙 제거는 `Shrink()`.
- 유일한 원소 집합이 필요하면 `TArray` 대신 `TSet`을 써라.

## UObject·액터

공식 코딩 표준 페이지 밖(오브젝트·액터 문서)에서 온 규칙이다.

- UObject는 런타임에 `NewObject`로, 생성자 안에서는 `CreateDefaultSubobject`로만 생성해라.
- Actor 초기화 로직은 생성자가 아닌 `BeginPlay()`에 두어라.
- UObject는 포인터로 전달해라. 참조 전달 금지: `void AddActorToList(AActor* Obj)`

## 출처

- https://dev.epicgames.com/documentation/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine
- https://dev.epicgames.com/documentation/unreal-engine/array-containers-in-unreal-engine
- https://dev.epicgames.com/documentation/unreal-engine/containers-in-unreal-engine
- https://dev.epicgames.com/documentation/unreal-engine/objects-in-unreal-engine
- https://dev.epicgames.com/documentation/unreal-engine/actors-in-unreal-engine
