// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSAnalogCursorSettings.h"

UBRSAnalogCursorSettings::UBRSAnalogCursorSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MaxAnalogCursorSpeed = 1300.0f;
	MaxAnalogCursorSpeedWhenHovered = 650.0f;
	AnalogCursorDragCoefficient = 8.0f;
	AnalogCursorDragCoefficientWhenHovered = 14.0f;
	MinAnalogCursorSpeed = 5.0f;
	AnalogCursorDeadZone = 0.15f;
	AnalogCursorAccelerationMultiplier = 9000.0f;
	AnalogCursorSize = 40.0f;

	AnalogCursorAccelerationCurve.EditorCurveData.AddKey(0, 0);
	AnalogCursorAccelerationCurve.EditorCurveData.AddKey(1, 1);
}
