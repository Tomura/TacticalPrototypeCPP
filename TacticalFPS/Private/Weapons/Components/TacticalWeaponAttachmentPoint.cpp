// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "Net/UnrealNetwork.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponAttachment.h"
#include "TacticalWeaponAttachmentPoint.h"
#include "TacticalWPNAList_Sight.h"
#include "TacticalWPNAList_Muzzle.h"
#include "TacticalWPNAList_Special.h"


// Sets default values for this component's properties
UTacticalWeaponAttachmentPoint::UTacticalWeaponAttachmentPoint()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);

	// ...
	CurrentAttachmentIdx = 0;
	CurrentAttachment = nullptr;
	Priority = 0;

	ArrowSize = 0.2f;
}


void UTacticalWeaponAttachmentPoint::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTacticalWeaponAttachmentPoint, CurrentAttachmentIdx);
	DOREPLIFETIME(UTacticalWeaponAttachmentPoint, bGadgetOnOff);
}

// Called when the game starts
void UTacticalWeaponAttachmentPoint::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if (!CurrentAttachment)
	{
		OnRepAttachmentClass();
	}
	
}


// Called every frame
void UTacticalWeaponAttachmentPoint::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

void UTacticalWeaponAttachmentPoint::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	DestroyCurrentAttachment();
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}




void UTacticalWeaponAttachmentPoint::DestroyCurrentAttachment()
{
	if (CurrentAttachment && CurrentAttachment->IsValidLowLevel() && !CurrentAttachment->IsPendingKill())
	{
		CurrentAttachment->Destroy();
		CurrentAttachment = nullptr;
	}
}

void UTacticalWeaponAttachmentPoint::SetCurrentAttachment(class ATacticalWeaponAttachment* NewAttachment)
{
	if (CurrentAttachment == NewAttachment)
		return;

	if (CurrentAttachment)
	{
		DestroyCurrentAttachment();
	}
	CurrentAttachment = NewAttachment;

	TSubclassOf<ATacticalWeaponAttachment> NewAttachmentClass = CurrentAttachment ? CurrentAttachment->GetClass() : nullptr;
	const int32 Idx = GetAttachmentIdx(NewAttachmentClass);

	SetAttachmentIdx(Idx);

	if (OnWeaponAttachmentChanged.IsBound())
	{
		OnWeaponAttachmentChanged.Broadcast(Idx);
	}
}

int32 UTacticalWeaponAttachmentPoint::GetAttachmentIdx(TSubclassOf<ATacticalWeaponAttachment> AttachmentClass) const
{
	TArray<TSubclassOf<class ATacticalWeaponAttachment>> AttachmentList;
	GetAttachmentList(AttachmentList);
	AttachmentList.Find(AttachmentClass);
	return -1;
}

void UTacticalWeaponAttachmentPoint::ToggleGadget()
{
	TurnGadgetOnOff(!bGadgetOnOff);
}

void UTacticalWeaponAttachmentPoint::TurnGadgetOnOff(bool bNewState)
{
	bGadgetOnOff = bNewState;
	if (CurrentAttachment)
	{
		CurrentAttachment->OnTurnOn(bNewState);
	}
}

void UTacticalWeaponAttachmentPoint::OnRepAttachmentClass()
{
	ATacticalWeapon* const WPNOwner = GetWeaponOwner();
	if (WPNOwner)
	{
		WPNOwner->CreateAttachment(this, CurrentAttachmentIdx);
	}
}

void UTacticalWeaponAttachmentPoint::OnRepGadgetOnOff()
{
	if (CurrentAttachment && CurrentAttachment->IsValidLowLevel())
	{
		CurrentAttachment->OnTurnOn(bGadgetOnOff);
	}
}

UTacticalWeaponAttachmentPoint_Muzzle::UTacticalWeaponAttachmentPoint_Muzzle() : Super()
{
	AttachPointName = NSLOCTEXT("AttachmentPoint", "DefaultNameMuzzle", "Muzzle");
	Priority = 3;
}

void UTacticalWeaponAttachmentPoint_Muzzle::GetAttachmentList(TArray<TSubclassOf<class ATacticalWeaponAttachment>>& AttachmentList) const
{
	//AttachmentList.Append(AllowedAttachments);

	for (TSubclassOf<ATacticalWeaponAttachment> ListAttachment : AllowedAttachments)
	{
		if (ListAttachment->IsChildOf(ATacticalWPNAList_Muzzle::StaticClass()))
		{
			const ATacticalWPNAList_Muzzle* ListCDO = GetDefault<ATacticalWPNAList_Muzzle>(ListAttachment);
			ListCDO->AppendAttachmentList(AttachmentList);
		}
		else
		{
			AttachmentList.AddUnique(ListAttachment);
		}
	}
}


UTacticalWeaponAttachmentPoint_Special::UTacticalWeaponAttachmentPoint_Special() : Super()
{
	AttachPointName = NSLOCTEXT("AttachmentPoint", "DefaultNameSpecial", "Special");
	Priority = 0;
}

void UTacticalWeaponAttachmentPoint_Special::GetAttachmentList(TArray<TSubclassOf<class ATacticalWeaponAttachment>>& AttachmentList) const
{
	//AttachmentList.Append(AllowedAttachments);

	for (TSubclassOf<ATacticalWeaponAttachment> ListAttachment : AllowedAttachments)
	{
		if (ListAttachment->IsChildOf(ATacticalWPNAList_Special::StaticClass()))
		{
			const ATacticalWPNAList_Special* ListCDO = GetDefault<ATacticalWPNAList_Special>(ListAttachment);
			ListCDO->AppendAttachmentList(AttachmentList);
		}
		else
		{
			AttachmentList.AddUnique(ListAttachment);
		}
	}
}



UTacticalWeaponAttachmentPoint_Sight::UTacticalWeaponAttachmentPoint_Sight() : Super()
{
	AttachPointName = NSLOCTEXT("AttachmentPoint", "DefaultNameSight", "Sight");
	Priority = 10;
}

void UTacticalWeaponAttachmentPoint_Sight::GetAttachmentList(TArray<TSubclassOf<class ATacticalWeaponAttachment>>& AttachmentList) const
{
	//AttachmentList.Append(AllowedAttachments);

	for (TSubclassOf<ATacticalWeaponAttachment> ListAttachment : AllowedAttachments)
	{
		if (ListAttachment->IsChildOf(ATacticalWPNAList_Sight::StaticClass()))
		{
			const ATacticalWPNAList_Sight* ListCDO = GetDefault<ATacticalWPNAList_Sight>(ListAttachment);
			ListCDO->AppendAttachmentList(AttachmentList);
		}
		else
		{
			AttachmentList.AddUnique(ListAttachment);
		}
	}

}
