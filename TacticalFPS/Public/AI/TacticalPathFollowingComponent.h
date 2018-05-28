// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Navigation/CrowdFollowingComponent.h"
#include "TacticalPathFollowingComponent.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalPathFollowingComponent : public UPathFollowingComponent //public UCrowdFollowingComponent  // // //
{
	GENERATED_BODY()
	
public:
	UTacticalPathFollowingComponent(const FObjectInitializer& OI);


	// used to detect properties of a path's segment a character is about to follow
	virtual void SetMoveSegment(int32 SegmentStartIndex) override;


protected:
	
};
