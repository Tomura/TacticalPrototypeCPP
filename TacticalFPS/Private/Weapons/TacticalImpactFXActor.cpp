// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#include "TacticalFPS.h"
#include "TacticalImpactFXActor.h"


// Sets default values
ATacticalImpactFXActor::ATacticalImpactFXActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATacticalImpactFXActor::BeginPlay()
{
	Super::BeginPlay();

	SpawnFX();
	
}

// Called every frame
void ATacticalImpactFXActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void ATacticalImpactFXActor::SpawnFX()
{

}

