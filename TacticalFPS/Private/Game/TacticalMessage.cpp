// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalPlayerController.h"
#include "TacticalMessage.h"




UTacticalMessage::UTacticalMessage(const FObjectInitializer& OI)
	: Super(OI)
{
	MessageArea = FName("Default");
	LifeTime = 5.f;
}

void UTacticalMessage::ClientReceive(const FClientReceiveData& ClientData) const
{
	if (ClientData.LocalPC)
	{
		FText MessageText = GetText(ClientData);

		if(!MessageText.IsEmptyOrWhitespace())
		{
			if (ATacticalPlayerController* TacPC = Cast<ATacticalPlayerController>(ClientData.LocalPC))
			{
				
			}
		}
	}
}

FText UTacticalMessage::GetText(const FClientReceiveData& ClientData) const
{
	return FText::GetEmpty();
}
