// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Weapons/TacticalInventory.h"
#include "TacticalGadgetInterface.h"
#include "TacticalPlaceable.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API ATacticalPlaceable : public ATacticalInventory, public ITacticalGadgetInterface
{
	GENERATED_BODY()
	
public:

	// overrides from ITacticalGadgetInterface
	/** Called when owner presses the key for this gadget */
	virtual void OwnerPressedKey_Implementation() override;
	/** Called when owner releases the key for this gadget */
	virtual void OwnerReleasedKey_Implementation() override;
	/** return type of this gadget */
	virtual ETacticalGadgetType GetGadgetType_Implementation()  override { return ETacticalGadgetType::Throwable; }
	
	
};
