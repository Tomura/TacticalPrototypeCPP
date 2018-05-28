// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSGameUserSettings.h"
#include "UMGAudioOptions.h"


UUMGAudioOptions::UUMGAudioOptions(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UUMGAudioOptions::SetSoundClassVolume(EBRSSoundClass::Type InSoundClass, float NewVolume, bool bSave)
{
	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());
	if (UserSettings)
	{
		UserSettings->SetSoundClassVolume(InSoundClass, NewVolume, bSave);
	}
}

float UUMGAudioOptions::GetSoundClassVolume(EBRSSoundClass::Type InSoundClass) const
{
	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());
	if (UserSettings)
	{
		return UserSettings->GetSoundClassVolume(InSoundClass);
	}
	return -1.f;
}