// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "TacticalWPNAttachmentSelection.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalWPNAttachmentSelection : public UUserWidget
{
	GENERATED_BODY()
	
	

public:
	UPROPERTY(Category = "Attachment", BlueprintReadOnly)
	class UTacticalWeaponAttachmentPoint* AttachPoint;

	class ATacticalWeaponPreviewScene* PreviewActor;

	void SetAttachmentPoint(class UTacticalWeaponAttachmentPoint* NewAttachPoint);
	void SetPreviewActor(class ATacticalWeaponPreviewScene* NewPreview);
	
	UFUNCTION(Category = "Attachment", BlueprintNativeEvent)
	void InitAttachmentList(bool bSelectDefault = true);
	virtual void InitAttachmentList_Implementation(bool bSelectDefault = true) {};

	UFUNCTION(Category = "Attachment", BlueprintCallable)
	virtual void OnClicked();

	UFUNCTION(Category = "Attachment", BlueprintImplementableEvent)
	void OnOtherClicked();
};
