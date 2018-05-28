// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "BRSSettingTypes.generated.h"

UENUM(BlueprintType)
namespace EBRSSoundClass
{
	enum Type
	{
		Master,
		Music,
		SFX,
		Voice,
		UI,
		VoIP,

		//Should always be last (used for Size of array)
		MAX UMETA(Hidden)
	};
}

namespace FSoundClassHelpers
{
	static FString SoundClassToString(EBRSSoundClass::Type SoundClass)
	{
		switch (SoundClass)
		{
		case EBRSSoundClass::Master:
			return FString(TEXT("Master"));
		case EBRSSoundClass::Music:
			return FString(TEXT("Music"));
		case EBRSSoundClass::SFX:
			return FString(TEXT("SFX"));
		case EBRSSoundClass::Voice:
			return FString(TEXT("Voice"));
		case EBRSSoundClass::UI:
			return FString(TEXT("UI"));
		case EBRSSoundClass::VoIP:
			return FString(TEXT("VoIP"));
		default:
			break;
		}
		return FString("ERROR");
	}
}