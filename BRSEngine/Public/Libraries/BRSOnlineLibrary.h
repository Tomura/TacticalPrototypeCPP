// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "BRSOnlineLibrary.generated.h"

//General Advanced Sessions Log
DECLARE_LOG_CATEGORY_EXTERN(BRSOnlineLog, Log, All);


UENUM(Blueprintable)
enum ESteamAvatarSize
{
	SteamAvatar_Small = 1,
	SteamAvatar_Medium = 2,
	SteamAvatar_Large = 3
};


/**
 * 
 */
UCLASS()
class BRSENGINE_API UBRSOnlineLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UBRSOnlineLibrary(const FObjectInitializer& OI) : Super(OI) {}


	static UTexture2D * GetSteamFriendAvatar(const TSharedPtr<const FUniqueNetId> UniqueNetId, ESteamAvatarSize AvatarSize);

	UFUNCTION(BlueprintCallable, Category = "BRS Online Library|Steam")
	static UTexture2D* GetSteamLocalPlayerAvatar(ULocalPlayer* Player, ESteamAvatarSize AvatarSize);


	UFUNCTION(BlueprintPure, Category = "BRS Online Library|Steam")
		static bool UsingSteam();

	UFUNCTION(BlueprintPure, Category = "BRS BP Library|Sessions")
		static const TSubclassOf<class ABRSGameMode_Base> GetSessionGameMode(const FBlueprintSessionResult& Session);

	UFUNCTION(BlueprintPure, Category = "BRS BP Library|Sessions")
		static void GetSessionGameModeName(const FBlueprintSessionResult& Session, FString& GameModeName);

	UFUNCTION(BlueprintPure, Category = "BRS BP Library|Sessions")
		static void GetSessionMapName(const FBlueprintSessionResult& Session, FString& MapName);


	UFUNCTION(BlueprintPure, Category = "BRS BP Library|Sessions")
		static void GetSessionDisplayName(const FBlueprintSessionResult& Session, FString& DisplayName);

};
