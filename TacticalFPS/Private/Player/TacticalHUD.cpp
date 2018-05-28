// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalHUD.h"
#include "TacticalPauseMenu.h"
#include "TacticalCoverMarker.h"




ATacticalHUD::ATacticalHUD(const FObjectInitializer& OI)
	: Super(OI)
{
	GameHUDClass = nullptr;
	PauseMenuClass = nullptr;
}

void ATacticalHUD::BeginPlay()
{
	if (GameHUDClass && GameHUDClass->IsValidLowLevel())
	{
		GameHUDWidget = CreateWidget<UUserWidget>(PlayerOwner, GameHUDClass);
		if (GameHUDWidget)
		{
			GameHUDWidget->AddToPlayerScreen();
		}
	}
	if (PauseMenuClass && PauseMenuClass->IsValidLowLevel())
	{
		PauseMenuWidget = CreateWidget<UTacticalPauseMenu>(PlayerOwner, PauseMenuClass);
		if (PauseMenuWidget)
		{
			PauseMenuWidget->AddToPlayerScreen();
		}
	}
}

void ATacticalHUD::DrawHUD()
{
	Super::DrawHUD();
	const float DeltaTime = GetWorld() ? GetWorld()->DeltaTimeSeconds : 0.01f;
}

void ATacticalHUD::DisplayDebug(class UCanvas* InCanvas, const class FDebugDisplayInfo& InDebugDisplay, float& YL, float& YPos)
{
	Super::DisplayDebug(Canvas, InDebugDisplay, YL, YPos);
}

void ATacticalHUD::ShowDebug(FName DebugType /* = NAME_None */)
{
	Super::ShowDebug(DebugType);

	static FName NAME_Weapon = FName(TEXT("Cover"));
	if (DebugType == NAME_Weapon)
	{
		if (GetWorld())
		{
			bool bDebugReservation = DebugDisplay.Find(NAME_Weapon) != INDEX_NONE;
			for (TActorIterator<ATacticalCoverMarker> Itr(GetWorld()); Itr; ++Itr)
			{
				ATacticalCoverMarker* Test = *Itr;
				if (Test)
				{
					Test->bDebugReservations = bDebugReservation;
				}
			}
		}
		
	}
}
