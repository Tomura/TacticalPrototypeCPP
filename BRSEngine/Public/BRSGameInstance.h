// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Engine/GameInstance.h"
#include "OnlineIdentityInterface.h"
#include "OnlineSessionInterface.h"
#include "FindSessionsCallbackProxy.h"
#include "BRSGameInstance.generated.h"

#define ZORDER_LOADINGSCREEN 100
#define ZORDER_MESSAGEWINDOW 110





DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionSearchCompleted, const TArray<FBlueprintSessionResult>&, SessionIds);


UENUM(BlueprintType)
namespace EBRSGameInstanceState
{
	enum Type
	{
		None			UMETA(DisplayName = "None"),
		PendingInvite	UMETA(DisplayName = "PendingInvite"),
		WelcomeScreen	UMETA(DisplayName = "WelcomeScreen"),
		MainMenu		UMETA(DisplayName = "MainMenu"),
		MessageMenu		UMETA(DisplayName = "MessageMenu"),
		Playing			UMETA(DisplayName = "Playing")
	};
}

class FBRSPendingInvite
{
public:
	FBRSPendingInvite() : ControllerId(-1), bPrivilegesCheckedAndAllowed(false) {}

	int32								ControllerId;
	TSharedPtr< const FUniqueNetId > 	UserId;
	FOnlineSessionSearchResult 			InviteResult;
	bool								bPrivilegesCheckedAndAllowed;
};

/** This class holds the value of what message to display when we are in the "MessageMenu" state */
class FBRSPendingMessage
{
public:
	FText	TitleString;
	FText	DisplayString;					// This is the display message in the main message body
	FText	OKButtonString;					// This is the ok button text
	FText	CancelButtonString;				// If this is not empty, it will be the cancel button text
	EBRSGameInstanceState::Type	NextState;	// Final destination state once message is discarded

	TWeakObjectPtr< ULocalPlayer > PlayerOwner;		// Owner of dialog who will have focus (can be NULL)
};

/**
 * 
 */
UCLASS(config=Game)
class BRSENGINE_API UBRSGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UBRSGameInstance(const FObjectInitializer& ObjectInitializer);

	TArray<TSharedPtr<FString> > AllMaps;
	//TArray<TSubclassOf<AGameMode>> AllGameModes;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BRSGameInstance")
		void GetAvailableMaps(TArray<FString> & OutMapList);

	//UFUNCTION(BlueprintCallable, Category = "BRSGameInstance")
	//void GetMapList(TArray<FString> & OutMapList, TSubclassOf<class ABRSGameMode> ForGameMode = NULL);

	//UFUNCTION(exec)
	//void PrintMapList();


	bool Tick(float DeltaTime);

	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void StartGameInstance() override;

	virtual bool HandleOpenCommand(const TCHAR* Cmd, FOutputDevice& Ar, UWorld* InWorld) override;


	virtual TSubclassOf<AGameModeBase> OverrideGameModeClass(TSubclassOf<AGameModeBase> GameModeClass, const FString& MapName, const FString& Options, const FString& Portal) const;

	UFUNCTION(BlueprintCallable, Category = BRSGameInstance)
	EBRSGameInstanceState::Type GetCurrentState();



	bool JoinSession(ULocalPlayer* LocalPlayer, int32 SessionIndexInSearchResults);
	bool JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& SearchResult);
	UFUNCTION(BlueprintCallable, Category = BRSGameInstance)
	bool JoinSession(ULocalPlayer* LocalPlayer, const FBlueprintSessionResult& SearchResult);

	/** Travel directly to the named session */
	void TravelToSession(const FName& SessionName);

	/** Initiates the session searching */
	UFUNCTION(Category = "BRSGameInstance|Sessions", BlueprintCallable)
	bool FindSessions(ULocalPlayer* PlayerOwner, bool bLANMatch);


	UFUNCTION(BlueprintCallable, Category = BRSGameInstance)
		void OpenFriends();


	UFUNCTION(BlueprintCallable, Category = BRSGameInstance)
		virtual bool LocalGame(class ULocalPlayer* LocalPlayer, const FString &inMapName, const FString &inGameMode, const FString &inOptions);
	UFUNCTION(BlueprintCallable, Category = BRSGameInstance)
		virtual bool HostGame(class ULocalPlayer* LocalPlayer, const FString &inMapName, const FString &inGameMode, const FString &inOptions, const bool bIsOnlineGame, const FString& ServerDisplayName);

	UFUNCTION(BlueprintCallable, Category = BRSGameInstance)
		virtual bool JoinGame(const FString &URL);



	class ABRSGameSession* GetGameSession() const;


	bool GetIsOnline() const;
	void SetIsOnline(bool bInIsOnline);
	/** Returns true if the passed in local player is signed in and online */
	bool IsLocalPlayerOnline(ULocalPlayer* LocalPlayer);

	/** Start task to get user privileges. */
	void StartOnlinePrivilegeTask(const IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate& Delegate, EUserPrivileges::Type Privilege, TSharedPtr< const FUniqueNetId > UserId);

	/** Common cleanup code for any Privilege task delegate */
	void CleanupOnlinePrivilegeTask();

	/** Show approved dialogs for various privileges failures */
	void DisplayOnlinePrivilegeFailureDialogs(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults);

	//UFUNCTION(exec)
	//void HttpQueryForServers();
	//void HttpServerQueryResponse(FHttpRequestPtr inRequest, FHttpResponsePtr inResponse, bool bConnected);
	//UFUNCTION(exec)
	//void HttpAdvertiseServer();
	/** Shuts down the session, and frees any net driver */
	void CleanupSessionOnReturnToMenu();


	/** Flag the local player when they quit the game */
	void LabelPlayerAsQuitter(ULocalPlayer* LocalPlayer) const;

	void RemoveExistingLocalPlayer(ULocalPlayer* ExistingPlayer);

	void RemoveSplitScreenPlayers();

	///** Removes local users from the party */
	//void RemovePlayersFromParty();


	/** Sends the game to the specified state. */
	void GotoState(EBRSGameInstanceState::Type NextState);

	/** Obtains the initial welcome state, which can be different based on platform */
	EBRSGameInstanceState::Type GetInitialState();

	/** Sends the game to the initial startup/frontend state  */
	void GotoInitialState();

	void ShowMessageThenGoMain(
		const FText& Title,
		const FText& Message,
		const FText& OKButtonString,
		const FText& CancelButtonString);
	void ShowMessageThenGotoState(
		const FText& Title,
		const FText& Message,
		const FText& OKButtonString,
		const FText& CancelButtonString,
		const EBRSGameInstanceState::Type& NewState,
		const bool OverrideExisting = true,
		TWeakObjectPtr<ULocalPlayer> PlayerOwner = nullptr);

	class UBRSMessageWindowUMG* SimpleShowMessageWindow(
		const FText& Title,
		const FText& Message,
		const FText& OKButtonString,
		const FText& CancelButtonString);


	FORCEINLINE FString GetMainMenuMap() const;
	FORCEINLINE FString GetWelcomeScreenMap() const;

	UPROPERTY(Category = "Game Instance", BlueprintAssignable)
	FOnSessionSearchCompleted OnSearchSessionsCompleteDelegate;

	void ShowWaitDialog(const FText& WindowTitle, const FText& Content);
	void HideWaitDialog();


	bool bRegenerateMessageWindow;

protected:

	FBRSPendingInvite PendingInvite;
	FBRSPendingMessage PendingMessage;
	FBRSPendingMessage LastPendingMessage;

	virtual void GenerateMapList(TArray<TSharedPtr<FString> > & OutMapList);

	//UPROPERTY(config)
	//TSubclassOf<class UBRSLoadingScreenUMG> LoadingScreenWidgetClass;
	UPROPERTY()
	class UBRSLoadingScreenUMG* LoadingScreenWidget;


	//UPROPERTY(config)
	//TSubclassOf<class UBRSMessageWindowUMG> MessageWindowWidgetClass;
	UPROPERTY()
	class UBRSMessageWindowUMG* MessageWindowWidget;

	//UPROPERTY(config)
	//TSubclassOf<class UBRSWaitDialogUMG> WaitDialogWidgetClass;
	UPROPERTY()
	class UBRSWaitDialogUMG* WaitDialogWidget;


	virtual void ShowLoadingScreen();
	virtual void HideLoadingScreen();

	virtual void ShowMessageWindow(const FText& HeadLine, const FText& Content, const FText& OkButtonText, const FText& CancelButtonText = FText::GetEmpty(), EBRSGameInstanceState::Type NextState = EBRSGameInstanceState::None);
	virtual void HideMessageWindow();

	virtual void MessageWindowConfirmed();
	FDelegateHandle MessageWindowConfirmHandle;



	virtual void FinishHostGame();
	FDelegateHandle FinishHostDelegateHandle;
	virtual void FinishJoinGame();
	FDelegateHandle FinishJoinDelegateHandle;

	virtual void FinishLoadFrontEndMap();
	FDelegateHandle FinishLoadFrontEndMapDelegateHandle;




private:
	//UPROPERTY(config)
	//FString WelcomeScreenMap;

	//UPROPERTY(config)
	//FString MainMenuMap;

	
	TEnumAsByte<EBRSGameInstanceState::Type> CurrentState;
	TEnumAsByte<EBRSGameInstanceState::Type> PendingState;

	/** URL to travel to after pending network operations */
	FString TravelURL;

	/** If true enable splitscreen when Map starts */
	bool bPendingEnableSplitscreen;

	/** Whether the user has an active license to play the game */
	bool bIsLicensed;
	bool bIsOnline;


	void AddNetworkFailureHandlers();
	void RemoveNetworkFailureHandlers();
	/** Called when there is an error trying to travel to a local session */
	void TravelLocalSessionFailure(UWorld *World, ETravelFailure::Type FailureType, const FString& ErrorString);


	// Map Delegates
	void OnPreLoadMap(const FString&  inString);
	void OnPostLoadMap(UWorld* World);
	void OnPostDemoPlay();

	/** Delegate function executed after checking privileges for starting quick match */
	void OnUserCanPlayInvite(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults);

	void HandleSessionUserInviteAccepted(const bool bWasSuccess, const int32 ControllerId, TSharedPtr< const FUniqueNetId > UserId, const FOnlineSessionSearchResult & InviteResult);
	void HandleNetworkConnectionStatusChanged(const FString& ServiceName, EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus);
	void HandleUserLoginChanged(int32 GameUserIndex, ELoginStatus::Type PreviousLoginStatus, ELoginStatus::Type LoginStatus, const FUniqueNetId& UserId);
	void HandleAppWillDeactivate();
	void HandleAppSuspend();
	void HandleAppResume();
	void HandleAppLicenseUpdate();
	void HandleSafeFrameChanged();
	void HandleControllerPairingChanged(int GameUserIndex, const FUniqueNetId& PreviousUser, const FUniqueNetId& NewUser);
	void HandleControllerConnectionChange(bool bIsConnection, int32 Unused, int32 GameUserIndex);
	void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);

	/** Show messaging and punt to welcome screen */
	void HandleSignInChangeMessaging();


	void MaybeChangeState();
	void EndCurrentState(EBRSGameInstanceState::Type NextState);
	void BeginNewState(EBRSGameInstanceState::Type NewState, EBRSGameInstanceState::Type PrevState);
	
	void BeginPendingInviteState();
	void BeginWelcomeScreenState();
	void BeginMainMenuState();
	void BeginMessageMenuState();
	void BeginPlayingState();

	void EndPendingInviteState();
	void EndWelcomeScreenState();
	void EndMainMenuState();
	void EndMessageMenuState();
	void EndPlayingState();

	void SetPresenceForLocalPlayers(const FVariantData& PresenceData);

	/** Travel directly to the named session */
	void InternalTravelToSession(const FName& SessionName);


	bool LoadFrontEndMap(const FString& MapName);

	FString PendingTravelURL;


	/** Callback which is intended to be called upon finding sessions */
	void OnSearchSessionsComplete(bool bWasSuccessful);


	/** Callback which is intended to be called upon joining session */
	void OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result);

	/** Callback which is intended to be called upon session creation */
	void OnCreatePresenceSessionComplete(FName SessionName, bool bWasSuccessful);

	/** Callback which is called after adding local users to a session */
	void OnRegisterLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result);

	/** Called after all the local players are registered */
	void FinishSessionCreation(EOnJoinSessionCompleteResult::Type Result);

	/** Callback which is called after adding local users to a session we're joining */
	void OnRegisterJoiningLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result);

	/** Called after all the local players are registered in a session we're joining */
	void FinishJoinSession(EOnJoinSessionCompleteResult::Type Result);



	/** Delegate for callbacks to Tick */
	FTickerDelegate TickDelegate;

	/** Handle to various registered delegates */
	FDelegateHandle TickDelegateHandle;
	FDelegateHandle TravelLocalSessionFailureDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;
	FDelegateHandle OnSearchSessionsCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	FDelegateHandle OnEndSessionCompleteDelegateHandle;
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
	FDelegateHandle OnCreatePresenceSessionCompleteDelegateHandle;

	/** Delegate for ending a session */
	FOnEndSessionCompleteDelegate OnEndSessionCompleteDelegate;


public:
	static FString GameInstanceStateToString(EBRSGameInstanceState::Type inState)
	{
		switch (inState)
		{
		case EBRSGameInstanceState::PendingInvite:
			return FString(TEXT("Pending Invite"));
		case EBRSGameInstanceState::WelcomeScreen:
			return FString(TEXT("WelcomeScreen"));
		case EBRSGameInstanceState::MainMenu:
			return FString(TEXT("MainMenu"));
		case EBRSGameInstanceState::MessageMenu:
			return FString(TEXT("MessageMenu"));
		case EBRSGameInstanceState::Playing:
			return FString(TEXT("Playing"));
		case EBRSGameInstanceState::None:
			return FString(TEXT("None"));
		default:
			break;
		}
		return FString(TEXT("Unknown State"));
	}

private:
	bool bStartingInstance;
};
