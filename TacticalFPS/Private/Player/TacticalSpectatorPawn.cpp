// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "Net/UnrealNetwork.h"
#include "TacticalSpectatorPawn.h"




ATacticalSpectatorPawn::ATacticalSpectatorPawn(const FObjectInitializer& OI) : Super(OI)
{
	SpectatingActor = nullptr;
	ViewRotation = FRotator::ZeroRotator;
	ViewLocation = FVector::ZeroVector;
}

void ATacticalSpectatorPawn::Reset()
{
	AActor::Reset();
}

void ATacticalSpectatorPawn::SetupPlayerInputComponent(class UInputComponent* inInputComponent)
{
	Super::SetupPlayerInputComponent(inInputComponent);
}



void ATacticalSpectatorPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
