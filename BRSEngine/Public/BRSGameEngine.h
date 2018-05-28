// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "BRSGameEngine.generated.h"

UCLASS()
class BRSENGINE_API UBRSGameEngine : public UGameEngine
{
	GENERATED_BODY()
public:
	UBRSGameEngine(const FObjectInitializer& ObjectInitializer);
	/* Hook up specific callbacks */
	virtual void Init(class IEngineLoop* InEngineLoop);

	/** 
	 * All regular engine handling, plus update BRSGameInstance state properly
	 */
	virtual void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString) override;
	virtual void HandleTravelFailure(UWorld* InWorld, ETravelFailure::Type FailureType, const FString& ErrorString) override;
};