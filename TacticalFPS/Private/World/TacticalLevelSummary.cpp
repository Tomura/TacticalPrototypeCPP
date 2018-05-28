// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalLevelSummary.h"

UTacticalLevelSummary::UTacticalLevelSummary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MapTitle = NSLOCTEXT("LevelSummary", "UntitledMapName", "Untitled Map");
	Author = TEXT("Unknown");
	Description = NSLOCTEXT("LevelSummary", "DefaultDesc", "Need Description");
	RecommendedPlayerCount = FIntPoint(8, 12);
}