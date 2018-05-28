// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#ifndef __BRSLOADINGSCREEN_H__
#define __BRSLOADINGSCREEN_H__

#include "ModuleInterface.h"


/** Module interface for this game's loading screens */
class IBRSLoadingScreenModule : public IModuleInterface
{
public:
	/** Kicks off the loading screen for in game loading (not startup) */
	virtual void StartInGameLoadingScreen() = 0;
};

#endif // __BRSLOADINGSCREEN_H__
