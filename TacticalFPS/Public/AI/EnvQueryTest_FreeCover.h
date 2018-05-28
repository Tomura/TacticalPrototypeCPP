// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#pragma once

#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvQueryTest_FreeCover.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UEnvQueryTest_FreeCover : public UEnvQueryTest
{
	GENERATED_BODY()
public:
	UEnvQueryTest_FreeCover(const FObjectInitializer& OI);
	
		/** context: other end of trace test */
		UPROPERTY(EditDefaultsOnly, Category = Trace)
		TSubclassOf<UEnvQueryContext> Context;
	

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
};
