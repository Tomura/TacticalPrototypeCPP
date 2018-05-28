// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSLoadingScreenUMG.h"


UBRSLoadingScreenUMG::UBRSLoadingScreenUMG(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UBRSLoadingScreenUMG::LoadingScreenVisible()
{
	LoadingScreenVisibleEvent.Broadcast();
}

void UBRSLoadingScreenUMG::HideLoadingScreen_Implementation()
{}