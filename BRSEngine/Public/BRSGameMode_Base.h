// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "GameFramework/GameMode.h"
#include "BRSGameMode_Base.generated.h"

UENUM(BlueprintType)
enum class EServerMessageType : uint8
{
	None,
	PlayerKill,
	TeamKill,
	BotKill,
	KilledByBot,
	Suicide,
	PlayerJoined,
	PlayerLeft,
	PlayerKicked,
	PlayerBanned
};

/**
 * 
 */
UCLASS()
class BRSENGINE_API ABRSGameMode_Base : public AGameMode
{
	GENERATED_BODY()
public:

	ABRSGameMode_Base(const FObjectInitializer& ObjectInitializer);
	
	virtual TSubclassOf<AGameSession>  GetGameSessionClass() const override;

	UFUNCTION(exec)
	void FinishMatch();
	void RequestFinishAndExitToMainMenu();

	UFUNCTION(BlueprintNativeEvent, Category = GameMode)
	void DetermineMatchWinner();
	virtual void DetermineMatchWinner_Implementation() {}

	UFUNCTION(BlueprintNativeEvent, Category = GameMode)
	bool IsWinner(class ABRSPlayerState_Base* inPlayerState) const;
	virtual bool IsWinner_Implementation(class ABRSPlayerState_Base* inPlayerState) const { return false; }

	
	UFUNCTION(Category = "Game | Chat", BlueprintCallable)
	void ServerMessage(EServerMessageType MsgType, APlayerState* P1 = nullptr, APlayerState* P2 = nullptr);


	UPROPERTY(Category = "Information", BlueprintReadOnly, EditDefaultsOnly)
	FText GameModeName;

	UPROPERTY(Category = "Information", BlueprintReadOnly, EditDefaultsOnly)
	FText GameModeDescription;
};
