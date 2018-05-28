// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Components/BoxComponent.h"
#include "TacticalCoverComponent.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalCoverComponent : public UBoxComponent
{
	GENERATED_BODY()
	
public:
	class ATacticalCoverMarker* GetCoverOwner() const;



protected:
	TMap<class ATacticalAIController*, FVector> Reservations;
	
};
