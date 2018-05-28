// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "SBRSComboBox.h"
#include "Components/Widget.h"
#include "ComboBoxWidgetStyle.h"
#include "BRSComboBoxInt.generated.h"


USTRUCT(Blueprintable, BlueprintType)
struct BRSENGINE_API FBRSComboBoxIntOption
{
	GENERATED_USTRUCT_BODY()

	bool bHeader;
public:

	UPROPERTY(Category = Default, EditAnywhere, BlueprintReadOnly)
		int32 Value;

	UPROPERTY(Category = Default, EditAnywhere, BlueprintReadOnly)
		FText DisplayText;

	FBRSComboBoxIntOption() :
		Value(0),
		DisplayText()
	{}

	FBRSComboBoxIntOption(int32 inValue, const FText& inDisplayText) :
		Value(inValue),
		DisplayText(inDisplayText)
	{}

	FString ToString() const
	{
		return DisplayText.ToString();
	}

	FORCEINLINE bool operator== (const FBRSComboBoxIntOption& Val) const
	{
		return ((Value == Val.Value) && (DisplayText.ToString() == Val.DisplayText.ToString()));
	}

	FORCEINLINE bool operator== (const FString& Val) const
	{
		return (DisplayText.ToString() == Val);
	}

	FORCEINLINE bool operator== (const int32& Val) const
	{
		return (Value == Val);
	}
};

/**
 * 
 */
UCLASS()
class BRSENGINE_API UBRSComboBoxInt : public UWidget
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBRSComboBoxSelectionChangedEvent, FBRSComboBoxIntOption, SelectedItem, ESelectInfo::Type, SelectionType);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBRSComboBoxIntOpeningEvent);

public:
	UBRSComboBoxInt(const FObjectInitializer& ObjectInitializer);

	/** The style */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style", meta = (DisplayName = "Style"))
		FComboBoxStyle WidgetStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style", meta = (DisplayName = "Text Style"))
	FTextBlockStyle TextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style", meta = (DisplayName = "List Text Style"))
	FTextBlockStyle ListTextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style", meta = (DisplayName = "Lost Row Style"))
	FTableRowStyle RowStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style", meta = (DisplayName = "Foreground Color"))
	FSlateColor ForegroundColor;


	FTextBlockStyle* CurrentTextStyle;

	/** The list of items to be displayed on the combobox. */
	UPROPERTY(EditAnywhere, Category = Content)
		TArray<FBRSComboBoxIntOption> DefaultOptions;
	/** The item in the combobox to select by default */
	UPROPERTY(EditAnywhere, Category = Content)
		FBRSComboBoxIntOption SelectedOption;
	UPROPERTY(EditAnywhere, Category = Content)
		FMargin ContentPadding;
	/** The max height of the combobox list that opens */
	UPROPERTY(EditAnywhere, Category = Content, AdvancedDisplay)
		float MaxListHeight;
	/**
	* When false, the down arrow is not generated and it is up to the API consumer
	* to make their own visual hint that this is a drop down.
	*/
	UPROPERTY(EditAnywhere, Category = Content, AdvancedDisplay)
		bool HasDownArrow;
	/** Called when the widget is needed for the item. */
	UPROPERTY(EditDefaultsOnly, Category = Events)
		FGenerateWidgetForString OnGenerateWidgetEvent;
	/** Called when a new item is selected in the combobox. */
	UPROPERTY(BlueprintAssignable, Category = Events)
		FOnBRSComboBoxSelectionChangedEvent OnSelectionChanged;
	/** Called when the combobox is opening */
	UPROPERTY(BlueprintAssignable, Category = Events)
		FOnBRSComboBoxIntOpeningEvent OnOpening;
public:
	/**
	* Adds a new Option to the Combo Box
	*
	* @param Option	Option that needs to be added
	*/
	void AddOption(const FBRSComboBoxIntOption& Option);
	UFUNCTION(BlueprintCallable, Category = "BRSComboBox")
	void AddOption(const FText& DisplayText, int32 Value);

	UFUNCTION(BlueprintCallable, Category = "BRSComboBox")
	bool RemoveOption(const FBRSComboBoxIntOption& Option);

	UFUNCTION(BlueprintCallable, Category = "BRSComboBox")
	int32 FindOptionIndex(const FBRSComboBoxIntOption& Option) const;

	UFUNCTION(BlueprintCallable, Category = "BRSComboBox")
	FBRSComboBoxIntOption GetOptionAtIndex(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "BRSComboBox")
	void ClearOptions();

	UFUNCTION(BlueprintCallable, Category = "BRSComboBox")
	void SetSelectedOption(FBRSComboBoxIntOption Option);

	UFUNCTION(BlueprintCallable, Category = "BRSComboBox")
	void SetSelectedOptionByIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "BRSComboBox")
	void SetSelectedOptionByValue(int32 Value);

	UFUNCTION(BlueprintCallable, Category = "BRSComboBox")
	FBRSComboBoxIntOption GetSelectedOption() const;

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

#if WITH_EDITOR
	//virtual const FSlateBrush* GetEditorIcon() override;
	virtual const FText GetPaletteCategory() override;
#endif
protected:
	/** Called by slate when it needs to generate a new item for the combobox */
	TSharedRef<SWidget> HandleGenerateWidget(TSharedPtr<FBRSComboBoxIntOption> Item) const;
	/** Called by slate when the underlying comobobox selection changes */
	void HandleSelectionChanged(TSharedPtr<FBRSComboBoxIntOption> Item, ESelectInfo::Type SelectionType);
	/** Called by slate when the underlying comobobox is opening */
	void HandleOpening();
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface
protected:
	/** The true objects bound to the Slate combobox. */
	TArray< TSharedPtr<FBRSComboBoxIntOption> > Options;
	/** A shared pointer to the underlying slate combobox */
	TSharedPtr< SBRSComboBox< TSharedPtr<FBRSComboBoxIntOption> > > MyComboBox;
	/** A shared pointer to a container that holds the comobobox content that is selected */
	TSharedPtr< SBox > ComoboBoxContent;
	/** A shared pointer to the current selected string */
	TSharedPtr<FBRSComboBoxIntOption> CurrentOptionPtr;
};
