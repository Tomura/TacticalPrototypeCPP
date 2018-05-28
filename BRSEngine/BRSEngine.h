// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#ifndef __BRSENGINE_H__
#define __BRSENGINE_H__

#include "Engine.h"
#include "Online.h"

#include "AssetRegistryModule.h"

#include "SoundDefinitions.h"

#include "AIModule.h"

// Includes neccessary header files for custom UMG Widgets
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

#include "BRSGameEngine.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBRSEngine, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogBRSGameSettings, Log, All);



///** returns asset data for all assets of the specified class
//* do not use for Blueprints as you can only query for all blueprints period; use GetAllBlueprintAssetData() to query the blueprint's underlying class
//* if bRequireEntitlements is set, assets on disk for which no local player has the required entitlement will not be returned
//*
//* WARNING: the asset registry does a class name search not a path search so the returned assets may not actually be the class you want in the case of name conflicts
//*			if you load any returned assets always verify that you got back what you were expecting!
//*/
//extern BRSGAME_API void GetAllAssetData(UClass* BaseClass, TArray<FAssetData>& AssetList);

/** returns asset data for all blueprints of the specified base class in the asset registry
* this does not actually load assets, so it's fast in a cooked build, although the first time it is run
* in an uncooked build it will hitch while scanning the asset registry
*/
extern BRSENGINE_API void GetAllBlueprintAssetData(UClass* BaseClass, TArray<FAssetData>& AssetList, FName Path = "");


template <typename SummaryType>
extern BRSENGINE_API SummaryType* GetMapSummary(const FString& SelectedMap, TSubclassOf<AGameMode> GameModeClass);


#endif
