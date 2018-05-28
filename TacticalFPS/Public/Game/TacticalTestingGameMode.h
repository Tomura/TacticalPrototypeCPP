// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Game/TacticalGameModeBase.h"
#include "TacticalTestingGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API ATacticalTestingGameMode : public ATacticalGameModeBase
{
	GENERATED_BODY()
	
public:
	ATacticalTestingGameMode(const FObjectInitializer& OI);
	
	virtual void BeginPlay() override;

	virtual void OnPlayerSetLoadout(class ATacticalPlayerController* PC) override;
	
};
