// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalWPNAList_Sight.h"




void ATacticalWPNAList_Sight::AppendAttachmentList(TArray<TSubclassOf<ATacticalWeaponAttachment> >& InList) const
{
	for (TSubclassOf<ATacticalWeaponAttachment_Sight> Attachment : AttachmentList)
	{
		InList.AddUnique(Attachment);
	}
}
