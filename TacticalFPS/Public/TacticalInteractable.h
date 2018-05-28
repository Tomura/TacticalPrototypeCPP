// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "BRSCharacterInteractionInterface.h"
#include "TacticalInteractable.generated.h"

UCLASS()
class TACTICALFPS_API ATacticalInteractable : public AActor, public IBRSCharacterInteractionInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATacticalInteractable();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	virtual void OnStartHighlight_Implementation(class ATacticalCharacter* TriggeringCharacter) override;
	virtual void OnStopHighlight_Implementation(class ATacticalCharacter* TriggeringCharacter) override;

	virtual void OnLocalStartInteract_Implementation(class ATacticalCharacter* TriggeringCharacter) override;
	virtual void OnLocalStopInteract_Implementation(class ATacticalCharacter* TriggeringCharacter) override;
	virtual void OnLocalInteractImmidiate_Implementation(class ATacticalCharacter* TriggeringCharacter) override;


	virtual void OnServerStartInteract_Implementation(class ATacticalCharacter* TriggeringCharacter) override{}
	virtual void OnServerStopInteract_Implementation(class ATacticalCharacter* TriggeringCharacter) override{}
	virtual void OnServerInteractImmidiate_Implementation(class ATacticalCharacter* TriggeringCharacter) override{}
	virtual void OnServerSlowOpen_Implementation(class ATacticalCharacter* TriggeringCharacter) override {}
	virtual void OnServerSlowClose_Implementation(class ATacticalCharacter* TriggeringCharacter) override {}
protected:
	UPROPERTY()
	class UWidgetComponent* TooltipComp;

	UPROPERTY()
	class UTacticalInteractionTooltip* TooltipWidget;

	FTimerHandle TooltipClearTimer;

	UPROPERTY(Category = Interaction, EditAnywhere)
	float TooltipClearTime;

	void HideTooltip();
	void PlayStartHighlightAnim();
	
};
