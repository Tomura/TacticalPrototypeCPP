// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponStates/TacticalWeaponStateGadget.h"
#include "TacticalWeaponStateGadgetFiring.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalWeaponStateGadgetFiring : public UTacticalWeaponStateGadget
{
	GENERATED_BODY()
public:
	UTacticalWeaponStateGadgetFiring();

	virtual void BeginState(const UTacticalWeaponState* PrevState) override;
	virtual void EndState(const UTacticalWeaponState* NextState) override;

	virtual void EndFiring() override;

	virtual void BeginWeaponObstructed() override;

	virtual void Reload() override;

	virtual void FireAmmunition();
	virtual void TimeWeaponRefire();

	virtual void Fire();
	virtual void FireInstant();
	virtual void FireProjectile();
	void FireCustom();

	void TryRefire();
	virtual bool ShouldRefire() const;

	virtual void Unequip() override
	{
		GetWeapon()->bPendingUnequip = true;
	}

	virtual bool IsFiring() const { return true; }

	virtual int32 GetShotCount() const { return ShotCount; }

protected:
	FTimerHandle TimerRefire;
	FTimerHandle TimerTrigger;

	int32 ShotCount;
	
};
