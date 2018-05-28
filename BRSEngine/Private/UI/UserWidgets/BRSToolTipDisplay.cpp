// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSCustomToolTipWidget.h"
#include "BRSToolTipDisplay.h"


UBRSToolTipDisplay::UBRSToolTipDisplay(const FObjectInitializer& OI)
	: Super(OI)
{
	CurrentTooltip = nullptr;
}

void UBRSToolTipDisplay::GetToolTipTextContent(FText& OutText)
{
	if (CurrentTooltip && CurrentTooltip->IsValidLowLevelFast())
	{
		CurrentTooltip->GetCustomTooltipText(OutText);
	}
}

void UBRSToolTipDisplay::GetToolTipTextHeader(FText& OutText)
{
	if (CurrentTooltip && CurrentTooltip->IsValidLowLevelFast())
	{
		CurrentTooltip->GetCustomTooltipHeader(OutText);
	}
}

void UBRSToolTipDisplay::OnSetTooltip(class UBRSCustomToolTipWidget* NewToolTipSource)
{
	if (CurrentTooltip != NewToolTipSource)
	{
		CurrentTooltip = NewToolTipSource;
	}
}

void UBRSToolTipDisplay::OnRemoveTooltip(class UBRSCustomToolTipWidget* OldToolTipSource)
{
	if (OldToolTipSource == nullptr)
		return;

	if (CurrentTooltip == OldToolTipSource)
	{
		CurrentTooltip = nullptr;
	}
}

bool UBRSToolTipDisplay::DisplayingTooltip()
{
	return (CurrentTooltip && CurrentTooltip->IsValidLowLevelFast());
}
