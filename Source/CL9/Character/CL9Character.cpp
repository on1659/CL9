// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/CL9Character.h"
#include "Internationalization/StringTableRegistry.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/PlayerCameraManager.h"
#include "Character/CL9CharacterMovement.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedPlayerInput.h"
#include "Engine/Engine.h"
#include "Engine/HitResult.h"
#include "Engine/OverlapResult.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "TimerManager.h"
#include "CL9.h"
#include "Core/CL9InputText.h"
#include "Core/CL9Interactable.h"
#include "Data/CL9BuffDef.h"
#include "Data/CL9CharacterDef.h"
#include "Data/CL9Constants.h"
#include "Data/CL9DefRegistry.h"
#include "Data/CL9ItemDef.h"
#include "Data/CL9MonsterDef.h"
#include "Data/CL9RulesDef.h"
#include "Data/CL9ShakeTable.h"
#include "Data/CL9SoundTable.h"
#include "Data/CL9WaveTable.h"
#include "FX/CL9WaveRenderer.h"
#include "GameMode/CL9GameInstance.h"
#include "GameMode/CL9GameMode.h"
#include "GameMode/CL9GameState.h"
#include "GameMode/CL9GameUserSettings.h"
#include "GameMode/CL9HurtLookModifier.h"
#include "GameMode/CL9PlayerController.h"
#include "GameMode/CL9PlayerState.h"
#include "Items/CL9Cart.h"
#include "Items/CL9Item.h"
#include "Items/CL9FlashItem.h"
#include "Monsters/CL9Monster.h"

namespace
{
	/** focus 후보가 되려면 최소한 이만큼은 시야 안(조준 방향과의 내적)에 들어와 있어야 한다 */
	constexpr float FocusMinAimDot = 0.3f;

	/** 휠 전환 사이 최소 간격, 초 단위 — Axis Triggered가 겹쳐 와도 한 노치에 한 칸만 넘긴다 (입력 처리 상수, 밸런스 값이 아니다) */
	constexpr float SlotScrollDebounce = 0.1f;

	/** 복제되는 "서버 시각" 필드의 비어 있음 값 — 서버 시각은 0 이상이라 음수면 "아직 없음/대기 중 아님"이다 */
	constexpr float ServerTimeNever = -1000.f;

	/**
	 * 짐칸 놓기(수레를 보고 G)의 호 상수 — 표 열이 아니라 조작 상수다 (docs/plan/data/item/cart.md 적재).
	 * 목표가 손보다 높을 때만 쓰는 여유 정점(cm). 짐칸이 손보다 낮은 보통의 경우에는 벽 넘기가 정점을 정한다.
	 */
	constexpr float CargoLobApexRise = 10.f;

	/**
	 * 짐칸 벽을 넘을 때 아이템 **밑면**이 벽 윗단 위로 떠야 하는 여유(cm) — 호는 이 여유를 지키는 가장 낮은 호다.
	 * 8cm인 이유: 벽 윗단으로 삼는 `CargoVolume` 윗면이 실제 벽 윗단보다 3cm쯤 낮게 잡혀 있고(2026-09-15 PIE 실측
	 * 89 vs 상자 86), 그 차이를 흡수하고도 5cm가 남아야 한다.
	 */
	constexpr float CargoLobWallClearance = 8.f;

	/**
	 * 짐칸 놓기의 수평 속도 절대 상한(cm/s) — 얹는 동작이지 던지기가 아니다. 이보다 세게 보내야 닿는 거리면
	 * 손 자리 낙하로 되돌아간다. **되돌아가는 조건은 이것 하나뿐이다** (2026-09-15 사용자: 깨짐을 기준으로 한
	 * CrackSpeed 상한은 폐기 — 놓기에서 계란에 금이 가는 것은 규칙으로 받아들인다).
	 */
	constexpr float CargoLobMaxSpeed = 600.f;

	/**
	 * 휘두르기 판정 원점이 눈높이보다 낮은 만큼, cm 단위 — 눈 156 − 66 = 손 높이 ≈ 90(캡슐 중심). 눈높이 원점이면 수평 조준에서
	 * 닭 중심(36)까지 높이차 ~120cm 라 붙어 선 닭을 놓친다. 농부 A 리그의 물리 치수라 표 열이 아니다 (docs/goal/combat-feel-pass-1.md D-1)
	 */
	constexpr float SwingHandDrop = 66.f;
}

bool ACL9Character::bCheatSwingLog = false;

ACL9Character::ACL9Character(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCL9CharacterMovement>(ACharacter::CharacterMovementComponentName))
{
	GetCapsuleComponent()->InitCapsuleSize(CL9::CapsuleRadius, CL9::CapsuleHalfHeight);

	// 카메라는 눈높이에 위치하며, 1인칭 메시는 소유자에게만 보인다
	// (docs/systems/character.md "시점 — 1인칭").
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.f, 0.f, CL9::EyeHeight - CL9::CapsuleHalfHeight));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// 다른 플레이어에게 보이는 것은 전신 메시다. 이것 자체의 트랜스폼은
	// 표준 마네킹 규격을 따른다 (발이 캡슐 바닥에, +X 방향을 바라봄).
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -CL9::CapsuleHalfHeight), FRotator(0.f, -90.f, 0.f));

	// 1인칭 팔은 카메라의 자식(뷰모델)이다 — 시선 피치·요를 통째로 따라간다(리썰 방식, 2026-09-13).
	// 상대 트랜스폼은 전신 마네킹과 같은 규격: 리그 root가 원점, 눈이 (0,0,EyeHeight)에서 +X를 보는
	// 공간 — 카메라가 눈이므로 root를 눈높이만큼 내리고 -90° 요를 준다. 팔 클립은 이 공간에서 저작한다.
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(FirstPersonCameraComponent);
	FirstPersonMesh->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -CL9::EyeHeight), FRotator(0.f, -90.f, 0.f));
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// 데스캠: 죽으면 1인칭 대신 이것이 시체를 본다 — 자리·회전은 죽는 순간 규칙 표에서(ApplyDeathCamera). 평소엔 꺼져 있어
	// CalcCamera 가 첫 활성 카메라인 1인칭을 고른다 (docs/systems/character.md 죽음 — 데스캠)
	DeathCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("DeathCamera"));
	DeathCamera->SetupAttachment(GetCapsuleComponent());
	DeathCamera->bAutoActivate = false;
	DeathCamera->SetActive(false);

	SetNetUpdateFrequency(20.f);
}

void ACL9Character::BeginPlay()
{
	Super::BeginPlay();

	// 호스트에서는 전신 메시의 본을 **화면에 없어도** 갱신한다. 든 아이템의 루트가 손 소켓에 붙어 있고 놓기(`Server_Drop`)가
	// "든 자리에서 그대로" `HeldItem->GetActorLocation()` 을 읽는데, ACharacter 기본값(AlwaysTickPose)은 포즈만 굴리고
	// 본 트랜스폼은 렌더될 때만 새로 쓴다 — 전신 메시는 자기 폰에서 OwnerNoSee이고 남의 폰은 화면 밖일 수 있어서, 그대로
	// 두면 호스트가 읽는 손 자리가 옛 자세에 얼어 있다. 휘두르기 판정은 2026-09-22부터 손 소켓을 읽지 않는다(컨트롤 회전 +
	// 손 높이 원점, HostTickSwingHit). 클라이언트는 표시만 하므로 기본값 그대로 둔다(성능). (docs/systems/inventory.md 놓기)
	if (HasAuthority())
	{
		GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}
}

void ACL9Character::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 폰이 사라지면 카메라 매니저에 남은 모디파이어를 뗀다 — 다음 폰이 죽은 폰의 사건을 읽지 않게
	if (HurtLookModifier != nullptr)
	{
		if (APlayerCameraManager* CameraManager = HurtLookModifier->GetCameraOwner())
		{
			CameraManager->RemoveCameraModifier(HurtLookModifier);
		}
		HurtLookModifier = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void ACL9Character::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	UpdateHurtLookModifier();
}

void ACL9Character::UpdateHurtLookModifier()
{
	// 소유 클라(리슨 호스트의 자기 폰 포함)에서만 — 남의 폰·서버의 원격 폰에는 카메라가 없다
	const APlayerController* PC = IsLocallyControlled() ? Cast<APlayerController>(GetController()) : nullptr;
	APlayerCameraManager* CameraManager = PC != nullptr ? PC->PlayerCameraManager : nullptr;

	// 소유가 풀렸거나 다른 카메라 매니저로 옮겨 갔으면 옛 등록을 뗀다
	if (HurtLookModifier != nullptr && HurtLookModifier->GetCameraOwner() != CameraManager)
	{
		if (APlayerCameraManager* OldManager = HurtLookModifier->GetCameraOwner())
		{
			OldManager->RemoveCameraModifier(HurtLookModifier);
		}
		HurtLookModifier = nullptr;
	}

	if (CameraManager != nullptr && HurtLookModifier == nullptr)
	{
		HurtLookModifier = Cast<UCL9HurtLookModifier>(CameraManager->AddNewCameraModifier(UCL9HurtLookModifier::StaticClass()));
		if (HurtLookModifier != nullptr)
		{
			HurtLookModifier->SetCharacter(this);
		}
	}
}

void ACL9Character::SetCharacterAlias(FName NewAlias)
{
	if (!HasAuthority())
	{
		return;
	}

	CharacterAlias = NewAlias;
	ApplyCharacterDef();

	// 개인의 HP는 호스트에서 한 번만 굴려진다 (docs/systems/character.md —
	// HP는 항상 PlayerState에 있고 캐릭터에는 없다).
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (CL9PlayerState != nullptr && CL9PlayerState->MaxHp == 0 && CharacterDef != nullptr)
	{
		CL9PlayerState->MaxHp = FMath::RandRange(CharacterDef->HpMin, CharacterDef->HpMax);
		CL9PlayerState->Hp = CL9PlayerState->MaxHp;
		CL9PlayerState->NotifyHpChanged();
	}
}

UCL9CharacterMovement* ACL9Character::GetCL9CharacterMovement() const
{
	return CastChecked<UCL9CharacterMovement>(GetCharacterMovement());
}

void ACL9Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACL9Character, CharacterAlias);
	DOREPLIFETIME(ACL9Character, HeldItem);
	DOREPLIFETIME(ACL9Character, PullingCart);
	DOREPLIFETIME(ACL9Character, SwingStartServerTime);
	// 표면음은 남의 폰에서도 들려야 한다 — 조건 없이(SwingHitServerTime 과 달리 OwnerOnly 가 아니다)
	DOREPLIFETIME(ACL9Character, SwingSurfaceHitServerTime);
	DOREPLIFETIME(ACL9Character, SwingWindupServerTime);
	DOREPLIFETIME(ACL9Character, EatStartServerTime);
	DOREPLIFETIME(ACL9Character, KnockdownEndServerTime);
	DOREPLIFETIME(ACL9Character, KnockdownStartServerTime);
	DOREPLIFETIME(ACL9Character, ThrowChargeStartTime);
	DOREPLIFETIME(ACL9Character, ThrowServerTime);
	DOREPLIFETIME(ACL9Character, WornItem);
	DOREPLIFETIME(ACL9Character, ActiveEffects);
	// 처형은 남의 화면(ABP)도 봐야 하고 CMC 가 속도 0 을 도출하므로 모두에게
	DOREPLIFETIME(ACL9Character, ExecuteStartServerTime);
	DOREPLIFETIME(ACL9Character, ExecutedBy);

	// 카메라 킥·피격 사건은 때린/맞은 본인만 쓴다 — 남의 폰에 보낼 이유가 없는 값이라 조건부다
	DOREPLIFETIME_CONDITION(ACL9Character, SwingHitServerTime, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ACL9Character, HurtEvent, COND_OwnerOnly);
}

void ACL9Character::OnRep_CharacterAlias()
{
	ApplyCharacterDef();
}

void ACL9Character::ApplyCharacterDef()
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	const UCL9CharacterDef* Def = Registry ? Registry->FindCharacter(CharacterAlias) : nullptr;

	if (Def == nullptr)
	{
		UE_LOG(LogCL9, Error, TEXT("'%s': character row '%s' is missing or broken — engine defaults stay (docs/data/schema.md 로드 검사)."),
			*GetNameSafe(this), *CharacterAlias.ToString());
		return;
	}
	CharacterDef = Def;

	// 피격 카메라의 진입(HurtLookTime)과 복귀(일어나기 0.6초)가 겹치면 카메라가 떤다 — 규칙 표와 캐릭터 행이 둘 다 보이는 자리라 여기서 검사한다
	if (const UCL9RulesDef* Rules = Registry->GetRules();
		Rules != nullptr && Rules->HurtLookTime > 0.f && Rules->HurtLookTime + CL9HurtLook::GetUpSeconds > Def->KnockdownTime)
	{
		UE_LOG(LogCL9, Warning, TEXT("CL9Rules.HurtLookTime %.2f + get-up %.2f exceeds character '%s' KnockdownTime %.2f — the hurt-look enter and exit blends overlap (docs/systems/character.md 피격)."),
			Rules->HurtLookTime, CL9HurtLook::GetUpSeconds, *CharacterAlias.ToString(), Def->KnockdownTime);
	}

	// 판정용 수치 — 모든 머신이 동일한 로컬 테이블 행을 적용하므로
	// 자율 프록시는 호스트가 판정할 때 쓰는 것과 같은 수치로 예측한다
	// (conventions § 5).
	GetCapsuleComponent()->SetCapsuleSize(Def->CapsuleRadius, Def->CapsuleHalfHeight);
	FirstPersonCameraComponent->SetRelativeLocation(FVector(0.f, 0.f, Def->EyeHeight - Def->CapsuleHalfHeight));
	// 팔(카메라 자식)의 root도 행의 눈높이만큼 내린다 — BP가 생성자 값을 덮어써도 여기서 다시 맞는다.
	FirstPersonMesh->SetRelativeLocation(FVector(0.f, 0.f, -Def->EyeHeight));

	UCL9CharacterMovement* Movement = GetCL9CharacterMovement();
	Movement->SetCharacterDef(Def);
	Movement->MaxWalkSpeed = Def->WalkSpeed;
	Movement->JumpZVelocity = Def->JumpVelocity;
	Movement->GravityScale = Def->JumpGravity / CL9::DefaultGravityZ;

	// 표시용 — 메시는 행에서 가져오므로 구매한 에셋을 C++ 없이 교체할 수 있다.
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -Def->CapsuleHalfHeight));
	if (USkeletalMesh* BodyMesh = Def->SkeletalMesh.LoadSynchronous())
	{
		GetMesh()->SetSkeletalMesh(BodyMesh);
	}
	if (UClass* BodyAnimClass = Def->AnimBP.LoadSynchronous())
	{
		GetMesh()->SetAnimInstanceClass(BodyAnimClass);
	}
	if (USkeletalMesh* ArmsMesh = Def->ArmsMesh.LoadSynchronous())
	{
		FirstPersonMesh->SetSkeletalMesh(ArmsMesh);
	}
	if (UClass* ArmsAnimClass = Def->ArmsAnimBP.LoadSynchronous())
	{
		FirstPersonMesh->SetAnimInstanceClass(ArmsAnimClass);
	}
}

void ACL9Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACL9Character::MoveInput);
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACL9Character::LookInput);
		EnhancedInput->BindAction(RunAction, ETriggerEvent::Started, this, &ACL9Character::RunInputStart);
		EnhancedInput->BindAction(RunAction, ETriggerEvent::Completed, this, &ACL9Character::RunInputEnd);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInput->BindAction(PickPlaceAction, ETriggerEvent::Started, this, &ACL9Character::PickPlaceInput);
		EnhancedInput->BindAction(DropAction, ETriggerEvent::Started, this, &ACL9Character::DropStartInput);
		EnhancedInput->BindAction(DropAction, ETriggerEvent::Completed, this, &ACL9Character::DropEndInput);
		// 숫자키 — 배열 순서가 칸 번호다. 휠은 Axis1D라 값이 있는 프레임마다 Triggered가 온다
		for (int32 Slot = 0; Slot < SlotActions.Num(); ++Slot)
		{
			if (SlotActions[Slot] != nullptr)
			{
				EnhancedInput->BindAction(SlotActions[Slot], ETriggerEvent::Started, this, &ACL9Character::SlotInput, Slot);
			}
		}
		EnhancedInput->BindAction(SlotScrollAction, ETriggerEvent::Triggered, this, &ACL9Character::SlotScrollInput);
		EnhancedInput->BindAction(FlashlightAction, ETriggerEvent::Started, this, &ACL9Character::FlashlightInput);
		EnhancedInput->BindAction(UseAction, ETriggerEvent::Started, this, &ACL9Character::UseStartInput);
		EnhancedInput->BindAction(UseAction, ETriggerEvent::Completed, this, &ACL9Character::UseEndInput);
		// 우클은 탭(스캔)/길게(먹기)를 캐릭터가 가른다 — PC는 RequestScan()만 내준다 (2026-09-13)
		if (ScanAction != nullptr)
		{
			EnhancedInput->BindAction(ScanAction, ETriggerEvent::Started, this, &ACL9Character::ScanStartInput);
			EnhancedInput->BindAction(ScanAction, ETriggerEvent::Completed, this, &ACL9Character::ScanEndInput);
		}
		else
		{
			UE_LOG(LogCL9, Error, TEXT("'%s': ScanAction is not set — right-click scan and eat are dead (BP_CL9Character.ScanAction → IA_Scan)."), *GetNameSafe(this));
		}

		// 프롬프트의 "[{0}]"에 들어갈 줍기 키 — 빙의 시 한 번 읽고, 이후는 리바인딩 변경 델리게이트가 갱신한다
		RefreshKeyText();
		if (UEnhancedInputUserSettings* UserSettings = CL9Input::GetUserSettings(Cast<APlayerController>(GetController())))
		{
			UserSettings->OnSettingsChanged.AddUniqueDynamic(this, &ACL9Character::HandleUserSettingsChanged);
			UserSettings->OnMappingContextRegistered.AddUniqueDynamic(this, &ACL9Character::HandleMappingContextRegistered);
		}
	}
	else
	{
		UE_LOG(LogCL9, Error, TEXT("'%s' expects an Enhanced Input component — legacy input is not supported (docs/systems/input.md)."), *GetNameSafe(this));
	}
}

void ACL9Character::MoveInput(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	if (GetController())
	{
		AddMovementInput(GetActorRightVector(), Axis.X);
		AddMovementInput(GetActorForwardVector(), Axis.Y);
	}
}

void ACL9Character::LookInput(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	// 처형 중에는 시선도 잠긴다 — 게임플레이 IMC 가 내려가 보통은 여기까지 오지 않지만, 내려가기 전 프레임의 입력을 막는다 (D2)
	if (IsExecuted())
	{
		return;
	}

	if (GetController())
	{
		// 감도·Y축 반전은 로컬 옵션이다 — AxisConfig 감도 위에 곱하는 배율일 뿐 조준 이득이 없다 (docs/systems/settings.md)
		float Sensitivity = 1.f;
		float PitchSign = 1.f;
		if (const UCL9GameUserSettings* Settings = Cast<UCL9GameUserSettings>(GEngine->GetGameUserSettings()))
		{
			Sensitivity = Settings->MouseSensitivity;
			PitchSign = Settings->bInvertY ? -1.f : 1.f;
		}
		AddControllerYawInput(Axis.X * Sensitivity);
		AddControllerPitchInput(Axis.Y * Sensitivity * PitchSign);
	}
}

void ACL9Character::RunInputStart()
{
	bRunHeld = true;
	GetCL9CharacterMovement()->SetWantsToRun(true);
}

void ACL9Character::RunInputEnd()
{
	bRunHeld = false;
	GetCL9CharacterMovement()->SetWantsToRun(false);
}

void ACL9Character::PickPlaceInput()
{
	Server_PickPlace();
}

void ACL9Character::DropStartInput()
{
	// 활성 칸이 비었으면 놓을 것도 던질 것도 없다 — RPC를 보내지 않는다
	if (GetHeldItem() == nullptr)
	{
		return;
	}

	// 달리기 키 + 놓기 키 = 던지기 차징. 조합은 고정이고 놓기 키만 리바인딩된다 (docs/systems/input.md 던지기 차징).
	// 차징 시각은 호스트가 잰다 — 로컬 시각은 HUD 게이지용이다.
	// 같은 틱에 두 키가 함께 눌리면 Enhanced Input이 Started 델리게이트를 바인딩 역순으로 쏘아 이 함수가 RunInputStart보다
	// 먼저 온다(bRunHeld가 아직 false) — 액션 값은 델리게이트 전에 전부 갱신돼 있으므로 달리기 액션 값을 직접 읽어 보완한다
	// (2026-09-14 리드 재현: 같은 틱 Run+Drop 주입이 차징되지 않음).
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	const UEnhancedPlayerInput* PlayerInput = PlayerController ? Cast<UEnhancedPlayerInput>(PlayerController->PlayerInput) : nullptr;
	const bool bRunPressedThisTick = PlayerInput != nullptr && RunAction != nullptr && PlayerInput->GetActionValue(RunAction).Get<bool>();
	if (bRunHeld || bRunPressedThisTick)
	{
		LocalThrowChargeStartTime = GetWorld()->GetTimeSeconds();
		Server_ThrowChargeStart();
		return;
	}
	Server_Drop();
}

void ACL9Character::DropEndInput()
{
	// 차징 없이 눌렀던 G(즉시 놓기)의 뗌은 아무 일도 아니다
	if (LocalThrowChargeStartTime < 0.f)
	{
		return;
	}
	LocalThrowChargeStartTime = -1.f;
	Server_ThrowRelease();
}

void ACL9Character::SlotInput(int32 Slot)
{
	// 잠금 판정은 호스트가 다시 한다 — 여기서는 헛 RPC만 거른다
	const ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (CL9PlayerState == nullptr || !CL9PlayerState->Slots.IsValidIndex(Slot) || !CanSwitchSlot())
	{
		return;
	}
	Server_SelectSlot(static_cast<uint8>(Slot));
}

void ACL9Character::SlotScrollInput(const FInputActionValue& Value)
{
	// 마우스 휠 축: 한 노치가 ±1.0이고 위가 +다. 아래(−) = 다음 칸(오른쪽), 위(+) = 이전 칸.
	const float Axis = Value.Get<float>();
	const ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	const int32 SlotCount = CL9PlayerState ? CL9PlayerState->Slots.Num() : 0;
	if (FMath::IsNearlyZero(Axis) || SlotCount <= 1 || !CanSwitchSlot())
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	if (LastSlotScrollTime >= 0.f && Now - LastSlotScrollTime < SlotScrollDebounce)
	{
		return;
	}
	LastSlotScrollTime = Now;
	Server_ScrollSlot(Axis < 0.f ? 1 : -1);
}

float ACL9Character::GetThrowChargeAlpha() const
{
	const UCL9RulesDef* Rules = GetRules();
	if (LocalThrowChargeStartTime < 0.f || Rules == nullptr || Rules->ThrowChargeTime <= 0.f)
	{
		return -1.f;
	}
	return FMath::Clamp((GetWorld()->GetTimeSeconds() - LocalThrowChargeStartTime) / Rules->ThrowChargeTime, 0.f, 1.f);
}

FText ACL9Character::GetFocusPrompt() const
{
	const FText Pattern = GetFocusPromptPattern();
	return Pattern.IsEmpty() ? Pattern : FText::Format(Pattern, PickPlaceKeyText);
}

void ACL9Character::RefreshKeyText()
{
	PickPlaceKeyText = CL9Input::GetActionKeyText(Cast<APlayerController>(GetController()), PickPlaceAction);
	// 같은 패턴이라도 키가 바뀌었으면 HUD가 다시 그려야 한다 — 다음 틱의 비교가 반드시 달라지게 비운다
	LastFocusPrompt = FText::GetEmpty();
}

void ACL9Character::HandleUserSettingsChanged(UEnhancedInputUserSettings* Settings)
{
	RefreshKeyText();
}

void ACL9Character::HandleMappingContextRegistered(const UInputMappingContext* Context)
{
	RefreshKeyText();
}

FText ACL9Character::GetFocusPromptPattern() const
{
	// Server_PickPlace와 같은 순서: 착용 가능 > 주울 수 있는 아이템 > 상호작용 대상 > 미는 수레 놓기.
	// "놓기" 프롬프트는 없다 — 놓기는 E가 아니라 G이고, 들고만 있을 때는 프롬프트가 비어 있다 (2026-09-13 사용자).
	AActor* Focus = FocusedActor.Get();
	if (ACL9Item* Wearable = Cast<ACL9Item>(Focus); Wearable != nullptr && Wearable->CanBePickedUp()
		&& Wearable->GetItemDef()->CarrySlot == ECL9CarrySlot::Head && WornItem == nullptr)
	{
		return LOCTABLE("CL9", "Prompt.Wear");
	}
	if (ACL9Item* Item = Cast<ACL9Item>(Focus); Item != nullptr && Item->CanBePickedUp()
		&& Item->GetItemDef()->CarrySlot != ECL9CarrySlot::Head && CanPickUpItem())
	{
		return LOCTABLE("CL9", "Prompt.PickUp");
	}
	if (ICL9Interactable* Interactable = Cast<ICL9Interactable>(Focus); Interactable != nullptr && Interactable->CanInteract(this))
	{
		return Interactable->GetPrompt(this);
	}
	if (Focus == nullptr && PullingCart != nullptr)
	{
		return LOCTABLE("CL9", "Prompt.CartRelease");
	}
	return FText::GetEmpty();
}

void ACL9Character::FlashlightInput()
{
	Server_Flashlight();
}

void ACL9Character::Server_Flashlight_Implementation()
{
	// F는 쓴 헤드램프만 — 든 손전등은 좌클(Server_UseStart)이 켠다 (2026-09-12). 정전 이후에는
	// 손전등 자체가 켜지길 거부한다 (docs/systems/light.md). 넘어졌거나 처형 중이면 손이 말을 듣지 않는다 (D2)
	if (IsIncapacitated())
	{
		return;
	}
	if (ACL9FlashItem* Lamp = Cast<ACL9FlashItem>(WornItem.Get()))
	{
		Lamp->ToggleLight();
	}
}

void ACL9Character::UseStartInput()
{
	// 누름·뗌을 그대로 호스트에 넘긴다 — 무엇을 할지(대기·토글·무동작)는 호스트가 든 행으로 정한다
	Server_UseStart();
}

void ACL9Character::UseEndInput()
{
	// 대기 중이었는지는 호스트만 안다(칸 전환으로 취소됐을 수 있다) — 뗌은 항상 보낸다
	Server_UseRelease();
}

void ACL9Character::ScanStartInput()
{
	// 문턱은 규칙 표의 UseHoldTime — 이 시간 안에 떼면 탭(스캔), 넘기면 먹기. 표가 없으면 타이머가 안 걸려 늘 탭이다
	bEatHoldReached = false;
	const UCL9RulesDef* Rules = GetRules();
	if (Rules != nullptr && Rules->UseHoldTime > 0.f)
	{
		GetWorldTimerManager().SetTimer(EatHoldTimer, this, &ACL9Character::EatHoldReached, Rules->UseHoldTime, false);
	}
}

void ACL9Character::ScanEndInput()
{
	GetWorldTimerManager().ClearTimer(EatHoldTimer);
	if (bEatHoldReached)
	{
		return;
	}
	// 탭 — 스캔은 표현이라 PC(HUD 응답)가 맡는다. 문턱까지 기다렸다 떼는 순간 나가므로 최대 UseHoldTime만큼 늦을 수 있다
	if (ACL9PlayerController* PC = Cast<ACL9PlayerController>(GetController()))
	{
		PC->RequestScan();
	}
}

void ACL9Character::EatHoldReached()
{
	bEatHoldReached = true;
	// 활성 칸이 Dish가 아니면 호스트가 조용히 무시한다 — 여기서 걸러도 되지만 판정은 호스트 것이다
	if (GetHeldItem() != nullptr)
	{
		Server_Eat();
	}
}

void ACL9Character::Server_UseStart_Implementation()
{
	const UCL9ItemDef* Def = HeldItem != nullptr ? HeldItem->GetItemDef() : nullptr;
	UE_LOG(LogCL9, Verbose, TEXT("UseStart: item=%s action=%d"), *GetNameSafe(HeldItem.Get()), Def ? static_cast<int32>(Def->UseAction) : -1);
	if (Def == nullptr || IsIncapacitated())
	{
		return;
	}

	switch (Def->UseAction)
	{
	case ECL9UseAction::ToggleLight:
		// 손전등은 대기가 없다 — 누르는 순간 켜고 끈다 (2026-09-13 기본값)
		if (ACL9FlashItem* Lamp = Cast<ACL9FlashItem>(HeldItem.Get()))
		{
			Lamp->ToggleLight();
		}
		break;
	case ECL9UseAction::Swing:
		// 공격 대기 — 무한이고 떼는 순간 휘두른다. 대기 시간은 결과에 영향이 없다 (리썰 삽 방식, 밸런스 열 없음)
		SwingWindupServerTime = GetServerTime();
		break;
	default:
		break;
	}
}

void ACL9Character::Server_UseRelease_Implementation()
{
	// 대기 중이 아니면(애초에 Swing이 아니었거나 칸 전환·놓기로 취소됐다) 뗌은 아무 일도 아니다
	if (SwingWindupServerTime < 0.f)
	{
		return;
	}
	HostCancelSwingWindup();

	if (!CanSwingNow())
	{
		return;
	}

	// 회복 중이면 거부 — 단, 회복 끝까지 SwingBufferTime 이하로 남았으면 하나 예약한다(HostTickSwingBuffer 가 낸다). 예약은
	// 하나뿐이고 덮어쓴다. 헤더 0 이면 예약 창이 없어 옛 동작(거부)이다 (docs/systems/character.md 입력 예약)
	const float Now = GetServerTime();
	if (Now < SwingReadyTime)
	{
		const UCL9RulesDef* Rules = GetRules();
		if (Rules != nullptr && SwingReadyTime - Now <= Rules->SwingBufferTime)
		{
			bSwingBuffered = true;
			SwingInputServerTime = Now;
		}
		else if (bCheatSwingLog)
		{
			UE_LOG(LogCL9, Log, TEXT("cl9.swing: rejected t=%.3f ready=%.3f"), Now, SwingReadyTime);
		}
		return;
	}
	SwingInputServerTime = Now;
	HostSwing();
}

const TCHAR* ACL9Character::HostCheatSwingTap()
{
	if (!HasAuthority())
	{
		return TEXT("lost");
	}
	// 누르고 바로 뗀다 — 대기 시간은 결과에 영향이 없다. 받아들인 입력은 SwingInputServerTime 이 지금이 되고, 그중 시작 시각도
	// 지금이면 즉시 휘두른 것, 아니면 예약이다. 같은 틱 안이라 서버 시각은 같다
	const float Now = GetServerTime();
	Server_UseStart_Implementation();
	Server_UseRelease_Implementation();
	if (SwingInputServerTime != Now)
	{
		return TEXT("lost");
	}
	return SwingStartServerTime == Now ? TEXT("swing") : (bSwingBuffered ? TEXT("buffered") : TEXT("lost"));
}

bool ACL9Character::CanSwingNow() const
{
	const UCL9ItemDef* Def = HeldItem != nullptr ? HeldItem->GetItemDef() : nullptr;
	return Def != nullptr && Def->UseAction == ECL9UseAction::Swing && !IsIncapacitated();
}

void ACL9Character::HostTickSwingBuffer()
{
	if (!bSwingBuffered || GetServerTime() < SwingReadyTime)
	{
		return;
	}
	// 예약은 여기서 소비된다 — 손이 바뀌어 낼 수 없으면 그냥 사라진다(HostCancelSwingWindup 이 보통 먼저 지우지만 이중 안전)
	bSwingBuffered = false;
	if (CanSwingNow())
	{
		HostSwing();
	}
}

void ACL9Character::Server_Eat_Implementation()
{
	// 먹을 수 있는 건 Dish뿐이고 별도 플래그는 없다 (plan/data/item/common.md). 프라이팬을 들고 우클을 오래
	// 누르는 건 흔한 일이라 거절 로그는 남기지 않는다 — HostEat의 "refused" 로그는 치트 경로 몫이다.
	const UCL9ItemDef* Def = HeldItem != nullptr ? HeldItem->GetItemDef() : nullptr;
	if (Def == nullptr || Def->ItemType != ECL9ItemType::Dish)
	{
		return;
	}
	HostEat();
}

AActor* ACL9Character::ResolveFocus() const
{
	const UCL9RulesDef* Rules = GetRules();
	if (Rules == nullptr)
	{
		return nullptr;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CL9Focus), false, this);
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity,
		ObjectParams, FCollisionShape::MakeSphere(Rules->PickRadius), QueryParams);

	const FVector AimDirection = GetControlRotation().Vector();
	const bool bCanPickUp = CanPickUpItem();
	AActor* BestItem = nullptr;
	float BestItemDot = -2.f;
	AActor* BestInteractable = nullptr;
	float BestInteractableDot = -2.f;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		// 밀고 있는 수레도 유효한 대상이다 — 그것을 조준하면 놓는다.
		if (Actor == nullptr || Actor == HeldItem)
		{
			continue;
		}
		const float AimDot = FVector::DotProduct(AimDirection, (Actor->GetActorLocation() - GetActorLocation()).GetSafeNormal());
		if (AimDot < FocusMinAimDot)
		{
			continue;
		}

		if (ACL9Item* Item = Cast<ACL9Item>(Actor); Item != nullptr && Item->CanBePickedUp())
		{
			// Head 슬롯 아이템은 머리에 착용하므로 퀵슬롯이 꽉 차거나 잠겨 있어도 상관없다 — 대신 머리가 비어 있어야 한다
			const bool bWearable = Item->GetItemDef()->CarrySlot == ECL9CarrySlot::Head;
			const bool bCanTake = bWearable ? (WornItem == nullptr) : bCanPickUp;
			if (bCanTake && AimDot > BestItemDot)
			{
				BestItem = Actor;
				BestItemDot = AimDot;
			}
			// 아이템은 인터페이스를 구현한 경우(카트가 그렇다)가 아니면
			// 상호작용 후보를 겸하지 않는다.
			if (Cast<ICL9Interactable>(Actor) == nullptr)
			{
				continue;
			}
		}
		if (ICL9Interactable* Interactable = Cast<ICL9Interactable>(Actor); Interactable != nullptr && Interactable->CanInteract(this))
		{
			if (AimDot > BestInteractableDot)
			{
				BestInteractable = Actor;
				BestInteractableDot = AimDot;
			}
		}
	}

	return BestItem != nullptr ? BestItem : BestInteractable;
}

void ACL9Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 휘두르기 타격은 호스트에서만, 모든 폰에 대해 판정한다 — 아래의 "로컬 조작 중이 아니면 나간다"보다 먼저 와야
	// 클라의 휘두르기도 호스트에서 판정된다 (docs/systems/inventory.md 사용).
	if (HasAuthority())
	{
		HostTickSwingBuffer();
		HostTickSwingHit();
	}

	// 발걸음 소음은 호스트(사건을 낸다)와 소유 클라이언트(같은 걸음을 예측해 먼저 그린다) 양쪽에서
	// 누적한다 — 남의 폰이 내는 걸음은 방송으로 온다 (docs/systems/wave.md 소유 클라의 예측)
	TickStepNoise();

	// 피격 플래시는 남의 화면에서도 보여야 한다(동료가 잡히는 것을 알아채는 유일한 단서다) — 로컬 조작 여부와 무관하게 모든 폰에서
	UpdateHurtFlash();
	// 휘두르기·피격 소리와 발소리도 남의 폰에서 들려야 한다 — 같은 자리
	UpdateHitSounds();
	TickFootstepSound();

	// 조준 하이라이트는 순전히 로컬 표시용이다 — 키가 눌렸을 때 호스트에서도
	// 같은 resolver가 실행되므로, 윤곽선은 E가 실제로 할 일을 그대로 보여준다.
	if (!IsLocallyControlled())
	{
		return;
	}

	// 데스캠은 복제된 HP 에서 파생한다 — 호스트 자기 폰은 죽음·부활 자리에서 바로 적용했고, 클라(와 늦게 도착한 PlayerState)는
	// 여기서 따라잡는다. 넘어짐 카메라·처형과 같은 "복제된 상태에서 파생" 틀 (docs/systems/character.md 죽음 — 데스캠)
	if (bDeathCameraOn != ShouldUseDeathCamera())
	{
		ApplyDeathCamera();
	}

	// 1인칭이므로 붙잡힘은 시점이 바닥으로 가라앉는 것으로 표현해야 한다 —
	// 소유자는 자기 몸을 볼 수 없으므로, 넘어짐·죽음은 바디 애니메이션 대신
	// 카메라를 가라앉힌다 (docs/systems/character.md 피격·넘어짐). 데스캠이 켜져 있으면 1인칭은 꺼져 있으니 건너뛴다
	// (죽음은 데스캠이 보여 준다; 오프셋 0 롤백이면 옛대로 죽음도 여기서 가라앉는다)
	if (!bDeathCameraOn)
	{
		const float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		const float EyeZ = CharacterDef ? CharacterDef->EyeHeight - HalfHeight : FirstPersonCameraComponent->GetRelativeLocation().Z;
		const float DownedZ = -HalfHeight + 30.f;
		const bool bDowned = IsKnockedDown() || IsDead();
		FVector CameraRel = FirstPersonCameraComponent->GetRelativeLocation();
		CameraRel.Z = FMath::FInterpTo(CameraRel.Z, bDowned ? DownedZ : EyeZ, DeltaSeconds, 6.f);
		FirstPersonCameraComponent->SetRelativeLocation(CameraRel);
	}

	// 처형 중·데스캠에는 조준 대상도 프롬프트도 없다 — 처형 카메라나 시체 위에 "[E] 줍기"가 뜨면 안 된다 (D6)
	AActor* NewFocus = IsExecuted() || bDeathCameraOn ? nullptr : ResolveFocus();
	if (NewFocus != FocusedActor.Get())
	{
		if (AActor* OldFocus = FocusedActor.Get())
		{
			SetFocusHighlight(OldFocus, false);
		}
		if (NewFocus != nullptr)
		{
			SetFocusHighlight(NewFocus, true);
		}
		FocusedActor = NewFocus;
	}

	// 대상이 바뀌지 않아도 프롬프트는 바뀔 수 있으므로(카트: 밀기 → 놓기)
	// 원문 패턴을 비교해서 달라졌을 때만 HUD에 알린다 — 서식은 HUD가 GetFocusPrompt를 읽을 때 한 번 한다.
	const FText Prompt = GetFocusPromptPattern();
	if (!Prompt.EqualTo(LastFocusPrompt))
	{
		LastFocusPrompt = Prompt;
		OnFocusPromptChanged.Broadcast();
	}

	UpdateStaminaShake();
	UpdateHitShakes();
}

void ACL9Character::UpdateStaminaShake()
{
	// 스태미나는 게이지가 없다 — 비어갈수록 화면이 흔들린다 (docs/plan/data/rules.md).
	// 소유 클라이언트 전용이며 표시 전용이지 판정이 아니다. 흔들림과 그 임계값은
	// shake 테이블에서 나온다 (rules 컬럼이 아니라 행이므로 새 흔들림은 행 추가로 처리한다).
	APlayerController* PC = Cast<APlayerController>(GetController());
	APlayerCameraManager* CameraManager = PC ? PC->PlayerCameraManager : nullptr;
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9ShakeTable* ShakeTable = GameInstance ? GameInstance->GetShakeTable() : nullptr;
	const FCL9ShakeInfo* Info = ShakeTable ? ShakeTable->Rows.Find(CL9ShakeNames::Stamina) : nullptr;
	if (CameraManager == nullptr || Info == nullptr || Info->StartAt <= 0.f)
	{
		return;
	}

	// 처형 중·데스캠에는 흔들지 않는다 — 처형 카메라는 몬스터 것이고 데스캠은 시체를 보는 자리다; 세기 0 이 아래에서 돌던 흔들림도 멈춘다 (D6)
	const float Stamina = GetCL9CharacterMovement()->GetStamina();
	const float Intensity = !IsExecuted() && !ShouldUseDeathCamera() && Stamina < Info->StartAt
		? (Info->StartAt - Stamina) / Info->StartAt
		: 0.f;

	if (Intensity > 0.f)
	{
		if (StaminaShakeInstance == nullptr || StaminaShakeInstance->IsFinished())
		{
			if (UClass* ShakeClass = Info->Shake.LoadSynchronous())
			{
				StaminaShakeInstance = CameraManager->StartCameraShake(ShakeClass, Intensity * Info->Scale);
			}
		}
		if (StaminaShakeInstance != nullptr)
		{
			// 게이지 세기 × 표의 배율 열 — 진폭 자체는 에셋에 있다 (CL9ShakeTable.h Scale)
			StaminaShakeInstance->ShakeScale = Intensity * Info->Scale;
		}
	}
	else if (StaminaShakeInstance != nullptr)
	{
		CameraManager->StopCameraShake(StaminaShakeInstance, false);
		StaminaShakeInstance = nullptr;
	}
}

void ACL9Character::UpdateHurtFlash()
{
	// 잡힌 직후만 빨갛다. 원천은 복제된 넘어짐 시각 하나이므로 호스트·클라·남의 화면이 전부 같은 그림을 만든다
	// (멀티캐스트 없음 — docs/systems/character.md 피격).
	bool bWantFlash = false;
	if (const UCL9RulesDef* Rules = GetRules(); Rules != nullptr && GetHurtElapsed() < Rules->HurtFlashTime)
	{
		if (!bHurtFlashLoaded)
		{
			bHurtFlashLoaded = true;
			HurtFlashMaterial = Rules->HitFlashMaterial.LoadSynchronous();
		}
		bWantFlash = HurtFlashMaterial != nullptr;
	}

	// 바뀔 때만 건드리고, 켠 적이 없으면 끄지도 않는다 — 다른 이유로 걸린 오버레이(줍기 하이라이트 같은)를 지우지 않기 위한 가드다
	if (bWantFlash == bHurtFlashOn)
	{
		return;
	}
	bHurtFlashOn = bWantFlash;

	UMaterialInterface* Overlay = bWantFlash ? HurtFlashMaterial.Get() : nullptr;
	if (USkeletalMeshComponent* BodyMesh = GetMesh())
	{
		BodyMesh->SetOverlayMaterial(Overlay);
	}
	// 1인칭 팔은 소유자에게만 그려지므로(SetOnlyOwnerSee) 조건 없이 걸어도 남의 화면에는 나타나지 않는다
	if (FirstPersonMesh != nullptr)
	{
		FirstPersonMesh->SetOverlayMaterial(Overlay);
	}
}

void ACL9Character::UpdateHitShakes()
{
	// 리슨 호스트는 권한이자 소유자라 자기 폰에서는 OnRep이 뜨지 않는다 — 복제 시각을 지난 틱 값과 비교하는 방식은
	// 호스트와 클라 양쪽에서 똑같이 작동한다 (docs/systems/character.md 피격).
	if (!bHitShakesPrimed)
	{
		bHitShakesPrimed = true;
		LastSeenSwingHitTime = SwingHitServerTime;
		LastSeenKnockdownTime = KnockdownStartServerTime;
		return;
	}

	if (SwingHitServerTime != LastSeenSwingHitTime)
	{
		LastSeenSwingHitTime = SwingHitServerTime;
		PlayShakeOnce(CL9ShakeNames::Hit);
	}
	if (KnockdownStartServerTime != LastSeenKnockdownTime)
	{
		LastSeenKnockdownTime = KnockdownStartServerTime;
		PlayShakeOnce(CL9ShakeNames::Hurt);
	}
}

void ACL9Character::PlayShakeOnce(FName RowName)
{
	// 스태미나 흔들림과 같은 경로다 — 흔들림을 늘리는 일은 표에 행을 추가하는 일이지 규칙에 컬럼을 늘리는 일이 아니다
	// (2026-09-05 결정, docs/plan/data/rules.md). 처형 중(처형의 HostKill 이 찍는 넘어짐 시각 포함)과 데스캠(죽는 타격의 넘어짐 시각)에는
	// 흔들지 않는다 (D6) — 데스캠은 HP 복제에서 파생하므로 같은 틱에 온 넘어짐 시각의 셰이크도 막힌다
	if (IsExecuted() || ShouldUseDeathCamera())
	{
		return;
	}
	APlayerController* PC = Cast<APlayerController>(GetController());
	APlayerCameraManager* CameraManager = PC ? PC->PlayerCameraManager : nullptr;
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9ShakeTable* ShakeTable = GameInstance ? GameInstance->GetShakeTable() : nullptr;
	const FCL9ShakeInfo* Info = ShakeTable ? ShakeTable->Rows.Find(RowName) : nullptr;
	if (CameraManager == nullptr || Info == nullptr)
	{
		return;
	}

	// 원샷이라 StartAt은 보지 않는다 — 게이지가 없는 순간 사건이므로 표의 배율(`Scale`, 기본 1)로 한 번 재생할 뿐이다
	if (UClass* ShakeClass = Info->Shake.LoadSynchronous())
	{
		CameraManager->StartCameraShake(ShakeClass, Info->Scale);
	}
}

void ACL9Character::UpdateHitSounds()
{
	// 셰이크(UpdateHitShakes)와 같은 "지난 틱 값과 비교" — 호스트 자기 폰에는 OnRep이 뜨지 않으므로. 접속 직후의 첫 틱은
	// 기준값만 받아 적어 이미 지나간 휘두르기·넘어짐을 되풀이하지 않는다
	if (!bHitSoundsPrimed)
	{
		bHitSoundsPrimed = true;
		LastSeenSwingStartTime = SwingStartServerTime;
		LastSeenSurfaceHitTime = SwingSurfaceHitServerTime;
		LastSeenHurtSoundTime = KnockdownStartServerTime;
		return;
	}

	// 휘두르기 시작에 난다 — 맞았는지는 모른다(맞으면 몬스터 쪽에서 `Hit`이 따로 난다)
	if (SwingStartServerTime != LastSeenSwingStartTime)
	{
		LastSeenSwingStartTime = SwingStartServerTime;
		CL9Sound::PlayAt(this, CL9SoundNames::Swing, GetActorLocation());
	}
	// 벽·바닥을 친 순간 — 충돌점은 복제하지 않으므로 폰 자리에서 (docs/systems/character.md 표면음)
	if (SwingSurfaceHitServerTime != LastSeenSurfaceHitTime)
	{
		LastSeenSurfaceHitTime = SwingSurfaceHitServerTime;
		CL9Sound::PlayAt(this, CL9SoundNames::HitSurface, GetActorLocation());
	}
	if (KnockdownStartServerTime != LastSeenHurtSoundTime)
	{
		LastSeenHurtSoundTime = KnockdownStartServerTime;
		CL9Sound::PlayAt(this, CL9SoundNames::Hurt, GetActorLocation());
	}
}

void ACL9Character::TickFootstepSound()
{
	const FVector Foot = GetFootLocation();
	if (!LastFootstepLocation.IsSet())
	{
		LastFootstepLocation = Foot;
		return;
	}
	const float Moved = FVector::Dist2D(LastFootstepLocation.GetValue(), Foot);
	LastFootstepLocation = Foot;

	// 서 있거나 공중이면 걸음이 아니다 — 남의 폰도 이동 모드가 복제되므로 IsMovingOnGround 는 어느 머신에서나 맞다
	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (Moved <= 0.f || Movement == nullptr || !Movement->IsMovingOnGround() || IsKnockedDown() || IsDead())
	{
		FootstepDistanceAccum = 0.f;
		return;
	}

	const UCL9RulesDef* Rules = GetRules();
	if (Rules == nullptr)
	{
		return;
	}
	// 달리기 입력(bWantsToRun)은 남의 폰에 복제되지 않는다 — 걷기 상한을 넘는 속도를 달리기로 친다(모든 머신이 같은 답)
	const bool bRun = CharacterDef != nullptr && GetGroundSpeed() > CharacterDef->WalkSpeed;
	if (bRun != bLastFootstepRun)
	{
		// 보폭이 바뀌면 누적을 버려야 새 보폭이 그대로 적용된다
		FootstepDistanceAccum = 0.f;
		bLastFootstepRun = bRun;
	}
	const float Stride = bRun ? Rules->FootstepStrideRun : Rules->FootstepStrideWalk;
	if (Stride <= 0.f)
	{
		FootstepDistanceAccum = 0.f;
		return;
	}
	FootstepDistanceAccum += Moved;
	if (FootstepDistanceAccum >= Stride)
	{
		FootstepDistanceAccum = 0.f;
		CL9Sound::PlayAt(this, CL9SoundNames::Footstep, Foot, CL9Sound::FootstepPitchJitter);
	}
}

void ACL9Character::SetFocusHighlight(AActor* Target, bool bOn) const
{
	if (UStaticMeshComponent* TargetMesh = Target->FindComponentByClass<UStaticMeshComponent>())
	{
		TargetMesh->SetOverlayMaterial(bOn ? HighlightOverlayMaterial.Get() : nullptr);
	}
}

void ACL9Character::Server_PickPlace_Implementation()
{
	// 넘어졌거나 처형 중이면 손이 말을 듣지 않는다 — 사용·먹기·던지기와 같은 규칙 (D2)
	if (IsIncapacitated())
	{
		return;
	}

	// 키 하나에 대상 하나: 조준이 가리키는 게 무엇이든 — 호스트는 같은
	// resolver를 다시 실행할 뿐 클라이언트를 신뢰하지 않는다 (docs/systems/input.md).
	AActor* Focus = ResolveFocus();

	// Head 슬롯 아이템은 드는 게 아니라 착용한다 — 같은 키 입력이지만 손은 그대로 둔다
	if (ACL9Item* Wearable = Cast<ACL9Item>(Focus); Wearable != nullptr && Wearable->CanBePickedUp()
		&& Wearable->GetItemDef()->CarrySlot == ECL9CarrySlot::Head && WornItem == nullptr)
	{
		HostWear(Wearable);
		return;
	}

	// 수레 짐칸 안의 아이템도 바닥의 것과 똑같이 여기로 온다 — 짐칸의 짐은 부착이 아니라 짐칸 위에 놓인 자유 아이템이다.
	if (ACL9Item* Item = Cast<ACL9Item>(Focus); Item != nullptr && Item->CanBePickedUp()
		&& Item->GetItemDef()->CarrySlot != ECL9CarrySlot::Head && CanPickUpItem())
	{
		// 활성 칸이 비었으면 거기, 아니면 첫 빈 칸 — 그리고 그 칸이 활성이 된다 (리썰 규칙)
		HostPickUp(Item, FindPickUpSlot());
		return;
	}
	if (ICL9Interactable* Interactable = Cast<ICL9Interactable>(Focus); Interactable != nullptr && Interactable->CanInteract(this))
	{
		Interactable->Interact(this);
		return;
	}

	// 아무것도 조준하지 않음: 끌던 카트를 놓는다 (뒤따르는 카트를 조준해도 마찬가지로 놓는다).
	// 손의 물건을 놓는 것은 E가 아니라 G(Server_Drop)다.
	if (PullingCart != nullptr)
	{
		PullingCart->StopPull(this);
	}
}

void ACL9Character::Server_ThrowChargeStart_Implementation()
{
	// 누워 있는 동안은 손이 말을 듣지 않는다 — 사용·먹기와 같은 규칙 (2026-09-15 사용자, character.md 넘어짐). 처형 중도 같다 (D2)
	if (HeldItem == nullptr || IsIncapacitated())
	{
		return;
	}
	ThrowChargeStartTime = GetServerTime();
}

void ACL9Character::Server_ThrowRelease_Implementation()
{
	const UCL9RulesDef* Rules = GetRules();
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (Rules == nullptr || CL9PlayerState == nullptr || HeldItem == nullptr || ThrowChargeStartTime < 0.f || IsIncapacitated())
	{
		return;
	}

	// 호스트는 자신의 시계로 차징을 측정한다 — 클라이언트가 보낸 차징
	// 값은 판정에 절대 들어가지 않는다 (docs/systems/input.md 던지기 차징).
	const float Charge = FMath::Clamp((GetServerTime() - ThrowChargeStartTime) / Rules->ThrowChargeTime, 0.f, 1.f);
	ThrowChargeStartTime = -1.f;
	ThrowServerTime = GetServerTime();
	const float Speed = FMath::Lerp(Rules->ThrowMinSpeed, Rules->ThrowMaxSpeed, Charge);

	const FVector Direction = GetControlRotation().Vector();
	const float EyeOffset = CharacterDef ? CharacterDef->EyeHeight - CharacterDef->CapsuleHalfHeight : 0.f;
	const float Clearance = GetCapsuleComponent()->GetScaledCapsuleRadius() + (HeldItem->GetItemDef() ? HeldItem->GetItemDef()->Radius : 20.f) + 10.f;
	const FVector Origin = GetActorLocation() + FVector(0.f, 0.f, EyeOffset) + Direction * Clearance;

	// 던진 칸은 비고 활성 칸 번호는 그대로다 — 다른 칸으로 자동 전환하지 않는다
	HostCancelSwingWindup();
	ACL9Item* Thrown = HeldItem;
	HeldItem = nullptr;
	HostClearActiveSlot();
	Thrown->SetOwner(nullptr);
	Thrown->OnThrown(Origin, Direction * Speed);
}

void ACL9Character::Server_SelectSlot_Implementation(uint8 Slot)
{
	const UCL9RulesDef* Rules = GetRules();
	const ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	// 넘어졌거나 처형 중이면 칸도 못 바꾼다 (D2)
	if (Rules == nullptr || CL9PlayerState == nullptr || !CL9PlayerState->Slots.IsValidIndex(Slot)
		|| Slot == CL9PlayerState->ActiveSlot || !CanSwitchSlot() || IsIncapacitated())
	{
		return;
	}
	HostSelectSlot(Slot, Rules);
}

void ACL9Character::Server_ScrollSlot_Implementation(int32 Direction)
{
	const UCL9RulesDef* Rules = GetRules();
	const ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	const int32 SlotCount = CL9PlayerState ? CL9PlayerState->Slots.Num() : 0;
	if (Rules == nullptr || SlotCount <= 1 || Direction == 0 || !CanSwitchSlot() || IsIncapacitated())
	{
		return;
	}
	// 양 끝은 순환한다 — 호스트의 ActiveSlot에서 한 칸만 옮기므로 클라 사본이 늦어도 노치마다 한 칸씩 간다
	const int32 Step = Direction < 0 ? -1 : 1;
	HostSelectSlot((static_cast<int32>(CL9PlayerState->ActiveSlot) + Step + SlotCount) % SlotCount, Rules);
}

void ACL9Character::Server_Drop_Implementation()
{
	// 누워 있는 동안은 놓지도 던지지도 못한다 — 사용·먹기와 같은 규칙 (2026-09-15 사용자, character.md 넘어짐).
	// 넘어질 때 짐을 그대로 들고 있게 된 뒤로는 이 검사가 없으면 누운 채로 물건을 내려놓을 수 있다. 처형 중도 같다 (D2)
	if (HeldItem == nullptr || IsIncapacitated())
	{
		return;
	}

	// **든 자리에서 그대로 놓는다** (2026-09-15 사용자 "그자리에서 떨어지게" — 2026-09-14의 조준점 놓기·수레 특례 폐기).
	// 아이템 액터의 루트는 어느 머신에서나 전신 메시의 손 소켓에 붙어 있으므로, 지금 위치가 곧 손의 자리다. 시선으로 옮기지
	// 않고 회전도 지금 것 그대로 두며, OnPlaced가 속도를 0으로 만들어 그 자리에서 떨어지게 한다.
	const FVector DropLocation = HeldItem->GetActorLocation();

	// 예외는 하나다: **짐칸 쪽을 보고 있으면 그 짐칸으로 톡 얹는다** (2026-09-15 사용자 "수레 쪽에서 떨구면 그냥 밑으로
	// 떨어지는 게 아니라 자연스럽게 들어가게"). 손 자리에서 낮은 호로 보내고 나머지는 물리에 맡긴다 — 짐칸 상자와
	// 겹쳐 멈추면 그것이 실린 것이다(부착이 아니다, cart.md 적재). 짐칸이 멀어 세게 보내야 닿으면 아래 손 자리 낙하 그대로다.
	FVector LobVelocity = FVector::ZeroVector;
	const ACL9Cart* AimedCart = FindAimedCart();
	const bool bLobIntoCargo = AimedCart != nullptr && ComputeCargoLobVelocity(*AimedCart, *HeldItem, DropLocation, LobVelocity);

	ThrowChargeStartTime = -1.f;
	HostCancelSwingWindup();
	ACL9Item* Placed = HeldItem;
	HeldItem = nullptr;
	HostClearActiveSlot();
	Placed->SetOwner(nullptr);
	if (bLobIntoCargo)
	{
		// 던지기가 아니라 놓기다 — ThrowServerTime(던지기 클립)은 건드리지 않는다. 손짓 없이 물건만 톡 간다.
		Placed->OnThrown(DropLocation, LobVelocity);
		return;
	}
	Placed->OnPlaced(DropLocation);
}

ACL9Cart* ACL9Character::FindAimedCart() const
{
	const UCL9RulesDef* Rules = GetRules();
	if (Rules == nullptr)
	{
		return nullptr;
	}

	TArray<FOverlapResult> Overlaps;
	const FCollisionObjectQueryParams ObjectParams(ECC_WorldDynamic);
	const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CL9CargoAim), false, this);
	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity,
		ObjectParams, FCollisionShape::MakeSphere(Rules->PickRadius), QueryParams);

	// 잣대는 ResolveFocus와 같다 — 캡슐 중심에서 본 방향이 조준 방향과 FocusMinAimDot 이상. 다만 겨누는 점은
	// 수레 중심이 아니라 **짐칸 지점**이다: 짐칸을 내려다보는 것과 수레 옆의 벽을 보는 것이 이걸로 갈린다.
	const FVector AimDirection = GetControlRotation().Vector();
	ACL9Cart* Best = nullptr;
	float BestDot = FocusMinAimDot;
	for (const FOverlapResult& Overlap : Overlaps)
	{
		ACL9Cart* Cart = Cast<ACL9Cart>(Overlap.GetActor());
		if (Cart == nullptr)
		{
			continue;
		}
		const float AimDot = FVector::DotProduct(AimDirection,
			(Cart->GetCargoDropPoint(0.f) - GetActorLocation()).GetSafeNormal());
		if (AimDot > BestDot)
		{
			Best = Cart;
			BestDot = AimDot;
		}
	}
	return Best;
}

bool ACL9Character::ComputeCargoLobVelocity(const ACL9Cart& Cart, const ACL9Item& Item, const FVector& Origin, FVector& OutVelocity) const
{
	const UCL9ItemDef* Def = Item.GetItemDef();
	// 아이템은 월드 중력이 아니라 **행의 Gravity**로 난다(CL9Item::Tick의 중력 주입) — 호도 그 중력으로 푼다
	const float Gravity = Def == nullptr ? 0.f : (Def->Gravity > 0.f ? Def->Gravity : -GetWorld()->GetGravityZ());
	if (Def == nullptr || Gravity <= 0.f)
	{
		return false;
	}

	const FVector Delta = Cart.GetCargoDropPoint(Def->Radius) - Origin;

	// 바닥값: 정점을 손 자리(목표가 손보다 높으면 목표 위 CargoLobApexRise)에 두는 호 — 올라간 만큼만 더 떨어지므로
	// 착지가 이 낙차에서 낼 수 있는 가장 느린 호다. 여기서 더 낮출 수는 없다.
	const float ApexRise = FMath::Max<float>(Delta.Z + CargoLobApexRise, 0.f);
	float FlightTime = (FMath::Sqrt(2.f * Gravity * ApexRise) + FMath::Sqrt(2.f * Gravity * (ApexRise - Delta.Z))) / Gravity;

	// 그 호는 **짐칸 벽을 못 넘는다** (2026-09-15 PIE B4: 손이 벽 윗단보다 높아도, 벽에 이를 때쯤 이미 몇 cm 내려와
	// 계란이 윗단에 맞고 튕겨 나왔다). 벽을 지나는 지점에서 아이템 **밑면**이 윗단 위에 있도록 비행 시간을 늘린다 —
	// 같은 두 점을 잇는 호는 오래 날수록 높이 뜬다. 착지가 그만큼 빨라지는 것은 감수한다(2026-09-15 사용자: 금은 그대로 둔다).
	float WallFraction = 0.f;
	float WallTopZ = 0.f;
	if (Cart.GetCargoWallCrossing(Origin, WallFraction, WallTopZ))
	{
		// 비율 f 지점의 높이: z(f) = 원점 + f·Δz + ½·g·T²·f(1−f) — 이것이 ClearZ 이상이 되는 가장 짧은 T를 푼다.
		const float ClearZ = WallTopZ + Def->Radius + CargoLobWallClearance;
		const float Needed = ClearZ - Origin.Z - WallFraction * Delta.Z;
		const float Shape = 0.5f * Gravity * WallFraction * (1.f - WallFraction);
		if (Needed > 0.f && Shape > UE_KINDA_SMALL_NUMBER)
		{
			FlightTime = FMath::Max(FlightTime, FMath::Sqrt(Needed / Shape));
		}
	}
	if (FlightTime <= UE_KINDA_SMALL_NUMBER)
	{
		return false;
	}

	// 같은 두 점을 T초에 잇는 포물선의 초기 속도 — 수평은 거리/T, 수직은 낙차와 중력이 정한다.
	const float LaunchUp = (Delta.Z + 0.5f * Gravity * FlightTime * FlightTime) / FlightTime;
	const FVector Launch = FVector(Delta.X, Delta.Y, 0.f) / FlightTime + FVector(0.f, 0.f, LaunchUp);
	const float LaunchSpeed = Launch.Size();

	// 되돌아가는 조건은 하나뿐이다: 얹는 동작이 아니라 던지기가 되어 버리는 속도 (2026-09-15 사용자 — 깨짐을 기준으로
	// 한 CrackSpeed 상한은 폐기. 놓기든 얹기든 계란에 금이 가는 것은 규칙으로 받아들인다). 벽 넘기가 호를 높이므로
	// 상한은 수평만이 아니라 **보내는 힘 전체**에 건다 — 먼 짐칸일수록 수평은 느려지고 수직이 커지기 때문이다.
	UE_LOG(LogCL9, Log, TEXT("Cargo lob '%s' → '%s': drop %.0f cm, wall %.0f%% @ %.0f cm, flight %.2f s, launch %.0f/%.0f cm/s (up %.0f), landing %.0f cm/s%s"),
		*GetNameSafe(&Item), *GetNameSafe(&Cart), -Delta.Z, WallFraction * 100.f, WallTopZ, FlightTime,
		LaunchSpeed, CargoLobMaxSpeed, LaunchUp,
		FMath::Sqrt(Launch.SizeSquared2D() + FMath::Square(LaunchUp - Gravity * FlightTime)),
		LaunchSpeed > CargoLobMaxSpeed ? TEXT(" — too far, placing at hand instead") : TEXT(""));

	if (LaunchSpeed > CargoLobMaxSpeed)
	{
		return false;
	}

	OutVelocity = Launch;
	return true;
}

int32 ACL9Character::GetActiveSlot() const
{
	const ACL9PlayerState* CL9PlayerState = GetPlayerState<ACL9PlayerState>();
	return CL9PlayerState != nullptr ? CL9PlayerState->ActiveSlot : 0;
}

bool ACL9Character::CanSwitchSlot() const
{
	// 수레는 두 손으로 민다 — 미는 동안은 칸을 뒤적일 수 없다. 두 손 아이템(냄비·작업등, HoldPose Large)도
	// 같다: 리썰의 twoHanded 규칙 — 들고 있는 동안 전환·줍기 불가 (docs/systems/inventory.md 잠금).
	if (PullingCart != nullptr)
	{
		return false;
	}
	const UCL9ItemDef* Def = HeldItem != nullptr ? HeldItem->GetItemDef() : nullptr;
	return Def == nullptr || Def->HoldPose != ECL9HoldPose::Large;
}

bool ACL9Character::CanPickUpItem() const
{
	// 줍기는 칸 전환을 겸한다(주운 칸이 활성이 된다) — 전환 잠금이 그대로 줍기 잠금이고, 꽉 차면 못 줍는다
	return CanSwitchSlot() && FindPickUpSlot() != INDEX_NONE;
}

int32 ACL9Character::FindPickUpSlot() const
{
	const ACL9PlayerState* CL9PlayerState = GetPlayerState<ACL9PlayerState>();
	if (CL9PlayerState == nullptr)
	{
		return INDEX_NONE;
	}
	const int32 Active = CL9PlayerState->ActiveSlot;
	if (CL9PlayerState->Slots.IsValidIndex(Active) && !CL9PlayerState->Slots[Active].IsSet())
	{
		return Active;
	}
	return CL9PlayerState->FindEmptySlot();
}

void ACL9Character::HostSwing()
{
	// 회복·판정 모양은 규칙 열이, 피해·경직 종류·넉백 거리는 아이템 행이, 경직 시간은 몬스터 행이 정한다 (docs/systems/character.md 휘두르기)
	const UCL9RulesDef* Rules = GetRules();
	const float Now = GetServerTime();
	if (!HasAuthority() || Rules == nullptr || Now < SwingReadyTime)
	{
		return;
	}
	SwingReadyTime = Now + Rules->SwingRecovery;
	SwingStartServerTime = Now;

	// 이번 휘두르기의 히트 구간을 연다 — 실제 타격은 HostTickSwingHit이 구간 동안 낸다. 맞은 목록을 비워 새 휘두르기에서는
	// 같은 몬스터를 다시 맞힐 수 있게 하고, 이전 틱 경과는 "아직 없음"으로 둔다. 표면음·로그 계수도 휘두르기 단위다
	SwingHitItem = HeldItem;
	SwingHitMonsters.Reset();
	SwingHitPrevElapsed = -1.f;
	SwingNearestStruckDist = TNumericLimits<float>::Max();
	bSwingSurfaceStamped = false;
	SwingTestedCount = 0;
	SwingOccludedCount = 0;
}

void ACL9Character::HostTickSwingHit()
{
	ACL9Item* Item = SwingHitItem.Get();
	if (Item == nullptr)
	{
		return;
	}

	// 구간 도중 칸 전환·놓기·던지기·먹기로 손의 것이 바뀌면 구간이 끝난다 — 사라진 물건으로 계속 때리지 않는다
	const UCL9RulesDef* Rules = GetRules();
	const UCL9ItemDef* Def = Item->GetItemDef();
	if (Item != HeldItem || Rules == nullptr || Def == nullptr)
	{
		HostLogSwing();
		SwingHitItem = nullptr;
		return;
	}

	const float Elapsed = GetSwingElapsed();
	const float PrevElapsed = SwingHitPrevElapsed;
	SwingHitPrevElapsed = Elapsed;

	// 판정은 점이 아니라 **구간 겹침**이다: 이번 틱이 덮는 시간 (PrevElapsed, Elapsed]가 히트 구간 [Start, End)와 겹치면
	// 그 틱에 판정한다. 점 판정(지금 Elapsed가 구간 안)이면 0.2초 폭이 틱 사이로 통째로 빠지는 낮은 프레임률(PIE 자동화
	// 3~5fps)에서 닿는 거리인데도 헛스윙이 난다 (2026-09-15 리드). 구간은 "판정이 열린 시간"이지 손이 지나는 자리가 아니다
	const bool bWindowOver = Elapsed >= Rules->SwingHitEnd;
	if (Elapsed >= Rules->SwingHitStart && PrevElapsed < Rules->SwingHitEnd)
	{
		// 원점은 손 높이(눈높이 − 66 ≈ 90cm), 방향은 **그 순간의 컨트롤 회전** — 던지기와 같은 방식이다. 카메라 컴포넌트의
		// 트랜스폼은 읽지 않는다: bUsePawnControlRotation 의 피치는 로컬 GetCameraView 안에서만 적용되므로 리슨 호스트에서
		// 클라 소유 폰의 카메라는 피치 0 이다. 매 틱 다시 잡으므로 구간 동안 시선을 돌리면 선분도 따라간다
		const FVector Dir = GetControlRotation().Vector();
		const float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		const float EyeHeight = CharacterDef != nullptr ? CharacterDef->EyeHeight : CL9::EyeHeight;
		const FVector Origin = GetActorLocation() + FVector(0.f, 0.f, EyeHeight - HalfHeight - SwingHandDrop);
		const float Radius = Rules->SwingHitRadius;
		const FVector SweepStart = Origin + Dir * Radius;
		const FVector SweepEnd = Origin + Dir * Rules->SwingHitReach;

		// 선분은 원점에서 반경 하나 앞에서 시작한다 — 구가 등 뒤로 나가 뒤에 선 닭이 bStartPenetrating 으로 걸리지 않게.
		// 몬스터 몸체는 WorldDynamic이지만 **오브젝트 질의를 쓰지 않는다** — 오브젝트 질의는 걸리는 것을 전부 "블로킹"으로
		// 보고해 가장 가까운 하나에서 스윕이 잘리고, 나란히 선 닭 둘 중 하나만 맞는다. 대신 WorldDynamic 채널 스윕에
		// 그 채널만 Overlap(나머지는 Ignore)으로 응답해 **겹침(touch)으로** 받는다: 닿은 전부가 돌아오고, 벽(WorldStatic)·
		// 폰(나·남)은 애초에 결과에 들어오지 않는다. 자신과 든 아이템은 따로 뺀다.
		FCollisionQueryParams Params(SCENE_QUERY_STAT(CL9SwingHit), false, this);
		Params.AddIgnoredActor(Item);
		FCollisionResponseParams ResponseParams(ECR_Ignore);
		ResponseParams.CollisionResponse.SetResponse(ECC_WorldDynamic, ECR_Overlap);
		TArray<FHitResult> Hits;
		GetWorld()->SweepMultiByChannel(Hits, SweepStart, SweepEnd, FQuat::Identity, ECC_WorldDynamic,
			FCollisionShape::MakeSphere(Radius), Params, ResponseParams);

		// 이번 휘두르기에서 아직 아무것도 못 맞혔는가 — 카메라 킥을 한 휘두르기에 한 번만 내기 위한 기준이다
		const bool bFirstConnect = SwingHitMonsters.IsEmpty();

		int32 Struck = 0;
		for (const FHitResult& Hit : Hits)
		{
			// 대상은 몬스터뿐이다 — 사람은 맞지 않는다(플레이어끼리의 타격은 없다)
			ACL9Monster* Monster = Cast<ACL9Monster>(Hit.GetActor());
			if (Monster == nullptr || SwingHitMonsters.Contains(Monster))
			{
				continue;
			}
			// 시체는 타격이 아니다 — OnStrike가 이미 Dead를 무시하므로 맞은 것으로 세지도 않는다(시체를 쳐도 킥이 나면 안 된다)
			if (Monster->GetState() == ECL9MonsterState::Dead)
			{
				continue;
			}
			++SwingTestedCount;

			// 가림: 원점 → 몬스터 구 **중심**(ImpactPoint 는 시작 관통이면 길이 0 이다)으로 Visibility 선 트레이스. 세상 지형이 먼저
			// 막으면 벽 너머다. 폰·아이템·다른 몬스터는 가리지 않는다(사람은 맞지도 가리지도 않는다). 열 없음, 항상 켜짐
			FHitResult Block;
			if (ACL9Monster::TraceWorld(GetWorld(), this, Origin, Monster->GetActorLocation(), Block, Item))
			{
				++SwingOccludedCount;
				continue;
			}
			SwingHitMonsters.Add(Monster);
			SwingNearestStruckDist = FMath::Min(SwingNearestStruckDist, FVector::Dist(Origin, Monster->GetActorLocation()));
			Monster->OnStrike(Def->HitDamage, Def->HitStun, Def->HitKnockback, this);
			++Struck;
		}

		// 맞은 순간을 찍어두면 소유 클라(와 리슨 호스트 자기 폰)가 그 변화를 보고 카메라를 한 번 튕긴다.
		// 나란한 둘을 한 번에 맞혀도 킥은 한 번이고, 헛스윙이면 값이 그대로라 아무 일도 없다. 회복도 팔 히트스톱만큼 늦춘다 —
		// 맞든 빗나가든 팔이 돌아온 뒤 같은 여유가 남고, 다음 시작 시각 리셋이 멈춘 팔 애님 도중에 떨어지지 않는다
		if (bFirstConnect && Struck > 0)
		{
			SwingHitServerTime = GetServerTime();
			SwingReadyTime += Rules->HitStopTime;
		}

		// 표면음: 같은 틱, 원점 → Reach **선** 트레이스(구가 아니다 — 손 높이에서 20° 아래를 보면 구는 30cm 앞 바닥에 닿아 닭보다
		// 먼저 잡힌다). 세상 지형에 막히고 그보다 가까운 명중이 이번 휘두르기에 없으면 휘두르기당 한 번 찍는다. 소리뿐이다
		if (!bSwingSurfaceStamped)
		{
			FHitResult Surface;
			if (ACL9Monster::TraceWorld(GetWorld(), this, Origin, Origin + Dir * Rules->SwingHitReach, Surface, Item)
				&& Surface.Distance <= SwingNearestStruckDist)
			{
				bSwingSurfaceStamped = true;
				SwingSurfaceHitServerTime = GetServerTime();
			}
		}

		UE_LOG(LogCL9, Verbose, TEXT("SwingHit: t=%.3f r=%.1f hits=%d struck=%d"), Elapsed, Radius, Hits.Num(), Struck);
	}

	if (bWindowOver)
	{
		HostLogSwing();
		SwingHitItem = nullptr;
	}
}

void ACL9Character::HostLogSwing() const
{
	if (!bCheatSwingLog)
	{
		return;
	}
	const bool bHit = SwingHitServerTime >= SwingStartServerTime;
	const bool bSurface = SwingSurfaceHitServerTime >= SwingStartServerTime;
	UE_LOG(LogCL9, Log, TEXT("cl9.swing: start=%.3f input=%.3f hit=%s surface=%s ready=%.3f arm_return=%.3f tested=%d occluded=%d"),
		SwingStartServerTime, SwingInputServerTime,
		bHit ? *FString::Printf(TEXT("%.3f"), SwingHitServerTime) : TEXT("-"),
		bSurface ? *FString::Printf(TEXT("%.3f"), SwingSurfaceHitServerTime) : TEXT("-"),
		SwingReadyTime, SwingStartServerTime + UCL9RulesDef::SwingClipWindow, SwingTestedCount, SwingOccludedCount);
}

float ACL9Character::GetStatBonus(ECL9Stat Stat) const
{
	// 만료된 것은 세지 않는다 — 목록에서 지우는 일은 호스트가 하지만, 판단은 시각으로 한다.
	// 그래야 복제가 늦게 도착해도 양쪽이 같은 값을 도출한다.
	const float Now = GetServerTime();
	float Bonus = 0.f;
	for (const FCL9ActiveEffect& Effect : ActiveEffects)
	{
		if (Effect.Stat == Stat && Now < Effect.EndServerTime)
		{
			Bonus += Effect.Amount;
		}
	}
	return Bonus;
}

float ACL9Character::GetStatBonusRemaining(ECL9Stat Stat) const
{
	// 만료 조건은 GetStatBonus와 글자 그대로 같아야 한다 — 같은 순회에서 가장 늦게
	// 끝나는 시각을 잡는다. 겹쳐 먹으면 합계가 그때까지 유지되기 때문이다.
	const float Now = GetServerTime();
	float LatestEnd = 0.f;
	for (const FCL9ActiveEffect& Effect : ActiveEffects)
	{
		if (Effect.Stat == Stat && Now < Effect.EndServerTime)
		{
			LatestEnd = FMath::Max(LatestEnd, Effect.EndServerTime);
		}
	}
	return LatestEnd > 0.f ? LatestEnd - Now : 0.f;
}

TArray<FName> ACL9Character::GetActiveBuffs() const
{
	// 만료 조건은 GetStatBonus와 글자 그대로 같다 — 다르면 "이름은 떠 있는데 0초"가 난다.
	// 한 버프가 스탯 여럿을 걸면 항목도 여럿이므로 별칭은 한 번만 담는다.
	const float Now = GetServerTime();
	TArray<FName> Aliases;
	for (const FCL9ActiveEffect& Effect : ActiveEffects)
	{
		if (Now < Effect.EndServerTime && !Effect.BuffAlias.IsNone())
		{
			Aliases.AddUnique(Effect.BuffAlias);
		}
	}
	return Aliases;
}

float ACL9Character::GetBuffRemaining(FName BuffAlias) const
{
	// GetStatBonusRemaining과 같은 꼴이다 — 묻는 기준만 스탯에서 버프로 바뀐다.
	const float Now = GetServerTime();
	float LatestEnd = 0.f;
	for (const FCL9ActiveEffect& Effect : ActiveEffects)
	{
		if (Effect.BuffAlias == BuffAlias && Now < Effect.EndServerTime)
		{
			LatestEnd = FMath::Max(LatestEnd, Effect.EndServerTime);
		}
	}
	return LatestEnd > 0.f ? LatestEnd - Now : 0.f;
}

void ACL9Character::HostEat()
{
	if (!HasAuthority() || HeldItem == nullptr || IsIncapacitated())
	{
		return;
	}

	const UCL9ItemDef* Def = HeldItem->GetItemDef();
	if (Def == nullptr)
	{
		return;
	}
	// 먹을 수 있는 것은 Dish뿐이다 — "먹을 수 있나" 플래그를 따로 두지 않고 타입이 가른다
	if (Def->ItemType != ECL9ItemType::Dish)
	{
		UE_LOG(LogCL9, Log, TEXT("Eat refused: '%s'/'%s' is not a dish."),
			*HeldItem->GetAlias().ToString(), *HeldItem->GetSubAlias().ToString());
		return;
	}

	const float Now = GetServerTime();
	// 지나간 효과는 여기서 청소한다 — 목록이 무한정 자라지 않게
	ActiveEffects.RemoveAll([Now](const FCL9ActiveEffect& Effect)
	{
		return Now >= Effect.EndServerTime;
	});

	// 효과 값은 아이템 행이 아니라 버프 표가 갖는다 — 아이템은 별칭으로 가리키기만
	// 한다 (docs/plan/data/buff.md). 없는 별칭은 로드 검사가 이미 깨진 행으로 잡았지만,
	// 그 행은 Find*로 나오지 않으므로 여기서 다시 걸러 준다.
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	for (const FName& BuffAlias : Def->EatBuffs)
	{
		const UCL9BuffDef* Buff = Registry ? Registry->FindBuff(BuffAlias) : nullptr;
		if (Buff == nullptr)
		{
			UE_LOG(LogCL9, Error, TEXT("Eat: '%s' names buff '%s' which is missing or broken — nothing applied."),
				*Def->Alias.ToString(), *BuffAlias.ToString());
			continue;
		}

		for (const FCL9StatEffect& Wanted : Buff->Effects)
		{
			if (Wanted.DurationSec <= 0.f)
			{
				// 즉시형은 지속 효과로 만들 수 없다 — 지금 범위에 즉시 스탯 변화가 없어 로그만 남긴다
				UE_LOG(LogCL9, Warning, TEXT("Eat: buff '%s' has an instant effect on stat %d — instant stat changes are not implemented."),
					*BuffAlias.ToString(), static_cast<int32>(Wanted.Stat));
				continue;
			}
			FCL9ActiveEffect& Effect = ActiveEffects.AddDefaulted_GetRef();
			Effect.Stat = Wanted.Stat;
			Effect.Amount = Wanted.Amount;
			Effect.EndServerTime = Now + Wanted.DurationSec;
			Effect.BuffAlias = BuffAlias;
			UE_LOG(LogCL9, Log, TEXT("Ate '%s'/'%s': buff '%s' gives stat %d %+.1f for %.0fs."),
				*HeldItem->GetAlias().ToString(), *HeldItem->GetSubAlias().ToString(),
				*BuffAlias.ToString(), static_cast<int32>(Wanted.Stat), Wanted.Amount, Wanted.DurationSec);
		}
	}

	// 먹은 것은 사라진다 — 판정은 여기서 끝이고, 먹기 클립(0.6초)은 이 시각을 복제받은 ABP가 뒤따라 그린다
	EatStartServerTime = Now;
	HostConsumeHeldItem();
}

void ACL9Character::HostStowHeld()
{
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (!HasAuthority() || HeldItem == nullptr || CL9PlayerState == nullptr)
	{
		return;
	}

	// 비활성 칸은 데이터로만 존재한다 — 액터를 숨겨 두는 대신 파괴하고 다시 꺼낼 때 리스폰한다
	// (복제 비용이 없고 상태가 단순하다, 2026-09-01 확정). 값은 여기서 붙잡아 재스폰 때 되돌린다.
	if (CL9PlayerState->Slots.IsValidIndex(CL9PlayerState->ActiveSlot))
	{
		FCL9CarriedItem& Stowed = CL9PlayerState->Slots[CL9PlayerState->ActiveSlot];
		Stowed.ItemAlias = HeldItem->GetAlias();
		Stowed.ItemSubAlias = HeldItem->GetSubAlias();
		Stowed.Value = HeldItem->GetValue();
	}
	// 차징 중이던 던지기·공격 대기는 대상이 사라진다
	ThrowChargeStartTime = -1.f;
	HostCancelSwingWindup();
	HeldItem->Destroy();
	HeldItem = nullptr;
}

void ACL9Character::HostSelectSlot(int32 Slot, const UCL9RulesDef* Rules)
{
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (!HasAuthority() || CL9PlayerState == nullptr || Rules == nullptr || !CL9PlayerState->Slots.IsValidIndex(Slot))
	{
		return;
	}

	HostStowHeld();
	CL9PlayerState->ActiveSlot = static_cast<uint8>(Slot);

	// 복사해 둔다 — HostPickUp이 같은 칸을 다시 쓴다
	const FCL9CarriedItem Chosen = CL9PlayerState->Slots[Slot];
	if (Chosen.IsSet())
	{
		if (ACL9GameMode* GameMode = GetWorld()->GetAuthGameMode<ACL9GameMode>())
		{
			// 손 높이에서 스폰해 곧바로 손 소켓에 붙인다 — 값은 칸에 저장된 것을 그대로 (다시 굴리지 않는다)
			const FTransform HandTransform(GetActorLocation() + FVector(0.f, 0.f, Rules->CarryHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
			if (ACL9Item* TakenOut = GameMode->SpawnItem(Chosen.ItemAlias, Chosen.ItemSubAlias, HandTransform, Chosen.Value))
			{
				// HostPickUp이 갱신 알림을 보낸다
				HostPickUp(TakenOut, Slot);
				return;
			}
		}
		// 행이 사라진 등 스폰 실패 — 유령 데이터를 남기지 않는다 (SpawnItem이 이미 Error 로그를 남겼다)
		CL9PlayerState->Slots[Slot] = FCL9CarriedItem();
	}
	CL9PlayerState->NotifyCarryChanged();
}

void ACL9Character::HostClearActiveSlot()
{
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (!HasAuthority() || CL9PlayerState == nullptr)
	{
		return;
	}
	if (CL9PlayerState->Slots.IsValidIndex(CL9PlayerState->ActiveSlot))
	{
		CL9PlayerState->Slots[CL9PlayerState->ActiveSlot] = FCL9CarriedItem();
	}
	CL9PlayerState->NotifyCarryChanged();
}

float ACL9Character::GetSwingElapsed() const
{
	return GetServerTime() - SwingStartServerTime;
}

float ACL9Character::GetSwingAnimElapsed() const
{
	const float Elapsed = GetSwingElapsed();
	// 이번 휘두르기의 명중만 본다 — 지난 휘두르기의 명중 시각은 시작 시각보다 앞서 있다. 남의 화면에서는 COND_OwnerOnly 라
	// 값이 오지 않아 항상 여기서 빠진다
	const UCL9RulesDef* Rules = GetRules();
	if (Rules == nullptr || Rules->HitStopTime <= 0.f || SwingHitServerTime < SwingStartServerTime)
	{
		return Elapsed;
	}
	// 명중 뒤 HitStopTime 동안은 명중 순간의 프레임에 머물고, 지나면 그만큼 늦게 이어서 돈다
	const float HitAt = SwingHitServerTime - SwingStartServerTime;
	const float SinceHit = GetServerTime() - SwingHitServerTime;
	// 클라의 서버 시각 추정이 뒤처져 명중 시각이 아직 미래면 실제 경과를 넘기지 않는다
	return SinceHit < Rules->HitStopTime ? FMath::Min(HitAt, Elapsed) : Elapsed - Rules->HitStopTime;
}

float ACL9Character::GetSwingWindupElapsed() const
{
	// 클라의 서버 시각 추정이 조금 앞설 수 있어 0 아래로 내려가지 않게 — "-1 = 대기 아님"이 흐려지지 않는다
	return SwingWindupServerTime < 0.f ? -1.f : FMath::Max(0.f, GetServerTime() - SwingWindupServerTime);
}

float ACL9Character::GetEatElapsed() const
{
	return GetServerTime() - EatStartServerTime;
}

float ACL9Character::GetKnockdownElapsed() const
{
	return IsKnockedDown() ? FMath::Max(0.f, GetServerTime() - KnockdownStartServerTime) : -1.f;
}

float ACL9Character::GetKnockdownRemaining() const
{
	return IsKnockedDown() ? KnockdownEndServerTime - GetServerTime() : -1.f;
}

float ACL9Character::GetExecuteElapsed() const
{
	// 클라의 서버 시각 추정이 조금 앞설 수 있어 0 아래로 내려가지 않게
	return IsExecuted() ? FMath::Max(0.f, GetServerTime() - ExecuteStartServerTime) : -1.f;
}

float ACL9Character::GetExecuteDuration() const
{
	const UCL9MonsterDef* Def = ExecutedBy != nullptr ? ExecutedBy->GetMonsterDef() : nullptr;
	return Def != nullptr ? Def->ExecuteDuration : 0.f;
}

FName ACL9Character::GetExecutedByAlias() const
{
	const UCL9MonsterDef* Def = ExecutedBy != nullptr ? ExecutedBy->GetMonsterDef() : nullptr;
	return Def != nullptr ? Def->Alias : NAME_None;
}

float ACL9Character::GetHurtElapsed() const
{
	// 애님용 GetKnockdownElapsed는 일어나는 순간 −1로 돌아간다(그쪽은 "지금 넘어져 있나"가 필요하다). 표현 시계는
	// 그렇게 되면 −1 < HurtFlashTime이 되어 플래시가 영원히 켜지므로, 여기서는 시각 차이만 그대로 쓴다.
	// 넘어진 적이 없으면 KnockdownStartServerTime이 −1000이라 아주 큰 값이 되어 저절로 꺼진 상태가 된다.
	return FMath::Max(0.f, GetServerTime() - KnockdownStartServerTime);
}

float ACL9Character::GetHurtFlash() const
{
	// 처형 중·데스캠에는 비네트가 없다 — 처형 카메라나 시체 위에 빨간 화면을 얹지 않는다 (D6). HUD 는 이 게터로만 게이트한다
	const UCL9RulesDef* Rules = GetRules();
	if (Rules == nullptr || Rules->HurtVignetteTime <= 0.f || IsExecuted() || ShouldUseDeathCamera())
	{
		return 0.f;
	}
	return FMath::Clamp(1.f - GetHurtElapsed() / Rules->HurtVignetteTime, 0.f, 1.f);
}

bool ACL9Character::HasHurtDirection() const
{
	// 관전 중엔 내 폰의 사건이 남의 화면에 뜬다 — 호를 숨긴다. 처형 중·데스캠도 같다(처형 카메라나 시체 위에 방향 호가 뜨면 안 된다, D6)
	const ACL9PlayerController* PC = Cast<ACL9PlayerController>(GetController());
	if ((PC != nullptr && PC->IsSpectating()) || IsExecuted() || ShouldUseDeathCamera())
	{
		return false;
	}
	// 치트(`cl9.damage`, 몬스터 없음)는 공격자가 null 이라 방향이 없다. 파괴된 공격자는 IsValid 로 걸러진다
	return IsHurtEventCurrent() && IsValid(HurtEvent.Attacker);
}

float ACL9Character::GetHurtDirectionAngle() const
{
	if (!HasHurtDirection())
	{
		return 0.f;
	}
	FVector ToAttacker = FVector(HurtEvent.From) - GetActorLocation();
	ToAttacker.Z = 0.f;
	if (ToAttacker.IsNearlyZero())
	{
		return 0.f;
	}

	// 카메라 요는 실제로 보고 있는 시점(POV — 피격 요 오프셋·셰이크 포함)이다. 카메라 매니저가 없으면 컨트롤 회전
	const APlayerController* PC = Cast<APlayerController>(GetController());
	const float CameraYaw = (PC != nullptr && PC->PlayerCameraManager != nullptr)
		? PC->PlayerCameraManager->GetCameraRotation().Yaw
		: GetControlRotation().Yaw;
	// 0 = 화면 위(앞), 시계 방향 양수 — UE 요는 위에서 볼 때 시계 방향으로 는다
	return FRotator::NormalizeAxis(ToAttacker.Rotation().Yaw - CameraYaw);
}

float ACL9Character::GetThrowWindupElapsed() const
{
	// 소유자는 키를 누른 로컬 시각으로 즉시 대기 자세에 들어간다(게이지와 같은 원천); 남은 복제된 호스트 시각을 읽는다
	if (IsLocallyControlled() && LocalThrowChargeStartTime >= 0.f)
	{
		return FMath::Max(0.f, GetWorld()->GetTimeSeconds() - LocalThrowChargeStartTime);
	}
	return ThrowChargeStartTime < 0.f ? -1.f : FMath::Max(0.f, GetServerTime() - ThrowChargeStartTime);
}

float ACL9Character::GetThrowElapsed() const
{
	return GetServerTime() - ThrowServerTime;
}

float ACL9Character::GetAimPitch() const
{
	// 전신 ABP(ABP_Unarmed) 전용 — 1인칭 팔은 카메라 자식이라 시선을 저절로 따르므로 이 값을 쓰지 않는다(2026-09-13).
	// 소유자는 컨트롤 회전을 직접 읽는다 — GetBaseAimRotation은 카메라 시점(POV)을 돌려주는데 거기에는 스태미나
	// 셰이크가 섞여 몸이 떨린다. 남의 폰은 GetBaseAimRotation이 호스트에서는 그 컨트롤러의 시점을, 클라에서는
	// 복제된 RemoteViewPitch(바이트 압축)를 준다.
	const float Pitch = IsLocallyControlled() ? GetControlRotation().Pitch : GetBaseAimRotation().Pitch;
	return FMath::Clamp(FRotator::NormalizeAxis(Pitch), -90.f, 90.f);
}

bool ACL9Character::IsRunning() const
{
	const UCL9CharacterMovement* Movement = GetCL9CharacterMovement();
	return Movement != nullptr && Movement->IsRunning();
}

float ACL9Character::GetGroundSpeed() const
{
	return GetVelocity().Size2D();
}

FVector ACL9Character::GetFootLocation() const
{
	return GetActorLocation() - FVector(0.f, 0.f, GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
}

void ACL9Character::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	// 착지 속도 문턱은 두지 않는다 — 제자리 점프도 링을 내고, 연타는 행의 MinInterval이 억제한다
	// (docs/systems/wave.md 소음 이벤트 `Land`). 판정 반경은 아직 없어 0이다.
	EmitNoise(ECL9NoiseSource::Land, GetFootLocation(), 0.f);
}

float ACL9Character::GetNoiseStepDistance(ECL9NoiseSource Source) const
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9WaveTable* Table = GameInstance != nullptr ? GameInstance->GetWaveTable() : nullptr;
	// 꺼진 행(기본값의 walk)은 간격도 0 — 사건 자체를 내지 않는다 (2026-09-16 사용자, HostReportNoise 입구와 같은 기준)
	const FCL9WaveInfo* Row = (Table != nullptr && Table->IsEnabled(Source)) ? Table->FindRow(Source) : nullptr;
	return Row != nullptr ? Row->StepDistance : 0.f;
}

void ACL9Character::EmitNoise(ECL9NoiseSource Source, const FVector& Location, float Radius)
{
	if (HasAuthority())
	{
		if (ACL9GameState* GameState = GetWorld()->GetGameState<ACL9GameState>())
		{
			GameState->HostReportNoise(Source, Location, Radius, GetPlayerState());
		}
		return;
	}

	// 소유 클라이언트의 예측 — 표현 전용이라 호스트 판정과 어긋나도 링 하나가 헛돌 뿐이고,
	// 몬스터가 무엇을 듣는가는 여전히 호스트의 사건만 정한다. 꺼진 행은 호스트 입구와 같은 기준으로 건너뛴다.
	if (IsLocallyControlled())
	{
		const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
		const UCL9WaveTable* Table = GameInstance != nullptr ? GameInstance->GetWaveTable() : nullptr;
		UCL9WaveRenderer* Renderer = UCL9WaveRenderer::Get(this);
		if (Table != nullptr && Table->IsEnabled(Source) && Renderer != nullptr)
		{
			Renderer->PlayWave(Source, Location, GetPlayerState(), true);
		}
	}
}

void ACL9Character::TickStepNoise()
{
	const bool bHost = HasAuthority();
	if (!bHost && !IsLocallyControlled())
	{
		return;
	}

	const UCL9CharacterMovement* Movement = GetCL9CharacterMovement();
	const bool bRunning = Movement != nullptr && Movement->IsRunning();
	if (bHost && bRunning)
	{
		// 줍기가 조용한지 시끄러운지를 가르는 기준 시각 (docs/plan/data/rules.md 소리 — RecentRunTime)
		LastRunServerTime = GetServerTime();
	}

	const FVector Foot = GetFootLocation();
	if (!LastStepLocation.IsSet())
	{
		LastStepLocation = Foot;
		return;
	}
	const float Moved = FVector::Dist2D(LastStepLocation.GetValue(), Foot);
	LastStepLocation = Foot;

	// 땅을 딛고 움직일 때만 걸음이다 — 공중에서는 발이 바닥을 치지 않는다(착지는 Landed가 낸다)
	if (Moved <= 0.f || Movement == nullptr || !Movement->IsMovingOnGround())
	{
		return;
	}

	const ECL9NoiseSource StepSource = bRunning ? ECL9NoiseSource::Run : ECL9NoiseSource::Walk;
	if (StepSource != LastStepSource)
	{
		// 달리기와 걷기는 간격이 다르다 — 바뀌는 순간 누적을 버려야 새 간격이 그대로 적용된다
		StepDistanceAccum = 0.f;
		LastStepSource = StepSource;
	}

	const float StepDistance = GetNoiseStepDistance(StepSource);
	if (StepDistance > 0.f)
	{
		StepDistanceAccum += Moved;
		if (StepDistanceAccum >= StepDistance)
		{
			StepDistanceAccum = 0.f;
			const UCL9RulesDef* Rules = GetRules();
			// 걷기의 어그로는 0이다 — 판정 반경이 있는 것은 달리기뿐이다
			const float Ruling = (bRunning && Rules != nullptr) ? Rules->RunNoiseRadius : 0.f;
			EmitNoise(StepSource, Foot, Ruling);
		}
	}

	// 수레는 사람 발밑이 아니라 수레 자리에서 난다 — 끄는 동안 걸음마다 링이 하나 더 핀다
	const float CartStepDistance = PullingCart != nullptr ? GetNoiseStepDistance(ECL9NoiseSource::Cart) : 0.f;
	if (CartStepDistance <= 0.f)
	{
		CartStepDistanceAccum = 0.f;
		return;
	}
	CartStepDistanceAccum += Moved;
	if (CartStepDistanceAccum >= CartStepDistance)
	{
		CartStepDistanceAccum = 0.f;
		EmitNoise(ECL9NoiseSource::Cart, PullingCart->GetActorLocation(), 0.f);
	}
}

void ACL9Character::HostCancelSwingWindup()
{
	if (HasAuthority())
	{
		SwingWindupServerTime = ServerTimeNever;
		// 예약된 휘두르기도 같이 — 손의 것이 바뀌는 경로마다 따로 취소 분기를 두지 않는다
		bSwingBuffered = false;
	}
}

ECL9HoldPose ACL9Character::GetHoldPose() const
{
	const UCL9ItemDef* Def = HeldItem != nullptr ? HeldItem->GetItemDef() : nullptr;
	return Def != nullptr ? Def->HoldPose : ECL9HoldPose::None;
}

void ACL9Character::HostPickUp(ACL9Item* Item, int32 Slot)
{
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (!HasAuthority() || CL9PlayerState == nullptr || Item == nullptr || Item->GetItemDef() == nullptr
		|| !CL9PlayerState->Slots.IsValidIndex(Slot))
	{
		return;
	}

	// 활성 칸이 차 있는데 다른 칸으로 줍는 경우: 손의 것을 먼저 데이터로 넣는다 — 주운 칸이 활성이 된다 (리썰 규칙)
	if (HeldItem != nullptr)
	{
		HostStowHeld();
	}

	// UI를 위한 미러(값 포함), 월드를 위한 액터 (docs/systems/inventory.md 상태 모델)
	FCL9CarriedItem& Mirror = CL9PlayerState->Slots[Slot];
	Mirror.ItemAlias = Item->GetAlias();
	Mirror.ItemSubAlias = Item->GetSubAlias();
	Mirror.Value = Item->GetValue();
	CL9PlayerState->ActiveSlot = static_cast<uint8>(Slot);
	HeldItem = Item;
	Item->SetOwner(this);

	// 손 소켓에 붙는다 — 위치는 행의 GripOffset이 정하고, CarryHeight는 칸에서 꺼내 스폰하는 높이에만 쓰인다(놓기는 조준점, Server_Drop)
	Item->OnPickedUp(this);
	CL9PlayerState->NotifyCarryChanged();

	// 줍는 소리 — 최근에 달렸으면(RecentRunTime) 달린 것과 같은 크기로 들린다
	// (docs/plan/data/rules.md 소리 · docs/systems/wave.md 소음 이벤트)
	if (const UCL9RulesDef* Rules = GetRules())
	{
		const bool bLoud = GetServerTime() - LastRunServerTime <= Rules->RecentRunTime;
		EmitNoise(bLoud ? ECL9NoiseSource::PickupLoud : ECL9NoiseSource::PickupQuiet, GetFootLocation(),
			bLoud ? Rules->PickupLoudRadius : Rules->PickupQuietRadius);
	}
}

void ACL9Character::HostWear(ACL9Item* Item)
{
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (!HasAuthority() || CL9PlayerState == nullptr || Item == nullptr || Item->GetItemDef() == nullptr || WornItem != nullptr)
	{
		return;
	}

	// UI를 위한 미러, 월드를 위한 액터 (docs/systems/inventory.md 상태 모델)
	CL9PlayerState->HeadItem.ItemAlias = Item->GetAlias();
	CL9PlayerState->HeadItem.ItemSubAlias = Item->GetSubAlias();
	CL9PlayerState->NotifyCarryChanged();
	WornItem = Item;
	Item->SetOwner(this);
	Item->OnWorn(this);
}

void ACL9Character::SetPullingCart(ACL9Cart* Cart)
{
	if (!HasAuthority())
	{
		return;
	}
	PullingCart = Cart;
}

void ACL9Character::HostConsumeHeldItem()
{
	if (!HasAuthority() || HeldItem == nullptr)
	{
		return;
	}

	HostCancelSwingWindup();
	HeldItem->Destroy();
	HeldItem = nullptr;
	HostClearActiveSlot();
}

bool ACL9Character::IsCarryingAnything() const
{
	if (HeldItem != nullptr || PullingCart != nullptr)
	{
		return true;
	}
	const ACL9PlayerState* CL9PlayerState = GetPlayerState<ACL9PlayerState>();
	return CL9PlayerState != nullptr && CL9PlayerState->HasAnyCarriedItem();
}

void ACL9Character::OnDamage(int32 Amount, AActor* Source, float /*Knockback*/)
{
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (!HasAuthority() || CL9PlayerState == nullptr || CL9PlayerState->Hp <= 0)
	{
		return;
	}

	// 넘어진 동안은 무적이다 (2026-09-20 사용자) — 다른 닭의 덮침도, `cl9.damage` 치트도 같은 문이라 우회가 없다.
	// HP·넘어짐 시계·피격 사건 전부 그대로다(몬스터 쪽은 넘어진 타깃에 애초에 예고·잡기를 하지 않는다).
	// 유일한 예외는 이 문을 아예 거치지 않는 처형의 HostKill 이다
	if (IsKnockedDown())
	{
		UE_LOG(LogCL9, Log, TEXT("'%s' hit by '%s' while knocked down — ignored (invulnerable until getting up)."),
			*GetNameSafe(this), *GetNameSafe(Source));
		return;
	}

	HostApplyDamage(Amount, Source, TEXT("caught by"));
}

void ACL9Character::HostKill(AActor* Source)
{
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (!HasAuthority() || CL9PlayerState == nullptr || CL9PlayerState->Hp <= 0)
	{
		return;
	}
	// 넘어짐 무적을 거치지 않는다 — 처형의 결과는 진입에서 정해졌다 (D3). 남은 HP 전부가 타격량이라 같은 경로로 0 이 된다
	HostApplyDamage(CL9PlayerState->Hp, Source, TEXT("executed by"));
}

void ACL9Character::HostApplyDamage(int32 Amount, AActor* Source, const TCHAR* Verb)
{
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (CL9PlayerState == nullptr)
	{
		return;
	}

	CL9PlayerState->Hp = FMath::Max(CL9PlayerState->Hp - Amount, 0);
	CL9PlayerState->NotifyHpChanged();

	// 짐을 흘리는 것은 죽을 때뿐이다 (2026-09-15 사용자) — HP가 남는 타격은 넘어뜨리기만 하고, 손에 든 것과
	// 퀵슬롯 네 칸은 그대로 둔다. 일어나면 들고 있던 그 물건을 그대로 들고 있다. 수레를 놓고 차징·대기를
	// 취소하는 것은 두 경우 다 한다 — 넘어진 채 차징이 이어지면 안 된다.
	if (CL9PlayerState->Hp <= 0)
	{
		HostDropEverything();
	}
	else
	{
		HostCancelCarryActions();
	}

	// 쓰러져서 움직일 수 없다; HP 0이면 다음 로비가 되살릴 때까지 쓰러진 채로 있는다
	// (docs/systems/character.md 피격·넘어짐·죽음 — 부활은 없다).
	const float Duration = CL9PlayerState->Hp > 0
		? (CharacterDef ? CharacterDef->KnockdownTime : 1.5f)
		: TNumericLimits<float>::Max();
	KnockdownStartServerTime = GetServerTime();
	KnockdownEndServerTime = KnockdownStartServerTime + Duration;

	// 피격 사건 — 이번 넘어짐의 시각·공격자의 자리·번호. 공격자가 없으면(치트) 자기 자리라 방향이 없다
	HurtEvent.ServerTime = KnockdownStartServerTime;
	HurtEvent.From = Source != nullptr ? Source->GetActorLocation() : GetActorLocation();
	HurtEvent.Attacker = Source;
	++HurtEvent.Seq;

	UE_LOG(LogCL9, Log, TEXT("'%s' %s '%s': -%d HP → %d%s"),
		*GetNameSafe(this), Verb, *GetNameSafe(Source), Amount, CL9PlayerState->Hp,
		CL9PlayerState->Hp <= 0 ? TEXT(" (dead — down for the night)") : TEXT(""));

	if (CL9PlayerState->Hp <= 0)
	{
		// 데스캠 — 죽은 순간부터(처형 즉사·보통 사망 공통) 자기 시체를 본다. 호스트 자기 폰은 바로 적용하고, 클라는 HP 복제로
		// 따라온다(PlayerState 를 지금 보내 뷰 타깃 RPC 와 같은 틱에 가게). 처형 중이었으면 몬스터 카메라에서 데스캠으로 블렌드
		// (HostBeginExecuted 의 SetViewTargetWithBlend(Executioner) 짝 — D6 의 "죽은 폰으로 되돌리지 않는다" 번복, 2026-09-20 밤).
		// 보통 사망은 뷰 타깃이 이미 자기 폰이라 엔진이 아무것도 하지 않는다 = 1인칭에서 데스캠으로 즉시 전환.
		// 오프셋 0 롤백이면 옛대로 — 뷰 타깃도 건드리지 않는다(처형이면 몬스터 카메라에 머문다)
		CL9PlayerState->ForceNetUpdate();
		ApplyDeathCamera();
		if (bDeathCameraOn)
		{
			if (ACL9PlayerController* PC = Cast<ACL9PlayerController>(GetController()))
			{
				const UCL9RulesDef* Rules = GetRules();
				PC->SetViewTargetWithBlend(this, Rules != nullptr ? Rules->DeathCameraBlend : 0.f);
			}
		}

		// 흐름 판정(전멸, 관전)은 게임 모드의 몫이다 — 호스트에서만 존재하므로 여기서 호출한다. 전멸이면 이 안에서 로비 부활까지
		// 이어질 수 있다(DawnLength 0) — 그때는 HostRevive 가 데스캠을 도로 끈다
		if (ACL9GameMode* GameMode = GetWorld()->GetAuthGameMode<ACL9GameMode>())
		{
			GameMode->HandlePlayerDeath(this);
		}
	}
}

void ACL9Character::HostHealFull()
{
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (!HasAuthority() || CL9PlayerState == nullptr || CL9PlayerState->MaxHp <= 0)
	{
		return;
	}

	CL9PlayerState->Hp = CL9PlayerState->MaxHp;
	CL9PlayerState->NotifyHpChanged();
}

void ACL9Character::HostRevive()
{
	if (!HasAuthority() || !IsDead())
	{
		return;
	}

	HostHealFull();
	KnockdownEndServerTime = 0.f;
	// 데스캠을 끄고 1인칭·팔을 되돌린다 — 뷰 타깃은 이미 자기 폰(관전 중이었으면 ReviveDead 가 되돌린다)
	ApplyDeathCamera();
	UE_LOG(LogCL9, Log, TEXT("'%s' revived at the truck."), *GetNameSafe(this));
}

void ACL9Character::HostBeginExecuted(ACL9Monster* Executioner)
{
	if (!HasAuthority() || Executioner == nullptr)
	{
		return;
	}

	ExecutedBy = Executioner;
	ExecuteStartServerTime = GetServerTime();
	// 무력화된 사람은 수레를 끌 수 없다 — 진입 순간 놓는다(던지기 차징·휘두르기 대기도 함께 취소). 짐은 죽는 순간 흘린다
	// (2026-09-20 리드 확인 ①: 수레는 진입 순간, 짐은 HostKill 의 HostDropEverything)
	HostCancelCarryActions();
	ApplyExecuted();

	ACL9PlayerController* PC = Cast<ACL9PlayerController>(GetController());

	// 두 리그(처형자 클립·피해자 클립)가 정렬되도록 피해자 요를 처형자에 맞춘다(2026-09-21 사용자 결정, Run 2 2인 클립의 가정):
	// 잡히면 처형인 종(bExecuteOnCatch — 돼지 클립은 "정면에서 박힘")은 처형자를 정면으로, 앵커 텔레포트 처형(스토커 — 너구리 클립은
	// "등 뒤")은 처형자를 등지게. 갈림은 표 열이지 종 이름이 아니다. 스토커 쪽은 앵커가 피해자 전방 기준이라 서버 요는 이미 정확히 등이고,
	// 클라 요(마지막 ServerMove 뒤의 마우스)와의 차이만 없앤다. 피치·롤은 0(캡슐 요만). 처형자가 같은 XY 위에 있으면 요는 그대로
	FVector ToExecutioner = Executioner->GetActorLocation() - GetActorLocation();
	ToExecutioner.Z = 0.f;
	if (!ToExecutioner.IsNearlyZero())
	{
		const UCL9MonsterDef* Def = Executioner->GetMonsterDef();
		const bool bFaceExecutioner = Def != nullptr && Def->bExecuteOnCatch;
		const float Yaw = (bFaceExecutioner ? ToExecutioner : -ToExecutioner).Rotation().Yaw;

		// 피해자는 자율 프록시라 서버의 SetActorRotation 만으로는 되돌아간다 — CMC ServerMove_HandleMoveData 가 매 이동마다
		// SetControlRotation(클라 컨트롤 회전) → UpdateRotation → FaceRotation 으로 캡슐 요를 컨트롤 요(bUseControllerRotationYaw)에 다시 맞춘다.
		// 그래서 컨트롤 회전을 양쪽에 맞춘다: 서버 SetControlRotation(즉시 — 조준·GetBaseAimRotation 이 읽는 값) + ClientSetRotation(엔진 Client RPC —
		// 클라의 SetControlRotation + Pawn->FaceRotation; 리슨 호스트 자기 폰은 로컬 호출이라 곧바로 실행된다). 이후 마우스는 LookInput 의
		// IsExecuted 게이트(D2)와 내려간 게임플레이 IMC 가 막는다. 클라의 RPC 도착 전에 이미 보낸 ServerMove 한두 개가 서버 요를 잠깐 옛 값으로
		// 되돌릴 수 있으나 RPC 가 닿으면 다시 맞는다. 피치는 지금 값 유지(처형 카메라를 보는 동안은 어차피 보이지 않는다)
		SetActorRotation(FRotator(0.f, Yaw, 0.f));
		if (PC != nullptr)
		{
			const FRotator ControlRotation(PC->GetControlRotation().Pitch, Yaw, 0.f);
			PC->SetControlRotation(ControlRotation);
			PC->ClientSetRotation(ControlRotation);
		}
	}

	// 피해자 시점을 처형자의 카메라로 — 관전과 같은 서버 SetViewTarget 경로(엔진이 소유 클라에 ClientSetViewTarget 전달, 리슨 호스트
	// 자기 폰도 같은 호출). 블렌드 길이는 규칙 표
	if (PC != nullptr)
	{
		const UCL9RulesDef* Rules = GetRules();
		PC->SetViewTargetWithBlend(Executioner, Rules != nullptr ? Rules->ExecuteCameraBlend : 0.f);
	}
	ForceNetUpdate();
	UE_LOG(LogCL9, Log, TEXT("'%s' is being executed by '%s'."), *GetNameSafe(this), *GetNameSafe(Executioner));
}

void ACL9Character::HostEndExecuted()
{
	if (!HasAuthority() || (!IsExecuted() && ExecutedBy == nullptr))
	{
		return;
	}

	ExecuteStartServerTime = -1000.f;
	ExecutedBy = nullptr;
	ApplyExecuted();

	// 관전 중이면 뷰 타깃은 관전 흐름의 것이다(로비 부활이 자기 폰으로 되돌린다). 아니면 — 살아 있든, 전멸로 관전 대상이 없었든 —
	// 곧 사라질 몬스터를 보고 있을 수 없으니 자기 폰으로 (D9). 죽어서 데스캠이 켜져 있으면 뷰 타깃은 이미 자기 폰(HostApplyDamage)이라
	// 이 호출은 아무것도 바꾸지 않고, 몸·팔 토글도 ApplyThirdPersonMeshes 가 데스캠을 보고 유지한다
	ACL9PlayerController* PC = Cast<ACL9PlayerController>(GetController());
	if (PC != nullptr && !PC->IsSpectating())
	{
		PC->SetViewTarget(this);
	}
	ForceNetUpdate();
	UE_LOG(LogCL9, Log, TEXT("'%s' execution state cleared."), *GetNameSafe(this));
}

void ACL9Character::OnRep_ExecuteStartServerTime()
{
	ApplyExecuted();
}

void ACL9Character::ApplyExecuted()
{
	const bool bExecuted = IsExecuted();

	// 소유 클라(리슨 호스트의 자기 폰 포함): 게임플레이 IMC 를 내리고 올린다 — 화면이 열릴 때와 같은 스위치. 관전 IMC 는 건드리지 않는다
	if (IsLocallyControlled())
	{
		if (ACL9PlayerController* PC = Cast<ACL9PlayerController>(GetController()))
		{
			PC->SetGameplayInputEnabled(!bExecuted);
		}
	}

	// 처형 카메라(몬스터)에서 자기 몸이 보이고 팔(과 팔에 붙은 손의 아이템)은 사라진다 — 데스캠과 같은 토글
	ApplyThirdPersonMeshes();
}

void ACL9Character::ApplyThirdPersonMeshes()
{
	// 죽은 뒤 관전 전환 때 몸이 보이는 채로 남아도 관전은 남의 시점이라 무방하다. 처형 표시가 걷혀도(HostEndExecuted) 데스캠이
	// 켜져 있으면 3인칭 그대로 — 부활이 데스캠을 끄면서 1인칭으로 돌아온다
	const bool bThirdPerson = IsExecuted() || bDeathCameraOn;
	if (USkeletalMeshComponent* BodyMesh = GetMesh())
	{
		// WorldSpaceRepresentation 이면 프록시가 bOwnerNoSee 를 강제로 true 로 덮어써서(PrimitiveSceneProxy.cpp) OwnerNoSee 토글만으로는
		// 몸이 안 보인다(그림자만 남는다) — 3인칭 동안은 타입을 None 으로 내려야 한다
		BodyMesh->SetFirstPersonPrimitiveType(bThirdPerson ? EFirstPersonPrimitiveType::None : EFirstPersonPrimitiveType::WorldSpaceRepresentation);
		BodyMesh->SetOwnerNoSee(!bThirdPerson);
	}
	if (FirstPersonMesh != nullptr)
	{
		FirstPersonMesh->SetVisibility(!bThirdPerson, /*bPropagateToChildren*/ true);
	}
}

bool ACL9Character::ShouldUseDeathCamera() const
{
	// 롤백: 오프셋 (0,0,0) 이면 데스캠 없음 — 옛 동작(넘어짐 카메라가 바닥으로, 처형이면 몬스터 카메라에 머묾) 그대로
	const UCL9RulesDef* Rules = GetRules();
	return IsDead() && Rules != nullptr && !Rules->DeathCameraOffset.IsZero();
}

void ACL9Character::ApplyDeathCamera()
{
	bDeathCameraOn = ShouldUseDeathCamera();
	if (bDeathCameraOn)
	{
		// 자리·회전 모두 액터 공간에서 한 번 — 카메라 자리에서 캡슐 중심 위 DeathCameraLookAtZ 를 보는 회전. 캡슐에 붙어 있으니
		// 액터가 돌아도 시체는 화면의 같은 자리에 잡힌다
		const UCL9RulesDef* Rules = GetRules();
		const FVector Offset = Rules->DeathCameraOffset;
		const FVector LookAt(0.f, 0.f, Rules->DeathCameraLookAtZ);
		DeathCamera->SetRelativeLocationAndRotation(Offset, FRotationMatrix::MakeFromX(LookAt - Offset).Rotator());
	}
	// 반드시 하나만 활성 — 둘 다 켜져 있으면 CalcCamera 가 첫 것(1인칭)을 고른다
	FirstPersonCameraComponent->SetActive(!bDeathCameraOn);
	DeathCamera->SetActive(bDeathCameraOn);
	ApplyThirdPersonMeshes();
}

bool ACL9Character::IsKnockedDown() const
{
	return GetServerTime() < KnockdownEndServerTime;
}

bool ACL9Character::IsDead() const
{
	// alias가 적용되어 굴려지기 전까지 MaxHp는 0이다 — 실제로 굴려진 0만 죽음으로 친다
	const ACL9PlayerState* CL9PlayerState = GetPlayerState<ACL9PlayerState>();
	return CL9PlayerState != nullptr && CL9PlayerState->IsDead();
}

void ACL9Character::HostCancelCarryActions()
{
	if (!HasAuthority())
	{
		return;
	}

	// 넘어지면 수레에서 손을 놓고(수레는 제 속도로 미끄러지다 선다), 차징하던 던지기와 대기하던 휘두르기가 사라진다.
	if (PullingCart != nullptr)
	{
		PullingCart->StopPull(this);
	}
	ThrowChargeStartTime = -1.f;
	HostCancelSwingWindup();
}

void ACL9Character::HostDropEverything()
{
	const UCL9RulesDef* Rules = GetRules();
	ACL9PlayerState* CL9PlayerState = GetCL9PlayerState();
	if (Rules == nullptr || CL9PlayerState == nullptr)
	{
		return;
	}

	HostCancelCarryActions();

	// 발목 높이에서 옆으로 살짝 밀쳐서 흘린다 — 그다음부터는 실제 물리가 알아서
	// 처리하고, 낮은 원점 덕분에 깨지기 쉬운 아이템도 CrackSpeed 아래에 머물러서
	// 붙잡히면 짐이 다 박살나는 대신 흩어지기만 한다.
	ACL9GameMode* GameMode = GetWorld()->GetAuthGameMode<ACL9GameMode>();
	const float FloorZ = GetActorLocation().Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	auto Spill = [&](ACL9Item* Item)
	{
		const float ItemRadius = Item->GetItemDef() ? Item->GetItemDef()->Radius : 20.f;
		const FVector Sideways = FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f).Vector();
		const FVector Origin(GetActorLocation().X, GetActorLocation().Y, FloorZ + ItemRadius + 10.f);
		Item->OnThrown(Origin + Sideways * (GetCapsuleComponent()->GetScaledCapsuleRadius() + ItemRadius + 5.f),
			Sideways * FMath::FRandRange(120.f, 240.f));
	};

	const bool bHadCarry = (HeldItem != nullptr) || CL9PlayerState->HasAnyCarriedItem();
	const bool bActiveSpilledAsActor = HeldItem != nullptr;

	if (HeldItem != nullptr)
	{
		ACL9Item* Dropped = HeldItem;
		HeldItem = nullptr;
		Dropped->SetOwner(nullptr);
		Spill(Dropped);
	}

	// 네 칸 전부 흘린다 — 비활성 칸은 데이터뿐이라 저장된 값 그대로 다시 스폰해 흘린다. 활성 칸 번호는 그대로 둔다.
	for (int32 Slot = 0; Slot < CL9PlayerState->Slots.Num(); ++Slot)
	{
		FCL9CarriedItem& Stowed = CL9PlayerState->Slots[Slot];
		if (!Stowed.IsSet())
		{
			continue;
		}
		const bool bAlreadySpilled = Slot == CL9PlayerState->ActiveSlot && bActiveSpilledAsActor;
		if (!bAlreadySpilled && GameMode != nullptr)
		{
			if (ACL9Item* Spawned = GameMode->SpawnItem(Stowed.ItemAlias, Stowed.ItemSubAlias, FTransform(GetActorLocation()), Stowed.Value))
			{
				Spill(Spawned);
			}
		}
		Stowed = FCL9CarriedItem();
	}

	if (bHadCarry)
	{
		CL9PlayerState->NotifyCarryChanged();
	}
}

const UCL9RulesDef* ACL9Character::GetRules() const
{
	const UCL9GameInstance* GameInstance = GetGameInstance<UCL9GameInstance>();
	const UCL9DefRegistry* Registry = GameInstance ? GameInstance->GetDefRegistry() : nullptr;
	return Registry ? Registry->GetRules() : nullptr;
}

ACL9PlayerState* ACL9Character::GetCL9PlayerState() const
{
	return GetPlayerState<ACL9PlayerState>();
}

float ACL9Character::GetServerTime() const
{
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	return GameState ? GameState->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
}
