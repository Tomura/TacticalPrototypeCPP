// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSGameUserSettings.h"
#include "Sound/AudioSettings.h"
#include "BRSAudioSettings.h"

const TCHAR* UBRSGameUserSettings::CustomScalabilitySection = TEXT("%s@Custom");


UBRSGameUserSettings::UBRSGameUserSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	for (int32 i = 0; i < EBRSSoundClass::MAX; i++)
	{
		AudioLevel[i] = 1.f;
	}
	MotionBlurScale = 1.f;
	DepthOfFieldScale = 1.f;
	bLensFlares = true;
	bEyeAdaptation = true;

	bInitSoundClasses = false;
}

void UBRSGameUserSettings::ApplySettings(bool bCheckForCommandLineOverrides)
{
	Super::ApplySettings(bCheckForCommandLineOverrides);

	if (!GEngine)
	{
		return;
	}

	if (!GEngine->IsInitialized())
	{
		return;
	}

	// Quality Settings
	if (ScalabilityQuality.AntiAliasingQuality < 0)	ApplyCVarSettingsFromIni(TEXT("AntiAliasingQuality@Custom"), *GGameUserSettingsIni, ECVF_SetByScalability);
	if (ScalabilityQuality.EffectsQuality < 0) ApplyCVarSettingsFromIni(TEXT("EffectsQuality@Custom"), *GGameUserSettingsIni, ECVF_SetByScalability);
	if (ScalabilityQuality.PostProcessQuality < 0) ApplyCVarSettingsFromIni(TEXT("PostProcessQuality@Custom"), *GGameUserSettingsIni, ECVF_SetByScalability);
	if (ScalabilityQuality.ShadowQuality < 0) ApplyCVarSettingsFromIni(TEXT("ShadowQuality@Custom"), *GGameUserSettingsIni, ECVF_SetByScalability);
	if (ScalabilityQuality.TextureQuality < 0) ApplyCVarSettingsFromIni(TEXT("TextureQuality@Custom"), *GGameUserSettingsIni, ECVF_SetByScalability);
	if (ScalabilityQuality.ViewDistanceQuality < 0) ApplyCVarSettingsFromIni(TEXT("ViewDistanceQuality@Custom"), *GGameUserSettingsIni, ECVF_SetByScalability);


	IConsoleManager& ConsoleManager = IConsoleManager::Get();
	if (IConsoleVariable* CVar = ConsoleManager.FindConsoleVariable(TEXT("r.MotionBlur.Scale")))
	{
		CVar->Set(MotionBlurScale, ECVF_SetByScalability);
	}
	if (IConsoleVariable* CVar = ConsoleManager.FindConsoleVariable(TEXT("r.DepthOfField.MaxSize")))
	{
		CVar->Set(DepthOfFieldScale, ECVF_SetByScalability);
	}
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetEngineShowFlags()->SetLensFlares(bLensFlares);
		GEngine->GameViewport->GetEngineShowFlags()->SetEyeAdaptation(bEyeAdaptation);
	}

	// Audio
	if (!bInitSoundClasses)
	{
		InitSoundClasses();
	}


	FAudioDevice* AudioDevice = GEngine->GetMainAudioDevice();
	if (AudioDevice != nullptr)
	{
		AudioDevice->SetDefaultBaseSoundMix(MainSoundMix);
		AudioDevice->SetBaseSoundMix(MainSoundMix);

		if (!MainSoundMix)
		{
			const FStringAssetReference& MixRef = GetDefault<UAudioSettings>()->DefaultBaseSoundMix;
			MainSoundMix = FindObject<USoundMix>(nullptr, *MixRef.ToString());
		}
		for (int32 i = 0; i < ARRAY_COUNT(AudioLevel); i++)
		{
			SetSoundClassVolume(EBRSSoundClass::Type(i), AudioLevel[i], false);
		}
	}
}


void UBRSGameUserSettings::SetSoundClassVolumeByName(const FString& InSoundClass, float NewVolume)
{
	// no effect in editor because we're altering a savable object
	// also just unintuitive when editing sounds
	if (!GIsEditor)
	{
		FAudioDevice* AudioDevice = GEngine->GetMainAudioDevice();
		if (AudioDevice != NULL)
		{
			//for (TMap<USoundClass*, FSoundClassProperties>::TIterator It(AudioDevice->SoundClasses); It; ++It)
			//{
			//	USoundClass* ThisSoundClass = It.Key();
			//	AudioDevice->SetSoundMixClassOverride(MainSoundMix, ThisSoundClass, NewVolume, 1.f, 0.f, true);

			//	
			//	if (ThisSoundClass != NULL && ThisSoundClass->GetFullName().Find(InSoundClass) != INDEX_NONE)
			//	{
			//		// the audio device function logspams for some reason
			//		//AudioDevice->SetClassVolume(ThisSoundClass, NewVolume); <- this logspams, but would be the better solution if possible
			//		UE_LOG(LogBRSGameSettings, Verbose, TEXT("Setting new Volume for %s to %f"), *InSoundClass, NewVolume);
			//		ThisSoundClass->Properties.Volume = NewVolume;
			//	}
			//}
			

		}
		else
		{
			UE_LOG(LogBRSGameSettings, Warning, TEXT("Couldn't find AudioDevice"));
		}
	}
	else
	{
		UE_LOG(LogBRSGameSettings, Warning, TEXT("Can't modify sound class volume in editor. (Otherwise it would be unintuitive for sound editing.)"));
	}
}

void UBRSGameUserSettings::SetSoundClassVolume(const EBRSSoundClass::Type& InSoundClass, float NewVolume, bool bSave)
{	
	FAudioDevice* AudioDevice = GEngine->GetMainAudioDevice();
	if (AudioDevice != NULL)
	{
		if (BasicSoundClass[InSoundClass])
		{
			// UE_LOG(LogTemp, Log, TEXT("Settings Sound Class Override of %s for class %s to %f"),
			//	*MainSoundMix->GetFullName(), *BasicSoundClass[InSoundClass]->GetFullName(), NewVolume);
			AudioDevice->SetSoundMixClassOverride(MainSoundMix, BasicSoundClass[InSoundClass], NewVolume, 1.f, 0.1f, true);
			//UGameplayStatics::SetSoundMixClassOverride(GWorld, MainSoundMix, BasicSoundClass[InSoundClass], 0.f, 1.f, 0.1f, true);
		}
	}
	AudioLevel[InSoundClass] = NewVolume;
	if (bSave)
	{
		SaveConfig();
	}
}


float UBRSGameUserSettings::GetSoundClassVolume(const EBRSSoundClass::Type& InSoundClass) const
{
	return AudioLevel[InSoundClass];
}

void UBRSGameUserSettings::InitSoundClasses()
{
	const UBRSAudioSettings* BRSAudioSettingsCDO = GetDefault<UBRSAudioSettings>();
	const UAudioSettings* AudioSettingsCDO = GetDefault<UAudioSettings>();
	//AudioSettingsCDO->DefaultSoundClassName
	//BasicSoundClass[EBRSSoundClass::Master] = LoadObject<USoundClass>(nullptr, *AudioSettingsCDO->DefaultSoundClassName.ToString());
	//BasicSoundClass[EBRSSoundClass::Master] = FindObject<USoundClass>(nullptr, *BRSAudioSettingsCDO->MasterSoundClass.ToString());
	BasicSoundClass[EBRSSoundClass::Master] = LoadObject<USoundClass>(nullptr, *BRSAudioSettingsCDO->MasterSoundClass.ToString());
	BasicSoundClass[EBRSSoundClass::Music] = LoadObject<USoundClass>(nullptr, *BRSAudioSettingsCDO->MusicSoundClass.ToString());
	BasicSoundClass[EBRSSoundClass::SFX] = LoadObject<USoundClass>(nullptr, *BRSAudioSettingsCDO->EffectsSoundClass.ToString());
	BasicSoundClass[EBRSSoundClass::Voice] = LoadObject<USoundClass>(nullptr, *BRSAudioSettingsCDO->VoiceSoundClass.ToString());
	BasicSoundClass[EBRSSoundClass::UI] = LoadObject<USoundClass>(nullptr, *BRSAudioSettingsCDO->UISoundClass.ToString());
	//BasicSoundClass[EBRSSoundClass::VoIP] = LoadObject<USoundClass>(nullptr, *BRSAudioSettingsCDO->VoiPSoundClass.ToString());
	BasicSoundClass[EBRSSoundClass::VoIP] = LoadObject<USoundClass>(nullptr, *AudioSettingsCDO->VoiPSoundClass.ToString());

	bInitSoundClasses = true;
}
