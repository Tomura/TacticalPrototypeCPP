// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalMessage.h"
#include "TacticalPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "TacticalGameMode.h"




ATacticalPlayerState::ATacticalPlayerState(const FObjectInitializer& OI)
	: Super(OI)
{
	EngineMessageClass = UTacticalMessage::StaticClass();
}

ETeamAttitude::Type ATacticalPlayerState::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (const IGenericTeamAgentInterface* OtherTeamInterface = Cast<IGenericTeamAgentInterface>(&Other))
	{
		const FGenericTeamId OtherTeam = OtherTeamInterface->GetGenericTeamId();
		const bool bSameTeam = (OtherTeam == GetGenericTeamId());
		if (bSameTeam)
		{
			return ETeamAttitude::Friendly;
		}
		else if (OtherTeam == TEAM_Civilians)
		{
			return ETeamAttitude::Neutral;
		}
		else
		{
			return ETeamAttitude::Hostile;
		}
	}

	return ETeamAttitude::Friendly;
}

void ATacticalPlayerState::OnRepTeamId()
{

}

void ATacticalPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATacticalPlayerState, bReady);
	DOREPLIFETIME(ATacticalPlayerState, Kills);
	DOREPLIFETIME(ATacticalPlayerState, Deaths);
	DOREPLIFETIME(ATacticalPlayerState, TeamKills);
	DOREPLIFETIME(ATacticalPlayerState, TeamId);
}