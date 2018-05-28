// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalGameplaySettings.h"




UTacticalGameplaySettings::UTacticalGameplaySettings(const FObjectInitializer& OI)
	:Super(OI)
{
	DefaultFOV = 65.f;

	ADSMouseSensitivityMultiplier = FVector2D(0.75f, 0.75f);
	ADSStickSensitivityMultiplier = FVector2D(0.5f, 0.5f);

	WeaponBobMultiplier = 1.f;
	WeaponLagMultiplier = 1.f;


	bToggleAim = true;
	bToggleCrouch = true;
}
