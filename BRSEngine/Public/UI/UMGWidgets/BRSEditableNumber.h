// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/EditableText.h"
#include "BRSEditableNumber.generated.h"

/**
 * 
 */
UCLASS()
class BRSENGINE_API UBRSEditableNumber : public UEditableText
{
	GENERATED_BODY()
	
public:
	TSharedRef<SWidget> RebuildWidget() override;
	
	virtual bool HandleTypedCharacterValid(const TCHAR InChar);

	UFUNCTION(BlueprintCallable, Category = "Editable Number", meta = (DisplayName = "Get Value"))
	virtual float GetNumberValue() const;
};
