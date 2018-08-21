// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "TacticalWeaponState_WPN.h"
#include "TacticalWeaponStateFiring.generated.h"

/**
 * 
 */


UCLASS()
class TACTICALFPS_API UTacticalWeaponStateFiring : public UTacticalWeaponState_WPN
{
	GENERATED_BODY()

public:
	UTacticalWeaponStateFiring();

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

	UFUNCTION(unreliable, NetMulticast)
	void NetMulti_SimulateFire();
	virtual void NetMulti_SimulateFire_Implementation();


	void TryRefire();
	virtual bool ShouldRefire() const;

	virtual void Unequip() override
	{
		GetWeapon()->bPendingUnequip = true;
	}

	virtual bool IsFiring() const { return true; }


	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSimulateFireEvent);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSimulateEmptyEvent);

	FSimulateFireEvent& GetSimulateFireEvent() { return SimulateFireEvent; }
	FSimulateEmptyEvent& GetSimulateEmptyEvent() { return SimulateEmptyEvent; }

	UFUNCTION(Category = "Weapon", BlueprintCallable)
	virtual int32 GetShotCount() const { return ShotCount; }
	TSubclassOf<class UTacticalAmmoType> GetAmmoType() const;

	virtual void OnRicochet(const FHitResult& Hit, const FVector& TraceDir);

protected:
	FTimerHandle TimerRefire;
	FTimerHandle TimerTrigger;

	int32 ShotCount;

	UPROPERTY(Category = "Weapon", BlueprintAssignable)
	FSimulateFireEvent SimulateFireEvent;
	UPROPERTY(Category = "Weapon", BlueprintAssignable)
		FSimulateEmptyEvent SimulateEmptyEvent;
};
