// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "BRSGameInstance.h"
#include "BRSMessageWindowUMG.generated.h"

/**
 * 
 */
UCLASS(abstract)
class BRSENGINE_API UBRSMessageWindowUMG : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_EVENT(UBRSMessageWindowUMG, FOnButtonPressedEvent);

	UBRSMessageWindowUMG(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, Category = MessageWindow)
	FText WindowTitle;

	UPROPERTY(BlueprintReadWrite, Category = MessageWindow)
	FText Content;

	UPROPERTY(BlueprintReadWrite, Category = MessageWindow)
	FText ButtonText;
	UPROPERTY(BlueprintReadWrite, Category = MessageWindow)
	FText CancelButtonText;
	UPROPERTY(BlueprintReadWrite, Category = MessageWindow)
	TEnumAsByte<EBRSGameInstanceState::Type> NextState;

	FOnButtonPressedEvent& GetOKPressedEvent(){ return OKPressedEvent; }
	FOnButtonPressedEvent& GetCancelPressedEvent(){ return CancelPressedEvent; }


protected:
	UFUNCTION(BlueprintCallable, Category = MessageWindow)
	void OnButtonPressed();

	UFUNCTION(BlueprintCallable, Category = MessageWindow)
	void OnCancelButtonPressed();

private:
	FOnButtonPressedEvent OKPressedEvent;
	FOnButtonPressedEvent CancelPressedEvent;

};
