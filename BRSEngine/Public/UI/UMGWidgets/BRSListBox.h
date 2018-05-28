// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Widgets/Views/SListView.h"
#include "Components/Widget.h"
//#include "ComboBoxWidgetStyle.h"
#include "BRSListBox.generated.h"

UCLASS()
class UBRSListBox : public UWidget
{
	GENERATED_BODY()
public:
	UBRSListBox(const FObjectInitializer& ObjectInitializer);

protected:
	TSharedPtr<SListView<TSharedPtr<FString> > > MyList;
};

