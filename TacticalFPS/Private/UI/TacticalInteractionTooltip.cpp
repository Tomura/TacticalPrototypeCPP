// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "BRSCharacterInteractionInterface.h"
#include "TacticalInteractable.h"
#include "TacticalInteractionTooltip.h"




UTacticalInteractionTooltip::UTacticalInteractionTooltip(const FObjectInitializer& OI)
	: Super(OI)
{

}

bool UTacticalInteractionTooltip::SetInteractableActor(AActor* Actor)
{
	//if (Actor->Implements<IBRSCharacterInteractionInterface>())
	//{
	//	InteractableActor = Actor;
	//}
	return false;
}
