// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Blueprint/UserWidget.h"
#include "TacticalWPNAmmoSelection.generated.h"

/**
 * 
 */
UCLASS(abstract)
class TACTICALFPS_API UTacticalWPNAmmoSelection : public UUserWidget
{
	GENERATED_BODY()
public:
	class ATacticalWeaponPreviewScene* PreviewActor;

	UPROPERTY(Category = "Ammo", BlueprintReadOnly)
		TSubclassOf<class ATacticalWeapon> WeaponType;



	UFUNCTION(Category = "Ammo", BlueprintCallable)
	class ATacticalWeaponPreviewScene* GetPreviewActor() const { return PreviewActor; }

	void SetPreviewActor(class ATacticalWeaponPreviewScene* NewPreview);

	UFUNCTION(Category = "Ammo", BlueprintNativeEvent)
	void InitAmmoList(bool bSelectDefault = true);
	virtual void InitAmmoList_Implementation(bool bSelectDefault = true) {};

	UFUNCTION(Category = "Ammo", BlueprintCallable)
	virtual void OnClicked();

	UFUNCTION(Category = "Ammo", BlueprintImplementableEvent)
	void OnOtherClicked();

protected:
	UFUNCTION(Category = "Ammo", BlueprintPure)
		bool IsMainWeapon();
	
};
