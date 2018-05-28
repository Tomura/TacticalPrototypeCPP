// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalTestingGameMode.h"
#include "TacticalCharacter.h"
#include "TacticalLoadout.h"
#include "TacticalPlayerController.h"
#include "TacticalWeapon.h"
#include "TacticalInventoryComponent.h"
#include "TacticalGameState.h"
#include "TacticalPlayerState.h"




ATacticalTestingGameMode::ATacticalTestingGameMode(const FObjectInitializer& OI)
	: Super(OI)
{

}

void ATacticalTestingGameMode::BeginPlay()
{
	Super::BeginPlay();

	ATacticalGameState* TacticalGameState = GetGameState<ATacticalGameState>();
	if (TacticalGameState)
	{
		TacticalGameState->bWaitingForNextRound = false;
	}
}

void ATacticalTestingGameMode::OnPlayerSetLoadout(class ATacticalPlayerController* PC)
{
	Super::OnPlayerSetLoadout(PC);

	UE_LOG(LogTemp, Log, TEXT("Loadout Changed"));

	ATacticalCharacter* MyChar = PC->GetPawn() ? Cast<ATacticalCharacter>(PC->GetPawn()) : nullptr;
	if (MyChar)
	{
		bool bEquipSidearm = MyChar->GetInventory()->Sidearm == MyChar->GetInventory()->Weapon;


		MyChar->GetInventory()->DiscardInventory();
		MyChar->GetInventory()->GiveStartingInventory();
		if (bEquipSidearm)
		{
			MyChar->GetInventory()->SetCurrentWeapon(MyChar->GetInventory()->Sidearm);
		}
		else
		{
			MyChar->GetInventory()->SetCurrentWeapon(MyChar->GetInventory()->PrimaryWeapon);
		}
	}
}
