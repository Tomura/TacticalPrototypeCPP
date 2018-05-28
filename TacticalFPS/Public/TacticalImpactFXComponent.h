// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Components/ActorComponent.h"
#include "TacticalImpactFXComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSimulateHitDelegate, const FHitResult&, Hit, const FVector&, ShotDirection);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSpawnImpactParticle, UParticleSystemComponent*, NewParticle, const FHitResult&, Hit, const FVector&, ShotDirection);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSpawnImpactParticle, UParticleSystemComponent*, NewParticle, const FHitResult&, Hit, const FVector&, ShotDirection);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TACTICALFPS_API UTacticalImpactFXComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTacticalImpactFXComponent();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;


	bool PlayImpactSound(const FHitResult& Hit);
	UParticleSystemComponent* SpawnImpactParticle(const FHitResult& Hit, const FVector& ShotDir);
	UDecalComponent* SpawnImpactDecal(const FHitResult& Hit);

	UParticleSystem* GetImpactParticle() const { return ImpactParticle; }

	FORCEINLINE bool OverrideParticle() const { return bOverrideParticle; }
	FORCEINLINE bool OverrideSound() const { return bOverrideSound; }
	FORCEINLINE bool OverrideDecal() const { return bOverrideDecal; }

	UPROPERTY(BlueprintAssignable, Category = "ImpactFx")
	FOnSimulateHitDelegate OnSimulateHit;

	UPROPERTY(BlueprintAssignable, Category = "ImpactFx")
		FOnSpawnImpactParticle OnSpawnImpactParticle;

protected:
	UPROPERTY(Category = "Particle", EditAnywhere)
	bool bOverrideParticle;
	UPROPERTY(Category = "Particle", EditAnywhere, meta=(EditCond="bOverrideParticle"))
	UParticleSystem* ImpactParticle;

	UPROPERTY(Category = "Sound", EditAnywhere)
	bool bOverrideSound;
	UPROPERTY(Category = "Sound", EditAnywhere, meta=(EditCond="bOverideSound"))
	USoundCue* ImpactSound;
	UPROPERTY(Category = "Sound", EditAnywhere)
	float VolumeModifier;
	UPROPERTY(Category = "Sound", EditAnywhere)
	float PitchModifier;

	UPROPERTY(Category = "Decal", EditAnywhere)
	bool bOverrideDecal;
	UPROPERTY(Category = "Decal", EditAnywhere)
	UMaterialInterface* ImpactDecal;
	UPROPERTY(Category = "Decal", EditAnywhere)
	FVector DecalSize;
	
};
