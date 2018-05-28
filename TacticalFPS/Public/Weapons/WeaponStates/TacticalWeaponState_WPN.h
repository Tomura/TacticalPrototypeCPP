// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponStates/TacticalWeaponState.h"
#include "TacticalWeaponState_WPN.generated.h"

/**
 * 
 */
UCLASS(abstract)
class TACTICALFPS_API UTacticalWeaponState_WPN : public UTacticalWeaponState
{
	GENERATED_BODY()
public:
	UTacticalWeaponState_WPN() : Super() {}

	FORCEINLINE virtual class ATacticalWeapon* GetWeapon() const override;
	FORCEINLINE virtual class ATacticalCharacter* GetInventoryOwner() const override;

	
	
};
