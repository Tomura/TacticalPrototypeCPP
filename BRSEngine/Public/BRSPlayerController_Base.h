// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "BRSPlayerController_Base.generated.h"


enum class EServerMessageType : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnClientReceiveSay, APlayerState*, Sender, const FString&, Msg, bool, bTeamMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnClientReceiveKill, APlayerState*, Killer, APlayerState*, Victim);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClientReceiveServerMessage, const FString&, Msg);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnClientReceiveServerMessage, const FText&, MessageText, APlayerState*, P1, APlayerState*, P2);

/**
 * 
 */
UCLASS(Config=Game)
class BRSENGINE_API ABRSPlayerController_Base : public APlayerController
{
	GENERATED_BODY()
public:
	ABRSPlayerController_Base(const FObjectInitializer& OI) : Super(OI) {}

	//virtual void InitInputSystem() override;
	//void ToggleChat();
	//UFUNCTION(BlueprintCallable, Category = Menu)
	//void ToggleMenu();


	virtual bool InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad) override;
	virtual bool InputAxis(FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad) override;
	

public:

	// MENU
	UFUNCTION(BlueprintCallable, Category = BRSPlayerController)
	virtual bool IsInMenu() const;

	UFUNCTION(BlueprintCallable, Category = BRSPlayerController)
	virtual void ShowPauseMenu(bool bShow) {}

	UFUNCTION(BlueprintCallable, Category = BRSPlayerController)
	virtual void ExitToMainMenu();


	UFUNCTION(BlueprintCallable, Category = BRSPlayerController)
	virtual void BP_RestartLevel();

	UFUNCTION(BlueprintCallable, Category = BRSPlayerController)
	virtual bool IsAllowedToRestartLevel() const;

	///////////////////////////////////////////
	// SESSION/GAME FLOW

	/** Starts the online game using the session name in the PlayerState */
	UFUNCTION(reliable, client)
	void ClientStartOnlineGame();
	void ClientStartOnlineGame_Implementation();


	/** Ends the online game using the session name in the PlayerState */
	UFUNCTION(reliable, client)
	void ClientEndOnlineGame();
	void ClientEndOnlineGame_Implementation();
	FTimerHandle TimerHandle_ClientStartOnlineGame;

	/** Notifies clients to send the end-of-round event */
	UFUNCTION(reliable, client)
	void ClientSendRoundEndEvent(bool bIsWinner, int32 ExpendedTimeInSeconds);
	void ClientSendRoundEndEvent_Implementation(bool bIsWinner, int32 ExpendedTimeInSeconds);
	
	bool bHasSentStartEvents;


	void HandleReturnToMainMenu();
	void ClientReturnToMainMenu_Implementation(const FString& ReturnReason) override;
	void CleanupSessionOnReturnToMenu();


	///////////////////////////////////////////
	// CHAT

	UFUNCTION(exec, BlueprintCallable, Category = "BRSPlayerController | Chat")
	void Say(FString Msg);
	UFUNCTION(exec, BlueprintCallable, Category = "BRSPlayerController | Chat")
	void TeamSay(FString Msg);

	UFUNCTION(reliable, server, WithValidation)
	void ServerSay(const FString& Msg, bool bTeamMessage);
	virtual void ServerSay_Implementation(const FString& Msg, bool bTeamMessage);
	virtual bool ServerSay_Validate(const FString& Msg, bool bTeamMessage);

	UFUNCTION(reliable, client)
	void ClientReceiveSay(class ABRSPlayerState_Base* MsgSender, const FString& Msg, bool bTeamMessage);
	virtual void ClientReceiveSay_Implementation(class ABRSPlayerState_Base* MsgSender, const FString& Msg, bool bTeamMessage);

	UFUNCTION(reliable, client)
	void ClientReceiveServerMessage(EServerMessageType MsgType, APlayerState* P1, APlayerState* P2);
	virtual void ClientReceiveServerMessage_Implementation(EServerMessageType MsgType, APlayerState* P1, APlayerState* P2);

	virtual void AddClientTravelParams(FString& URL){}

	bool IsUsingGamepad() const { return bUsingGamepad; }
	void SetUsingGamepad(bool bNewValue) { bUsingGamepad = bNewValue; }

	virtual bool IsAlliedTo(APlayerState* OtherPlayer) const { return false; }

	UPROPERTY(Category = "Chat", BlueprintAssignable)
	FOnClientReceiveSay OnClientReceiveSay;
	UPROPERTY(Category = "Chat", BlueprintAssignable)
	FOnClientReceiveServerMessage OnClientReceiveServerMessage;
	UPROPERTY(Category = "Chat", BlueprintAssignable)
	FOnClientReceiveKill OnClientReceiveKill;

protected:

	/** true for the first frame after the game has ended */
	uint8 bGameEndedFrame : 1;

	uint8 bUsingGamepad : 1;
};
