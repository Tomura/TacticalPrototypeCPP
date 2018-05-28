// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TacticalWeapon.h"
#include "Weapons/Attachment/TacticalWeaponAttachment.h"
#include "TacticalWeaponAttachment_Weapon.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API ATacticalWeaponAttachment_Weapon : public ATacticalWeaponAttachment_Special
{
	GENERATED_BODY()



	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
		class USkeletalMeshComponent* Mesh;
	
public:

	ATacticalWeaponAttachment_Weapon(const FObjectInitializer& OI);

	friend class UTacticalWeaponStateGadgetFiring;
	friend class UTacticalWeaponStateGadgetReload;


	virtual void BeginPlay() override;


	UPROPERTY(Category = "Weapon|Reload", EditDefaultsOnly, BlueprintReadOnly)
	bool bReloadRoundsIndividually;

	UPROPERTY(Category = "Weapon|Reload", EditDefaultsOnly, BlueprintReadOnly)
	float ReloadTime;


	UPROPERTY(Category = "Weapon|Reload", EditDefaultsOnly, BlueprintReadOnly)
	float ReloadStartTime;


	UPROPERTY(Category = "Weapon|Reload", EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* WeaponReloadAnim;
	UPROPERTY(Category = "Weapon|Reload", EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* FPReloadAnim;


	UPROPERTY(Category = "Weapon|Firing", EditDefaultsOnly, BlueprintReadOnly)
		float TriggerDelay;
	UPROPERTY(Category = "Weapon|Firing", EditDefaultsOnly, BlueprintReadOnly)
		float FireRate;
	UPROPERTY(Category = "Weapon|Firing", EditDefaultsOnly, BlueprintReadOnly)
		class UTacticalAmmoType* AmmoType;
	UPROPERTY(Category = "Weapon|Firing", EditDefaultsOnly, BlueprintReadOnly)
		int32 BurstCount;
	UPROPERTY(Category = "Weapon|Firing", EditDefaultsOnly, BlueprintReadOnly)
		EWeaponFiringMode FiringMode;


	UPROPERTY(Category = "Weapon", EditDefaultsOnly, BlueprintReadWrite)
		bool bAttachmentActive;

	UFUNCTION(Category = "Weapon", BlueprintCallable)
		bool IsAttachmentActive() const { return bAttachmentActive; }


	UFUNCTION(Category = "Weapon", BlueprintCallable, BlueprintNativeEvent)
	int32 GetAmmoLoaded() const; 
	virtual int32 GetAmmoLoaded_Implementation() const { return AmmoLoaded; }

	UFUNCTION(Category = "Weapon", BlueprintCallable, BlueprintNativeEvent)
	int32 GetAmmoRemaining() const;
	virtual int32 GetAmmoRemaining_Implementation() const { return AmmoRemaining; }
	
	void OnEmpty();
	UFUNCTION(Category = "Weapon", BlueprintImplementableEvent)
	void LocalSimulateEmpty();

	virtual bool HasAmmoLoaded() const { return AmmoLoaded > 0; }
	virtual EWeaponFiringMode GetFiringMode() const { return FiringMode; }


	UFUNCTION(unreliable, NetMulticast)
	void NetMulti_SimulateFire();
	virtual void NetMulti_SimulateFire_Implementation();

	virtual void NativeLocalSimulateFire();
	UFUNCTION(Category = "Weapon", BlueprintImplementableEvent)
	void LocalSimulateFire();

	virtual void OnFire();
	UFUNCTION(Category = "Weapon", BlueprintImplementableEvent, meta=(DisplayName="OnFire"))
	void BP_OnFire();

	UFUNCTION(Category = "WeaponAttachment", BlueprintNativeEvent)
	void ConsumeAmmo();
	virtual void ConsumeAmmo_Implementation() { AmmoLoaded = FMath::Max(AmmoLoaded - 1, 0); };

	void LocalSimulateStartReload();
	void LocalSimulateStopReload();
	void LocalSimulateContinueReload();

	UFUNCTION(unreliable, NetMulticast)
	void NetMulti_Reload();
	virtual void NetMulti_Reload_Implementation();

	UPROPERTY()
	class UTacticalWeaponState* StateReloading;
	UPROPERTY()
	class UTacticalWeaponState* StateFiring;

	UPROPERTY(Category = "Weapon|Reload", EditDefaultsOnly, BlueprintReadOnly)
	bool bCanBulletStayInChamber;


public:
	virtual float GetReloadTime() const { return ReloadTime; }
	virtual bool NeedsReload() const;
	virtual bool ReloadRoundsIndividually() const { return false; }
	FORCEINLINE bool HasAnyAmmo() const { return (AmmoLoaded+AmmoRemaining) > 0; }

protected:

	UPROPERTY(Category = "Weapon", BlueprintReadWrite, Replicated)
	int32 AmmoLoaded;

	UPROPERTY(Category = "Weapon", BlueprintReadWrite, Replicated)
	int32 AmmoRemaining;

	UPROPERTY(Category = "Weapon", EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxAmmo;
	UPROPERTY(Category = "Weapon", EditDefaultsOnly, BlueprintReadOnly)
	int32 AmmoCapacity;

};
