// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/CL9Damageable.h"
#include "Data/CL9Types.h"
#include "GameFramework/Character.h"
#include "CL9Character.generated.h"

class ACL9Cart;
class ACL9Item;
class ACL9PlayerState;
class UCameraComponent;
class UCL9CharacterDef;
class UCL9CharacterMovement;
class UCL9RulesDef;
class UCameraShakeBase;
class UInputAction;
class UMaterialInterface;
struct FInputActionValue;

/** "[E] …" 프롬프트 텍스트가 바뀔 때 소유 클라이언트에서 발동 — HUD가 폴링 대신 이걸 구독한다 (docs/systems/ui_system.md) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCL9FocusPromptChanged);

/**
 * 1인칭 협동 캐릭터: 캡슐, 무브먼트, 카메라, 입력이 모두 여기에 있다.
 * 모든 수치와 메시는 SetCharacterAlias를 통해 적용되는 캐릭터 테이블 행
 * (UCL9CharacterDef)에서 나온다 — 이 클래스는 에셋 참조를 전혀 갖지 않으므로
 * 구매한 에셋을 C++을 건드리지 않고 교체할 수 있다 (docs/plan/build-order.md 원칙 2·3).
 *
 * 소지 관련 입력은 이 (소유된) 액터에서 Server RPC를 타고, 호스트는 소유되지
 * 않은 아이템 액터에 대해 일반 함수를 직접 호출한다 (docs/systems/input.md 상호작용 틀).
 */
UCLASS(abstract)
class ACL9Character : public ACharacter, public ICL9Damageable
{
	GENERATED_BODY()

public:
	ACL9Character(const FObjectInitializer& ObjectInitializer);

	/**
	 * 서버 전용: 캐릭터 테이블 행을 골라 로컬에 적용한다.
	 * 클라이언트는 리플리케이트되는 alias를 따라간다 (docs/systems/character.md
	 * "캐릭터 Def 적용 경로").
	 */
	void SetCharacterAlias(FName NewAlias);

	/** 현재 적용된 테이블 행 — alias가 도착하기 전까지는 null */
	const UCL9CharacterDef* GetCharacterDef() const
	{
		return CharacterDef;
	}

	/** 이 캐릭터의 손에 들려 있는 아이템 액터, 없으면 null */
	ACL9Item* GetHeldItem() const
	{
		return HeldItem;
	}

	/** 머리에 착용한 아이템(헤드램프), 없으면 null — 손은 이것 때문에 막히지 않는다 */
	ACL9Item* GetWornItem() const
	{
		return WornItem;
	}

	/** 이 캐릭터가 끌고 있는 카트, 없으면 null — 예측 무브먼트가 CartMult 계산에 이 값을 읽는다 */
	ACL9Cart* GetPullingCart() const
	{
		return PullingCart;
	}

	/** 호스트 전용, 카트가 연결/분리될 때 카트가 호출한다 */
	void SetPullingCart(ACL9Cart* Cart);

	/** 호스트 전용: 든 아이템을 파괴하고 미러를 비운다 (항아리 반납이 이걸 먹는다) */
	void HostConsumeHeldItem();

	/** 호스트 전용: 든 아이템을 미러에서 떼어내고 액터를 넘긴다 (카트 적재용) */
	ACL9Item* HostTakeHeldItem();

	/** 손, 주머니, 카트 중 무엇이든 하나라도 있으면 — 빈손이면 DeaggroRadius에서 추격을 놓는다 */
	bool IsCarryingAnything() const;

	/**
	 * 호스트 전용 (docs/systems/character.md 피격·넘어짐·죽음): HP 감소, 넘어짐,
	 * 손+주머니 흘림. HP가 0이 되면 그날 밤 내내 쓰러진 채로 있으며 죽음 처리를
	 * 게임 모드로 넘긴다 (전멸 체크, SpectateDelay 이후 관전).
	 */
	virtual void OnDamage(int32 Amount, AActor* Source) override;

	/** 호스트 전용: HP를 굴려진 최대치로 되돌린다 — 트럭 패드의 치유 (docs/plan/data/rules.md 죽음과 회복) */
	void HostHealFull();

	/** 호스트 전용: 다음 로비에서 죽은 캐릭터를 풀 HP로 다시 세운다 — 부활은 없다, 밤이 끝나야 돌아온다 */
	void HostRevive();

	/** 붙잡혀 넘어진 상태로 이동이 막혀 있는 동안 true — CMC가 이 값을 읽어 속도 0을 적용한다 */
	UFUNCTION(BlueprintPure, Category="CL9")
	bool IsKnockedDown() const;

	/**
	 * 지금 걸려 있는 먹기 효과들이 이 스탯에 **더하는** 값의 합 (docs/plan/data/item/common.md 타입과 먹기).
	 * 만료된 것은 세지 않는다. CMC가 이동 공식에서 읽으므로, 서버와 자율 프록시가 같은
	 * 값을 도출해야 예측이 어긋나지 않는다 — 그래서 효과는 복제되고 시각으로 만료된다.
	 */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetStatBonus(ECL9Stat Stat) const;

	/**
	 * 이 스탯에 걸린 효과 중 가장 늦게 끝나는 것까지 남은 초, 걸린 게 없으면 0.
	 * 만료 판정은 `GetStatBonus`와 같은 시각·같은 조건을 쓴다 — 둘이 어긋나면
	 * "+40인데 남은 시간 0초" 같은 표시가 난다. HUD의 버프 줄이 이 값을 읽는다.
	 */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetStatBonusRemaining(ECL9Stat Stat) const;

	/**
	 * 지금 걸려 있는 버프들의 별칭, 중복 없이 — 만료된 것은 빼고 건 순서대로.
	 * 화면이 효과를 스탯이 아니라 **버프 단위로** 묶어 보여줄 때 쓴다.
	 */
	UFUNCTION(BlueprintPure, Category="CL9")
	TArray<FName> GetActiveBuffs() const;

	/** 그 버프가 끝날 때까지 남은 초 — 같은 별칭의 효과 중 가장 늦게 끝나는 것 기준, 없으면 0 */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetBuffRemaining(FName BuffAlias) const;

	/**
	 * 호스트 전용: 손에 든 것을 먹는다 — `Dish` 타입만 먹을 수 있고(별도 플래그를 두지 않는다),
	 * 행의 `EatBuffs`가 가리키는 버프들이 걸린다. 먹은 물건은 사라진다. HP는 효과 대상이 아니다 —
	 * 회복 채널은 트럭 패드다 (docs/plan/data/rules.md).
	 */
	void HostEat();

	/** PlayerState의 HP가 0이다 — 몬스터는 죽은 대상을 건너뛴다 */
	UFUNCTION(BlueprintPure, Category="CL9")
	bool IsDead() const;

	/**
	 * 지금 E를 한 번 누르면 무엇을 하게 될지, HUD 프롬프트("[E] 줍기" …)로 표현한 것 —
	 * 로컬 focus에 대해 Server_PickPlace의 분기 순서를 그대로 반영하며, 없으면 비어
	 * 있다. 표시 전용이다 (docs/systems/input.md 상호작용 프롬프트).
	 */
	UFUNCTION(BlueprintPure, Category="Interaction")
	FText GetFocusPrompt() const;

	/** 소유 클라이언트: 프롬프트 텍스트가 바뀜(대상 또는 그 상태) — HUD 갱신용 */
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FCL9FocusPromptChanged OnFocusPromptChanged;

	/** 로컬 조준이 지금 향하고 있는 대상(윤곽선이 그려진 액터) — 없으면 null, HUD의 월드 라벨이 이걸 따라간다 */
	UFUNCTION(BlueprintPure, Category="Interaction")
	AActor* GetFocusedActor() const
	{
		return FocusedActor.Get();
	}

	/**
	 * HUD 게이지용 소유 클라이언트의 로컬 차징 값: 차징 중이 아니면 -1,
	 * 그 외에는 ThrowChargeTime에 걸쳐 0..1 (docs/systems/input.md 던지기 차징 — 로컬 예측,
	 * 실제 판정 시각은 호스트가 별도로 잰다).
	 */
	UFUNCTION(BlueprintPure, Category="Throw")
	float GetThrowChargeAlpha() const;

	/** 무브먼트 컴포넌트를 CL9 예측 무브먼트 타입으로 반환한다 */
	UCL9CharacterMovement* GetCL9CharacterMovement() const;

	/** 1인칭 메시를 반환한다 (소유한 플레이어에게만 보임) */
	USkeletalMeshComponent* GetFirstPersonMesh() const
	{
		return FirstPersonMesh;
	}

	/** 1인칭 카메라를 반환한다 */
	UCameraComponent* GetFirstPersonCameraComponent() const
	{
		return FirstPersonCameraComponent;
	}

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** IA_Move를 처리한다 (docs/systems/input.md) */
	void MoveInput(const FInputActionValue& Value);

	/** IA_Look을 처리한다 */
	void LookInput(const FInputActionValue& Value);

	/** IA_Run 누름을 처리한다 — 속도가 아니라 이동 플래그만 설정한다 */
	void RunInputStart();

	/** IA_Run 뗌을 처리한다 */
	void RunInputEnd();

	/** IA_PickPlace(E)를 처리한다 — 줍기 > 상호작용 > 놓기 순서로 호스트가 판정한다 */
	void PickPlaceInput();

	/** IA_Throw 누름을 처리한다 — 호스트가 차징 시각을 잰다 (docs/systems/input.md 던지기 차징) */
	void ThrowStartInput();

	/** IA_Throw 뗌을 처리한다 */
	void ThrowEndInput();

	/** IA_Swap(R)을 처리한다 — SwapTime 잠금 아래에서 손↔주머니를 교체한다 */
	void SwapInput();

	/** IA_Flashlight(F)를 처리한다 — 든 손전등을 켜고 끈다 (docs/systems/light.md) */
	void FlashlightInput();

	/** E 한 번 누름 — 우선순위: 주울 수 있는 아이템 > 상호작용 대상 > 놓기 (docs/systems/input.md 감지) */
	UFUNCTION(Server, Reliable)
	void Server_PickPlace();

	UFUNCTION(Server, Reliable)
	void Server_ThrowChargeStart();

	UFUNCTION(Server, Reliable)
	void Server_ThrowRelease();

	UFUNCTION(Server, Reliable)
	void Server_Swap();

	UFUNCTION(Server, Reliable)
	void Server_Eat();

	/** 로컬 입력 → 서버 판정 */
	void EatInput();

	/** 빈손 좌클릭 — 몬스터를 앞으로 밀친다 (프라이팬 대역, docs/systems/input.md) */
	UFUNCTION(Server, Reliable)
	void Server_Shove();

	/** 호스트에서 든 손전등을 켜고 끈다 (아이템 상태는 리플리케이트됨 — light.md) */
	UFUNCTION(Server, Reliable)
	void Server_Flashlight();

	UFUNCTION()
	void OnRep_CharacterAlias();

	/** 로컬 레지스트리에서 행을 읽어와 판정용 수치와 표시를 적용한다 */
	void ApplyCharacterDef();

	/** 소유 클라이언트 전용: 스태미나가 규칙 임계값 아래로 떨어지면 카메라가 흔들리고, 비워질수록 더 심해진다 (docs/systems/character.md) */
	void UpdateStaminaShake();

	/** 로컬 플레이어에서 재생 중인 피로 흔들림 인스턴스 — 쉬고 있을 때는 null */
	UPROPERTY(Transient)
	TObjectPtr<UCameraShakeBase> StaminaShakeInstance;

	/** 이동 입력 액션 — BP 자식에서 지정되며 C++에서는 로드하지 않는다 */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	/** 시점 입력 액션 */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> LookAction;

	/** 달리기 입력 액션 */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> RunAction;

	/** 점프 입력 액션 */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> JumpAction;

	/** 줍기/놓기/상호작용 액션 (E) */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> PickPlaceAction;

	/** 던지기 차징 액션 (마우스 좌클릭) */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> ThrowAction;

	/** 손↔주머니 교체 액션 (R) */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> SwapAction;

	/** 손전등 토글 액션 (F) */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> FlashlightAction;

	/** 먹기 액션 (G) — 손에 든 요리를 먹는다 */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> EatAction;

	/** 조준이 상호작용할 대상 위에 그려지는 오버레이 — BP 자식에서 지정된다 */
	UPROPERTY(EditAnywhere, Category="Interaction")
	TObjectPtr<UMaterialInterface> HighlightOverlayMaterial;

private:
	/**
	 * 상호작용 키가 작동할 유일한 대상: PickRadius 안쪽에서 대략 정면에 있는
	 * 후보들 중 줍을 수 있는 아이템이 상호작용 대상보다 우선하고, 동률이면
	 * 조준 방향이 기준이 된다 (docs/systems/input.md 감지). 하이라이트를 위해
	 * 매 프레임 로컬에서 실행되고, 판정을 위해 호스트에서 다시 실행된다.
	 */
	AActor* ResolveFocus() const;

	/** 표시 전용: 대상 메시의 오버레이 머티리얼을 켜고 끈다 */
	void SetFocusHighlight(AActor* Target, bool bOn) const;

	/** 로컬 플레이어가 현재 조준 중인 대상 (하이라이트 기록용) */
	TWeakObjectPtr<AActor> FocusedActor;

	/** HUD에 마지막으로 전달한 프롬프트 — 텍스트가 달라질 때만 OnFocusPromptChanged가 발동한다 */
	FText LastFocusPrompt;

	/** 호스트 전용: 손과 주머니를 바닥에 흘리고 카트를 놓는다 (잡힘 짐 흘림) */
	void HostDropEverything();

	/** 호스트 전용: 아이템을 이 캐릭터의 손 위치에 붙이고 미러링한다 */
	void HostPickUp(ACL9Item* Item, const UCL9RulesDef* Rules);

	/** 호스트 전용: Head 슬롯 아이템(헤드램프)을 착용시키고 미러링한다 — E 한 번, 손은 그대로 둔다 */
	void HostWear(ACL9Item* Item);

	/** 호스트 전용: 든 아이템을 앞쪽 바닥에 내려놓는다 */
	void HostPlaceHeld(const UCL9RulesDef* Rules);

	const UCL9RulesDef* GetRules() const;
	ACL9PlayerState* GetCL9PlayerState() const;
	float GetServerTime() const;

	/** 이 폰이 연기할 캐릭터 행 — 스폰 시 서버가 정하고 모든 머신에 적용된다 */
	UPROPERTY(ReplicatedUsing=OnRep_CharacterAlias)
	FName CharacterAlias;

	/** alias가 가리키는 캐시된 행 (레지스트리가 살아있게 유지하며, 빠른 읽기를 위해 캐시한다) */
	UPROPERTY()
	TObjectPtr<const UCL9CharacterDef> CharacterDef;

	/** 소지 상태의 실제 정답 링크 — PlayerState.HandItem이 UI를 위해 이걸 미러링한다 (docs/systems/inventory.md) */
	UPROPERTY(Replicated)
	TObjectPtr<ACL9Item> HeldItem;

	UPROPERTY(Replicated)
	TObjectPtr<ACL9Cart> PullingCart;

	/** 머리에 착용한 아이템 액터 (Head 소지 슬롯) — PlayerState.HeadItem이 이걸 미러링한다 */
	UPROPERTY(Replicated)
	TObjectPtr<ACL9Item> WornItem;

	/** 현재 던지기 차징이 시작된 호스트 시각; 대기 중이면 음수 */
	float ThrowChargeStartTime = -1.f;

	/** 같은 차징에 대한 소유 클라이언트의 로컬 시각 (게이지 전용, 판정에는 쓰이지 않음); 대기 중이면 음수 */
	float LocalThrowChargeStartTime = -1.f;

	/** 다음 밀치기가 거부되는 기준이 되는 호스트 시각 (휘두른 뒤 회복 시간) */
	float ShoveReadyTime = 0.f;

	/**
	 * 넘어짐 상태가 캐릭터를 붙들어두는 서버 시각 (0이면 일어난 상태). CartMult처럼
	 * CMC가 읽는 리플리케이트 값이다 — 양쪽 모두 같은 값에서 속도 0을 도출하므로
	 * 보정 폭이 작게 유지된다 (docs/systems/character.md
	 * 넘어짐 이동 차단 — CMC 최대 속도 0 방식 선택, 2026-09-03).
	 */
	UPROPERTY(Replicated)
	float KnockdownEndServerTime = 0.f;

	/**
	 * 지금 걸려 있는 먹기 효과들 — 복제된다. CMC가 이동 공식에서 읽으므로 서버와 자율
	 * 프록시가 같은 값을 봐야 한다(넘어짐의 종료 시각과 같은 이유).
	 */
	UPROPERTY(Replicated)
	TArray<FCL9ActiveEffect> ActiveEffects;

	/** 눈높이에서 캡슐에 붙어 있는 1인칭 카메라 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	/**
	 * 1인칭 시점 메시. 소지한 아이템은 여기의 손 소켓에 붙는다.
	 * 템플릿 방식대로 바디 메시 아래에 붙어 있다: 소유한 FP 리그
	 * (ABP_FP_Copy + CtrlRig_FPWarp)가 바디 공간에서 전신 메시를 애니메이션하고
	 * 팔을 카메라 쪽으로 워프한다 — 팔만 있는 뷰모델 에셋으로는 나중에
	 * 테이블 컬럼만 바꿔서 교체할 수 있다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;
};
