// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "BRSCustomToolTipWidget.h"
#include "BRSCVarOptionUMG.generated.h"

/**
 * 
 */
UCLASS(abstract)
class BRSENGINE_API UBRSCVarOptionUMG : public UBRSCustomToolTipWidget
{
	GENERATED_BODY()
	
public:
	UBRSCVarOptionUMG(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = CVar)
	void SetCVarInt(FString Name, int32 Value, bool bSave);

	UFUNCTION(BlueprintCallable, Category = CVar)
	void SetCVarBool(FString Name, bool Value, bool bSave);

	UFUNCTION(BlueprintCallable, Category = CVar)
	void SetCVarFloat(FString Name, float Value, bool bSave);


	UFUNCTION(BlueprintCallable, Category = CVar)
	int32 GetCVarInt(FString Name) const;

	UFUNCTION(BlueprintCallable, Category = CVar)
	bool GetCVarBool(FString Name) const;

	UFUNCTION(BlueprintCallable, Category = CVar)
	float GetCVarFloat(FString Name) const;


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = CVar)
	void UpdateValue();
	virtual void UpdateValue_Implementation();


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = CVar)
	class UBRSSGOptionUMG* SGWidget;

protected:
	const TCHAR* SECTIONSystemSettings = TEXT("SystemSettings");

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = CVar)
	FString CVarName;
};
