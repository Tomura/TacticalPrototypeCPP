// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "GameFramework/PlayerState.h"
#include "BRSPlayerState_Base.generated.h"

/**
 * 
 */
UCLASS()
class BRSENGINE_API ABRSPlayerState_Base : public APlayerState
{
	GENERATED_BODY()

public:
	/** Constructor*/
	ABRSPlayerState_Base(const FObjectInitializer& OI);

	virtual void UnregisterPlayerWithSession() override;

public:
	//// overrides from IGenericTeamAgentInterface
	///** Assigns Team Agent to given TeamID */
	//virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override { TeamID = NewTeamID; }

	///** Retrieve team identifier in form of FGenericTeamId */
	//virtual FGenericTeamId GetGenericTeamId() const override { return TeamID; }

	///** Retrieved owner attitude toward given Other object */
	//virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

public: // Methods and Functions

	/** Labels PlayerState as Quitter (or not) */
	void SetQuitter(bool bInQuitter);

	/** Returns if PlayerState has Quit the game */
	bool IsQuitter() const;

	UFUNCTION(Category = PlayerState, BlueprintNativeEvent)
	int32 GetScore() const;
	int32 GetScore_Implementation() const { return 0; }


protected:
	/** whether the user quit the match */
	UPROPERTY()
	uint8 bQuitter : 1;
};
