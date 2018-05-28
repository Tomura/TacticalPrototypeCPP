// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "Engine/DeveloperSettings.h"
#include "TacticalHUDSettings.generated.h"

/**
* Game related settings
*/
UCLASS(config = Game, defaultconfig, meta=(DisplayName="[Tactical] HUD"))
class TACTICALFPS_API UTacticalHUDSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:

	UTacticalHUDSettings(const FObjectInitializer& OI) : Super(OI)
	{
		bShowCrosshair = true;
		bUse3dWeaponWidget = false;
		HUD_AccentColor = FLinearColor::Red;
	}


public:
	FORCEINLINE bool ShowCrosshair() const
	{
		return bShowCrosshair;
	}

	FORCEINLINE bool Use3dWeaponWidget() const
	{
		return bUse3dWeaponWidget;
	}

	FORCEINLINE FLinearColor GetAccentColor() const
	{
		return HUD_AccentColor;
	}

	UMaterialParameterCollection* GetHUDMaterialParameterCollection() const;

	static void SetAccentColor(const FLinearColor& NewColor);

	void SetHUDAccentColor(const FLinearColor& NewColor) { HUD_AccentColor = NewColor; SaveConfig(); }
protected:
	/** The max speed of the Analog Cursor */
	UPROPERTY(config, EditAnywhere, Category = "HUD")
	uint16 bShowCrosshair : 1;

	/** The max speed of the Analog Cursor */
	UPROPERTY(config, EditAnywhere, Category = "HUD")
	uint16 bUse3dWeaponWidget : 1;


	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "HUD", meta = (AllowedClasses = "MaterialParameterCollection"))
	FStringAssetReference HUD_MaterialCollection;

	UPROPERTY(config, EditAnywhere, Category = "HUD")
		FLinearColor HUD_AccentColor;
};