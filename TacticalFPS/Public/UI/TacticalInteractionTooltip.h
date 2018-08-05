// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "TacticalInteractionTooltip.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalInteractionTooltip : public UUserWidget
{
	GENERATED_BODY()

	friend class ATacticalInteractable;

public:
	UTacticalInteractionTooltip(const FObjectInitializer& OI);

	UFUNCTION(Category = "Interaction", BlueprintCallable)
		bool SetInteractableActor(AActor* Actor);

	UFUNCTION(Category = "Interaction", BlueprintImplementableEvent)
	void OnStartHighlight(class ATacticalCharacter* InteractingCharacter);
	UFUNCTION(Category = "Interaction", BlueprintImplementableEvent)
	void OnStopHighlight(class ATacticalCharacter* InteractingCharacter);
	UFUNCTION(Category = "Interaction", BlueprintImplementableEvent)
	void OnStartInteract(class ATacticalCharacter* InteractingCharacter);
	UFUNCTION(Category = "Interaction", BlueprintImplementableEvent)
	void OnStopInteract(class ATacticalCharacter* InteractingCharacter);
	UFUNCTION(Category = "Interaction", BlueprintImplementableEvent)
	void OnInteractImmidiate(class ATacticalCharacter* InteractingCharacter);


protected:
	UPROPERTY(Category = "Interaction", BlueprintReadOnly)
	TScriptInterface<class IBRSCharacterInteractionInterface> InteractableActor;
	


	
};
