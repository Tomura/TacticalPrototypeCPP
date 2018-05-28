// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "TacticalInventory.h"
#include "Curves/CurveFloat.h"
#include "TacticalTypes.h"
#include "TacticalWeapon.generated.h"

#define WEAPON_MIN_TRACE_RANGE 100.f


UENUM(BlueprintType)
enum class EWeaponFiringMode : uint8
{
	FM_Auto		UMETA(DisplayName = "Auto"),
	FM_Burst	UMETA(DisplayName = "Burst"),
	FM_Single	UMETA(DisplayName = "Single"),
	FM_None		UMETA(DisplayName = "None")
};

struct FPendingAnimRecoil
{
	FPendingAnimRecoil(class UTacticalRecoilCurve* newAnim, bool bRandomizeY, bool bRandomizeYaw, float newScale);

	float YawFactor;
	float YFactor;
	float Time;
	float Duration;
	class UTacticalRecoilCurve* Anim;
	bool bPendingKill;
	float Scale;

	void Tick(float DeltaTime);
};



UCLASS(abstract, NotPlaceable, HideDropdown)
class TACTICALFPS_API ATacticalWeapon : public ATacticalInventory
{
	GENERATED_BODY()
	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UParticleSystemComponent* BulletEjectPSC;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	//class UChildActorComponent* AmmoDisplayComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UBillboardComponent* WidgetPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* BaseAimPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* SightAimPoint;

public:	

	friend class UTacticalWeaponState;
	friend class UTacticalWeaponState_WPN;
	friend class UTacticalWeaponStateActive;
	friend class UTacticalWeaponStateInactive;
	friend class UTacticalWeaponStateEquipping;
	friend class UTacticalWeaponStateUnequipping;
	friend class UTacticalWeaponStateFiring;
	friend class UTacticalWeaponStateReloading;
	friend class UTacticalWeaponStateObstructed;

	friend class ATacticalWeaponStateGadget;
	friend class UTacticalWeaponStateGadgetReload;
	friend class UTacticalWeaponStateGadgetFiring;

	friend class ATacticalWeaponPreviewScene;

	// Sets default values for this actor's properties
	ATacticalWeapon(const FObjectInitializer& OI);

	virtual void PostInitializeComponents() override;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	virtual void Destroyed() override;
	
	virtual void ItemRemovedFromOwner(class ATacticalCharacter* OldOwner) override;
	virtual void OnOwnerDied();

	USkeletalMeshComponent* GetMesh() const { return Mesh; }

	

public:

	virtual void OnFirstPerson();
	virtual void OnThirdPerson();

	UFUNCTION(Category = "Weapon", BlueprintImplementableEvent, meta = (DisplayName = "On First Person"))
	void BP_OnFirstPerson();
	UFUNCTION(Category = "Weapon", BlueprintImplementableEvent, meta = (DisplayName = "On Third Person"))
	void BP_OnThirdPerson();

	void OnStartAiming();
	void OnStopAiming();

	UFUNCTION(Category = "Weapon", BlueprintImplementableEvent, meta = (DisplayName = "On Start Aiming"))
	void BP_OnStartAiming();
	UFUNCTION(Category = "Weapon", BlueprintImplementableEvent, meta = (DisplayName = "On Stop Aiming"))
	void BP_OnStopAiming();


	UParticleSystemComponent* GetMuzzleFlashParticle() const;

	UFUNCTION(Category = "Weapon", BlueprintNativeEvent, meta = (DisplayName = "Get Muzzle flash Particle"))
	UParticleSystemComponent* BP_GetMuzzleFlashParticle() const;
	virtual UParticleSystemComponent* BP_GetMuzzleFlashParticle_Implementation() const { return nullptr; }

	//////////////////////////
	// Getters/Stats

	UFUNCTION(Category = "Weapon", BlueprintCallable, BlueprintPure)
	virtual FVector GetWidgetPoint() const { return WidgetPoint->GetComponentLocation(); }
	virtual UBillboardComponent* GetWidgetPointComponent() const { return WidgetPoint; }

	FVector GetWeaponOffset(bool bAimed = false) const;


	UFUNCTION(Category = "Weapon", BlueprintCallable)
	void GetAttachmentPoints(TArray<class UTacticalWeaponAttachmentPoint*>& AttachPoints) const;
	class ATacticalWeaponAttachment_Sight* GetSightAttachment() const;
	class ATacticalWeaponAttachment_Muzzle* GetMuzzleAttachment() const;


	/** Returns whether crosshair should be displayed or not */
	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintPure)
	virtual bool ShowCrosshair() const;



	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintPure)
	virtual float GetAimZoomModifier() const;

	// Spread
	/** Returns current bullet spread with all calculations applied */
	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintPure)
	float GetSpread() const;
	/** Returns current bullet spread with all calculations applied */
	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintPure)
	float GetCombinedSpread() const;

	/** Returns the maximum spread for this weapon without external influences, but with addons */
	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintPure)
	float GetMaxSpread() const;
	/** Returns the minimum spread for this weapon without external influences, but with addons */
	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintPure)
	float GetMinSpread() const;

	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintPure)
	float GetMaxMovementSpread() const;

	/** Returns spread regeneration rate with addons */
	float GetSpreadRegeneration() const;
	/** Returns spread increment rate with addons */
	float GetSpreadIncrement() const;


	// Recoil
	/** Return recoil in degrees with addons */
	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintPure)
	FVector2D GetRecoil() const;

	// Firing
	/** Returns fire interval with addons */
	float GetFireInterval() const;

	// Range
	/** Returns effective range with addons */
	float GetRange() const;



	// Ammo

	/** returns if the current Magazine has any Ammo */
	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintPure)
	bool HasAmmoLoaded() const;

	/** return if there is any ammo available for this weapon */
	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintPure)
	bool HasAnyAmmo() const;

	/** returns number of Magazines that have ammo */
	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintPure)
	int32 GetNumMagazines() const;

	/** returns ammo that is in current magazine + chamber */
	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintPure)
	int32 GetAmmoLoaded() const;

	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintPure)
	int32 GetMagazineCapacity() const;

	/** returns the best Magazine that is available (for reload) */
	int32 GetBestMag() const;

	// Reload
	/** Returns reload time in seconds with addons taken into account */
	float GetReloadTime() const;

	UAnimMontage* GetFPReloadAnim() const;
	UAnimMontage* GetReloadAnim() const;


	UFUNCTION(Category = Weapon, BlueprintCallable)
	bool ReloadRoundsIndividually() const { return bReloadRoundsIndividually; }


	UFUNCTION(Category = Weapon, BlueprintCallable)
	float GetReloadProgress() const;


	//////////////////////////
	// State

	void GotoState(class UTacticalWeaponState* NextState);
	void StateChanged();

	void Activate(bool bSkipEquip = false);

	//////////////////////////
	// Equip/Unequip

	bool IsChangingWeapon() const;

	bool TryUnequip();

	void LocalSimulateEquip();
	UFUNCTION(reliable, NetMulticast)
	void NetMulti_EquipWeapon();
	void NetMulti_EquipWeapon_Implementation();

	void WeaponUnequipped();
	void LocalSimulateUnequip();
	UFUNCTION(reliable, NetMulticast)
	void NetMulti_UnequipWeapon();
	void NetMulti_UnequipWeapon_Implementation();
	


	//////////////////////////
	// Firing

	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintPure)
	bool IsFiring() const;
	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintPure)
	bool CanFire() const;

	UFUNCTION(Category = Weapon, BlueprintNativeEvent)
	void GetWeaponTraceOriginAndDirection(FVector& Origin, FVector& Direction) const;
	virtual void GetWeaponTraceOriginAndDirection_Implementation(FVector& Origin, FVector& Direction) const;


	FVector GetCameraAim() const;
	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintPure)
	EWeaponFiringMode GetFiringMode() const;
	UFUNCTION(Category = Weapon, BlueprintNativeEvent)
	EWeaponFiringMode GetDefaultFiringMode() const;
	virtual EWeaponFiringMode GetDefaultFiringMode_Implementation() const;

	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintPure)
	int32 GetBurstCount() const { return BurstCount; }


	bool IsPendingFire() const;
	void SetPendingFire();
	void ClearPendingFire();

	UFUNCTION()
	virtual void NativeLocalSimulateFire();
	UFUNCTION(Category = "Weapon", BlueprintNativeEvent)
	void LocalSimulateFire();
	virtual void LocalSimulateFire_Implementation(){}

	UFUNCTION(Category = "Weapon", BlueprintNativeEvent)
	void PlayFiringSound();
	virtual void PlayFiringSound_Implementation();

	UPROPERTY(Category = "Weapon | Sound", EditDefaultsOnly, BlueprintReadOnly)
	USoundCue* FiringSound_Default;
	UPROPERTY(Category = "Weapon | Sound", EditDefaultsOnly, BlueprintReadOnly)
	USoundCue* FiringSound_Silenced;


	UFUNCTION(unreliable, NetMulticast)
	void SimulateHit(int32 RandomSeed, float InSpread);
	void SimulateHit_Implementation(int32 RandomSeed, float InSpread);
	void SimulateHit_Internal(int32 RandomSeed, float InSpread);

	UFUNCTION(Category = "Weapon", BlueprintNativeEvent)
	void LocalSimulateHit(FHitResult Hit, const FVector& TraceDir);
	virtual void LocalSimulateHit_Implementation(FHitResult Hit, const FVector& TraceDir);


	//UFUNCTION(reliable, server, WithValidation)
	//	void ServerConfirmHit(int32 RandomSeed, float Spread, AActor* LocalHitActor);

	void OnFire();

	virtual void MakeFiringNoise();

	void TraceShot(TArray<FHitResult>& OutHits, const FVector& TraceStart, const FVector& TraceEnd) const;
	FHitResult TraceShotCompensated(FVector TraceStart, FVector TraceEnd, float RewindTime);
	
	void OnEmpty();

	UFUNCTION(Category = Weapon, BlueprintNativeEvent)
	void LocalSimulateEmpty();
	void LocalSimulateEmpty_Implementation(){}


	void StartFiring();
	UFUNCTION(reliable, server, WithValidation)
	void ServerStartFiring(int32 RandomSeed);
	void ServerStartFiring_Implementation(int32 RandomSeed);
	bool ServerStartFiring_Validate(int32 RandomSeed){ return true; }
	void StartFiring_Internal(int32 RandomSeed);



	void StopFiring();
	UFUNCTION(reliable, server, WithValidation)
	void ServerStopFiring();
	void ServerStopFiring_Implementation();
	bool ServerStopFiring_Validate(){ return true; }
	void StopFiring_Internal();


	void TickSpread(float DeltaTime);

	void IncrementSpread();

	void ApplyRecoil();


	//////////////////////////
	// Reloading
	void StartReload();
	UFUNCTION(reliable, server, WithValidation)
	void ServerStartReload();
	void ServerStartReload_Implementation();
	bool ServerStartReload_Validate(){ return true; }
	void StartReload_Internal();


	void StopReload();
	UFUNCTION(reliable, server, WithValidation)
	void ServerStopReload();
	void ServerStopReload_Implementation();
	bool ServerStopReload_Validate() { return true; }
	void StopReload_Internal();


	bool NeedsReload() const;
	bool CanReload() const;
	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintPure)
	bool IsReloading() const;

	void LocalSimulateReload();
	UFUNCTION(reliable, NetMulticast)
	void NetMulti_Reload();
	void NetMulti_Reload_Implementation();

	void LocalSimulateContinueReload();

	void LocalSimulateStopReload();
	UFUNCTION(reliable, NetMulticast)
	void NetMulti_StopReload();
	void NetMulti_StopReload_Implementation();


	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon | Events")
		void OnSimulateReloadStart(float Duration);
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon | Events")
		void OnSimulateReloadContinue(float Duration);
	UFUNCTION(BlueprintImplementableEvent, Category = "Weapon | Events")
		void OnSimulateReloadFinish(float Duration);


	//UFUNCTION(Category = Weapon, BlueprintImplementableEvent)
	//void PlayReloadAnimation(float ReloadTime);
	//UFUNCTION(Category = Weapon, BlueprintImplementableEvent)
	//void PlayStopReloadAnimation(float ReloadTime);

	void ToggleFiringMode();
	UFUNCTION(reliable, server, WithValidation)
	void ServerToggleFiringMode();
	void ServerToggleFiringMode_Implementation();
	bool ServerToggleFiringMode_Validate(){ return true; }
	void LocalToggleFiringMode();
	UFUNCTION(Category = Weapon, BlueprintImplementableEvent)
	void OnToggleFiringMode();

	UPROPERTY(Category = Attachment, EditDefaultsOnly, BlueprintReadOnly)
	FName EquippedSocketName;
	UPROPERTY(Category = Attachment, EditDefaultsOnly, BlueprintReadOnly)
	FName EquippedSocketNameLeft;

	FName GetEquippedSocketName();


	UPROPERTY(Category = Attachment, EditDefaultsOnly, BlueprintReadOnly)
	FName GunIKSocketName;

	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintPure)
	EWeaponType GetWeaponType() const;

	UFUNCTION(Category = Weapon, BlueprintCallable)
	void AttachGunToHolster();
	UFUNCTION(Category = Weapon, BlueprintCallable)
	void AttachGunToHand();
	UFUNCTION(Category = Weapon, BlueprintCallable)
	void AttachGunToIK();

	UFUNCTION(BlueprintCallable, Category = Weapon)
	void GetCurrentAnimRecoil(FRotator& outPitch, FVector& outOffset) const;

	virtual FTransform GetLeftHandTransform() const;

	UFUNCTION(BlueprintNativeEvent, Category = Weapon)
	FTransform GetOwnLeftHandTransform() const;
	virtual FTransform GetOwnLeftHandTransform_Implementation() const;


	// Attachments

	UFUNCTION(Category = Weapon, BlueprintCallable)
	void SwitchLight(const bool bTurnOn);
	
	void NativeToggleAttachment(int32 AttachmentIdx = 0);
	
	UFUNCTION(Category = Weapon, BlueprintCallable, BlueprintNativeEvent)
	void ToggleAttachment(int32 AttachmenIdx = 0);
	void ToggleAttachment_Implementation(int32 AttachmenIdx = 0);

	UFUNCTION(reliable, server, WithValidation)
	void ServerToggleAttachment(int32 AttachmenIdx = 0);
	bool ServerToggleAttachment_Validate(int32 AttachmenIdx = 0) { return true; }
	void ServerToggleAttachment_Implementation(int32 AttachmenIdx = 0);

	UFUNCTION(Category = Weapon, BlueprintCallable)
		void ToggleSight();

	uint32 bIsLightOn : 1;

	UFUNCTION(Category = Weapon, BlueprintNativeEvent)
	void BPSwitchLight(bool TurnOn);
	virtual void BPSwitchLight_Implementation(bool TurnOn){}

	float GetObstructionSweepDistance() const { return ObstructionTraceDistance; }
	bool CanMoveBack(float Distance) const { return (MaxMoveBackDistance >= Distance); }


	UPROPERTY(Category = "Weapon | Noise", EditDefaultsOnly)
	float FiringNoiseLoudness;

	UPROPERTY(Category = "Weapon | Noise", EditDefaultsOnly)
	float FiringNoiseMaxRange;


	UPROPERTY(Category = "Weapon | Animation", EditDefaultsOnly, BlueprintReadOnly)
	class UAnimSequence* FPHandsIdleAnim;

	UPROPERTY(Category = "Weapon | Animation", EditDefaultsOnly)
	class UAnimMontage*  ReloadAnimationDefault;
	UPROPERTY(Category = "Weapon | Animation", EditDefaultsOnly)
	class UAnimMontage*  ReloadAnimationCrouched;
	UPROPERTY(Category = "Weapon | Animation", EditDefaultsOnly)
	class UAnimMontage*  ReloadAnimationProne;
	UPROPERTY(Category = "Weapon | Animation", EditDefaultsOnly)
	class UAnimMontage* FPReloadAnim;
	UPROPERTY(Category = "Weapon | Animation", EditDefaultsOnly)
	class UAnimMontage* WeaponReloadAnim;
	UPROPERTY(Category = "Weapon | Animation", EditDefaultsOnly)
	class UAnimMontage* FiringAnimation;

	UPROPERTY(Category = "Preview", EditDefaultsOnly)
	FTransform PreviewTransform;

	//Attachments
	class ATacticalWeaponAttachment* CreateAttachment(const FString& AttachmentPointName, int32 AttachmentIdx, const FString& Options = FString());
	class ATacticalWeaponAttachment* CreateAttachment(UTacticalWeaponAttachmentPoint* AttachmentPoint, int32 AttachmentIdx, const FString& Options = FString());

	bool IsAttachmentAllowed(TSubclassOf<class ATacticalWeaponAttachment> AttachmentClass) const;
	bool IsAttachmentAllowed(class ATacticalWeaponAttachment* Attachment) const;

	UFUNCTION(Category = Weapon, BlueprintCallable)
	const TArray<TSubclassOf<UTacticalAmmoType> >& GetAllowedAmmo() const { return AllowedAmmoTypes; }

	UFUNCTION(Category = Animation, BlueprintCallable)
	float MontagePlay(UAnimMontage* Montage, float Rate, FName Section);

	UFUNCTION(Category = Weapon, BlueprintNativeEvent)
	class UTacticalRecoilCurve* GetRecoilCurve() const;
	virtual class UTacticalRecoilCurve* GetRecoilCurve_Implementation() const;

	virtual void SetAmmoType(int32 AmmoTypeIdx);

	UFUNCTION(Category = Weapon, BlueprintCallable)
	float GetADSSpeed() const;

public:
	UPROPERTY(Category = "Attachment", EditDefaultsOnly, BlueprintReadWrite)
		FVector WeaponOffset;

protected:

	//UFUNCTION(Category = Weapon, BlueprintCallable, server, WithValidation)
	//void ServerDisableWeaponFireWithDuration(float Duration);
	//bool ServerDisableWeaponFireWithDuration_Validate(float Duration) { return true; }
	//void ServerDisableWeaponFireWithDuration(float Duration);

	FTimerHandle WeaponDisabledTimer;

	UFUNCTION(Category = Weapon, BlueprintCallable)
	void AddRecoilAnimCurve(UTacticalRecoilCurve* Curve, bool bRandomY, bool bRandomYaw, float Scale = 1.f);

	FTimerHandle BulletEjectTimer;
	void SimulateBulletEject();

	UFUNCTION()
	void OnBulletDropCollision(FName EventName, float EmitterTime, int32 ParticleTime, FVector Location, FVector Velocity, FVector Direction, FVector Normal, FName BoneName, UPhysicalMaterial* PhysMat);

	void ConsumeAmmo();
	void ApplyDamage(const FHitResult& Hit, const FVector& ShotDir, int32 NumPenetrations);


	UPROPERTY(Category = Weapon, EditDefaultsOnly)
	float ObstructionTraceDistance;
	
	UPROPERTY(Category = Weapon, EditDefaultsOnly)
	float MaxMoveBackDistance;

	UPROPERTY(Category = Weapon, EditDefaultsOnly)
	EWeaponType WeaponType;

	UPROPERTY(Category = "Attachment", EditDefaultsOnly)
	FVector WeaponAimOffset;
	/** Allows Adjustment for Weapons with an integrated sight in the model*/
	UPROPERTY(Category = "Attachment", EditDefaultsOnly)
	FVector WeaponAimOffset_NoSight;

	UPROPERTY(Category = "Crosshair", EditDefaultsOnly)
	uint32 bShowCrosshairWhenAiming : 1;
	UPROPERTY(Category = "Crosshair", EditDefaultsOnly)
	TSubclassOf<UUserWidget> CrosshairWidget;


	UPROPERTY(Category = "Stats", EditDefaultsOnly)
	float AimZoomModifier;

	UPROPERTY(Category = "Stats", EditDefaultsOnly)
	float EquipTime;
	UPROPERTY(Category = "Stats", EditDefaultsOnly)
	float UnequipTime;


	// Stats
	/** The angle range of the cone that the weapon will fire in (without any effects by movement and rotation) */
	UPROPERTY(Category = "Stats | Spread", EditDefaultsOnly)
	FFloatInterval Spread;
	/** The speed at which the weapon spread will recover in deg/s */
	UPROPERTY(Category = "Stats | Spread", EditDefaultsOnly)
	float SpreadRecovery;
	/** The angle increment of weapon spread per shot */
	UPROPERTY(Category = "Stats | Spread", EditDefaultsOnly)
	float SpreadIncrement;
	/** factor that converts movement speed to additional weapon spread in deg/(cm/s) */
	UPROPERTY(Category = "Stats | Spread", EditDefaultsOnly)
	float MovementSpreadModifier;
	/** factor that converts movement speed to additional weapon spread in 1/s */
	UPROPERTY(Category = "Stats | Spread", EditDefaultsOnly)
	float RotationSpreadModifier;
	/** upper limit of the additional spread that movement can cause */
	UPROPERTY(Category = "Stats | Spread", EditDefaultsOnly)
	float MaxMovementSpread;
	/** factor at which aiming does reduce overall weapon spread */
	UPROPERTY(Category = "Stats | Spread", EditDefaultsOnly)
	float AimSpreadModifer;
	/** factor at which aiming does reduce overall weapon spread */
	UPROPERTY(Category = "Stats | Spread", EditDefaultsOnly)
	float AddSpread_Standing;
	UPROPERTY(Category = "Stats | Spread", EditDefaultsOnly)
	float AddSpread_Crouched;	
	UPROPERTY(Category = "Stats | Spread", EditDefaultsOnly)
	float AddSpread_Prone;

	UPROPERTY(Category = "Stats | Recoil", EditDefaultsOnly)
	FVector2D BaseRecoil;	// Recoil
	UPROPERTY(Category = "Stats | Recoil", EditDefaultsOnly)
	uint32 bRandomizeYRecoil : 1;
	UPROPERTY(Category = "Stats | Recoil", EditDefaultsOnly)
	uint32 bRandomizeYawRecoil : 1;
	UPROPERTY(Category = "Stats | Recoil", EditDefaultsOnly)
	float CrouchRecoilModifier;
	UPROPERTY(Category = "Stats | Recoil", EditDefaultsOnly)
	float ProneRecoilModifier;
	UPROPERTY(Category = "Stats | Recoil", EditDefaultsOnly)
	float AimRecoilModifier;
	//UPROPERTY(Category = "Stats | Recoil", EditDefaultsOnly)
	//	FAnimRecoilInfo AnimatedRecoil;
	UPROPERTY(Category = "Stats | Recoil", EditDefaultsOnly)
	class UTacticalRecoilCurve* RecoilAnimation;
	UPROPERTY(Category = "Stats | Recoil", EditDefaultsOnly)
	UCameraAnim* RecoilCameraAnim;
	UPROPERTY(Category = "Stats | Recoil", EditDefaultsOnly)
	float AimRecoilCameraAnimScale;

	float ScaleRecoil(float BaseRecoilScale) const;

	/*** Time that it takes to insert mag or bullet */
	UPROPERTY(Category = "Stats | Reload", EditDefaultsOnly)
	float ReloadTime;
	/*** Time that it takes to start the reload (opening chamber) */
	UPROPERTY(Category = "Stats | Reload", EditDefaultsOnly)
	float ReloadStartTime; 
	/*** Time that it takes to finishe the reload (loading bullet in chamber) */
	UPROPERTY(Category = "Stats | Reload", EditDefaultsOnly)
	float ReloadFinishTime;
	UPROPERTY(Category = "Stats | Reload", EditDefaultsOnly)
	uint32 bReloadRoundsIndividually : 1;

	FTimerHandle ReloadStartTimer;

	UPROPERTY(Category = "Stats | Firing", EditDefaultsOnly)
	float BaseRange;		// Range
	UPROPERTY(Category = "Stats | Firing", EditDefaultsOnly)
	float BaseFireInterval;
	UPROPERTY(Category = "Stats | Firing", EditDefaultsOnly)
	float TriggerDelay;

	UPROPERTY(Category = "Stats | Firing", EditDefaultsOnly)
	bool bCanFMAuto;
	UPROPERTY(Category = "Stats | Firing", EditDefaultsOnly)
	bool bCanFMBurst;
	UPROPERTY(Category = "Stats | Firing", EditDefaultsOnly)
	bool bCanFMSingle;

	UPROPERTY(Category = "Stats | Firing", EditDefaultsOnly)
	float PenetrationModifier;	// Penetration through Surfaces
	UPROPERTY(Category = "Stats | Firing", EditDefaultsOnly)
	float DamageModifier;		// Damage

	UPROPERTY(Category = "Stats | Firing", EditDefaultsOnly, Meta = (EditCondition = "bCanFMBurst"))
	int32 BurstCount;


	UPROPERTY(Category = "Stats | Firing", EditDefaultsOnly, meta = (DisplayName = "ADS Speed"))
	float ADSSpeed;

	UPROPERTY(Category = "Stats | Ammo/Magazine", EditDefaultsOnly)
	int32 NumMagazines;
	UPROPERTY(Category = "Stats | Ammo/Magazine", EditDefaultsOnly)
	int32 MagazineCapacity;
	UPROPERTY(Category = "Stats | Ammo/Magazine", EditDefaultsOnly)
	bool bCanBulletStayInChamber;

	UPROPERTY(Category = "Stats | Ammo/Magazine", EditDefaultsOnly, BlueprintReadOnly)
	TArray< TSubclassOf<class UTacticalAmmoType> > AllowedAmmoTypes;
	UPROPERTY(Category = "Stats | Ammo/Magazine", EditDefaultsOnly)
	int32 DefaultAmmoType;

	UPROPERTY(Category = "Stats | Ammo/Magazine", EditDefaultsOnly)
	float BulletEjectDelay;

	TSubclassOf<class UTacticalAmmoType> AmmoType;
	// TArray<ATacticalWeaponPeripheral> AllowedPeripherals; // SoundSupressors, HiCapMag, etc



	// Attachments

	UPROPERTY(Category = "Attachments", BlueprintReadOnly, EditDefaultsOnly)
	TArray<TSubclassOf<class ATacticalWeaponAttachment_Special> > AllowedSpecialAttachments;


	UPROPERTY(Category = "Attachments", BlueprintReadOnly)
	class ATacticalWeaponAttachment_Special* AttachmentSpecial;

	UFUNCTION(BlueprintNativeEvent, Category = Weapon)
	void RemoveIronSight();
	virtual void RemoveIronSight_Implementation() {}

	FRandomStream RandomSpreadStream;

	uint32 bPendingUnequip : 1;
	uint32 bPendingReload : 1;

	UPROPERTY(Replicated)
	EWeaponFiringMode FiringMode;

	UPROPERTY(Replicated)
	int32 CurrentMagazine;
	UPROPERTY(Replicated)
	TArray<int32> Magazine;

	
	float CurrentSpread;
	float TargetSpread;

	TArray<FPendingAnimRecoil> PendingAnimRecoils;

	//UPROPERTY(Category = AI, EditDefaultsOnly, BlueprintReadOnly)
	UPROPERTY(Category = AI, EditDefaultsOnly)
	FFloatInterval AIBurstInterval;
	UPROPERTY(Category = AI, EditDefaultsOnly /*, BlueprintReadOnly*/)
	FInt32Interval AIBurstCount;

public:
	const FFloatInterval& GetAIBurstInterval() const { return AIBurstInterval; }
	const FInt32Interval& GetAIBurstCount() const { return AIBurstCount; }

	class UTacticalWeaponState* GetCurrentState() const { return CurrentState; }

protected:
	////////////////////////////////////////////
	//Weapon State
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState)
	uint8 RepWeaponState;

	UFUNCTION()
	virtual void OnRep_WeaponState(uint8 PrevState);

	UTacticalWeaponState* GetWeaponStateForByte(uint8 Val) const;

	UPROPERTY(BlueprintReadOnly, Category = Weapon)
	class UTacticalWeaponState* CurrentState;

	UPROPERTY(Instanced, BlueprintReadOnly, Category = States)
	UTacticalWeaponState* StateActive;
	UPROPERTY(Instanced, BlueprintReadOnly, Category = States)
	UTacticalWeaponState* StateEquipping;
	UPROPERTY(Instanced, BlueprintReadOnly, Category = States)
	UTacticalWeaponState* StateUnequipping;
	UPROPERTY(Instanced, BlueprintReadOnly, Category = States)
	UTacticalWeaponState* StateInactive;
	UPROPERTY(Instanced, BlueprintReadOnly, Category = States)
	UTacticalWeaponState* StateReloading;
	UPROPERTY(Instanced, BlueprintReadOnly, Category = States)
	UTacticalWeaponState* StateFiring;
	UPROPERTY(Instanced, BlueprintReadOnly, Category = States)
	UTacticalWeaponState* StateObstructed;
};


UCLASS(abstract, NotPlaceable, HideDropdown)
class TACTICALFPS_API ATacticalPrimaryWeapon : public ATacticalWeapon
{
	GENERATED_BODY()

public:
	ATacticalPrimaryWeapon(const FObjectInitializer& OI);

};


UCLASS(abstract, NotPlaceable, HideDropdown)
class TACTICALFPS_API ATacticalSidearm : public ATacticalWeapon
{
	GENERATED_BODY()

public:
	ATacticalSidearm(const FObjectInitializer& OI);

	virtual EWeaponFiringMode GetDefaultFiringMode_Implementation() const override
	{
		if (bCanFMSingle)
		{
			return EWeaponFiringMode::FM_Single;
		}
		else if (bCanFMAuto)
		{
			return EWeaponFiringMode::FM_Auto;
		}
		else if (bCanFMBurst)
		{
			return EWeaponFiringMode::FM_Burst;
		}
		return EWeaponFiringMode::FM_None;
	}

};