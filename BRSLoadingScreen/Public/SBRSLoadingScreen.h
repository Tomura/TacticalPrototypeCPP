// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "GenericApplication.h"
#include "GenericApplicationMessageHandler.h"
#include "SlateBasics.h"
#include "SlateExtras.h"

struct BRSLOADINGSCREEN_API FBRSGameLoadingScreenBrush : public FSlateDynamicImageBrush, public FGCObject
{
	FBRSGameLoadingScreenBrush(const FName InTextureName, const FVector2D& InImageSize)
		: FSlateDynamicImageBrush(InTextureName, InImageSize)
	{
		ResourceObject = LoadObject<UObject>(NULL, *InTextureName.ToString());
	}

	virtual void AddReferencedObjects(FReferenceCollector& Collector)
	{
		if (ResourceObject)
		{
			Collector.AddReferencedObject(ResourceObject);
		}
	}
};

class BRSLOADINGSCREEN_API SBRSLoadingScreen2 : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SBRSLoadingScreen2) {}
	SLATE_END_ARGS()

	
	virtual bool IsInteractable() const override { return false;}

	void Construct(const FArguments& InArgs)
	{
		static const FName LoadingScreenName(TEXT("/Game/UI/LoadingScreen/LoadingScreen.LoadingScreen"));

		//since we are not using game styles here, just load one image
		LoadingScreenBrush = MakeShareable(new FBRSGameLoadingScreenBrush(LoadingScreenName, FVector2D(1920, 1080)));

		ChildSlot
			[
			SNew(SOverlay)
			.Cursor(EMouseCursor::None)
			+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SImage)
					.Image(LoadingScreenBrush.Get())
					.Cursor(EMouseCursor::None)
				]
			+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SSafeZone)
					.VAlign(VAlign_Bottom)
					.HAlign(HAlign_Center)
					.Padding(30.0f)
					.IsTitleSafe(true)
					.Cursor(EMouseCursor::None)
					[
						SNew(SThrobber)
						.Visibility(this, &SBRSLoadingScreen2::GetLoadIndicatorVisibility)
						.Cursor(EMouseCursor::None)
					]
				]
			];
		Cursor = EMouseCursor::None;
	}

	
private:
	EVisibility GetLoadIndicatorVisibility() const
	{
		return EVisibility::Visible;
	}

	/** loading screen image brush */
	TSharedPtr<FSlateDynamicImageBrush> LoadingScreenBrush;

};