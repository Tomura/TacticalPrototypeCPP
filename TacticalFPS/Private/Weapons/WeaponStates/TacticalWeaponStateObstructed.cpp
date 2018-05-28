// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalCharacter.h"
#include "TacticalInventoryComponent.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponStateObstructed.h"


UTacticalWeaponStateObstructed::UTacticalWeaponStateObstructed()
	: Super()
{
	bDoObstructionCheck = true;
}

void UTacticalWeaponStateObstructed::BeginState(const UTacticalWeaponState* PrevState)
{
	if (GetWeapon()->GetInventoryComponent()->PendingWeapon != NULL && GetWeapon()->bPendingUnequip)
	{
		Unequip();
	}
}

void UTacticalWeaponStateObstructed::EndWeaponObstructed()
{
	GetWeapon()->GotoState(GetWeapon()->StateActive);
}