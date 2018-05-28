 //Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#include "BRSEngine.h"
#include "BRSBTComposite_Random.h"




UBRSBTComposite_Random::UBRSBTComposite_Random(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	NodeName = "Random";

	OnNextChild.BindUObject(this, &UBRSBTComposite_Random::GetNextChildHandler);
}


int32 UBRSBTComposite_Random::GetNextChildHandler(struct FBehaviorTreeSearchData& SearchData, int32 PrevChild, EBTNodeResult::Type LastResult) const
{
	// success = quit
	int32 NextChildIdx = BTSpecialChild::ReturnToParent;

	const int32 NumChildren = GetChildrenNum();

	if(NumChildren >= 1)
	{
		if (PrevChild == BTSpecialChild::NotInitialized)
		{
			if (NumChildren > 1)
			{
				// newly activated: start from first
				NextChildIdx = FMath::RandRange(0, NumChildren-1);
			}
			else
			{
				NextChildIdx = 0;
			}
		}
	}

	return NextChildIdx;
}


#if WITH_EDITOR

FName UBRSBTComposite_Random::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Composite.Selector.Icon");
}

#endif