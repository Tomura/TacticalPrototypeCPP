// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "GameFramework/Actor.h"
#include "Math/Axis.h"
#include "TacticalImpactFX.generated.h"

UCLASS(NotPlaceable, abstract)
class TACTICALFPS_API ATacticalImpactFX : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATacticalImpactFX();

	/* Check if spawned for local players */
	UPROPERTY(Category = "Effects", EditDefaultsOnly)
	bool bSpawnLocal;

	/* Check if any player can see this effect before spawning */
	UPROPERTY(Category = "Effects", EditDefaultsOnly)
	bool bCheckView; 

	/* Determines of ImpactFXComponent can override the effect */
	UPROPERTY(Category = "Effects", EditDefaultsOnly)
	uint8 bAllowParticleOverride : 1;
	UPROPERTY(Category = "Effects", EditDefaultsOnly)
	uint8 bAllowDecalOverride : 1;
	UPROPERTY(Category = "Effects", EditDefaultsOnly)
	uint8 bAllowSoundOverride : 1;

	UPROPERTY(Category = "Effects", EditDefaultsOnly)
	float DecalLifeTime;

	UFUNCTION(Category = "Weapon|ImpactFX", BlueprintNativeEvent)
	UParticleSystem* GetImpactParticle(const FHitResult& Hit, EPhysicalSurface SurfaceType, TEnumAsByte<EAxis::Type>& Axis) const;
	virtual UParticleSystem* GetImpactParticle_Implementation(const FHitResult& Hit, EPhysicalSurface SurfaceType, TEnumAsByte<EAxis::Type>& Axis) const { return nullptr; }

	UFUNCTION(Category = "Weapon|ImpactFX", BlueprintImplementableEvent)
	TSubclassOf<AActor> GetImpactActorClass(const FHitResult& Hit, EPhysicalSurface SurfaceType = SurfaceType_Default) const;

	UFUNCTION(Category = "Weapon|ImpactFX", BlueprintImplementableEvent)
	USoundCue* GetImpactSound(const FHitResult& Hit, EPhysicalSurface SurfaceType = SurfaceType_Default) const;
	virtual USoundCue* GetImpactSound_Implementation(const FHitResult& Hit, EPhysicalSurface SurfaceType = SurfaceType_Default) const { return nullptr; }

	UFUNCTION(Category = "Weapon|ImpactFX", BlueprintImplementableEvent)
	UMaterialInterface* GetImpactDecal(const FHitResult& Hit, EPhysicalSurface SurfaceType = SurfaceType_Default) const;
	virtual UMaterialInterface* GetImpactDecal_Implementation(const FHitResult& Hit, EPhysicalSurface SurfaceType = SurfaceType_Default) const { return nullptr; }

	UFUNCTION(Category = "Weapon|ImpactFX", BlueprintImplementableEvent)
	FVector GetDecalSize(EPhysicalSurface SurfaceType = SurfaceType_Default) const;
	virtual FVector GetDecalSize_Implementation(EPhysicalSurface SurfaceType = SurfaceType_Default) const { return FVector(1.f); }

	//UFUNCTION(Category = "Weapon|ImpactFX")
	virtual void SpawnImpactEffects(UWorld* World, const FHitResult& Hit, const FVector& ShotDir, class UTacticalImpactFXComponent* ImpactComp = nullptr);



	//Statics
public:
	static UDecalComponent* SpawnImpactDecal(UMaterialInterface* ImpactDecal, const FHitResult& Hit, const FVector& DecalSize, float DecalLifeTime);
};
