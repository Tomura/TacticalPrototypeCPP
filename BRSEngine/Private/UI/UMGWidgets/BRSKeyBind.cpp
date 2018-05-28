// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "SlateWidgets/SKeyBind.h"
#include "BRSKeyBind.h"


#define LOCTEXT_NAMESPACE "UMG"

UBRSKeyBind::UBRSKeyBind(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SKeyBind::FArguments KeyBindDefaults;
	WidgetStyle = *KeyBindDefaults._ButtonStyle;
	TextStyle = *KeyBindDefaults._TextStyle;
	ForegroundColor = ForegroundColor = FSlateColor(FLinearColor::Black);
}



TSharedRef<SWidget> UBRSKeyBind::RebuildWidget()
{
	MyKeyBind = SNew(SKeyBind)
		.ButtonStyle(&WidgetStyle)
		.TextStyle(&TextStyle);

	return MyKeyBind.ToSharedRef(); // BuildDesignTimeWidget(MyKeyBind.ToSharedRef());
}

void UBRSKeyBind::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyKeyBind.Reset();
}

void UBRSKeyBind::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	MyKeyBind->SetForegroundColor(ForegroundColor);
}

void UBRSKeyBind::BindToKey(TSharedPtr<FKey> NewKey)
{
	MyKeyBind->BindKeyPtr(NewKey);
}



#if WITH_EDITOR
//const FSlateBrush* UBRSKeyBind::GetEditorIcon()
//{
//	return FUMGStyle::Get().GetBrush("Widget.Button");
//}
const FText UBRSKeyBind::GetPaletteCategory()
{
	return LOCTEXT("Common", "Common");
}
#endif

/////////////////////////////////////////////////////
#undef LOCTEXT_NAMESPACE