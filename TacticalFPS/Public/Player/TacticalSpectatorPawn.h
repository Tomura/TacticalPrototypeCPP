// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "GameFramework/SpectatorPawn.h"
#include "TacticalSpectatorPawn.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API ATacticalSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	ATacticalSpectatorPawn(const FObjectInitializer& OI);
	
	virtual void Reset();

	virtual void SetupPlayerInputComponent(class UInputComponent* inInputComponent);

protected:
	/** Actor that is currently spectated (Pawn or Camera)*/
	UPROPERTY()
	class AActor* SpectatingActor;

	UPROPERTY(Replicated)
	FRotator ViewRotation;
	UPROPERTY(Replicated)
	FVector ViewLocation;
};
