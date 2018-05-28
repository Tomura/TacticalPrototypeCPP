// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "GameFramework/Actor.h"
#include "TacticalInventory.generated.h"



// Log Categories
DECLARE_LOG_CATEGORY_EXTERN(LogInventory, Log, All);

/**
 * 
 */
UCLASS(abstract, NotPlaceable, HideDropdown)
class TACTICALFPS_API ATacticalInventory : public AActor
{
	GENERATED_BODY()
	friend class ATacticalCharacter;

public:
	ATacticalInventory(const FObjectInitializer& ObjectInitializer);

	virtual void Destroyed() override;


	UFUNCTION(BlueprintCallable, Category = Inventory)
	FText GetItemName() const;

	UFUNCTION(BlueprintCallable, Category = Inventory)
		bool HideFromLoadoutList() const { return bHideFromLoadoutList; }

	//TODO Pickup Item related stuff

	/** Give this Inventory Item to Character */
	virtual void GiveTo(class ATacticalCharacter* Other);

	/* [server] Called when Inventory was given to Character */
	virtual void GivenTo(class ATacticalCharacter* NewOwner);
	//virtual void GivenTo(class UTacticalInventoryComponent* NewOwnerComp);
	
	/* [owner] Called by server when Inventory was given to Character */
	UFUNCTION(reliable, netmulticast, WithValidation)
	virtual void ClientGivenTo(class ATacticalCharacter* NewOwner);
	virtual void ClientGivenTo_Implementation(class ATacticalCharacter* NewOwner);
	virtual bool ClientGivenTo_Validate(class ATacticalCharacter* NewOwner);


	/* [server] Called when Item is removed from Inventory Manager */
	virtual void ItemRemovedFromOwner(class ATacticalCharacter* OldOwner);


	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnRemovedFromOwner(class ATacticalCharacter* OldOwner);

	void SetInventoryOwner(class ATacticalCharacter* NewOwner);

	UFUNCTION(Category = Inventory, BlueprintCallable)
	class ATacticalCharacter* GetInventoryOwner() const;

	UFUNCTION(Category = Inventory, BlueprintCallable)
	class UTacticalInventoryComponent* GetInventoryComponent() const;

	UFUNCTION(Category = Inventory, BlueprintCallable)
	bool IsOwnerLocallyControlled() const;
	UFUNCTION(Category = Inventory, BlueprintCallable)
	bool IsOwnerPlayerControlled() const;

protected:

	UPROPERTY(Category = Inventory, BlueprintReadWrite, EditAnywhere)
	FText ItemName;

	UPROPERTY(Category = Inventory, BlueprintReadWrite, EditDefaultsOnly)
	bool bHideFromLoadoutList;

	UPROPERTY(Category = Attachment, EditDefaultsOnly)
	FName AttachSocketName;

	FTimerHandle TimerDetachFromController;

	UPROPERTY(ReplicatedUsing = OnRep_AttachPoint)
	FName CurrentAttachmentPoint;
	UFUNCTION()
	void OnRep_AttachPoint();
};
