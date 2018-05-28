// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "Scalability.h"
#include "UMGGraphicsOptions.generated.h"



/**
 * 
 */
UCLASS(abstract)
class BRSENGINE_API UUMGGraphicsOptions : public UUserWidget
{
	GENERATED_BODY()

public:
	UUMGGraphicsOptions(const FObjectInitializer& ObjectInitializer);

	const TCHAR* SECTIONSystemSettings = TEXT("SystemSettings");

	UFUNCTION(BlueprintCallable, Category = RHI)
	void GetDisplayAdapterResolutions(TArray<FString> & AvailableResolutions, FString& CurrentRes) const;


	UFUNCTION(BlueprintCallable, Category = Settings)
	void ChangeResolution(FString Resolution);

	UFUNCTION(BlueprintCallable, Category = Settings)
	void SetFullscreenMode(int32 WindowMode);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Settings)
	FString GetCurrentResolution() const;

	UFUNCTION(BlueprintCallable, Category = Settings)
	void SetResolutionQuality(int32 inResolutionQuality);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Settings)
	int32 GetResolutionQuality();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Settings)
	bool IsFullScreen() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Settings)
	int32 GetFullscreenMode() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Settings)
		void GetScalabilitySettings(
		int32& TextureQuality,
		int32& ViewDistanceQuality,
		int32& ShadowQuality,
		int32& PostProcessQuality,
		int32& EffectsQuality,
		int32& AntiAliasingQuality) const;




	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Settings)
	bool GetUseVSync() const;

	UFUNCTION(BlueprintCallable, Category = Settings)
	void SetUseVSync(bool bUseVSync);
	
	UFUNCTION(BlueprintCallable, Category = "Settings|PostProcessing")
	void SetMotionBlurScale(float Scale);
	UFUNCTION(BlueprintCallable, Category = "Settings|PostProcessing")
	float GetMotionBlurScale() const;


	UFUNCTION(BlueprintCallable, Category = "Settings|PostProcessing")
	void SetDepthOfFieldScale(float Scale);
	UFUNCTION(BlueprintCallable, Category = "Settings|PostProcessing")
	float GetDepthOfFieldScale() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|PostProcessing")
	void SetLensFlares(bool bEnabled);
	UFUNCTION(BlueprintCallable, Category = "Settings|PostProcessing")
	bool GetLensFlares() const;

	UFUNCTION(BlueprintCallable, Category = "Settings|PostProcessing")
		void SetEyeAdaptation(bool bEnabled);
	UFUNCTION(BlueprintCallable, Category = "Settings|PostProcessing")
		bool GetEyeAdaptation() const;


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "System Info")
	FString GetGraphicsAdapterName();

	/**
	 * Returns GPU Memory Sizes in MB
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "System Info")
	void GetGPUMemoryInfo(int32& DedicatedVideoMemory, int32& DedicatedSystemMemory, int32& SharedSystemMemory);


	UFUNCTION(BlueprintCallable, Category = "Settings")
	void CallAutoDetectQualitySettings();


	UFUNCTION(BlueprintCallable, Category = "Settings")
	void AutoDetectQualitySettings();

	UFUNCTION()
	void OnBenchmarkConfirmed();

	UFUNCTION()
	void OnBenchmarkCanceled();

protected:


	UFUNCTION(BlueprintImplementableEvent, Category = "Settings")
	void OnUpdate();

	Scalability::FQualityLevels RecommendedQualityLevels;

	FDelegateHandle BenchmarkConfirmedHandle;
	FDelegateHandle BenchmarkCanceledHandle;

	class UBRSMessageWindowUMG* PendingMsgWindow;

	UPROPERTY(Category = "Settings", BlueprintReadWrite)
	TArray<class UBRSSGOptionUMG*> SGOptionWidgets;
};
