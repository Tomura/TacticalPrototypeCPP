// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "AutoPanner.h"
#include "SAutoPanner.h"


UAutoPanner::UAutoPanner(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

FVector2D UAutoPanner::GetNormalizedCursorPosition() const
{
	return MyAutoPanner.IsValid() ? MyAutoPanner->GetNormalizedCursorPosition() : FVector2D::ZeroVector;
}

void UAutoPanner::SynchronizeProperties()
{
	Super::SynchronizeProperties();
}

void UAutoPanner::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyAutoPanner.Reset();
}

void UAutoPanner::PostLoad()
{
	Super::PostLoad();
}

UClass* UAutoPanner::GetSlotClass() const
{
	return UPanelSlot::StaticClass();
}

void UAutoPanner::OnSlotAdded(UPanelSlot* inSlot)
{
	// Add the child to the live slot if it already exists
	if (MyAutoPanner.IsValid())
	{
		MyAutoPanner->SetContent(inSlot->Content ? inSlot->Content->TakeWidget() : SNullWidget::NullWidget);
	}
}

void UAutoPanner::OnSlotRemoved(UPanelSlot* inSlot)
{
	// Remove the widget from the live slot if it exists.
	if (MyAutoPanner.IsValid())
	{
		MyAutoPanner->SetContent(SNullWidget::NullWidget);
	}
}

TSharedRef<SWidget> UAutoPanner::RebuildWidget()
{
	if ( GetChildrenCount() > 0 )
	{
		UPanelSlot* NewSlot = GetContentSlot();
		if ( !IsDesignTime() )
		{
			MyAutoPanner = SNew(SAutoPanner)
				.PaddingScale(PaddingScale);

			if ( GetChildrenCount() > 0 )
			{
				MyAutoPanner->SetContent(NewSlot->Content ? NewSlot->Content->TakeWidget() : SNullWidget::NullWidget);
			}

			return MyAutoPanner.ToSharedRef();
		}
		else
		{
			return SNew(SBox)
				[
					NewSlot->Content ? NewSlot->Content->TakeWidget() : SNullWidget::NullWidget
				];
		}
	}

	return SNullWidget::NullWidget;
}