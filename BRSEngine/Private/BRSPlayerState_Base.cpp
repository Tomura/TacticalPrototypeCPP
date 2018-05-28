// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "Net/UnrealNetwork.h"
#include "BRSPlayerState_Base.h"


ABRSPlayerState_Base::ABRSPlayerState_Base(const FObjectInitializer& OI)
	: Super(OI)
{
}

void ABRSPlayerState_Base::UnregisterPlayerWithSession()
{
	if (!bFromPreviousLevel)
	{
		Super::UnregisterPlayerWithSession();
	}
}


//void ABRSPlayerState_Base::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
//	//DOREPLIFETIME(ABRSPlayerState_Base, TeamID);
//}


//ETeamAttitude::Type ABRSPlayerState_Base::GetTeamAttitudeTowards(const AActor& Other) const
//{
//	return ETeamAttitude::Hostile;
//}

void ABRSPlayerState_Base::SetQuitter(bool bInQuitter)
{
	bQuitter = bInQuitter;
}

bool ABRSPlayerState_Base::IsQuitter() const
{
	return bQuitter;
}

