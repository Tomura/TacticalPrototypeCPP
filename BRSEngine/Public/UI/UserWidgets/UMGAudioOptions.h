// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "BRSCustomToolTipWidget.h"
#include "BRSSettingTypes.h"
#include "UMGAudioOptions.generated.h"


/**
 * 
 */
UCLASS(abstract)
class BRSENGINE_API UUMGAudioOptions : public UBRSCustomToolTipWidget
{
	GENERATED_BODY()

public:
	UUMGAudioOptions(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Audio Settings")
	void SetSoundClassVolume(EBRSSoundClass::Type InSoundClass, float NewVolume, bool bSave);

	UFUNCTION(BlueprintCallable, Category = "AudioSettings")
		float GetSoundClassVolume(EBRSSoundClass::Type InSoundClass) const;
};
