// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "AI/Navigation/NavAreas/NavArea.h"
#include "TacticalNavigationHelpers.h"
#include "NavArea_Vault.generated.h"

UCLASS()
class UNavArea_Vault : public UNavArea
{
	GENERATED_BODY()
public:
	UNavArea_Vault(const FObjectInitializer& OI) : Super(OI)
	{
		DrawColor = FColor(255, 0, 255); // purple
		DefaultCost = 1.f;

		FNavAreaHelper::Set(AreaFlags, ENavAreaFlag::Jump);
	}
};
