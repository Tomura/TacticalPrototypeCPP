// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "Widget.h"
#include "UserWidget.h"
#include "BRSCustomToolTipWidget.generated.h"

UCLASS(Config = Game, abstract)
class UBRSCustomToolTipWidget : public UUserWidget
{
	GENERATED_BODY()

	friend class UBRSToolTipDisplay;

public:
	UBRSCustomToolTipWidget(const FObjectInitializer& OI);

	virtual void SynchronizeProperties() override;

	UPROPERTY(Category = Tooltip, BlueprintReadOnly, EditAnywhere)
	FText CustomTooltipHeader;
	UPROPERTY()
	FGetText CustomTooltipHeaderDelegate;

	UPROPERTY(Category = Tooltip, BlueprintReadOnly, EditAnywhere, meta = (MultiLine = true))
	FText CustomTooltipText;
	UPROPERTY()
	FGetText CustomTooltipTextDelegate;
	
	
	void GetCustomTooltipText(FText& OutText);
	void GetCustomTooltipHeader(FText& OutText);



	/** Tooltip widget to show when the user hovers over the widget with the mouse */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Tooltip, AdvancedDisplay)
	class UWidget* CustomToolTipWidget;

	/** A bindable delegate for ToolTipWidget */
	UPROPERTY()
	FGetWidget CustomToolTipWidgetDelegate;

	virtual void NativeOnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& MouseEvent) override;


private:
	GAME_SAFE_BINDING_IMPLEMENTATION(FText, CustomTooltipText);
	GAME_SAFE_BINDING_IMPLEMENTATION(FText, CustomTooltipHeader);
};