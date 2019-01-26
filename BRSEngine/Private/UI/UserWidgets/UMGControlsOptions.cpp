// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "UMGKeyBindWidget_Base.h"
#include "UMGControlsOptions.h"


///////////////////////////////////////////////////
// UUMGControlsOptions

UUMGControlsOptions::UUMGControlsOptions(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UUMGControlsOptions::ResetKeyBindsToDefaults()
{
	TArray<UWidget*> Children;
	WidgetTree->GetAllWidgets(Children);
	//UE_LOG(LogTemp, Log, TEXT("%d"), Children.Num());
	for (UWidget* Child : Children)
	{
		UUMGKeyBindWidget_Base*  KeyBind = Cast<UUMGKeyBindWidget_Base>(Child);
		if (KeyBind && KeyBind->IsValidLowLevel())
		{
			KeyBind->ResetToDefaults();
		}
	}
}

float UUMGControlsOptions::GetMouseSensitivity() const
{
	return UPlayerInput::StaticClass()->GetDefaultObject<UPlayerInput>()->GetMouseSensitivityX(); // todo or Y
}

void UUMGControlsOptions::SetMouseSettings(float NewSensitivity, bool bInvertY)
{
	for (TObjectIterator<UPlayerInput> It(RF_NoFlags); It; ++It)
	{
		It->SetMouseSensitivity(NewSensitivity);
		//It->InvertAxisKey(EKeys::MouseY);
		for (FInputAxisConfigEntry& Entry : It->AxisConfig)
		{
			if (Entry.AxisKeyName == EKeys::MouseY)
			{
				Entry.AxisProperties.bInvert = bInvertY;
			}
		}
	}
	
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	for (FInputAxisConfigEntry& Entry : InputSettings->AxisConfig)
	{
		if (Entry.AxisKeyName == EKeys::MouseX || Entry.AxisKeyName == EKeys::MouseY)
		{
			Entry.AxisProperties.Sensitivity = NewSensitivity;
			if (Entry.AxisKeyName == EKeys::MouseY)
			{
				Entry.AxisProperties.bInvert = bInvertY;
			}
		}
	}

	InputSettings->SaveConfig();
}

bool UUMGControlsOptions::GetInvertMouseYAxis() const
{
	//Is Mouse Inverted
	bool bMouseInverted = false;
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	for (FInputAxisConfigEntry& Entry : InputSettings->AxisConfig)
	{
		if (Entry.AxisKeyName == EKeys::MouseY)
		{
			bMouseInverted = Entry.AxisProperties.bInvert;
			break;
		}
	}
	return bMouseInverted;
}

float UUMGControlsOptions::GetAnalogStickSensitivity(bool bLeftStick) const
{
	const FKey& AxisKey = bLeftStick ? EKeys::Gamepad_LeftY : EKeys::Gamepad_RightY;
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	for (FInputAxisConfigEntry& Entry : InputSettings->AxisConfig)
	{
		if (Entry.AxisKeyName == AxisKey)
		{
			return Entry.AxisProperties.Sensitivity;
			break;
		}
	}
	return 0.f;
}

float UUMGControlsOptions::GetAnalogStickExponent(bool bLeftStick) const
{
	const FKey& AxisKey = bLeftStick ? EKeys::Gamepad_LeftY : EKeys::Gamepad_RightY;
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	for (FInputAxisConfigEntry& Entry : InputSettings->AxisConfig)
	{
		if (Entry.AxisKeyName == AxisKey)
		{
			return Entry.AxisProperties.Exponent;
			break;
		}
	}
	return 0.f;
}

bool UUMGControlsOptions::GetInvertAnalogStickYAxis(bool bLeftStick) const
{
	bool bStickInverted = false;

	const FKey& AxisKey = bLeftStick ? EKeys::Gamepad_LeftY : EKeys::Gamepad_RightY;
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	for (FInputAxisConfigEntry& Entry : InputSettings->AxisConfig)
	{
		if (Entry.AxisKeyName == AxisKey)
		{
			bStickInverted = Entry.AxisProperties.bInvert;
			break;
		}
	}
	return bStickInverted;
}

void UUMGControlsOptions::SetAnalogStickSettings(float NewSensitivity, bool bInvertY, float Exponent, bool bLeftStick)
{
	const FKey& YAxisKey = bLeftStick ? EKeys::Gamepad_LeftY : EKeys::Gamepad_RightY;
	const FKey& XAxisKey = bLeftStick ? EKeys::Gamepad_LeftX : EKeys::Gamepad_RightX;

	float ActualExponent = (Exponent > 1.f) ? Exponent : 1.f;

	for (TObjectIterator<UPlayerInput> It(RF_NoFlags); It; ++It)
	{
		for (FInputAxisConfigEntry& Entry : It->AxisConfig)
		{
			if (Entry.AxisKeyName == XAxisKey || Entry.AxisKeyName == YAxisKey)
			{
				Entry.AxisProperties.Sensitivity = NewSensitivity;
				Entry.AxisProperties.Exponent = ActualExponent;
				if (Entry.AxisKeyName == YAxisKey)
				{
					Entry.AxisProperties.bInvert = bInvertY;
				}
				It->SetAxisProperties(Entry.AxisKeyName, Entry.AxisProperties);
			}
		}
	}

	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	for (FInputAxisConfigEntry& Entry : InputSettings->AxisConfig)
	{
		if (Entry.AxisKeyName == XAxisKey || Entry.AxisKeyName == YAxisKey)
		{
			Entry.AxisProperties.Sensitivity = NewSensitivity;
			Entry.AxisProperties.Exponent = ActualExponent;
			if (Entry.AxisKeyName == YAxisKey)
			{
				Entry.AxisProperties.bInvert = bInvertY;
			}
		}
	}

	InputSettings->SaveConfig();
}

float UUMGControlsOptions::GetAnalogStickDeadzone() const
{
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	for (FInputAxisConfigEntry& Entry : InputSettings->AxisConfig)
	{
		if (Entry.AxisKeyName == EKeys::Gamepad_LeftY || Entry.AxisKeyName == EKeys::Gamepad_LeftX 
			|| Entry.AxisKeyName == EKeys::Gamepad_RightY || Entry.AxisKeyName == EKeys::Gamepad_RightX)
		{
			return Entry.AxisProperties.DeadZone;
		}
	}
	return 0.f;
}

void UUMGControlsOptions::SetAnalogStickDeadzone(float Deadzone)
{
	
	float ActualDeadzone = FMath::Clamp(Deadzone, 0.f, 1.f);
	for (TObjectIterator<UPlayerInput> It(RF_NoFlags); It; ++It)
	{
		for (FInputAxisConfigEntry& Entry : It->AxisConfig)
		{
			if (Entry.AxisKeyName == EKeys::Gamepad_LeftY || Entry.AxisKeyName == EKeys::Gamepad_LeftX
				|| Entry.AxisKeyName == EKeys::Gamepad_RightY || Entry.AxisKeyName == EKeys::Gamepad_RightX)
			{
				Entry.AxisProperties.DeadZone = ActualDeadzone;
				It->SetAxisProperties(Entry.AxisKeyName, Entry.AxisProperties);
			}
		}
	}

	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	for (FInputAxisConfigEntry& Entry : InputSettings->AxisConfig)
	{
		if (Entry.AxisKeyName == EKeys::Gamepad_LeftY || Entry.AxisKeyName == EKeys::Gamepad_LeftX
			|| Entry.AxisKeyName == EKeys::Gamepad_RightY || Entry.AxisKeyName == EKeys::Gamepad_RightX)
		{
			Entry.AxisProperties.DeadZone = ActualDeadzone;
		}
	}

	InputSettings->SaveConfig();
}
