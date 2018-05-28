// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "TacticalWeaponState.generated.h"

UCLASS(DefaultToInstanced, EditInlineNew, CustomConstructor)
class TACTICALFPS_API UTacticalWeaponState : public UActorComponent
{
	GENERATED_BODY()

public:

	UTacticalWeaponState();

	FORCEINLINE virtual class ATacticalWeapon* GetWeapon() const;
	FORCEINLINE virtual class ATacticalCharacter* GetInventoryOwner() const;

	class FTimerManager& GetWorldTimerManager() const;

	virtual void BeginState(const UTacticalWeaponState* PrevState){}
	virtual void EndState(const UTacticalWeaponState* NextState){}

	virtual void BeginFiring(){}
	virtual void EndFiring(){}

	virtual void Equip(){}
	virtual void Unequip(){}

	virtual void Reload(){}
	virtual void CancelReload(){}

	virtual void BeginWeaponObstructed(){}
	virtual void EndWeaponObstructed(){}

	virtual bool IsFiring() const { return false; }
	virtual bool IsReloading() const { return false; }
	virtual bool IsChangingWeapon() const { return false; }

	virtual void UpdateTiming(){}

	virtual bool CanUseGadget() const { return false; }


	bool bDoObstructionCheck;
};