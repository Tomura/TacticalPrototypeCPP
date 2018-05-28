// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "TacticalWeaponStateActive.h"
#include "TacticalWeaponStateObstructed.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalWeaponStateObstructed : public UTacticalWeaponStateActive
{
	GENERATED_BODY()

public:
	UTacticalWeaponStateObstructed();

	virtual void BeginState(const UTacticalWeaponState* PrevState) override;

	virtual void BeginWeaponObstructed() override {};
	virtual void BeginFiring() override {};


	virtual void EndWeaponObstructed() override;
};
