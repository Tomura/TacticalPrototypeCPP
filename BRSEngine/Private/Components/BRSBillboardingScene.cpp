// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "BRSBillboardingScene.h"


// Sets default values for this component's properties
UBRSBillboardingScene::UBRSBillboardingScene()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UBRSBillboardingScene::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UBRSBillboardingScene::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
	APlayerController* PC = GetWorld()->GetFirstPlayerController(); // this needs to be better to find the actual player looking at the object
	FVector CamLoc;
	FRotator CamRot;
	PC->PlayerCameraManager->GetCameraViewPoint(CamLoc, CamRot);
	
}

