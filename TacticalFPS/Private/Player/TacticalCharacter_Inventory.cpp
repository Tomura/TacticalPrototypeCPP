// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalCharacter.h"
#include "TacticalInventory.h"
#include "TacticalWeapon.h"
#include "TacticalGrenade.h"
#include "TacticalCharacterMovement.h"
#include "TacticalLoadout.h"
#include "TacticalPlayerController.h"
#include "TacticalInventoryComponent.h"






void ATacticalCharacter::WeaponUnequipped()
{
	GetInventory()->ChangeWeapon();
}


bool ATacticalCharacter::CanSwitchWeapon() const
{
	return IsAlive();
}

void ATacticalCharacter::TrySwitchWeapon(class ATacticalWeapon* DesiredWeapon)
{
	// used to have a null check for DesiredWeapon
	if (!(GetTacticalMovement() && (GetTacticalMovement()->Stance == ETacticalStance::STANCE_Prone || GetTacticalMovement()->IsChangingStance())))
	{
		StopAim();
		StopSprint();
		GetInventory()->SetCurrentWeapon(DesiredWeapon);
	}
}

