// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#include "BRSEngine.h"
#include "BRSEditableNumber.h"




TSharedRef<SWidget> UBRSEditableNumber::RebuildWidget()
{
	MyEditableText = SNew(SEditableText)
		.Style(&WidgetStyle)
		.MinDesiredWidth(MinimumDesiredWidth)
		.IsCaretMovedWhenGainFocus(IsCaretMovedWhenGainFocus)
		.SelectAllTextWhenFocused(SelectAllTextWhenFocused)
		.RevertTextOnEscape(RevertTextOnEscape)
		.ClearKeyboardFocusOnCommit(ClearKeyboardFocusOnCommit)
		.SelectAllTextOnCommit(SelectAllTextOnCommit)
		.OnTextChanged(BIND_UOBJECT_DELEGATE(FOnTextChanged, HandleOnTextChanged))
		.OnTextCommitted(BIND_UOBJECT_DELEGATE(FOnTextCommitted, HandleOnTextCommitted))
		.VirtualKeyboardType(EVirtualKeyboardType::AsKeyboardType(KeyboardType.GetValue()))
		.OnIsTypedCharValid(BIND_UOBJECT_DELEGATE(FOnIsTypedCharValid, HandleTypedCharacterValid));

	return MyEditableText.ToSharedRef();
}

bool UBRSEditableNumber::HandleTypedCharacterValid(const TCHAR InChar)
{
	// only limit input to numbers
	switch(InChar)
	{
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '0':
		return true;
	default:
		return false;
	}
	return false;
}

float UBRSEditableNumber::GetNumberValue() const
{
	return FCString::Atof(*(GetText().ToString()));
}
