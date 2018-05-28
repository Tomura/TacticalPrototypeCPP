// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Components/ActorComponent.h"
#include "TacticalInventoryComponent.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponEvent, class ATacticalWeapon*, Weapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponEventTimed, class ATacticalWeapon*, Weapon, float, Time);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TACTICALFPS_API UTacticalInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTacticalInventoryComponent();

	// Called when actor wants to init component
	virtual void InitializeComponent() override;
	// Called when the game starts
	virtual void BeginPlay() override;

	// // Called every frame
	// virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	ENetRole GetRole() const { return (GetOwner() ? GetOwner()->Role : ENetRole::ROLE_Authority); }

	// Called when Owner Pawn Restarts (needs to be called by pawn/character class)
	virtual void OnOwnerRestart();
	
	class ATacticalCharacter* GetTacticalOwner() const;
	
	class ATacticalInventory* CreateInventoryItem(TSubclassOf<class ATacticalInventory>InventoryClass);

	// Inventory stuff
	virtual bool AddInventory(class ATacticalInventory* ItemToAdd);
	virtual void RemoveInventory(class ATacticalInventory* ItemToRemove);
	virtual void DiscardInventory();

	UFUNCTION(Category = "Tactical Character", BlueprintCallable, BlueprintPure)
	class ATacticalWeapon* GetWeapon() const { return Weapon; }

	virtual void SetPendingFire();
	virtual void ClearPendingFire();
	virtual bool IsPendingFire() const;

	virtual void OnInventoryDestroyed(class ATacticalInventory* DestroyedInventory);
	virtual void OnWeaponDestroyed(class ATacticalWeapon* DestroyedWeapon);

	void GiveStartingInventory();


	//Weapon Stuff

	UFUNCTION(Category = "Inventory", BlueprintCallable)
	void SetCurrentWeapon(class ATacticalWeapon* DesiredWeapon, bool bChangeImmediately = false);
	void LocalSetCurrentWeapon(class ATacticalWeapon* DesiredWeapon, bool bChangeImmediately = false);
	UFUNCTION(reliable, client)
	void ClientSetCurrentWeapon(class ATacticalWeapon* DesiredWeapon, bool bChangeImmediately = false);
	void ClientSetCurrentWeapon_Implementation(class ATacticalWeapon* DesiredWeapon, bool bChangeImmediately = false);
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetCurrentWeapon(class ATacticalWeapon* DesiredWeapon, bool bChangeImmediately = false);
	void ServerSetCurrentWeapon_Implementation(class ATacticalWeapon* DesiredWeapon, bool bChangeImmediately = false);
	bool ServerSetCurrentWeapon_Validate(class ATacticalWeapon* DesiredWeapon, bool bChangeImmediately = false) { return true; }

	void ChangeWeapon();
	virtual void WeaponChanged();

	uint32 bPendingFire : 1;

	UPROPERTY(Replicated)
	class ATacticalWeapon* Weapon;

	UPROPERTY(Replicated)
	class ATacticalWeapon* PendingWeapon;

	UPROPERTY(Category = Inventory, EditDefaultsOnly)
	TSubclassOf<class ATacticalWeapon> DefaultWeapon;

	UPROPERTY(Category = Inventory, EditAnywhere)
	TSubclassOf<class ATacticalPrimaryWeapon> DefaultPrimaryWeapon;
	UPROPERTY(Category = Inventory, EditAnywhere)
	TSubclassOf<class ATacticalSidearm> DefaultSidearm;
	UPROPERTY(Category = Inventory, EditAnywhere)
	TSubclassOf<class ATacticalGrenade> DefaultGrenade;

	UPROPERTY(Replicated)
	class ATacticalPrimaryWeapon* PrimaryWeapon;
	UPROPERTY(Replicated)
	class ATacticalSidearm* Sidearm;
	UPROPERTY(Replicated)
	class ATacticalGrenade* Grenade;
	UPROPERTY(Replicated)
	class ATacticalInventory* Gadget1;
	UPROPERTY(Replicated)
	class ATacticalInventory* Gadget2;

	// Events

	//UPROPERTY(BlueprintAssignable, Category = "Inventory | Weapon")
	//FOnWeaponEvent OnWeaponActive;
	//UPROPERTY(BlueprintAssignable, Category = "Inventory | Weapon")
	//FOnWeaponEvent OnWeaponEquipped;
	//UPROPERTY(BlueprintAssignable, Category = "Inventory | Weapon")
	//FOnWeaponEvent OnWeaponUnequipped;
	//UPROPERTY(BlueprintAssignable, Category = "Inventory | Weapon")
	//FOnWeaponEvent OnWeaponInactive;
	UPROPERTY(BlueprintAssignable, Category = "Inventory | Weapon")
	FOnWeaponEventTimed OnWeaponReload;
	UPROPERTY(BlueprintAssignable, Category = "Inventory | Weapon")
	FOnWeaponEventTimed OnWeaponBeginEquip;
	UPROPERTY(BlueprintAssignable, Category = "Inventory | Weapon")
	FOnWeaponEventTimed OnWeaponBeginUnequip;

protected:
	UPROPERTY(Replicated)
	bool bTestBool;
};
