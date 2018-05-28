// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalCharacter.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponState.h"
#include "TacticalWeaponStateReloading.h"


UTacticalWeaponStateReloading::UTacticalWeaponStateReloading()
	: Super()
{
	bPendingStop = false;
}


void UTacticalWeaponStateReloading::BeginState(const UTacticalWeaponState* PrevState)
{
	GetWeapon()->bPendingReload = false;
	bPendingStop = false;
	//UE_LOG(LogInventory, Log, TEXT("Reloading"));
	if ((GetWeapon()->NeedsReload()) && (GetWeapon()->HasAnyAmmo()))
	{
		if (GetOwnerRole() == ROLE_Authority && !(GetNetMode() == NM_Standalone))
		{
			GetWeapon()->NetMulti_Reload();
		}
		else
		{
			GetWeapon()->LocalSimulateReload();
		}
		if (!GetWeapon()->ReloadRoundsIndividually())
		{
			// Conditions to reload exist -> Start Timer
			GetWorldTimerManager().SetTimer(TimerReload, this, &UTacticalWeaponStateReloading::SwapMagazine, FMath::Max(GetWeapon()->GetReloadTime(), 0.01f));

			GetWeapon()->LocalSimulateReload();
		}
		else
		{
			const int32 MagNum = GetWeapon()->Magazine.Num();
			if (MagNum > 1)
			{
				// todo: Initial Time (Pre round load animation opening gun) + ReloadTime instead of just ReloadTime
				const float FirstReloadTime = GetWeapon()->ReloadStartTime + GetWeapon()->GetReloadTime();
				GetWorldTimerManager().SetTimer(TimerReload, this, &UTacticalWeaponStateReloading::ContinueReloading, FMath::Max(FirstReloadTime, 0.01f));

				//GetWeapon()->LocalSimulateReload();
			}
		}
	}
	else
	{
		// Requirements for Reloading do not exits go back to active
		GetWeapon()->GotoState(GetWeapon()->StateActive);
	}
}
void UTacticalWeaponStateReloading::EndState(const UTacticalWeaponState* NextState)
{
	GetWorldTimerManager().ClearTimer(TimerReload);
}

void UTacticalWeaponStateReloading::ReloadFinished()
{
	GetWeapon()->GotoState(GetWeapon()->StateActive);
}


void UTacticalWeaponStateReloading::SwapMagazine()
{
	/*int32 TransferAmmo = FMath::Min(GetWeapon()->AmmoCapacity - GetWeapon()->CurrentAmmoLoaded, GetWeapon()->CurrentAmmo);
	GetWeapon()->CurrentAmmo -= TransferAmmo;
	GetWeapon()->CurrentAmmoLoaded += TransferAmmo;*/
	int32 TransferAmmo = 0;
	if (GetWeapon()->bCanBulletStayInChamber && GetWeapon()->GetAmmoLoaded() > 0)
	{
		// Leave Bullet in Chamber
		// remove that bullet from current Mag
		GetWeapon()->Magazine[GetWeapon()->CurrentMagazine] -= 1;
		// add bullet to next mag
		TransferAmmo = 1;
	}
	int32 NextMagId = GetWeapon()->GetBestMag();
	GetWeapon()->Magazine[NextMagId] += TransferAmmo;
	GetWeapon()->CurrentMagazine = NextMagId;

	// todo: maybe start another timer so we have a state where the mag is removed and one where the new mag is inserted but not loaded
	ReloadFinished();
}

void UTacticalWeaponStateReloading::ContinueReloading()
{
	// Mag 0 contains rounds in weapon. Mag 1 contains spare rounds. Feed from mag 1 to mag 0
	if (GetWeapon()->HasAnyAmmo() && GetWeapon()->Magazine[1])
	{
		GetWeapon()->CurrentMagazine = 0;
		GetWeapon()->Magazine[0] += 1;

		GetWeapon()->Magazine[1] -=1;
	}
	if ((GetWeapon()->Magazine[1] > 0) && GetWeapon()->NeedsReload() && !bPendingStop)
	{
		//We still have ammo left to fill and need more
		// we also don't want to stop reloading
		GetWorldTimerManager().SetTimer(TimerReload, this, &UTacticalWeaponStateReloading::ContinueReloading, FMath::Max(GetWeapon()->GetReloadTime(), 0.01f));
	}
	else
	{
		GetWorldTimerManager().SetTimer(TimerReload, this, &UTacticalWeaponStateReloading::ReloadFinished, FMath::Max(GetWeapon()->ReloadFinishTime, 0.01f));
		if (GetOwnerRole() == ROLE_Authority && !(GetNetMode() == NM_Standalone))
			GetWeapon()->NetMulti_StopReload();
		else
			GetWeapon()->LocalSimulateStopReload();
	}
}

void UTacticalWeaponStateReloading::CancelReload()
{
	bPendingStop = true;
	
	//TODO insert stuff to simulate it, replicate it
}

void UTacticalWeaponStateReloading::Unequip()
{
	GetWeapon()->GotoState(GetWeapon()->StateUnequipping);

	// TODO simulate and replicate it
}

float UTacticalWeaponStateReloading::GetReloadTimeElapsed() const
{
	if (GetWeapon())
	{
		return GetWorldTimerManager().GetTimerElapsed(TimerReload);
	}
	return -1.f;
}
