// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalHUDSettings.h"
#include "Kismet/KismetMaterialLibrary.h"

UMaterialParameterCollection* UTacticalHUDSettings::GetHUDMaterialParameterCollection() const
{
	return LoadObject<UMaterialParameterCollection>(nullptr, *(HUD_MaterialCollection.ToString()));
	 
}

void UTacticalHUDSettings::SetAccentColor(const FLinearColor& NewColor)
{
	UTacticalHUDSettings* DefaultHUDSettings = UTacticalHUDSettings::StaticClass()->GetDefaultObject<UTacticalHUDSettings>();
	DefaultHUDSettings->SetHUDAccentColor(NewColor);

	UMaterialParameterCollection* HUD_MPC = DefaultHUDSettings->GetHUDMaterialParameterCollection();
	if (HUD_MPC && GWorld)
	{
		UKismetMaterialLibrary::SetVectorParameterValue(GWorld, HUD_MPC, FName("AccentColor"), NewColor);
	}

	//if (DefaultHUDSettings->HUD_MaterialCollection)
	//{
	//	
	//	//FCollectionVectorParameter* VectorParam =  DefaultHUDSettings->HUD_MaterialCollection->SetVectorParameterValue();
	//	//VectorParam->DefaultValue = NewColor;
	//}
}