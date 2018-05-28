// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Engine/DeveloperSettings.h"
#include "BRSAudioSettings.generated.h"

/**
 * 
 */
UCLASS(defaultconfig, config = Engine)
class BRSENGINE_API UBRSAudioSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	UBRSAudioSettings(const FObjectInitializer& OI);
	

	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowedClasses = "SoundClass"))
	FStringAssetReference MasterSoundClass;
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowedClasses = "SoundClass"))
	FStringAssetReference MusicSoundClass;
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowedClasses = "SoundClass"))
	FStringAssetReference EffectsSoundClass;
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowedClasses = "SoundClass"))
	FStringAssetReference VoiceSoundClass;
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Audio", meta = (AllowedClasses = "SoundClass"))
	FStringAssetReference UISoundClass;
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = "Audio", meta=(DisplayName = "VoIP Sound Class", AllowedClasses = "SoundClass"))
	FStringAssetReference VoiPSoundClass;
	
};
