// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "TacticalThrowable.h"
#include "TacticalGrenade.generated.h"

/**
 * 
 */
UCLASS(abstract, NotPlaceable, HideDropdown)
class TACTICALFPS_API ATacticalGrenade : public ATacticalThrowable
{
	GENERATED_BODY()

public:
	ATacticalGrenade(const FObjectInitializer& OI);

	


};
