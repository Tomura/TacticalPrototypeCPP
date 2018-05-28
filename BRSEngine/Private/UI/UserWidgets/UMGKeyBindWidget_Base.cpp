// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "SKeyBind.h"
#include "BRSKeyBind.h"
#include "UMGKeyBindWidget_Base.h"


UUMGKeyBindWidget_Base::UUMGKeyBindWidget_Base(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayText = FText();
	Key = MakeShareable(new FKey);
	AltKey = MakeShareable(new FKey);

	AddDefaults(FKey());

	bCanReset = true;
	WaitingForKey = EWaitingForKeyState::WAIT_None;
}


void UUMGKeyBindWidget_Base::NativeConstruct()
{
	// Empty mappings
	if (ActionMappings.Num() > 0)
		ActionMappings.Empty();

	if (AxisMappings.Num() > 0)
		AxisMappings.Empty();

	// acquire mappings
	if (Binds.Num() > 0)
	{
		for (FBRSKeyBindData Bind : Binds)
		{
			AddMapping(Bind.Mapping, Bind.Scale);
		}
	}

	Super::NativeConstruct();
}


UUMGKeyBindWidget_Base* UUMGKeyBindWidget_Base::AddMapping(const FString& Mapping, float Scale)
{
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();

	//UE_LOG(LogTemp, Log, TEXT("AddingMapping: %s"), *Mapping);

	//Add any ActionMappings to the Array
	for (FInputActionKeyMapping Action : InputSettings->ActionMappings)
	{
		if (Mapping.Compare(Action.ActionName.ToString()) == 0 && !Action.Key.IsGamepadKey())
		{
			ActionMappings.Add(Action);
			if (*Key == FKey())
			{
				*Key = Action.Key;
			}
			else if (*AltKey == FKey() && Action.Key != *Key)
			{
				*AltKey = Action.Key;
			}
		}
	}

	// Add any AxisMappings to the Array
	for (FInputAxisKeyMapping Axis : InputSettings->AxisMappings)
	{
		if (Mapping.Compare(Axis.AxisName.ToString()) == 0 && !Axis.Key.IsGamepadKey() && Axis.Scale == Scale)
		{
			AxisMappings.Add(Axis);
			if (*Key == FKey())
			{
				*Key = Axis.Key;
			}
			else if (*AltKey == FKey() && Axis.Key != *Key)
			{
				*AltKey == Axis.Key;
			}
		}
	}

	//Special Console case
	if (Mapping.Compare(TEXT("Console")) == 0)
	{
		*Key = InputSettings->ConsoleKeys[0];
	}
	return this;
}

void UUMGKeyBindWidget_Base::WriteBind()
{
	UInputSettings* InputSettings = UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>();
	//Remove the original bindings
	for (auto& Bind : ActionMappings)
	{
		InputSettings->RemoveActionMapping(Bind);
	}
	for (auto& Bind : AxisMappings)
	{
		InputSettings->RemoveAxisMapping(Bind);
	}

	//Set our new keys and readd them
	for (auto Bind : ActionMappings)
	{
		if (*Key != FKey())
		{
			Bind.Key = *Key;
			InputSettings->AddActionMapping(Bind);
		}
		if (*AltKey != FKey())
		{
			Bind.Key = *AltKey;
			InputSettings->AddActionMapping(Bind);
		}
	}
	for (auto Bind : AxisMappings)
	{
		if (*Key != FKey())
		{
			Bind.Key = *Key;
			InputSettings->AddAxisMapping(Bind);
		}
		if (*AltKey != FKey())
		{
			Bind.Key = *AltKey;
			InputSettings->AddAxisMapping(Bind);
		}
	}

	// Reset our arrays to be in sync with the actual action/axis mappings

	// Empty mappings
	if (ActionMappings.Num() > 0)
		ActionMappings.Empty();

	if (AxisMappings.Num() > 0)
		AxisMappings.Empty();

	// acquire mappings
	if (Binds.Num() > 0)
	{
		for (FBRSKeyBindData Bind : Binds)
		{
			AddMapping(Bind.Mapping, Bind.Scale);
		}
	}

	////Special Console case
	//if (DisplayName.Compare(TEXT("Console")) == 0)
	//{
	//	InputSettings->ConsoleKeys.Empty();
	//	InputSettings->ConsoleKeys.Add(*Key);
	//}
}

void UUMGKeyBindWidget_Base::GetKeys(FKey& OutKey, FKey& OutAltKey) const
{
	OutKey = *Key;
	OutAltKey = *AltKey;
}


void UUMGKeyBindWidget_Base::SetKey(FKey InKey, bool bAltKey)
{
	FKey* KeyToChange = NULL;
	if (InKey != FKey())
	{
		if (((!bAltKey && (InKey == *Key)) || (bAltKey && (InKey == *AltKey))) && bCanReset)
		{
			InKey = FKey();
		}

		KeyToChange = !bAltKey ? Key.Get() : AltKey.Get();
		if (KeyToChange)
		{
			*KeyToChange = InKey;
		}
	}
	WaitingForKey = EWaitingForKeyState::WAIT_None;

	FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->Show(true);
	FSlateApplication::Get().ClearKeyboardFocus(EKeyboardFocusCause::SetDirectly);
	FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->Lock(NULL);
	//FSlateApplication::Get().LockCursor(NULL);
}



void UUMGKeyBindWidget_Base::WaitForKey(UButton* TriggeringWidget, bool bAltKey)
{
	if (WaitingForKey != EWaitingForKeyState::WAIT_None)
	{
		SetKey(EKeys::LeftMouseButton, WaitingForKey == EWaitingForKeyState::WAIT_Key ? false : true);
		return;
	}

	WaitingForKey = !bAltKey ? EWaitingForKeyState::WAIT_Key : EWaitingForKeyState::WAIT_Alt;
	FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->Show(false);
	FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->Lock(NULL);
	//FSlateApplication::Get().LockCursor(TriggeringWidget->TakeWidget());
}


void UUMGKeyBindWidget_Base::BindWidgetToKey(class UBRSKeyBind* Widget, bool bIsAlt)
{
	Widget->BindToKey(bIsAlt ? AltKey : Key);
	//Widget->MyKeyBind->OnKeyRebound.(this, &UUMGKeyBindWidget_Base::OnKeyRebound);
	Widget->MyKeyBind->OnKeyRebound().AddUObject<UUMGKeyBindWidget_Base>(this, &UUMGKeyBindWidget_Base::OnKeyRebound);
}


void UUMGKeyBindWidget_Base::OnKeyRebound()
{
	WriteBind();

	UInputSettings::StaticClass()->GetDefaultObject<UInputSettings>()->SaveConfig();
	UPlayerInput::StaticClass()->GetDefaultObject<UPlayerInput>()->SaveConfig();
}

void UUMGKeyBindWidget_Base::ResetToDefaults()
{
	*Key = DefaultKey;
	*AltKey = DefaultAltKey;

	TArray<UWidget*> Children;
	WidgetTree->GetAllWidgets(Children);

	for (UWidget* Child : Children)
	{
		UBRSKeyBind* KeyBindWidget = Cast<UBRSKeyBind>(Child);
		if (KeyBindWidget && KeyBindWidget->IsValidLowLevel())
		{
			KeyBindWidget->MyKeyBind->UpdateText();
		}
	}

	OnKeyRebound();
}