// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalAmmoType.h"




UTacticalAmmoType::UTacticalAmmoType()
	: Super()
{
	ArmorPenetration = 0.f;
	Damage = 0.f;
	DamageType = UDamageType::StaticClass();
	RangeModifier = 1.f;

	bAllowSurfacePenetrations = false;
	SurfacePenetrationDamageModifier = 0.5f;

	bIsBuckshot = false;
	NumBuckshots = 1;
	ScatterAngle = 30.f;

	PreviewBoxMesh = nullptr;
	PreviewBulletMesh = nullptr;

	ImpactEffect = nullptr;
}
