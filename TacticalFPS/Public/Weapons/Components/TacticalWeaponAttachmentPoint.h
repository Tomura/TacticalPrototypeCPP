// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Components/SceneComponent.h"
#include "TacticalTypes.h"
#include "TacticalWeaponAttachmentPoint.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponAttachmentChangedDelegate, int32, NewAttachmentIdx);

UCLASS(abstract, ClassGroup = (Weapon), meta = (BlueprintSpawnableComponent), hideCategories = (ArrowComponent))
class TACTICALFPS_API UTacticalWeaponAttachmentPoint : public UArrowComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTacticalWeaponAttachmentPoint();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	UFUNCTION(Category = "Weapon | Attachment", BlueprintPure)
	virtual void GetAttachmentList(TArray<TSubclassOf<class ATacticalWeaponAttachment>>& AttachmentList) const { return; }
	
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	void DestroyCurrentAttachment();

	UFUNCTION(Category = "Weapon | Attachment", BlueprintCallable)
	class ATacticalWeaponAttachment* GetCurrentAttachment() const { return CurrentAttachment; }
	void SetCurrentAttachment(class ATacticalWeaponAttachment* NewAttachment);


	UFUNCTION(Category = "Weapon | Attachment", BlueprintCallable)
		const FText& GetAttachPointName() const { return AttachPointName; }

	UFUNCTION(Category = "Weapon | Attachment", BlueprintCallable)
	virtual int32 GetAttachmentIdx(TSubclassOf<class ATacticalWeaponAttachment> AttachmentClass) const;

	int32 GetPriority() const { return Priority; }

	UPROPERTY(Category = "Weapon | Attachment", BlueprintAssignable)
	FOnWeaponAttachmentChangedDelegate OnWeaponAttachmentChanged;

	FORCEINLINE class ATacticalWeapon* GetWeaponOwner() const 
	{
		return (GetOwner() ? Cast<ATacticalWeapon>(GetOwner()) : nullptr);
	}

	void SetAttachmentIdx(uint8 inIdx) { CurrentAttachmentIdx = inIdx; }

	void ToggleGadget();
	void TurnGadgetOnOff(bool bNewState);


protected:
	UPROPERTY(Category = Attachment, EditDefaultsOnly)
	int32 Priority;

	UPROPERTY(Category = Attachment, EditDefaultsOnly)
		FText AttachPointName;

	UPROPERTY(ReplicatedUsing = OnRepAttachmentClass)
	uint8 CurrentAttachmentIdx;
	UPROPERTY(ReplicatedUsing = OnRepGadgetOnOff)
	bool bGadgetOnOff;

	UPROPERTY()
	class ATacticalWeaponAttachment* CurrentAttachment;

	UFUNCTION()
		void OnRepAttachmentClass();
	UFUNCTION()
		void OnRepGadgetOnOff();


};



UCLASS(ClassGroup = (Weapon), meta = (BlueprintSpawnableComponent, DisplayName = "Weapon Attachment Point (Muzzle)"))
class TACTICALFPS_API UTacticalWeaponAttachmentPoint_Muzzle : public UTacticalWeaponAttachmentPoint
{
	GENERATED_BODY()


public:

	UTacticalWeaponAttachmentPoint_Muzzle();
	virtual void GetAttachmentList(TArray<TSubclassOf<class ATacticalWeaponAttachment>>& AttachmentList) const override;

protected:
	UPROPERTY(Category = "Attachment", EditDefaultsOnly)
		TArray<TSubclassOf<class ATacticalWeaponAttachment_Muzzle>> AllowedAttachments;

};


UCLASS(ClassGroup = (Weapon), meta = (BlueprintSpawnableComponent, DisplayName = "Weapon Attachment Point (Sight)"))
class TACTICALFPS_API UTacticalWeaponAttachmentPoint_Sight : public UTacticalWeaponAttachmentPoint
{
	GENERATED_BODY()

public:
	UTacticalWeaponAttachmentPoint_Sight();
	virtual void GetAttachmentList(TArray<TSubclassOf<class ATacticalWeaponAttachment>>& AttachmentList) const override;

protected:

	UPROPERTY(Category = "Attachment", EditDefaultsOnly)
		TArray<TSubclassOf<class ATacticalWeaponAttachment_Sight>> AllowedAttachments;

};


UCLASS(ClassGroup = (Weapon), meta = (BlueprintSpawnableComponent, DisplayName = "Weapon Attachment Point (Special)"))
class TACTICALFPS_API UTacticalWeaponAttachmentPoint_Special : public UTacticalWeaponAttachmentPoint
{
	GENERATED_BODY()

public:
	UTacticalWeaponAttachmentPoint_Special();
	virtual void GetAttachmentList(TArray<TSubclassOf<class ATacticalWeaponAttachment>>& AttachmentList) const override;

protected:
	UPROPERTY(Category = "Attachment", EditDefaultsOnly)
		TArray<TSubclassOf<class ATacticalWeaponAttachment_Special>> AllowedAttachments;
};
