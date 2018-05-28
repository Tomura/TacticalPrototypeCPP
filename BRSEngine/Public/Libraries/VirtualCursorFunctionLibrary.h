// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "VirtualCursorFunctionLibrary.generated.h"

//class UVaultModel;

/**
 * 
 */
UCLASS()
class BRSENGINE_API UVirtualCursorFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, Category="Game")
	static void EnableVirtualCursor(class APlayerController* PC);

	UFUNCTION(BlueprintCallable, Category = "Game")
	static void DisableVirtualCursor(class APlayerController* PC);

	UFUNCTION(BlueprintPure, Category="Game")
	static bool IsCursorOverInteractableWidget();
};
