// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "BRSFriendsUMG_Base.generated.h"



/**
 * 
 */
UCLASS()
class BRSENGINE_API UBRSFriendsUMG_Base : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UBRSFriendsUMG_Base(const FObjectInitializer& OI);

	void OnFriendSelected(class UBRSFriendsListItemUMG_Base* SelectedFriend);

protected:	
	TArray< TSharedRef<FOnlineFriend> > Friends;

	IOnlineSubsystem* OnlineSub;
	IOnlineFriendsPtr OnlineFriendsPtr;
	
	UPROPERTY()
	TArray<class UBRSFriendsListItemUMG_Base*> FriendListItems;

	UFUNCTION(Category = Friends, BlueprintNativeEvent)
	class UBRSFriendsListItemUMG_Base* GenerateFriendItemWidget();
	class UBRSFriendsListItemUMG_Base* GenerateFriendItemWidget_Implementation(){ return nullptr; }

	UFUNCTION(Category = Friends, BlueprintCallable)
	void UpdateFriends();


	UFUNCTION(Category = Friends, BlueprintNativeEvent)
	void AddFriendWidget(class UBRSFriendsListItemUMG_Base* Friend);
	void AddFriendWidget_Implementation(class UBRSFriendsListItemUMG_Base* Friend){}


};
