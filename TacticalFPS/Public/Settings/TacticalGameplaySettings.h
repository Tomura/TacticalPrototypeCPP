// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Engine/DeveloperSettings.h"
#include "TacticalGameplaySettings.generated.h"

/**
 * 
 */
UCLASS(defaultconfig, config = Game, meta = (DisplayName="[Tactical] Gameplay"))
class TACTICALFPS_API UTacticalGameplaySettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UTacticalGameplaySettings(const FObjectInitializer& OI);

	UPROPERTY(Config, Category = "Display", EditAnywhere, meta=(ClampMin="80.0", ClampMax="140.0"))
	float DefaultFOV;

	UPROPERTY(Config, Category = "Controls", EditAnywhere, meta = (ClampMin = "0.1", ClampMax = "1.0"))
	FVector2D ADSMouseSensitivityMultiplier;
	
	UPROPERTY(Config, Category = "Controls", EditAnywhere, meta = (ClampMin = "0.1", ClampMax = "1.0"))
	FVector2D ADSStickSensitivityMultiplier;

	UPROPERTY(Config, Category = "Controls", EditAnywhere)
	uint32 bToggleCrouch : 1;

	UPROPERTY(Config, Category = "Controls", EditAnywhere)
	uint32 bToggleAim : 1;


	UPROPERTY(Config, Category = "Weapon", EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WeaponBobMultiplier;
	UPROPERTY(Config, Category = "Weapon", EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WeaponLagMultiplier;

};
