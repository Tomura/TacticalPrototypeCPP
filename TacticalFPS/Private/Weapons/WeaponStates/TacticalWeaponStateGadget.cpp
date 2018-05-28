// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#include "TacticalFPS.h"
#include "TacticalCharacter.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponAttachment_Weapon.h"
#include "TacticalWeaponStateGadget.h"




FORCEINLINE class ATacticalWeaponAttachment_Weapon* UTacticalWeaponStateGadget::GetAttachment() const
{
	return GetOwner() ? Cast<ATacticalWeaponAttachment_Weapon>(GetOwner()) : nullptr;
}

FORCEINLINE class ATacticalWeapon* UTacticalWeaponStateGadget::GetWeapon() const
{
	return GetAttachment()->GetWeaponOwner();
}

FORCEINLINE class ATacticalCharacter* UTacticalWeaponStateGadget::GetInventoryOwner() const
{
	return GetWeapon()->GetInventoryOwner();
}

