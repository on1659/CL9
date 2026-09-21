// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/CL9DefBase.h"
#include "CL9RulesDef.generated.h"

class UMaterialInterface;
class UNiagaraSystem;

/**
 * 유일한 전역 규칙 애셋 — 어떤 캐릭터도 다르게 가질 수 없는 값들이다: 소음,
 * 죽음, 밤 시계, 소지 마찰 (docs/plan/data/rules.md). 기본값은 문서화된 표를
 * 그대로 반영하며, 미정 컬럼은 결정되기 전까지 0으로 둔다.
 */
UCLASS()
class UCL9RulesDef : public UCL9DefBase
{
	GENERATED_BODY()

public:
	virtual void Validate(TArray<FString>& OutErrors) const override;

	// -- 소리 (몬스터가 이 반경 안에서 듣는다 — build-order ③에서 사용) --

	/** 달릴 때의 소음 반경, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float RunNoiseRadius = 390.f;

	/** 걷는 중에 줍는 소음 반경, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float PickupQuietRadius = 150.f;

	/** 달린 직후에 줍는 소음 반경, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float PickupLoudRadius = 390.f;

	/** 밀치기, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float PushNoiseRadius = 255.f;

	/** 미끼, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float DecoyRadius = 330.f;

	/** 이 시간 안에 달린 적이 있으면 줍기가 시끄러운 것으로 간주된다, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sound")
	float RecentRunTime = 1.f;

	// -- 죽음과 회복 --

	/** 죽은 뒤 팀원 관전으로 전환되기까지의 시간, 초 단위 — 초안 3, 플레이하며 조정 (docs/plan/data/rules.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float SpectateDelay = 3.f;

	/** 트럭 패드에 서서 HP가 다시 차기까지의 시간, 초 단위 — 초안 5, 플레이하며 조정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float TruckPadHealTime = 5.f;

	/** 이탈자의 폰이 남아있는 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float LeaverGraceTime = 3.f;

	/** 이 반경 안에서는 위협이 절대 깨어나거나 스폰되지 않는다, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float ThreatActMinRadius = 420.f;

	/**
	 * 처형 진입 때 피해자 시점이 몬스터의 처형 카메라로 넘어가는 블렌드 시간, 초 단위 — 0 이면 즉시. 관전과 같은
	 * `SetViewTarget` 경로에 `SetViewTargetWithBlend` 로 얹는다 (docs/systems/character.md 처형). 값은 balance
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float ExecuteCameraBlend = 0.f;

	/**
	 * 데스캠 자리 — 죽은 순간부터 관전 전까지(혼자면 밤이 끝날 때까지) 자기 시체를 3인칭으로 보는 카메라의 **액터 공간** 오프셋,
	 * cm 단위(X = 캐릭터 전방, Y = 오른쪽, Z = 위 — 처형 카메라와 같은 규약). **(0,0,0) 이면 데스캠 없음** = 옛 동작(넘어짐 1인칭
	 * 카메라가 바닥으로, 처형이면 몬스터 카메라에 머묾). (−320, 140, 260) = 뒤·옆·위 — PIE 에서 누운 몸 전체·옆의 돼지가 잡히는 값(2026-09-21), 아트 튜닝 (docs/systems/character.md 죽음)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	FVector DeathCameraOffset = FVector(-320.f, 140.f, 260.f);

	/** 데스캠이 바라보는 점 — 캡슐 중심(액터 위치) 기준 높이, cm 단위(음수 = 중심 아래). −60 = 누운 몸 높이(캡슐 반높이 −90 근처)라 시체가 화면 중앙에 온다 — 아트 튜닝 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float DeathCameraLookAtZ = -60.f;

	/**
	 * 죽는 순간 몬스터의 처형 카메라에서 데스캠으로 넘어가는 블렌드 시간, 초 단위 — 0 이면 즉시(`SetViewTargetWithBlend(폰)`,
	 * `HostBeginExecuted` 의 짝). 보통 사망은 뷰 타깃이 이미 자기 폰이라 엔진이 블렌드하지 않는다 — 넘어짐 카메라에서 데스캠으로 즉시 전환
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Death")
	float DeathCameraBlend = 0.5f;

	// -- 밤 시계 --

	/** 밤 길이, 초 단위 — 시간이 다 되면 새벽 실패가 된다 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float NightLength = 245.f;

	/** 정전까지 걸리는 기본 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float DarkBase = 185.f;

	/** 항아리 반납 한 번이 정전 시각을 앞당기는 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float DarkAccel = 4.f;

	/** 가능한 가장 이른 정전 시각, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float DarkMin = 125.f;

	/** 새벽 화면 길이, 초 단위 — 미정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Night")
	float DawnLength = 0.f;

	// -- 던지기와 소지 --

	/** 퀵슬롯 칸 수 — 한 번에 들고 다닐 수 있는 아이템 수. 손에 실물로 들리는 것은 활성 칸 하나뿐이다 (docs/systems/inventory.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	int32 QuickSlots = 4;

	/** 달리며 소지하는 상태가 지속되어 흔들림 게이지가 가득 차기까지의 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ShakeFullTime = 3.f;

	/** 흔들림 게이지 회복 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ShakeDrainTime = 1.5f;

	/** 충전 0일 때의 투척 속도, cm/s 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ThrowMinSpeed = 600.f;

	/** 완전 충전 시의 투척 속도, cm/s 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ThrowMaxSpeed = 1680.f;

	/** 완전 충전까지 걸리는 시간, 초 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float ThrowChargeTime = 0.85f;

	/** 칸에서 꺼내 스폰하는 높이, cm 단위 — 놓기에는 안 쓴다(놓기는 손 자리, 2026-09-14) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float CarryHeight = 120.f;

	/** 줍기 사거리, cm 단위 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float PickRadius = 120.f;

	/**
	 * 우클 길게 누르기 문턱, 초 단위 — 이 시간 안에 떼면 탭(스캔), 넘기면 활성 칸의 요리를 먹는다
	 * (2026-09-13 사용자: "우클 길게 = 먹기"). 소유 클라가 로컬 타이머로 잰다 (docs/systems/inventory.md 조작).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Carry")
	float UseHoldTime = 0.4f;

	// -- 휘두르기 히트 구간 (docs/plan/data/item/common.md 사용) --

	/**
	 * 휘두르기 히트 구간의 시작, 초 단위 — 좌클을 뗀 뒤(`GetSwingElapsed()`) 이 시각부터 손에 든 것의 히트박스가 몬스터를
	 * 맞힌다. 휘두르기 클립은 ABP가 `SwingElapsed + 0.12`로 평가해 `[0, 0.347)` 동안만 돌아가므로 구간은 그 안이어야 한다
	 * (`Validate`). 접촉 프레임에서 정한다 (docs/asset/character/farmer_a.md 026/031). 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Swing")
	float SwingHitStart = 0.05f;

	/** 휘두르기 히트 구간의 끝, 초 단위 — 이 시각부터는 맞지 않는다(구간은 반열림 `[Start, End)`). 초안값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Swing")
	float SwingHitEnd = 0.25f;

	/**
	 * 휘두르기 히트박스 반경 배율, 무단위 — 1 = 손에 든 메시의 경계 구 그대로(`ACL9Item::GetSwingHitSphere`). 호스트 스윕의
	 * 반경에만 곱하고 중심은 그대로다. 모든 `Swing` 행 공통이라 팬·고기가 같이 커진다. 0·음수·NaN 은 로드 오류(`Validate`).
	 * 값은 balance(docs/plan/data/rules.md)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Swing")
	float SwingHitScale = 1.f;

	// -- 타격감·피격감 (docs/systems/monster.md 플래시, docs/systems/character.md 피격) --

	/**
	 * 맞은 몬스터가 빨간 오버레이를 쓰고 있는 시간, 초 단위 — 가장 짧은 경직(`HitStunMs` 300ms)의 절반이라 색이
	 * 먼저 꺼진다: 빨강은 맞는 순간, 젖혀진 자세는 경직을 말한다. 모든 머신이 복제된 상태 시각(`GetStateElapsed`)에서
	 * 파생한다. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeedback")
	float HitFlashTime = 0.15f;

	/**
	 * 잡힌 사람의 몸(과 소유자의 1인칭 팔)이 빨간 시간, 초 단위 — `HurtStun` 0.35초 비틀거림을 덮고 일어서기가
	 * 시작되는 0.9초 전에 꺼진다. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeedback")
	float HurtFlashTime = 0.45f;

	/**
	 * 잡힌 사람 화면의 빨간 비네트가 사라지기까지, 초 단위 — 넘어짐 경과에 따라 1 → 0으로 선형으로 옅어진다.
	 * `KnockdownTime` 1.5초보다 0.3초 길어서 일어설 때쯤 화면도 갠다. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeedback")
	float HurtVignetteTime = 1.8f;

	/**
	 * 위 두 플래시가 메시에 덧씌우는 오버레이 머티리얼 — 애셋 경로는 C++에 없다(conventions § 5).
	 * 비어 있으면 플래시는 조용히 꺼지고 나머지 연출(카메라 킥·비네트)만 남는다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeedback")
	TSoftObjectPtr<UMaterialInterface> HitFlashMaterial;

	// -- 피격 카메라 (docs/systems/character.md 피격 — 잡는 순간 카메라가 범인을 본다) --

	/**
	 * 잡힌 사람의 카메라가 `HurtLookPitch` 까지 내려가는 데 걸리는 시간, 초 단위 — 넘어진 순간부터 EaseOut 으로 도달을
	 * 보장한다(`UCL9HurtLookModifier`, POV 오프셋만 — 컨트롤 회전은 그대로). **0 이면 이 효과 전체가 꺼진다.**
	 * 로드 검사: `HurtLookTime + 일어나기 0.6초 <= CharacterDef.KnockdownTime` 이어야 진입·복귀 보간이 겹치지 않는다(어기면 Warning). 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeedback")
	float HurtLookTime = 0.45f;

	/** 넘어진 동안 POV 에 더하는 피치 오프셋, 도 단위 — 음수가 아래. 발밑의 닭이 화면 중앙 아래에 오는 값. 초안값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeedback")
	float HurtLookPitch = -25.f;

	/**
	 * 공격자 방향으로 POV 요를 돌리는 상한, 도 단위 — 공격자가 화면 가장자리에 걸칠 때까지만 돌고 이 값을 넘지 않는다.
	 * 가로 FOV 90° + 60° = 시선에서 105° 안의 공격자만 화면에 걸친다(그 밖은 방향 호가 답한다). 0 이면 요는 돌지 않는다. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeedback")
	float HurtLookYawMax = 60.f;

	/** HUD 가장자리의 피격 방향 호를 그리나 — 장르 톤이 맞지 않으면 빌드 없이 끈다 (docs/systems/ui.md) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeedback")
	bool bHurtDirectionIndicator = true;

	// -- 타격의 표현 (2026-09-18, docs/goal/game-feel.md §2·§3, docs/systems/monster.md 타격의 표현) --

	/**
	 * 히트스톱, 초 단위 — 맞은 몬스터의 메시가 Stun·Dead 진입 뒤 이 시간 동안 애니메이션을 멈추고(모든 머신,
	 * `bPauseAnims`), 때린 사람의 1인칭 팔은 `SwingHitServerTime` 뒤 같은 시간 동안 그 프레임에 머문다
	 * (`GetSwingAnimElapsed`, 소유자 화면). 이동·AI·히트박스는 멈추지 않는다 — 전역 시간 지연은 쓰지 않는다.
	 * 0.08 = `HitFlashTime` 0.15 안에 끝나고(정지 ⊂ 빨강), 히트박스 창 폭 0.20 의 절반 아래. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeel")
	float HitStopTime = 0.08f;

	/**
	 * 죽는 타격에 몸이 날아가는 수평 속도, cm/s 단위 — 방향은 타격 원점(`Die(Source)`) → 몬스터. 호스트가 넉백과 같은
	 * 방식으로 매 틱 적분하고(수직은 월드 중력 980) 트랜스폼 복제로 퍼진다. 0이면 제자리에서 죽는다.
	 * 300 = 체공 2·260/980 = 0.53초 동안 수평 159cm — 복도 한 칸(500cm)에서 가로로 맞아도 벽 앞에 눕는다. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeel")
	float DeathLaunchSpeed = 300.f;

	/**
	 * 죽음 발사의 위쪽 초기 속도, cm/s 단위 — 월드 중력(`GetGravityZ`)이 끌어내려 포물선을 그린다.
	 * 260 = 정점 34.5cm ≈ 닭 몸통(`Radius` 36), 체공 0.53초라 죽음 클립이 나는 동안 보인다. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeel")
	float DeathLaunchUp = 260.f;

	/**
	 * 죽을 때 흩어지는 드랍 아이템의 발사 속도, cm/s 단위 — 몬스터 중심 높이·옆 30~90cm 자리에서 앙각 45~70°·요 무작위로
	 * `OnThrown`. 이후는 아이템 물리·멈춤 규칙 그대로다. 0이면 던지지 않고 놓인다.
	 * 300 = 계란 표 중력 1950·앙각 70°·낙차 15cm 에서 착지 371 < `CrackSpeed` 450 — 평지에서 금이 가지 않는다. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeel")
	float DeathLootSpeed = 300.f;

	/**
	 * 폰이 달릴 때 발소리 한 번의 보폭, cm 단위 — 수평 이동 거리를 누적해 이 값마다 `Footstep` 행을 발밑에서 한 번 튼다.
	 * 폰은 수평 속도가 행의 `WalkSpeed`를 넘으면 달리기 보폭, 몬스터는 Chase 상태에서 달리기 보폭.
	 * 196 = `RunSpeed` 654 × 0.3초(`AN_Farmer_FPRun` 0.6초 루프 = 두 걸음) — 팔 펌프마다 한 발. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeel")
	float FootstepStrideRun = 196.f;

	/**
	 * 걸을 때 발소리 한 번의 보폭, cm 단위 — 0이면 발소리가 나지 않는다.
	 * 150 = `WalkSpeed` 396 에서 2.64 걸음/초(0.38초 간격) — 달리기 0.30초와 케이던스만으로 갈린다. 초안값
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeel")
	float FootstepStrideWalk = 150.f;

	/**
	 * 타격 퍼프 나이아가라 — Stun·Dead 진입 에지(타격 플래시·`Hit` 소리와 같은 자리)에 몬스터 메시 중심(`spine` 소켓,
	 * 없으면 경계 중심)에서 스폰한다. `User.Scale`(float)을 Dead 는 3.0, 아니면 1.0 으로 준다. 비어 있으면 건너뛴다 —
	 * 애셋 경로는 C++에 없다(conventions § 5).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitFeel")
	TSoftObjectPtr<UNiagaraSystem> HitPuffSystem;
};
