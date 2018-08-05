// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "Components/WidgetComponent.h"
#include "TacticalInteractionTooltip.h"
#include "TacticalInteractable.h"
#include "TacticalCharacter.h"


// Sets default values
ATacticalInteractable::ATacticalInteractable()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	TooltipWidget = nullptr;
	TooltipClearTime = 0.5f;
}

// Called when the game starts or when spawned
void ATacticalInteractable::BeginPlay()
{
	Super::BeginPlay();
	
	TArray<UWidgetComponent*> WidgetComps;
	GetComponents<UWidgetComponent>(WidgetComps);
	for (UWidgetComponent* TestComp : WidgetComps)
	{
		if (TestComp && TestComp->IsValidLowLevelFast())
		{
			if (TestComp->GetWidgetClass() && TestComp->GetUserWidgetObject())
			{
				TooltipComp = TestComp;
				UTacticalInteractionTooltip* Widget = Cast<UTacticalInteractionTooltip>(TestComp->GetUserWidgetObject());
				if (Widget)
				{
					TooltipWidget = Widget;
					TooltipWidget->InteractableActor.SetObject(this);
					TooltipWidget->InteractableActor.SetInterface(this);
					//UE_LOG(LogTemp, Log, TEXT("Widget Found"));
				}
			}
		}
	}
}

// Called every frame
void ATacticalInteractable::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void ATacticalInteractable::OnStartHighlight_Implementation(class ATacticalCharacter* TriggeringCharacter)
{
	if (TooltipComp)
	{
		GetWorldTimerManager().ClearTimer(TooltipClearTimer);
		TooltipComp->SetVisibility(true);
		UE_LOG(LogTemp, Log, TEXT("Make Tooltip visible"));
	}
	GetWorldTimerManager().SetTimerForNextTick(this, &ATacticalInteractable::PlayStartHighlightAnim);
}

void ATacticalInteractable::OnStopHighlight_Implementation(class ATacticalCharacter* TriggeringCharacter)
{
	GetWorldTimerManager().SetTimer(TooltipClearTimer, this, &ATacticalInteractable::HideTooltip, TooltipClearTime, false);
	if (TooltipWidget)
	{
		TooltipWidget->OnStopHighlight(TriggeringCharacter);
	}
}
void ATacticalInteractable::HideTooltip()
{
	if (TooltipComp)
	{
		UE_LOG(LogTemp, Log, TEXT("Make Tooltip invisible"));
		TooltipComp->SetVisibility(false);
	}
}

void ATacticalInteractable::PlayStartHighlightAnim()
{
	if (TooltipWidget)
	{
		TooltipWidget->OnStartHighlight(nullptr);
	}
}

void ATacticalInteractable::OnLocalStartInteract_Implementation(class ATacticalCharacter* TriggeringCharacter)
{
	if (TooltipWidget)
	{
		TooltipWidget->OnStartInteract(TriggeringCharacter);
	}
}

void ATacticalInteractable::OnLocalStopInteract_Implementation(class ATacticalCharacter* TriggeringCharacter)
{
	if (TooltipWidget)
	{
		TooltipWidget->OnStopInteract(TriggeringCharacter);
	}
}

void ATacticalInteractable::OnLocalInteractImmidiate_Implementation(class ATacticalCharacter* TriggeringCharacter)
{
	if (TooltipWidget)
	{
		TooltipWidget->OnInteractImmidiate(TriggeringCharacter);
	}
}


