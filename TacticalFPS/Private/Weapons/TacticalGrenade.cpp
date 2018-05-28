// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "Net/UnrealNetwork.h"
#include "TacticalProjectile.h"
#include "TacticalGrenade.h"
#include "TacticalCharacter.h"

ATacticalGrenade::ATacticalGrenade(const FObjectInitializer& OI)
	: Super(OI)
{
	NumCharges = 3;
}






