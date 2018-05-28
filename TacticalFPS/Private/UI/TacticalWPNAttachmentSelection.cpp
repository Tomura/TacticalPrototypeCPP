// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponAttachment.h"
#include "TacticalWeaponAttachmentPoint.h"
#include "TacticalWPNAttachmentSelection.h"
#include "TacticalPlayerController.h"
#include "TacticalWeaponPreviewScene.h"




void UTacticalWPNAttachmentSelection::SetAttachmentPoint(class UTacticalWeaponAttachmentPoint* NewAttachPoint)
{
	AttachPoint = NewAttachPoint;
}

void UTacticalWPNAttachmentSelection::SetPreviewActor(class ATacticalWeaponPreviewScene* NewPreview)
{
	PreviewActor = NewPreview;
}

void UTacticalWPNAttachmentSelection::OnClicked()
{
	if (PreviewActor)
	{
		for (UTacticalWPNAttachmentSelection* TestWidget : PreviewActor->AttachmentWidgets)
		{
			if (TestWidget != this)
			{
				TestWidget->OnOtherClicked();
			}
		}
		if (PreviewActor->AmmoWidget)
		{
			PreviewActor->AmmoWidget->OnOtherClicked();
		}
	}
}


