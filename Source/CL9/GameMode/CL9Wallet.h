// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CL9Wallet.generated.h"

/**
 * 바깥 재화(화면 이름 "이더") — 플레이어 개인의 것이라 판·판 세이브와 무관한 **각자 기계의 파일**에 둔다
 * (docs/goal/loop-7f-out-game-currency.md, loop-rework 결정 1). 판 세이브·기록(`CL9Records`)은 호스트 것이고, 이것은 모든 머신에 하나씩이다.
 */
UCLASS()
class UCL9WalletSave : public USaveGame
{
	GENERATED_BODY()

public:
	static constexpr int32 CurrentVersion = 1;

	UPROPERTY()
	int32 WalletVersion = CurrentVersion;

	/** 보유 이더(바깥 재화) — 0 아래로 내려가지 않는다 */
	UPROPERTY()
	int32 Ether = 0;
};

/**
 * 이 머신의 지갑 — 게임 인스턴스마다 하나(PIE 인스턴스마다 따로). 시작할 때 파일을 읽고, 호스트가 보낸 지급
 * (`ACL9PlayerController::Client_WalletGrant`)을 더해 저장한다. 새벽 화면·타이틀이 읽는다.
 */
UCLASS()
class UCL9WalletSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** 이번 판 끝에 받은 지급을 더한다(0 도 받는다 — "이번 판"의 값을 갱신하려고). 0 보다 크면 파일에 저장한다 */
	void ApplyGrant(int32 Amount);

	/** 치트용: 잔액을 Delta 만큼 바꾼다(0 아래로는 안 내려간다). 저장한다. 이번 판 지급 값은 건드리지 않는다 */
	void CheatAdd(int32 Delta);

	int32 GetEther() const { return Ether; }

	/** 이번 판 끝에 받은 지급 — 런타임 값(파일에 없다). 아직 받은 적 없으면 0 */
	int32 GetLastGrant() const { return LastGrant; }

	/** 저장 슬롯 이름 — `CL9Wallet`, PIE 인스턴스 1 이상이면 `CL9Wallet_PIE<n>`(한 머신의 호스트·클라가 파일을 나눠 쓰지 않게) */
	FString GetSlotName() const;

	/** 잔액이나 이번 판 지급이 바뀌면(이 머신) */
	FSimpleMulticastDelegate OnWalletChanged;

private:
	void Save() const;

	int32 Ether = 0;
	int32 LastGrant = 0;
};
