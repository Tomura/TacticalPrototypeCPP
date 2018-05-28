// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSGameInstance.h"
#include "BRSMessageWindowUMG.h"


UBRSMessageWindowUMG::UBRSMessageWindowUMG(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NextState = EBRSGameInstanceState::None;
}

void UBRSMessageWindowUMG::OnButtonPressed()
{
	OKPressedEvent.Broadcast();

	if (NextState != EBRSGameInstanceState::None)
	{
		UBRSGameInstance* GI = Cast<UBRSGameInstance>(GetOwningPlayer()->GetGameInstance());
		if (GI)
		{
			GI->GotoState(NextState);
		}
	}

	this->RemoveFromViewport();
}

void UBRSMessageWindowUMG::OnCancelButtonPressed()
{
	CancelPressedEvent.Broadcast();
	this->RemoveFromViewport();
}
