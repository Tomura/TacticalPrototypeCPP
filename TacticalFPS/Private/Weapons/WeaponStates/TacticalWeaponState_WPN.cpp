// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#include "TacticalFPS.h"
#include "TacticalCharacter.h"
#include "TacticalInventoryComponent.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponState_WPN.h"




FORCEINLINE class ATacticalWeapon* UTacticalWeaponState_WPN::GetWeapon() const
{
	return GetOwner() ? Cast<ATacticalWeapon>(GetOwner()) : nullptr;
}

FORCEINLINE class ATacticalCharacter* UTacticalWeaponState_WPN::GetInventoryOwner() const
{
	return GetWeapon()->GetInventoryOwner();
}

