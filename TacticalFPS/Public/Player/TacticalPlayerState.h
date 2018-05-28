// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "GenericTeamAgentInterface.h"
#include "BRSPlayerState_Base.h"
#include "TacticalPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API ATacticalPlayerState : public ABRSPlayerState_Base, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

	friend class ATacticalGameMode;
public:
	ATacticalPlayerState(const FObjectInitializer& OI);

	/** Retrieved owner attitude toward given Other object */
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; };
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamId) override { TeamId = NewTeamId; }

	UFUNCTION(Category = "Tactical Player State", BlueprintCallable)
	int32 GetKills() const { return Kills; }
	UFUNCTION(Category = "Tactical Player State", BlueprintCallable)
	int32 GetDeaths() const { return Deaths; }
	UFUNCTION(Category = "Tactical Player State", BlueprintCallable)
	int32 GetTeamKills() const { return TeamKills; }



	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Tactical Player State")
	uint32 bReady : 1;

	bool IsOnlySpecator() const { return bOnlySpectator; }

protected:
	UPROPERTY(ReplicatedUsing=OnRepTeamId)
	FGenericTeamId TeamId;

	UPROPERTY(Replicated)
	int32 Kills;

	UPROPERTY(Replicated)
	int32 Deaths;

	UPROPERTY(Replicated)
	int32 TeamKills;

	UFUNCTION()
	virtual void OnRepTeamId();

};
