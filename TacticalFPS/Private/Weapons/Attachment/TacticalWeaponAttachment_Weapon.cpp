// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#include "TacticalFPS.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponAnimInstance.h"
#include "TacticalCharacter.h"
#include "TacticalInventoryComponent.h"
#include "TacticalWeaponAttachment_Weapon.h"
#include "Net/UnrealNetwork.h"
#include "TacticalWeaponStateGadget.h"
#include "TacticalWeaponStateGadgetReload.h"
#include "TacticalWeaponStateGadgetFiring.h"
#include "TacticalAmmoType.h"




ATacticalWeaponAttachment_Weapon::ATacticalWeaponAttachment_Weapon(const FObjectInitializer& OI)
	: Super(OI)
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AttachmentMesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->LightingChannels.bChannel0 = false;
	Mesh->LightingChannels.bChannel1 = true;
	Mesh->LightingChannels.bChannel2 = false;

	bAttachmentActive = false;

	bReloadRoundsIndividually = false;
	ReloadTime = 1.f;
	ReloadStartTime = 1.f;
	WeaponReloadAnim = nullptr; 
	bCanBulletStayInChamber = false;

	MaxAmmo = 5;
	AmmoCapacity = 1;

	SetReplicates(true);

	StateReloading = OI.CreateDefaultSubobject<UTacticalWeaponStateGadgetReload>(this, TEXT("StateReload"));
	StateFiring = OI.CreateDefaultSubobject<UTacticalWeaponStateGadgetFiring>(this, TEXT("StateFiring"));
}

void ATacticalWeaponAttachment_Weapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATacticalWeaponAttachment_Weapon, AmmoLoaded);
	DOREPLIFETIME(ATacticalWeaponAttachment_Weapon, AmmoRemaining);
}

void ATacticalWeaponAttachment_Weapon::BeginPlay()
{
	Super::BeginPlay();

	AmmoLoaded = AmmoCapacity;
	AmmoRemaining = MaxAmmo;
}

void ATacticalWeaponAttachment_Weapon::OnEmpty()
{
	// Gun becomes Empty
	if (GetWeaponOwner()->GetInventoryOwner()->IsLocallyControlled() 
		&& Cast<APlayerController>(GetWeaponOwner()->GetInventoryOwner()->GetController()))
	{
		LocalSimulateEmpty();
	}
}

void ATacticalWeaponAttachment_Weapon::NetMulti_SimulateFire_Implementation()
{
	if (Role == ROLE_Authority || (!GetWeaponOwner()->GetInventoryOwner() || !GetWeaponOwner()->GetInventoryOwner()->IsLocallyControlled()))
	{
		NativeLocalSimulateFire();
	}
}

void ATacticalWeaponAttachment_Weapon::NativeLocalSimulateFire()
{
	// insert any native code that always needs to be called

	// blueprint defined code
	LocalSimulateFire();
}

void ATacticalWeaponAttachment_Weapon::OnFire()
{
	// todo: Add noise for AI

	// todo: Add any recoil + camera anims
	BP_OnFire();
}

void ATacticalWeaponAttachment_Weapon::LocalSimulateStartReload()
{
	if (GetWeaponOwner() && GetWeaponOwner()->GetInventoryOwner())
	{
		ATacticalWeapon* WPNOwner = GetWeaponOwner();
		ATacticalCharacter* InvOwner = WPNOwner->GetInventoryOwner();

		const float AnimTime = bReloadRoundsIndividually ? ReloadStartTime : ReloadTime;
		//GetInventoryOwner()->OnWeaponReload(AnimTime); //todo
		if (WPNOwner->GetInventoryComponent()->OnWeaponReload.IsBound())
		{
			WPNOwner->GetInventoryComponent()->OnWeaponReload.Broadcast(WPNOwner, AnimTime);
		}

		if (!InvOwner->IsThirdPerson())
		{
			UAnimInstance* WeaponAnimInstance = Mesh->GetAnimInstance();
			if ((WeaponReloadAnim != nullptr) && (WeaponAnimInstance != nullptr))
			{
				const float MontageTime = WeaponReloadAnim->GetSectionLength(0);
				WeaponAnimInstance->Montage_Play(WeaponReloadAnim, MontageTime / AnimTime);

			}
		}

		if (bReloadRoundsIndividually)
		{
			//GetWorldTimerManager().SetTimer(ReloadStartTimer, this, &ATacticalWeaponAttachment_Weapon::LocalSimulateContinueReload, FMath::Max(ReloadStartTime, 0.01f), false);
		}
		//OnSimulateReloadStart(AnimTime);
	}
}

void ATacticalWeaponAttachment_Weapon::LocalSimulateStopReload()
{
	ATacticalWeapon* WPNOwner = GetWeaponOwner();
	ATacticalCharacter* InvOwner = WPNOwner ? WPNOwner->GetInventoryOwner():nullptr;

	//if (InvOwner)
	//{
	//	InvOwner->OnWeaponStopReload(GetReloadTime());

	//	const float AnimTime = ReloadFinishTime;
	//	UAnimInstance* WeaponAnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	//	if (WeaponReloadAnim && WeaponAnimInstance)
	//	{
	//		const float MontageTime = WeaponReloadAnim->GetSectionLength(2);
	//		WeaponAnimInstance->Montage_SetPlayRate(WeaponReloadAnim, MontageTime / AnimTime);
	//		WeaponAnimInstance->Montage_JumpToSection(FName("End"), WeaponReloadAnim);
	//	}
	//	//OnSimulateReloadFinish(AnimTime);
	//}
}

void ATacticalWeaponAttachment_Weapon::LocalSimulateContinueReload()
{
	//if (GetInventoryOwner())
	//{
	//	if (GetInventoryOwner()->GetFirstPersonMesh())
	//	{
	//		UAnimInstance* FPAnimInstance = GetInventoryOwner()->GetFirstPersonMesh()->GetAnimInstance();
	//		if (FPAnimInstance)
	//		{
	//			const float MontageLoopTime = FPReloadAnim->GetSectionLength(1);
	//			FPAnimInstance->Montage_SetPlayRate(FPReloadAnim, MontageLoopTime / ReloadTime);
	//		}
	//	}
	//	UAnimInstance* WeaponAnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
	//	//GetMesh()->GetAnimInstance()
	//	if (WeaponReloadAnim && WeaponAnimInstance)
	//	{
	//		const float MontageLoopTime = WeaponReloadAnim->GetSectionLength(1);
	//		WeaponAnimInstance->Montage_SetPlayRate(WeaponReloadAnim, MontageLoopTime / ReloadTime);
	//	}
	//	OnSimulateReloadContinue(ReloadTime);
	//}
}

void ATacticalWeaponAttachment_Weapon::NetMulti_Reload_Implementation()
{
	const ATacticalCharacter* InvOwner = GetWeaponOwner()->GetInventoryOwner();
	if (InvOwner && !InvOwner->IsLocallyControlled())
	{
		LocalSimulateStartReload();
	}
}

bool ATacticalWeaponAttachment_Weapon::NeedsReload() const
{
	return GetWeaponOwner() && GetWeaponOwner()->CanReload() && GetAmmoLoaded() < AmmoCapacity;
}
