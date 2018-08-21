// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalCharacter.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponState.h"
#include "TacticalWeaponStateFiring.h"
#include "TacticalPlayerController.h"
#include "TacticalAmmoType.h"
#include "TacticalProjectile.h"
#include "TacticalBulletProjectile.h"


UTacticalWeaponStateFiring::UTacticalWeaponStateFiring()
	: Super()
{
	bDoObstructionCheck = true;
}

void UTacticalWeaponStateFiring::BeginState(const UTacticalWeaponState* PrevState)
{
	ShotCount = 0;
	
	FireAmmunition();
	TimeWeaponRefire();

}

void UTacticalWeaponStateFiring::EndState(const UTacticalWeaponState* NextState)
{
	//GetWeapon()->ClearPendingFire(GetWeapon()->CurrentFireMode);
	GetWorldTimerManager().ClearTimer(TimerRefire);
	ShotCount = 0;
}

void UTacticalWeaponStateFiring::EndFiring()
{
	// release charging shot
}

void UTacticalWeaponStateFiring::BeginWeaponObstructed()
{
	GetWeapon()->GotoState(GetWeapon()->StateObstructed);
}

void UTacticalWeaponStateFiring::FireAmmunition()
{
	const bool IsLocalMachine = GetInventoryOwner()->IsLocallyControlled();

	ShotCount += 1;

	GetWeapon()->IncrementSpread();
	if (GetOwnerRole() == ROLE_Authority)
	{
		// we need to replicate the simulate fire command first
		NetMulti_SimulateFire();
	}
	else
	{
		// this is likely the owner. Simulate fire instantly
		if (SimulateFireEvent.IsBound())
		{
			SimulateFireEvent.Broadcast();
		}
	}
	GetWeapon()->ConsumeAmmo();
	if (GetWeapon()->TriggerDelay <= 0.01f)
	{
		Fire();
	}
	else
	{
		GetWorldTimerManager().SetTimer(TimerTrigger, this, &UTacticalWeaponStateFiring::Fire, GetWeapon()->TriggerDelay);
	}

}


void UTacticalWeaponStateFiring::FireCustom()
{
	//if (GetOwnerRole() == ROLE_Authority)
	//{
	//	GetWeapon()->OnFireCustom();
	//}
}

void UTacticalWeaponStateFiring::FireInstant()
{
	// todo implement a GetRewindTime function inside BRSPlayerController
	ATacticalPlayerController* BRSPC = GetInventoryOwner() ? Cast<ATacticalPlayerController>(GetInventoryOwner()->Controller) : nullptr;
	float RewindTime = (GetOwnerRole() == ROLE_Authority && BRSPC) ? BRSPC->GetPredictionTime() : 0.f;

	if (!GetWeapon()->AmmoType)
	{
		// no Ammo type set -> return
		return;
	}
	const UTacticalAmmoType* AmmoCDO = GetAmmoType()->GetDefaultObject<UTacticalAmmoType>();


	int32 RandomSeed = int32(RAND_MAX * GetWeapon()->RandomSpreadStream.FRand());

	// Generate Spread
	FRandomStream WeaponRandomStream(RandomSeed);
	const float CurrentSpread = GetWeapon()->GetSpread();
	const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);

	const float BuckshotConeHalfAngle = FMath::DegreesToRadians(AmmoCDO->GetBuckshotAngle() * 0.5f);

	FVector TraceStart, ShotDir;
	GetWeapon()->GetWeaponTraceOriginAndDirection(TraceStart, ShotDir);


	const int32 NumShots = AmmoCDO->GetBuckshots();


	for (int32 i = 0; i < NumShots; i++)
	{
		FVector TraceEnd = TraceStart + WeaponRandomStream.VRandCone(ShotDir, BuckshotConeHalfAngle) * FMath::Max(GetWeapon()->GetRange(), WEAPON_MIN_TRACE_RANGE);
		TArray<FHitResult> Hits;
		GetWeapon()->TraceShot(Hits, TraceStart, TraceEnd);


		//todo?: Maybe trace from gun after this
		//todo make more accurate?

		const FVector TraceDir = (TraceEnd - TraceStart).GetSafeNormal();

		// Apply Damage, allow one penetration
		// First Hit
		const int32 LastPene = (AmmoCDO->AllowsSurfacePenetration() ? 2 : 1);
		for (int32 NumPenetrations = 0; NumPenetrations < LastPene; NumPenetrations++)
		{
			if (Hits.IsValidIndex(NumPenetrations))
			{
				GetWeapon()->ApplyDamage(Hits[NumPenetrations], TraceDir, NumPenetrations);

				if(NumPenetrations == Hits.Num()-1)
				{
					OnRicochet(Hits[NumPenetrations], TraceDir);
				}
			}
		}
	}

	// Tell Clients to Simulate the Hit
	if (GetOwnerRole()== ROLE_Authority && GetNetMode() != NM_Standalone)
	{
		GetWeapon()->SimulateHit(RandomSeed, CurrentSpread);
	}
	else
	{
		GetWeapon()->SimulateHit_Internal(RandomSeed, CurrentSpread);
	}
}

void UTacticalWeaponStateFiring::FireProjectile()
{
	//ATacticalWeapon* const WPN = GetWeapon();

	//if (WPN->Role == ROLE_Authority)
	//{
	//	FFireMode& FM = GetCurrentFireMode();

	//	if (!FM.ProjectileType)
	//	{
	//		UE_LOG(LogInventory, Warning, TEXT("Weapon %s has no valid projectile specified."), *GetWeapon()->GetName())
	//			return;
	//	}

	//	int32 RandomSeed = FMath::Rand();
	//	FRandomStream WeaponRandomStream(RandomSeed);

	//	const float CurrentSpread = WPN->GetCurrentSpread();
	//	//UE_LOG(LogTemp, Log, TEXT("Spread: %f"), WPN->GetCurrentSpread());
	//	const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);

	//	FVector TraceStart = GetInventoryOwner() ? GetInventoryOwner()->GetWeaponStartTraceLocation() : WPN->Instigator->GetActorLocation();


	//	FActorSpawnParameters SpawnParams;
	//	SpawnParams.Owner = WPN->GetOwner();
	//	SpawnParams.Instigator = WPN->GetInventoryOwner();
	//	SpawnParams.bNoCollisionFail = true;

	//	TArray<ABRSProjectile*> NewProjectiles;
	//	const int32 NumShots = FMath::Max(1, FM.NumProjectiles);
	//	//SpawnParams.
	//	for (int32 i = 0; i < NumShots; i++)
	//	{
	//		const FVector TraceDir = WeaponRandomStream.VRandCone(WPN->GetCameraAim(), ConeHalfAngle);
	//		FVector TraceEnd = TraceStart + TraceDir * FMath::Max(FM.Range, WEAPON_MIN_TRACE_RANGE);

	//		FHitResult Hit = WPN->TraceShot(TraceStart, TraceEnd);

	//		const FVector ProjectileSpawnPoint = TraceStart + WPN->GetCameraAim().Rotation().RotateVector(WPN->MuzzleOffset);
	//		const FRotator ProjectileRot = TraceDir.Rotation();

	//		//DrawDebugLine(GetWorld(), TraceStart, TraceDir, FColor::Red, true, 2.f);

	//		ABRSProjectile* NewProj = GetWorld()->SpawnActor<ABRSProjectile>(*FM.ProjectileType,
	//			ProjectileSpawnPoint,
	//			ProjectileRot,
	//			SpawnParams);
	//		if (NewProj)
	//		{
	//			NewProj->CollisionComp->MoveIgnoreActors.Add(WPN);
	//			if (FM.bOverrideDamage)
	//			{
	//				NewProj->OverrideProjectileDamage(FM.Damage);
	//			}
	//		}
	//		NewProjectiles.Add(NewProj);
	//	}
	//	for (ABRSProjectile* LoopProjectile : NewProjectiles)
	//	{
	//		if (LoopProjectile)
	//		{
	//			for (ABRSProjectile* IgnoreProjectile : NewProjectiles)
	//			{
	//				if (IgnoreProjectile)
	//					LoopProjectile->CollisionComp->MoveIgnoreActors.Add(IgnoreProjectile);
	//			}
	//		}
	//	}

	//	WPN->BP_OnFireProjectile(NewProjectiles);
	//}
}

void UTacticalWeaponStateFiring::TimeWeaponRefire()
{
	GetWorldTimerManager().SetTimer(TimerRefire, this, &UTacticalWeaponStateFiring::TryRefire, GetWeapon()->GetFireInterval(), true);
}

void UTacticalWeaponStateFiring::Fire()
{		
	// Hit Detection and Spawning only handled by Authority
		// If we decide to use local HitScanning we will need to implement this somewhere else

		//switch (FM.ShotType)
		//{
		//case EShotType::Instant:
		//	FireInstant();
		//	break;
		//case EShotType::Projectile:
		//	FireProjectile();
		//	break;
		//case EShotType::Custom:
		//	FireCustom();
		//	break;
		//default:
		//	break;
		//}
		// todo: Local Hit Scan, then Server confirm with compensated trace to Hit Collision Capsule
	FireInstant();
	GetWeapon()->OnFire();
}

void UTacticalWeaponStateFiring::NetMulti_SimulateFire_Implementation()
{
	if (GetOwnerRole() == ROLE_Authority || (!GetInventoryOwner() || !GetInventoryOwner()->IsLocallyControlled()))
	{
		if (SimulateFireEvent.IsBound())
		{
			SimulateFireEvent.Broadcast();
		}
	}
}

void UTacticalWeaponStateFiring::TryRefire()
{
	if (GetWeapon()->bPendingReload)
	{
		GetWeapon()->GotoState(GetWeapon()->StateReloading);
		return;
	}

	if (ShouldRefire())
	{
		if (!GetWeapon()->HasAmmoLoaded())
		{ 		//Simulate Empty Weapon
			GetWeapon()->OnEmpty();		
			GetWeapon()->ClearPendingFire();
			GetWeapon()->GotoState(GetWeapon()->StateActive);
			return;
		}
		FireAmmunition();
		return;
	}
	else
	{
		GetWeapon()->ClearPendingFire();
		GetWeapon()->GotoState(GetWeapon()->StateActive);
	}
}

bool UTacticalWeaponStateFiring::ShouldRefire() const
{
	if (GetWeapon()->bPendingUnequip)
		return false;


	if (GetWeapon()->CanFire())
	{
		switch (GetWeapon()->GetFiringMode())
		{
		case EWeaponFiringMode::FM_Single:
			return ShotCount < 1;
		case EWeaponFiringMode::FM_Burst:
			return (ShotCount < GetWeapon()->BurstCount);
		case EWeaponFiringMode::FM_Auto:
			return GetWeapon()->IsPendingFire();
		default:
			break;
		}
	}
	return false;
}

TSubclassOf<class UTacticalAmmoType> UTacticalWeaponStateFiring::GetAmmoType() const
{
	return GetWeapon()->AmmoType;
}

void UTacticalWeaponStateFiring::OnRicochet(const FHitResult& Hit, const FVector& TraceDir)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (!GetAmmoType()) // if no Ammo type set -> return
			return;
		

		const UTacticalAmmoType* AmmoCDO = GetAmmoType()->GetDefaultObject<UTacticalAmmoType>();
		if (!AmmoCDO->DoesRicochet(Hit) || !AmmoCDO->GetRicochetProjectile())
			return;

		//int32 RandomSeed = int32(RAND_MAX * GetWeapon()->RandomSpreadStream.FRand());
		//FRandomStream WeaponRandomStream(RandomSeed);
		const float ImpactAngle = FMath::RadiansToDegrees(FMath::Acos(-TraceDir | Hit.ImpactNormal)); // Range 0 - 180°

		if (ImpactAngle < AmmoCDO->GetRicochetAngleThreshold())
			return;

		const FVector RicochetDirBase = FMath::GetReflectionVector(TraceDir, Hit.ImpactNormal);
		//const FVector RicochetDir = WeaponRandomStream.VRandCone(RicochetDirBase, 10.f);


		const FVector SpawnLoc(Hit.ImpactPoint.X, Hit.ImpactPoint.Y, Hit.ImpactPoint.Z);
		const FRotator SpawnRot = RicochetDirBase.Rotation();
		ATacticalBulletProjectile* NewBullet = GetWorld()->SpawnActorDeferred<ATacticalBulletProjectile>(AmmoCDO->GetRicochetProjectile(), FTransform(SpawnRot, SpawnLoc), GetWeapon(), GetInventoryOwner(),
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		if (NewBullet)
		{
			NewBullet->AmmoType = GetAmmoType();
			NewBullet->FinishSpawning(FTransform(SpawnRot, SpawnLoc));
		}
	}
}

void UTacticalWeaponStateFiring::Reload()
{
	GetWeapon()->bPendingReload = true;
}