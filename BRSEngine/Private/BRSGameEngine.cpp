// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSGameInstance.h"

UBRSGameEngine::UBRSGameEngine(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

void UBRSGameEngine::Init(IEngineLoop* InEngineLoop)
{
	// Note: Lots of important things happen in Super::Init(), including spawning the player pawn in-game and
	// creating the renderer.
	Super::Init(InEngineLoop);
	
	// HACK: ApplyNonResolutionSettings() isn't virtual so we need to force apply here
	GetGameUserSettings()->ApplySettings(true);	
}

void UBRSGameEngine::HandleNetworkFailure(UWorld *World, UNetDriver *NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	// Determine if we need to change the King state based on network failures.
	// Only handle failure at this level for game or pending net drivers.
	FName NetDriverName = NetDriver ? NetDriver->NetDriverName : NAME_None;
	if (NetDriverName == NAME_GameNetDriver || NetDriverName == NAME_PendingNetDriver)
	{
		// If this net driver has already been unregistered with this world, then don't handle it.
		//if (World)
		{
			//UNetDriver * NetDriver = FindNamedNetDriver(World, NetDriverName);
			if (NetDriver)
			{
				const FText FailureTitle = NSLOCTEXT("NetworkErrors", "NetworkErrorTitle", "Network Error");
				switch (FailureType)
				{
				case ENetworkFailure::FailureReceived:
				{
					UBRSGameInstance* const GI = Cast<UBRSGameInstance>(GameInstance);
					if (GI && NetDriver->GetNetMode() == NM_Client)
					{
						const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

						// NOTE - We pass in false here to not override the message if we are already going to the main menu
						// We're going to make the assumption that someone else has a better message than "Lost connection to host" if
						// this is the case
						GI->ShowMessageThenGotoState(FailureTitle, FText::FromString(ErrorString), OKButton, FText::GetEmpty(), EBRSGameInstanceState::MainMenu, false);
					}
					break;
				}
				case ENetworkFailure::PendingConnectionFailure:
				{
					UBRSGameInstance* const GI = Cast<UBRSGameInstance>(GameInstance);
					if (GI && NetDriver->GetNetMode() == NM_Client)
					{
						const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

						// NOTE - We pass in false here to not override the message if we are already going to the main menu
						// We're going to make the assumption that someone else has a better message than "Lost connection to host" if
						// this is the case
						GI->ShowMessageThenGotoState(FailureTitle, FText::FromString(ErrorString), OKButton, FText::GetEmpty(), EBRSGameInstanceState::MainMenu, false);
					}
					break;
				}
				case ENetworkFailure::ConnectionLost:
				case ENetworkFailure::ConnectionTimeout:
				{
					UBRSGameInstance* const GI = Cast<UBRSGameInstance>(GameInstance);
					if (GI && NetDriver->GetNetMode() == NM_Client)
					{
						const FText ReturnReason = NSLOCTEXT("NetworkErrors", "HostDisconnect", "Lost connection to host.");
						const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

						// NOTE - We pass in false here to not override the message if we are already going to the main menu
						// We're going to make the assumption that someone else has a better message than "Lost connection to host" if
						// this is the case
						GI->ShowMessageThenGotoState(FailureTitle, ReturnReason, OKButton, FText::GetEmpty(), EBRSGameInstanceState::MainMenu, false);
					}
					break;
				}
				case ENetworkFailure::NetDriverAlreadyExists:
				case ENetworkFailure::NetDriverCreateFailure:
				case ENetworkFailure::OutdatedClient:
				case ENetworkFailure::OutdatedServer:
				default:
					break;
				}
			}
		}
	}
	// standard failure handling.
	Super::HandleNetworkFailure(World, NetDriver, FailureType, ErrorString);
}

void UBRSGameEngine::HandleTravelFailure(UWorld* InWorld, ETravelFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogTemp, Log, TEXT("hajkodaskjdhkjahsd as"));
	//if(InWorld)
	{
		const FText FailureTitle = NSLOCTEXT("TravelErrors", "TravelErrorsTitle", "Travel Failure");
		switch (FailureType)
		{
		case ETravelFailure::ClientTravelFailure:
		{
			UBRSGameInstance* const GI = Cast<UBRSGameInstance>(GameInstance);
			if (GI)
			{
				const FText ReturnReason = NSLOCTEXT("TravelErrors", "ClientTravelFailure", "Failure during Client Travel.");
				const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

				// NOTE - We pass in false here to not override the message if we are already going to the main menu
				// We're going to make the assumption that someone else has a better message than "Lost connection to host" if
				// this is the case
				GI->ShowMessageThenGotoState(FailureTitle, ReturnReason, OKButton, FText::GetEmpty(), EBRSGameInstanceState::MainMenu, false);
			}
			break;
		}
		case ETravelFailure::ServerTravelFailure:
		{
			UBRSGameInstance* const GI = Cast<UBRSGameInstance>(GameInstance);
			if (GI)
			{
				const FText ReturnReason = NSLOCTEXT("TravelErrors", "ClientTravelFailure", "Failure during Server Travel.");
				const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

				// NOTE - We pass in false here to not override the message if we are already going to the main menu
				// We're going to make the assumption that someone else has a better message than "Lost connection to host" if
				// this is the case
				GI->ShowMessageThenGotoState(FailureTitle, ReturnReason, OKButton, FText::GetEmpty(), EBRSGameInstanceState::MainMenu, false);
				//GI->bRegenerateMessageWindow = true;
			}
			break;
		}
		case ETravelFailure::TravelFailure:
		{
			UBRSGameInstance* const GI = Cast<UBRSGameInstance>(GameInstance);
			if (GI)
			{
				const FText ReturnReason = NSLOCTEXT("TravelErrors", "ClientTravelFailure", "Failure during Travel.");
				const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

				// NOTE - We pass in false here to not override the message if we are already going to the main menu
				// We're going to make the assumption that someone else has a better message than "Lost connection to host" if
				// this is the case
				GI->ShowMessageThenGotoState(FailureTitle, ReturnReason, OKButton, FText::GetEmpty(), EBRSGameInstanceState::MainMenu, false);
				//GI->bRegenerateMessageWindow = true;
			}
			break;
		}
		case ETravelFailure::InvalidURL:
		{
			UBRSGameInstance* const GI = Cast<UBRSGameInstance>(GameInstance);
			if (GI)
			{
				const FText ReturnReason = NSLOCTEXT("TravelErrors", "InvalidURL", "Invalid Travel URL.");
				const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

				// NOTE - We pass in false here to not override the message if we are already going to the main menu
				// We're going to make the assumption that someone else has a better message than "Lost connection to host" if
				// this is the case
				GI->ShowMessageThenGotoState(FailureTitle, ReturnReason, OKButton, FText::GetEmpty(), EBRSGameInstanceState::MainMenu, true);
				//GI->bRegenerateMessageWindow = true;
			}
			break;
		}
		case ETravelFailure::NoDownload:
		{
			UBRSGameInstance* const GI = Cast<UBRSGameInstance>(GameInstance);
			if (GI)
			{
				const FText ReturnReason = NSLOCTEXT("TravelErrors", "NoDownload", "A package is missing and the client is unable to download.");
				const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

				// NOTE - We pass in false here to not override the message if we are already going to the main menu
				// We're going to make the assumption that someone else has a better message than "Lost connection to host" if
				// this is the case
				GI->ShowMessageThenGotoState(FailureTitle, ReturnReason, OKButton, FText::GetEmpty(), EBRSGameInstanceState::MainMenu, true);
				//GI->bRegenerateMessageWindow = true;
			}
			break;
		}
		case ETravelFailure::NoLevel:
		{
			UBRSGameInstance* const GI = Cast<UBRSGameInstance>(GameInstance);
			if (GI)
			{
				const FText ReturnReason = NSLOCTEXT("TravelErrors", "NoLevel", "No level found in the loaded package.");
				const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

				// NOTE - We pass in false here to not override the message if we are already going to the main menu
				// We're going to make the assumption that someone else has a better message than "Lost connection to host" if
				// this is the case
				GI->ShowMessageThenGotoState(FailureTitle, ReturnReason, OKButton, FText::GetEmpty(), EBRSGameInstanceState::MainMenu, true);
				//GI->bRegenerateMessageWindow = true;
			}
			break;
		}
		case ETravelFailure::PackageMissing:
		{
			UBRSGameInstance* const GI = Cast<UBRSGameInstance>(GameInstance);
			if (GI)
			{
				const FText ReturnReason = NSLOCTEXT("TravelErrors", "PackageMissing", "A package is missing on client.");
				const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

				// NOTE - We pass in false here to not override the message if we are already going to the main menu
				// We're going to make the assumption that someone else has a better message than "Lost connection to host" if
				// this is the case
				GI->ShowMessageThenGotoState(FailureTitle, ReturnReason, OKButton, FText::GetEmpty(), EBRSGameInstanceState::MainMenu, true);
				//GI->bRegenerateMessageWindow = true;
			}
			break;
		}
		case ETravelFailure::PackageVersion:
		{
			UBRSGameInstance* const GI = Cast<UBRSGameInstance>(GameInstance);
			if (GI)
			{
				const FText ReturnReason = NSLOCTEXT("TravelErrors", "PackageVersion", "A package mismatch has occured on server and client.");
				const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

				// NOTE - We pass in false here to not override the message if we are already going to the main menu
				// We're going to make the assumption that someone else has a better message than "Lost connection to host" if
				// this is the case
				GI->ShowMessageThenGotoState(FailureTitle, ReturnReason, OKButton, FText::GetEmpty(), EBRSGameInstanceState::MainMenu, true);
				//GI->bRegenerateMessageWindow = true;
			}
			break;
		}
		case ETravelFailure::PendingNetGameCreateFailure:
		{
			UBRSGameInstance* const GI = Cast<UBRSGameInstance>(GameInstance);
			if (GI)
			{
				const FText ReturnReason = NSLOCTEXT("TravelErrors", "PendingNetGameCreateFailure", "Failed to create pending net game for travel.");
				const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

				// NOTE - We pass in false here to not override the message if we are already going to the main menu
				// We're going to make the assumption that someone else has a better message than "Lost connection to host" if
				// this is the case
				GI->ShowMessageThenGotoState(FailureTitle, ReturnReason, OKButton, FText::GetEmpty(), EBRSGameInstanceState::MainMenu, false);
				//GI->bRegenerateMessageWindow = true;
			}
			break;
		}
		case ETravelFailure::LoadMapFailure:
		{
			UBRSGameInstance* const GI = Cast<UBRSGameInstance>(GameInstance);
			if (GI)
			{
				const FText ReturnReason = NSLOCTEXT("TravelErrors", "LoadMapFailure", "Failed to load map.");
				const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

				// NOTE - We pass in false here to not override the message if we are already going to the main menu
				// We're going to make the assumption that someone else has a better message than "Lost connection to host" if
				// this is the case
				GI->ShowMessageThenGotoState(FailureTitle, ReturnReason, OKButton, FText::GetEmpty(), EBRSGameInstanceState::MainMenu, true);
				//GI->bRegenerateMessageWindow = true;
			}
			break;
		}
		case ETravelFailure::CheatCommands:
		case ETravelFailure::CloudSaveFailure:
		default:
			break;
		}
	}

	Super::HandleTravelFailure(InWorld, FailureType, ErrorString);
}

