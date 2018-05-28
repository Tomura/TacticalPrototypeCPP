// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#pragma once

#include "BRSLevelScriptActor.h"
#include "TacticalLevelScriptActor.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API ATacticalLevelScriptActor : public ABRSLevelScriptActor
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void OnGenericUIEvent(APlayerController* TriggeringPlayer, FName Message);
	
	
	
};
