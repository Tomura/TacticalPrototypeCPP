// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalLevelSummary.h"
#include "TacticalWorldSettings.h"
#include "TacticalWeaponPreviewScene.h"
#include "Misc/UObjectToken.h"



ATacticalWorldSettings::ATacticalWorldSettings(const FObjectInitializer& OI)
	: Super(OI)
{

}


void ATacticalWorldSettings::PostLoad()
{
	Super::PostLoad();
	CreateLevelSummary();
}

void ATacticalWorldSettings::PostInitProperties()
{
	Super::PostInitProperties();
	if (!HasAnyFlags(RF_NeedLoad))
	{
		CreateLevelSummary();
	}
}

#if WITH_EDITOR
void ATacticalWorldSettings::CheckForErrors()
{
	Super::CheckForErrors();

	UWorld* World = GetWorld();
	if (World == nullptr)
		return;

	if (World->GetWorldSettings() == this)
	{
		int32 NumWeaponPreview = 0;
		for (TActorIterator<ATacticalWeaponPreviewScene> WeaponPreviewIt(GetWorld()); WeaponPreviewIt; ++WeaponPreviewIt)
		{
			NumWeaponPreview++;
		}
		if (NumWeaponPreview <= 0)
		{
			FMessageLog("MapCheck").Warning()
				->AddToken(FUObjectToken::Create(this))
				->AddToken(FTextToken::Create(NSLOCTEXT("TacticalMapError", "ErrorMissingWeaponPreview", "Could not find a suitable weapon preview scene.")));
		}
		else if (NumWeaponPreview > 1)
		{
			FMessageLog("MapCheck").Warning()
				->AddToken(FUObjectToken::Create(this))
				->AddToken(FTextToken::Create(NSLOCTEXT("TacticalMapError", "ErrorMissingWeaponPreview", "Found two or more weapon preview scenes. It's not clear which one to use.")));
		}
		/* Display Errors with:
		FMessageLog("MapCheck").Warning()
			->AddToken(FUObjectToken::Create(this))
			->AddToken(FTextToken::Create(LOCTEXT( "loctext name", "loctext default content" ) ))
			->AddToken(FMapErrorToken::Create(TEXT(""))); <- this might not be needed as it needs a shared ref, not that worth it
		*/

	}

}
#endif


UTacticalLevelSummary* ATacticalWorldSettings::CreateLevelSummary()
{
	if (!IsTemplate())
	{
		// we need the object name to be reliable so we can pull it out by itself in the menus without loading the whole map
		static FName NAME_LevelSummary(TEXT("LevelSummary"));
		if (LevelSummary == NULL)
		{
			LevelSummary = FindObject<UTacticalLevelSummary>(UTacticalLevelSummary::StaticClass(), *NAME_LevelSummary.ToString());
			if (LevelSummary == NULL)
			{
				LevelSummary = NewObject<UTacticalLevelSummary>(GetOutermost(), UTacticalLevelSummary::StaticClass(), NAME_LevelSummary, RF_Standalone);
			}
		}
		else if (LevelSummary->GetFName() != NAME_LevelSummary)
		{
			// we have to duplicate instead of rename because we may be in PostLoad() and it's not safe to rename from there
			LevelSummary = DuplicateObject<UTacticalLevelSummary>(LevelSummary, GetOutermost(), *NAME_LevelSummary.ToString());
		}
		return LevelSummary;
	}
	
	return nullptr;
}