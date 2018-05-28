// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalWPNAList_Muzzle.h"




void ATacticalWPNAList_Muzzle::AppendAttachmentList(TArray<TSubclassOf<ATacticalWeaponAttachment> >& InList) const
{
	for (TSubclassOf<ATacticalWeaponAttachment_Muzzle> Attachment : AttachmentList)
	{
		InList.AddUnique(Attachment);
	}
}
