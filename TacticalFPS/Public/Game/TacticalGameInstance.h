// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "BRSGameInstance.h"
#include "TacticalGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalGameInstance : public UBRSGameInstance
{
	GENERATED_BODY()
	
public:
	UTacticalGameInstance(const FObjectInitializer& OI);
	
	virtual void Init() override;

	virtual TSubclassOf<AGameModeBase> OverrideGameModeClass(TSubclassOf<AGameModeBase> GameModeClass, const FString& MapName, const FString& Options, const FString& Portal) const;

};
