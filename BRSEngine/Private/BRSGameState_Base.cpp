// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSGameInstance.h"
#include "BRSPlayerController_Base.h"
#include "BRSGameMode_Base.h"
#include "BRSGameState_Base.h"



void ABRSGameState_Base::RequestFinishAndExitToMainMenu()
{
	if (AuthorityGameMode)
	{
		// we are server, tell the gamemode
		ABRSGameMode_Base* const GameMode = Cast<ABRSGameMode_Base>(AuthorityGameMode);
		if (GameMode)
		{
			GameMode->RequestFinishAndExitToMainMenu();
		}
	}
	else
	{
		// we are client, handle it here (because gamemode does not exist for us)
		UBRSGameInstance* GI = Cast<UBRSGameInstance>(GetGameInstance());
		if (GI)
		{
			GI->RemoveSplitScreenPlayers();
		}

		ABRSPlayerController_Base* const PrimaryPC = Cast<ABRSPlayerController_Base>(GetGameInstance()->GetFirstLocalPlayerController());
		if (PrimaryPC)
		{
			check(PrimaryPC->GetNetMode() == ENetMode::NM_Client);
			PrimaryPC->HandleReturnToMainMenu();
		}
	}
}