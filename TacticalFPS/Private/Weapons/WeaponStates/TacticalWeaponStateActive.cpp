// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalCharacter.h"
#include "TacticalInventoryComponent.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponState.h"
#include "TacticalWeaponStateActive.h"
#include "TacticalWeaponAttachmentPoint.h"
#include "TacticalWeaponAttachment_Weapon.h"

void UTacticalWeaponStateActive::BeginState(const UTacticalWeaponState* PrevState)
{
	if (GetInventoryOwner() == nullptr || GetInventoryOwner()->GetInventory() == nullptr)
	{
		GetWeapon()->GotoState(GetWeapon()->StateInactive);
		return;
	}

	if (GetInventoryOwner()->GetInventory()->PendingWeapon == NULL || !GetWeapon()->bPendingUnequip)
	{
		// check if firemode is pending
		if (GetWeapon()->IsPendingFire())
		{
			//GetWeapon()->GotoState(GetWeapon()->StateFiring);
			BeginFiring();
			return;
		}
	}
	else
	{
		Unequip();
	}
}

void UTacticalWeaponStateActive::BeginFiring()
{
	if (!GetWeapon()->CanFire())
		return;

	TArray<UTacticalWeaponAttachmentPoint*> AttachPoints;
	GetWeapon()->GetAttachmentPoints(AttachPoints);
	for (UTacticalWeaponAttachmentPoint* TestPoint : AttachPoints)
	{
		if (TestPoint->GetCurrentAttachment() == nullptr)
			continue;

		if (ATacticalWeaponAttachment_Weapon* TestAttach = Cast<ATacticalWeaponAttachment_Weapon>(TestPoint->GetCurrentAttachment()))
		{
			if (TestAttach->IsAttachmentActive())
			{
				if (!TestAttach->HasAmmoLoaded())
				{
					//Simulate Empty Weapon
					TestAttach->OnEmpty();
					//if (bAutoReload) Reload();
					return;
				}

				if (TestAttach->HasAmmoLoaded() && TestAttach->GetFiringMode() != EWeaponFiringMode::FM_None)
				{
					GetWeapon()->GotoState(TestAttach->StateFiring);
					return;
				}

			}
			break;
		}
	}


	if (!GetWeapon()->HasAmmoLoaded())
	{
		//Simulate Empty Weapon
		GetWeapon()->OnEmpty();
		//if (bAutoReload) Reload();
		return;
	}

	if (GetWeapon()->HasAmmoLoaded() && GetWeapon()->GetFiringMode() != EWeaponFiringMode::FM_None)
	{
		GetWeapon()->GotoState(GetWeapon()->StateFiring);
	}
}

void UTacticalWeaponStateActive::Reload()
{
	TArray<UTacticalWeaponAttachmentPoint*> AttachPoints;
	GetWeapon()->GetAttachmentPoints(AttachPoints);
	for (UTacticalWeaponAttachmentPoint* TestPoint : AttachPoints)
	{
		if (TestPoint->GetCurrentAttachment() == nullptr)
			continue;

		if (ATacticalWeaponAttachment_Weapon* TestAttach = Cast<ATacticalWeaponAttachment_Weapon>(TestPoint->GetCurrentAttachment()))
		{
			if (TestAttach->IsAttachmentActive())
			{
				if (TestAttach->NeedsReload() && TestAttach->HasAnyAmmo())
				{
					GetWeapon()->GotoState(TestAttach->StateReloading);
				}
				return;
			}
			break;
		}
	}

	//UE_LOG(LogInventory, Log, TEXT("Wants to Reload"));
	if (GetWeapon()->NeedsReload() && GetWeapon()->HasAnyAmmo())
	{
		GetWeapon()->GotoState(GetWeapon()->StateReloading);
	}
}

void UTacticalWeaponStateActive::Unequip()
{
	if (GetInventoryOwner()->CanFire())
	{
		GetWeapon()->bPendingUnequip = false;
		GetWeapon()->GotoState(GetWeapon()->StateUnequipping);
	}
	else
	{
		GetWeapon()->bPendingUnequip = true;
	}
}

void UTacticalWeaponStateActive::BeginWeaponObstructed()
{
	GetWeapon()->GotoState(GetWeapon()->StateObstructed);
}
