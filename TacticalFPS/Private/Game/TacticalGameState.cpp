// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalGameState.h"
#include "Net/UnrealNetwork.h"
#include "TacticalGameMode.h"
#include "TacticalPlayerState.h"




ATacticalGameState::ATacticalGameState(const FObjectInitializer& OI)
	: Super(OI)
{
	RoundNumber = 0;
	bWaitingForNextRound = false;
	bWaitingForEndRound = false;
	TimeRoundStart = 0.f;
	TimeRoundEnd = 0.f;

	RoundTimeLimit = 0.f;
}

void ATacticalGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATacticalGameState, RoundNumber);
	DOREPLIFETIME(ATacticalGameState, bWaitingForNextRound);
	DOREPLIFETIME(ATacticalGameState, bWaitingForEndRound);
	DOREPLIFETIME(ATacticalGameState, TimeRoundStart);
	DOREPLIFETIME(ATacticalGameState, TimeRoundEnd);
}

void ATacticalGameState::GetPlayerStateSortedByScore(TArray<class ATacticalPlayerState*> OutPlayers) const
{
	OutPlayers.Empty();
	// Get all Player States in World
	for (TActorIterator<ATacticalPlayerState> It(GetWorld()); It; ++It)
	{
		ATacticalPlayerState* Player = *It;
		if(Player)
		{
			OutPlayers.Add(Player);
		}
	}
	// Sort them by Score if necessary
	if(OutPlayers.Num() > 1)
	{
		OutPlayers.Sort([&](const ATacticalPlayerState& Test1, const ATacticalPlayerState& Test2)
		{
			const float Score1 = Test1.GetScore();
			const float Score2 = Test2.GetScore();
			return Score1 > Score2;
		});
	}

}

void ATacticalGameState::OnBeginRound()
{

}


