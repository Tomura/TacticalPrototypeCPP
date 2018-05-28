// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponStates/TacticalWeaponState.h"
#include "TacticalWeaponStateGadget.generated.h"

/**
 * 
 */
UCLASS(abstract)
class TACTICALFPS_API UTacticalWeaponStateGadget : public UTacticalWeaponState
{
	GENERATED_BODY()
	
public:

	FORCEINLINE virtual class ATacticalWeaponAttachment_Weapon* GetAttachment() const;
	FORCEINLINE virtual class ATacticalWeapon* GetWeapon() const override;
	FORCEINLINE virtual class ATacticalCharacter* GetInventoryOwner() const override;

	
	
};
