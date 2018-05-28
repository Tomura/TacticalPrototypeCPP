// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "Engine/DeveloperSettings.h"
#include "BRSGameInstanceSettings.generated.h"

/**
 * BRSEngine related settings
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName="[BRS] Game Instance"))
class BRSENGINE_API UBRSGameInstanceSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UBRSGameInstanceSettings(const FObjectInitializer& OI);

	FORCEINLINE FString GetMainMenuMap() const{ return MainMenuMap; }
	FORCEINLINE FString GetWelcomeScreenMap() const{ return WelcomeScreenMap; }

	FORCEINLINE class TSubclassOf<class UBRSMessageWindowUMG> GetMessageWindowWidgetClass() const { return MessageWindowWidgetClass; }
	FORCEINLINE class TSubclassOf<class UBRSWaitDialogUMG> GetWaitDialogWidgetClass() const { return WaitDialogWidgetClass; }

private:
	UPROPERTY(config, EditAnywhere, Category = "User Interface")
	TSubclassOf<class UBRSMessageWindowUMG> MessageWindowWidgetClass;

	UPROPERTY(config, EditAnywhere, Category = "User Interface")
	TSubclassOf<class UBRSWaitDialogUMG> WaitDialogWidgetClass;

	UPROPERTY(config, EditAnywhere, Category = "Maps")
	FString WelcomeScreenMap;

	UPROPERTY(config, EditAnywhere, Category = "Maps")
	FString MainMenuMap;
};