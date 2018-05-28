// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalCoverComponent.h"
#include "TacticalCoverMarker.h"
#include "TacticalAIController.h"




class ATacticalCoverMarker* UTacticalCoverComponent::GetCoverOwner() const
{
	if (GetOwner() != nullptr)
	{
		if (ATacticalCoverMarker* CoverOwner = Cast<ATacticalCoverMarker>(GetOwner()))
		{
			return CoverOwner;
		}
	}

	return nullptr;
}


