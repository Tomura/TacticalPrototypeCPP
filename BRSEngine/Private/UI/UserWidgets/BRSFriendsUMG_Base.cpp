// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSFriendsListItemUMG_Base.h"
#include "BRSFriendsUMG_Base.h"




UBRSFriendsUMG_Base::UBRSFriendsUMG_Base(const FObjectInitializer& OI)
	: Super(OI)
{
	OnlineSub = IOnlineSubsystem::Get();
	OnlineFriendsPtr = OnlineSub->GetFriendsInterface();
	if (OnlineFriendsPtr.IsValid())
	{
		OnlineFriendsPtr->ReadFriendsList(0, EFriendsLists::ToString(EFriendsLists::Default));
	}
}

void UBRSFriendsUMG_Base::OnFriendSelected(UBRSFriendsListItemUMG_Base* SelectedFriend)
{
	for (UBRSFriendsListItemUMG_Base* Friend : FriendListItems)
	{
		if (Friend != SelectedFriend)
		{
			Friend->Unselect();
		}
		else
		{
			Friend->Select();
		}
	}
}

void UBRSFriendsUMG_Base::UpdateFriends()
{
	if (OnlineFriendsPtr.IsValid())
	{
		OnlineFriendsPtr->ReadFriendsList(GetOwningLocalPlayer()->GetControllerId(), EFriendsLists::ToString(EFriendsLists::Default));
		
		Friends.Empty();
		if (OnlineFriendsPtr->GetFriendsList(GetOwningLocalPlayer()->GetControllerId(), EFriendsLists::ToString(EFriendsLists::Default), Friends))
		{
			if (Friends.Num() < FriendListItems.Num())
			{
				// too many items in list. Remove excess
				for (int32 i = (Friends.Num()); i < FriendListItems.Num(); i++)
				{
					if (FriendListItems.IsValidIndex(i) && FriendListItems[i]->IsPendingKill())
					{
						FriendListItems[i]->RemoveFromParent();
						// mark for GC
						FriendListItems[i]->MarkPendingKill();
						FriendListItems[i] = nullptr;
					}
				}
				// Resize the array
				FriendListItems.SetNum(Friends.Num());
			}

			for (int32 i = 0; i < Friends.Num(); ++i)
			{
				const FString FriendUserName = Friends[i]->GetDisplayName();
				const FOnlineUserPresence& FriendPresense = Friends[i]->GetPresence();
				// Insert Data into List Item
				if (!FriendListItems.IsValidIndex(i))
				{
					FriendListItems.Add(GenerateFriendItemWidget());
				}
				else if (FriendListItems[i] == nullptr)
				{
					FriendListItems[i] = GenerateFriendItemWidget();
				}
				FriendListItems[i]->UserName = FriendUserName;
				FriendListItems[i]->Status = FriendPresense.Status.StatusStr;

				FriendListItems[i]->bIsPlayingThisGame = FriendPresense.bIsPlayingThisGame;
				FriendListItems[i]->bIsOnline = FriendPresense.bIsOnline;
				FriendListItems[i]->bIsJoinable = FriendPresense.bIsJoinable;

				FriendListItems[i]->SessionId = FriendPresense.SessionId;
				FriendListItems[i]->UserId = Friends[i]->GetUserId();

				FriendListItems[i]->RemoveFromParent();
				AddFriendWidget(FriendListItems[i]);
			}
		}
	}
}

