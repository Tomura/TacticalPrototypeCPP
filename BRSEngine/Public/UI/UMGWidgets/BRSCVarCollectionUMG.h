// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "BRSCustomToolTipWidget.h"
#include "BRSCVarCollectionUMG.generated.h"


/*UENUM()
enum class ECVarType : uint8
{
	Float,
	Int,
	Boolean
};

USTRUCT(BlueprintType)
struct FCVarCollection
{
	GENERATED_USTRUCT_BODY()

	FCVarCollection()
		: Type(ECVarType::Float)
		, CVarName(TEXT(""))
		, ValueLow(0.f)
		, ValueMedium(0.f)
		, ValueHigh(0.f)
		, ValueEpic(0.f)
	{}

public:
	
	UPROPERTY(Category = CVar, BlueprintReadWrite, EditAnywhere)
	FString CVarName;

	UPROPERTY(Category = CVar, BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<ECVarType> Type;

	UPROPERTY(Category = CVar, BlueprintReadWrite, EditAnywhere)
	float ValueLow;
	UPROPERTY(Category = CVar, BlueprintReadWrite, EditAnywhere)
	float ValueMedium;
	UPROPERTY(Category = CVar, BlueprintReadWrite, EditAnywhere)
	float ValueHigh;
	UPROPERTY(Category = CVar, BlueprintReadWrite, EditAnywhere)
	float ValueEpic;

	FORCEINLINE float GetValue(uint8 QualityLevel)
	{
		switch (QualityLevel)
		{
		case 0:
			return ValueLow;
		case 1:
			return ValueMedium;
		case 2:
			return ValueHigh;
		case 3:
			return ValueEpic;
		}

		return ValueEpic;
	}

	FORCEINLINE int32 AsInt(uint8 QualityLevel)
	{
		return FMath::FloorToInt(GetValue(QualityLevel));
	}
	FORCEINLINE float AsFloat(uint8 QualityLevel)
	{
		return GetValue(QualityLevel);
	}
	FORCEINLINE bool AsBool(uint8 QualityLevel)
	{
		return (GetValue(QualityLevel) > 0.f);
	}
};*/

/**
*
*/
UCLASS(abstract)
class BRSENGINE_API UBRSCVarCollectionUMG : public UBRSCustomToolTipWidget
{
	GENERATED_BODY()

public:
	UBRSCVarCollectionUMG(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = CVar)
	void UpdateValue();
	virtual void UpdateValue_Implementation();


	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = CVar)
	class UBRSSGOptionUMG* SGWidget;

	UFUNCTION(Category = CVar, BlueprintCallable)
	void SetQuality(int32 Quality);

	UFUNCTION(Category = CVar, BlueprintCallable)
	int32 GetQuality();

protected:
	void SetCVarInt(FString Name, int32 Value, bool bSave);
	void SetCVarBool(FString Name, bool Value, bool bSave);
	void SetCVarFloat(FString Name, float Value, bool bSave);

	int32 GetCVarInt(FString Name) const;
	bool GetCVarBool(FString Name) const;
	float GetCVarFloat(FString Name) const;
	
	const TCHAR* SECTIONSystemSettings = TEXT("SystemSettings");

	//UPROPERTY(Category = CVar, EditAnywhere)
	//TArray<FCVarCollection> CVars;

	UPROPERTY(Category = CVar, EditAnywhere)
	TArray<FString> CVarNames;

};
