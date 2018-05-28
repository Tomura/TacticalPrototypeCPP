// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
//#include "TacticalWeapon.h"
#include "TacticalWeaponAnimInstance.h"




UTacticalWeaponAnimInstance::UTacticalWeaponAnimInstance(const FObjectInitializer& OI)
	: Super(OI)
{
	FiringMode = EWeaponFiringMode::FM_None;
	AmmoLoaded = 0;
}

void UTacticalWeaponAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	ATacticalWeapon* WPN = GetOwningWeapon();
	if (WPN)
	{
		FiringMode = WPN->GetFiringMode();
		AmmoLoaded = WPN->GetAmmoLoaded();
	}
}

 ATacticalWeapon* UTacticalWeaponAnimInstance::GetOwningWeapon() const
{
	if (GetOwningActor())
	{
		return Cast<ATacticalWeapon>(GetOwningActor());
	}
	return nullptr;
}
