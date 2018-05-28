// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Game/TacticalMessage.h"
#include "TacticalGameMessage.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalGameMessage : public UTacticalMessage
{
	GENERATED_BODY()
	
public:
	// Message when a player kills another player
	UPROPERTY()
		FText PlayerKillMessage;

	// Message when a player dies to something that isn't a player
	UPROPERTY()
		FText PlayerDiedMessage;

	// Message when a player commits suicide
	UPROPERTY()
		FText PlayerSuicideMessage;

	// Message when a player team kills another player
	UPROPERTY()
		FText PlayerTeamKillMessage;

	// Message when a player kills an hostage
	UPROPERTY()
		FText PlayerKillsHostageMessage;

	// Message when a enemy kills an hostage
	UPROPERTY()
		FText EnemyKillsHostageMessage;

	
	
};
