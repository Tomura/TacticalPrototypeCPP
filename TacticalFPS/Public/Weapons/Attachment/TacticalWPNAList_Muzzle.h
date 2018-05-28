// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "TacticalWeaponAttachment.h"
#include "TacticalWPNAList_Muzzle.generated.h"

/**
 * 
 */
UCLASS(abstract, hideCategories = ("Weapon Attachment", Replication, Actor, Input, Rendering, Muzzle, "Actor Tick"))
class TACTICALFPS_API ATacticalWPNAList_Muzzle : public ATacticalWeaponAttachment_Muzzle
{
	GENERATED_BODY()
	
public:
	void AppendAttachmentList(TArray<TSubclassOf<ATacticalWeaponAttachment> >& InList) const;

protected:
	UPROPERTY(Category = "Attachment List", EditDefaultsOnly)
		TArray<TSubclassOf<ATacticalWeaponAttachment_Muzzle> > AttachmentList;
	
	
};
