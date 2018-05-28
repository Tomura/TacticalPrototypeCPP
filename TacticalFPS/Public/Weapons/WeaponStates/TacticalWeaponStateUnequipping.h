// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "TacticalWeaponState_WPN.h"
#include "TacticalWeaponStateUnequipping.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalWeaponStateUnequipping : public UTacticalWeaponState_WPN
{
	GENERATED_BODY()

public:
	UTacticalWeaponStateUnequipping();


	virtual void BeginState(const UTacticalWeaponState* PrevState) override;
	virtual void EndState(const UTacticalWeaponState* NextState) override;

	virtual void Equip() override;
	virtual bool IsChangingWeapon() const override { return true; }

	void UnequipFinished();

	float PartialUnequipTime;

protected:
	FTimerHandle TimerUnequip;
};
