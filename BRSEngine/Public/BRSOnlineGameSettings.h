// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "OnlineSessionSettings.h"
/**
* General session settings for a Shooter game
*/
class BRSENGINE_API FBRSOnlineSessionSettings : public FOnlineSessionSettings
{
public:

	FBRSOnlineSessionSettings(bool bIsLAN = false, bool bIsPresence = false, int32 MaxNumPlayers = 4);
	virtual ~FBRSOnlineSessionSettings() {}
};

/**
* General search setting for a Shooter game
*/
class BRSENGINE_API FBRSOnlineSearchSettings : public FOnlineSessionSearch
{
public:
	FBRSOnlineSearchSettings(bool bSearchingLAN = false, bool bSearchingPresence = false);

	virtual ~FBRSOnlineSearchSettings() {}
};

/**
* Search settings for an empty dedicated server to host a match
*/
class BRSENGINE_API FBRSOnlineSearchSettingsEmptyDedicated : public FBRSOnlineSearchSettings
{
public:
	FBRSOnlineSearchSettingsEmptyDedicated(bool bSearchingLAN = false, bool bSearchingPresence = false);

	virtual ~FBRSOnlineSearchSettingsEmptyDedicated() {}
};