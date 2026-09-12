// Copyright Epic Games, Inc. All Rights Reserved.

#include "Items/CL9LampItem.h"

void ACL9LampItem::LoadData()
{
	Super::LoadData();

	// 램프는 세상에 나오는 순간부터 켜져 있다 — 바닥에 스폰되든 상점에서 떨어지든 누가 집기를
	// 기다리지 않는다. 호스트만 스위치를 만지고, 나머지 머신은 복제로 따라온다.
	SetLightOn(true);
}

void ACL9LampItem::OnReleased()
{
	// 내려놓거나 던져도 그대로 켜 둔다 — 세워 두고 주변을 밝히는 게 이 물건의 쓸모다
}
