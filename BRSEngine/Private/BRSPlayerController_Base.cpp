// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
//#include "Player/BRSPlayerInput.h"
#include "BRSPlayerState_Base.h"
#include "BRSGameState_Base.h"
#include "BRSGameInstance.h"
//#include "BRSMenuGameMode.h"
#include "BRSPlayerController_Base.h"
#include "BRSMenuGameMode_Base.h"

bool ABRSPlayerController_Base::InputKey(FKey Key, EInputEvent EventType, float AmountDepressed, bool bGamepad)
{
	if (EventType == EInputEvent::IE_Released && !bShowMouseCursor)
	{
		//UE_LOG(LogTemp, Log, TEXT("GamepadEvent: %d, %s"), bGamepad, *Key.ToString());
		bUsingGamepad = bGamepad;
	}
	
	if (Key.IsMouseButton())
	{
		bUsingGamepad = false;
	}

	return Super::InputKey(Key, EventType, AmountDepressed, bGamepad);
}



bool ABRSPlayerController_Base::InputAxis(FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad)
{
	//UE_LOG(LogTemp, Log, TEXT("[%f] Axis: %s, %f"), GetWorld()->TimeSeconds, *Key.ToString(), Delta/DeltaTime);
	
	if (FMath::Abs(Delta / DeltaTime) >= 2.f && !bShowMouseCursor)
	{
		bUsingGamepad = bGamepad;
	}

	return Super::InputAxis(Key, Delta, DeltaTime, NumSamples, bGamepad);
}


bool ABRSPlayerController_Base::IsInMenu() const
{
	if (GetWorld() && GetWorld()->GetAuthGameMode())
	{
		if (GetWorld()->GetAuthGameMode()->IsA(ABRSMenuGameMode_Base::StaticClass()))
		{
			return true;
		}
	}
	return false;
}

//
//void ABRSPlayerController::InitInputSystem()
//{
//	if (PlayerInput == nullptr)
//	{
//		PlayerInput = NewObject<UBRSPlayerInput>(this, UBRSPlayerInput::StaticClass());
//	}
//
//	Super::InitInputSystem();
//}
//void ABRSPlayerController::ToggleChat()
//{
//	// Stop stuff when in Menu
//	OnStopFire();
//	OnStopAltFire();
//
//	ABRSGameHUD* BRSHUD = Cast<ABRSGameHUD>(GetHUD());
//	if (BRSHUD)
//	{
//		BRSHUD->OpenChat(true, false);
//	}
//}
//void ABRSPlayerController::ToggleMenu()
//{	
//	bool bIsInMainMenu = GetWorld()->GetGameState()->GameModeClass->IsChildOf(ABRSMenuGameMode::StaticClass());
//	if (!bIsInMainMenu)
//	{
//		// Stop stuff when in Menu
//		OnStopFire();
//		OnStopAltFire();
//
//		ABRSGameHUD* BRSHUD = Cast<ABRSGameHUD>(GetHUD());
//		if (BRSHUD)
//		{
//			BRSHUD->ToggleMenu(IsInMenu());
//
//			if (IsInMenu() && GetNetMode() == NM_Standalone)
//			{
//				SetPause(true);
//			}
//			else
//			{
//				SetPause(false);
//			}
//		}
//	}
//}


void ABRSPlayerController_Base::ExitToMainMenu()
{
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if (LocalPlayer && IsLocalPlayerController())
	{
		UBRSGameInstance* const GI = Cast<UBRSGameInstance>(GetGameInstance());
		UE_LOG(LogTemp, Log, TEXT("Exit to menu (%s)"), GI ? TEXT("GI is there") : TEXT("no GI"));
		if (GI)
		{
			// tell GameInstance that we quit so it can handle the rest
			GI->LabelPlayerAsQuitter(LocalPlayer);
			GI->GotoState(EBRSGameInstanceState::MainMenu);
		}
	}
}

void ABRSPlayerController_Base::BP_RestartLevel()
{
	RestartLevel();
}

bool ABRSPlayerController_Base::IsAllowedToRestartLevel() const
{
	AGameModeBase* GM = GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr;

	bool bMainMenu = ((GM != nullptr) && (Cast<ABRSMenuGameMode_Base>(GM) != nullptr));
	return (GetNetMode() == NM_Standalone) && !bMainMenu;
}

/** Starts the online game using the session name in the PlayerState */
void ABRSPlayerController_Base::ClientStartOnlineGame_Implementation()
{
	if (!IsPrimaryPlayer())
		return;

	
	ABRSPlayerState_Base* myPlayerState = Cast<ABRSPlayerState_Base>(PlayerState);
	if(myPlayerState)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid())
			{
				UE_LOG(LogOnline, Log, TEXT("Starting session %s on client"), *myPlayerState->SessionName.ToString());
				Sessions->StartSession(myPlayerState->SessionName);
			}
		}
	}
	else
	{
		// Keep retrying until player state is replicated
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ClientStartOnlineGame, this, &ABRSPlayerController_Base::ClientStartOnlineGame_Implementation, 0.2f, false);
	}
}

/** Ends the online game using the session name in the PlayerState */
void ABRSPlayerController_Base::ClientEndOnlineGame_Implementation()
{
	if (!IsPrimaryPlayer())
		return;

	ABRSPlayerState_Base* myPlayerState = Cast<ABRSPlayerState_Base>(PlayerState);
	if (myPlayerState)
	{
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid())
			{
				UE_LOG(LogOnline, Log, TEXT("Ending session %s on client"), *myPlayerState->SessionName.ToString());
				Sessions->EndSession(myPlayerState->SessionName);
			}
		}
	}
}


void ABRSPlayerController_Base::ClientSendRoundEndEvent_Implementation(bool bIsWinner, int32 ExpendedTimeInSeconds)
{
	const auto Events = Online::GetEventsInterface();
	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);

	if (bHasSentStartEvents && LocalPlayer != nullptr && Events.IsValid())
	{
		auto UniqueId = LocalPlayer->GetPreferredUniqueNetId();

		if (UniqueId.IsValid())
		{
			FString MapName = *FPackageName::GetShortName(GetWorld()->PersistentLevel->GetOutermost()->GetName());
			ABRSPlayerState_Base* myPlayerState = Cast<ABRSPlayerState_Base>(PlayerState);
			int32 PlayerScore = myPlayerState ? myPlayerState->GetScore() : 0;


			// Fire session end event for all cases
			FOnlineEventParms Params;
			Params.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
			Params.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
			Params.Add(TEXT("ExitStatusId"), FVariantData((int32)0)); // unused
			Params.Add(TEXT("PlayerScore"), FVariantData((int32)PlayerScore));
			Params.Add(TEXT("PlayerWon"), FVariantData((bool)bIsWinner));
			Params.Add(TEXT("MapName"), FVariantData(MapName));
			Params.Add(TEXT("MapNameString"), FVariantData(MapName)); // @todo workaround for a bug in backend service, remove when fixed

			Events->TriggerEvent(*UniqueId, TEXT("PlayerSessionEnd"), Params);

			// Online matches require the MultiplayerRoundEnd event as well
			UBRSGameInstance* SGI = GetWorld() != NULL ? Cast<UBRSGameInstance>(GetWorld()->GetGameInstance()) : NULL;
			if (SGI->GetIsOnline())
			{
				FOnlineEventParms MultiplayerParams;

				ABRSGameState_Base* const MyGameState = GetWorld() != NULL ? GetWorld()->GetGameState<ABRSGameState_Base>() : NULL;
				if (ensure(MyGameState != nullptr))
				{
					MultiplayerParams.Add(TEXT("SectionId"), FVariantData((int32)0)); // unused
					MultiplayerParams.Add(TEXT("GameplayModeId"), FVariantData((int32)1)); // @todo determine game mode (ffa v tdm)
					MultiplayerParams.Add(TEXT("MatchTypeId"), FVariantData((int32)1)); // @todo abstract the specific meaning of this value across platforms
					MultiplayerParams.Add(TEXT("DifficultyLevelId"), FVariantData((int32)0)); // unused
					MultiplayerParams.Add(TEXT("TimeInSeconds"), FVariantData((float)ExpendedTimeInSeconds));
					MultiplayerParams.Add(TEXT("ExitStatusId"), FVariantData((int32)0)); // unused

					Events->TriggerEvent(*UniqueId, TEXT("MultiplayerRoundEnd"), MultiplayerParams);
				}
			}
		}

		bHasSentStartEvents = false;
	}
}

void ABRSPlayerController_Base::HandleReturnToMainMenu()
{
	// todo: Hide Scoreboard
	CleanupSessionOnReturnToMenu();
}

void ABRSPlayerController_Base::ClientReturnToMainMenu_Implementation(const FString& ReturnReasonString)
{
	UBRSGameInstance* GI = GetWorld() != NULL ? Cast<UBRSGameInstance>(GetWorld()->GetGameInstance()) : NULL;

	if (!ensure(GI != NULL))
	{
		return;
	}

	if (GetNetMode() == NM_Client)
	{
		const FText ReturnTitle = NSLOCTEXT("NetworkErrors", "ErrorTitle", "Network Error");
		const FText ReturnReason = NSLOCTEXT("NetworkErrors", "HostQuit", "The host has quit the match.");
		const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

		GI->ShowMessageThenGotoState(ReturnTitle, ReturnReason, OKButton, FText::GetEmpty(), EBRSGameInstanceState::MainMenu);
	}
	else
	{
		GI->GotoState(EBRSGameInstanceState::MainMenu);
	}

	// Clear the flag so we don't do normal end of round stuff next
	bGameEndedFrame = false;
}

/** Ends and/or destroys game session */
void ABRSPlayerController_Base::CleanupSessionOnReturnToMenu()
{
	UBRSGameInstance* GI = GetWorld() != NULL ? Cast<UBRSGameInstance>(GetWorld()->GetGameInstance()) : NULL;

	if (ensure(GI != NULL))
	{
		GI->CleanupSessionOnReturnToMenu();
	}
}




void ABRSPlayerController_Base::Say(FString Msg)
{
	// clamp msg
	Msg = Msg.Left(128);
	ServerSay(Msg, false);
}

void ABRSPlayerController_Base::TeamSay(FString Msg)
{
	// clamp msg
	Msg = Msg.Left(128);
	ServerSay(Msg, true);
}

bool ABRSPlayerController_Base::ServerSay_Validate(const FString& Msg, bool bTeamMessage) { return true; }
void ABRSPlayerController_Base::ServerSay_Implementation(const FString& Msg, bool bTeamMessage)
{
	ABRSPlayerState_Base* MyPlayerState = Cast<ABRSPlayerState_Base>(PlayerState);
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABRSPlayerController_Base* BRSPC = Cast<ABRSPlayerController_Base>(*It);
		if (BRSPC != nullptr)
		{
			if (!bTeamMessage || IsAlliedTo(BRSPC->PlayerState))
			{
				BRSPC->ClientReceiveSay(MyPlayerState, Msg, bTeamMessage);
			}
		}
	}
}


void ABRSPlayerController_Base::ClientReceiveSay_Implementation(ABRSPlayerState_Base* MsgSender, const FString& Msg, bool bTeamMessage)
{
	//ABRSGameHUD* BRSHUD = Cast<ABRSGameHUD>(GetHUD());
	//if (BRSHUD)
	//{
	//	BRSHUD->ReceiveChatMessage(MsgSender->PlayerName, Msg, bTeamMessage);
	//}

	// todo: implement override
	if (OnClientReceiveSay.IsBound())
	{
		OnClientReceiveSay.Broadcast(MsgSender, Msg, bTeamMessage);
	}
}



void ABRSPlayerController_Base::ClientReceiveServerMessage_Implementation(EServerMessageType MsgType, APlayerState* P1, APlayerState* P2)
{
	if (OnClientReceiveServerMessage.IsBound())
	{
		switch (MsgType)
		{
		case EServerMessageType::PlayerKill:
			if (P1 && P2)
			{
				OnClientReceiveServerMessage.Broadcast(
					NSLOCTEXT("ServerMessages", "msg_Kill", "{0} killed {1}."), P1, P2);
			}
			break;
		case EServerMessageType::KilledByBot:
			if (P1)
			{
				OnClientReceiveServerMessage.Broadcast(
					NSLOCTEXT("ServerMessages", "msg_KilledByBot", "{0} was killed."), P1, nullptr);
			}
			break;
		case EServerMessageType::Suicide:
			if (P1)
			{
				OnClientReceiveServerMessage.Broadcast(
					NSLOCTEXT("ServerMessages", "msg_Suicide", "{0} comitted suicide."), P1, nullptr);
			}
			break;
		case EServerMessageType::TeamKill:
			if (P1 && P2)
			{
				OnClientReceiveServerMessage.Broadcast(
					NSLOCTEXT("ServerMessages", "msg_TeamKill", "{0} team killed {1}."), P1, P2);
			}
			break;
		case EServerMessageType::PlayerLeft :
			if (P1)
			{
				OnClientReceiveServerMessage.Broadcast(
					NSLOCTEXT("ServerMessages", "msg_PlayerLeft", "{0} left the game."), P1, nullptr);
			}
			break;
		case EServerMessageType::PlayerJoined:
			if (P1)
			{
				OnClientReceiveServerMessage.Broadcast(
					NSLOCTEXT("ServerMessages", "msg_PlayerJoined", "{0} joined the game."), P1, nullptr);
			}
			break;
		case EServerMessageType::PlayerKicked:
			if (P1)
			{
				OnClientReceiveServerMessage.Broadcast(
					NSLOCTEXT("ServerMessages", "msg_PlayerKicked", "{0} was kicked from server."), P1, nullptr);
			}
			break;
		case EServerMessageType::PlayerBanned:
			if (P1)
			{
				OnClientReceiveServerMessage.Broadcast(
					NSLOCTEXT("ServerMessages", "msg_PlayerBanned", "{0} was banned from server."), P1, nullptr);
			}
			break;
		}
	}
}


