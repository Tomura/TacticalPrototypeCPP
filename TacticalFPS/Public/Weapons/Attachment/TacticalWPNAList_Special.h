// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "TacticalWeaponAttachment.h"
#include "TacticalWPNAList_Special.generated.h"

/**
 * 
 */
UCLASS(abstract, hideCategories = ("Weapon Attachment", Replication, Actor, Input, Rendering, Special, "Actor Tick"))
class TACTICALFPS_API ATacticalWPNAList_Special : public ATacticalWeaponAttachment_Special
{
	GENERATED_BODY()
	
public:
	void AppendAttachmentList(TArray<TSubclassOf<ATacticalWeaponAttachment> >& InList) const;
	
protected:
	UPROPERTY(Category = "Attachment List", EditDefaultsOnly)
	TArray<TSubclassOf<ATacticalWeaponAttachment_Special> > AttachmentList;
};
