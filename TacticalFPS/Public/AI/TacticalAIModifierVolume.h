// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "TacticalAIModifierVolume.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API ATacticalAIModifierVolume : public AVolume
{
	GENERATED_BODY()
	
public:
	ATacticalAIModifierVolume();
	

	float GetAwarenessIncrementMultiplier() const { return AwarenessIncrementMultiplier; }
	float GetMinAwareness() const { return MinAwareness; }

protected:
	UPROPERTY(Category = "AI", EditAnywhere)
	float AwarenessIncrementMultiplier;


	UPROPERTY(Category = "AI", EditAnywhere)
	float MinAwareness;

};
