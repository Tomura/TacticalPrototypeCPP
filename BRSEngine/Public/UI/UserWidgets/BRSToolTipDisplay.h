// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "UserWidget.h"
#include "BRSToolTipDisplay.generated.h"


UCLASS(Config = Game, abstract)
class UBRSToolTipDisplay : public UUserWidget
{
	GENERATED_BODY()

public:
	UBRSToolTipDisplay(const FObjectInitializer& OI);

	UPROPERTY()
	class UBRSCustomToolTipWidget* CurrentTooltip;

	UFUNCTION(Category = ToolTip, BlueprintPure)
	void GetToolTipTextContent(FText& OutText);

	UFUNCTION(Category = ToolTip, BlueprintPure)
	void GetToolTipTextHeader(FText& OutText);


	virtual void OnSetTooltip(class UBRSCustomToolTipWidget* NewToolTipSource);
	virtual void OnRemoveTooltip(class UBRSCustomToolTipWidget* OldToolTipSource);

	UFUNCTION(Category = Tooltip, BlueprintPure)
	bool DisplayingTooltip();
};