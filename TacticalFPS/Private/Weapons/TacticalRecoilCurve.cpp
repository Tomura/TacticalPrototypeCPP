// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalRecoilCurve.h"




UTacticalRecoilCurve::UTacticalRecoilCurve(const FObjectInitializer& OI)
{
	
}

FVector UTacticalRecoilCurve::GetVectorValue(float InTime) const
{
	FVector Result;
	Result.X = FloatCurves[0].Eval(InTime);
	Result.Y = FloatCurves[1].Eval(InTime);
	Result.Z = FloatCurves[2].Eval(InTime);
	return Result;
}

FRotator UTacticalRecoilCurve::GetRotationValue(float InTime) const
{
	return FRotator(FloatCurves[3].Eval(InTime), FloatCurves[4].Eval(InTime), FloatCurves[5].Eval(InTime));
}

void UTacticalRecoilCurve::GetVectorAndRotationValue(float InTime, FVector& OutVector, FRotator& OutRotator) const
{
	OutVector.X = FloatCurves[0].Eval(InTime);
	OutVector.Y = FloatCurves[1].Eval(InTime);
	OutVector.Z = FloatCurves[2].Eval(InTime);
	OutRotator = FRotator(FloatCurves[3].Eval(InTime), FloatCurves[4].Eval(InTime), FloatCurves[5].Eval(InTime));
}

float UTacticalRecoilCurve::GetDuration() const
{
	float MaxDuration = 0.f;
	for (int32 i = 0; i < 6; ++i)
	{
		const float TestTime = FloatCurves[i].GetLastKey().Time;
		if (TestTime > MaxDuration)
		{
			MaxDuration = TestTime;
		}
	}
	return MaxDuration;
}

static const FName XCurveName(TEXT("X"));
static const FName YCurveName(TEXT("Y"));
static const FName ZCurveName(TEXT("Z"));
static const FName PitchCurveName(TEXT("Pitch"));
static const FName YawCurveName(TEXT("Yaw"));
static const FName RollCurveName(TEXT("Roll"));

TArray<FRichCurveEditInfoConst> UTacticalRecoilCurve::GetCurves() const
{
	TArray<FRichCurveEditInfoConst> Curves;
	Curves.Add(FRichCurveEditInfoConst(&FloatCurves[0], XCurveName));
	Curves.Add(FRichCurveEditInfoConst(&FloatCurves[1], YCurveName));
	Curves.Add(FRichCurveEditInfoConst(&FloatCurves[2], ZCurveName));
	Curves.Add(FRichCurveEditInfoConst(&FloatCurves[3], PitchCurveName));
	Curves.Add(FRichCurveEditInfoConst(&FloatCurves[4], YawCurveName));
	Curves.Add(FRichCurveEditInfoConst(&FloatCurves[5], RollCurveName));
	return Curves;
}

TArray<FRichCurveEditInfo> UTacticalRecoilCurve::GetCurves()
{
	TArray<FRichCurveEditInfo> Curves;
	Curves.Add(FRichCurveEditInfo(&FloatCurves[0], XCurveName));
	Curves.Add(FRichCurveEditInfo(&FloatCurves[1], YCurveName));
	Curves.Add(FRichCurveEditInfo(&FloatCurves[2], ZCurveName));
	Curves.Add(FRichCurveEditInfo(&FloatCurves[3], PitchCurveName));
	Curves.Add(FRichCurveEditInfo(&FloatCurves[4], YawCurveName));
	Curves.Add(FRichCurveEditInfo(&FloatCurves[5], RollCurveName));

	return Curves;
}

FLinearColor UTacticalRecoilCurve::GetCurveColor(FRichCurveEditInfo CurveInfo) const
{
	const FString CurveName = CurveInfo.CurveName.ToString();

	if (CurveName == TEXT("X") || CurveName == TEXT("R"))
	{
		return FLinearColor(1.0f, 0.0f, 0.0f);
	}

	if (CurveName == TEXT("Y") || CurveName == TEXT("G"))
	{
		return FLinearColor(0.0f, 1.0f, 0.0f);
	}

	if (CurveName == TEXT("Z") || CurveName == TEXT("B"))
	{
		return FLinearColor(0.05f, 0.05f, 1.0f);
	}

	if (CurveName == TEXT("Pitch"))
	{
		return FLinearColor(1.0f, 0.1f, 0.0f);
	}	
	if (CurveName == TEXT("Yaw"))
	{
		return FLinearColor(0.0f, 1.0f, 0.2f);
	}
	if (CurveName == TEXT("Roll"))
	{
		return FLinearColor(0.05f, 0.35f, 1.0f);
	}
	return FLinearColor::Gray;
}

bool UTacticalRecoilCurve::IsValidCurve(FRichCurveEditInfo CurveInfo)
{
	return CurveInfo.CurveToEdit == &FloatCurves[0] ||
		CurveInfo.CurveToEdit == &FloatCurves[1] ||
		CurveInfo.CurveToEdit == &FloatCurves[2] ||
		CurveInfo.CurveToEdit == &FloatCurves[3] ||
		CurveInfo.CurveToEdit == &FloatCurves[4] ||
		CurveInfo.CurveToEdit == &FloatCurves[5];
}

bool UTacticalRecoilCurve::operator==(const UTacticalRecoilCurve& Curve) const
{
	return (FloatCurves[0] == Curve.FloatCurves[0]) &&
		(FloatCurves[1] == Curve.FloatCurves[1]) && 
		(FloatCurves[2] == Curve.FloatCurves[2]) &&
		(FloatCurves[3] == Curve.FloatCurves[3]) &&
		(FloatCurves[4] == Curve.FloatCurves[4]) &&
		(FloatCurves[5] == Curve.FloatCurves[5]);
}
