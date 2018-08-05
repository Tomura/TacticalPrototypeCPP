// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineDelegateMacros.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemImpl.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemUtilsModule.h"
#include "OnlineSubsystemUtilsClasses.h"
#include "BRSOnlineLibrary.h"

#include "BRSGameMode_Base.h"

// This is taken directly from UE4 - OnlineSubsystemSteamPrivatePCH.h as a fix for the array_count macro

// @todo Steam: Steam headers trigger secure-C-runtime warnings in Visual C++. Rather than mess with _CRT_SECURE_NO_WARNINGS, we'll just
//	disable the warnings locally. Remove when this is fixed in the SDK
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996)
#endif

#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX

#pragma push_macro("ARRAY_COUNT")
#undef ARRAY_COUNT

#include <steam/steam_api.h>

#pragma pop_macro("ARRAY_COUNT")

#endif

// @todo Steam: See above
#ifdef _MSC_VER
#pragma warning(pop)
#endif

//General Log
DEFINE_LOG_CATEGORY(BRSOnlineLog);

UTexture2D * UBRSOnlineLibrary::GetSteamFriendAvatar(const TSharedPtr<const FUniqueNetId> UniqueNetId, ESteamAvatarSize AvatarSize)
{
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	if (!UniqueNetId.IsValid() || !UniqueNetId->IsValid())
	{
		UE_LOG(BRSOnlineLog, Warning, TEXT("GetSteamFriendAvatar Had a bad UniqueNetId!"));
		return nullptr;
	}

	uint32 Width = 0;
	uint32 Height = 0;

	if (SteamAPI_Init())
	{
		//Getting the PictureID from the SteamAPI and getting the Size with the ID
		//virtual bool RequestUserInformation( CSteamID steamIDUser, bool bRequireNameOnly ) = 0;
		
		uint64 id = *((uint64*)UniqueNetId->GetBytes());
		int Picture = 0;

		switch (AvatarSize)
		{
		case ESteamAvatarSize::SteamAvatar_Small: Picture = SteamFriends()->GetSmallFriendAvatar(id); break;
		case ESteamAvatarSize::SteamAvatar_Medium: Picture = SteamFriends()->GetMediumFriendAvatar(id); break;
		case ESteamAvatarSize::SteamAvatar_Large: Picture = SteamFriends()->GetLargeFriendAvatar(id); break;
		default: break;
		}

		if (Picture == -1)
			return NULL;

		SteamUtils()->GetImageSize(Picture, &Width, &Height);

		// STOLEN FROM ANSWERHUB :p, then fixed because answerhub wasn't releasing the memory O.o

		if (Width > 0 && Height > 0)
		{
			//Creating the buffer "oAvatarRGBA" and then filling it with the RGBA Stream from the Steam Avatar
			uint8 *oAvatarRGBA = new uint8[Width * Height * 4];


			//Filling the buffer with the RGBA Stream from the Steam Avatar and creating a UTextur2D to parse the RGBA Steam in
			SteamUtils()->GetImageRGBA(Picture, (uint8*)oAvatarRGBA, 4 * Height * Width * sizeof(char));


			// Removed as I changed the image bit code to be RGB, I think the original author was unaware that there were different pixel formats
			/*
			//Swap R and B channels because for some reason the games whack
			for (uint32 i = 0; i < (Width * Height * 4); i += 4)
			{
			uint8 Temp = oAvatarRGBA[i + 0];
			oAvatarRGBA[i + 0] = oAvatarRGBA[i + 2];
			oAvatarRGBA[i + 2] = Temp;
			}*/

			UTexture2D* Avatar = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);

			// Switched to a Memcpy instead of byte by byte transer
			uint8* MipData = (uint8*)Avatar->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(MipData, (void*)oAvatarRGBA, Height * Width * 4);
			Avatar->PlatformData->Mips[0].BulkData.Unlock();

			// Original implementation was missing this!! Delete variable at RGBA steam pointer (or else we'll leek data)
			// the hell man......
			delete[] oAvatarRGBA;

			//Setting some Parameters for the Texture and finally returning it
			Avatar->PlatformData->NumSlices = 1;
			Avatar->NeverStream = true;
			//Avatar->CompressionSettings = TC_EditorIcon;

			Avatar->UpdateResource();

			return Avatar;
		}
		else
		{
			UE_LOG(BRSOnlineLog, Warning, TEXT("Bad Height / Width with steam avatar!"));
		}

		return nullptr;
	}
#endif

	UE_LOG(BRSOnlineLog, Warning, TEXT("STEAM Couldn't be verified as initialized"));
	return nullptr;
}

UTexture2D* UBRSOnlineLibrary::GetSteamLocalPlayerAvatar(ULocalPlayer* Player, ESteamAvatarSize AvatarSize)
{
	if (Player && Player->IsValidLowLevel())
	{
		TSharedPtr<const FUniqueNetId> UniqueNetId = Player->GetUniqueNetIdFromCachedControllerId()->AsShared();
		return GetSteamFriendAvatar(UniqueNetId, AvatarSize);
	}
	return nullptr;
}

bool UBRSOnlineLibrary::UsingSteam()
{
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
	if (SteamAPI_Init())
	{
		return true;
	}
#endif
	return false;
}



const TSubclassOf<ABRSGameMode_Base> UBRSOnlineLibrary::GetSessionGameMode(const FBlueprintSessionResult& Session)
{
	if (Session.OnlineResult.IsValid() && GEngine && GWorld)
	{
		FString GameModePath;
		Session.OnlineResult.Session.SessionSettings.Get(SETTING_GAMEMODE, GameModePath);

		TSubclassOf<ABRSGameMode_Base> LoadGameMode = LoadClass<ABRSGameMode_Base>(nullptr, *GameModePath);
		if (LoadGameMode)
		{
			return LoadGameMode;
		}
	}
	return nullptr;
}

void UBRSOnlineLibrary::GetSessionGameModeName(const FBlueprintSessionResult& Session, FString& GameModeName)
{
	if (Session.OnlineResult.IsValid())
	{
		Session.OnlineResult.Session.SessionSettings.Get(SETTING_GAMEMODE, GameModeName);
	}
}

void UBRSOnlineLibrary::GetSessionMapName(const FBlueprintSessionResult& Session, FString& MapName)
{
	if (Session.OnlineResult.IsValid())
	{
		Session.OnlineResult.Session.SessionSettings.Get(SETTING_MAPNAME, MapName);
	}
}

void UBRSOnlineLibrary::GetSessionDisplayName(const FBlueprintSessionResult& Session, FString& DisplayName)
{
	if (Session.OnlineResult.IsValid())
	{
		//Session.OnlineResult.Session.SessionSettings.Get(SETTING_CUSTOM, DisplayName);
		DisplayName = FString(TEXT("Some server"));
	}
}
