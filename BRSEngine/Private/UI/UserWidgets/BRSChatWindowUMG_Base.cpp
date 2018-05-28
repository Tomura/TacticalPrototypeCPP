// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSChatWindowUMG_Base.h"

UBRSChatWindowUMG_Base::UBRSChatWindowUMG_Base(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MaxStoredMessages = 20;
	MessageLifeTimeSeconds = 30.f;
}




