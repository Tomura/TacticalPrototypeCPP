// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalProjectile.h"
#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Touch.h"
#include "Perception/AISense_Team.h"


// Sets default values
ATacticalProjectile::ATacticalProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	ProjectileCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ProjectileCollision"));
	RootComponent = ProjectileCollision;
	ProjectileCollision->SetSphereRadius(16.f);
	
	MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	MovementComponent->SetUpdatedComponent(ProjectileCollision);

	SetReplicates(true);
	SetReplicateMovement(true);

	bAIShouldRun = false;
}
// Called when the game starts or when spawned
void ATacticalProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (bAIShouldRun)
	{
		UAIPerceptionSystem::RegisterPerceptionStimuliSource(this, UAISense_Sight::StaticClass(), this);
		UAIPerceptionSystem::RegisterPerceptionStimuliSource(this, UAISense_Hearing::StaticClass(), this);
		UAIPerceptionSystem::RegisterPerceptionStimuliSource(this, UAISense_Touch::StaticClass(), this);
		UAIPerceptionSystem::RegisterPerceptionStimuliSource(this, UAISense_Team::StaticClass(), this);
	}
}


// Called every frame
void ATacticalProjectile::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

