// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "GameFramework/GameState.h"
#include "BRSGameState_Base.generated.h"

/**
 * 
 */
UCLASS()
class BRSENGINE_API ABRSGameState_Base : public AGameState
{
	GENERATED_BODY()

public:
	ABRSGameState_Base(const FObjectInitializer& OI) : Super(OI) {}

	void RequestFinishAndExitToMainMenu();
};
