// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "TacticalWeaponState_WPN.h"
#include "TacticalWeaponStateInactive.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalWeaponStateInactive : public UTacticalWeaponState_WPN
{
	GENERATED_BODY()

public:
	UTacticalWeaponStateInactive();
	virtual void BeginState(const UTacticalWeaponState* PrevState) override;
	virtual void Equip() override;
	
};
