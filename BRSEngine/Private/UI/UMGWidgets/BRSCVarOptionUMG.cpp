// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSCVarOptionUMG.h"
#include "BRSSGOptionUMG.h"
#include "BRSGameUserSettings.h"

UBRSCVarOptionUMG::UBRSCVarOptionUMG(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SGWidget = nullptr;
}


void UBRSCVarOptionUMG::SetCVarInt(FString Name, int32 Value, bool bSave)
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (CVar)
	{
		if (SGWidget)
		{
			SGWidget->UsingCustomSettings();
			FString SGName = SGWidget->GetScalabilityGroupName();
			FString CustomScalabiltySection = FString::Printf(UBRSGameUserSettings::CustomScalabilitySection, *SGName);
			GConfig->SetInt(*CustomScalabiltySection, *Name, Value, GGameUserSettingsIni);
		}
		//GetOwningLocalPlayer()->ViewportClient->ConsoleCommand(FString::Printf(TEXT("%s %d"), *Name, Value));
		//CVar->Set(Value, ECVF_SetBySystemSettingsIni);
		CVar->Set(Value, ECVF_SetByScalability);
/*		if (bSave)
		{
			GConfig->SetInt(SECTIONSystemSettings, *Name, Value, GEngineIni);
			GEngine->SaveConfig();
		}	*/	

	}
}
void UBRSCVarOptionUMG::SetCVarBool(FString Name, bool Value, bool bSave)
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (CVar)
	{
		if (SGWidget)
		{
			SGWidget->UsingCustomSettings();
			FString SGName = SGWidget->GetScalabilityGroupName();
			FString CustomScalabiltySection = FString::Printf(UBRSGameUserSettings::CustomScalabilitySection, *SGName);
			GConfig->SetBool(*CustomScalabiltySection, *Name, Value, GGameUserSettingsIni);
		}
		//GetOwningLocalPlayer()->ViewportClient->ConsoleCommand(FString::Printf(TEXT("%s %d"), *Name, Value));
		CVar->Set(Value, ECVF_SetByScalability);
		//if (bSave)
		//{
		//	GConfig->SetInt(SECTIONSystemSettings, *Name, Value ? 0 : 1, GEngineIni);
		//	GEngine->SaveConfig();
		//}

	}
}
void UBRSCVarOptionUMG::SetCVarFloat(FString Name, float Value, bool bSave)
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (CVar)
	{
		if (SGWidget)
		{
			SGWidget->UsingCustomSettings();
			FString SGName = SGWidget->GetScalabilityGroupName();
			FString CustomScalabiltySection = FString::Printf(UBRSGameUserSettings::CustomScalabilitySection, *SGName);
			GConfig->SetFloat(*CustomScalabiltySection, *Name, Value, GGameUserSettingsIni);
		}
		//GetOwningLocalPlayer()->ViewportClient->ConsoleCommand(FString::Printf(TEXT("%s %f"), *Name, Value));
		CVar->Set(Value, ECVF_SetByScalability);
		//if (bSave)
		//{
		//	GConfig->SetFloat(SECTIONSystemSettings, *Name, Value, GEngineIni);
		//	GEngine->SaveConfig();
		//}

	}
}

int32 UBRSCVarOptionUMG::GetCVarInt(FString Name) const
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (CVar)
	{
		return CVar->GetInt();
	}
	return -1;
}
bool UBRSCVarOptionUMG::GetCVarBool(FString Name) const
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (CVar)
	{
		return (CVar->GetInt() > 0);
	}
	return false;
}
float UBRSCVarOptionUMG::GetCVarFloat(FString Name) const
{
	IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*Name);
	if (CVar)
	{
		return CVar->GetFloat();
	}
	return -1;
}

void UBRSCVarOptionUMG::UpdateValue_Implementation()
{
	
}