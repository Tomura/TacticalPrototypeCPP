// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSSGOptionUMG.h"
#include "BRSCVarCollectionUMG.h"
#include "BRSGameUserSettings.h"


UBRSCVarCollectionUMG::UBRSCVarCollectionUMG(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	SGWidget = nullptr;
}

void UBRSCVarCollectionUMG::UpdateValue_Implementation()
{

}

int32 UBRSCVarCollectionUMG::GetQuality()
{
	return 0;
}

void UBRSCVarCollectionUMG::SetQuality(int32 Quality)
{

}

void UBRSCVarCollectionUMG::SetCVarInt(FString Name, int32 Value, bool bSave)
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (CVar)
	{
		if (SGWidget)
		{
			SGWidget->UsingCustomSettings();
			FString SGName = SGWidget->GetScalabilityGroupName();
			//FString CustomScalabiltySection = FString::Printf(UBRSGameUserSettings::CustomScalabilitySection, *SGName);
			FString CustomScalabiltySection = FString::Printf(CUSTOM_SCALABILITY, *SGName);
			GConfig->SetInt(*CustomScalabiltySection, *Name, Value, GGameUserSettingsIni);
		}
		CVar->Set(Value, ECVF_SetByScalability);
	}
}

void UBRSCVarCollectionUMG::SetCVarBool(FString Name, bool Value, bool bSave)
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (CVar)
	{
		if (SGWidget)
		{
			SGWidget->UsingCustomSettings();
			FString SGName = SGWidget->GetScalabilityGroupName();
			FString CustomScalabiltySection = FString::Printf(CUSTOM_SCALABILITY, *SGName);
			GConfig->SetBool(*CustomScalabiltySection, *Name, Value, GGameUserSettingsIni);
		}
		CVar->Set(Value, ECVF_SetByScalability);
	}
}

void UBRSCVarCollectionUMG::SetCVarFloat(FString Name, float Value, bool bSave)
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (CVar)
	{
		if (SGWidget)
		{
			SGWidget->UsingCustomSettings();
			FString SGName = SGWidget->GetScalabilityGroupName();
			FString CustomScalabiltySection = FString::Printf(CUSTOM_SCALABILITY, *SGName);
			GConfig->SetFloat(*CustomScalabiltySection, *Name, Value, GGameUserSettingsIni);
		}
		CVar->Set(Value, ECVF_SetByScalability);
	}
}

int32 UBRSCVarCollectionUMG::GetCVarInt(FString Name) const
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (CVar)
	{
		return CVar->GetInt();
	}
	return -1;
}
bool UBRSCVarCollectionUMG::GetCVarBool(FString Name) const
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (CVar)
	{
		return (CVar->GetInt() > 0);
	}
	return false;
}
float UBRSCVarCollectionUMG::GetCVarFloat(FString Name) const
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (CVar)
	{
		return CVar->GetFloat();
	}
	return -1;
}