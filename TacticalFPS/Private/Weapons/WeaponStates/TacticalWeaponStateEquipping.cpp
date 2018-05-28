// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalCharacter.h"
#include "TacticalInventoryComponent.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponState.h"
#include "TacticalWeaponStateUnequipping.h"
#include "TacticalWeaponStateEquipping.h"


UTacticalWeaponStateEquipping::UTacticalWeaponStateEquipping()
	: Super()
{

}


void UTacticalWeaponStateEquipping::BeginState(const UTacticalWeaponState* PrevState)
{
	const UTacticalWeaponStateUnequipping* PrevEquip = Cast<UTacticalWeaponStateUnequipping>(PrevState);
	float EquipTime = GetWeapon()->EquipTime;

	// Simulate
	GetWeapon()->LocalSimulateEquip();

	// Replicate Event to Clients
	if (GetOwnerRole() == ROLE_Authority && !(GetNetMode() == ENetMode::NM_Standalone))
	{
		GetWeapon()->NetMulti_EquipWeapon();
	}

	if (EquipTime <= 0.f)
	{
		EquipFinished();
	}
	else
	{
		GetWorldTimerManager().SetTimer(TimerEquip, this, &UTacticalWeaponStateEquipping::EquipFinished, EquipTime);
	}
	if (GetWeapon()->GetInventoryComponent())
	{
		GetWeapon()->GetInventoryComponent()->WeaponChanged();
	}
}

void UTacticalWeaponStateEquipping::EndState(const UTacticalWeaponState* NextState)
{
	GetWorldTimerManager().ClearTimer(TimerEquip);
	GetWeapon()->AttachGunToHand();
}

void UTacticalWeaponStateEquipping::EquipFinished()
{
	GetWeapon()->GotoState(GetWeapon()->StateActive);
}

void UTacticalWeaponStateEquipping::Unequip()
{
	PartialEquipTime = FMath::Max(0.001f, GetWorldTimerManager().GetTimerElapsed(TimerEquip));
	GetWeapon()->GotoState(GetWeapon()->StateUnequipping);
}

