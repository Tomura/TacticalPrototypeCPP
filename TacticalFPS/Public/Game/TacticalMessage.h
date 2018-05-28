// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "GameFramework/EngineMessage.h"
#include "TacticalMessage.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalMessage : public ULocalMessage
{
	GENERATED_BODY()
public:
	UTacticalMessage(const FObjectInitializer& OI);
	
	UPROPERTY(EditDefaultsOnly, Category = "Local Message")
	FName MessageArea;

	UPROPERTY(EditDefaultsOnly, Category = "Local Message")
	float LifeTime;

	virtual void ClientReceive(const FClientReceiveData& ClientData) const override;

	virtual FText GetText(const FClientReceiveData& ClientData) const;
	
};
