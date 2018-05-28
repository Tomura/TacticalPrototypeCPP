#include "TacticalFPS.h"
#include "TacticalCharacter.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponAttachment_Weapon.h"
#include "TacticalWeaponStateGadgetReload.h"


UTacticalWeaponStateGadgetReload::UTacticalWeaponStateGadgetReload()
	: Super()
{
	bPendingStop = false;
}


void UTacticalWeaponStateGadgetReload::BeginState(const UTacticalWeaponState* PrevState)
{
	GetWeapon()->bPendingReload = false;
	bPendingStop = false;
	//UE_LOG(LogInventory, Log, TEXT("Reloading"));
	if ((GetAttachment()->NeedsReload()) && (GetAttachment()->HasAnyAmmo()))
	{
		if (GetOwnerRole() == ROLE_Authority && !(GetNetMode() == NM_Standalone))
		{
			GetAttachment()->NetMulti_Reload();
			//GetWeapon()->SimulateReload();
		}
		else
		{
			GetAttachment()->LocalSimulateStartReload();
		}
		if (!GetAttachment()->ReloadRoundsIndividually())
		{
			// Conditions to reload exist -> Start Timer
			GetWorldTimerManager().SetTimer(TimerReload, this, &UTacticalWeaponStateGadgetReload::SwapMagazine, FMath::Max(GetAttachment()->GetReloadTime(), 0.01f));
			GetAttachment()->LocalSimulateStartReload();
		}
		else
		{
			const int32 MagNum = GetAttachment()->GetAmmoRemaining();
			if (MagNum > 1)
			{
				// todo: Initial Time (Pre round load animation opening gun) + ReloadTime instead of just ReloadTime
				const float FirstReloadTime = GetAttachment()->ReloadStartTime + GetAttachment()->GetReloadTime();
				GetWorldTimerManager().SetTimer(TimerReload, this, &UTacticalWeaponStateGadgetReload::ContinueReloading, FMath::Max(FirstReloadTime, 0.01f));

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

void UTacticalWeaponStateGadgetReload::EndState(const UTacticalWeaponState* NextState)
{
	GetWorldTimerManager().ClearTimer(TimerReload);
}

void UTacticalWeaponStateGadgetReload::CancelReload()
{
	bPendingStop = true;
}

void UTacticalWeaponStateGadgetReload::ReloadFinished()
{
	GetWeapon()->GotoState(GetWeapon()->StateActive);
}

void UTacticalWeaponStateGadgetReload::SwapMagazine()
{
	int32 TransferAmmo = 0;

	TransferAmmo = FMath::Max(GetAttachment()->AmmoCapacity - GetAttachment()->AmmoLoaded, 0);
	if (GetAttachment()->bCanBulletStayInChamber && GetWeapon()->GetAmmoLoaded() > 0)
	{
		// add bullet to next mag
		TransferAmmo += 1;
	}

	TransferAmmo = FMath::Clamp(TransferAmmo, 0, GetAttachment()->AmmoRemaining);
	GetAttachment()->AmmoRemaining -= TransferAmmo;
	GetAttachment()->AmmoLoaded = TransferAmmo;

	ReloadFinished();
}

void UTacticalWeaponStateGadgetReload::ContinueReloading()
{
	//// Mag 0 contains rounds in weapon. Mag 1 contains spare rounds. Feed from mag 1 to mag 0
	//if (GetWeapon()->HasAnyAmmo() && GetWeapon()->Magazine[1])
	//{
	//	GetWeapon()->CurrentMagazine = 0;
	//	GetWeapon()->Magazine[0] += 1;

	//	GetWeapon()->Magazine[1] -= 1;
	//}
	//if ((GetWeapon()->Magazine[1] > 0) && GetWeapon()->NeedsReload() && !bPendingStop)
	//{
	//	//We still have ammo left to fill and need more
	//	// we also don't want to stop reloading
	//	GetWorldTimerManager().SetTimer(TimerReload, this, &UTacticalWeaponStateReloading::ContinueReloading, FMath::Max(GetWeapon()->GetReloadTime(), 0.01f));
	//}
	//else
	//{
	//	GetWorldTimerManager().SetTimer(TimerReload, this, &UTacticalWeaponStateReloading::ReloadFinished, FMath::Max(GetWeapon()->ReloadFinishTime, 0.01f));
	//	if (GetOwnerRole() == ROLE_Authority && !(GetNetMode() == NM_Standalone))
	//		GetWeapon()->NetMulti_StopReload();
	//	else
	//		GetWeapon()->LocalSimulateStopReload();
	//}
}

void UTacticalWeaponStateGadgetReload::Unequip() {}

float UTacticalWeaponStateGadgetReload::GetReloadTimeElapsed() const
{
	if (GetAttachment())
	{
		return GetWorldTimerManager().GetTimerElapsed(TimerReload);
	}
	return -1.f;
}

