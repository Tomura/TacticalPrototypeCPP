// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Curves/CurveBase.h"
#include "TacticalRecoilCurve.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalRecoilCurve : public UCurveBase
{
	GENERATED_BODY()
	
public:
	UTacticalRecoilCurve(const FObjectInitializer& OI);

	/** Keyframe data, one curve for X, Y, Z, Pitch, Yaw, Roll */
	UPROPERTY()
	FRichCurve FloatCurves[6];



	/** Evaluate this float curve at the specified time */
	UFUNCTION(BlueprintCallable, Category = "Math|Curves")
	FVector GetVectorValue(float InTime) const;
	/** Evaluate this float curve at the specified time */
	UFUNCTION(BlueprintCallable, Category = "Math|Curves")
	FRotator GetRotationValue(float InTime) const;

	UFUNCTION(BlueprintCallable, Category = "Math|Curves")
	void GetVectorAndRotationValue(float InTime, FVector& OutVector, FRotator& OutRotator) const;

	float GetDuration() const;

	// Begin FCurveOwnerInterface
	virtual TArray<FRichCurveEditInfoConst> GetCurves() const override;
	virtual TArray<FRichCurveEditInfo> GetCurves() override;

	/** @return Color for this curve */
	virtual FLinearColor GetCurveColor(FRichCurveEditInfo CurveInfo) const override;

	/** Determine if Curve is the same */
	bool operator == (const UTacticalRecoilCurve& Curve) const;

	virtual bool IsValidCurve(FRichCurveEditInfo CurveInfo) override;
	
	
};
