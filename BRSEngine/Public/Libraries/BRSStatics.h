// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "BRSStatics.generated.h"

UCLASS()
class UBRSStatics : public UObject
{
	GENERATED_BODY()
public:
	UBRSStatics() {}

	/** Linear Interpolation between two Rotators (the one in FMath has bad results). Alpha is a float between 0.0 and 1.0 */
	static FORCEINLINE FRotator LerpRotators(const FRotator& RotA, const FRotator& RotB, float Alpha) 
	{
		const float clampedAlpha = FMath::Clamp(Alpha, 0.f, 1.f);
		return (RotA + (RotB - RotA).GetNormalized() * clampedAlpha).GetNormalized();
	}

};