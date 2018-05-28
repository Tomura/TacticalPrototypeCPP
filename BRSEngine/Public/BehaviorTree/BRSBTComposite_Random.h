// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#pragma once


#include "BehaviorTree/BTCompositeNode.h"
#include "BRSBTComposite_Random.generated.h"

/**
 * 
 */
UCLASS()
class BRSENGINE_API UBRSBTComposite_Random : public UBTCompositeNode
{
	GENERATED_BODY()
	
public:
	UBRSBTComposite_Random(const FObjectInitializer& ObjectInitializer);


	int32 GetNextChildHandler(struct FBehaviorTreeSearchData& SearchData, int32 PrevChild, EBTNodeResult::Type LastResult) const;

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif
};
