// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalImpactFX.h"
#include "TacticalImpactFXComponent.h"




// Sets default values for this component's properties
UTacticalImpactFXComponent::UTacticalImpactFXComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	ImpactParticle = nullptr;
	ImpactSound = nullptr;
	VolumeModifier = 1.f;
	PitchModifier = 1.f;
	// ...
}


// Called when the game starts
void UTacticalImpactFXComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTacticalImpactFXComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

bool UTacticalImpactFXComponent::PlayImpactSound(const FHitResult& Hit)
{
	if (OverrideSound() && ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactSound, Hit.ImpactPoint, Hit.ImpactNormal.Rotation(), VolumeModifier, PitchModifier);
		return true;
	}
	return false;
}

UParticleSystemComponent* UTacticalImpactFXComponent::SpawnImpactParticle(const FHitResult& Hit, const FVector& ShotDir)
{
	if (OverrideParticle() && ImpactParticle)
	{
		const FVector SpawnLoc = Hit.ImpactPoint;
		const FRotator SpawnRot = Hit.ImpactNormal.Rotation();
		const FRotator ImpactRot = (ShotDir - 2 * (Hit.ImpactNormal | ShotDir) * Hit.ImpactNormal).Rotation();	
		UParticleSystemComponent* NewParticle = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, SpawnLoc, ImpactRot, true);

		if (NewParticle && OnSpawnImpactParticle.IsBound())
		{
			OnSpawnImpactParticle.Broadcast(NewParticle, Hit, ShotDir);
		}
		return NewParticle;
	}
	return nullptr;
}

UDecalComponent* UTacticalImpactFXComponent::SpawnImpactDecal(const FHitResult& Hit)
{
	if (OverrideDecal() && ImpactDecal)
	{
		UDecalComponent* SpawnedDecal = ATacticalImpactFX::SpawnImpactDecal(ImpactDecal, Hit, DecalSize, 0.f);
	}
	return nullptr;
}
