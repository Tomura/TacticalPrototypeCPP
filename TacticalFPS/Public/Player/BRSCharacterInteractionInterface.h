// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "BRSCharacterInteractionInterface.generated.h"

UINTERFACE(MinimalAPI)
class UBRSCharacterInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

class IBRSCharacterInteractionInterface
{
	GENERATED_IINTERFACE_BODY()

public:
	UFUNCTION(Category = Interaction, BlueprintNativeEvent)
	void OnServerStartInteract(class ATacticalCharacter* TriggeringCharacter);

	UFUNCTION(Category = Interaction, BlueprintNativeEvent)
	void OnServerStopInteract(class ATacticalCharacter* TriggeringCharacter);

	UFUNCTION(Category = Interaction, BlueprintNativeEvent)
	void OnServerInteractImmidiate(class ATacticalCharacter* TriggeringCharacter);

	UFUNCTION(Category = Interaction, BlueprintNativeEvent)
	void OnLocalStartInteract(class ATacticalCharacter* TriggeringCharacter);

	UFUNCTION(Category = Interaction, BlueprintNativeEvent)
	void OnLocalStopInteract(class ATacticalCharacter* TriggeringCharacter);

	UFUNCTION(Category = Interaction, BlueprintNativeEvent)
	void OnLocalInteractImmidiate(class ATacticalCharacter* TriggeringCharacter);



	UFUNCTION(Category = Interaction, BlueprintNativeEvent)
	void OnServerSlowOpen(class ATacticalCharacter* TriggeringCharacter);

	UFUNCTION(Category = Interaction, BlueprintNativeEvent)
	void OnServerSlowClose(class ATacticalCharacter* TriggeringCharacter);

	UFUNCTION(Category = Interaction, BlueprintNativeEvent)
	void OnStartHighlight(class ATacticalCharacter* TriggeringCharacter);

	UFUNCTION(Category = Interaction, BlueprintNativeEvent)
	void OnStopHighlight(class ATacticalCharacter* TriggeringCharacter);

	UFUNCTION(Category = Interaction, BlueprintImplementableEvent, BlueprintCallable)
	void GetActionText(FText& OutText);
};