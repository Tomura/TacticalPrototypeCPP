// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "TacticalPauseMenu.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalPauseMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(Category = PauseMenu, BlueprintImplementableEvent)
	void OpenMenu(bool bOpen);
	
	UFUNCTION(Category = PauseMenu, BlueprintImplementableEvent)
	void ToggleMenu();

	UFUNCTION(Category = PauseMenu, BlueprintNativeEvent)
	UOverlay* GetMainOverlay() const;
	virtual UOverlay* GetMainOverlay_Implementation() const { return nullptr; }


	UFUNCTION(Category = PauseMenu, BlueprintImplementableEvent)
	void OpenChat(bool bTeamChat);

	UFUNCTION(Category = PauseMenu, BlueprintNativeEvent)
		bool IsInChat() const;
	virtual bool IsInChat_Implementation() const { return false; }
	
	UFUNCTION(Category = PauseMenu, BlueprintImplementableEvent)
	void FocusChat();


	ALevelScriptActor* GetLevelScriptActor() const;

	UFUNCTION(Category = PauseMenu, BlueprintCallable)
	void LevelScriptUIEvent(FName EventName);

protected:
	UPROPERTY(Category = PauseMenu, BlueprintReadWrite)
	bool bIsOpen;
};
