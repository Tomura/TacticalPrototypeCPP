// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "BRSSGOptionUMG.generated.h"

UENUM()
namespace EBRSScalabilityGroup
{
	enum Type
	{
		SG_AntiAliasingQuality	UMETA(DisplayName = "AntiAliasingQuality"),
		SG_EffectsQuality		UMETA(DisplayName = "EffectsQuality"),
		SG_PostProcessQuality	UMETA(DisplayName = "PostProcessQuality"),
		SG_ShadowQuality		UMETA(DisplayName = "ShadowQuality"),
		SG_TextureQuality		UMETA(DisplayName = "TextureQuality"),
		SG_ViewDistanceQuality	UMETA(DisplayName = "ViewDistanceQuality"),
		SG_ResolutionQuality	UMETA(DisplayName = "ResolutionQuality"),
		
		// Should be last for size
		MAX						UMETA(Hidden)
	};
}

/**
 * 
 */
UCLASS(abstract)
class BRSENGINE_API UBRSSGOptionUMG : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UBRSSGOptionUMG(const FObjectInitializer& ObjectInitializer);

	virtual void SynchronizeProperties() override;

	const TCHAR* SECTIONSystemSettings = TEXT("SystemSettings");

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Scalability)
	void UpdateValue();
	virtual void UpdateValue_Implementation();

	static void RemoveScalabilityKeysFromConfig(const FString& ScalabilityGroup, bool bSave = true);

	UFUNCTION(BlueprintCallable, Category = Scalability)
	void SetQualityLevel(int32 inQuality);

	void UsingCustomSettings();

	FString GetScalabilityGroupName() const;
protected:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Scalability)
	TEnumAsByte<EBRSScalabilityGroup::Type> ScalabilityGroup;


	UPROPERTY(BlueprintReadWrite, Category = Scalability)
	TArray<class UBRSCVarOptionUMG*> CVarWidgets;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Scalability)
	int32 GetQualityLevel() const;

	bool IsUsingCustomSettings();

	UPROPERTY(Category = Tooltip, BlueprintReadOnly, EditAnywhere)
		FText CustomTooltipHeader;

	UPROPERTY(Category = Tooltip, BlueprintReadOnly, EditAnywhere, meta = (MultiLine = true))
		FText CustomTooltipText;

	/** Tooltip widget to show when the user hovers over the widget with the mouse */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Tooltip, AdvancedDisplay)
	class UWidget* CustomToolTipWidget;

	/** A bindable delegate for ToolTipWidget */
	UPROPERTY()
	FGetWidget CustomToolTipWidgetDelegate;
};
