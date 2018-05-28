// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "SlateBasics.h"

class BRSENGINE_API SKeyBind : public SButton
{
public:
	DECLARE_EVENT(SKeyBind, FBRSKeyReboundEvent);
	FBRSKeyReboundEvent& OnKeyRebound(){ return KeyReboundEvent; }

	SLATE_BEGIN_ARGS(SKeyBind)
		: _ButtonStyle(&FCoreStyle::Get().GetWidgetStyle< FButtonStyle >("Button"))
		, _TextStyle(&FCoreStyle::Get().GetWidgetStyle< FTextBlockStyle >("NormalText"))
		, _HAlign(HAlign_Center)
		, _VAlign(VAlign_Center)
		, _ContentPadding(FMargin(4.0, 2.0))
		, _DesiredSizeScale(FVector2D(1, 1))
		, _ContentScale(FVector2D(1, 1))
		, _ButtonColorAndOpacity(FLinearColor::White)
		, _ForegroundColor(FCoreStyle::Get().GetSlateColor("InvertedForeground"))
		, _Key(nullptr)
	{}

	SLATE_STYLE_ARGUMENT(FButtonStyle, ButtonStyle)
		SLATE_STYLE_ARGUMENT(FTextBlockStyle, TextStyle)
		SLATE_ARGUMENT(EHorizontalAlignment, HAlign)
		SLATE_ARGUMENT(EVerticalAlignment, VAlign)
		SLATE_ATTRIBUTE(FMargin, ContentPadding)
		SLATE_ATTRIBUTE(FVector2D, DesiredSizeScale)
		SLATE_ATTRIBUTE(FVector2D, ContentScale)
		SLATE_ATTRIBUTE(FSlateColor, ButtonColorAndOpacity)
		SLATE_ATTRIBUTE(FSlateColor, ForegroundColor)
		SLATE_ARGUMENT(TSharedPtr<FKey>, Key)
		SLATE_END_ARGS()


	void Construct(const FArguments& InArgs)
	{
		SButton::Construct(SButton::FArguments()
			.ButtonStyle(InArgs._ButtonStyle)
			.TextStyle(InArgs._TextStyle)
			.HAlign(InArgs._HAlign)
			.VAlign(InArgs._VAlign)
			.ContentPadding(InArgs._ContentPadding)
			.DesiredSizeScale(InArgs._DesiredSizeScale)
			.ContentScale(InArgs._ContentScale)
			.ButtonColorAndOpacity(InArgs._ButtonColorAndOpacity)
			.ForegroundColor(InArgs._ForegroundColor)
			);

		ChildSlot
			[
				SAssignNew(KeyText, STextBlock)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.TextStyle(InArgs._TextStyle)
			];

		if (InArgs._Key.IsValid())
		{
			Key = InArgs._Key;
			UpdateText();
		}
		//OnKeyRebound = _OnKeyRebound;
		bWaitingForKey = false;
	}


	virtual void SetKey(FKey NewKey, bool bCanReset = true)
	{
		if (Key.IsValid())
		{
			if ((NewKey == *Key || NewKey == EKeys::Escape) && bCanReset)
			{
				NewKey = FKey();
			}
			*Key = NewKey;
			UpdateText();
			if (KeyReboundEvent.IsBound())
			{
				KeyReboundEvent.Broadcast();
			}
		}
		else
		{
			KeyText->SetText(FString(TEXT("Pointer Not Valid")));
		}

		bWaitingForKey = false;
		FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->Lock(NULL);
		FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->Show(true);
		FSlateApplication::Get().ClearKeyboardFocus(EKeyboardFocusCause::SetDirectly);
	}

	void BindKeyPtr(TSharedPtr<FKey> InKey)
	{
		if (InKey.IsValid())
		{
			Key = InKey; 
			UpdateText();
		}
	}

	void UpdateText()
	{
		if (Key.IsValid())
		{
			KeyText->SetText(*Key == FKey() ? FString() : Key->GetDisplayName().ToString());
		}
	}


protected:

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyboardEvent) override
	{
		if (bWaitingForKey)
		{
			SetKey(InKeyboardEvent.GetKey());
			return FReply::Handled();
		}
		return FReply::Unhandled();
	}


	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (bWaitingForKey)
		{
			SetKey(MouseEvent.GetEffectingButton());
			return FReply::Handled();
		}
		else if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			//Get the center of the widget ao we can lock our mouse there
			FSlateRect Rect = MyGeometry.GetLayoutBoundingRect();
			WaitingMousePos.X = (Rect.Left + Rect.Right) * 0.5f;
			WaitingMousePos.Y = (Rect.Top + Rect.Bottom) * 0.5f;
			FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->SetPosition(WaitingMousePos.X, WaitingMousePos.Y);

			RECT Bounds;
			Bounds.left = FMath::RoundToInt((Rect.Left - Rect.Right)*0.25f + WaitingMousePos.X);
			Bounds.top = FMath::RoundToInt((Rect.Top - Rect.Bottom)*0.25f + WaitingMousePos.Y);
			Bounds.right = FMath::RoundToInt((Rect.Right - Rect.Left)*0.25f + WaitingMousePos.X);
			Bounds.bottom = FMath::RoundToInt((Rect.Bottom - Rect.Top)*0.25f + WaitingMousePos.Y);

			FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->Lock(&Bounds);

			KeyText->SetText(NSLOCTEXT("SKeyBind", "PressAnyKey", "*** Press Any Button ***"));
			bWaitingForKey = true;
			FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->Show(false);
			return FReply::Handled();
		}
		return FReply::Unhandled();
	}


	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (bWaitingForKey)
		{
			SetKey(MouseEvent.GetWheelDelta() > 0 ? EKeys::MouseScrollUp : EKeys::MouseScrollDown);
			return FReply::Handled();
		}
		return FReply::Unhandled();
	}



	virtual FReply OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& InAnalogInputEvent) override
	{
		//TODO: Get this working
		if (bWaitingForKey)
		{
			SetKey(InAnalogInputEvent.GetKey());
			return FReply::Handled();
		}
		return FReply::Unhandled();
	}


	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		//Make sure the mouse pointer doesnt leave the button
		if (bWaitingForKey)
		{
			FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->SetPosition(WaitingMousePos.X, WaitingMousePos.Y);
		}
		return SButton::OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	}

	virtual FVector2D ComputeDesiredSize(float Size) const override
	{
		return FVector2D(180.0f, 20.0f);
	}



private:
	TSharedPtr<FKey> Key;

	TSharedPtr<STextBlock> KeyText;

	FVector2D WaitingMousePos;

	bool bWaitingForKey;

	FBRSKeyReboundEvent KeyReboundEvent;
};