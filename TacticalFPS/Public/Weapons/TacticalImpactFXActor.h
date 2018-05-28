// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "TacticalImpactFXActor.generated.h"

UCLASS()
class TACTICALFPS_API ATacticalImpactFXActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATacticalImpactFXActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	
	virtual void SpawnFX();


	UPROPERTY(Category = FX, BlueprintReadOnly, EditDefaultsOnly)
	UParticleSystem* ParticleSystem;
	UPROPERTY(Category = FX, BlueprintReadOnly, EditDefaultsOnly)
	USoundCue* ImpactSound;
	UPROPERTY(Category = FX, BlueprintReadOnly, EditDefaultsOnly)
	UMaterialInterface* ImpactDecal;
	UPROPERTY(Category = FX, BlueprintReadOnly, EditDefaultsOnly)
	FVector ImpactDecalSize;

	UFUNCTION(Category = FX, BlueprintImplementableEvent)
		void PostParticleSpawned(UParticleSystemComponent* NewParticle);
	UFUNCTION(Category = FX, BlueprintImplementableEvent)
		void PostDecalSpawned(UDecalComponent* NewDecal);
};
