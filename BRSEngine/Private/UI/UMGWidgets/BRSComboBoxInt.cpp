// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSComboBoxInt.h"


#define LOCTEXT_NAMESPACE "UMG"

/////////////////////////////////////////////////////
// UComboBoxString
UBRSComboBoxInt::UBRSComboBoxInt(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SBRSComboBox< TSharedPtr<FBRSComboBoxIntOption> >::FArguments SlateDefaults;
	WidgetStyle = *SlateDefaults._ComboBoxStyle;
	TextStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText");
	ListTextStyle = FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText");
	RowStyle = FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row");
	ContentPadding = FMargin(4.0, 2.0);
	MaxListHeight = 450.0f;
	HasDownArrow = true;
	ForegroundColor = FSlateColor(FLinearColor::Black);

	CurrentTextStyle = &ListTextStyle;
}
void UBRSComboBoxInt::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	MyComboBox.Reset();
	ComoboBoxContent.Reset();
}
TSharedRef<SWidget> UBRSComboBoxInt::RebuildWidget()
{
	for (FBRSComboBoxIntOption& TestOption : DefaultOptions)
	{
		AddOption(TestOption);
	}
	int32 InitialIndex = FindOptionIndex(SelectedOption);
	if (InitialIndex != -1)
	{
		CurrentOptionPtr = Options[InitialIndex];
	}
	MyComboBox =
		SNew(SBRSComboBox< TSharedPtr<FBRSComboBoxIntOption> >)
		.ComboBoxStyle(&WidgetStyle)
		.OptionsSource(&Options)
		.RowStyle(&RowStyle)
		.InitiallySelectedItem(CurrentOptionPtr)
		.ContentPadding(ContentPadding)
		.MaxListHeight(MaxListHeight)
		.HasDownArrow(HasDownArrow)
		.ForegroundColor(ForegroundColor)
		.OnGenerateWidget(BIND_UOBJECT_DELEGATE(SBRSComboBox< TSharedPtr<FBRSComboBoxIntOption> >::FOnGenerateWidget, HandleGenerateWidget))
		.OnSelectionChanged(BIND_UOBJECT_DELEGATE(SBRSComboBox< TSharedPtr<FBRSComboBoxIntOption> >::FOnSelectionChanged, HandleSelectionChanged))
		.OnComboBoxOpening(BIND_UOBJECT_DELEGATE(FOnComboBoxOpening, HandleOpening))
		[
			SAssignNew(ComoboBoxContent, SBox)
		];
	if (InitialIndex != -1)
	{
		// Generate the widget for the initially selected widget if needed
		CurrentTextStyle = &TextStyle;
		ComoboBoxContent->SetContent(HandleGenerateWidget(CurrentOptionPtr));
		CurrentTextStyle = &ListTextStyle;
	}
	return MyComboBox.ToSharedRef();
}
void UBRSComboBoxInt::AddOption(const FBRSComboBoxIntOption& Option)
{
	Options.Add(MakeShareable(new FBRSComboBoxIntOption(Option)));
}

void UBRSComboBoxInt::AddOption(const FText& DisplayText, int32 Value)
{
	AddOption(FBRSComboBoxIntOption(Value, DisplayText));
}

bool UBRSComboBoxInt::RemoveOption(const FBRSComboBoxIntOption& Option)
{
	int32 OptionIndex = FindOptionIndex(Option);
	if (Options.IsValidIndex(OptionIndex) && Options[OptionIndex] == CurrentOptionPtr)
	{
		CurrentOptionPtr.Reset();
	}
	if (OptionIndex != -1)
	{
		Options.RemoveAt(OptionIndex);
		return true;
	}
	return false;
}
int32 UBRSComboBoxInt::FindOptionIndex(const FBRSComboBoxIntOption& Option) const
{
	for (int32 OptionIndex = 0; OptionIndex < Options.Num(); OptionIndex++)
	{
		const TSharedPtr<FBRSComboBoxIntOption>& OptionAtIndex = Options[OptionIndex];
		if ((*OptionAtIndex) == Option)
		{
			return OptionIndex;
		}
	}
	return -1;
}
FBRSComboBoxIntOption UBRSComboBoxInt::GetOptionAtIndex(int32 Index) const
{
	if (Index >= 0 && Index < Options.Num())
	{
		return *(Options[Index]);
	}
	return FBRSComboBoxIntOption();
}

void UBRSComboBoxInt::ClearOptions()
{
	Options.Empty();
}

void UBRSComboBoxInt::SetSelectedOption(FBRSComboBoxIntOption Option)
{
	int32 InitialIndex = FindOptionIndex(Option);
	if (InitialIndex != -1)
	{
		CurrentOptionPtr = Options[InitialIndex];		
		CurrentTextStyle = &TextStyle;
		ComoboBoxContent->SetContent(HandleGenerateWidget(CurrentOptionPtr));
		CurrentTextStyle = &ListTextStyle;
		MyComboBox->SetSelectedItem(CurrentOptionPtr);
	}
}

void UBRSComboBoxInt::SetSelectedOptionByIndex(int32 Index)
{
	if (Options.IsValidIndex(Index))
	{
		SetSelectedOption(*Options[Index]);
	}
}

void UBRSComboBoxInt::SetSelectedOptionByValue(int32 Value)
{
	for (TSharedPtr<FBRSComboBoxIntOption> Option : Options)
	{
		if (Option.IsValid() && Option->Value == Value)
		{
			SetSelectedOption(*Option);
		}
	}
}

FBRSComboBoxIntOption UBRSComboBoxInt::GetSelectedOption() const
{
	if (CurrentOptionPtr.IsValid())
	{
		return *CurrentOptionPtr;
	}
	return FBRSComboBoxIntOption();
}

TSharedRef<SWidget> UBRSComboBoxInt::HandleGenerateWidget(TSharedPtr<FBRSComboBoxIntOption> Item) const
{
	// Call the user's delegate to see if they want to generate a custom widget bound to the data source.
	if (!IsDesignTime() && OnGenerateWidgetEvent.IsBound())
	{
		UWidget* Widget = OnGenerateWidgetEvent.Execute(*Item->ToString());
		if (Widget != NULL)
		{
			return Widget->TakeWidget();
		}
	}
	// If a row wasn't generated just create the default one, a simple text block of the item's name.
	return SNew(STextBlock)
		.Text(FText::FromString(*Item->ToString()))
		.TextStyle(CurrentTextStyle);
}

void UBRSComboBoxInt::HandleSelectionChanged(TSharedPtr<FBRSComboBoxIntOption> Item, ESelectInfo::Type SelectionType)
{
	CurrentOptionPtr = Item;
	if (!IsDesignTime())
	{
		OnSelectionChanged.Broadcast(*Item, SelectionType);
	}
	// When the selection changes we always generate another widget to represent the content area of the combobox.
	CurrentTextStyle = &TextStyle;
	ComoboBoxContent->SetContent(HandleGenerateWidget(Item));
	CurrentTextStyle = &ListTextStyle;
}

void UBRSComboBoxInt::HandleOpening()
{
	OnOpening.Broadcast();
}


#if WITH_EDITOR
//const FSlateBrush* UBRSComboBoxInt::GetEditorIcon()
//{
//	return FUMGStyle::Get().GetBrush("Widget.ComboBox");
//}

const FText UBRSComboBoxInt::GetPaletteCategory()
{
	return LOCTEXT("Input", "Input");
}
#endif


#undef LOCTEXT_NAMESPACE