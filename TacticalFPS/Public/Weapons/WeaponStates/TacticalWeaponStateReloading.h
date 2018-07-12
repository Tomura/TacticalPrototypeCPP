// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "TacticalWeaponState_WPN.h"
#include "TacticalWeaponStateReloading.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalWeaponStateReloading : public UTacticalWeaponState_WPN
{
	GENERATED_BODY()

public:
	UTacticalWeaponStateReloading();

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	virtual void BeginState(const UTacticalWeaponState* PrevState) override;
	virtual void EndState(const UTacticalWeaponState* NextState) override;

	virtual bool IsReloading() const override { return true; }

	virtual void CancelReload() override;
	void ReloadFinished();

	void SwapMagazine();
	void ContinueReloading();

	virtual void Unequip() override;
	
	float GetReloadTimeElapsed() const;

protected:
	FTimerHandle TimerReload;
	bool bPendingStop;
};

