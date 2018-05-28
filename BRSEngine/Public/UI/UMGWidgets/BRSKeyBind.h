// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Components/Widget.h"
#include "BRSKeyBind.generated.h"


/**
 * 
 */
UCLASS()
class BRSENGINE_API UBRSKeyBind : public UWidget
{
	GENERATED_BODY()

public:
	UBRSKeyBind(const FObjectInitializer& ObjectInitializer);

	/** The button style used at runtime */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style", meta = (DisplayName = "ButtonStyle"))
	FButtonStyle WidgetStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style", meta = (DisplayName = "TextStyle"))
	FTextBlockStyle TextStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style", meta = (DisplayName = "Fireground Color"))
	FSlateColor ForegroundColor;

	// UVisual interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	// End of UVisual interface

	// UWidget interface
	virtual void SynchronizeProperties() override;

#if WITH_EDITOR
//	virtual const FSlateBrush* GetEditorIcon() override;
	virtual const FText GetPaletteCategory() override;
#endif
	
	void BindToKey(TSharedPtr<FKey> NewKey);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

public:
	TSharedPtr<class SKeyBind> MyKeyBind;
};


