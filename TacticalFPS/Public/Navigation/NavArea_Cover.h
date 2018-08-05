// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "NavAreas/NavArea.h"
#include "TacticalNavigationHelpers.h"
#include "NavArea_Cover.generated.h"

UCLASS()
class UNavArea_CoverHigh : public UNavArea
{
	GENERATED_BODY()
public:
	UNavArea_CoverHigh(const FObjectInitializer& OI) : Super(OI)
	{
		DrawColor = FColor(0, 150, 0); // yellow
		DefaultCost = 1.f;

		FNavAreaHelper::Set(AreaFlags, ENavAreaFlag::Cover);
	}
};

UCLASS()
class UNavArea_CoverLow : public UNavArea
{
	GENERATED_BODY()
public:
	UNavArea_CoverLow(const FObjectInitializer& OI) : Super(OI)
	{
		DrawColor = FColor(0, 100, 0); // yellow
		DefaultCost = 1.f;

		FNavAreaHelper::Set(AreaFlags, ENavAreaFlag::Cover);
	}
};