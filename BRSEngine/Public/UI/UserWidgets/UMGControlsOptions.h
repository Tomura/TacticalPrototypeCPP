// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "UMGControlsOptions.generated.h"



/**
 * 
 */
UCLASS(abstract)
class BRSENGINE_API UUMGControlsOptions : public UUserWidget
{
	GENERATED_BODY()

public:
	UUMGControlsOptions(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "KeyBinds")
	void ResetKeyBindsToDefaults();

	UFUNCTION(BlueprintCallable, Category = "Mouse")
	float GetMouseSensitivity() const;

	UFUNCTION(BlueprintCallable, Category = "Mouse")
	void SetMouseSettings(float NewSensitivity, bool bInvertY);

	UFUNCTION(BlueprintCallable, Category = "Mouse")
	bool GetInvertMouseYAxis() const;


	UFUNCTION(BlueprintCallable, Category = "Gamepad")
	float GetAnalogStickSensitivity(bool bLeftStick) const;

	UFUNCTION(BlueprintCallable, Category = "Gamepad")
	float GetAnalogStickExponent(bool bLeftStick) const;

	UFUNCTION(BlueprintCallable, Category = "Gamepad")
	bool GetInvertAnalogStickYAxis(bool bLeftStick) const;

	UFUNCTION(BlueprintCallable, Category = "Gamepad")
	void SetAnalogStickSettings(float NewSensitivity, bool bInvertY, float Exponent, bool bLeftStick);

	UFUNCTION(BlueprintCallable, Category = "Gamepad")
	float GetAnalogStickDeadzone() const;

	UFUNCTION(BlueprintCallable, Category = "Gamepad")
	void SetAnalogStickDeadzone(float Deadzone);

};
