// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "BRSGameMode_Base.h"
#include "BRSMenuGameMode_Base.generated.h"

/**
 * 
 */
UCLASS()
class BRSENGINE_API ABRSMenuGameMode_Base : public ABRSGameMode_Base
{
	GENERATED_BODY()
public:
	ABRSMenuGameMode_Base(const FObjectInitializer& OI) : Super(OI) {}
	
	virtual void RestartGame() override { return; }
	virtual void BeginGame() { return; }
	virtual void GenericPlayerInitialization(class AController* C) override;
	virtual void RestartPlayer(class AController* aPlayer) override {}
	//virtual TSubclassOf<AGameMode> GetGameModeClass(const FString& MapName, const FString& Options, const FString& Portal) const override;
	//virtual TSubclassOf<AGameMode> SetGameMode(const FString& MapName, const FString& Options, const FString& Portal);
	void Logout(class AController* Exiting) override { Super::Logout(Exiting); }
	

	UPROPERTY(Category = Menu, BlueprintReadWrite)
	class ACameraActor* MenuCamera;
};
