// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TacticalPhysicalMaterial.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalPhysicalMaterial : public UPhysicalMaterial
{
	GENERATED_BODY()
	
public:
	UTacticalPhysicalMaterial();

	virtual void SpawnImpactFX(UWorld* World, const FHitResult& Hit, const FVector& ShotDir, class UTacticalImpactFXComponent* ImpactComp);


	FVector GetDecalSize() const;

public:
	UPROPERTY(Category = "Impact", BlueprintReadOnly, EditAnywhere)
	UParticleSystem* ImpactPSC;
	UPROPERTY(Category = "Impact", BlueprintReadOnly, EditAnywhere)
	TEnumAsByte<EAxis::Type> ImpactPSCAxis;

	UPROPERTY(Category = "Impact", BlueprintReadOnly, EditAnywhere)
	USoundCue* ImpactSound;

	UPROPERTY(Category = "Impact", BlueprintReadOnly, EditAnywhere)
	UMaterialInterface* ImpactDecal;

	UPROPERTY(Category = "Impact", BlueprintReadOnly, EditAnywhere)
	FFloatRange DecalSizeRange;

	UPROPERTY(Category = "Impact", BlueprintReadOnly, EditAnywhere)
	TSubclassOf<class AActor> ImpactActorClass;

	UPROPERTY(Category = "Foot Step", BlueprintReadOnly, EditAnywhere)
	USoundCue* FootStepSound;
	UPROPERTY(Category = "Foot Step", BlueprintReadOnly, EditAnywhere)
	USoundCue* FallSound;

	UPROPERTY(Category = "Foot Step", BlueprintReadOnly, EditAnywhere)
	float FootStepNoiseMod;


	UPROPERTY(Category = "BulletDrop", BlueprintReadOnly, EditAnywhere)
		USoundCue* BulletDropSound;
	UPROPERTY(Category = "BulletDrop_ShotGun", BlueprintReadOnly, EditAnywhere)
		USoundCue* BulletDropSound_Shotgun;
	

};
