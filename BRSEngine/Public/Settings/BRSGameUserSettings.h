// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "GameFramework/GameUserSettings.h"
#include "BRSSettingTypes.h"
#include "BRSGameUserSettings.generated.h"

#define CUSTOM_SCALABILITY TEXT("%s@Custom")


/**
 * 
 */
UCLASS()
class BRSENGINE_API UBRSGameUserSettings : public UGameUserSettings
{
	GENERATED_BODY()

public:
	static const TCHAR* CustomScalabilitySection;

	UBRSGameUserSettings(const FObjectInitializer& ObjectInitializer);
	

	void ApplySettings(bool bCheckForCommandLineOverrides) override;



public:
	void SetSoundClassVolumeByName(const FString& InSoundClass, float NewVolume);
	void SetSoundClassVolume(const EBRSSoundClass::Type& InSoundClass, float NewVolume, bool bSave);
	float GetSoundClassVolume(const EBRSSoundClass::Type& InSoundClass) const;
	
	float GetMotionBlurScale() const { return MotionBlurScale; }
	void SetMotionBlurScale(float InScale, bool bSave = true) { MotionBlurScale = InScale; if (bSave) ApplySettings(true); }

	float GetDepthOfFieldScale() const { return DepthOfFieldScale; }
	void SetDepthOfFieldScale(float InScale, bool bSave = true) { DepthOfFieldScale = InScale; if (bSave) ApplySettings(true); }
	
	bool GetLensFlares() const{ return bEyeAdaptation; }
	void SetLensFlares(bool bEnabled) { bLensFlares = bEnabled; }

	bool GetEyeAdaptation() const{ return bEyeAdaptation; }
	void SetEyeAdaptation(bool bEnabled) { bEyeAdaptation = bEnabled; }


	void InitSoundClasses();

private:

	bool bInitSoundClasses;

	UPROPERTY(config)
	float AudioLevel[EBRSSoundClass::MAX];

	USoundClass* BasicSoundClass[EBRSSoundClass::MAX];

	UPROPERTY(config)
		float MotionBlurScale;
	UPROPERTY(config)
		float DepthOfFieldScale;
	UPROPERTY(config)
		uint32 bLensFlares : 1;
	UPROPERTY(config)
		uint32 bEyeAdaptation : 1;

	UPROPERTY()
		USoundMix* MainSoundMix;
};
