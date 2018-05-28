// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSFriendsListItemUMG_Base.h"
#include "BRSFriendsUMG_Base.h"





UBRSFriendsListItemUMG_Base::UBRSFriendsListItemUMG_Base(const FObjectInitializer& OI)
	: Super(OI)
{
	OnlineSub = IOnlineSubsystem::Get();
	OnlineFriendsPtr = OnlineSub->GetFriendsInterface();
	if (OnlineFriendsPtr.IsValid())
	{
		OnlineFriendsPtr->ReadFriendsList(0, EFriendsLists::ToString(EFriendsLists::Default));
	}
}

void UBRSFriendsListItemUMG_Base::SetSelected(bool bSelect)
{
	if (bSelect)
	{
		FriendsList->OnFriendSelected(this);
	}
}

void UBRSFriendsListItemUMG_Base::OnAddedToList(class UBRSFriendsUMG_Base* List)
{
	FriendsList = List;


}

void UBRSFriendsListItemUMG_Base::InviteFriend()
{
	if (OnlineFriendsPtr.IsValid() && UserId.IsValid())
	{
		OnlineFriendsPtr->SendInvite(GetOwningLocalPlayer()->GetControllerId(), *UserId.Get(), EFriendsLists::ToString(EFriendsLists::Default));
	}
}

UTexture2D* UBRSFriendsListItemUMG_Base::GetAvatar(ESteamAvatarSize AvatarSize)
{
	if (UserId.IsValid() && UserId->IsValid())
	{
		return UBRSOnlineLibrary::GetSteamFriendAvatar(UserId, AvatarSize);
	}
	return nullptr;
}
