// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSGameInstance.h"
#include "BRSGameSession.h"
#include "BRSGameState_Base.h"
#include "BRSPlayerController_Base.h"
#include "BRSPlayerState_Base.h"
//#include "BRSCharacter.h"
#include "Online.h"
#include "OnlineSubsystem.h"
#include "OnlineIdentityInterface.h"
#include "OnlinePartyInterface.h"
#include "OnlineKeyValuePair.h"
#include "OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

#include "BRSLoadingScreenUMG.h"
#include "BRSMessageWindowUMG.h"
#include "BRSWaitDialogUMG.h"

#include "BRSGameInstanceSettings.h"

#include "BRSLoadingScreen.h"
#include "BRSGameViewportClient.h"



UBRSGameInstance::UBRSGameInstance(const FObjectInitializer & ObjectInitializer) 
	: Super(ObjectInitializer)
{
	AllMaps.Empty();
	GenerateMapList(AllMaps);

	CurrentState = EBRSGameInstanceState::None;
	bStartingInstance = false;
	//static ConstructorHelpers::FObjectFinder<UBlueprint> LoadingScreenObj(TEXT("/Game/UMG/LoadingScreen/LoadingScreen"));
	//if (LoadingScreenObj.Succeeded())
	//{
	//	LoadingScreenWidgetClass = (UClass*)LoadingScreenObj.Object->GeneratedClass;
	//}

	//static ConstructorHelpers::FObjectFinder<UBlueprint> MessageWindowObj(TEXT("/Game/UMG/Windows/MessageWindow"));
	//if (MessageWindowObj.Succeeded())
	//{
	//	MessageWindowWidgetClass = (UClass*)MessageWindowObj.Object->GeneratedClass;
	//}

	//static ConstructorHelpers::FObjectFinder<UBlueprint> WaitDialogObj(TEXT("/Game/UMG/MessageWindow/WaitDialog"));
	//if (WaitDialogObj.Succeeded())
	//{
	//	WaitDialogWidgetClass = (UClass*)WaitDialogObj.Object->GeneratedClass;
	//}
	bRegenerateMessageWindow = false;
}


void UBRSGameInstance::Init()
{
	Super::Init();

	const auto OnlineSub = IOnlineSubsystem::Get();
	//UE_LOG(LogTemp, Log, TEXT("OnlineSubsystem: %s"), OnlineSub ? TEXT("IsValid") : TEXT("Not Valid"));
	check(OnlineSub);
	const auto IdentityInterface = OnlineSub->GetIdentityInterface();
	check(IdentityInterface.IsValid());
	const auto SessionInterface = OnlineSub->GetSessionInterface();
	check(SessionInterface.IsValid());


	// bind any OSS delegates we needs to handle
	for (int i = 0; i < MAX_LOCAL_PLAYERS; ++i)
	{
		IdentityInterface->AddOnLoginStatusChangedDelegate_Handle(i, FOnLoginStatusChangedDelegate::CreateUObject(this, &UBRSGameInstance::HandleUserLoginChanged));
	}

	IdentityInterface->AddOnControllerPairingChangedDelegate_Handle(FOnControllerPairingChangedDelegate::CreateUObject(this, &UBRSGameInstance::HandleControllerPairingChanged));

	FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject(this, &UBRSGameInstance::HandleAppWillDeactivate);

	FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUObject(this, &UBRSGameInstance::HandleAppSuspend);
	FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddUObject(this, &UBRSGameInstance::HandleAppResume);

	FCoreDelegates::OnSafeFrameChangedEvent.AddUObject(this, &UBRSGameInstance::HandleSafeFrameChanged);
	FCoreDelegates::OnControllerConnectionChange.AddUObject(this, &UBRSGameInstance::HandleControllerConnectionChange);
	FCoreDelegates::ApplicationLicenseChange.AddUObject(this, &UBRSGameInstance::HandleAppLicenseUpdate);



	// Map/Demo Load Delegates
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UBRSGameInstance::OnPreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UBRSGameInstance::OnPostLoadMap);
	FCoreUObjectDelegates::PostDemoPlay.AddUObject(this, &UBRSGameInstance::OnPostDemoPlay);

	bPendingEnableSplitscreen = false;

	OnlineSub->AddOnConnectionStatusChangedDelegate_Handle(FOnConnectionStatusChangedDelegate::CreateUObject(this, &UBRSGameInstance::HandleNetworkConnectionStatusChanged));

	OnEndSessionCompleteDelegate = FOnEndSessionCompleteDelegate::CreateUObject(this, &UBRSGameInstance::OnEndSessionComplete);

	SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UBRSGameInstance::HandleSessionUserInviteAccepted));


	// Register delegate for ticker callback
	TickDelegate = FTickerDelegate::CreateUObject(this, &UBRSGameInstance::Tick);
	TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(TickDelegate);

	if (!GetDefault<UBRSGameInstanceSettings>()->GetMessageWindowWidgetClass()->IsValidLowLevel())
	{
		UE_LOG(LogTemp, Warning, TEXT("UBRSGameInstance: Message Dialog Class is invalid. Check the config file!"));
	}	
	if (!GetDefault<UBRSGameInstanceSettings>()->GetWaitDialogWidgetClass()->IsValidLowLevel())
	{
		UE_LOG(LogTemp, Warning, TEXT("UBRSGameInstance: Waiting Dialog Class is invalid. Check the config file!"));
	}	
}

void UBRSGameInstance::Shutdown()
{
	Super::Shutdown();
	//Unregister Ticker
	FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);
}

void UBRSGameInstance::StartGameInstance()
{
//#if PLATFORM_PS4 == 0
	TGuardValue<bool> RestoreStartingInstance(bStartingInstance, true);
	

	TCHAR Parm[4096] = TEXT("");

	const TCHAR* Cmd = FCommandLine::Get();

	// Catch the case where we want to override the map name on startup (used for connecting to other MP instances)
	if (FParse::Token(Cmd, Parm, ARRAY_COUNT(Parm), 0) && Parm[0] != '-')
	{
		// if we're 'overriding' with the default map anyway, don't set a bogus 'playing' state.
		if (!GetMainMenuMap().Contains(Parm))
		{
			FURL DefaultURL;
			DefaultURL.LoadURLConfig(TEXT("DefaultPlayer"), GGameIni);

			FURL URL(&DefaultURL, Parm, TRAVEL_Partial);

			if (URL.Valid)
			{
				UEngine* const Engine = GetEngine();

				FString Error;

				const EBrowseReturnVal::Type BrowseRet = Engine->Browse(*WorldContext, URL, Error);

				if (BrowseRet == EBrowseReturnVal::Success)
				{
					// Success, we loaded the map, go directly to playing state
					GotoState(EBRSGameInstanceState::Playing);
					return;
				}
				else if (BrowseRet == EBrowseReturnVal::Pending)
				{
					// Assume network connection
					LoadFrontEndMap(GetMainMenuMap());
					AddNetworkFailureHandlers();
					ShowLoadingScreen();
					GotoState(EBRSGameInstanceState::Playing);
					return;
				}
			}
		}
	}
//#endif

	GotoInitialState();
}


bool UBRSGameInstance::HandleOpenCommand(const TCHAR* Cmd, FOutputDevice& Ar, UWorld* InWorld)
{
	bool const bOpenSuccessful = Super::HandleOpenCommand(Cmd, Ar, InWorld);
	if (bOpenSuccessful)
	{
		ShowLoadingScreen();
		GotoState(EBRSGameInstanceState::Playing);
	}

	return bOpenSuccessful;
}

TSubclassOf<AGameModeBase> UBRSGameInstance::OverrideGameModeClass(TSubclassOf<AGameModeBase> GameModeClass, const FString& MapName, const FString& Options, const FString& Portal) const
{
	const FString MainMenuMapName = GetMainMenuMap().RightChop(GetMainMenuMap().Find(TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromEnd));
	const FString WelcomeScreenMapName = GetWelcomeScreenMap().RightChop(GetWelcomeScreenMap().Find(TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromEnd));

	UE_LOG(LogTemp, Log, TEXT("OVERRIDE GAME MODE CLASS (%s) %s %s"), *MapName, *MainMenuMapName, *WelcomeScreenMapName);

	if (MapName == MainMenuMapName || MapName == WelcomeScreenMapName)
	{
		UE_LOG(LogTemp, Log, TEXT("Hello this is a menu: (%s)"), *GameModeClass->GetName());
		return GameModeClass;
	}

	return Super::OverrideGameModeClass(GameModeClass, MapName, Options, Portal);
}

void UBRSGameInstance::OnPreLoadMap(const FString& inString)
{
	// enable splitscreen for game since we are out of the menu

	//if (bPendingEnableSplitscreen)
	//{
	//	// Allow splitscreen
	//	GetGameViewportClient()->SetDisableSplitscreenOverride(false);
	//	bPendingEnableSplitScreen = false;
	//}
	if (GetCurrentState() == EBRSGameInstanceState::MessageMenu)
	{
		UE_LOG(LogTemp, Log, TEXT("LoadingMap From Message Window State"));
		bRegenerateMessageWindow = true;
	}
}

void UBRSGameInstance::OnPostLoadMap(UWorld* World)
{
	// todo: Map is loaded hide loading screen
	HideLoadingScreen();
	//FString MapPath = GetDefault<UBRSGameInstanceSettings>()->GetMainMenuMap();
	//FString Left;
	//FString Right;
	//MapPath.Split("/", &Left, &Right, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	//if (GetWorld()->GetMapName().Equals(Right))
	//{
	//	UE_LOG(LogTemp, Log, TEXT("Main Menu Map loaded!!"));
	//	if (CurrentState == EBRSGameInstanceState::MessageMenu)
	//	{
	//		BeginMessageMenuState();
	//	}
	//}

	if (GetCurrentState() == EBRSGameInstanceState::MessageMenu)
	{
		UE_LOG(LogTemp, Log, TEXT("Loaded Map From Message Window State"));
		bRegenerateMessageWindow = false;
		PendingMessage = LastPendingMessage;
		BeginMessageMenuState();
		//MessageWindowWidget->AddToViewport(ZORDER_MESSAGEWINDOW);
	}
}

void UBRSGameInstance::OnPostDemoPlay()
{
	
}

void UBRSGameInstance::OnUserCanPlayInvite(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults)
{
	CleanupOnlinePrivilegeTask();
	//if (WelcomeMenuUI.IsValid())
	//{
	//	WelcomeMenuUI->LockControls(false);
	//}

	if (PrivilegeResults == (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures)
	{
		if (UserId == *PendingInvite.UserId)
		{
			PendingInvite.bPrivilegesCheckedAndAllowed = true;
		}
	}
	else
	{
		DisplayOnlinePrivilegeFailureDialogs(UserId, Privilege, PrivilegeResults);
		GotoState(EBRSGameInstanceState::WelcomeScreen);
	}
}



/** Returns true if the game is in online mode */
bool UBRSGameInstance::GetIsOnline() const 
{ 
	return bIsOnline; 
}

void UBRSGameInstance::SetIsOnline(bool bInIsOnline)
{
	bIsOnline = bInIsOnline;
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub)
	{
		for (int32 i = 0; i < LocalPlayers.Num(); ++i)
		{
			ULocalPlayer* LocalPlayer = LocalPlayers[i];

			TSharedPtr<const FUniqueNetId> PlayerId = LocalPlayer->GetPreferredUniqueNetId();
			if (PlayerId.IsValid())
			{
				OnlineSub->SetUsingMultiplayerFeatures(*PlayerId, bIsOnline);
			}
		}
	}
}

bool UBRSGameInstance::IsLocalPlayerOnline(ULocalPlayer* LocalPlayer)
{
	if (LocalPlayer == NULL)
	{
		return false;
	}
	const auto OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		const auto IdentityInterface = OnlineSub->GetIdentityInterface();
		if (IdentityInterface.IsValid())
		{
			auto UniqueId = LocalPlayer->GetCachedUniqueNetId();
			if (UniqueId.IsValid())
			{
				const auto LoginStatus = IdentityInterface->GetLoginStatus(*UniqueId);
				if (LoginStatus == ELoginStatus::LoggedIn)
				{
					return true;
				}
			}
		}
	}

	return false;
}


void UBRSGameInstance::HandleSessionUserInviteAccepted(const bool bWasSuccess, const int32 ControllerId, TSharedPtr< const FUniqueNetId > UserId, const FOnlineSessionSearchResult& InviteResult)
{
	UE_LOG(LogOnline, Verbose, TEXT("HandleSessionUserInviteAccepted: bSuccess: %d, ControllerId: %d, User: %s"), bWasSuccess, ControllerId, UserId.IsValid() ? *UserId->ToString() : TEXT("NULL "));

	if (!bWasSuccess)
	{
		return;
	}

	if (!InviteResult.IsValid())
	{
		UE_LOG(LogOnline, Warning, TEXT("Invite accept returned no search result."));
		return;
	}

	if (!UserId.IsValid())
	{
		UE_LOG(LogOnline, Warning, TEXT("Invite accept returned no user."));
		return;
	}

	// Set the pending invite, and then go to the initial screen, which is where we will process it
	PendingInvite.ControllerId = ControllerId;
	PendingInvite.UserId = UserId;
	PendingInvite.InviteResult = InviteResult;
	PendingInvite.bPrivilegesCheckedAndAllowed = false;

	GotoState(EBRSGameInstanceState::PendingInvite);
}

void UBRSGameInstance::HandleNetworkConnectionStatusChanged(EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus)
{
	UE_LOG(LogOnlineGame, Warning, TEXT("UBRSGameInstance::HandleNetworkConnectionStatusChanged: %s"), EOnlineServerConnectionStatus::ToString(ConnectionStatus));

//#if SHOOTER_CONSOLE_UI
//	// If we are disconnected from server, and not currently at (or heading to) the welcome screen
//	// then display a message on consoles
//	if (bIsOnline &&
//		PendingState != ShooterGameInstanceState::WelcomeScreen &&
//		CurrentState != ShooterGameInstanceState::WelcomeScreen &&
//		ConnectionStatus != EOnlineServerConnectionStatus::Connected)
//	{
//		UE_LOG(LogOnlineGame, Log, TEXT("UShooterGameInstance::HandleNetworkConnectionStatusChanged: Going to main menu"));
//
//		// Display message on consoles
//#if PLATFORM_XBOXONE
//		const FText ReturnReason = NSLOCTEXT("NetworkFailures", "ServiceUnavailable", "Connection to Xbox LIVE has been lost.");
//#elif PLATFORM_PS4
//		const FText ReturnReason = NSLOCTEXT("NetworkFailures", "ServiceUnavailable", "Connection to \"PSN\" has been lost.");
//#else
//		const FText ReturnReason = NSLOCTEXT("NetworkFailures", "ServiceUnavailable", "Connection has been lost.");
//#endif
//		const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
//
//		ShowMessageThenGotoState(ReturnReason, OKButton, FText::GetEmpty(), ShooterGameInstanceState::MainMenu);
//	}
//
//	CurrentConnectionStatus = ConnectionStatus;
//#endif
}

void UBRSGameInstance::HandleUserLoginChanged(int32 GameUserIndex, ELoginStatus::Type PreviousLoginStatus, ELoginStatus::Type LoginStatus, const FUniqueNetId& UserId)
{
	const bool bDowngraded = (LoginStatus == ELoginStatus::NotLoggedIn && !GetIsOnline()) || (LoginStatus != ELoginStatus::LoggedIn && GetIsOnline());

	UE_LOG(LogOnline, Log, TEXT("HandleUserLoginChanged: bDownGraded: %i"), (int)bDowngraded);

	TSharedPtr<GenericApplication> GenericApplication = FSlateApplication::Get().GetPlatformApplication();
	bIsLicensed = GenericApplication->ApplicationLicenseValid();

	// Find the local player associated with this unique net id
	ULocalPlayer * LocalPlayer = FindLocalPlayerFromUniqueNetId(UserId);

	// If this user is signed out, but was previously signed in, punt to welcome (or remove splitscreen if that makes sense)
	if (LocalPlayer != NULL)
	{
		if (bDowngraded)
		{
			UE_LOG(LogOnline, Log, TEXT("HandleUserLoginChanged: Player logged out: %s"), *UserId.ToString());

			LabelPlayerAsQuitter(LocalPlayer);

			// Check to see if this was the master, or if this was a split-screen player on the client
			if (LocalPlayer == GetFirstGamePlayer() || GetIsOnline())
			{
				HandleSignInChangeMessaging();
			}
			else
			{
				// Remove local split-screen players from the list
				RemoveExistingLocalPlayer(LocalPlayer);
			}
		}
	}
}

void UBRSGameInstance::HandleAppWillDeactivate()
{
	if (CurrentState == EBRSGameInstanceState::Playing)
	{
		// Just have the first player controller pause the game.
		UWorld* const GameWorld = GetWorld();
		if (GameWorld)
		{
			// protect against a second pause menu loading on top of an existing one if someone presses the Jewel / PS buttons.
			bool bNeedsPause = true;
			for (FConstControllerIterator It = GameWorld->GetControllerIterator(); It; ++It)
			{
				ABRSPlayerController_Base* Controller = Cast<ABRSPlayerController_Base>(*It);
				if (Controller && (Controller->IsPaused() || Controller->IsInMenu()))
				{
					bNeedsPause = false;
					break;
				}
			}

			if (bNeedsPause)
			{
				// Pause when needed
				ABRSPlayerController_Base* const Controller = Cast<ABRSPlayerController_Base>(GameWorld->GetFirstPlayerController());
				if (Controller)
				{
					Controller->ShowPauseMenu(true);
				}
			}
		}
	}
}

void UBRSGameInstance::HandleAppSuspend()
{
	// Players will lose connection on resume. However it is possible the game will exit before we get a resume, so we must kick off round end events here.
	UE_LOG(LogOnline, Warning, TEXT("UShooterGameInstance::HandleAppSuspend"));
	UWorld* const World = GetWorld();
	ABRSGameState_Base* const GameState = World != NULL ? World->GetGameState<ABRSGameState_Base>() : NULL;

	if (CurrentState != EBRSGameInstanceState::None && CurrentState != GetInitialState())
	{
		UE_LOG(LogOnline, Warning, TEXT("UShooterGameInstance::HandleAppSuspend: Sending round end event for players"));

		// Send round end events for local players
		for (int i = 0; i < LocalPlayers.Num(); ++i)
		{
			auto BRSPC = Cast<ABRSPlayerController_Base>(LocalPlayers[i]->PlayerController);
			if (BRSPC)
			{
				// Assuming you can't win if you quit early
				BRSPC->ClientSendRoundEndEvent(false, GameState->ElapsedTime);
			}
		}
	}
}

void UBRSGameInstance::HandleAppResume()
{
	UE_LOG(LogOnline, Log, TEXT("UShooterGameInstance::HandleAppResume"));

	if (CurrentState != EBRSGameInstanceState::None && CurrentState != GetInitialState())
	{
		UE_LOG(LogOnline, Warning, TEXT("UShooterGameInstance::HandleAppResume: Attempting to sign out players"));

		for (int32 i = 0; i < LocalPlayers.Num(); ++i)
		{
			if (LocalPlayers[i]->GetCachedUniqueNetId().IsValid() && !IsLocalPlayerOnline(LocalPlayers[i]))
			{
				UE_LOG(LogOnline, Log, TEXT("UShooterGameInstance::HandleAppResume: Signed out during resume."));
				HandleSignInChangeMessaging();
				break;
			}
		}
	}
}

void UBRSGameInstance::HandleAppLicenseUpdate()
{
	TSharedPtr<GenericApplication> GenericApplication = FSlateApplication::Get().GetPlatformApplication();
	bIsLicensed = GenericApplication->ApplicationLicenseValid();
}

void UBRSGameInstance::HandleSafeFrameChanged()
{
	UCanvas::UpdateAllCanvasSafeZoneData();
}

void UBRSGameInstance::HandleControllerPairingChanged(int GameUserIndex, const FUniqueNetId& PreviousUser, const FUniqueNetId& NewUser)
{
	UE_LOG(LogOnlineGame, Log, TEXT("UBRSGameInstance::HandleControllerPairingChanged GameUserIndex %d PreviousUser '%s' NewUser '%s'"),
		GameUserIndex, *PreviousUser.ToString(), *NewUser.ToString());

	if (CurrentState == EBRSGameInstanceState::WelcomeScreen)
	{
		// Don't care about pairing changes at welcome screen
		return;
	}

//#if SHOOTER_CONSOLE_UI && PLATFORM_XBOXONE
//	if (IgnorePairingChangeForControllerId != -1 && GameUserIndex == IgnorePairingChangeForControllerId)
//	{
//		// We were told to ignore
//		IgnorePairingChangeForControllerId = -1;	// Reset now so there there is no chance this remains in a bad state
//		return;
//	}
//
//	if (PreviousUser.IsValid() && !NewUser.IsValid())
//	{
//		// Treat this as a disconnect or signout, which is handled somewhere else
//		return;
//	}
//
//	if (!PreviousUser.IsValid() && NewUser.IsValid())
//	{
//		// Treat this as a signin
//		ULocalPlayer * ControlledLocalPlayer = FindLocalPlayerFromControllerId(GameUserIndex);
//
//		if (ControlledLocalPlayer != NULL && !ControlledLocalPlayer->GetCachedUniqueNetId().IsValid())
//		{
//			// If a player that previously selected "continue without saving" signs into this controller, move them back to welcome screen
//			HandleSignInChangeMessaging();
//		}
//
//		return;
//	}
//
//	// Find the local player currently being controlled by this controller
//	ULocalPlayer * ControlledLocalPlayer = FindLocalPlayerFromControllerId(GameUserIndex);
//
//	// See if the newly assigned profile is in our local player list
//	ULocalPlayer * NewLocalPlayer = FindLocalPlayerFromUniqueNetId(NewUser);
//
//	// If the local player being controlled is not the target of the pairing change, then give them a chance 
//	// to continue controlling the old player with this controller
//	if (ControlledLocalPlayer != nullptr && ControlledLocalPlayer != NewLocalPlayer)
//	{
//		UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GetGameViewportClient());
//
//		if (ShooterViewport != nullptr)
//		{
//			ShooterViewport->ShowDialog(
//				nullptr,
//				EShooterDialogType::Generic,
//				NSLOCTEXT("ProfileMessages", "PairingChanged", "Your controller has been paired to another profile, would you like to switch to this new profile now? Selecting YES will sign out of the previous profile."),
//				NSLOCTEXT("DialogButtons", "YES", "A - YES"),
//				NSLOCTEXT("DialogButtons", "NO", "B - NO"),
//				FOnClicked::CreateUObject(this, &UBRSGameInstance::OnPairingUseNewProfile),
//				FOnClicked::CreateUObject(this, &UBRSGameInstance::OnPairingUsePreviousProfile)
//				);
//		}
//	}
//#endif
}

void UBRSGameInstance::HandleControllerConnectionChange(bool bIsConnection, int32 Unused, int32 GameUserIndex)
{
	UE_LOG(LogOnlineGame, Log, TEXT("UBRSGameInstance::HandleControllerConnectionChange bIsConnection %d GameUserIndex %d"),
		bIsConnection, GameUserIndex);

	if (!bIsConnection)
	{
		// Controller was disconnected

		// Find the local player associated with this user index
		ULocalPlayer * LocalPlayer = FindLocalPlayerFromControllerId(GameUserIndex);

		if (LocalPlayer == NULL)
		{
			return;		// We don't care about players we aren't tracking
		}

		// Invalidate this local player's controller id.
		LocalPlayer->SetControllerId(-1);
	}
}

void UBRSGameInstance::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogOnline, Log, TEXT("UBRSGameInstance::OnEndSessionComplete: Session=%s bWasSuccessful=%s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
		if (Sessions.IsValid())
		{
			Sessions->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);
			Sessions->ClearOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegateHandle);
			Sessions->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
		}
	}

	// continue
	CleanupSessionOnReturnToMenu();
}

void UBRSGameInstance::CleanupSessionOnReturnToMenu()
{
	bool bPendingOnlineOp = false;

	// end online game and then destroy it
	IOnlineSubsystem * OnlineSub = IOnlineSubsystem::Get();
	IOnlineSessionPtr Sessions = (OnlineSub != NULL) ? OnlineSub->GetSessionInterface() : NULL;

	if (Sessions.IsValid())
	{
		EOnlineSessionState::Type SessionState = Sessions->GetSessionState(GameSessionName);
		//UE_LOG(LogOnline, Log, TEXT("Session %s is '%s'"), *GameSessionName.ToString(), EOnlineSessionState::ToString(SessionState));

		if (EOnlineSessionState::InProgress == SessionState)
		{
			//UE_LOG(LogOnline, Log, TEXT("Ending session %s on return to main menu"), *GameSessionName.ToString());
			OnEndSessionCompleteDelegateHandle = Sessions->AddOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
			Sessions->EndSession(GameSessionName);
			bPendingOnlineOp = true;
		}
		else if (EOnlineSessionState::Ending == SessionState)
		{
			//UE_LOG(LogOnline, Log, TEXT("Waiting for session %s to end on return to main menu"), *GameSessionName.ToString());
			OnEndSessionCompleteDelegateHandle = Sessions->AddOnEndSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
			bPendingOnlineOp = true;
		}
		else if (EOnlineSessionState::Ended == SessionState || EOnlineSessionState::Pending == SessionState)
		{
			//UE_LOG(LogOnline, Log, TEXT("Destroying session %s on return to main menu"), *GameSessionName.ToString());
			OnDestroySessionCompleteDelegateHandle = Sessions->AddOnDestroySessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
			Sessions->DestroySession(GameSessionName);
			bPendingOnlineOp = true;
		}
		else if (EOnlineSessionState::Starting == SessionState)
		{
			//UE_LOG(LogOnline, Log, TEXT("Waiting for session %s to start, and then we will end it to return to main menu"), *GameSessionName.ToString());
			OnStartSessionCompleteDelegateHandle = Sessions->AddOnStartSessionCompleteDelegate_Handle(OnEndSessionCompleteDelegate);
			bPendingOnlineOp = true;
		}
	}

	if (!bPendingOnlineOp)
	{
		//GEngine->HandleDisconnect( GetWorld(), GetWorld()->GetNetDriver() );
	}
}


void UBRSGameInstance::HandleSignInChangeMessaging()
{
	// Master user signed out, go to initial state (if we aren't there already)
	if (CurrentState != GetInitialState())
	{
//#if SHOOTER_CONSOLE_UI
//		// Display message on consoles
//		const FText ReturnReason = NSLOCTEXT("ProfileMessages", "SignInChange", "Sign in status change occurred.");
//		const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
//
//		ShowMessageThenGotoState(ReturnReason, OKButton, FText::GetEmpty(), GetInitialState());
//#else								
		GotoInitialState();
//#endif
	}
}


void UBRSGameInstance::RemoveExistingLocalPlayer(ULocalPlayer* ExistingPlayer)
{
	check(ExistingPlayer);
	if (ExistingPlayer->PlayerController != NULL)
	{
		// todo: Kill the player
		//ABRSCharacter* MyPawn = Cast<ABRSCharacter>(ExistingPlayer->PlayerController->GetPawn());
		//if (MyPawn)
		//{
		//	MyPawn->Suicide();
		//}
	}

	// Remove local split-screen players from the list
	RemoveLocalPlayer(ExistingPlayer);
}

void UBRSGameInstance::RemoveSplitScreenPlayers()
{
	// if we had been split screen, toss the extra players now
	// remove every player, back to front, except the first one
	while (LocalPlayers.Num() > 1)
	{
		ULocalPlayer* const PlayerToRemove = LocalPlayers.Last();
		RemoveExistingLocalPlayer(PlayerToRemove);
	}
}


void UBRSGameInstance::GotoState(EBRSGameInstanceState::Type NextState)
{
	UE_LOG(LogOnline, Log, TEXT("GotoState: NewState: %s"), *GameInstanceStateToString(NextState));
	PendingState = NextState;
}
void UBRSGameInstance::MaybeChangeState()
{
	if ((PendingState != CurrentState) && (PendingState != EBRSGameInstanceState::None))
	{
		EBRSGameInstanceState::Type const OldState = CurrentState;
		// end current state
		EndCurrentState(PendingState);
		// begin new state
		BeginNewState(PendingState, OldState);

		// clear pending change
		PendingState = EBRSGameInstanceState::None;
	}
}

void UBRSGameInstance::EndCurrentState(EBRSGameInstanceState::Type NextState)
{
	switch (CurrentState)
	{
	case EBRSGameInstanceState::PendingInvite:
		EndPendingInviteState();
		break;
	case EBRSGameInstanceState::WelcomeScreen:
		EndWelcomeScreenState();
		break;
	case EBRSGameInstanceState::MainMenu:
		EndMainMenuState();
		break;
	case EBRSGameInstanceState::MessageMenu:
		EndMessageMenuState();
		break;
	case EBRSGameInstanceState::Playing:
		EndPlayingState();
		break;
	default:
		break;
	}
	CurrentState = EBRSGameInstanceState::None;
}
void UBRSGameInstance::BeginNewState(EBRSGameInstanceState::Type NewState, EBRSGameInstanceState::Type PrevState)
{
	// per-state custom starting code here
	switch(NewState)
	{
	case EBRSGameInstanceState::PendingInvite:
		BeginPendingInviteState();
		break;
	case EBRSGameInstanceState::WelcomeScreen:
		BeginWelcomeScreenState();
		break;
	case EBRSGameInstanceState::MainMenu:
		BeginMainMenuState();
		break;
	case EBRSGameInstanceState::MessageMenu:
		BeginMessageMenuState();
		break;
	case EBRSGameInstanceState::Playing:
		BeginPlayingState();
		break;
	default:
		break;
	}
	CurrentState = NewState;
}

void UBRSGameInstance::BeginPendingInviteState()
{
	if (LoadFrontEndMap(GetMainMenuMap()))
	{
		StartOnlinePrivilegeTask(IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate::CreateUObject(this, &UBRSGameInstance::OnUserCanPlayInvite), EUserPrivileges::CanPlayOnline, PendingInvite.UserId);
	}
	else
	{
		GotoState(EBRSGameInstanceState::WelcomeScreen);
	}
}

void UBRSGameInstance::EndPendingInviteState()
{
	// cleanup in case the state changed before the pending invite was handled.
	CleanupOnlinePrivilegeTask();
}

void UBRSGameInstance::BeginWelcomeScreenState()
{
	//this must come before split screen player removal so that the OSS sets all players to not using online features.
	SetIsOnline(false);

	// Remove any possible splitscreen players
	RemoveSplitScreenPlayers();

	LoadFrontEndMap(GetWelcomeScreenMap());

	ULocalPlayer* const LocalPlayer = GetFirstGamePlayer();
	LocalPlayer->SetCachedUniqueNetId(nullptr);

	// todo: Show Welcome screen

	// Disallow splitscreen (we will allow while in the playing state)
	GetGameViewportClient()->SetDisableSplitscreenOverride(true);
}

void UBRSGameInstance::EndWelcomeScreenState()
{
	// todo: Hide Welcome screen
}

void UBRSGameInstance::BeginMainMenuState()
{
	// Make sure we're not showing the loadscreen
	HideLoadingScreen();

	SetIsOnline(false);

	// Disallow splitscreen
	GetGameViewportClient()->SetDisableSplitscreenOverride(true);

	// Remove any possible splitscren players
	RemoveSplitScreenPlayers();

	// Set presence to menu state for the owning player
	SetPresenceForLocalPlayers(FVariantData(FString(TEXT("OnMenu"))));

	// load startup map
	LoadFrontEndMap(GetMainMenuMap());

	// player 0 gets to own the UI
	ULocalPlayer* const Player = GetFirstGamePlayer();

	// todo: add MainMenu

//#if !SHOOTER_CONSOLE_UI
//	// The cached unique net ID is usually set on the welcome screen, but there isn't
//	// one on PC/Mac, so do it here.
//	if (Player != nullptr)
//	{
//		Player->SetControllerId(0);
//		Player->SetCachedUniqueNetId(Player->GetUniqueNetIdFromCachedControllerId());
//	}
//#endif

	RemoveNetworkFailureHandlers();
}

void UBRSGameInstance::EndMainMenuState()
{
	// todo: Remove MainMenu
}

void UBRSGameInstance::BeginMessageMenuState()
{
	if (PendingMessage.DisplayString.IsEmpty())
	{
		UE_LOG(LogOnlineGame, Warning, TEXT("UShooterGameInstance::BeginMessageMenuState: Display string is empty"));
		GotoInitialState();
		return;
	}

	// Make sure we're not showing the loadscreen
	HideLoadingScreen();

	ShowMessageWindow(
		PendingMessage.TitleString,
		PendingMessage.DisplayString,
		PendingMessage.OKButtonString,
		PendingMessage.CancelButtonString,
		PendingMessage.NextState);

	LastPendingMessage = PendingMessage;
	PendingMessage = FBRSPendingMessage();


	//PendingMessage.DisplayString = FText::GetEmpty();
}

void UBRSGameInstance::EndMessageMenuState()
{
	// todo: hide MessageMenu
	HideMessageWindow();
}

void UBRSGameInstance::BeginPlayingState()
{
	bPendingEnableSplitscreen = true;

	// Set presence for playing in a map
	SetPresenceForLocalPlayers(FVariantData(FString(TEXT("InGame"))));

	// Make sure viewport has focus
	FSlateApplication::Get().SetAllUserFocusToGameViewport();
}

void UBRSGameInstance::EndPlayingState()
{
	// Disallow splitscreen
	GetGameViewportClient()->SetDisableSplitscreenOverride(true);

	// Clear the players' presence information
	SetPresenceForLocalPlayers(FVariantData(FString(TEXT("OnMenu"))));

	UWorld* const World = GetWorld();
	ABRSGameState_Base* const GameState = World != NULL ? World->GetGameState<ABRSGameState_Base>() : NULL;

	if (GameState)
	{
		// Send round end events for local players
		for (int i = 0; i < LocalPlayers.Num(); ++i)
		{
			auto ShooterPC = Cast<ABRSPlayerController_Base>(LocalPlayers[i]->PlayerController);
			if (ShooterPC)
			{
				// Assuming you can't win if you quit early
				ShooterPC->ClientSendRoundEndEvent(false, GameState->ElapsedTime);
			}
		}

		// Give the game state a chance to cleanup first
		GameState->RequestFinishAndExitToMainMenu();
	}
	else
	{
		// If there is no game state, make sure the session is in a good state
		CleanupSessionOnReturnToMenu();
	}
}

EBRSGameInstanceState::Type UBRSGameInstance::GetInitialState()
{
	// On PC, go directly to the main menu
	return EBRSGameInstanceState::MainMenu;
}

void UBRSGameInstance::GotoInitialState()
{
	GotoState(GetInitialState());
}

void UBRSGameInstance::SetPresenceForLocalPlayers(const FVariantData& PresenceData)
{
	const auto Presence = Online::GetPresenceInterface();
	if (Presence.IsValid())
	{
		for (int i = 0; i < LocalPlayers.Num(); ++i)
		{
			const TSharedPtr<const FUniqueNetId> UserId = LocalPlayers[i]->GetPreferredUniqueNetId();

			if (UserId.IsValid())
			{
				FOnlineUserPresenceStatus PresenceStatus;
				PresenceStatus.Properties.Add(DefaultPresenceKey, PresenceData);

				Presence->SetPresence(*UserId, PresenceStatus);
			}
		}
	}
}

void UBRSGameInstance::InternalTravelToSession(const FName& SessionName)
{
	APlayerController * const PlayerController = GetFirstLocalPlayerController();

	FText ErrorTitle =  NSLOCTEXT("NetworkErrors", "NetworkErrorTitle", "Network Error");
	if (PlayerController == nullptr)
	{
		FText ReturnReason = NSLOCTEXT("NetworkErrors", "InvalidPlayerController", "Invalid Player Controller");
		FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
		RemoveNetworkFailureHandlers();
		ShowMessageThenGoMain(ErrorTitle, ReturnReason, OKButton, FText::GetEmpty());
		return;
	}

	// travel to session
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();

	if (OnlineSub == nullptr)
	{
		FText ReturnReason = NSLOCTEXT("NetworkErrors", "OSSMissing", "OSS missing");
		FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
		RemoveNetworkFailureHandlers();
		ShowMessageThenGoMain(ErrorTitle, ReturnReason, OKButton, FText::GetEmpty());
		return;
	}

	FString URL;
	IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();

	if (!Sessions.IsValid() || !Sessions->GetResolvedConnectString(SessionName, URL))
	{
		FText FailReason = NSLOCTEXT("NetworkErrors", "TravelSessionFailed", "Travel to Session failed.");
		FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
		ShowMessageThenGoMain(ErrorTitle, FailReason, OKButton, FText::GetEmpty());
		UE_LOG(LogOnlineGame, Warning, TEXT("Failed to travel to session upon joining it"));
		return;
	}

	if(ABRSPlayerController_Base* BRSPC = Cast<ABRSPlayerController_Base>(PlayerController))
	{
		BRSPC->AddClientTravelParams(URL);
	}
	PlayerController->ClientTravel(URL, TRAVEL_Absolute);
}

void UBRSGameInstance::LabelPlayerAsQuitter(ULocalPlayer* LocalPlayer) const
{
	ABRSPlayerState_Base* const PlayerState = LocalPlayer && LocalPlayer->PlayerController ? Cast<ABRSPlayerState_Base>(LocalPlayer->PlayerController->PlayerState) : nullptr;
	if (PlayerState)
	{
		PlayerState->SetQuitter(true);
	}
}

void UBRSGameInstance::AddNetworkFailureHandlers()
{
	// Add network/travel error handlers (if they are not already there)
	if (GEngine->OnTravelFailure().IsBoundToObject(this) == false)
	{
		TravelLocalSessionFailureDelegateHandle = GEngine->OnTravelFailure().AddUObject(this, &UBRSGameInstance::TravelLocalSessionFailure);
	}
}

void UBRSGameInstance::RemoveNetworkFailureHandlers()
{
	// Remove the local session/travel failure bindings if they exist
	if (GEngine->OnTravelFailure().IsBoundToObject(this) == true)
	{
		GEngine->OnTravelFailure().Remove(TravelLocalSessionFailureDelegateHandle);
	}
}

void UBRSGameInstance::TravelLocalSessionFailure(UWorld *World, ETravelFailure::Type FailureType, const FString& ReasonString)
{
	// todo: Replace with MenuControllerClass
	ABRSPlayerController_Base* const FirstPC = Cast<ABRSPlayerController_Base>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (FirstPC != nullptr)
	{
		FText ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Join Session failed.");
		if (ReasonString.IsEmpty() == false)
		{
			ReturnReason = FText::Format(NSLOCTEXT("NetworkErrors", "JoinSessionFailedReasonFmt", "Join Session failed. {0}"), FText::FromString(ReasonString));
		}

		FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
		ShowMessageThenGoMain(FText::GetEmpty(), ReturnReason, OKButton, FText::GetEmpty());
	}
}




void UBRSGameInstance::OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result)
{
	// unhook the delegate
	ABRSGameSession* const GameSession = GetGameSession();
	if (GameSession)
	{
		GameSession->GetOnJoinSessionComplete().Remove(OnJoinSessionCompleteDelegateHandle);
	}

	// Add the splitscreen player if one exists
	if (Result == EOnJoinSessionCompleteResult::Success && LocalPlayers.Num() > 1)
	{
		auto Sessions = Online::GetSessionInterface();
		if (Sessions.IsValid() && LocalPlayers[1]->GetPreferredUniqueNetId().IsValid())
		{
			Sessions->RegisterLocalPlayer(*LocalPlayers[1]->GetPreferredUniqueNetId(), GameSessionName,
				FOnRegisterLocalPlayerCompleteDelegate::CreateUObject(this, &UBRSGameInstance::OnRegisterJoiningLocalPlayerComplete));
		}
	}
	else
	{
		// We either failed or there is only a single local user
		FinishJoinSession(Result);
	}
}




void UBRSGameInstance::ShowMessageThenGoMain(const FText& Title, const FText& Message, const FText& OKButtonString, const FText& CancelButtonString)
{
	ShowMessageThenGotoState(Title, Message, OKButtonString, CancelButtonString, EBRSGameInstanceState::MainMenu);
}
void UBRSGameInstance::ShowMessageThenGotoState(const FText& Title, const FText& Message, const FText& OKButtonString, const FText& CancelButtonString, const EBRSGameInstanceState::Type& NewState, const bool OverrideExisting, TWeakObjectPtr< ULocalPlayer > PlayerOwner)
{
	UE_LOG(LogTemp, Log, TEXT("ShowMessageThenGotoState: Message: %s, NewState: %s"), *Message.ToString(), 
		*GameInstanceStateToString(NewState));

	const bool bAtWelcomeScreen = PendingState == EBRSGameInstanceState::WelcomeScreen || CurrentState == EBRSGameInstanceState::WelcomeScreen;

	// Never override the welcome screen
	if (bAtWelcomeScreen)
	{
		UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Ignoring due to higher message priority in queue (at welcome screen)."));
		return;
	}

	const bool bAlreadyAtMessageMenu = PendingState == EBRSGameInstanceState::MessageMenu || CurrentState == EBRSGameInstanceState::MessageMenu;
	const bool bAlreadyAtDestState = PendingState == NewState || CurrentState == NewState;

	// If we are already going to the message menu, don't override unless asked to
	if (bAlreadyAtMessageMenu && PendingMessage.NextState == NewState && !OverrideExisting)
	{
		UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Ignoring due to higher message priority in queue (check 1)."));
		return;
	}

	// If we are already going to the message menu, and the next dest is welcome screen, don't override
	if (bAlreadyAtMessageMenu && PendingMessage.NextState == EBRSGameInstanceState::WelcomeScreen)
	{
		UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Ignoring due to higher message priority in queue (check 2)."));
		return;
	}

	// If we are already at the dest state, don't override unless asked
	if (bAlreadyAtDestState && !OverrideExisting)
	{
		UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Ignoring due to higher message priority in queue (check 3)"));
		return;
	}

	PendingMessage.TitleString = Title;
	PendingMessage.DisplayString = Message;
	PendingMessage.OKButtonString = OKButtonString;
	PendingMessage.CancelButtonString = CancelButtonString;
	PendingMessage.NextState = NewState;
	PendingMessage.PlayerOwner = PlayerOwner;
	
	if (CurrentState == EBRSGameInstanceState::MessageMenu)
	{
		UE_LOG(LogOnline, Log, TEXT("ShowMessageThenGotoState: Forcing new message"));
		EndMessageMenuState();
		BeginMessageMenuState();
	}
	else
	{
		GotoState(EBRSGameInstanceState::MessageMenu);
	}
}


class UBRSMessageWindowUMG* UBRSGameInstance::SimpleShowMessageWindow(const FText& Title, const FText& Message, const FText& OKButtonString, const FText& CancelButtonString)
{
	ShowMessageWindow(Title, Message, OKButtonString, CancelButtonString);
	return MessageWindowWidget;
}

FString UBRSGameInstance::GetMainMenuMap() const
{
	return GetDefault<UBRSGameInstanceSettings>()->GetMainMenuMap();
}

FString UBRSGameInstance::GetWelcomeScreenMap() const
{
	return GetDefault<UBRSGameInstanceSettings>()->GetWelcomeScreenMap();
}

void UBRSGameInstance::ShowLoadingScreen()
{
	
	// This can be confusing, so here is what is happening:
	//	For LoadMap, we use the IShooterGameLoadingScreenModule interface to show the load screen
	//  This is necessary since this is a blocking call, and our viewport loading screen won't get updated.
	//  We can't use IShooterGameLoadingScreenModule for seamless travel though
	//  In this case, we just add a widget to the viewport, and have it update on the main thread
	//  To simplify things, we just do both, and you can't tell, one will cover the other if they both show at the same time
	IBRSLoadingScreenModule* const LoadingScreenModule = FModuleManager::LoadModulePtr<IBRSLoadingScreenModule>("BRSLoadingScreen");
	if (LoadingScreenModule != nullptr)
	{
		LoadingScreenModule->StartInGameLoadingScreen();
	}

	UBRSGameViewportClient * BRSViewport = Cast<UBRSGameViewportClient>(GetGameViewportClient());
	if (BRSViewport != NULL)
	{
		BRSViewport->ShowLoadingScreen();
	}
	
	// note: the default cursor seems to crash the game sometimes. Debugging is hard because it's blueprint and sitting in another thread than the main thread.

}
void UBRSGameInstance::HideLoadingScreen()
{
	// Only hides Viewport loading screen for seamless travel
	// the IBRSGameLoadingScreenModule one will hide itself automatically because of the settings
	UBRSGameViewportClient * BRSViewport = Cast<UBRSGameViewportClient>(GetGameViewportClient());
	if (BRSViewport != NULL)
	{
		BRSViewport->HideLoadingScreen();
	}
}

void UBRSGameInstance::ShowMessageWindow(const FText& WindowTitle, const FText& Content, const FText& ButtonText, const FText& CancelButtonText, EBRSGameInstanceState::Type NextState)
{
	if (MessageWindowWidget == nullptr || !MessageWindowWidget->IsValidLowLevel())
	{
		TSubclassOf<UBRSMessageWindowUMG> MessageWindowClass = GetDefault<UBRSGameInstanceSettings>()->GetMessageWindowWidgetClass();
		// check if WidgetClass is set
		if (MessageWindowClass == nullptr || !MessageWindowClass->IsValidLowLevel())
		{
			return;
		}

		//Create Widget
		MessageWindowWidget = CreateWidget<UBRSMessageWindowUMG>(this, MessageWindowClass);
		MessageWindowConfirmHandle = MessageWindowWidget->GetOKPressedEvent().AddUObject(this, &UBRSGameInstance::MessageWindowConfirmed);

		// verify
		if (MessageWindowWidget == nullptr || !MessageWindowWidget->IsValidLowLevel())
		{
			return;
		}
	}
	MessageWindowWidget->AddToViewport(ZORDER_MESSAGEWINDOW);
	MessageWindowWidget->WindowTitle = WindowTitle;
	MessageWindowWidget->Content = Content;
	MessageWindowWidget->ButtonText = ButtonText;
	MessageWindowWidget->CancelButtonText = CancelButtonText;
	MessageWindowWidget->NextState = NextState;
}
void UBRSGameInstance::HideMessageWindow()
{
	if (MessageWindowWidget != nullptr && MessageWindowWidget->IsValidLowLevel())
	{
		MessageWindowWidget->RemoveFromViewport();
	}
}

void UBRSGameInstance::MessageWindowConfirmed()
{
}

void UBRSGameInstance::ShowWaitDialog(const FText& WindowTitle, const FText& Content)
{
	if (GetGameViewportClient())
	{
		if (WaitDialogWidget == nullptr || !WaitDialogWidget->IsValidLowLevel())
		{
			TSubclassOf<UBRSWaitDialogUMG> WaitWindowClass = GetDefault<UBRSGameInstanceSettings>()->GetWaitDialogWidgetClass();
			// check if WidgetClass is set
			if (WaitWindowClass == nullptr || !WaitWindowClass->IsValidLowLevel())
			{
				return;
			}

			//Create Widget
			WaitDialogWidget = CreateWidget<UBRSWaitDialogUMG>(this, WaitWindowClass);

			// verify
			if (WaitDialogWidget == nullptr || !WaitDialogWidget->IsValidLowLevel())
			{
				return;
			}
		}
		WaitDialogWidget->AddToViewport(ZORDER_MESSAGEWINDOW);
		WaitDialogWidget->WindowTitle = WindowTitle;
		WaitDialogWidget->Content = Content;
	}
}
void UBRSGameInstance::HideWaitDialog()
{
	if (WaitDialogWidget != nullptr && WaitDialogWidget->IsValidLowLevel())
	{
		WaitDialogWidget->RemoveFromViewport();
	}
}


void UBRSGameInstance::GenerateMapList(TArray<TSharedPtr<FString> > & OutMapList)
{
	TArray<FString> RootPaths;
	FPackageName::QueryRootContentPaths(RootPaths);
	int32 MapExtentionLen = FPackageName::GetMapPackageExtension().Len();
	for (int32 i = 0; i < RootPaths.Num(); i++)
	{
		// don't use Engine Directory for scanning
		if (!RootPaths[i].StartsWith(TEXT("/Engine/")))
		{
			TArray<FString> PackageList;
			FPackageName::FindPackagesInDirectory(PackageList, *FPackageName::LongPackageNameToFilename(RootPaths[i]));
			for (int32 j = 0; j < PackageList.Num(); j++)
			{
				// compare Right part of file to Map extention
				if ((PackageList[j].Right(MapExtentionLen) == FPackageName::GetMapPackageExtension())
					&& (!PackageList[j].Contains(TEXT("/Entry/"))) && (!PackageList[j].Contains(TEXT("/Restricted/")))
				)
				{
					//UE_LOG(LogTemp, Log, TEXT("%s | %s"), *FPaths::GetPath(PackageList[j]), *FPaths::GetBaseFilename(PackageList[j]));
					TSharedPtr<FString> BaseName = MakeShareable(new FString(FPaths::GetBaseFilename(PackageList[j])));
					// todo Ask selected GameMode of Map is supported before adding(compare Prefix for example)
					OutMapList.Add(BaseName);
				}
			}
		}
	}
}

//void UBRSGameInstance::PrintMapList()
//{
//	UE_LOG(LogTemp, Log, TEXT("Printing Map list ..."));
//
//	if (AllMaps.Num() > 0)
//	{
//		for (int32 i = 0; i < AllMaps.Num(); i++)
//		{
//			if (AllMaps[i].IsValid())
//				UE_LOG(LogTemp, Log, TEXT("Map(%d): %s"), i, **AllMaps[i].Get());
//		}
//	}
//}
//
//void UBRSGameInstance::GetMapList(TArray<FString> & OutMapList, TSubclassOf<class ABRSGameMode> ForGameMode)
//{
//	OutMapList.Empty();
//
//	if (AllMaps.Num() <= 0)
//		return;
//
//	for (int32 i = 0; i < AllMaps.Num(); i++)
//	{
//		if (AllMaps.IsValidIndex(i) && AllMaps[i].IsValid())
//		{
//			if ((ForGameMode == NULL) || ForGameMode.GetDefaultObject()->IsValidMapName(*AllMaps[i]))
//			{
//				OutMapList.Add(*AllMaps[i]);
//			}
//		}
//	}
//}


bool UBRSGameInstance::LocalGame(class ULocalPlayer* LocalPlayer, const FString &inMapName, const FString &inGameMode, const FString &inOptions)
{
	return HostGame(LocalPlayer, inMapName, inGameMode, inOptions, false, FString(TEXT("Local Game")));
}

bool UBRSGameInstance::HostGame(class ULocalPlayer* LocalPlayer, const FString &inMapName, const FString &inGameMode, const FString &inOptions, const bool bIsOnlineGame, const FString& ServerDisplayName)
{
	// Format of URL
	// /Game/Maps/[MAPNAME]?game=[GAMEMODE]?listen?bIsLanMatch other options like bots and demo filename
	// Demo -> ?DemoRev=[PATHTODEMO] (Demos/FILENAME.demo)
	// see GameMode->InitGame for other options. Take UT or Shootergame as reference

	FString const GameModeString = inGameMode.IsEmpty() ? FString("") : FString::Printf(TEXT("?game=%s"), *inGameMode);
	FString const inTravelURL = FString::Printf(TEXT("/Game/Maps/%s%s%s%s"), *inMapName, *GameModeString, *inOptions, bIsOnlineGame ? TEXT("?listen") : TEXT(""));

	if (!bIsOnlineGame)
	{
		// Show Loading Screen
		GotoState(EBRSGameInstanceState::Playing);

		TravelURL = inTravelURL;
		ShowLoadingScreen();
		FinishHostGame();

		return true;
	}

	ABRSGameSession* const GameSession = GetGameSession();
	if (GameSession)
	{
		// temp: just open URL see ShooterGame for how to handle this correctly
		//if (LoadingScreenWidget != nullptr && LoadingScreenWidget->IsValidLowLevel())
		//{
		//	LoadingScreenWidget->LoadingScreenTitle = inMapName;
		//	FinishHostDelegateHandle = LoadingScreenWidget->OnLoadingScreenVisible().AddUObject(this, &UBRSGameInstance::FinishHostGame);
		//}
		//else
		//{
		//	FinishHostGame();
		//}
		//return true;

		if (LocalPlayer && LocalPlayer->IsValidLowLevel())
		{
			OnCreatePresenceSessionCompleteDelegateHandle = GameSession->GetOnCreatePresenceSessionComplete().AddUObject(this, &UBRSGameInstance::OnCreatePresenceSessionComplete);

			TravelURL = inTravelURL;
			bool const bIsLanMatch = inTravelURL.Contains(TEXT("?bIsLanMatch"));

			//determine map name from TravelURL
			const FString& MapName = inMapName;
			if (GameSession->HostSession(LocalPlayer->GetPreferredUniqueNetId(), GameSessionName, inGameMode, MapName, bIsLanMatch, true, ABRSGameSession::DEFAULT_NUM_PLAYERS, ServerDisplayName))
			{
				// todo: Check for pending state changes. If State is pending change don't do anything
				ShowLoadingScreen();
				GotoState(EBRSGameInstanceState::Playing);
				return true;

				//TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
				//HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
				//FString HttpURL = FString(TEXT("91.228.52.204/testing/regserver.php"));

			}
		}
		else
		{
			ShowMessageWindow(
				NSLOCTEXT("HostingErrors", "ErrorTitle", "Error!"),
				NSLOCTEXT("HostingErrors", "NoLocalPlayerFound", "Could not find a valid Local Player"),
				NSLOCTEXT("DialogButtons", "OKAY", "OK"));
		}
	}

	return false;
}

/** Callback which is intended to be called upon session creation */
void UBRSGameInstance::OnCreatePresenceSessionComplete(FName SessionName, bool bWasSuccessful)
{
	ABRSGameSession* const GameSession = GetGameSession();
	if (GameSession)
	{
		GameSession->GetOnCreatePresenceSessionComplete().Remove(OnCreatePresenceSessionCompleteDelegateHandle);

		// Add the splitscreen player if one exists
		if (bWasSuccessful && LocalPlayers.Num() > 1)
		{
			auto Sessions = Online::GetSessionInterface();
			if (Sessions.IsValid() && LocalPlayers[1]->GetPreferredUniqueNetId().IsValid())
			{
				Sessions->RegisterLocalPlayer(*LocalPlayers[1]->GetPreferredUniqueNetId(), GameSessionName,
					FOnRegisterLocalPlayerCompleteDelegate::CreateUObject(this, &UBRSGameInstance::OnRegisterLocalPlayerComplete));
			}
		}
		else
		{
			// We either failed or there is only a single local user
			FinishSessionCreation(bWasSuccessful ? EOnJoinSessionCompleteResult::Success : EOnJoinSessionCompleteResult::UnknownError);
		}
	}
}

void UBRSGameInstance::OnRegisterLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result)
{
	FinishSessionCreation(Result);
}

void UBRSGameInstance::FinishSessionCreation(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		// Travel to the specified match URL

		GetWorld()->ServerTravel(TravelURL);
		//if (LoadingScreenWidget != nullptr && LoadingScreenWidget->IsValidLowLevel())
		//{
		//	FinishHostDelegateHandle = LoadingScreenWidget->OnLoadingScreenVisible().AddUObject(this, &UBRSGameInstance::FinishHostGame);
		//}
		//else
		//{
		//	FinishHostGame();
		//}
	}
	else
	{
		FText Title = NSLOCTEXT("NetworkErrors", "NetworkErrorTitle", "Network Error");
		FText ReturnReason = NSLOCTEXT("NetworkErrors", "CreateSessionFailed", "Failed to create session.");
		FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
		ShowMessageThenGoMain(Title, ReturnReason, OKButton, FText::GetEmpty());

	}
}

void UBRSGameInstance::OnRegisterJoiningLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result)
{
	FinishJoinSession(Result);
}

void UBRSGameInstance::FinishJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	FText Title = NSLOCTEXT("NetworkErrors", "NetworkErrorTitle", "Network Error");
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		FText ReturnReason;
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::SessionIsFull:
			ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Game is full.");
			break;
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Game no longer exists.");
			break;
		default:
			ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Join failed.");
			break;
		}

		FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");
		RemoveNetworkFailureHandlers();
		ShowMessageThenGoMain(Title, ReturnReason, OKButton, FText::GetEmpty());
		return;
	}

	InternalTravelToSession(GameSessionName);
}

void UBRSGameInstance::FinishHostGame()
{
	if (LoadingScreenWidget && LoadingScreenWidget->IsValidLowLevel())
	{
		LoadingScreenWidget->OnLoadingScreenVisible().Remove(FinishHostDelegateHandle);
	}
	if (TravelURL.IsEmpty())
		return;

	UE_LOG(LogTemp, Log, TEXT("Traveling to: %s"), *TravelURL);
	GetWorld()->ServerTravel(TravelURL);
}

bool UBRSGameInstance::JoinGame(const FString& URL)
{
	APlayerController * const PlayerController = GetFirstLocalPlayerController();
	if (PlayerController == nullptr)
	{
		// todo: Output Return Reason
		return false;
	}


	GotoState(EBRSGameInstanceState::Playing);
	TravelURL = URL;
	ShowLoadingScreen(); 		

	FinishJoinGame();


	return true;
}


void UBRSGameInstance::FinishJoinGame()
{
	if (TravelURL.IsEmpty())
		return;

	APlayerController * const PlayerController = GetFirstLocalPlayerController();
	if (PlayerController == nullptr)
	{
		return;
	}
	if (ABRSPlayerController_Base* BRSPC = Cast<ABRSPlayerController_Base>(PlayerController))
	{
		BRSPC->AddClientTravelParams(TravelURL);
	}
	PlayerController->ClientTravel(TravelURL, TRAVEL_Absolute);
}


ABRSGameSession* UBRSGameInstance::GetGameSession() const
{
	UWorld* const World = GetWorld();
	if (World)
	{
		AGameModeBase* const Game = World->GetAuthGameMode();
		if (Game)
		{
			return Cast<ABRSGameSession>(Game->GameSession);
		}
	}
	return nullptr;
}



void UBRSGameInstance::GetAvailableMaps(TArray<FString> & OutMapList)
{
	OutMapList.Empty();
	if (AllMaps.Num() > 0)
	{
		for (int32 i = 0; i < AllMaps.Num(); i++)
		{
			if (AllMaps.IsValidIndex(i) && AllMaps[i].IsValid())
			{
				OutMapList.Add(*AllMaps[i].Get());
			}
		}
	}
}

bool UBRSGameInstance::Tick(float DeltaSeconds)
{
	// Dedicated server doesn't need to worry about game state
	if (IsRunningDedicatedServer() == true)
	{
		return true;
	}

	MaybeChangeState();


	// todo: check license
	// todo: check connected controllers


	// If we have a pending invite, and we are at the welcome screen, and the session is properly shut down, accept it
	if (PendingInvite.UserId.IsValid() && PendingInvite.bPrivilegesCheckedAndAllowed && CurrentState == EBRSGameInstanceState::PendingInvite)
	{
		IOnlineSubsystem * OnlineSub = IOnlineSubsystem::Get();
		IOnlineSessionPtr Sessions = (OnlineSub != NULL) ? OnlineSub->GetSessionInterface() : NULL;

		if (Sessions.IsValid())
		{
			EOnlineSessionState::Type SessionState = Sessions->GetSessionState(GameSessionName);

			if (SessionState == EOnlineSessionState::NoSession)
			{
				ULocalPlayer * NewPlayerOwner = GetFirstGamePlayer();

				if (NewPlayerOwner != nullptr)
				{
					NewPlayerOwner->SetControllerId(PendingInvite.ControllerId);
					NewPlayerOwner->SetCachedUniqueNetId(PendingInvite.UserId);
					SetIsOnline(true);
					JoinSession(NewPlayerOwner, PendingInvite.InviteResult);
				}

				PendingInvite.UserId.Reset();
			}
		}
	}

	return true;
}


bool UBRSGameInstance::LoadFrontEndMap(const FString& MapName)
{
	bool bSuccess = true;

	// if already loaded, do nothing
	UWorld* const World = GetWorld();
	if (World)
	{
		FString const CurrentMapName = *World->PersistentLevel->GetOutermost()->GetName();
		//if (MapName.Find(TEXT("Highrise")) != -1)
		if (CurrentMapName == MapName)
		{
			return bSuccess;
		}
		if (World->IsPlayInEditor())
		{
			//GetGameViewportClient()->ConsoleCommand(TEXT("exit"));
			//LocalPlayers[0]->
			return bSuccess;
		}
	}
	
	// === Browse doesn't seem to load BSP which is bad for testing. Consider the commented stuff for later use
	FString Error;
	EBrowseReturnVal::Type BrowseRet = EBrowseReturnVal::Failure;
	//FURL URL(*FString::Printf(TEXT("%s"), *MapName));
	// Create default URL.
	// @note: if we change how we determine the valid start up map update LaunchEngineLoop's GetStartupMap()
	FURL DefaultURL;
	DefaultURL.LoadURLConfig(TEXT("DefaultPlayer"), GGameIni);
	//const UGameMapsSettings* GameMapsSettings = GetDefault<UGameMapsSettings>();
	FURL URL(&DefaultURL, *(MapName /* + GameMapsSettings->LocalMapOptions*/), TRAVEL_Partial);

	if (URL.Valid && !HasAnyFlags(RF_ClassDefaultObject)) //CastChecked<UEngine>() will fail if using Default__ShooterGameInstance, so make sure that we're not default
	{
		ShowLoadingScreen();
		UE_LOG(LogTemp, Log, TEXT("URL: %s"), *URL.ToString());
		BrowseRet = GetEngine()->Browse(*WorldContext, URL, Error);

		// Handle failure.
		if (BrowseRet != EBrowseReturnVal::Success)
		{
			UE_LOG(LogLoad, Fatal, TEXT("%s"), *FString::Printf(TEXT("Failed to enter %s: %s. Please check the log for errors."), *MapName, *Error));
			bSuccess = false;
		}
	}
	//GetWorld()->ServerTravel(FString::Printf(TEXT("/Game/Maps/%s"), *MapName));
	return bSuccess;
}

void UBRSGameInstance::FinishLoadFrontEndMap()
{

}

EBRSGameInstanceState::Type UBRSGameInstance::GetCurrentState()
{
	return CurrentState;
}




bool UBRSGameInstance::JoinSession(ULocalPlayer* LocalPlayer, int32 SessionIndexInSearchResults)
{
	// needs to tear anything down based on current state?

	ABRSGameSession* const GameSession = GetGameSession();
	if (GameSession)
	{
		AddNetworkFailureHandlers();

		OnJoinSessionCompleteDelegateHandle = GameSession->GetOnJoinSessionComplete().AddUObject(this, &UBRSGameInstance::OnJoinSessionComplete);
		if (GameSession->JoinSession(LocalPlayer->GetPreferredUniqueNetId(), GameSessionName, SessionIndexInSearchResults))
		{
			// If any error occured in the above, pending state would be set
			if ((PendingState == CurrentState) || (PendingState == EBRSGameInstanceState::None))
			{
				// Go ahead and go into loading state now
				// If we fail, the delegate will handle showing the proper messaging and move to the correct state
				ShowLoadingScreen();
				GotoState(EBRSGameInstanceState::Playing);
				return true;
			}
		}
	}

	return false;
}

bool UBRSGameInstance::JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& SearchResult)
{
	// needs to tear anything down based on current state?
	ABRSGameSession* const GameSession = GetGameSession();
	if (GameSession)
	{
		AddNetworkFailureHandlers();

		OnJoinSessionCompleteDelegateHandle = GameSession->GetOnJoinSessionComplete().AddUObject(this, &UBRSGameInstance::OnJoinSessionComplete);
		if (GameSession->JoinSession(LocalPlayer->GetPreferredUniqueNetId(), GameSessionName, SearchResult))
		{
			// If any error occured in the above, pending state would be set
			if ((PendingState == CurrentState) || (PendingState == EBRSGameInstanceState::None))
			{
				// Go ahead and go into loading state now
				// If we fail, the delegate will handle showing the proper messaging and move to the correct state
				ShowLoadingScreen();
				GotoState(EBRSGameInstanceState::Playing);
				return true;
			}
		}
	}

	return false;
}


bool UBRSGameInstance::JoinSession(ULocalPlayer* LocalPlayer, const FBlueprintSessionResult& SearchResult)
{
	if (SearchResult.OnlineResult.IsValid())
	{
		return JoinSession(LocalPlayer, SearchResult.OnlineResult);
	}
	return false;
}

void UBRSGameInstance::TravelToSession(const FName& SessionName)
{
	// Added to handle failures when joining using quickmatch (handles issue of joining a game that just ended, i.e. during game ending timer)
	AddNetworkFailureHandlers();
	ShowLoadingScreen();
	GotoState(EBRSGameInstanceState::Playing);
	InternalTravelToSession(SessionName);
}

bool UBRSGameInstance::FindSessions(ULocalPlayer* PlayerOwner, bool bLANMatch)
{
	bool bResult = false;

	check(PlayerOwner != nullptr);
	if (PlayerOwner)
	{
		ABRSGameSession* const GameSession = GetGameSession();
		if (GameSession)
		{
			GameSession->GetOnFindSessionsComplete().RemoveAll(this);
			OnSearchSessionsCompleteDelegateHandle = GameSession->GetOnFindSessionsComplete().AddUObject(this, &UBRSGameInstance::OnSearchSessionsComplete);

			GameSession->FindSessions(PlayerOwner->GetPreferredUniqueNetId(), GameSessionName, bLANMatch, true);
			bResult = true;
		}
	}

	return bResult;
}

void UBRSGameInstance::OpenFriends()
{
	IOnlineExternalUIPtr ExternalUI = IOnlineSubsystem::Get()->GetExternalUIInterface();

	if (ExternalUI.IsValid())
	{
		ExternalUI->ShowFriendsUI(0);
	}

}

/** Callback which is intended to be called upon finding sessions */
void UBRSGameInstance::OnSearchSessionsComplete(bool bWasSuccessful)
{
	ABRSGameSession* const Session = GetGameSession();

	TArray<FBlueprintSessionResult> FoundSessionIds;

	if (Session)
	{
		Session->GetOnFindSessionsComplete().Remove(OnSearchSessionsCompleteDelegateHandle);
		if (bWasSuccessful)
		{
			const TArray<FOnlineSessionSearchResult>& FoundServers = Session->GetSearchResults();
			if (FoundServers.Num() > 0)
			{
				for (int32 i = 0; i < FoundServers.Num(); i++)
				{
					if (FoundServers.IsValidIndex(i))
					{
 						const FOnlineSessionSearchResult& TestServer = FoundServers[i];

						FBlueprintSessionResult Test;
						Test.OnlineResult = TestServer;
						FoundSessionIds.Add(Test);
 					}
				}
			}
		}
	}
	if (OnSearchSessionsCompleteDelegate.IsBound())
	{
		OnSearchSessionsCompleteDelegate.Broadcast(FoundSessionIds);
	}
}


void UBRSGameInstance::StartOnlinePrivilegeTask(const IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate& Delegate, EUserPrivileges::Type Privilege, TSharedPtr<const FUniqueNetId > UserId)
{
	ShowWaitDialog(
		NSLOCTEXT("NetworkStatus", "WaitDialogTitle", "Waiting ..."),
		NSLOCTEXT("NetworkStatus", "CheckingPrivilegesWithServer", "Checking privileges with server.  Please wait..."));

	auto Identity = Online::GetIdentityInterface();
	if (Identity.IsValid() && UserId.IsValid())
	{
		Identity->GetUserPrivilege(*UserId, Privilege, Delegate);
	}
	else
	{
		// Can only get away with faking the UniqueNetId here because the delegates don't use it
		Delegate.ExecuteIfBound(FUniqueNetIdString(), Privilege, (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures);
	}
}

void UBRSGameInstance::CleanupOnlinePrivilegeTask()
{
	HideWaitDialog();
}

void UBRSGameInstance::DisplayOnlinePrivilegeFailureDialogs(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults)
{
	// Show warning that the user cannot play due to age restrictions
	TWeakObjectPtr<ULocalPlayer> OwningPlayer;
	if (GEngine)
	{
		for (auto It = GEngine->GetLocalPlayerIterator(GetWorld()); It; ++It)
		{
			TSharedPtr<const FUniqueNetId> OtherId = (*It)->GetPreferredUniqueNetId();
			if (OtherId.IsValid())
			{
				if (UserId == (*OtherId))
				{
					OwningPlayer = *It;
				}
			}
		}
	}

	if (GetGameViewportClient() != nullptr && OwningPlayer.IsValid())
	{
		if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::AccountTypeFailure) != 0)
		{
			IOnlineExternalUIPtr ExternalUI = Online::GetExternalUIInterface();
			if (ExternalUI.IsValid())
			{
				ExternalUI->ShowAccountUpgradeUI(UserId);
			}
		}
		else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::RequiredSystemUpdate) != 0)
		{
			ShowMessageWindow(FText::GetEmpty(),
				NSLOCTEXT("OnlinePrivilegeResult", "RequiredSystemUpdate", "A required system update is available.  Please upgrade to access online features."),
				NSLOCTEXT("DialogButtons", "OKAY", "OK"));
		}
		else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::RequiredPatchAvailable) != 0)
		{
			ShowMessageWindow(FText::GetEmpty(),
				NSLOCTEXT("OnlinePrivilegeResult", "RequiredPatchAvailable", "A required game patch is available.  Please upgrade to access online features."),
				NSLOCTEXT("DialogButtons", "OKAY", "OK"));
		}
		else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::AgeRestrictionFailure) != 0)
		{
			ShowMessageWindow(FText::GetEmpty(),
				NSLOCTEXT("OnlinePrivilegeResult", "AgeRestrictionFailure", "Cannot play due to age restrictions!"),
				NSLOCTEXT("DialogButtons", "OKAY", "OK"));
		}
		else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::UserNotFound) != 0)
		{
			ShowMessageWindow(FText::GetEmpty(),
				NSLOCTEXT("OnlinePrivilegeResult", "UserNotFound", "Cannot play due invalid user!"),
				NSLOCTEXT("DialogButtons", "OKAY", "OK"));
		}
		else if ((PrivilegeResults & (uint32)IOnlineIdentity::EPrivilegeResults::GenericFailure) != 0)
		{
			ShowMessageWindow(FText::GetEmpty(),
				NSLOCTEXT("OnlinePrivilegeResult", "GenericFailure", "Cannot play online.  Check your network connection."),
				NSLOCTEXT("DialogButtons", "OKAY", "OK"));
		}
	}
}


