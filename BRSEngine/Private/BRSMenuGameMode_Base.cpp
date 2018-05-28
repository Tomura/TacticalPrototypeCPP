// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSMenuGameMode_Base.h"
#include "BRSGameInstance.h"



//
//TSubclassOf<AGameMode> ABRSMenuGameMode_Base::SetGameMode(const FString& MapName, const FString& Options, const FString& Portal)
//{
//	if (UBRSGameInstance* GI = Cast<UBRSGameInstance>(GetGameInstance()))
//	{
//		if (MapName == GI->GetMainMenuMap() || MapName == GI->GetWelcomeScreenMap())
//		{
//			return GetClass();
//		}
//	}
//	return Super::SetGameMode(MapName, Options, Portal);
//}

void ABRSMenuGameMode_Base::GenericPlayerInitialization(class AController* C)
{
	Super::GenericPlayerInitialization(C);
}
