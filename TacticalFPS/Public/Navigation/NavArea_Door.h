// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "AI/Navigation/NavAreas/NavArea.h"
#include "TacticalNavigationHelpers.h"
#include "NavArea_Door.generated.h"

UCLASS()
class UNavArea_Door : public UNavArea
{
	GENERATED_BODY()
public:
	UNavArea_Door(const FObjectInitializer& OI) : Super(OI) 
	{
		DrawColor = FColor(200, 200, 0); // yellow
		DefaultCost = 1.f;

		FNavAreaHelper::Set(AreaFlags, ENavAreaFlag::Door);
	}
};
