// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "GameFramework/Actor.h"
#include "GenericTeamAgentInterface.h"
#include "TacticalProjectile.generated.h"

UCLASS(NotPlaceable, HideDropdown)
class TACTICALFPS_API ATacticalProjectile : public AActor, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class UProjectileMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* ProjectileCollision;

public:	
	// Sets default values for this actor's properties
	ATacticalProjectile();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// IGenericTeamAgentInterface
	/** Assigns Team Agent to given TeamID */
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) {}

	/** Retrieve team identifier in form of FGenericTeamId */
	virtual FGenericTeamId GetGenericTeamId() const { return FGenericTeamId::NoTeam; }

	/** Retrieved owner attitude toward given Other object */
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const
	{
		// Grenade is always hostile, because it deals damage to everyone in Proximity
		return ETeamAttitude::Hostile;
	}

public:
	UFUNCTION(Category = "Projectile", BlueprintCallable)
	class UProjectileMovementComponent* GetProjectileMovement() const { return MovementComponent; }
	
	UFUNCTION(Category = "Projectile", BlueprintCallable)
	class USphereComponent* GetCollisionComponent() const { return ProjectileCollision; }

	UFUNCTION(Category = "Projectile", BlueprintCallable)
	bool ShouldAIRun() const { return bAIShouldRun; }

protected:
	UPROPERTY(Category = "AI", EditAnywhere)
	uint16 bAIShouldRun : 1;
};
