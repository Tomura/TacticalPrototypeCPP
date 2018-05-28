// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalCharacter.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponState.h"
#include "TacticalWeaponStateEquipping.h"
#include "TacticalWeaponStateUnequipping.h"


UTacticalWeaponStateUnequipping::UTacticalWeaponStateUnequipping()
	: Super()
{

}


void UTacticalWeaponStateUnequipping::BeginState(const UTacticalWeaponState* PrevState)
{
	const UTacticalWeaponStateEquipping* PrevEquip = Cast<UTacticalWeaponStateEquipping>(PrevState);
	float UnequipTime = GetWeapon()->UnequipTime;

	GetWeapon()->SwitchLight(false);

	// Remove Pending Unequips
	GetWeapon()->bPendingUnequip = false;

	// Simulate Unequip
	GetWeapon()->LocalSimulateUnequip();

	// Replicate Event to Clients
	if (GetOwnerRole() == ROLE_Authority && !(GetNetMode() == ENetMode::NM_Standalone))
	{
		GetWeapon()->NetMulti_UnequipWeapon();
	}

	// Setup Timer or Finish it if time is 0
	if (UnequipTime <= 0.f)
	{
		UnequipFinished();
	}
	else
	{
		GetWorldTimerManager().SetTimer(TimerUnequip, this, &UTacticalWeaponStateUnequipping::UnequipFinished, UnequipTime);
	}

	UE_LOG(LogInventory, Log, TEXT("Unequipping %s"), *GetName());
}

void UTacticalWeaponStateUnequipping::EndState(const UTacticalWeaponState* NextState)
{
	GetWorldTimerManager().ClearTimer(TimerUnequip);
}

void UTacticalWeaponStateUnequipping::UnequipFinished()
{
	GetWeapon()->GotoState(GetWeapon()->StateInactive);
	GetWeapon()->WeaponUnequipped();
}

void UTacticalWeaponStateUnequipping::Equip()
{
	PartialUnequipTime = FMath::Max(0.001f, GetWorldTimerManager().GetTimerElapsed(TimerUnequip));
	GetWeapon()->GotoState(GetWeapon()->StateEquipping);
}