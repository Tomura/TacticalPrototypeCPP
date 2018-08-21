// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/TacticalProjectile.h"
#include "TacticalBulletProjectile.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API ATacticalBulletProjectile : public ATacticalProjectile
{
	GENERATED_BODY()
public:
	ATacticalBulletProjectile();
	
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnImpact(const FHitResult& Hit);


	TSubclassOf<class UTacticalAmmoType> AmmoType;

	UFUNCTION(NetMulticast, Unreliable)
	void OnSimulateHit(const FHitResult& Hit);
	virtual void OnSimulateHit_Implementation(const FHitResult& Hit);
};
