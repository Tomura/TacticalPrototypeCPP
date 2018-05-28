// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalWeapon.h"
#include "TacticalGameInstance.h"




UTacticalGameInstance::UTacticalGameInstance(const FObjectInitializer& OI)
	: Super(OI)
{
}

void UTacticalGameInstance::Init()
{
	Super::Init();

#if WITH_EDITOR
	// Force Asynchronous Scan to create registry in time
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	AssetRegistry.SearchAllAssets(true);
#endif
}

TSubclassOf<AGameModeBase> UTacticalGameInstance::OverrideGameModeClass(TSubclassOf<AGameModeBase> GameModeClass, const FString& MapName, const FString& Options, const FString& Portal) const
{
	return Super::OverrideGameModeClass(GameModeClass, MapName, Options, Portal);
}
