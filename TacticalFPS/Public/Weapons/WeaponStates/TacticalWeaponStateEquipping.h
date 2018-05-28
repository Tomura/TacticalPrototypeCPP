// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "TacticalWeaponState_WPN.h"
#include "TacticalWeaponStateEquipping.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalWeaponStateEquipping : public UTacticalWeaponState_WPN
{
	GENERATED_BODY()

public:
	UTacticalWeaponStateEquipping();

	virtual void BeginState(const UTacticalWeaponState* PrevState) override;
	virtual void EndState(const UTacticalWeaponState* NextState) override;

	virtual bool IsChangingWeapon() const override { return true; }

	void EquipFinished();

	virtual void Unequip() override;
	
	float PartialEquipTime;

protected:
	FTimerHandle TimerEquip;
};
