// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Components/SceneComponent.h"
#include "BRSBillboardingScene.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Experimental)
class BRSENGINE_API UBRSBillboardingScene : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBRSBillboardingScene();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

		
	
};
