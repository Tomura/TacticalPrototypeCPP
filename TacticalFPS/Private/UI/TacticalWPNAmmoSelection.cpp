// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalWeaponPreviewScene.h"
#include "TacticalWPNAmmoSelection.h"
#include "TacticalWeapon.h"
#include "TacticalWPNAttachmentSelection.h"



void UTacticalWPNAmmoSelection::SetPreviewActor(class ATacticalWeaponPreviewScene* NewPreview)
{
	PreviewActor = NewPreview;
}

void UTacticalWPNAmmoSelection::OnClicked()
{
	if (PreviewActor)
	{
		for (UTacticalWPNAttachmentSelection* TestWidget : PreviewActor->AttachmentWidgets)
		{
			TestWidget->OnOtherClicked();
		}
	}
}

bool UTacticalWPNAmmoSelection::IsMainWeapon()
{
	if (WeaponType)
	{
		if (WeaponType->IsChildOf(ATacticalPrimaryWeapon::StaticClass()))
		{
			return true;
		}
	}
	return false;
}
