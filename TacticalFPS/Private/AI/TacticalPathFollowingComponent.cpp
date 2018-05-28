// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalNavigationHelpers.h"
#include "TacticalPathFollowingComponent.h"
#include "TacticalAIController.h"
#include "TacticalCharacterMovement.h"
#include "TacticalCharacter.h"




UTacticalPathFollowingComponent::UTacticalPathFollowingComponent(const FObjectInitializer& OI)
	: Super(OI)
{

	//bEnableObstacleAvoidance = false;
}

void UTacticalPathFollowingComponent::SetMoveSegment(int32 SegmentStartIndex)
{
	Super::SetMoveSegment(SegmentStartIndex);

	ATacticalAIController* AIController = Cast<ATacticalAIController>(GetOwner());
	
	if (AIController)
	{
		const FNavPathPoint& SegmentStart = Path->GetPathPoints()[MoveSegmentStartIndex];
		const FNavPathPoint& SegmentEnd = Path->GetPathPoints().IsValidIndex(MoveSegmentStartIndex + 1) ? Path->GetPathPoints()[MoveSegmentStartIndex + 1] : Path->GetPathPoints()[MoveSegmentStartIndex];

		if (FNavAreaHelper::HasFlag(SegmentStart, ENavAreaFlag::Door))
		{
			FVector SegmentDir = (SegmentEnd.Location - SegmentStart.Location).GetSafeNormal();

			// Open Door
			AIController->OpenDoor(SegmentDir);
			//Cast<UTacticalCharacterMovement>(MovementComp)->bWalking = true;
		}
		else if(FNavAreaHelper::HasFlag(SegmentStart, ENavAreaFlag::Jump))
		{
			ATacticalCharacter* MyChar = Cast<ATacticalCharacter>(AIController->GetPawn());
			if (MyChar && MyChar->CanVault())
			{
				MyChar->Jump();
			}
		}
		else
		{
			//Cast<UTacticalCharacterMovement>(MovementComp)->bWalking = false;
		}

		if (FNavAreaHelper::HasFlag(SegmentStart, ENavAreaFlag::Cover))
		{
			AIController->OnEnteredCover();
		}
		else
		{
			AIController->OnLeftCover();
		}
	}
}
