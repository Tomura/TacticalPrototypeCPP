// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#include "TacticalFPS.h"
#include "AITypes.h"
#include "AISystem.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/EnvQueryGenerator.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_ActorBase.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Item.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "TacticalBPFunctionLibrary.h"

#include "EnvQueryTest_FreeCover.h"




UEnvQueryTest_FreeCover::UEnvQueryTest_FreeCover(const FObjectInitializer& OI)
	: Super(OI)
{
	Cost = EEnvTestCost::High;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
	SetWorkOnFloatValues(false);

	Context = UEnvQueryContext_Querier::StaticClass();
}

void UEnvQueryTest_FreeCover::RunTest(FEnvQueryInstance& QueryInstance) const
{
	TArray<AActor*> ContextActors;
	if (!QueryInstance.PrepareContext(Context, ContextActors))
	{
		return;
	}

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		FVector ItemLoc = GetItemLocation(QueryInstance, It.GetIndex());
		for (AActor* ContextActor : ContextActors)
		{
			const bool bSuccess = UTacticalBPFunctionLibrary::IsCoverFree(ContextActor, ItemLoc);
			It.SetScore(TestPurpose, FilterType, bSuccess, true);
		}
	}
}
