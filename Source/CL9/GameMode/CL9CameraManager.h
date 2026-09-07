// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "CL9CameraManager.generated.h"

/**
 *  기본 1인칭 카메라 매니저.
 *  최소/최대 시야각(피치)을 제한한다.
 */
UCLASS()
class ACL9CameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:

	/** 생성자 */
	ACL9CameraManager();
};
