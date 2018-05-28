// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalCharacter.h"
#include "TacticalPlayerController.h"
#include "TacticalInventory.h"
#include "TacticalWeapon.h"
#include "TacticalGrenade.h"
#include "TacticalInventoryComponent.h"
#include "TacticalGadgetInterface.h"
#include "TacticalWeaponAttachmentPoint.h"


// Sets default values for this component's properties
UTacticalInventoryComponent::UTacticalInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true;

	SetIsReplicated(true);
	bAutoActivate = true;
	// ...

	Weapon = nullptr;
	PendingWeapon = nullptr;
	PrimaryWeapon = nullptr;
	Sidearm = nullptr;
	Grenade = nullptr;
	Gadget1 = nullptr;
	Gadget2 = nullptr;
}

void UTacticalInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

// Network
void UTacticalInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTacticalInventoryComponent, PrimaryWeapon);
	DOREPLIFETIME(UTacticalInventoryComponent, Sidearm);
	DOREPLIFETIME(UTacticalInventoryComponent, Weapon);
	DOREPLIFETIME(UTacticalInventoryComponent, Grenade);
	DOREPLIFETIME(UTacticalInventoryComponent, Gadget1);
	DOREPLIFETIME(UTacticalInventoryComponent, Gadget2);
}

// Called when the game starts
void UTacticalInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


void UTacticalInventoryComponent::OnOwnerRestart()
{
	if (GetTacticalOwner() == nullptr)
		return;


	GiveStartingInventory();
	if (GetTacticalOwner()->IsLocallyControlled())
	{
		if (PrimaryWeapon)
		{
			SetCurrentWeapon(PrimaryWeapon);
		}
		else if (Sidearm)
		{
			SetCurrentWeapon(Sidearm);
		}
	}
}

class ATacticalCharacter* UTacticalInventoryComponent::GetTacticalOwner() const
{
	return (GetOwner() ? Cast<class ATacticalCharacter>(GetOwner()) : nullptr);
}

class ATacticalInventory* UTacticalInventoryComponent::CreateInventoryItem(TSubclassOf<class ATacticalInventory>InventoryClass)
{
	if (GetOwner() == nullptr)
		return nullptr;
	if (!GetOwner()->IsValidLowLevel())
		return nullptr;


	ATacticalInventory* NewItem = nullptr;
	if (InventoryClass)
	{
		UE_LOG(LogTemp, Log, TEXT("Trying to create item: %s"), *InventoryClass->GetName());
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		NewItem = GetWorld()->SpawnActor<ATacticalInventory>(InventoryClass, GetOwner()->GetActorLocation(), GetOwner()->GetActorRotation(), SpawnParams);
		if (NewItem)
		{
			if (!AddInventory(NewItem))
			{
				NewItem->Destroy();
				NewItem = nullptr;
			}
			if(NewItem)
			{
				UE_LOG(LogTemp, Log, TEXT("Successfully added item: %s"), *NewItem->GetName());
			}
		}
	}
	return NewItem;
}

bool UTacticalInventoryComponent::AddInventory(class ATacticalInventory* ItemToAdd)
{
	bool bSuccess = false;
	if (ItemToAdd && !ItemToAdd->IsPendingKill())
	{
		if (!bSuccess)
		{
			if (ItemToAdd->GetClass()->ImplementsInterface(UTacticalGadgetInterface::StaticClass()))
			{
				if (Gadget1 == nullptr)
				{
					UE_LOG(LogTemp, Log, TEXT("Gadget 1 null"));
					Gadget1 = ItemToAdd;
					bSuccess = true;
				}
				else if (Gadget2 == nullptr)
				{
					UE_LOG(LogTemp, Log, TEXT("Gadget 2 null"));
					Gadget2 = ItemToAdd;
					bSuccess = true;
				}
			}
		}

		if(!bSuccess)
		{
			if (ATacticalPrimaryWeapon* PrimaryWeaponToAdd = Cast<ATacticalPrimaryWeapon>(ItemToAdd))
			{
				if (!PrimaryWeapon)
				{
					PrimaryWeapon = PrimaryWeaponToAdd;
					bSuccess = true;
				}
			}
		}

		if(!bSuccess)
		{
			if (ATacticalSidearm* SidearmToAdd = Cast<ATacticalSidearm>(ItemToAdd))
			{
				if (!Sidearm)
				{
					Sidearm = SidearmToAdd;
					bSuccess = true;
				}
			}
		}

		//if(!bSuccess)
		//{
		//	if (ATacticalGrenade* GrenadeToAdd = Cast<ATacticalGrenade>(ItemToAdd))
		//	{
		//		if (!Grenade)
		//		{
		//			Grenade = GrenadeToAdd;
		//			bSuccess = true;
		//		}
		//	}
		//}
		


		if (bSuccess)
		{
			ItemToAdd->GivenTo(this->GetTacticalOwner());
			return true;
		}
	}
	return false;
}

void UTacticalInventoryComponent::RemoveInventory(class ATacticalInventory* ItemToRemove)
{
	if (ItemToRemove)
	{
		bool bFound = false;
		if (ItemToRemove == PrimaryWeapon)
		{
			PrimaryWeapon = nullptr;
			bFound = true;
		}
		if (ItemToRemove == Sidearm)
		{
			Sidearm = nullptr;
			bFound = true;
		}
		if (ItemToRemove == Grenade)
		{
			Grenade = nullptr;
			bFound = true;
		}
		if(ItemToRemove == Gadget1)
		{
			Gadget1 = nullptr;
			bFound = true;
		}
		if(ItemToRemove == Gadget1)
		{
			Gadget2 = nullptr;
			bFound = true;
		}

		if (bFound)
		{
			ItemToRemove->ItemRemovedFromOwner(GetTacticalOwner());
		}

		if (ItemToRemove == Weapon)
		{
			Weapon = nullptr;
		}
		// todo: if is alive and changed something that was currently equipped was lost switch to best
		if (/*ISAlive() &&*/ Weapon == nullptr)
		{
			if (PendingWeapon && PendingWeapon != ItemToRemove)
			{
				//ChangeWeapon();
			}
			else
			{
				// switch to best weapon
				// SwitchToBestWeapon();
			}
		}
	}
}

void UTacticalInventoryComponent::DiscardInventory()
{
	if (PrimaryWeapon) PrimaryWeapon->Destroy();
	if (Sidearm) Sidearm->Destroy();
	if (Grenade) Grenade->Destroy();
	if (Gadget1) Gadget1->Destroy();
	if (Gadget2) Gadget2->Destroy();

	// null all pointers
	PrimaryWeapon = nullptr;
	Sidearm = nullptr;
	Grenade = nullptr;
	Gadget1 = nullptr;
	Gadget2 = nullptr;

	Weapon = nullptr;
	PendingWeapon = nullptr;
}


void UTacticalInventoryComponent::GiveStartingInventory()
{
	if (GetTacticalOwner() == nullptr)
		return;

	if (GetRole() != ROLE_Authority)
		return;


	if (ATacticalPlayerController* myPC = Cast<ATacticalPlayerController>(GetTacticalOwner()->GetController()))
	{
		FTacticalLoadout SpawnLoadout = myPC->GetLoadout();

		// Main Weapon
		CreateInventoryItem(SpawnLoadout.MainWeaponType);
		if (PrimaryWeapon)
		{
			TArray<UTacticalWeaponAttachmentPoint*> AttachPoints;
			PrimaryWeapon->GetAttachmentPoints(AttachPoints);

			for (UTacticalWeaponAttachmentPoint* P : AttachPoints)
			{
				const FString& AttachName = P->GetName();
				const int32 Idx = SpawnLoadout.GetAttachmentIdxFromName(AttachName, EWeaponClass::MainWeapon);
				PrimaryWeapon->CreateAttachment(P, Idx);
			}
		}
		// Sidearm
		CreateInventoryItem(SpawnLoadout.SidearmType);
		if (Sidearm)
		{
			TArray<UTacticalWeaponAttachmentPoint*> AttachPoints;
			Sidearm->GetAttachmentPoints(AttachPoints);

			for (UTacticalWeaponAttachmentPoint* P : AttachPoints)
			{
				const FString& AttachName = P->GetName();
				const int32 Idx = SpawnLoadout.GetAttachmentIdxFromName(AttachName, EWeaponClass::Sidearm);
				Sidearm->CreateAttachment(P, Idx);
			}
		}

		// Gadgets
		CreateInventoryItem(SpawnLoadout.Gadget1);
		CreateInventoryItem(SpawnLoadout.Gadget2);
	}
	else
	{
		CreateInventoryItem(DefaultPrimaryWeapon);
		CreateInventoryItem(DefaultSidearm);
		//CreateInventoryItem(DefaultGrenade);
	}
}


//// Called every frame
//void UTacticalInventoryComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
//{
//	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
//
//	// ...
//}




void UTacticalInventoryComponent::SetCurrentWeapon(class ATacticalWeapon* DesiredWeapon, bool bChangeImmediately /* = false */)
{
	if (!GetTacticalOwner()) return;
	
	if (GetTacticalOwner()->CanSwitchWeapon())
	{
		if (GetRole() == ROLE_Authority)
		{
			// if server then call from client
			ClientSetCurrentWeapon(DesiredWeapon);
		}
		else if (!GetTacticalOwner()->IsLocallyControlled())
		{
			// if is not locally controlled this is a simulated client
			// so do nothing
			UE_LOG(LogTemp, Warning, TEXT("SetCurrentWeapon called on simulated proxy. Call on remote client"))
		}
		else
		{
			// should be client
			LocalSetCurrentWeapon(DesiredWeapon, bChangeImmediately);
			ServerSetCurrentWeapon(DesiredWeapon, bChangeImmediately);
		}
	}
}

void UTacticalInventoryComponent::LocalSetCurrentWeapon(class ATacticalWeapon* DesiredWeapon, bool bChangeImmediately /* = false */)
{
	if (GetTacticalOwner() && GetTacticalOwner()->IsAlive())
	{
		if (DesiredWeapon != nullptr && (DesiredWeapon->GetInventoryOwner() == nullptr || (GetRole() == ROLE_Authority && DesiredWeapon->GetInventoryComponent() != this)))
		{
			UE_LOG(LogInventory, Warning, TEXT("LocalSetCurrentWeapon: Weapon is invalid or has invalid owner."));
		}
		else
		{
			ATacticalWeapon* OldWeapon = Weapon;

			if (OldWeapon && OldWeapon == DesiredWeapon)
			{
				// We are switching to current Weapon -> do nothing?
				//Add additional checks
				return;
			}

			// Set New Weapon as Pending
			PendingWeapon = DesiredWeapon;

			// Handle Old Weapon First (Putting Down)
			if (OldWeapon && !OldWeapon->IsPendingKill() && !bChangeImmediately)
			{
				OldWeapon->TryUnequip();
			}
			else
			{
				ChangeWeapon();
			}
		}
	}
}

void UTacticalInventoryComponent::ClientSetCurrentWeapon_Implementation(class ATacticalWeapon* DesiredWeapon, bool bChangeImmediately /* = false */)
{
	if (GetTacticalOwner() == nullptr)
		return;

	if (DesiredWeapon && !DesiredWeapon->IsValidLowLevel())
	{
		DesiredWeapon = nullptr;
	}

	LocalSetCurrentWeapon(DesiredWeapon);
	if (GetRole() < ROLE_Authority)
	{
		ServerSetCurrentWeapon(DesiredWeapon, bChangeImmediately);
	}
}

void UTacticalInventoryComponent::ServerSetCurrentWeapon_Implementation(class ATacticalWeapon* DesiredWeapon, bool bChangeImmediately /* = false */)
{
	LocalSetCurrentWeapon(DesiredWeapon, bChangeImmediately);
}

void UTacticalInventoryComponent::ChangeWeapon()
{
	ATacticalWeapon* OldWeapon = Weapon;

	//TODO: maybe check if a NULL switch is allowed

	UE_LOG(LogInventory, Verbose, TEXT("Switch from %s to %s"),
		OldWeapon ? *OldWeapon->GetName() : *FString(TEXT("No Weapon")),
		PendingWeapon ? *PendingWeapon->GetName() : *FString(TEXT("No Weapon")));

	Weapon = PendingWeapon;
	//TODO Play weapon switch animations notify Blueprint

	// Empty PendingFire
	bPendingFire = false;

	// if PendingWeapon is a weapon handle activation, etc
	if (PendingWeapon)
	{
		PendingWeapon->SetInventoryOwner(GetTacticalOwner());
		//PendingWeapon->Activate(OldWeapon ? false : true);
		PendingWeapon->Activate(false);
		PendingWeapon = nullptr;
	}
	WeaponChanged();
}

void UTacticalInventoryComponent::WeaponChanged()
{
	// Stuff that happens on WeaponChange
	// Probably best to add a EventHandler/Delegate
}




void UTacticalInventoryComponent::SetPendingFire()
{
	bPendingFire = true;
}

void UTacticalInventoryComponent::ClearPendingFire()
{
	bPendingFire = false;
}

bool UTacticalInventoryComponent::IsPendingFire() const
{
	return bPendingFire;
}

void UTacticalInventoryComponent::OnInventoryDestroyed(class ATacticalInventory* DestroyedInventory)
{

}

void UTacticalInventoryComponent::OnWeaponDestroyed(class ATacticalWeapon* DestroyedWeapon)
{
	if (DestroyedWeapon == nullptr)
	{
		return;
	}

	if (DestroyedWeapon == PrimaryWeapon)
	{
		PrimaryWeapon = nullptr;
	}
	if (DestroyedWeapon == Sidearm)
	{
		Sidearm = nullptr;
	}

	if (GetTacticalOwner() && GetOwner()->HasAuthority() && !GetOwner()->IsPendingKillOrUnreachable())
	{
		if (GetTacticalOwner()->GetWeapon() != nullptr && GetTacticalOwner()->GetWeapon() == DestroyedWeapon)
		{
			SetCurrentWeapon(nullptr, true);
		}
	}
}