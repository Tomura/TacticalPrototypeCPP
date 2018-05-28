// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "BRSGameState_Base.h"
#include "TacticalGameState.generated.h"


//UENUM(Blueprintable)
//enum class ETacticalRoundState : uint8
//{
//
//};

/**
 * 
 */
UCLASS()
class TACTICALFPS_API ATacticalGameState : public ABRSGameState_Base
{
	GENERATED_BODY()

	friend class ATacticalGameMode;
	friend class ATacticalTestingGameMode;
public:
	ATacticalGameState(const FObjectInitializer& OI);

	UFUNCTION(Category = "Tactical Game State", BlueprintCallable)
	int32 GetRoundNumber() const { return RoundNumber; }

	UFUNCTION(Category = "Tactical Game State", BlueprintCallable)
	bool WaitingForNextRound() const { return bWaitingForNextRound; }

	UFUNCTION(Category = "Tactical Game State", BlueprintCallable)
	bool WaitingForEndRound() const { return bWaitingForEndRound; }


	UFUNCTION(Category = "Tactical Game State", BlueprintCallable)
		float GetRemainingRoundTime() const { return (TimeRoundStart + RoundTimeLimit) - GetWorld()->GetTimeSeconds(); }
	UFUNCTION(Category = "Tactical Game State", BlueprintCallable)
		float GetTimeUntillEndRound() const { return  (TimeRoundEnd + 5.f) - GetWorld()->GetTimeSeconds(); }

	UFUNCTION(Category = "Tactical Game State", BlueprintCallable)
		virtual void GetPlayerStateSortedByScore(TArray<class ATacticalPlayerState*> OutPlayers) const;

	virtual void OnBeginRound();

protected:
	UPROPERTY(Replicated)
	int32 RoundNumber;

	UPROPERTY(Replicated)
	uint32 bWaitingForNextRound : 1;
	UPROPERTY(Replicated)
	uint32 bWaitingForEndRound : 1;

	UPROPERTY(Replicated)
	float TimeRoundEnd;
	UPROPERTY(Replicated)
	float TimeRoundStart;


	UPROPERTY(Replicated)
	float RoundTimeLimit;

};
