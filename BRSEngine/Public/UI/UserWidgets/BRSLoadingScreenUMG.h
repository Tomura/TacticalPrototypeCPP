// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "BRSLoadingScreenUMG.generated.h"

/**
 * 
 */
UCLASS(abstract)
class BRSENGINE_API UBRSLoadingScreenUMG : public UUserWidget
{
	GENERATED_BODY()

public:
	DECLARE_EVENT(UBRSLoadingScreenUMG, FOnLoadingScreenVisibleEvent);
	UBRSLoadingScreenUMG(const FObjectInitializer& ObjectInitializer);
	FOnLoadingScreenVisibleEvent& OnLoadingScreenVisible(){ return LoadingScreenVisibleEvent; }

	UFUNCTION(BlueprintNativeEvent, Category = LoadingScreen)
	void HideLoadingScreen();
	virtual void HideLoadingScreen_Implementation();

	UPROPERTY(BlueprintReadWrite, Category = LoadingScreen)
	FString LoadingScreenTitle;

private:
	FOnLoadingScreenVisibleEvent LoadingScreenVisibleEvent;

protected:
	UFUNCTION(BlueprintCallable, Category = LoadingScreen)
	void LoadingScreenVisible();
};
