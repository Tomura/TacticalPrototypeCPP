// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalCharacter.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponState.h"
#include "TacticalWeaponStateInactive.h"


UTacticalWeaponStateInactive::UTacticalWeaponStateInactive()
	: Super()
{

}


void UTacticalWeaponStateInactive::BeginState(const UTacticalWeaponState* PrevState)
{
	Super::BeginState(PrevState);
}

void UTacticalWeaponStateInactive::Equip()
{
	GetWeapon()->GotoState(GetWeapon()->StateEquipping);
}

