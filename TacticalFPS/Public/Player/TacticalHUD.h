// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "GameFramework/HUD.h"
#include "TacticalHUD.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API ATacticalHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	ATacticalHUD(const FObjectInitializer& OI);
	
	virtual void BeginPlay() override;

	virtual void DrawHUD() override;
	
	virtual void DisplayDebug(class UCanvas* Canvas, const class FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	virtual void ShowDebug(FName DebugType /* = NAME_None */) override;

	virtual class UUserWidget* GetGameHUDWidget() const { return GameHUDWidget; }
	virtual class UTacticalPauseMenu* GetPauseMenuWidget() const { return PauseMenuWidget; }

protected:
	/** Widget Class for Main Gameplay HUD */
	UPROPERTY(Category = "Classes | HUD", EditDefaultsOnly)
	TSubclassOf<class UUserWidget> GameHUDClass;
	/** Widget Class for Pause Menu */
	UPROPERTY(Category = "Classes | HUD", EditDefaultsOnly)
	TSubclassOf<class UTacticalPauseMenu> PauseMenuClass;

private:
	/** Widget for the main Game HUD */
	UPROPERTY(Transient)
	class UUserWidget* GameHUDWidget;
	/** Widget for the Pause Menu */
	UPROPERTY(Transient)
	class UTacticalPauseMenu* PauseMenuWidget;

};
