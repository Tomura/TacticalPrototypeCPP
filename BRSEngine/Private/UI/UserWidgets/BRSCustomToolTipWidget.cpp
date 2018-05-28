// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSToolTipDisplay.h"
#include "BRSCustomToolTipWidget.h"


UBRSCustomToolTipWidget::UBRSCustomToolTipWidget(const FObjectInitializer& OI)
	:Super(OI)
{

}

void UBRSCustomToolTipWidget::SynchronizeProperties()
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

void UBRSCustomToolTipWidget::GetCustomTooltipText(FText& OutText)
{
	TAttribute<FText> tip = GAME_SAFE_OPTIONAL_BINDING(FText, CustomTooltipText);
	OutText = tip.Get();
}

void UBRSCustomToolTipWidget::GetCustomTooltipHeader(FText& OutText)
{
	TAttribute<FText> tip = GAME_SAFE_OPTIONAL_BINDING(FText, CustomTooltipHeader);
	OutText = tip.Get();
}

void UBRSCustomToolTipWidget::NativeOnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	Super::NativeOnMouseEnter(MyGeometry, MouseEvent);

	if (CustomToolTipWidgetDelegate.IsBound())
	{
		UWidget* DelegateResult = CustomToolTipWidgetDelegate.Execute();
		CustomToolTipWidget = Cast<UBRSToolTipDisplay>(DelegateResult);
	}

	if (CustomToolTipWidget && CustomToolTipWidget->IsValidLowLevel())
	{
		UBRSToolTipDisplay* TipWidget = Cast<UBRSToolTipDisplay>(CustomToolTipWidget);
		if(TipWidget)
			TipWidget->OnSetTooltip(this);
	}
}

void UBRSCustomToolTipWidget::NativeOnMouseLeave(const FPointerEvent& MouseEvent)
{
	Super::NativeOnMouseLeave(MouseEvent);

	if (CustomToolTipWidget && CustomToolTipWidget->IsValidLowLevel())
	{
		UBRSToolTipDisplay* TipWidget = Cast<UBRSToolTipDisplay>(CustomToolTipWidget);
		if (TipWidget)
			TipWidget->OnRemoveTooltip(this);
	}
}

