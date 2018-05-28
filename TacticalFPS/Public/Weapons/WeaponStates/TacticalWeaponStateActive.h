// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "TacticalWeaponState_WPN.h"
#include "TacticalWeaponStateActive.generated.h"

UCLASS(CustomConstructor)
class TACTICALFPS_API UTacticalWeaponStateActive : public UTacticalWeaponState_WPN
{
	GENERATED_BODY()

public:

	UTacticalWeaponStateActive()
	: Super()
	{
		bDoObstructionCheck = true;
	}

	virtual void BeginState(const UTacticalWeaponState* PrevState) override;
	virtual void BeginFiring() override;

	virtual void Reload() override;

	virtual void Unequip() override;

	virtual void BeginWeaponObstructed() override;


	virtual bool CanUseGadget() const override { return true; }
};