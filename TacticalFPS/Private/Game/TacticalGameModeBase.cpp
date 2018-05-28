// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalCharacter.h"
#include "TacticalPlayerController.h"
#include "TacticalAIController.h"
#include "TacticalGameModeBase.h"


ATacticalGameModeBase::ATacticalGameModeBase(const FObjectInitializer& OI)
	: Super(OI)
{
	bFriendlyFire = true;
	bSelfDamage = true;

	DamageModifier = 1.f;	
	FriendlyDamageModifier = 0.5f;
	EnemyDamageModifier = 1.f;

}

void ATacticalGameModeBase::Killed(class AController* Killer, class AController* Victim, class AActor* KilledActor, const class UDamageType* DamageType)
{
	// we make a distinction between minor and player equivalent bots.
	// minor bots are bots that show up in higher numbers like the enemies in TerrorHunt or other Coop modes
	// player equivalent bots are supposed to play as replacements for real players. (NYI) These need their AI Controller to require a PlayerState (bWantsPlayerState)

	if (Victim && Killer)
	{
		if (Killer == Victim)
		{
			// Killer is Victim. So it's a suicide
			ServerMessage(EServerMessageType::Suicide, Killer->PlayerState);
		}
		else
		{
			if (Victim->PlayerState)
			{
				// Victim has a player state so it's important to display. (Victim is Player or Bot equivalent to a player)
				if (Killer->PlayerState)
				{
					// Killer has a player state. This counts as a PvP kill. Could also be a bot that is equivalent to a player.
					IGenericTeamAgentInterface* TeamController = Cast<IGenericTeamAgentInterface>(Killer);
					if (TeamController && (TeamController->GetTeamAttitudeTowards(*Victim) == ETeamAttitude::Friendly))
					{
						ServerMessage(EServerMessageType::TeamKill, Killer->PlayerState, Victim->PlayerState);
					}
					else
					{
						ServerMessage(EServerMessageType::PlayerKill, Killer->PlayerState, Victim->PlayerState);
					}
				}
				else
				{
					// Killer has no player state, but killed an important player. Report that Victim has died so all other players receive this.
					ServerMessage(EServerMessageType::KilledByBot, Victim->PlayerState);
				}
			}
		}
	}
}

float ATacticalGameModeBase::GetModifiedDamage(float Damage, AController* EventInstigator, AActor* DamageCauser, AActor* DamagedActor)
{
	float ActualDamage = Damage;

	if (!bSelfDamage && DamagedActor == DamageCauser)
	{
		return 0.f;
	}


	UE_LOG(LogTemp, Log, TEXT("Damage: %s"), (DamageCauser ? *DamageCauser->GetName() : TEXT("NULL")));
	IGenericTeamAgentInterface* TeamDamageCauser = DamageCauser ? Cast<IGenericTeamAgentInterface>(DamageCauser) : nullptr;
	if (TeamDamageCauser && DamagedActor)
	{
		if (TeamDamageCauser->GetTeamAttitudeTowards(*DamagedActor) == ETeamAttitude::Friendly)
		{
			if (!bFriendlyFire)
			{
				return 0.f;
			}

			ActualDamage *= FriendlyDamageModifier;
		}
	}

	if (TeamDamageCauser && TeamDamageCauser->GetGenericTeamId().GetId() == TEAM_Enemies)
	{
		ActualDamage *= EnemyDamageModifier;
	}
	ActualDamage *= DamageModifier;

	return ActualDamage;
}

void ATacticalGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	ServerMessage(EServerMessageType::PlayerJoined, NewPlayer->PlayerState);
}

void ATacticalGameModeBase::Logout(AController* Exiting)
{
	ServerMessage(EServerMessageType::PlayerLeft, Exiting->PlayerState);

	Super::Logout(Exiting);
}
