// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#include "TacticalFPS.h"
#include "TacticalCharacter.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponAttachment_Weapon.h"
#include "TacticalPlayerController.h"
#include "TacticalAmmoType.h"
#include "TacticalWeaponStateGadgetFiring.h"




UTacticalWeaponStateGadgetFiring::UTacticalWeaponStateGadgetFiring()
	: Super()
{

}



void UTacticalWeaponStateGadgetFiring::BeginState(const UTacticalWeaponState* PrevState)
{
	ShotCount = 0;

	FireAmmunition();
	TimeWeaponRefire();

}

void UTacticalWeaponStateGadgetFiring::EndState(const UTacticalWeaponState* NextState)
{
	//GetWeapon()->ClearPendingFire(GetWeapon()->CurrentFireMode);
	GetWorldTimerManager().ClearTimer(TimerRefire);
	ShotCount = 0;
}

void UTacticalWeaponStateGadgetFiring::EndFiring()
{
	// release charging shot
}

void UTacticalWeaponStateGadgetFiring::BeginWeaponObstructed()
{
	GetWeapon()->GotoState(GetWeapon()->StateObstructed);
}

void UTacticalWeaponStateGadgetFiring::FireAmmunition()
{
	const bool IsLocalMachine = GetInventoryOwner()->IsLocallyControlled();

	ShotCount += 1;

	//GetWeapon()->IncrementSpread();
	if (GetOwnerRole() == ROLE_Authority)
	{
		//UE_LOG(LogTemp, Log, TEXT("Server firing!!! Replicate Simulate"));
		GetAttachment()->NetMulti_SimulateFire();
	}
	else
	{
		//UE_LOG(LogTemp, Log, TEXT("Locally firing!!! simulate"));
		GetAttachment()->NativeLocalSimulateFire();
	}
	GetAttachment()->ConsumeAmmo();
	if (GetAttachment()->TriggerDelay <= 0.01f)
	{
		Fire();
	}
	else
	{
		GetWorldTimerManager().SetTimer(TimerTrigger, this, &UTacticalWeaponStateGadgetFiring::Fire, GetAttachment()->TriggerDelay);
	}

}


void UTacticalWeaponStateGadgetFiring::FireCustom()
{
	//if (GetOwnerRole() == ROLE_Authority)
	//{
	//	GetWeapon()->OnFireCustom();
	//}
}

void UTacticalWeaponStateGadgetFiring::FireInstant()
{
	// todo implement a GetRewindTime function inside BRSPlayerController
	ATacticalPlayerController* BRSPC = GetWeapon()->GetInventoryOwner() ? Cast<ATacticalPlayerController>(GetWeapon()->GetInventoryOwner()->Controller) : nullptr;
	float RewindTime = (GetOwnerRole() == ROLE_Authority && BRSPC) ? BRSPC->GetPredictionTime() : 0.f;

	if (!GetAttachment()->AmmoType)
	{
		// no Ammo type set -> return
		return;
	}
	const UTacticalAmmoType* AmmoCDO = GetWeapon()->AmmoType->GetDefaultObject<UTacticalAmmoType>();


	//GetWeapon()->NetSynchRandomSeed(); //todo to synch RNG of spread
	//int32 RandomSeed = FMath::Rand();
	int32 RandomSeed = int32(RAND_MAX * GetWeapon()->RandomSpreadStream.FRand());

	// Generate Spread
	FRandomStream WeaponRandomStream(RandomSeed);
	const float CurrentSpread = 0.f;
	//const float CurrentSpread = GetWeapon()->GetSpread();
	const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);

	const float BuckshotConeHalfAngle = FMath::DegreesToRadians(AmmoCDO->GetBuckshotAngle() * 0.5f);

	FVector TraceStart, ShotDir;
	GetWeapon()->GetWeaponTraceOriginAndDirection(TraceStart, ShotDir);

	//const FVector TraceStart = GetInventoryOwner() ? Get0InventoryOwner()->GetWeaponTraceOrigin() : GetWeapon()->GetActorLocation();
	//const FVector ShotDir = WeaponRandomStream.VRandCone(GetWeapon()->GetCameraAim(), ConeHalfAngle);

	const int32 NumShots = AmmoCDO->GetBuckshots();


	for (int32 i = 0; i < NumShots; i++)
	{
		FVector TraceEnd = TraceStart + WeaponRandomStream.VRandCone(ShotDir, BuckshotConeHalfAngle) * FMath::Max(10000.f, WEAPON_MIN_TRACE_RANGE);
		TArray<FHitResult> Hits;
		GetWeapon()->TraceShot(Hits, TraceStart, TraceEnd);

		//FHitResult Hit = GetWeapon()->TraceShotCompensated(TraceStart, TraceEnd, RewindTime);

		//todo?: Maybe trace from gun after this
		//todo make more accurate?

		const FVector TraceDir = (TraceEnd - TraceStart).GetSafeNormal();

		// Apply Damage, allow one penetration
		// First Hit
		for (int32 NumPenetrations = 0; NumPenetrations < (AmmoCDO->AllowsSurfacePenetration() ? 2 : 1); NumPenetrations++)
		{
			if (Hits.IsValidIndex(NumPenetrations))
			{
				// TODO apply Damage from attachment
				//GetWeapon()->ApplyDamage(Hits[NumPenetrations], TraceDir, NumPenetrations);
			}
		}
	}

	// Tell Clients to Simulate the Hit
	if (GetOwnerRole() == ROLE_Authority && GetNetMode() != NM_Standalone)
	{
		GetWeapon()->SimulateHit(RandomSeed, CurrentSpread); // todo move to attachment
	}
	else
	{
		GetWeapon()->SimulateHit_Internal(RandomSeed, CurrentSpread); // todo move to attachment
	}
}

void UTacticalWeaponStateGadgetFiring::FireProjectile()
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

void UTacticalWeaponStateGadgetFiring::TimeWeaponRefire()
{
	GetWorldTimerManager().SetTimer(TimerRefire, this, &UTacticalWeaponStateGadgetFiring::TryRefire, GetWeapon()->GetFireInterval(), true);
}

void UTacticalWeaponStateGadgetFiring::Fire()
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
	//FireInstant();
	// todo check with Weapon->OnFire
	GetAttachment()->OnFire();
}

void UTacticalWeaponStateGadgetFiring::TryRefire()
{
	if (GetWeapon()->bPendingReload)
	{
		GetWeapon()->GotoState(GetAttachment()->StateReloading);
		return;
	}

	if (ShouldRefire())
	{
		if (!(GetAttachment()->HasAmmoLoaded()))
		{ 		//Simulate Empty Weapon
			GetAttachment()->OnEmpty();
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

bool UTacticalWeaponStateGadgetFiring::ShouldRefire() const
{
	if (GetWeapon()->bPendingUnequip)
		return false;


	if (GetWeapon()->CanFire())
	{
		switch (GetAttachment()->FiringMode)
		{
		case EWeaponFiringMode::FM_Single:
			return ShotCount < 1;
		case EWeaponFiringMode::FM_Burst:
			return (ShotCount < GetAttachment()->BurstCount);
		case EWeaponFiringMode::FM_Auto:
			return GetWeapon()->IsPendingFire();
		default:
			break;
		}
	}
	return false;
}

void UTacticalWeaponStateGadgetFiring::Reload()
{
	GetWeapon()->bPendingReload = true;
}