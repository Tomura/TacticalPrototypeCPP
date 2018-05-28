// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "BRSOnlineLibrary.h"
#include "BRSFriendsListItemUMG_Base.generated.h"

/**
 * 
 */
UCLASS()
class BRSENGINE_API UBRSFriendsListItemUMG_Base : public UUserWidget
{
	GENERATED_BODY()

	friend class UBRSFriendsUMG_Base;

public:
	UBRSFriendsListItemUMG_Base(const FObjectInitializer& OI);
	
	UFUNCTION(Category = Friends, BlueprintNativeEvent)
	void Unselect();
	void Unselect_Implementation(){}

	UFUNCTION(Category = Friends, BlueprintNativeEvent)
	void Select();
	void Select_Implementation(){}

	UFUNCTION(Category = Friends, BlueprintCallable)
	void SetSelected(bool bSelect);

	void OnAddedToList(class UBRSFriendsUMG_Base* List);

protected:
	IOnlineSubsystem* OnlineSub;
	IOnlineFriendsPtr OnlineFriendsPtr;

	UPROPERTY(Category = Friends, BlueprintReadOnly)
	FString UserName;
	UPROPERTY(Category = Friends, BlueprintReadOnly)
	FString Status;

	UPROPERTY(Category = Friends, BlueprintReadOnly)
	uint32 bIsOnline : 1;
	UPROPERTY(Category = Friends, BlueprintReadOnly)
	uint32 bIsPlayingThisGame : 1;
	UPROPERTY(Category = Friends, BlueprintReadOnly)
	uint32 bIsJoinable : 1;


	TSharedPtr<const FUniqueNetId> SessionId;
	TSharedPtr<const FUniqueNetId> UserId;

	class UBRSFriendsUMG_Base* FriendsList;

	UFUNCTION(Category = Friends, BlueprintCallable)
	void InviteFriend();


	UFUNCTION(Category = Friends, BlueprintCallable)
		UTexture2D* GetAvatar(ESteamAvatarSize AvatarSize);
};
