// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#include "TacticalFPS.h"
#include "TacticalAIModifierVolume.h"




ATacticalAIModifierVolume::ATacticalAIModifierVolume()
	: Super()
{
	MinAwareness = 0.f;
	AwarenessIncrementMultiplier = 1.f;
}