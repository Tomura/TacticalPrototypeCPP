// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Engine/DeveloperSettings.h"
#include "BRSGameModeSettings.generated.h"

/**
 * 
 */

//USTRUCT()
//struct FGameMode


UCLASS()
class BRSENGINE_API UBRSGameModeSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UBRSGameModeSettings(const FObjectInitializer& OI) : Super(OI) {}

	//UPROPERTY(Category = "Game Mode", EditAnywhere)
	//TArray<
	
	//FGameModePrefix.GameMode
};
