// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "AIController.h"
#include "GenericTeamAgentInterface.h"
#include "TacticalControllerInterface.h"
#include "TacticalAIController.generated.h"


USTRUCT(Blueprintable)
struct TACTICALFPS_API FEnemyPerceptionInfo
{
	GENERATED_USTRUCT_BODY()
public:
	FEnemyPerceptionInfo() : 
		TimeStamp(0.f),
		LastConfirmedLocation(FVector::ZeroVector),
		LastConfirmedVelocity(FVector::ZeroVector),
		Target(nullptr)
	{}

	FVector LastConfirmedLocation;
	FVector LastConfirmedVelocity;
	TWeakObjectPtr<AActor> Target;
	float TimeStamp;
};

/**
 * 
 */
UCLASS()
class TACTICALFPS_API ATacticalAIController : public AAIController, public ITacticalControllerInterface /*, public IGenericTeamAgentInterface*/
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	class UAIPerceptionComponent* AIPerceptionComp;

	UPROPERTY()
	class UAISenseConfig_Sight* SightConfig;
	UPROPERTY()
	class UAISenseConfig_Hearing* HearingConfig;
	UPROPERTY()
	class UAISenseConfig_Damage* DamageConfig;
	UPROPERTY()
	class UAISenseConfig_Team* TeamConfig;

public:
	ATacticalAIController(const FObjectInitializer& OI);

	virtual void BeginPlay() override;
	virtual void Possess(class APawn* PossessedPawn) override;
	virtual void UnPossess() override;

	virtual void Tick(float DeltaTime) override;

	// From AIInterfaces

	/** Assigns Team Agent to given TeamID */
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	/** Retrieve team identifier in form of FGenericTeamId */
	virtual FGenericTeamId GetGenericTeamId() const override;

	/** Retrieved owner attitude toward given Other object */
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

	// TacticalControlerInterface
	virtual void OnPawnDied_Implementation(float Damage, const class UDamageType* DamageType, class APawn* InstigatedBy, AActor* DamageCauser) override;
	virtual void OnPawnTakeDamage_Implementation(float Damage, const class UDamageType* DamageType, class APawn* InstigatedBy, AActor* DamageCauser) override;


	UFUNCTION(Category = TacticalAI, BlueprintCallable)
	virtual float GetAwareness() const;

	UFUNCTION(Category = TacticalAI, BlueprintCallable)
	virtual void GetEnemiesInSight(TArray<AActor*>& OutActors) const;

	UFUNCTION(Category = TacticalAI, BlueprintCallable)
	virtual void GetPercievedDamageOrigins(TArray<FVector>& OutVectors, TArray<AActor*>& OutActors, float MaxAge = 0.f) const;

	UFUNCTION(Category = TacticalAI, BlueprintCallable)
		virtual void GetPercievedHearingOrigins(TArray<FVector>& OutVectors, TArray<AActor*>& OutActors, float MaxAge = 0.f, TSubclassOf<AActor> ActorClassFilter =nullptr) const;

	UFUNCTION(Category = TacticalAI, BlueprintCallable)
	virtual void GetPercievedTeamNotifications(TArray<FVector>& OutVectors, float MaxAge = 0.f) const;

	UFUNCTION(Category = TacticalAI, BlueprintCallable)
	virtual bool GetBestDamageOrigin(FVector& OutVector, float MaxCost, float MaxAge = 0.f) const;

	UFUNCTION(Category = TacticalAI, BlueprintCallable)
		virtual bool GetBestHearingOrigin(FVector& OutVector, float MaxCost, float MaxAge = 0.f) const;

	UFUNCTION(Category = TacticalAI, BlueprintCallable)
		virtual bool GetBestTeamOrigin(FVector& OutVector, float MaxCost, float MaxAge = 0.f) const;

	UFUNCTION(Category = TacticalAI, BlueprintCallable)
	virtual void GetPerceivedGrenades(TArray<class ATacticalProjectile*>& OutGrenades) const;

	UFUNCTION(Category = TacticalAI, BlueprintCallable)
		virtual AActor* GetBestChaseActor(float MaxCost, float MaxAge) const;


	UFUNCTION(Category = TacticalAI, BlueprintCallable)
	virtual bool IsFocusActorInSight() const;

	UFUNCTION(Category = TacticalAI, BlueprintCallable)
	virtual void NotifyTeamInRange(float Range, AActor* NotifyActor);

	// tries to find most recent perception for actor for estimation
	UFUNCTION(Category = "AI | Combat", BlueprintCallable)
		virtual bool GetActorPerceptionLocation(AActor* TrackingActor, FVector& OutLocation, float& OutAge);
	
	UFUNCTION(Category = TacticalAI, BlueprintCallable)
	bool IsIdle() const { return bIsIdle; }
	
	UFUNCTION(Category = "AI | Combat", BlueprintCallable)
	class ATacticalCharacter* GetTacticalCharacter() const;
	
	UFUNCTION(Category = TacticalAI, BlueprintCallable)
	void OnStartChasingLastTarget();
	UFUNCTION(Category = TacticalAI, BlueprintCallable)
	void OnStartAttackingTarget();
	UFUNCTION(Category = TacticalAI, BlueprintCallable)
	void OnReturnToIdle();
	
	UFUNCTION(Category = "AI | Combat", BlueprintCallable)
	float GetAimConfidence() const;
	UFUNCTION(Category = "AI | Combat", BlueprintCallable)
	int32 GetShotCount() const;



	UFUNCTION(Category = TacticalAI, BlueprintNativeEvent, BlueprintCallable)
	bool OpenDoor(const FVector& Dir);
	virtual bool OpenDoor_Implementation(const FVector& Dir){ return false; }

	UFUNCTION(Category = "AI | Cover", BlueprintCallable)
	void SetWantsToCover(bool newWantstoCover);
	
	UFUNCTION(Category = "AI | Cover", BlueprintCallable)
		bool WantsToCover() const;

	UFUNCTION(Category = "AI | Combat", BlueprintCallable)
		void SetWantsToCoverFire(bool newVal);

	UFUNCTION(Category = "AI | Combat", BlueprintCallable)
		bool WantsToCoverFire() const;

	virtual void OnEnteredCover();
	virtual void OnLeftCover();


	UFUNCTION(Category = "AI | Movement", BlueprintCallable)
		void SetWantsToSprint(bool newWantsToSprint);

	UFUNCTION(Category = "AI | Movement", BlueprintCallable)
		void SetDesiredSpeedModifier(float newSpeedMod);

	UFUNCTION(Category = "AI | Combat", BlueprintCallable)
	virtual bool IsGrenadeInProximity(float Range=500.f) const;

	UFUNCTION(Category = "AI | Combat", BlueprintCallable)
	virtual FVector GetAimLocation() const;


	UFUNCTION(Category = "AI | Combat", BlueprintCallable)
	virtual int32 GetBurstCount() const;

	UFUNCTION(Category = "AI | Combat", BlueprintCallable)
	virtual float GetBurstInterval() const;
	UFUNCTION(Category = AI, BlueprintCallable)
		bool ReserveCover(FVector CoverLocation);
	bool CanReserveCover(FVector CoverLocation);


	UPROPERTY(Category = TacticalAI, BlueprintReadWrite)
	TSubclassOf<class UPawnAction> IdlePawnActionClass;

	UPROPERTY(Category = TacticalAI, BlueprintReadWrite)
	class UPawnAction* IdlePawnAction;





protected:
	void UpdateControlRotation(float DeltaTime, bool bUpdatePawn /* = true */) override;

	UFUNCTION()
	virtual void OnSenseUpdate(const TArray<AActor*>& SensedActors);

	void SetAlertness(float NewValue);

	bool bWantsToFire;
	bool bWantsToCover;
	bool bCanCover;



protected:
	UPROPERTY(Category = Team, EditAnywhere)
	FGenericTeamId TeamID;

	bool bIsIdle;
	bool bWantsToMiss;

	FVector2D RotationErrorIntegrator;
	FVector2D RotationSpeedErrorIntegrator;
	FVector2D AimVelocity;

	UPROPERTY(Category = Aim, EditAnywhere)
	float KP_Aim;
	UPROPERTY(Category = Aim, EditAnywhere)
	float KI_Aim;
	UPROPERTY(Category = Aim, EditAnywhere)
	float KP_AimSpeed;
	UPROPERTY(Category = Aim, EditAnywhere)
	float KI_AimSpeed;

	UPROPERTY(Category = Aim, EditAnywhere)
	float MaxAimRotationAccel;

	UPROPERTY(Category = Aim, EditAnywhere)
	float MaxAimRotationSpeed;

	UPROPERTY(Category = Aim, EditAnywhere)
	float AimError;


	UPROPERTY(Category = Perception, EditAnywhere)
	FRuntimeFloatCurve VisionStrengthOverDistance;
	UPROPERTY(Category = Perception, EditAnywhere)
		FRuntimeFloatCurve VisionAngleOverDistance;
	UPROPERTY(Category = Perception, EditAnywhere)
	float VisionAngleMargin;


	// 0 - does not expect enemy. 1 - does know of enemy and has a idea of the location (hearing, damage, team) but has not seem him. 2 engaged
	UPROPERTY(Category = Alert, BlueprintReadOnly)
	TMap<class AActor*, float> Alertness;
	float OverallAlertness;

	// Minimal Level of Alertness. Prevents the  Alertness value to go down further if enemy knows that a thread could be running around
	UPROPERTY(Category = Alert, EditDefaultsOnly, BlueprintReadWrite)
	float MinAlertness;
	// Minimal Level of Alertness. Prevents the  Alertness value to go down further if enemy knows that a thread could be running around
	UPROPERTY(Category = Alert, EditDefaultsOnly, BlueprintReadWrite)
	float MaxAlert;
	UPROPERTY(Category = Alert, EditDefaultsOnly, BlueprintReadOnly)
	float AlertRateScale;
	UPROPERTY(Category = Alert, EditDefaultsOnly, BlueprintReadOnly)
	float AlertRegenRate;


	FVector OriginalLocation;
	FRotator OriginalRotation;

	UPROPERTY(Category = AI, EditAnywhere)
	UBehaviorTree* BehaviorTreeAsset;
	UPROPERTY(Category = AI, EditAnywhere)
	UBlackboardData* BlackBoardAsset;

	UPROPERTY(Category = Aim, EditAnywhere)
	TSubclassOf<UNavigationQueryFilter> HearingNavQueryFilter;


	UFUNCTION(Category = AI, BlueprintNativeEvent)
	void InitializeBlackboardKeys();
	virtual void InitializeBlackboardKeys_Implementation();



};
