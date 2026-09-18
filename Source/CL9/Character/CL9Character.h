// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/CL9Damageable.h"
#include "Data/CL9Types.h"
#include "GameFramework/Character.h"
#include "CL9Character.generated.h"

enum class ECL9HoldPose : uint8;

class ACL9Cart;
class ACL9Item;
class ACL9Monster;
class ACL9PlayerState;
class UCameraComponent;
class UCL9CharacterDef;
class UCL9CharacterMovement;
class UCL9ItemDef;
class UCL9RulesDef;
class UCameraShakeBase;
class UEnhancedInputUserSettings;
class UInputAction;
class UInputMappingContext;
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

	/** 지금 손에 들린 퀵슬롯 칸 번호(0부터) — PlayerState.ActiveSlot의 읽기 창구 */
	UFUNCTION(BlueprintPure, Category="Carry")
	int32 GetActiveSlot() const;

	/**
	 * 칸 전환(휠·숫자키)과 줍기가 허용되는 상태인가 — 활성 칸에 두 손 아이템(`HoldPose` Large)이
	 * 들려 있거나 수레를 미는 중이면 false (docs/systems/inventory.md 잠금). 판정은 호스트가 같은
	 * 함수로 다시 하고, 클라에서는 프롬프트·입력 걸러내기의 보조로만 쓴다.
	 */
	UFUNCTION(BlueprintPure, Category="Carry")
	bool CanSwitchSlot() const;

	/** 호스트 전용: 든 아이템을 파괴하고 활성 칸을 비운다 (항아리 반납·먹기가 이걸 먹는다) */
	void HostConsumeHeldItem();

	/** 퀵슬롯 어느 칸이든, 또는 카트 중 하나라도 있으면 — 빈손이면 DeaggroRadius에서 추격을 놓는다 */
	bool IsCarryingAnything() const;

	/**
	 * 호스트 전용 (docs/systems/character.md 피격·넘어짐·죽음): HP 감소, 넘어짐,
	 * 퀵슬롯 네 칸 전부 흘림. HP가 0이 되면 그날 밤 내내 쓰러진 채로 있으며 죽음 처리를
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
	 * 손에 든 아이템 행의 자세 범주, 빈손이면 None — 전신 ABP(ABP_Unarmed)가 상체 Hold 레이어를
	 * 고르는 데 읽는다. HeldItem이 복제되므로 모든 머신에서 같은 값이 나온다 (docs/systems/inventory.md).
	 */
	UFUNCTION(BlueprintPure, Category="CL9")
	ECL9HoldPose GetHoldPose() const;

	/**
	 * 마지막 휘두르기(HostSwing)로부터 지난 서버 초 — ABP가 이 값으로 휘두르기 클립을 시퀀스 평가기에
	 * 태운다(클립 길이 안이면 재생 중). 시작 시각이 복제되므로 모든 머신이 같은 시점을 보고, 애님은
	 * 판정(즉시)과 무관한 표현이다 (docs/systems/animation.md 상태→애님). 휘두른 적 없으면 큰 값.
	 */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetSwingElapsed() const;

	/**
	 * 좌클을 누른 채 공격 대기 중이면 대기를 시작한 뒤 지난 서버 초, 대기 중이 아니면 -1 — ABP가 대기 자세
	 * (`AN_Farmer_SwingWindup` 루프)를 켜는 데 읽는다. 대기는 무한이고 떼는 순간 휘두른다 (docs/systems/inventory.md 사용).
	 */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetSwingWindupElapsed() const;

	/**
	 * 마지막으로 먹은(HostEat) 뒤 지난 서버 초 — ABP가 먹기 클립(`AN_Farmer_Eat` 0.6초)을 태운다. 판정은 먹는 순간
	 * 끝났고 애니는 뒤따르는 표현이라 먹는 동안 칸 전환도 막지 않는다. 먹은 적 없으면 큰 값(1000 이상).
	 */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetEatElapsed() const;

	/**
	 * 넘어진 뒤 지난 초 — 넘어짐·죽음 애님(AN_Farmer_Knockdown → Down → GetUp)의 원천. 넘어져 있지 않으면 −1
	 * (docs/systems/animation.md 넘어짐 가지). 죽음도 넘어짐이라 죽은 뒤로도 계속 커진다.
	 */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetKnockdownElapsed() const;

	/** 넘어짐이 풀릴 때까지 남은 초 — 일어나기 클립(AN_Farmer_GetUp 0.6초)을 끝에 맞춰 트는 데 읽는다. 죽었으면 큰 값, 넘어져 있지 않으면 −1 */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetKnockdownRemaining() const;

	/**
	 * 피격 비네트의 세기 0..1 — 잡힌 순간 1이고 `HurtVignetteTime`에 걸쳐 0으로 옅어진다. 잡힌 적이 없거나
	 * 다 옅어졌으면 0. HUD가 빨간 전면 이미지의 불투명도로 읽는다 (docs/systems/ui.md 비네트).
	 */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetHurtFlash() const;

	/** 던지기 차징을 시작한 뒤 지난 초 — 대기 루프(AN_Farmer_ThrowWindup)를 켜는 데 읽는다. 소유자는 로컬 시각으로 즉시. 차징 중이 아니면 −1 */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetThrowWindupElapsed() const;

	/** 마지막 던지기(놓은 순간)부터 지난 초 — AN_Farmer_Throw 원샷의 원천. 던진 적 없으면 큰 값(1000 이상) */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetThrowElapsed() const;

	/**
	 * 시선 피치(도, -90..90 — 위가 양수). 소유자는 컨트롤 피치, 남의 화면에서는 복제된 `RemoteViewPitch`
	 * (ACharacter 기본, `GetBaseAimRotation`)다. **전신 ABP(`ABP_Unarmed`) 전용** — Hold 레이어 뒤
	 * `Transform (Modify) Bone`(spine_03)에 이 값 × 비율을 넣어 남의 화면에서 상체·손이 시선을 따르게 한다.
	 * 1인칭 팔은 카메라 자식이라 시선을 저절로 따르므로 팔 ABP는 이 값을 쓰지 않는다
	 * (docs/systems/character.md 시점, 2026-09-13).
	 */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetAimPitch() const;

	/**
	 * 달리기 속도로 실제 움직이는 중 — CMC `IsRunning()` 위임. 달리기 플래그는 예측 무브에만 실리므로 소유자와
	 * 호스트에서만 참이 나오고 남의 클라에서는 항상 거짓이다 — 1인칭 팔(소유자 전용)의 `AN_Farmer_FPRun`
	 * 분기용이지 전신 로코모션용이 아니다(그쪽은 속도로 고른다).
	 */
	UFUNCTION(BlueprintPure, Category="CL9")
	bool IsRunning() const;

	/** 수평 이동 속도(cm/s) — 로코모션 PlayRate = 이 값 ÷ 클립 기준 속도 (docs/systems/animation.md 재생 배율) */
	UFUNCTION(BlueprintPure, Category="CL9")
	float GetGroundSpeed() const;

	/** 좌클 액션 — HUD 첫 줍기 힌트가 "[{0}] 살짝 누르기"의 키 표기를 읽는다 (docs/systems/ui.md 힌트) */
	UFUNCTION(BlueprintPure, Category="Input")
	const UInputAction* GetUseAction() const
	{
		return UseAction;
	}

	/** 우클 액션 — 힌트의 "[{1}] 꾹 누르기 — 먹기" 키 표기용 */
	UFUNCTION(BlueprintPure, Category="Input")
	const UInputAction* GetScanAction() const
	{
		return ScanAction;
	}

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

	/** 발밑 월드 위치 — 소음이 나는 자리다(캡슐 중심이 아니라 딛고 선 바닥). 밀치기 파동도 민 사람의 이 자리를 쓴다 */
	FVector GetFootLocation() const;

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

	/** 착지 — 호스트는 소음 사건을 내고, 소유 클라는 같은 파동을 예측해 먼저 그린다 (docs/systems/wave.md `Land`) */
	virtual void Landed(const FHitResult& Hit) override;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** IA_Move를 처리한다 (docs/systems/input.md) */
	void MoveInput(const FInputActionValue& Value);

	/** IA_Look을 처리한다 */
	void LookInput(const FInputActionValue& Value);

	/** IA_Run 누름을 처리한다 — 속도가 아니라 이동 플래그만 설정하고, 던지기 조합(달리기 키 + 놓기 키)용 플래그를 기록한다 */
	void RunInputStart();

	/** IA_Run 뗌을 처리한다 */
	void RunInputEnd();

	/** IA_PickPlace(E)를 처리한다 — 착용 > 줍기 > 상호작용 > 수레 놓기 순서로 호스트가 판정한다 */
	void PickPlaceInput();

	/**
	 * IA_Drop(G) 누름 — 달리기 키가 눌려 있으면 던지기 차징 시작(호스트가 차징 시각을 잰다,
	 * docs/systems/input.md 던지기 차징), 아니면 즉시 앞에 놓는다.
	 */
	void DropStartInput();

	/** IA_Drop 뗌 — 차징 중이었으면 던진다. 차징 중 달리기 키를 먼저 떼도 차징은 유지된다 */
	void DropEndInput();

	/** IA_Slot1..4(숫자키) — 그 번호 칸을 활성으로 (docs/systems/inventory.md 칸 전환) */
	void SlotInput(int32 Slot);

	/**
	 * IA_SlotScroll(마우스 휠, Axis1D) — 아래(−)는 다음 칸, 위(+)는 이전 칸, 양 끝은 순환.
	 * Triggered는 값이 있는 프레임마다 오므로 부호만 보고 한 번에 한 칸씩, 짧은 디바운스로 겹침을 거른다.
	 */
	void SlotScrollInput(const FInputActionValue& Value);

	/** IA_Flashlight(F)를 처리한다 — 쓴 헤드램프를 켜고 끈다 (docs/systems/light.md) */
	void FlashlightInput();

	/** IA_Use(좌클) 누름 — 호스트에 사용 시작을 알린다: Swing이면 공격 대기, ToggleLight면 즉시 토글 (docs/systems/input.md 사용) */
	void UseStartInput();

	/** IA_Use(좌클) 뗌 — 대기 중이었으면 이 순간 휘두른다. 탭이면 대기 한 틱 뒤 바로 타격이라 "평범한 공격"이 된다 */
	void UseEndInput();

	/**
	 * IA_Scan(우클) 누름 — `CL9Rules.UseHoldTime` 로컬 타이머를 건다. 판정은 호스트가 하지만 "얼마나 눌렀나"는
	 * 입력이라 로컬에서 잰다 (던지기 차징이 호스트 시각을 쓰는 것과 달리 결과가 이산적이다).
	 */
	void ScanStartInput();

	/** IA_Scan(우클) 뗌 — 문턱 전이면 탭 = 스캔(`ACL9PlayerController::RequestScan`). 문턱을 넘겼으면 이미 먹기가 나갔다 */
	void ScanEndInput();

	/** 우클 길게 누르기 문턱을 넘겼다 — 먹기 요청 (활성 칸이 Dish가 아니면 호스트가 무시한다) */
	void EatHoldReached();

	/** E 한 번 누름 — 우선순위: 착용 > 주울 수 있는 아이템 > 상호작용 대상 > 미는 수레 놓기 (docs/systems/input.md 감지). 놓기는 여기 없다 — G다 */
	UFUNCTION(Server, Reliable)
	void Server_PickPlace();

	UFUNCTION(Server, Reliable)
	void Server_ThrowChargeStart();

	UFUNCTION(Server, Reliable)
	void Server_ThrowRelease();

	/**
	 * 칸 전환 — 호스트가 CanSwitchSlot을 다시 확인한 뒤, 활성 칸 액터를 파괴해 데이터로 만들고
	 * 새 활성 칸을 다시 스폰해 손에 붙인다(값은 칸에 저장된 것을 되돌린다). 빈 칸도 고를 수 있다 = 빈손.
	 */
	UFUNCTION(Server, Reliable)
	void Server_SelectSlot(uint8 Slot);

	/**
	 * 휠 전환 — Direction +1은 다음 칸, −1은 이전 칸(순환). 클라의 ActiveSlot 사본은 복제가 늦을 수 있어
	 * 목표 칸을 클라가 계산하지 않고 호스트가 자기 값에서 한 칸 옮긴다 — 연속 노치가 같은 칸을 겨누지 않는다.
	 */
	UFUNCTION(Server, Reliable)
	void Server_ScrollSlot(int32 Direction);

	/** G — 활성 칸의 아이템을 지금 손이 있는 자리에 그대로 놓는다(옮기지 않는다). 그 칸은 비고 활성 칸 번호는 그대로다 */
	UFUNCTION(Server, Reliable)
	void Server_Drop();

	/**
	 * 좌클 누름 — 호스트가 든 아이템 행의 UseAction으로 판정한다. `Swing`이면 **공격 대기**를 시작하고
	 * (`SwingWindupServerTime` 기록, 무한 대기), `ToggleLight`면 즉시 켜고 끄며, `None`·빈손은 아무 일도 없다
	 * (2026-09-13 리썰 삽 방식: "누르면 대기, 떼면 타격"). 먹기는 여기 없다 — 우클 길게(Server_Eat)다.
	 */
	UFUNCTION(Server, Reliable)
	void Server_UseStart();

	/**
	 * 좌클 뗌 — 대기 중이었으면 HostSwing(피해·사거리·경직은 대기 시간과 무관하게 동일). 대기 중 칸 전환·놓기·
	 * 던지기·먹기·잡힘으로 손의 것이 바뀌면 대기가 취소되어 있어 타격이 나가지 않는다.
	 */
	UFUNCTION(Server, Reliable)
	void Server_UseRelease();

	/**
	 * 우클 길게 — 활성 칸이 `ItemType == Dish`일 때만 HostEat(그 칸 비움, 활성 칸 유지). 판정은 문턱을 넘는
	 * 순간이고 먹기 애니(0.6초)는 뒤따른다 — 먹는 동안 칸 전환을 막지 않는다 (2026-09-13 사용자: "우클 길게 = 먹기").
	 */
	UFUNCTION(Server, Reliable)
	void Server_Eat();

	/**
	 * 호스트 전용: 휘두르기를 시작한다 — 시작 시각을 복제해 애님을 태우고, 이번 휘두르기의 히트 구간 상태(맞은 목록·이전 틱
	 * 히트박스)를 연다. 실제 타격은 `HostTickSwingHit`이 구간 동안 매 틱 스윕해서 낸다 (docs/systems/inventory.md 사용).
	 */
	void HostSwing();

	/** 호스트에서 쓴 헤드램프를 켜고 끈다 (아이템 상태는 리플리케이트됨 — light.md) */
	UFUNCTION(Server, Reliable)
	void Server_Flashlight();

	UFUNCTION()
	void OnRep_CharacterAlias();

	/** 로컬 레지스트리에서 행을 읽어와 판정용 수치와 표시를 적용한다 */
	void ApplyCharacterDef();

	/** 소유 클라이언트 전용: 스태미나가 규칙 임계값 아래로 떨어지면 카메라가 흔들리고, 비워질수록 더 심해진다 (docs/systems/character.md) */
	void UpdateStaminaShake();

	/**
	 * 모든 머신, 매 틱: 잡힌 뒤 `HurtFlashTime` 동안 몸(과 1인칭 팔)에 빨간 오버레이를 씌우고 지나면 벗긴다 —
	 * 남의 화면에서 "쟤가 맞았다"를 알리는 표현이다 (docs/systems/character.md 피격).
	 */
	void UpdateHurtFlash();

	/**
	 * 소유 클라이언트(리슨 호스트의 자기 폰 포함), 매 틱: 복제된 타격·피격 시각이 지난 틱과 달라졌으면 그때마다
	 * 셰이크를 한 번씩 재생한다. OnRep을 안 쓰는 이유는 리슨 호스트가 권한이자 소유자라 자기 화면에서는
	 * OnRep이 뜨지 않기 때문이다 — 틱 비교는 호스트와 클라 양쪽에서 똑같이 작동한다.
	 */
	void UpdateHitShakes();

	/** 셰이크 표의 행 하나를 세기 1로 한 번 재생한다 — 행이나 애셋이 없으면 조용히 지나간다 */
	void PlayShakeOnce(FName RowName);

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

	/** 줍기/상호작용 액션 (E) — 놓기는 여기 없다 */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> PickPlaceAction;

	/** 놓기 액션 (G, IA_Drop) — 달리기 키와 함께 누르면 던지기 차징. 던지기는 이 키를 따라 리바인딩된다 */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> DropAction;

	/** 퀵슬롯 칸 지정 액션들 (IA_Slot1..4) — 배열 순서가 칸 번호다 */
	UPROPERTY(EditAnywhere, Category="Input")
	TArray<TObjectPtr<UInputAction>> SlotActions;

	/** 퀵슬롯 칸 스크롤 액션 (IA_SlotScroll, Axis1D 마우스 휠) — 리바인딩 대상이 아니다 */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> SlotScrollAction;

	/** 사용 액션 (마우스 좌클릭, IA_Use) — 누르면 공격 대기·떼면 휘두르기 / 손전등은 즉시 토글 (행의 UseAction) */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> UseAction;

	/**
	 * 스캔 액션 (마우스 우클릭, IA_Scan) — 탭은 스캔(`ACL9PlayerController::RequestScan`), `UseHoldTime` 이상 길게
	 * 누르면 활성 칸의 요리를 먹는다. 2026-09-13부터 PC가 아니라 여기서 바인딩한다 — 길게/탭을 가르는 것이
	 * 캐릭터 입력이기 때문이다. BP 자식에서 지정한다.
	 */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> ScanAction;

	/** 헤드램프 토글 액션 (F) */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> FlashlightAction;

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

	/** HUD에 마지막으로 전달한 프롬프트의 원문 패턴(키 서식 전) — 달라질 때만 OnFocusPromptChanged가 발동한다 */
	FText LastFocusPrompt;

	/**
	 * 서식 전의 프롬프트 — "[{0}] 줍기"의 String Table 원문. GetFocusPrompt가 출구에서 현재 키로 채운다.
	 * ICL9Interactable::GetPrompt도 원문 패턴을 돌려준다 (docs/systems/input.md 키 표기)
	 */
	FText GetFocusPromptPattern() const;

	/** 줍기 키의 현재 표기(캐시) — 리바인딩이 바뀌면 유저 설정 델리게이트가 다시 채운다. 틱마다 프로필을 뒤지지 않는다 */
	FText PickPlaceKeyText;

	/** 소유 클라이언트: 키 표기를 다시 읽고, 프롬프트가 새 키로 다시 나가도록 마지막 패턴을 비운다 */
	void RefreshKeyText();

	UFUNCTION()
	void HandleUserSettingsChanged(UEnhancedInputUserSettings* Settings);

	/** 호스트는 PC BeginPlay(IMC 등록)보다 빙의가 먼저라 첫 조회가 비어 있다 — 등록이 끝나면 다시 읽는다 */
	UFUNCTION()
	void HandleMappingContextRegistered(const UInputMappingContext* Context);

	/**
	 * 호스트 전용: 넘어질 때 손에서 끊기는 것들 — 수레를 놓고, 던지기 차징과 휘두르기 대기를 취소한다.
	 * 짐은 건드리지 않는다: 넘어진 채로 차징·대기가 이어지면 안 되지만, 손에 든 것과 퀵슬롯은 그대로 남는다
	 * (2026-09-15 — 짐 흘림은 죽을 때만, docs/systems/character.md 피격·넘어짐).
	 */
	void HostCancelCarryActions();

	/** 호스트 전용: 손과 퀵슬롯 전 칸을 발밑에 흘린다 — 죽을 때만 부른다(동료가 시체 옆에서 주워 가게). 위의 취소도 함께 한다 */
	void HostDropEverything();

	/**
	 * 호스트 전용: 놓기(G) 순간 짐칸에 얹어 줄 수레 — PickRadius 안에서 짐칸 쪽을 보고 있는 수레.
	 * ResolveFocus는 쓸 수 없다: 손에 뭔가 든 채로는 수레의 CanInteract가 false라(프롬프트 규칙, cart.md)
	 * 조준 대상에서 빠진다. 그래서 같은 반경·같은 조준 내적으로 수레만 따로 찾는다.
	 */
	ACL9Cart* FindAimedCart() const;

	/**
	 * 호스트 전용: 손 자리에서 짐칸 지점으로 톡 얹는 초기 속도를 낸다. **짐칸 벽을 넘는 가장 낮은 호**다 —
	 * 벽을 지나는 지점에서 아이템 밑면이 벽 윗단 위에 있도록 비행 시간을 늘리고(2026-09-15 PIE B4: 더 낮은 호는
	 * 벽 윗단에 맞고 튕겨 나왔다), 그 시간에 두 점을 잇는 포물선의 초기 속도를 돌려준다. 보내는 힘이
	 * CargoLobMaxSpeed를 넘으면(짐칸이 멀다) false — 호출자가 손 자리 낙하로 되돌아간다.
	 */
	bool ComputeCargoLobVelocity(const ACL9Cart& Cart, const ACL9Item& Item, const FVector& Origin, FVector& OutVelocity) const;

	/** 호스트 전용: 아이템을 그 칸에 기록하고 그 칸을 활성으로 해 손에 붙인다 — 손은 비어 있어야 한다 */
	void HostPickUp(ACL9Item* Item, int32 Slot);

	/** 호스트 전용: Head 슬롯 아이템(헤드램프)을 착용시키고 미러링한다 — E 한 번, 손은 그대로 둔다 */
	void HostWear(ACL9Item* Item);

	/** 호스트 전용: 손에 든 액터를 파괴하고 활성 칸을 데이터로만 남긴다 (칸 전환의 앞 절반) */
	void HostStowHeld();

	/** 호스트 전용: 그 칸을 활성으로 하고, 들어 있으면 저장된 값 그대로 다시 스폰해 손에 붙인다 (칸 전환의 뒤 절반) */
	void HostSelectSlot(int32 Slot, const UCL9RulesDef* Rules);

	/** 호스트 전용: 활성 칸의 미러를 비우고 HUD에 알린다 — 손의 액터를 놓고·던지고·먹고·실은 뒤 부른다 */
	void HostClearActiveSlot();

	/** 호스트 전용: 좌클 공격 대기를 취소한다 — 손의 것이 바뀌는 모든 경로(칸 전환·놓기·던지기·먹기·잡힘)가 부른다 */
	void HostCancelSwingWindup();

	/**
	 * 호스트 전용, 매 틱: 휘두르기 히트 구간이 열려 있으면 손에 든 것의 히트박스를 **이전 틱 자리 → 지금 자리**로 스윕해
	 * 닿은 몬스터마다 한 번씩 OnStrike를 낸다. 겹침이 아니라 스윕인 이유는 낮은 프레임률(PIE 자동화 3~8fps)에서 팬이
	 * 닭을 지나쳐 버리기 때문이다. 손이 비거나 바뀌면 구간이 닫힌다 (docs/systems/inventory.md 사용).
	 */
	void HostTickSwingHit();

	/**
	 * 매 틱, 호스트와 소유 클라이언트에서: 이동한 누적 거리가 행의 `StepDistance`를 넘을 때마다
	 * 발걸음(달리기/걷기) 소음과, 수레를 끌고 있으면 수레 소음을 낸다 (docs/systems/wave.md 소음 이벤트).
	 * 호스트는 `HostReportNoise`로 사건을 내고, 소유 클라이언트는 같은 걸음을 로컬에서 예측해 먼저 그린다 —
	 * 발걸음은 짧은 간격으로 나므로 RTT만큼 늦으면 링이 걸음과 어긋나 보인다.
	 */
	void TickStepNoise();

	/** 발걸음·착지·줍기 소음 하나 — 호스트면 사건으로, 소유 클라면 예측 재생으로 나간다 */
	void EmitNoise(ECL9NoiseSource Source, const FVector& Location, float Radius);

	/** 그 행의 `StepDistance`(cm), 행이 없으면 0 — 0이면 그 소음원은 걸음으로 나지 않는다 */
	float GetNoiseStepDistance(ECL9NoiseSource Source) const;

	/** 지금 E로 아이템을 주울 수 있는 상태인가 — 수레를 밀지 않고, 전환이 잠기지 않았고, 들어갈 칸이 있다 */
	bool CanPickUpItem() const;

	/** 주운 아이템이 들어갈 칸 — 활성 칸이 비었으면 활성 칸, 아니면 첫 빈 칸, 꽉 찼으면 INDEX_NONE (리썰 규칙) */
	int32 FindPickUpSlot() const;

	const UCL9RulesDef* GetRules() const;
	ACL9PlayerState* GetCL9PlayerState() const;
	float GetServerTime() const;

	/**
	 * 마지막으로 넘어진 뒤 지난 초 — `GetKnockdownElapsed()`와 달리 일어난 뒤에도 계속 커진다. 플래시·비네트는
	 * 넘어짐이 풀린 뒤까지 이어질 수 있어야 하고, 넘어진 적이 없으면(−1000) 아주 큰 값이 되어 저절로 꺼진다.
	 */
	float GetHurtElapsed() const;

	/** 이 폰이 연기할 캐릭터 행 — 스폰 시 서버가 정하고 모든 머신에 적용된다 */
	UPROPERTY(ReplicatedUsing=OnRep_CharacterAlias)
	FName CharacterAlias;

	/** alias가 가리키는 캐시된 행 (레지스트리가 살아있게 유지하며, 빠른 읽기를 위해 캐시한다) */
	UPROPERTY()
	TObjectPtr<const UCL9CharacterDef> CharacterDef;

	/** 활성 칸의 실물 — PlayerState.Slots[ActiveSlot]이 UI를 위해 이걸 미러링한다 (docs/systems/inventory.md) */
	UPROPERTY(Replicated)
	TObjectPtr<ACL9Item> HeldItem;

	UPROPERTY(Replicated)
	TObjectPtr<ACL9Cart> PullingCart;

	/** 머리에 착용한 아이템 액터 (Head 소지 슬롯) — PlayerState.HeadItem이 이걸 미러링한다 */
	UPROPERTY(Replicated)
	TObjectPtr<ACL9Item> WornItem;

	/** 현재 던지기 차징이 시작된 호스트 시각; 대기 중이면 음수. 복제는 남이 보는 대기 자세(AN_Farmer_ThrowWindup)용 — 판정은 호스트 값만 쓴다 */
	UPROPERTY(Replicated)
	float ThrowChargeStartTime = -1.f;

	/** 마지막으로 던진(놓은) 서버 시각 — GetThrowElapsed의 원천. 음수 큰 값 = 아직 없음 */
	UPROPERTY(Replicated)
	float ThrowServerTime = -1000.f;

	/** 같은 차징에 대한 소유 클라이언트의 로컬 시각 (게이지 전용, 판정에는 쓰이지 않음); 대기 중이면 음수 */
	float LocalThrowChargeStartTime = -1.f;

	/** 소유 클라이언트: 달리기 키가 눌려 있는 동안 true — G가 놓기가 아니라 던지기 차징이 되는 조건 */
	bool bRunHeld = false;

	/** 소유 클라이언트: 마지막으로 휠 전환을 보낸 로컬 시각 — 한 노치에 한 칸만 넘기는 디바운스 기준 */
	float LastSlotScrollTime = -1.f;

	/** 다음 휘두르기가 거부되는 기준이 되는 호스트 시각 (휘두른 뒤 회복 시간) */
	float SwingReadyTime = 0.f;

	/** 마지막 휘두르기의 서버 시각 — GetSwingElapsed의 원천. 음수 큰 값 = 아직 없음 */
	UPROPERTY(Replicated)
	float SwingStartServerTime = -1000.f;

	/** 호스트 전용: 히트 구간이 열려 있는 휘두르기를 시작한 아이템 — null이면 판정할 것이 없다(구간이 닫혔거나 손이 바뀌었다) */
	TWeakObjectPtr<ACL9Item> SwingHitItem;

	/** 호스트 전용: 이번 휘두르기에서 이미 맞은 몬스터들 — 한 번 휘두를 때 같은 몬스터는 한 번만 맞는다. 휘두르기 시작에 비운다 */
	TSet<TWeakObjectPtr<ACL9Monster>> SwingHitMonsters;

	/** 호스트 전용: 직전 틱의 히트박스 중심(월드) — 이번 틱 중심까지 스윕해 틱 사이를 메운다 */
	FVector SwingHitPrevCenter = FVector::ZeroVector;

	/** 호스트 전용: 직전 틱의 SwingElapsed, 음수면 이번이 이 휘두르기의 첫 틱이다(옛 자세에서 스윕하지 않는다) */
	float SwingHitPrevElapsed = -1.f;

	/**
	 * 이번 휘두르기가 **처음으로** 살아 있는 몬스터를 맞힌 서버 시각 — 공격자 카메라 킥의 원천이며 한 휘두르기에 한 번만
	 * 갱신된다(헛스윙·시체 타격에는 찍히지 않는다). 남의 화면에는 쓸 데가 없어 COND_OwnerOnly로 보낸다.
	 * 음수 큰 값 = 아직 없음.
	 */
	UPROPERTY(Replicated)
	float SwingHitServerTime = -1000.f;

	/** 좌클을 눌러 공격 대기를 시작한 서버 시각 — GetSwingWindupElapsed의 원천. 음수 큰 값 = 대기 중 아님 */
	UPROPERTY(Replicated)
	float SwingWindupServerTime = -1000.f;

	/** 마지막으로 먹은 서버 시각 — GetEatElapsed의 원천. 음수 큰 값 = 아직 없음 */
	UPROPERTY(Replicated)
	float EatStartServerTime = -1000.f;

	/** 우클 길게 누르기(먹기) 판정 타이머 (소유 클라 로컬) — 만료 전에 떼면 탭 = 스캔이다 */
	FTimerHandle EatHoldTimer;

	/** 소유 클라이언트: 이번 우클 누름이 먹기 문턱을 넘겼다 — 뗄 때 스캔을 내보내지 않는 조건 */
	bool bEatHoldReached = false;

	/**
	 * 넘어짐 상태가 캐릭터를 붙들어두는 서버 시각 (0이면 일어난 상태). CartMult처럼
	 * CMC가 읽는 리플리케이트 값이다 — 양쪽 모두 같은 값에서 속도 0을 도출하므로
	 * 보정 폭이 작게 유지된다 (docs/systems/character.md
	 * 넘어짐 이동 차단 — CMC 최대 속도 0 방식 선택, 2026-09-03).
	 */
	UPROPERTY(Replicated)
	float KnockdownEndServerTime = 0.f;

	/** 마지막으로 넘어진 서버 시각 — GetKnockdownElapsed의 원천(끝 시각만으로는 죽음(끝 = 무한)의 경과를 못 구한다). 음수 큰 값 = 아직 없음 */
	UPROPERTY(Replicated)
	float KnockdownStartServerTime = -1000.f;

	/**
	 * 지금 걸려 있는 먹기 효과들 — 복제된다. CMC가 이동 공식에서 읽으므로 서버와 자율
	 * 프록시가 같은 값을 봐야 한다(넘어짐의 종료 시각과 같은 이유).
	 */
	UPROPERTY(Replicated)
	TArray<FCL9ActiveEffect> ActiveEffects;

	// -- 타격감·피격감 표현 상태 (복제 없음, 머신마다 따로 굴러간다) --

	/** 규칙 애셋에서 한 번 로드해 둔 피격 플래시 머티리얼(몬스터와 같은 것) — 행이 비었으면 null로 남는다 */
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> HurtFlashMaterial;

	/** 플래시 머티리얼을 이미 한 번 로드해 봤는가 */
	bool bHurtFlashLoaded = false;

	/** 지금 피격 오버레이가 켜져 있는가 — 내가 켠 것만 끈다(줍기 하이라이트 등 남의 오버레이를 건드리지 않는 가드) */
	bool bHurtFlashOn = false;

	/** 셰이크 비교 기준값을 한 번은 그냥 받아 적었는가 — 접속 직후 이미 지나간 사건으로 화면을 흔들지 않는다 */
	bool bHitShakesPrimed = false;

	/** 지난 틱에 본 SwingHitServerTime — 달라지면 타격 셰이크를 한 번 재생한다 */
	float LastSeenSwingHitTime = -1000.f;

	/** 지난 틱에 본 KnockdownStartServerTime — 달라지면 피격 셰이크를 한 번 재생한다 */
	float LastSeenKnockdownTime = -1000.f;

	// -- 소음(파동) 상태: 호스트 또는 소유 클라이언트에서만 굴러간다 (docs/systems/wave.md) --

	/** 걸음 누적을 잰 직전 위치 — 아직 재기 시작하지 않았으면 미설정 */
	TOptional<FVector> LastStepLocation;

	/** 발걸음 행(`Run`/`Walk`)의 누적 이동 거리(cm) — 행이 바뀌면 0으로 되돌린다 */
	float StepDistanceAccum = 0.f;

	/** 지난 틱의 발걸음 행 — 달리기와 걷기는 `StepDistance`가 달라 누적을 함께 쓸 수 없다 */
	ECL9NoiseSource LastStepSource = ECL9NoiseSource::None;

	/** 수레 행의 누적 이동 거리(cm) — 발걸음과 따로 잰다(간격이 다를 수 있다) */
	float CartStepDistanceAccum = 0.f;

	/** 호스트 전용: 마지막으로 달리는 것이 보인 서버 시각 — 줍기가 조용한지 시끄러운지(`RecentRunTime`)를 가른다 */
	float LastRunServerTime = -1000.f;

	/** 눈높이에서 캡슐에 붙어 있는 1인칭 카메라 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	/**
	 * 1인칭 팔 메시(뷰모델). 소유자 화면에서는 손에 든 아이템의 메시가 여기의 손 소켓 `HandGrip_R`에
	 * 옮겨 붙는다(`ACL9Item::UpdateFirstPersonView`). **카메라의 자식**이라 시선 피치·요를 통째로 따른다
	 * (리썰 방식, 2026-09-13 — 옛 "전신 메시 자식 + 몸 포즈 복사(ABP_FP_Copy) + CtrlRig_FPWarp"는 폐기).
	 * 상대 트랜스폼은 `(0,0,-EyeHeight)`·요 −90°로 전신 마네킹과 같은 규격(리그 root가 원점, 눈이
	 * (0,0,EyeHeight)에서 +X를 봄) — 팔 ABP(`ABP_FP_View`)는 이 공간에서 저작한 카메라 로컬 클립만
	 * 재생하고 몸 포즈는 보지 않는다. 메시·ABP는 캐릭터 행(`ArmsMesh`·`ArmsAnimBP`)에서 온다.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;
};
