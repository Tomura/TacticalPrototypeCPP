// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSAnalogCursor.h"
#include "BRSAnalogCursorSettings.h"
#include "WidgetLayoutLibrary.h"
#include "Engine/UserInterfaceSettings.h"
#include "BRSPlayerController_Base.h"


bool IsWidgetInteractable(const TSharedPtr<SWidget> Widget)
{
	return Widget.IsValid() && Widget->IsInteractable();
}

////////////////////////////////////////////////////////////////////////////
// FBRSAnalogCursor
////////////////////////////////////////////////////////////////////////////

void FBRSAnalogCursor::EnableAnalogCursor(class APlayerController* PC, TSharedPtr<SWidget> WidgetToFocus)
{
	if (PC)
	{
		const float CursorRadius = GetDefault<UBRSAnalogCursorSettings>()->GetAnalogCursorRadius();
		TSharedPtr<FBRSAnalogCursor> AnalogCursor = MakeShareable(new FBRSAnalogCursor(PC, CursorRadius));
		FSlateApplication::Get().RegisterInputPreProcessor(AnalogCursor); //SetInputPreProcessor(true, AnalogCursor);
		FSlateApplication::Get().SetCursorRadius(CursorRadius);

		GetMutableDefault<UBRSAnalogCursorSettings>()->SetAnalogCursor(AnalogCursor);

		//setup the new input mode
		FInputModeGameAndUI NewInputMode;
		//NewInputMode.SetLockMouseToViewport(true);
		NewInputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		NewInputMode.SetWidgetToFocus(WidgetToFocus);
		PC->SetInputMode(NewInputMode);
	}
}

void FBRSAnalogCursor::DisableAnalogCursor(class APlayerController* PC)
{
	if (PC)
	{
		if (FSlateApplication::IsInitialized())
		{
			FSlateApplication::Get().UnregisterAllInputPreProcessors();
			FSlateApplication::Get().SetCursorRadius(0.0f);
		}

		FInputModeGameOnly NewInputMode;
		PC->SetInputMode(NewInputMode);
	}
}

FBRSAnalogCursor::FBRSAnalogCursor(class APlayerController* PC, float _Radius)
	: Velocity(FVector2D::ZeroVector)
	, CurrentPosition(FLT_MAX, FLT_MAX)
	, LastCursorDirection(FVector2D::ZeroVector)
	, HoveredWidgetName(NAME_None)
	, bIsUsingAnalogCursor(false)
	, Radius(FMath::Max<float>(_Radius, 16.0f))
	, PlayerContext(PC)
{
	ensure(PlayerContext.IsValid());
}

void FBRSAnalogCursor::Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor)
{
	if (PlayerContext.IsValid())
	{
		const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(PlayerContext.GetPlayerController());
		const float DPIScale = GetDefault<UUserInterfaceSettings>()->GetDPIScaleBasedOnSize(FIntPoint(FMath::RoundToInt(ViewportSize.X), FMath::RoundToInt(ViewportSize.Y)));


		const UBRSAnalogCursorSettings* Settings = GetDefault<UBRSAnalogCursorSettings>();

		//if we have no acceleration curve, then move on;
		if (Settings->GetUseEngineAnalogCursor())
		{
			FAnalogCursor::Tick(DeltaTime, SlateApp, Cursor);
			return;
		}

		//set the current position if we haven't already
		static const float MouseMoveSizeBuffer = 2.0f;
		const FVector2D CurrentPositionTruc = FVector2D(FMath::TruncToFloat(CurrentPosition.X), FMath::TruncToFloat(CurrentPosition.Y));
		if (CurrentPositionTruc != Cursor->GetPosition())
		{
			CurrentPosition = Cursor->GetPosition();
			Velocity = FVector2D::ZeroVector;
			LastCursorDirection = FVector2D::ZeroVector;
			bIsUsingAnalogCursor = false;
			FSlateApplication::Get().SetCursorRadius(0.0f);
		}

		//cache the old position
		const FVector2D OldPosition = CurrentPosition;

		//figure out if we should clamp the speed or not
		const float MaxSpeedNoHover = Settings->GetMaxAnalogCursorSpeed()*DPIScale;
		const float MaxSpeedHover = Settings->GetMaxAnalogCursorSpeedWhenHovered()*DPIScale;
		const float DragCoNoHover = Settings->GetAnalogCursorDragCoefficient()*DPIScale;
		const float DragCoHovered = Settings->GetAnalogCursorDragCoefficientWhenHovered()*DPIScale;
		const float MinCursorSpeed = Settings->GetMinAnalogCursorSpeed()*DPIScale;

		HoveredWidgetName = NAME_None;
		float DragCo = DragCoNoHover;
		float UseMaxSpeed = MaxSpeedNoHover;

		//see if we are hovered over a widget or not
		FWidgetPath WidgetPath = SlateApp.LocateWindowUnderMouse(OldPosition, SlateApp.GetInteractiveTopLevelWindows());
		if (WidgetPath.IsValid())
		{
			for (int32 i = WidgetPath.Widgets.Num() - 1; i >= 0; --i)
			{
				//grab the widget
				FArrangedWidget& ArrangedWidget = WidgetPath.Widgets[i];
				TSharedRef<SWidget> Widget = ArrangedWidget.Widget;

				//see if it is acceptable or not
				if (IsWidgetInteractable(Widget))
				{
					HoveredWidgetName = Widget->GetType();
					DragCo = DragCoHovered;
					UseMaxSpeed = MaxSpeedHover;
					break;
				}
			}
		}

		//grab the cursor acceleration
		const FVector2D AccelFromAnalogStick = GetAnalogCursorAccelerationValue(GetAnalogValues(), DPIScale);

		FVector2D NewAccelerationThisFrame = FVector2D::ZeroVector;
		if (!Settings->GetAnalogCursorNoAcceleration())
		{
			//Calculate a new velocity. RK4.
			if (!AccelFromAnalogStick.IsZero() || !Velocity.IsZero())
			{
				const FVector2D A1 = (AccelFromAnalogStick - (DragCo * Velocity)) * DeltaTime;
				const FVector2D A2 = (AccelFromAnalogStick - (DragCo * (Velocity + (A1 * 0.5f)))) * DeltaTime;
				const FVector2D A3 = (AccelFromAnalogStick - (DragCo * (Velocity + (A2 * 0.5f)))) * DeltaTime;
				const FVector2D A4 = (AccelFromAnalogStick - (DragCo * (Velocity + A3))) * DeltaTime;
				NewAccelerationThisFrame = (A1 + (2.0f * A2) + (2.0f * A3) + A4) / 6.0f;

				Velocity += NewAccelerationThisFrame;
			}
		}
		else
		{
			//else, use what is coming straight from the analog stick
			Velocity = AccelFromAnalogStick;
		}

		//if we are smaller than out min speed, zero it out
		const float VelSizeSq = Velocity.SizeSquared();
		if (VelSizeSq < (MinCursorSpeed * MinCursorSpeed))
		{
			Velocity = FVector2D::ZeroVector;
		}
		else if (VelSizeSq >(UseMaxSpeed * UseMaxSpeed))
		{
			//also cap us if we are larger than our max speed
			Velocity = Velocity.GetSafeNormal() * UseMaxSpeed;
		}

		//store off the last cursor direction
		if (!Velocity.IsZero())
		{
			LastCursorDirection = Velocity.GetSafeNormal();
		}

		//update the new position
		CurrentPosition += (Velocity * DeltaTime);

		//update the cursor position
		UpdateCursorPosition(SlateApp, Cursor, CurrentPosition);

		//if we get here, and we are moving the stick, then hooray
		if (!AccelFromAnalogStick.IsZero())
		{
			bIsUsingAnalogCursor = true;
			FSlateApplication::Get().SetCursorRadius(Settings->GetAnalogCursorRadius()*DPIScale);
		}
	}
}



bool FBRSAnalogCursor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	// call parent version of this function. Store return value for later use.
	// handles Bottom Face Button -> Left Mouse
	bool bReturnVal = FAnalogCursor::HandleKeyDownEvent(SlateApp, InKeyEvent);

	// Catch input and determine if player is using game pad or not to properly change the labels
	if (GWorld && PlayerContext.IsValid())
	{
		ABRSPlayerController_Base* PC = PlayerContext.GetPlayerController<ABRSPlayerController_Base>(false);
		if (PC)
		{
			const FKey Key = InKeyEvent.GetKey();

			bool bUsingGamepad = PC->IsUsingGamepad();

			if (!bUsingGamepad && Key.IsGamepadKey())
			{
				PC->SetUsingGamepad(true);
			}
			else if (bUsingGamepad && !Key.IsGamepadKey())
			{
				PC->SetUsingGamepad(false);
			}
		}
	}

	// return stored return value
	return bReturnVal;
}

bool FBRSAnalogCursor::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	// call parent version of this function. Store return value for later use.
	// handles Bottom Face Button -> Left Mouse
	bool bReturnVal = FAnalogCursor::HandleKeyUpEvent(SlateApp, InKeyEvent);

	// Catch input and determine if player is using game pad or not to properly change the labels
	if (GWorld && PlayerContext.IsValid())
	{
		ABRSPlayerController_Base* PC = PlayerContext.GetPlayerController<ABRSPlayerController_Base>(false);
		if (PC)
		{
			const FKey Key = InKeyEvent.GetKey();

			bool bUsingGamepad = PC->IsUsingGamepad();

			if (!bUsingGamepad && Key.IsGamepadKey())
			{
				PC->SetUsingGamepad(true);
			}
			else if (bUsingGamepad && !Key.IsGamepadKey())
			{
				PC->SetUsingGamepad(false);
			}
		}
	}

	// return stored return value
	return bReturnVal;
}

bool FBRSAnalogCursor::HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent)
{
	/*
	// Catch input and determine if player is using game pad or not to properly change the labels
	if (GWorld && PlayerContext.IsValid())
	{
		ABRSPlayerController_Base* PC = PlayerContext.GetPlayerController<ABRSPlayerController_Base>();
		if (PC)
		{
			const FKey Key = InAnalogInputEvent.GetKey();
			const float AnalogValue = InAnalogInputEvent.GetAnalogValue();

			bool bUsingGamepad = PC->IsUsingGamepad();

			if (!bUsingGamepad && (Key == EKeys::Gamepad_LeftX || Key == EKeys::Gamepad_LeftY) && FMath::Abs(AnalogValue) >= 0.1f)
			{
				PC->SetUsingGamepad(true);
			}
			else if (bUsingGamepad && (Key == EKeys::MouseX || Key == EKeys::MouseY) && FMath::Abs(AnalogValue) > 0.f)
			{
				PC->SetUsingGamepad(false);
			}
		}
	}*/

	return FAnalogCursor::HandleAnalogInputEvent(SlateApp, InAnalogInputEvent);
}


FVector2D FBRSAnalogCursor::GetAnalogCursorAccelerationValue(const FVector2D& inAnalogValues, float DPIScale)
{
	const UBRSAnalogCursorSettings* Settings = GetDefault<UBRSAnalogCursorSettings>();

	FVector2D RetValue = FVector2D::ZeroVector;
	if (const FRichCurve* AccelerationCurve = Settings->GetAnalogCursorAccelerationCurve())
	{
		const float DeadZoneSize = Settings->GetAnalogCursorDeadZone();
		const float AnalogValSize = inAnalogValues.Size();
		if (AnalogValSize > DeadZoneSize)
		{
			RetValue = AccelerationCurve->Eval(AnalogValSize) * inAnalogValues.GetSafeNormal() * DPIScale;
			RetValue *= Settings->GetAnalogCursorAccelerationMultiplier()*DPIScale;
		}
	}
	return RetValue;
}


