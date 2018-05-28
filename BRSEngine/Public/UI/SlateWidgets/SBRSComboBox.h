// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "SlateBasics.h"


DECLARE_DELEGATE(FOnBRSComboBoxOpening)


template<typename OptionType>
class SBRSComboRow : public STableRow< OptionType >
{
public:

	SLATE_BEGIN_ARGS(SBRSComboRow)
		: _Content()
		, _RowStyle(&FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row"))
	{}

	SLATE_DEFAULT_SLOT(FArguments, Content)
	SLATE_STYLE_ARGUMENT(FTableRowStyle, RowStyle)
		SLATE_END_ARGS()

public:

	/**
	* Constructs this widget.
	*/
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable)
	{
		STableRow< OptionType >::Construct(
			typename STableRow<OptionType>::FArguments()
			.Style(InArgs._RowStyle)
			.Content()
			[
				InArgs._Content.Widget
			]
		, InOwnerTable
			);
	}

	// handle case where user clicks on an existing selected item
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
	{
		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			TSharedPtr< ITypedTableView<OptionType> > OwnerWidget = this->OwnerTablePtr.Pin();

			const OptionType* MyItem = OwnerWidget->Private_ItemFromWidget(this);
			const bool bIsSelected = OwnerWidget->Private_IsItemSelected(*MyItem);

			if (bIsSelected)
			{
				// Reselect content to ensure selection is taken
				OwnerWidget->Private_SignalSelectionChanged(ESelectInfo::Direct);
				return FReply::Handled();
			}
		}
		return STableRow<OptionType>::OnMouseButtonDown(MyGeometry, MouseEvent);
	}
};


/**
* A combo box that shows arbitrary content.
*/
template< typename OptionType >
class SBRSComboBox : public SComboButton
{
public:
	/** Type of list used for showing menu options. */
	typedef SListView< OptionType > SComboListType;
	/** Delegate type used to generate widgets that represent Options */
	typedef typename TSlateDelegates< OptionType >::FOnGenerateWidget FOnGenerateWidget;
	typedef typename TSlateDelegates< OptionType >::FOnSelectionChanged FOnSelectionChanged;

	SLATE_BEGIN_ARGS(SBRSComboBox)
		: _Content()
		, _ComboBoxStyle(&FCoreStyle::Get().GetWidgetStyle< FComboBoxStyle >("ComboBox"))
		, _ButtonStyle(nullptr)
		, _RowStyle(&FCoreStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row"))
		, _ContentPadding(FMargin(4.0, 2.0))
		, _ForegroundColor(FCoreStyle::Get().GetSlateColor("InvertedForeground"))
		, _OptionsSource()
		, _OnSelectionChanged()
		, _OnGenerateWidget()
		, _InitiallySelectedItem(nullptr)
		, _Method()
		, _MaxListHeight(450.0f)
		, _HasDownArrow(true)
	{}

	/** Slot for this button's content (optional) */
	SLATE_DEFAULT_SLOT(FArguments, Content)

		SLATE_STYLE_ARGUMENT(FComboBoxStyle, ComboBoxStyle)

		/** The visual style of the button part of the combo box (overrides ComboBoxStyle) */
		SLATE_STYLE_ARGUMENT(FButtonStyle, ButtonStyle)

		SLATE_STYLE_ARGUMENT(FTableRowStyle, RowStyle)

		SLATE_ATTRIBUTE(FMargin, ContentPadding)
		SLATE_ATTRIBUTE(FSlateColor, ForegroundColor)

		SLATE_ARGUMENT(const TArray< OptionType >*, OptionsSource)
		SLATE_EVENT(FOnSelectionChanged, OnSelectionChanged)
		SLATE_EVENT(FOnGenerateWidget, OnGenerateWidget)

		/** Called when combo box is opened, before list is actually created */
		SLATE_EVENT(FOnBRSComboBoxOpening, OnComboBoxOpening)

		/** The option that should be selected when the combo box is first created */
		SLATE_ARGUMENT(OptionType, InitiallySelectedItem)

		SLATE_ARGUMENT(TOptional<EPopupMethod>, Method)

		/** The max height of the combo box menu */
		SLATE_ARGUMENT(float, MaxListHeight)

		/** The sound to play when the button is pressed (overrides ComboBoxStyle) */
		SLATE_ARGUMENT(TOptional<FSlateSound>, PressedSoundOverride)

		/** The sound to play when the selection changes (overrides ComboBoxStyle) */
		SLATE_ARGUMENT(TOptional<FSlateSound>, SelectionChangeSoundOverride)

		/**
		* When false, the down arrow is not generated and it is up to the API consumer
		* to make their own visual hint that this is a drop down.
		*/
		SLATE_ARGUMENT(bool, HasDownArrow)

		SLATE_END_ARGS()

		/**
		* Construct the widget from a declaration
		*
		* @param InArgs   Declaration from which to construct the combo box
		*/
		void Construct(const FArguments& InArgs)
	{
		check(InArgs._ComboBoxStyle);
		check(InArgs._RowStyle);

		// Work out which values we should use based on whether we were given an override, or should use the style's version
		const FComboButtonStyle& OurComboButtonStyle = InArgs._ComboBoxStyle->ComboButtonStyle;
		const FButtonStyle* const OurButtonStyle = InArgs._ButtonStyle ? InArgs._ButtonStyle : &OurComboButtonStyle.ButtonStyle;
		
		RowStyle = InArgs._RowStyle;

		PressedSound = InArgs._PressedSoundOverride.Get(InArgs._ComboBoxStyle->PressedSlateSound);
		SelectionChangeSound = InArgs._SelectionChangeSoundOverride.Get(InArgs._ComboBoxStyle->SelectionChangeSlateSound);

		this->OnComboBoxOpening = InArgs._OnComboBoxOpening;
		this->OnSelectionChanged = InArgs._OnSelectionChanged;
		this->OnGenerateWidget = InArgs._OnGenerateWidget;

		OptionsSource = InArgs._OptionsSource;

		TSharedRef<SWidget> ComboBoxMenuContent =
			SNew(SBox)
			.MaxDesiredHeight(InArgs._MaxListHeight)
			[
				SAssignNew(this->ComboListView, SComboListType)
				.ListItemsSource(InArgs._OptionsSource)
				.OnGenerateRow(this, &SBRSComboBox< OptionType >::GenerateMenuItemRow)
				.OnSelectionChanged(this, &SBRSComboBox< OptionType >::OnSelectionChanged_Internal)
				.SelectionMode(ESelectionMode::Single)
			];

		// Set up content
		TSharedPtr<SWidget> ButtonContent = InArgs._Content.Widget;
		if (InArgs._Content.Widget == SNullWidget::NullWidget)
		{
			SAssignNew(ButtonContent, STextBlock)
				.Text(NSLOCTEXT("SBRSComboBox", "ContentWarning", "No Content Provided"))
				.ColorAndOpacity(FLinearColor::Red);
		}


		SComboButton::Construct(SComboButton::FArguments()
			.ComboButtonStyle(&OurComboButtonStyle)
			.ButtonStyle(OurButtonStyle)
			.Method(InArgs._Method)
			.ButtonContent()
			[
				ButtonContent.ToSharedRef()
			]
		.MenuContent()
			[
				ComboBoxMenuContent
			]
		.HasDownArrow(InArgs._HasDownArrow)
			.ContentPadding(InArgs._ContentPadding)
			.ForegroundColor(InArgs._ForegroundColor)
			);
		SetMenuContentWidgetToFocus(ComboListView);

		// Need to establish the selected item at point of construction so its available for querying
		// NB: If you need a selection to fire use SetItemSelection rather than setting an IntiallySelectedItem
		this->SelectedItem = InArgs._InitiallySelectedItem;
		if (TListTypeTraits<OptionType>::IsPtrValid(InArgs._InitiallySelectedItem))
		{
			ComboListView->Private_SetItemSelection(SelectedItem, true);
		}
	}

	void ClearSelection()
	{
		ComboListView->ClearSelection();
	}

	void SetSelectedItem(OptionType InSelectedItem)
	{
		ComboListView->SetSelection(InSelectedItem);
	}

	/** @return the item currently selected by the combo box. */
	OptionType GetSelectedItem()
	{
		return SelectedItem;
	}

	/**
	* Requests a list refresh after updating options
	* Call SetSelectedItem to update the selected item if required
	* @see SetSelectedItem
	*/
	void RefreshOptions()
	{
		ComboListView->RequestListRefresh();
	}

protected:
	/** Handle key presses that SListView ignores */
	virtual FReply OnHandleKeyPressed(FKey KeyPressed)
	{
		if (KeyPressed == EKeys::Enter)
		{
			TArray<OptionType> SelectedItems = ComboListView->GetSelectedItems();
			if (SelectedItems.Num() > 0)
			{
				ComboListView->SetSelection(SelectedItems[0]);
			}
			return FReply::Handled();
		}
		else if (KeyPressed == EKeys::Escape)
		{
			this->SetIsOpen(false);
			return FReply::Handled();
		}
		return FReply::Unhandled();
	}

	FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override
	{
		const FKey Key = InKeyEvent.GetKey();

		if (Key == EKeys::Up)
		{
			const int32 SelectionIndex = OptionsSource->Find(GetSelectedItem());
			if (SelectionIndex >= 1)
			{
				// Select an item on the prev row
				SetSelectedItem((*OptionsSource)[SelectionIndex - 1]);
			}

			return FReply::Handled();
		}
		else if (Key == EKeys::Down)
		{
			const int32 SelectionIndex = OptionsSource->Find(GetSelectedItem());
			if (SelectionIndex < OptionsSource->Num() - 1)
			{
				// Select an item on the next row
				SetSelectedItem((*OptionsSource)[SelectionIndex + 1]);
			}

			return FReply::Handled();
		}

		return SComboButton::OnKeyDown(MyGeometry, InKeyEvent);
	}

	virtual bool SupportsKeyboardFocus() const override
	{
		return true;
	}

private:

	/** Generate a row for the InItem in the combo box's list (passed in as OwnerTable). Do this by calling the user-specified OnGenerateWidget */
	TSharedRef<ITableRow> GenerateMenuItemRow(OptionType InItem, const TSharedRef<STableViewBase>& OwnerTable)
	{
		if (OnGenerateWidget.IsBound())
		{
			return SNew(SBRSComboRow<OptionType>, OwnerTable)
				.RowStyle(RowStyle)
				[
					OnGenerateWidget.Execute(InItem)
				];
		}
		else
		{
			return SNew(SBRSComboRow<OptionType>, OwnerTable)
				.RowStyle(RowStyle)
				[
					SNew(STextBlock).Text(NSLOCTEXT("SlateCore", "ComboBoxMissingOnGenerateWidgetMethod", "Please provide a .OnGenerateWidget() handler."))
				];

		}
	}

	/** Invoked when the selection in the list changes */
	void OnSelectionChanged_Internal(OptionType ProposedSelection, ESelectInfo::Type SelectInfo)
	{
		// Ensure that the proposed selection is different
		if (SelectInfo != ESelectInfo::OnNavigation)
		{
			// Ensure that the proposed selection is different from selected
			if (ProposedSelection != SelectedItem)
			{
				PlaySelectionChangeSound();
				SelectedItem = ProposedSelection;
				OnSelectionChanged.ExecuteIfBound(ProposedSelection, SelectInfo);
			}
			// close combo even if user reselected item
			this->SetIsOpen(false);
		}
	}

	/** Handle clicking on the content menu */
	virtual FReply OnButtonClicked() override
	{
		// if user clicked to close the combo menu
		if (this->IsOpen())
		{
			// Re-select first selected item, just in case it was selected by navigation previously
			TArray<OptionType> SelectedItems = ComboListView->GetSelectedItems();
			if (SelectedItems.Num() > 0)
			{
				OnSelectionChanged_Internal(SelectedItems[0], ESelectInfo::Direct);
			}
		}
		else
		{
			PlayPressedSound();
			OnComboBoxOpening.ExecuteIfBound();
		}

		return SComboButton::OnButtonClicked();
	}

	/** Play the pressed sound */
	void PlayPressedSound() const
	{
		FSlateApplication::Get().PlaySound(PressedSound);
	}

	/** Play the selection changed sound */
	void PlaySelectionChangeSound() const
	{
		FSlateApplication::Get().PlaySound(SelectionChangeSound);
	}

	/** The Sound to play when the button is pressed */
	FSlateSound PressedSound;

	/** The Sound to play when the selection is changed */
	FSlateSound SelectionChangeSound;

	const FTableRowStyle* RowStyle;

private:
	/** Delegate that is invoked when the selected item in the combo box changes */
	FOnSelectionChanged OnSelectionChanged;
	/** The item currently selected in the combo box */
	OptionType SelectedItem;
	/** The ListView that we pop up; visualized the available options. */
	TSharedPtr< SComboListType > ComboListView;
	/** Delegate to invoke before the combo box is opening. */
	FOnBRSComboBoxOpening OnComboBoxOpening;
	/** Delegate to invoke when we need to visualize an option as a widget. */
	FOnGenerateWidget OnGenerateWidget;

	const TArray< OptionType >* OptionsSource;
};