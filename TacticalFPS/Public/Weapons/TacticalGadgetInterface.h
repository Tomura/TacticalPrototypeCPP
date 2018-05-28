// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "TacticalGadgetInterface.generated.h"


UENUM(BlueprintType)
enum class ETacticalGadgetType : uint8
{
	Instant,
	Placeable,
	Throwable,
	Weapon
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTacticalGadgetInterface : public UInterface
{
	GENERATED_UINTERFACE_BODY()
};



/**
 * 
 */
class TACTICALFPS_API ITacticalGadgetInterface
{
	GENERATED_IINTERFACE_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(Category = "Gadget", BlueprintNativeEvent)
	void OwnerPressedKey();

	UFUNCTION(Category = "Gadget", BlueprintNativeEvent)
	void OwnerReleasedKey();

	UFUNCTION(Category = "Gadget", BlueprintNativeEvent)
	ETacticalGadgetType GetGadgetType();
	
};
