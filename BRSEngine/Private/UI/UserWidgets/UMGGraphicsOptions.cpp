// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSGameInstance.h"
#include "Scalability.h"
#include "BRSGameUserSettings.h"
#include "UMGGraphicsOptions.h"
#include "BRSMessageWindowUMG.h"
#include "BRSSGOptionUMG.h"


UUMGGraphicsOptions::UUMGGraphicsOptions(const FObjectInitializer& OI)
	: Super(OI)
{}


void UUMGGraphicsOptions::GetDisplayAdapterResolutions(TArray<FString> & AvailableResolutions, FString& CurrentRes) const
{
	FScreenResolutionArray Resolutions;
	if (RHIGetAvailableResolutions(Resolutions, false))
	{
		for (FScreenResolutionRHI EachResolution : Resolutions)
		{
			FString NewResolution = FString::Printf(TEXT("%d x %d"), EachResolution.Width, EachResolution.Height);
			if (!AvailableResolutions.Contains(NewResolution))
				AvailableResolutions.Add(NewResolution);
		}
	}

	CurrentRes = GetCurrentResolution();
}

void UUMGGraphicsOptions::ChangeResolution(FString Resolution)
{
	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());

	FString ResText;
	ResText = Resolution.Replace(TEXT(" "), TEXT(""));


	const TCHAR* Cmd = *ResText;
	int32 X = FCString::Atoi(Cmd);
	const TCHAR* CmdTemp = FCString::Strchr(Cmd, 'x') ? FCString::Strchr(Cmd, 'x') + 1 : FCString::Strchr(Cmd, 'X') ? FCString::Strchr(Cmd, 'X') + 1 : TEXT("");
	int32 Y = FCString::Atoi(CmdTemp);

	UserSettings->SetScreenResolution(FIntPoint(X, Y));
	UserSettings->ApplyResolutionSettings(false);
	UserSettings->SaveConfig();
}

void UUMGGraphicsOptions::SetFullscreenMode(int32 WindowMode)
{
	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());

	EWindowMode::Type NewMode = EWindowMode::ConvertIntToWindowMode(WindowMode);

	UserSettings->SetFullscreenMode(NewMode);
	UserSettings->ApplyResolutionSettings(false);
	UserSettings->SaveConfig();
}

FString UUMGGraphicsOptions::GetCurrentResolution() const
{
	//FVector2D ViewportSize;
	//GetOwningLocalPlayer()->ViewportClient->GetViewportSize(ViewportSize);
	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());
	FIntPoint Resolution = UserSettings->GetScreenResolution();
	return FString::Printf(TEXT("%d x %d"), Resolution.X, Resolution.Y);
}

bool UUMGGraphicsOptions::IsFullScreen() const
{
	return GetOwningLocalPlayer()->ViewportClient->IsFullScreenViewport();
}

int32 UUMGGraphicsOptions::GetFullscreenMode() const
{
	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());
	return (int32)UserSettings->GetFullscreenMode();
}

void UUMGGraphicsOptions::SetResolutionQuality(int32 inResolutionQuality)
{
	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());
	UserSettings->ScalabilityQuality.ResolutionQuality = inResolutionQuality;
	Scalability::SetQualityLevels(UserSettings->ScalabilityQuality);
	Scalability::SaveState(GGameUserSettingsIni);

	UserSettings->ApplySettings(true);
	UserSettings->SaveConfig();
	//ApplyCVarSettingsGroupFromIni(TEXT("ResolutionQuality"), inResolutionQuality, *GScalabilityIni, ECVF_SetByGameSetting);
}

int32 UUMGGraphicsOptions::GetResolutionQuality()
{
	return Scalability::GetQualityLevels().ResolutionQuality;
}


void UUMGGraphicsOptions::GetScalabilitySettings(
	int32& TextureQuality,
	int32& ViewDistanceQuality,
	int32& ShadowQuality,
	int32& PostProcessQuality,
	int32& EffectsQuality,
	int32& AntiAliasingQuality) const
{
	const Scalability::FQualityLevels QualityLevels = Scalability::GetQualityLevels();

	TextureQuality = QualityLevels.TextureQuality;
	ViewDistanceQuality = QualityLevels.ViewDistanceQuality;
	ShadowQuality = QualityLevels.ShadowQuality;
	PostProcessQuality = QualityLevels.PostProcessQuality;
	EffectsQuality = QualityLevels.EffectsQuality;
	AntiAliasingQuality = QualityLevels.AntiAliasingQuality;
}




bool UUMGGraphicsOptions::GetUseVSync() const
{
	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());
	return UserSettings->IsVSyncEnabled();
}

void UUMGGraphicsOptions::SetUseVSync(bool bUseVSync)
{
	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());
	//GetOwningLocalPlayer()->ViewportClient->ConsoleCommand(FString::Printf(TEXT("r.VSync %d"), bUseVSync));
	UserSettings->SetVSyncEnabled(bUseVSync);
	UserSettings->SaveConfig();
}


void UUMGGraphicsOptions::SetMotionBlurScale(float Scale)
{
	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());
	UserSettings->SetMotionBlurScale(Scale);
	UserSettings->SaveConfig();
}

float UUMGGraphicsOptions::GetMotionBlurScale() const
{
	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());
	return UserSettings->GetMotionBlurScale();
}

void UUMGGraphicsOptions::SetDepthOfFieldScale(float Scale)
{
	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());
	UserSettings->SetDepthOfFieldScale(Scale);
	UserSettings->SaveConfig();
}

float UUMGGraphicsOptions::GetDepthOfFieldScale() const
{
	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());
	return UserSettings->GetDepthOfFieldScale();
}

void UUMGGraphicsOptions::SetLensFlares(bool bEnabled)
{
	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());
	UserSettings->SetLensFlares(bEnabled);
	UserSettings->SaveConfig();
}

bool UUMGGraphicsOptions::GetLensFlares() const
{
	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());
	return UserSettings->GetLensFlares();
}

void UUMGGraphicsOptions::SetEyeAdaptation(bool bEnabled)
{
	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());
	UserSettings->SetEyeAdaptation(bEnabled);
	UserSettings->SaveConfig();
}

bool UUMGGraphicsOptions::GetEyeAdaptation() const
{
	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());
	return UserSettings->GetEyeAdaptation();
}


FString UUMGGraphicsOptions::GetGraphicsAdapterName()
{
	return GRHIAdapterName;
}


void UUMGGraphicsOptions::GetGPUMemoryInfo(int32& DedicatedVideoMemory, int32& DedicatedSystemMemory, int32& SharedSystemMemory)
{
	FTextureMemoryStats Stats;
	RHIGetTextureMemoryStats(Stats);
	if (Stats.AreHardwareStatsValid())
	{
		DedicatedVideoMemory = FMath::DivideAndRoundUp(Stats.DedicatedVideoMemory, (int64)(1024 * 1024));
		DedicatedSystemMemory = FMath::DivideAndRoundUp(Stats.DedicatedSystemMemory, (int64)(1024 * 1024));
		SharedSystemMemory = FMath::DivideAndRoundUp(Stats.SharedSystemMemory, (int64)(1024 * 1024));
	}
	else
	{
		DedicatedVideoMemory = -1;
		DedicatedSystemMemory = -1;
		SharedSystemMemory = -1;
	}
}

void UUMGGraphicsOptions::CallAutoDetectQualitySettings()
{
	UBRSGameInstance* GI = GetOwningPlayer() ? Cast<UBRSGameInstance>(GetOwningPlayer()->GetGameInstance()) : nullptr;
	GI->ShowWaitDialog(NSLOCTEXT("VideoSettings", "AutoDetectWaitTitle", "Please Wait"),
		NSLOCTEXT("VideoSettings", "AutoDetectWaitMessage", "The game is currently detecting the optimal quality settings. This can take a couple of seconds."));

	FTimerHandle TempTimer;
	GetWorld()->GetTimerManager().SetTimer(TempTimer, this, &UUMGGraphicsOptions::AutoDetectQualitySettings, 0.1f, false);
}

void UUMGGraphicsOptions::AutoDetectQualitySettings()
{
	RecommendedQualityLevels = Scalability::BenchmarkQualityLevels();

	UBRSGameInstance* GI = GetOwningPlayer() ? Cast<UBRSGameInstance>(GetOwningPlayer()->GetGameInstance()) : nullptr;
	if (GI)
	{

		const FText QualityLevelText[4] = 
		{
			NSLOCTEXT("VideoSettings", "QualityLow", "Low"),
			NSLOCTEXT("VideoSettings", "QualityMedium", "Medium"),
			NSLOCTEXT("VideoSettings", "QualityHigh", "High"),
			NSLOCTEXT("VideoSettings", "QualityEpic", "Epic") 
		};
		const FText QualitySettingsText[7] =
		{
			NSLOCTEXT("VideoSettings", "BenchmarkResolution", "Screen Percentage"),
			NSLOCTEXT("VideoSettings", "BenchmarkAA", "Anti-Aliasing Quality"),
			NSLOCTEXT("VideoSettings", "BenchmarkTexture", "Texture Quality"),
			NSLOCTEXT("VideoSettings", "BenchmarkViewDistance", "View Distance Quality"),
			NSLOCTEXT("VideoSettings", "BenchmarkShadow", "Shadow Quality"),
			NSLOCTEXT("VideoSettings", "BenchmarkEffects", "Effects Quality"),
			NSLOCTEXT("VideoSettings", "BenchmarkPostProcessing", "Post Process Quality")
		};
		
		// todo: This is super messy. Make it more readable
		FString SettingsString = FString::Printf(TEXT("    %s: %d% %\n    %s: %s\n    %s: %s\n    %s: %s\n    %s: %s\n    %s: %s\n    %s: %s"),
			*QualitySettingsText[0].ToString(), RecommendedQualityLevels.ResolutionQuality,
			*QualitySettingsText[1].ToString(), *QualityLevelText[RecommendedQualityLevels.AntiAliasingQuality].ToString(),
			*QualitySettingsText[2].ToString(), *QualityLevelText[RecommendedQualityLevels.TextureQuality].ToString(),
			*QualitySettingsText[3].ToString(), *QualityLevelText[RecommendedQualityLevels.ViewDistanceQuality].ToString(),
			*QualitySettingsText[4].ToString(), *QualityLevelText[RecommendedQualityLevels.ShadowQuality].ToString(),
			*QualitySettingsText[5].ToString(), *QualityLevelText[RecommendedQualityLevels.EffectsQuality].ToString(),
			*QualitySettingsText[6].ToString(), *QualityLevelText[RecommendedQualityLevels.PostProcessQuality].ToString()
			);

		FText Message = FText::Format(NSLOCTEXT("VideoSettings", "BenchmarkResultMessageFormat",
			"The benchmark has determined the following results: \n{0}\n\nDo you want to apply these settings?"), FText::FromString(SettingsString));

		PendingMsgWindow = GI->SimpleShowMessageWindow(
			NSLOCTEXT("VideoSettings", "BenchmarkResultTitle", "Auto Detect Settings"),
			Message,
			NSLOCTEXT("VideoSettings", "BenchmarkResultBTNApply", "Apply"),
			NSLOCTEXT("VideoSettings", "BenchmarkResultBTNCancel", "Discard"));

		if (PendingMsgWindow)
		{
			BenchmarkConfirmedHandle = PendingMsgWindow->GetOKPressedEvent().AddUObject(this, &UUMGGraphicsOptions::OnBenchmarkConfirmed);
			BenchmarkCanceledHandle = PendingMsgWindow->GetCancelPressedEvent().AddUObject(this, &UUMGGraphicsOptions::OnBenchmarkCanceled);
		}

		GI->HideWaitDialog();
	}
}

void UUMGGraphicsOptions::OnBenchmarkCanceled()
{
	UE_LOG(LogTemp, Log, TEXT("Canceled"));
	if(PendingMsgWindow)
	{
		PendingMsgWindow->GetOKPressedEvent().Remove(BenchmarkConfirmedHandle);
		PendingMsgWindow->GetCancelPressedEvent().Remove(BenchmarkCanceledHandle);
		PendingMsgWindow = nullptr;
		BenchmarkConfirmedHandle.Reset();
		BenchmarkCanceledHandle.Reset();
	}

}

void UUMGGraphicsOptions::OnBenchmarkConfirmed()
{
	UE_LOG(LogTemp, Log, TEXT("Confirmed"));
	if (PendingMsgWindow)
	{
		PendingMsgWindow->GetOKPressedEvent().Remove(BenchmarkConfirmedHandle);
		PendingMsgWindow->GetCancelPressedEvent().Remove(BenchmarkCanceledHandle);
		PendingMsgWindow = nullptr;
		BenchmarkConfirmedHandle.Reset();
		BenchmarkCanceledHandle.Reset();
	}

	UBRSGameUserSettings* UserSettings = Cast<UBRSGameUserSettings>(GEngine->GetGameUserSettings());
	UserSettings->ScalabilityQuality = RecommendedQualityLevels;
	Scalability::SetQualityLevels(UserSettings->ScalabilityQuality);
	Scalability::SaveState(GGameUserSettingsIni);

	// todo: Make it use the SG var widget's function?
	// Cleanup System Settings Section from configs
	UBRSSGOptionUMG::RemoveScalabilityKeysFromConfig(FString(TEXT("AntiAliasingQuality")), true);
	UBRSSGOptionUMG::RemoveScalabilityKeysFromConfig(FString(TEXT("EffectsQuality")), true);
	UBRSSGOptionUMG::RemoveScalabilityKeysFromConfig(FString(TEXT("PostProcessQuality")), true);
	UBRSSGOptionUMG::RemoveScalabilityKeysFromConfig(FString(TEXT("ShadowQuality")), true);
	UBRSSGOptionUMG::RemoveScalabilityKeysFromConfig(FString(TEXT("TextureQuality")), true);
	UBRSSGOptionUMG::RemoveScalabilityKeysFromConfig(FString(TEXT("ViewDistanceQuality")), true);
	UBRSSGOptionUMG::RemoveScalabilityKeysFromConfig(FString(TEXT("ResolutionQuality")), true);

	// Apply Settings with higher priority
	ApplyCVarSettingsGroupFromIni(TEXT("AntiAliasingQuality"), RecommendedQualityLevels.AntiAliasingQuality, *GScalabilityIni, ECVF_SetBySystemSettingsIni);
	ApplyCVarSettingsGroupFromIni(TEXT("EffectsQuality"), RecommendedQualityLevels.EffectsQuality, *GScalabilityIni, ECVF_SetBySystemSettingsIni);
	ApplyCVarSettingsGroupFromIni(TEXT("PostProcessQuality"), RecommendedQualityLevels.PostProcessQuality, *GScalabilityIni, ECVF_SetBySystemSettingsIni);
	ApplyCVarSettingsGroupFromIni(TEXT("ShadowQuality"), RecommendedQualityLevels.ShadowQuality, *GScalabilityIni, ECVF_SetBySystemSettingsIni);
	ApplyCVarSettingsGroupFromIni(TEXT("TextureQuality"), RecommendedQualityLevels.TextureQuality, *GScalabilityIni, ECVF_SetBySystemSettingsIni);
	ApplyCVarSettingsGroupFromIni(TEXT("ViewDistanceQuality"), RecommendedQualityLevels.ViewDistanceQuality, *GScalabilityIni, ECVF_SetBySystemSettingsIni);
	ApplyCVarSettingsGroupFromIni(TEXT("ResolutionQuality"), RecommendedQualityLevels.ResolutionQuality, *GScalabilityIni, ECVF_SetBySystemSettingsIni);

	OnUpdate();
}


