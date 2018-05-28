// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalCharacter.h"
#include "TacticalInventoryComponent.h"
#include "TacticalInventory.h"
#include "Net/UnrealNetwork.h"


DEFINE_LOG_CATEGORY(LogInventory);

ATacticalInventory::ATacticalInventory(const FObjectInitializer& OI)
	: Super(OI)
{
	SetReplicates(true);
	AttachSocketName = NAME_None;
}

void ATacticalInventory::Destroyed()
{
	//UE_LOG(LogInventory, Verbose, TEXT("Destroying %s"), *GetName());
	//Remove from inventory manager/Pawn
	if (GetInventoryOwner())
	{
		GetInventoryOwner()->GetInventory()->RemoveInventory(this);
	}

	Super::Destroyed();
}

void ATacticalInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATacticalInventory, CurrentAttachmentPoint);
}

FText ATacticalInventory::GetItemName() const
{
	return ItemName;
}


void ATacticalInventory::GiveTo(ATacticalCharacter* Other)
{
	if (Other)
	{
		UE_LOG(LogInventory, Verbose, TEXT("Item give to %s"), *Other->GetName());
		Other->GetInventory()->AddInventory(this);
	}
}

//WIP
void ATacticalInventory::GivenTo(ATacticalCharacter* NewOwner)
{
	SetInventoryOwner(NewOwner);

	UE_LOG(LogInventory, Verbose, TEXT("Item given to %s"), NewOwner ? *NewOwner->GetName() : *FString(TEXT("NULL")));
	
	//todo: Maybe notify Controller and Character about new Item
	if (NewOwner)
	{
		FAttachmentTransformRules AttachRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false);
		AttachToComponent(NewOwner->GetMesh(), AttachRules, AttachSocketName);
		CurrentAttachmentPoint = AttachSocketName;
	}
	//BP_OnGivenTo(NewOwner);
	//Notify Client about new Inventory Items
	ClientGivenTo(NewOwner);
}

//WIP
void ATacticalInventory::ClientGivenTo_Implementation(ATacticalCharacter* NewOwner)
{
	SetInventoryOwner(NewOwner);
	//AttachRootComponentToActor(NewOwner, AttachSocketName, EAttachLocation::SnapToTarget);
	if (NewOwner)
	{
		FAttachmentTransformRules AttachRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false);
		AttachToComponent(NewOwner->GetMesh(), AttachRules, AttachSocketName);
		CurrentAttachmentPoint = AttachSocketName;
	}
	//Notify Controller about new Item (needs Interface)

}
bool ATacticalInventory::ClientGivenTo_Validate(ATacticalCharacter* NewOwner)
{
	return true;
}

//void ABRSInventory::ItemRemovedFromInvManager(UBRSInventoryManager* OldInventoryManager)
//{
//	BP_OnRemovedFromInventoryManager(OldInventoryManager);
//}
void ATacticalInventory::ItemRemovedFromOwner(ATacticalCharacter* OldOwner)
{
	SetInventoryOwner(nullptr);
	BP_OnRemovedFromOwner(OldOwner);
}

void ATacticalInventory::SetInventoryOwner(ATacticalCharacter* NewOwner)
{
	SetOwner(NewOwner);
}

ATacticalCharacter* ATacticalInventory::GetInventoryOwner() const
{
	return GetOwner() ? Cast<ATacticalCharacter>(GetOwner()) : nullptr;
}

class UTacticalInventoryComponent* ATacticalInventory::GetInventoryComponent() const
{
	return (GetInventoryOwner() ? GetInventoryOwner()->GetInventory() : nullptr);
}

bool ATacticalInventory::IsOwnerPlayerControlled() const
{
	return GetInventoryOwner() && Cast<APlayerController>(GetInventoryOwner()->Controller);
}

bool ATacticalInventory::IsOwnerLocallyControlled() const
{
	return GetInventoryOwner() && GetInventoryOwner()->IsLocallyControlled();
}

void ATacticalInventory::OnRep_AttachPoint()
{
	if (GetInventoryOwner())
	{
		FAttachmentTransformRules AttachRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false);
		AttachToComponent(GetInventoryOwner()->GetMesh(), AttachRules, CurrentAttachmentPoint);
	}
}
