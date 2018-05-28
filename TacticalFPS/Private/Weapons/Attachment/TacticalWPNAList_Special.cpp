// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalWPNAList_Special.h"




void ATacticalWPNAList_Special::AppendAttachmentList(TArray<TSubclassOf<ATacticalWeaponAttachment> >& InList) const
{
	for (TSubclassOf<ATacticalWeaponAttachment_Special> Attachment : AttachmentList)
	{
		InList.AddUnique(Attachment);
	}
}
