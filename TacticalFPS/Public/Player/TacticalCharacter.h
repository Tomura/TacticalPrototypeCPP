// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "Perception/AISightTargetInterface.h"
#include "TacticalTypes.h"
#include "BRSPerlinNoiseStream.h"
#include "TacticalCharacter.generated.h"


struct TACTICALFPS_API FSavedPosition
{
	float Time;
	FVector Position;
	FVector Velocity;
	FRotator ViewRotation;
	uint32 bJustTeleported : 1;
	uint32 bShotSpawned : 1;

	FSavedPosition(float inTime, const FVector& inPosition, const FVector& inVelocity, const FRotator& inViewRot, bool bInJustTeleported, bool bInShotSpawned)
		: Time(inTime), Position(inPosition), Velocity(inVelocity), ViewRotation(inViewRot), bJustTeleported(bInJustTeleported), bShotSpawned(bInShotSpawned)
	{}
};

USTRUCT(BlueprintType)
struct TACTICALFPS_API FTacticalAnimMontages
{
	GENERATED_USTRUCT_BODY()
public:
	FTacticalAnimMontages() :
		DefaultMontage(nullptr),
		PistolMontage(nullptr),
		RifleMontage(nullptr)
	{}

	UPROPERTY(Category = Animation, BlueprintReadOnly, EditDefaultsOnly)
	UAnimMontage* DefaultMontage;
	UPROPERTY(Category = Animation, BlueprintReadOnly, EditDefaultsOnly)
	UAnimMontage* PistolMontage;
	UPROPERTY(Category = Animation, BlueprintReadOnly, EditDefaultsOnly)
	UAnimMontage* RifleMontage;

	UAnimMontage* GetMontage(EWeaponType WeaponType) const
	{
		switch (WeaponType)
		{
		case EWeaponType::Pistol:
			return PistolMontage;
		case EWeaponType::Rifle:
			return RifleMontage;
		case EWeaponType::None:
		default:
			break;
		}
		return DefaultMontage;
	}
};

USTRUCT(BlueprintType)
struct TACTICALFPS_API FTacticalAnimMontageCollection
{
	GENERATED_USTRUCT_BODY()
public:
	FTacticalAnimMontageCollection()
	{
		DefaultMontages.Empty();
		PistolMontages.Empty();
		RifleMontages.Empty();
	}

	UPROPERTY(Category = Animation, BlueprintReadOnly, EditDefaultsOnly)
	TArray<UAnimMontage*> DefaultMontages;
	UPROPERTY(Category = Animation, BlueprintReadOnly, EditDefaultsOnly)
	TArray<UAnimMontage*> PistolMontages;
	UPROPERTY(Category = Animation, BlueprintReadOnly, EditDefaultsOnly)
	TArray<UAnimMontage*> RifleMontages;

	const TArray<UAnimMontage*>& GetMontageArray(EWeaponType WeaponType) const
	{
		switch (WeaponType)
		{
		case EWeaponType::Pistol:
			return PistolMontages;
		case EWeaponType::Rifle:
			return RifleMontages;
		case EWeaponType::None:
		default:
			break;
		}
		return DefaultMontages;
	}
};

UCLASS(config=Game)
class TACTICALFPS_API ATacticalCharacter : public ACharacter, public IAISightTargetInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()


	/** Camera boom positioning the camera behind the character */
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	//class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Perception, meta = (AllowPrivateAccess = "true"))
	class UPawnNoiseEmitterComponent* NoiseEmitter;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Health, meta = (AllowPrivateAccess = "true"))
	class UBRSActorHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Health, meta = (AllowPrivateAccess = "true"))
	class UTacticalInventoryComponent* InventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Projectile, meta = (AllowPrivateAccess = "true"))
	class USceneComponent* ProjectileSpawn;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* FPMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	class UWidgetInteractionComponent* WidgetInteractionComp;
public:

	ATacticalCharacter(const FObjectInitializer& OI);

	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;

	virtual void PostInitializeComponents() override;

	virtual void BeginPlay() override;
	virtual void Restart() override;
	virtual void Destroyed() override;

	virtual void DisplayDebug(class UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void FaceRotation(FRotator NewControlRotation, float DeltaTime) override;

	virtual FRotator GetBaseAimRotation() const;

	virtual void Landed(const FHitResult& Hit);

	/** 
	 * calculates camera view info for this actor.
	 * @param DeltaTime		time in s between two frames.
	 * @param OutResult		reference to view info struct. Will be changed by function.
	 */
	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;
	
	virtual void PossessedBy(AController* NewController) override;

	virtual void GetActorEyesViewPoint(FVector& Location, FRotator& Rotation) const override;


	virtual void OnFirstPerson();
	virtual void OnThirdPerson();

	/** Replicated so we can see where remote clients are looking. */
	UPROPERTY(Replicated)
	uint8 RemoteViewYaw;

	void SetRemoteViewYaw(float inYaw);


	/** returns the third person camera component. */
	UFUNCTION(Category = Camera, BlueprintNativeEvent)
	UCameraComponent* GetThirdPersonCamera() const;
	virtual UCameraComponent* GetThirdPersonCamera_Implementation() const { return nullptr; }

	/** returns the first person camera component. */
	UFUNCTION(Category = Camera, BlueprintNativeEvent)
	UCameraComponent* GetFirstPersonCamera() const;
	virtual UCameraComponent* GetFirstPersonCamera_Implementation() const { return FollowCamera; }

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	/** returns MovementComponent as TacticalCharacterMovementComponent */
	class UTacticalCharacterMovement* GetTacticalMovement() const;

	/** returns lean angle */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tactical Character")
	float GetLeanAngle() const;


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tactical Character")
		bool UseWeaponAimIK() const;

	FRotator LastViewRotation;
	FRotator ViewRotationSpeed;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tactical Character")
		FRotator GetViewRotationSpeed();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tactical Character")
		bool CanFire() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tactical Character")
		bool IsAiming() const;

	bool CanAim() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tactical Character")
		bool CanReload() const;


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tactical Character")
		FVector GetWeaponHandOffset() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tactical Character")
		FVector GetWeaponTraceOrigin() const;

	USkeletalMeshComponent* GetFirstPersonMesh() const { return FPMesh; }


	UFUNCTION(Category = "Tactical Character", BlueprintCallable, BlueprintPure)
	class ATacticalWeapon* GetWeapon() const;


	UFUNCTION(Category = "Tactical Character", BlueprintCallable, BlueprintPure)
	ETacticalStance GetStance() const;

	// Events
	UFUNCTION()
	void OnWeaponEquip(ATacticalWeapon* Weapon, float EquipTime) { PlayEquipAnimation(EquipTime); }
	virtual void PlayEquipAnimation(float Time);

	UFUNCTION()
	void OnWeaponUnequip(ATacticalWeapon* Weapon, float UnequipTime) { PlayUnequipAnimation(UnequipTime); }
	virtual void PlayUnequipAnimation(float Time);

	UFUNCTION()
	void OnWeaponReload(ATacticalWeapon* Weapon, float ReloadTime) {PlayReloadAnimation(ReloadTime);}
	virtual void PlayReloadAnimation(float Time);


	UFUNCTION(Category = "Tactical Character", BlueprintNativeEvent)
	void OnWeaponStopReload(float ReloadTime);
	virtual void OnWeaponStopReload_Implementation(float ReloadTime) { PlayStopReloadAnimation(ReloadTime); }
	virtual void PlayStopReloadAnimation(float Time);


	void WeaponUnequipped();

	/** called by movement component to store movement data for rewind prediction. */
	void PerformedMovement();
	/** returns rewind position of character for time offset. */
	FVector GetRewindPosition(float inTime = -1.f) const;
	/** maximum positions to save. */
	float MaxPositionSaveTime;

	UFUNCTION(Category = "Tactical Character", BlueprintCallable)
	bool IsAlive() const;

	void AddRecoil(FVector2D Recoil);


	UFUNCTION(Category = "Tactical Character", BlueprintCallable)
	bool IsFiring() const;

	UFUNCTION(Category = "Tactical Character", BlueprintCallable)
		bool IsChangingWeapon() const;

	UFUNCTION(Category = "Tactical Character", BlueprintCallable)
		bool IsReloading() const;

	UFUNCTION(Category = "Tactical Character", BlueprintCallable)
		bool IsThrowingGrenade() const;

	UFUNCTION(Category = "Animation", BlueprintCallable, BlueprintPure)
		void GetDesiredFootLocations(FVector& RightFoot, FVector& LeftFoot, FVector& RightFootNormal, FVector& LeftFootNormal) const;


	bool CanVault() const;


	UFUNCTION(Category = "Tactical Character", BlueprintCallable)
		bool IsVaulting() const { return bIsVaulting; }


	UPROPERTY(Replicated)
		ETacticalRotationType RotationType;
	//UPROPERTY(Replicated)
	//	uint8 ReplicatedAimYaw;
	//UPROPERTY(Replicated)
	//	uint8 ReplicatedAimPitch;



	UFUNCTION(Category = "Tactical Character", BlueprintCallable)
		bool ShowCrosshair() const;


	bool WeaponObstructionCheck() const;
	bool IsWeaponObstructed() const;

	UPROPERTY(Replicated)
		uint32 bIsWeaponObstructed : 1;

	FTimerHandle WeaponObstructionTimer;

	void TimeCheckWeaponObstruction();
	UPROPERTY(Category = Weapon, EditDefaultsOnly)
		FVector ObstructionTesterExtent;

	float WeaponMoveBackDistance;
	bool bMoveWeaponBack;

	UFUNCTION(Category = "Interaction", BlueprintCallable)
		AActor* GetInteractActor() { return InteractActor; }

	UPawnNoiseEmitterComponent* GetNoiseEmitter() const { return NoiseEmitter; }

	UPROPERTY(config)
		bool bUseFPArmsModel;

	

	UFUNCTION(Category = "AI | Cover", BlueprintCallable)
	bool WantsToCover() const;

	UFUNCTION(Category = "AI | Cover", BlueprintCallable)
	virtual bool IsInCover(ECoverType CoverFilter = ECoverType::None) const;
	UFUNCTION(Category = "AI | Cover", BlueprintCallable)
	bool IsInHighCover() const;
	UFUNCTION(Category = "AI | Cover", BlueprintCallable)
	bool IsInLowCover() const;

	UFUNCTION(Category = "Combat", BlueprintCallable)
	bool IsWeaponLeftHanded() const { return bWeaponLeftHanded; }

	UFUNCTION(Category = "AI | Cover", BlueprintCallable)
	bool GetOverlappingCover(TArray<class ATacticalCoverMarker*>& OutCovers, ECoverType CoverFilter = ECoverType::None) const;


	void SetWalkSpeedFactor(float newSpeedFactor);
	float GetWalkSpeedFactor() { return WalkSpeedFactor; }

	UFUNCTION(Category = "Camera", BlueprintCallable)
		bool IsThirdPerson() const;

	UFUNCTION(Category = "Tactical Character", BlueprintCallable)
	virtual void Melee();

	UFUNCTION(reliable, Server, WithValidation)
	void ServerMelee();
	virtual bool ServerMelee_Validate() { return true; }
	virtual void ServerMelee_Implementation();


	UFUNCTION(reliable, NetMultiCast)
	void NetMultiMelee();
	virtual void NetMultiMelee_Implementation();

	UFUNCTION(Category = "Tactical Character | Melee", BlueprintNativeEvent)
	void SimulateMelee();
	virtual void SimulateMelee_Implementation();

	UPROPERTY(Category = "Tactical Character | Melee", EditDefaultsOnly)
	TSubclassOf<class UTacticalDamageType> MeleeDamageType;

	UPROPERTY(Category = "Tactical Character | Melee", EditDefaultsOnly)
		float MeleeDamageDelay;

	UPROPERTY(Category = "Tactical Character | Melee", EditDefaultsOnly)
		float MeleeRange;
	UPROPERTY(Category = "Tactical Character | Melee", EditDefaultsOnly)
		float MeleeDamage;
	UPROPERTY(Category = "Tactical Character | Melee", EditDefaultsOnly)
		float MeleeCooldown;

	UPROPERTY(Category = "Tactical Character | Melee", EditDefaultsOnly)
	TSubclassOf<class ATacticalImpactFX> MeleeImpactFX;


	virtual void DoMeleeDamage();

	FTimerHandle TimerMeleeDamageDelay;
	FTimerHandle TimerMeleeCooldown;

	UFUNCTION(Category = "Tactical Character | Combat", BlueprintCallable)
		float GetADSState() const { return ADSState; }

protected:
	bool bThirdPersonView;
	UFUNCTION(Exec)
		void BehindView(bool bUseBehindView);

	UPROPERTY(Category = Animation, EditDefaultsOnly)
		TArray<FName> BonesToHide;

	UPROPERTY(Category = Animation, EditDefaultsOnly)
		TArray<FVector> RootTraceLocs;

	UPROPERTY(Category = Movement, EditDefaultsOnly, BlueprintReadOnly)
		float MinSafeFallSpeedZ;
	UPROPERTY(Category = Movement, EditDefaultsOnly, BlueprintReadOnly)
		float LethalFallSpeedZ;

	/** Saved positions for Rewind */
	TArray<FSavedPosition> SavedPositions;

	UPROPERTY(Replicated)
		bool bIsAiming;

	UPROPERTY(Replicated)
		bool bWantsToAim;

	UPROPERTY(Replicated)
		bool bWeaponLeftHanded;

	UPROPERTY()
		bool bPendingWeaponHandLeft;

	void LeanRight(float Val);
	void ToggleLeanRight();
	void ToggleLeanLeft();
	void ToggleLean(EDirection Dir);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerLeanRight(float Val);
	virtual void ServerLeanRight_Implementation(float Val);
	virtual bool ServerLeanRight_Validate(float Val) { return true; }

	/** angle at which the character is leaned. todo: Could be optimized as byte */
	UPROPERTY(Replicated)
	float LeanAngle;
	/** cached values from toggled lean. Is added to lean axis input */
	float ToggleLeanVal;
	/** desired lean angle */
	float TargetLeanAngle;

	/** absolute maximum lean angle */
	UPROPERTY(Category = TacticalCharacter, EditDefaultsOnly)
	float MaxLeanAngle;
	/** speed at which character leans */
	UPROPERTY(Category = TacticalCharacter, EditDefaultsOnly)
	float LeanSpeed;

	/** increment/decrement per button press of walk speed modifier */
	UPROPERTY(Category = TacticalCharacter, EditDefaultsOnly)
	float SpeedIncrement;

	float WalkSpeedFactor;

	void SpeedUp();
	void SpeedDown();


	UPROPERTY(Category = "AI | Hearing", EditDefaultsOnly)
	float FootStepNoiseMaxRange;

	UPROPERTY(Category = "AI | Hearing", EditDefaultsOnly)
		float FootStepNoiseSpeedFactor;

	UPROPERTY(Category = TacticalCharacter, EditDefaultsOnly)
		FVector BaseWeaponHandOffset_Default;
	UPROPERTY(Category = TacticalCharacter, EditDefaultsOnly)
		FVector BaseWeaponHandOffset_Aim;

	FVector WeaponHandOffset;

	UPROPERTY(Category = TacticalCharacter, EditDefaultsOnly)
		float RecoilSpeed;

	UPROPERTY(Category = Grenade, EditDefaultsOnly)
	float GrenadePreThrowDelay;
	FTimerHandle TimerGrenadePreThrowDelay;
	UPROPERTY(Category = Grenade, EditDefaultsOnly)
	float GrenadePostThrowDelay;
	FTimerHandle TimerGrenadePostThrowDelay;
	float GrenadeCooldown;
	FTimerHandle TimerGrenadeCooldown;

	UPROPERTY(Category = TacticalCharacter, EditDefaultsOnly)
	float InteractionTraceDistance;

	UPROPERTY(Category = "Interaction", BlueprintReadOnly, Transient)
	AActor* InteractActor;

	UPROPERTY(Category = "Interaction", BlueprintReadOnly, Transient)
	bool bIsInteracting;

	FTimerHandle InteractPressedTimer;

	UPROPERTY(Category = "Interaction", EditDefaultsOnly)
	float InteractImmidiateTimeFrame;

	UPROPERTY(Category = "AI", BlueprintReadOnly, EditAnywhere)
	float TeamNotifyDamageRadius;

	UPROPERTY(Replicated)
	float TargetCapsuleSize;

	UPROPERTY(Category = Collision, EditDefaultsOnly)
	float CapsuleGrowthSpeed;

	UPROPERTY(Category = Animation, EditDefaultsOnly)
		FTacticalAnimMontageCollection DeathAnimations;
	UPROPERTY(Category = Animation, EditDefaultsOnly)
		FTacticalAnimMontages ReloadAnimations;
	UPROPERTY(Category = Animation, EditDefaultsOnly)
		FTacticalAnimMontages VaultAnimations;
	UPROPERTY(Category = Animation, EditDefaultsOnly)
		FTacticalAnimMontages ClimbAnimations;
	UPROPERTY(Category = Animation, EditDefaultsOnly)
		FTacticalAnimMontages EquipAnimations;
	UPROPERTY(Category = Animation, EditDefaultsOnly)
		FTacticalAnimMontages UneqipAnimations;

	float ADSState;


public:
	UFUNCTION(Category = Controls, BlueprintCallable)
		void StartReload();

	UFUNCTION(Category = Controls, BlueprintCallable)
		void ChangeStance(ETacticalStance NewStance);

	UFUNCTION(Category = Controls, BlueprintCallable)
		void StartFire();

	UFUNCTION(Category = Controls, BlueprintCallable)
		void StopFire();

	UFUNCTION(Category = Controls, BlueprintCallable)
	void ToggleWeaponSight();

	UFUNCTION(Category = Controls, BlueprintCallable)
	void ToggleWeaponGadget1();

	UFUNCTION(Category = Controls, BlueprintCallable)
	void ToggleWeaponGadget2();

	UFUNCTION(Category = Controls, BlueprintCallable)
	void TacticalJump();

	bool CanSwitchWeapon() const;

	void StartSprint();
	void StopSprint();

	UFUNCTION(Category = TacticalCharacter, BlueprintCallable)
	void TrySwitchWeapon(class ATacticalWeapon* DesiredWeapon);

	UFUNCTION(Category = TacticalCharacter, BlueprintCallable)
	void Gadget1_Pressed();
	UFUNCTION(Category = TacticalCharacter, BlueprintCallable)
	void Gadget1_Released();
	UFUNCTION(Category = TacticalCharacter, BlueprintCallable)
	void Gadget2_Pressed();
	UFUNCTION(Category = TacticalCharacter, BlueprintCallable)
	void Gadget2_Released();


	void StartThrow(class ATacticalThrowable* GadgetToThrow);
	void FinishThrow(class ATacticalThrowable* GadgetToThrow);

protected:

	// for debugging
	//UFUNCTION(Exec)
	//void DumpWeaponInfo();

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** Called for forwards/backward input */
	void KeyMoveForward(float Value);

	/** Called for side to side input */
	void KeyMoveRight(float Value);


	virtual void AddControllerYawInput(float Value) override;
	virtual void AddControllerPitchInput(float Value) override;

	/**
	* Called via input to turn at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate(float Rate);

	/**
	* Called via input to turn look up/down at a given rate.
	* @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	*/
	void LookUpAtRate(float Rate);

	/**
	* Called on server by owner. Sends jump input to server
	*/
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerJump();
	void ServerJump_Implementation();
	bool ServerJump_Validate() { return true; }

	/**
	* Called on all clients. Replicates Vault and plays vault montage on all relevant clients
	*/
	UFUNCTION(Reliable, NetMulticast)
	void NetMultiVault(bool bClimb);
	void NetMultiVault_Implementation(bool bClimb);

	/**
	* local. Plays vault anim montage (w/ root motion)
	*/
	void Vault(class ATacticalVaultMarker* Marker);
	UFUNCTION(Category = TacticalCharacter, BlueprintNativeEvent)
	float PlayVaultAnimations();
	virtual float PlayVaultAnimations_Implementation();

	/**
	* local. Plays climb anim montage (w/ root motion)
	*/
	UFUNCTION(Category = TacticalCharacter, BlueprintNativeEvent)
	float PlayClimbAnimations();
	virtual float PlayClimbAnimations_Implementation();

	/**
	* local. called when vault timer elapses. Handles collision and physics changes.
	*/
	UFUNCTION()
	void VaultEnd();


	void StopReload();

	float CrouchTime;
	UPROPERTY(Category = Movement, EditAnywhere)
	float CrouchTimeForToggle;

	void CrouchPressed();
	void CrouchReleased();
	void PronePressed();

	void TryStand();
	void TryCrouch();
	void TryProne();

	void TryLowerStance();
	void TryRaiseStance();

	void GamepadCrouch();
	void GamepadCrouchReleased();
	FTimerHandle GamepadCrouchTimer;
	void GamepadProne();

	//UFUNCTION(Reliable, Server, WithValidation)
	//void ServerChangeStance(ETacticalStance NewStance);
	//void ServerChangeStance_Implementation(ETacticalStance NewStance);
	//bool ServerChangeStance_Validate(ETacticalStance NewStance) { return true; }

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartSprint();
	void ServerStartSprint_Implementation();
	bool ServerStartSprint_Validate() { return true; }

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopSprint();
	void ServerStopSprint_Implementation();
	bool ServerStopSprint_Validate() { return true; }

	void StartAim();
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartAim();
	void ServerStartAim_Implementation();
	bool ServerStartAim_Validate() { return true; }

	void ReleaseAim();

	void StopAim();
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopAim();
	void ServerStopAim_Implementation();
	bool ServerStopAim_Validate() { return true; }

	AActor* InteractionTrace();
	class ATacticalVaultMarker* GetBestVaultMarker() const;

	void StartInteract();
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartInteract();
	void ServerStartInteract_Implementation();
	bool ServerStartInteract_Validate() { return true; }

	void StopInteract();
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopInteract();
	void ServerStopInteract_Implementation();
	bool ServerStopInteract_Validate() { return true; }

	void InteractImmidiate();
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerInteractImmidiate();
	void ServerInteractImmidiate_Implementation();
	bool ServerInteractImmidiate_Validate() { return true; }

	void SlowOpen();
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerSlowOpen();
	void ServerSlowOpen_Implementation();
	bool ServerSlowOpen_Validate() { return true; }

	void SlowClose();
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerSlowClose();
	void ServerSlowClose_Implementation();
	bool ServerSlowClose_Validate() { return true; }


	void ToggleFireMode();

	void SwitchToPrimaryWeapon();
	void SwitchToSidearm();
	void HolsterWeapon();
	void GamepadSwitchWeapon();


	void SwitchWeaponHand();

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerSwitchWeaponHand(bool bUseLeftHand);
	virtual void ServerSwitchWeaponHand_Implementation(bool bUseLeftHand);
	virtual bool ServerSwitchWeaponHand_Validate(bool bUseLeftHand) { return true; }

	virtual void SwitchWeaponHand_Internal(bool bUseLeftHand);
	virtual void PerformSwitchWeaponHand();
	virtual void FinishSwitchWeaponHand();


	UPROPERTY(ReplicatedUsing = OnRep_ThrowingGrenade)
	bool bThrowingGrenade;
	UFUNCTION()
	void OnRep_ThrowingGrenade(bool bPrevValue);

	uint32 bWantsToFinishGrenadeThrow : 1;
	class ATacticalThrowable* PendingThrowable;

	bool CanThrowGrenade(class ATacticalThrowable* GadgetToThrow) const;

	void OnGrenadePressed();

	void EnableThrowGrenade();

	void StartGrenadeThrow();


	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStartGrenadeThrow(class ATacticalThrowable* GadgetToThrow);
	void ServerStartGrenadeThrow_Implementation(class ATacticalThrowable* GadgetToThrow);
	bool ServerStartGrenadeThrow_Validate(class ATacticalThrowable* GadgetToThrow) { return true; }


	UFUNCTION(Category = TacticalCharacter, BlueprintNativeEvent)
	void OnStartGrenadeThrow();
	void OnStartGrenadeThrow_Implementation() {}

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerFinishGrenadeThrow();
	void ServerFinishGrenadeThrow_Implementation();
	bool ServerFinishGrenadeThrow_Validate() { return true; }

	UFUNCTION(Reliable, NetMulticast, WithValidation)
	void NetMulti_FinishGrenadeThrow();
	void NetMulti_FinishGrenadeThrow_Implementation() { OnFinishGrenadeThrow(); }
	bool NetMulti_FinishGrenadeThrow_Validate() { return true; }

	UFUNCTION(Category = TacticalCharacter, BlueprintNativeEvent)
	void OnFinishGrenadeThrow();
	void OnFinishGrenadeThrow_Implementation() {}

	void CancelGrenadeThrow();
	UFUNCTION(Reliable, Server, WithValidation)
	void ServerCancelGrenadeThrow();
	void ServerCancelGrenadeThrow_Implementation();
	bool ServerCancelGrenadeThrow_Validate() { return true; }

	UFUNCTION(Category = TacticalCharacter, BlueprintNativeEvent)
	void OnCancelGrenadeThrow();
	void OnCancelGrenadeThrow_Implementation() {}

	void ThrowGrenade();






		/** notification when killed, for both the server and client. */
	UFUNCTION()
		void OnDeath(float Damage, const class UDamageType* DamageType, class APawn* InstigatedBy, AActor* DamageCauser);
	UFUNCTION()
		void OnDamage(float Damage, const class UDamageType* DamageType, class APawn* InstigatedBy, AActor* DamageCauser);

	/** Detaches character from controller. */
	UFUNCTION()
		virtual void DetachCharacterFromController();

	FTimerHandle TimerDetachFromController;

	/** Enables rag doll physics for Character mesh. */
	virtual void SetRagdollPhysics();

	virtual void StopAllAnimMontages();

	UFUNCTION(Category = Character, BlueprintNativeEvent)
		void PlayDeathAnimation();
	virtual void PlayDeathAnimation_Implementation();

	UPROPERTY(ReplicatedUsing = OnRep_FullyRagdoll)
		bool bFullyRagdoll;

	UFUNCTION()
		void OnRep_FullyRagdoll();

	UFUNCTION(Unreliable, Server, WithValidation)
	void ServerSendLeanAngle(float Angle);
	void ServerSendLeanAngle_Implementation(float Angle);
	bool ServerSendLeanAngle_Validate(float Angle) { return true; }


	FVector2D RecoilToDo;

	TArray<FInputBinding> InputBindings;

	UFUNCTION(exec)
	void Suicide();

	FTimerHandle VaultTimer;
	UPROPERTY(Replicated)
	uint32 bIsVaulting : 1;
	UPROPERTY(Replicated)
	uint32 bVaultForward : 1;
	UPROPERTY(Replicated)
	class ATacticalVaultMarker* CurrentVaultMarker;

	UPROPERTY(Replicated)
	uint32 bIsStunned : 1;

	FTimerHandle TimerStunned;
	UFUNCTION(Category = TacticalCharacter, BlueprintCallable)
	void Stun(float Duration);
	void EndStun();

	void PerformHitReaction(const class UTacticalDamageType* DamageType);
	UFUNCTION(Category = "Character | AI", Reliable, NetMultiCast)
	void NetMulti_PerformHitReaction(uint8 MontageIdx, float StunTime);
	void NetMulti_PerformHitReaction_Implementation(uint8 MontageIdx, float StunTime);

	UPROPERTY(Category = "Animation", EditDefaultsOnly)
	UAnimMontage* HitMontageDefault;
	UPROPERTY(Category = "Animation", EditDefaultsOnly)
	UAnimMontage* HitMontageRifle;
	UPROPERTY(Category = "Animation", EditDefaultsOnly)
	UAnimMontage* HitMontagePistol;

	UAnimMontage* GetHitMontage() const;
	FTimerHandle HitReactionTimer;
	void FinishHitReaction();


protected:

	FTimerHandle WeaponHandSwitchTimer;

	bool bForceRotation;
	bool bMoveFwd;
	bool bMoveRight;

	void TickFootIK(float DeltaTime);
	bool TraceFoot(FHitResult& Hit, const FName& FootSocketName);

	FVector RightFootLoc;
	FVector LeftFootLoc;
	FVector RightFootNorm;
	FVector LeftFootNorm;


	float RootZAdjust;

	UPROPERTY(Category = Animation, EditDefaultsOnly)
		float RootAdjustSpeed;
	UPROPERTY(Category = Animation, EditDefaultsOnly)
		float MaxRootAdjustment;
	UPROPERTY(Category = Animation, EditDefaultsOnly)
		float FootPlacementSpeed;

	UPROPERTY(Category = Weapon, EditDefaultsOnly)
		FBRSPerlinNoiseStream1D WeaponSwayX;
	UPROPERTY(Category = Weapon, EditDefaultsOnly)
		FBRSPerlinNoiseStream1D WeaponSwayY;
	FVector WeaponSway;

	UPROPERTY(Category = AI, EditInstanceOnly)
		UBehaviorTree* BehaviorTreeAssetOverride;

	UPROPERTY(Replicated)
		bool bIsAIControlled;

	UPROPERTY(Replicated)
		bool bWeaponDown;

	UPROPERTY(Category = Sound, BlueprintReadOnly, EditDefaultsOnly)
	TMap<TEnumAsByte<EPhysicalSurface>, USoundCue*> FootstepSounds;
	UPROPERTY(Category = Sound, BlueprintReadOnly, EditDefaultsOnly)
	USoundCue* DefaultFootStepSound;

	
public:

	bool IsWeaponDown() { return bWeaponDown; }

	/** Returns CameraBoom subobject **/
//	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	void AdjustMeshHeight(float HalfHeightAdjust, float ScaledHalfHeightAdjust);

	UFUNCTION(Category = Health, BlueprintCallable, BlueprintPure)
	class UBRSActorHealthComponent* GetHealthComponent() const { return HealthComponent; }


	UFUNCTION(Category = Inventory, BlueprintCallable, BlueprintPure)
	class UTacticalInventoryComponent* GetInventory() const { return InventoryComponent; }

	void Tick(float DeltaSeconds) override;

public:
	// overrides from IGenericTeamAgentInterface
	/** Assigns Team Agent to given TeamID */
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID);

	/** Retrieve team identifier in form of FGenericTeamId */
	virtual FGenericTeamId GetGenericTeamId() const;

	/** Retrieved owner attitude toward given Other object */
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const;
	
	// overrides from IAISightTargetInterface
	/**	Implementation should check whether from given ObserverLocation
	*	implementer can be seen. If so OutSeenLocation should contain
	*	first visible location
	*  Return sight strength for how well the target is seen.
	*/
	virtual bool CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation, int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor = NULL) const;


	UFUNCTION(Category = TacticalCharacter, BlueprintCallable)
	bool IsAIControlled() const;
	UFUNCTION(Category = TacticalCharacter, BlueprintPure)
	class ATacticalPlayerController* GetTacticalPlayerController() const;
	UFUNCTION(Category = TacticalCharacter, BlueprintPure)
	class ATacticalAIController* GetTacticalAIController() const;

	UFUNCTION(Category = Weapon, BlueprintCallable)
	const FVector& GetWeaponSway() const { return WeaponSway; }

	// Curve Velocity -> Shake Speed
	UPROPERTY(Category = Weapon, EditDefaultsOnly)
		FRuntimeFloatCurve WeaponShakeFrequencyCurve;
	// Curve Velocity -> Aplitude Speed
	UPROPERTY(Category = Weapon, EditDefaultsOnly)
		FRuntimeFloatCurve WeaponShakeAmplitudeCurve;


	UBehaviorTree* GetBehaviorTreeAssetOverride() const { return BehaviorTreeAssetOverride; }

	UPROPERTY(Replicated)
	bool bAIWantsToFire;
	UPROPERTY(Replicated)
	bool bAIWantsToCover;
	UPROPERTY(Replicated)
	ECoverAimType CoverAimType;

	void SetAIWantsToCoverFire(bool bVal);
	UFUNCTION(Category = "AI | Cover", BlueprintCallable)
		bool GetAIWantsToCoverFire() const { return bAIWantsToCover; };

	class ATacticalCoverMarker* GetBestCover() const;


	UFUNCTION(exec)
		void RemoveWeapon();
};

