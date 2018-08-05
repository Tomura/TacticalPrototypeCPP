// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSCVarOptionUMG.h"
#include "BRSSGOptionUMG.h"
#include "BRSGameUserSettings.h"
#include "BRSToolTipDisplay.h"


UBRSSGOptionUMG::UBRSSGOptionUMG(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


void UBRSSGOptionUMG::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (CustomToolTipWidgetDelegate.IsBound() && !IsDesignTime())
	{
		UWidget* DelegateResult = CustomToolTipWidgetDelegate.Execute();
		CustomToolTipWidget = Cast<UBRSToolTipDisplay>(DelegateResult);
	}
	else
	{
		CustomToolTipWidget = nullptr;
	}
}

void UBRSSGOptionUMG::UpdateValue_Implementation()
{
	if (CVarWidgets.Num() > 0)
	{
		for (UBRSCVarOptionUMG* CVarUMG : CVarWidgets)
		{
			CVarUMG->UpdateValue();
			//UE_LOG(LogTemp, Log, TEXT("%s"), *CVarUMG->GetName());
		}
	}
}

void UBRSSGOptionUMG::SetQualityLevel(int32 inQuality)
{
	// Set Level in Scalability Settings
	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());

	switch (ScalabilityGroup)
	{
	case EBRSScalabilityGroup::SG_AntiAliasingQuality:
		UserSettings->ScalabilityQuality.AntiAliasingQuality = inQuality;
		break;
	case EBRSScalabilityGroup::SG_EffectsQuality:
		UserSettings->ScalabilityQuality.EffectsQuality = inQuality;
		break;
	case EBRSScalabilityGroup::SG_PostProcessQuality:
		UserSettings->ScalabilityQuality.PostProcessQuality = inQuality;
		break;
	case EBRSScalabilityGroup::SG_ShadowQuality:
		UserSettings->ScalabilityQuality.ShadowQuality = inQuality;
		break;
	case EBRSScalabilityGroup::SG_TextureQuality:
		UserSettings->ScalabilityQuality.TextureQuality = inQuality;
		break;
	case EBRSScalabilityGroup::SG_ViewDistanceQuality:
		UserSettings->ScalabilityQuality.ViewDistanceQuality = inQuality;
		break;
	case EBRSScalabilityGroup::SG_ResolutionQuality:
		UserSettings->ScalabilityQuality.ResolutionQuality = inQuality;
		break;
	default:
		break;
	}
	UserSettings->ApplySettings(true);
	UserSettings->SaveConfig();

	UpdateValue();
}

void UBRSSGOptionUMG::UsingCustomSettings()
{
	if (GetQualityLevel() < 0)
		return; // We are already using custom settings

	// Copy Scalability Settings
	FString SGName = GetScalabilityGroupName();
	FString ScalabiltySection = FString::Printf(TEXT("%s@%d"), *SGName, GetQualityLevel());
	//UE_LOG(LogTemp, Log, TEXT("Copying from %s"), *ScalabiltySection);
	FString CustomScalabiltySection = FString::Printf(CUSTOM_SCALABILITY, *SGName);

	TArray<FString> SectionStrings;
	GConfig->GetSection(*ScalabiltySection, SectionStrings, GScalabilityIni);
	// Copy Current Settings over
	for (FString SectionString : SectionStrings)
	{
		FString CVarName;
		FString CVarValue;
		SectionString.Split(TEXT("="), &CVarName, &CVarValue);
		//UE_LOG(LogTemp, Log, TEXT("Section: %s"), *CVarName);
		
		GConfig->SetString(*CustomScalabiltySection, *CVarName, *CVarValue, GGameUserSettingsIni);
	}

	SetQualityLevel(-1);
	UpdateValue();
}

int32 UBRSSGOptionUMG::GetQualityLevel() const
{
	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());
	switch (ScalabilityGroup)
	{
	case EBRSScalabilityGroup::SG_AntiAliasingQuality:
		return UserSettings->ScalabilityQuality.AntiAliasingQuality;
	case EBRSScalabilityGroup::SG_EffectsQuality:
		return UserSettings->ScalabilityQuality.EffectsQuality;
	case EBRSScalabilityGroup::SG_PostProcessQuality:
		return UserSettings->ScalabilityQuality.PostProcessQuality;
	case EBRSScalabilityGroup::SG_ShadowQuality:
		return UserSettings->ScalabilityQuality.ShadowQuality;
	case EBRSScalabilityGroup::SG_TextureQuality:
		return UserSettings->ScalabilityQuality.TextureQuality;
	case EBRSScalabilityGroup::SG_ViewDistanceQuality:
		return UserSettings->ScalabilityQuality.ViewDistanceQuality;
	case EBRSScalabilityGroup::SG_ResolutionQuality:
		return UserSettings->ScalabilityQuality.ResolutionQuality;
	default:
		break;
	}
	return -1;
}

void UBRSSGOptionUMG::RemoveScalabilityKeysFromConfig(const FString& SGName, bool bSave)
{
	FString ScalabiltySection = FString::Printf(TEXT("%s@0"), *SGName);

	TArray<FString> SectionStrings;
	GConfig->GetSection(*ScalabiltySection, SectionStrings, GScalabilityIni);
	for (FString SectionString : SectionStrings)
	{
		FString CVarName;
		FString CVarValue;
		SectionString.Split(TEXT("="), &CVarName, &CVarValue);
		//UE_LOG(LogTemp, Log, TEXT("Section: %s"), *CVarName);
		GConfig->RemoveKey(TEXT("SystemSettings"), *CVarName, GEngineIni);
	}

	if (bSave) GEngine->SaveConfig();
}

bool UBRSSGOptionUMG::IsUsingCustomSettings()
{

	return false;
}

FString UBRSSGOptionUMG::GetScalabilityGroupName() const
{
	switch (ScalabilityGroup)
	{
	case EBRSScalabilityGroup::SG_AntiAliasingQuality:
		return TEXT("AntiAliasingQuality");
	case EBRSScalabilityGroup::SG_EffectsQuality:
		return TEXT("EffectsQuality");
	case EBRSScalabilityGroup::SG_PostProcessQuality:
		return TEXT("PostProcessQuality");
	case EBRSScalabilityGroup::SG_ShadowQuality:
		return TEXT("ShadowQuality");
	case EBRSScalabilityGroup::SG_TextureQuality:
		return TEXT("TextureQuality");
	case EBRSScalabilityGroup::SG_ViewDistanceQuality:
		return TEXT("ViewDistanceQuality");
	case EBRSScalabilityGroup::SG_ResolutionQuality:
		return TEXT("ResolutionQuality");
	default:
		break;
	}
	return TEXT("");
}