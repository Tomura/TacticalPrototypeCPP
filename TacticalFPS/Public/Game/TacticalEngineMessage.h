// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Game/TacticalMessage.h"
#include "TacticalEngineMessage.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalEngineMessage : public UTacticalMessage
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
		FText PlayerEnteredMessage;
	UPROPERTY()
		FText PlayerLeftMessage;
	
	
};
