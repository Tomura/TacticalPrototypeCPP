// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSGameInstance.h"
#include "BRSGameState_Base.h"
#include "BRSPlayerState_Base.h"
#include "BRSPlayerController_Base.h"
#include "BRSGameSession.h"
#include "BRSGameMode_Base.h"


ABRSGameMode_Base::ABRSGameMode_Base(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = ABRSGameState_Base::StaticClass();
	PlayerStateClass = ABRSPlayerState_Base::StaticClass();
	PlayerControllerClass = ABRSPlayerController_Base::StaticClass();
	//HUDClass = ABRSGameHUD::StaticClass();

	GameModeName = NSLOCTEXT("GameMode", "UnnamedGameModeName", "Unnamed Game Mode");
}

TSubclassOf<AGameSession> ABRSGameMode_Base::GetGameSessionClass() const
{
	return ABRSGameSession::StaticClass();
}


void ABRSGameMode_Base::FinishMatch()
{
	ABRSGameState_Base* const MyGameState = Cast<ABRSGameState_Base>(GameState);
	if (IsMatchInProgress())
	{
		EndMatch();
		DetermineMatchWinner();

		// notify everyone else
		for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			ABRSPlayerState_Base* PlayerState = Cast<ABRSPlayerState_Base>((*It)->PlayerState);
			const bool bIsWinner = IsWinner(PlayerState);

			(*It)->GameHasEnded(NULL, bIsWinner);
		}

		// lock pawns
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			(*It)->TurnOff();
		}

		// set up to restart the match
		//MyGameState->RemainingTime = TimeBetweenMatches;
	}
}

void ABRSGameMode_Base::RequestFinishAndExitToMainMenu()
{
	FinishMatch();

	UBRSGameInstance* const GI = Cast<UBRSGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->RemoveSplitScreenPlayers();
	}

	ABRSPlayerController_Base* LocalPrimaryController = nullptr;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABRSPlayerController_Base* Controller = Cast<ABRSPlayerController_Base>(*It);
		if (Controller == nullptr)
		{
			continue;
		}
		if (!Controller->IsLocalPlayerController())
		{
			const FText RemoteReturnReason = NSLOCTEXT("NetworkErrors", "HostHasLeft", "Host has left the game");
			Controller->ClientReturnToMainMenuWithTextReason(RemoteReturnReason);
		}
		else
		{
			LocalPrimaryController = Controller;
		}
	}

	// Game Instance should be calling this from an EndState. So call the PC function that performs cleanup, not the one that sends to GI state
	if (LocalPrimaryController != nullptr)
	{
		LocalPrimaryController->HandleReturnToMainMenu();
	}
}




void ABRSGameMode_Base::ServerMessage(EServerMessageType MsgType, APlayerState* P1 /*= nullptr*/, APlayerState* P2 /*= nullptr*/)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABRSPlayerController_Base* BRSPC = Cast<ABRSPlayerController_Base>(*It);
		if (BRSPC != nullptr)
		{
			BRSPC->ClientReceiveServerMessage(MsgType, P1, P2);
		}
	}
}

