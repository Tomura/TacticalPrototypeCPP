// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "BRSWaitDialogUMG.generated.h"

/**
 * 
 */
UCLASS(abstract)
class BRSENGINE_API UBRSWaitDialogUMG : public UUserWidget
{
	GENERATED_BODY()

public:
	UBRSWaitDialogUMG(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, Category = MessageWindow)
	FText WindowTitle;

	UPROPERTY(BlueprintReadWrite, Category = MessageWindow)
	FText Content;
};
