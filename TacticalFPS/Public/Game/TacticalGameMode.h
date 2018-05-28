// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "TacticalGameModeBase.h"
#include "TacticalGameMode.generated.h"



UCLASS(minimalapi)
class ATacticalGameMode : public ATacticalGameModeBase
{
	GENERATED_BODY()

public:
	ATacticalGameMode(const FObjectInitializer& OI);

	virtual void BeginPlay() override;
	virtual void HandleMatchHasStarted() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void InitializeHUDForPlayer(APlayerController* NewPlayer);

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;


	virtual APlayerController* Login(class UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	virtual void EndRoundCheck();

	UFUNCTION(Exec, BlueprintCallable, Category = Game)
	virtual void RestartRound();

	virtual void InitRound(bool bRestart = false);
	virtual void BeginRound();
	virtual void EndRound();

	TSubclassOf<class UUserWidget> GetPreRoundWidgetClass() const { return PreRoundWidgetClass; }


	virtual void PlayerBecameReady(class ATacticalPlayerController* ReadyPC);
	virtual void Killed(class AController* Killer, class AController* Victim, class AActor* KilledActor, const class UDamageType* DamageType);


	bool AllPlayersDead() const;
	bool AllEnemiesDead() const;

protected:
	uint32 bPendingNewRound : 1;
	uint32 bRoundInProgress : 1;

	UPROPERTY(Category = HUD, EditAnywhere)
	TSubclassOf<class UUserWidget> PreRoundWidgetClass;

	virtual void SpawnEnemies();
	class APawn* SpawnEnemy(class ATacticalEnemyStart* StartPoint);

	UPROPERTY(Category = Classes, EditAnywhere)
	TSubclassOf<class ACharacter> DefaultEnemyClass;


	FTimerHandle TimerEndRound;
	FTimerHandle TimerNewRound;

	// Round Settings

	/** Number of Enemies that should be spawned each round */
	UPROPERTY(Category = "Game Mode Settings", BlueprintReadWrite, EditAnywhere)
	int32 NumEnemies;

	/** Time limit of the round */
	UPROPERTY(Category = "Game Mode Settings", BlueprintReadWrite, EditAnywhere)
	int32 RoundTimeLimit;

	
	UPROPERTY(Category = "Game Mode Settings", EditAnywhere)
	float RoundEndTime;
};



