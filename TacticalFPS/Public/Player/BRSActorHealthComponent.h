// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Components/ActorComponent.h"
#include "BRSActorHealthComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthDamageSignature, float, Damage, const class UDamageType*, DamageType, class APawn*, InstigatedBy, AActor*, DamageCauser);


/** replicated information on a hit we've taken */
USTRUCT()
struct TACTICALFPS_API FTakeHitReplicationInfo
{
	GENERATED_USTRUCT_BODY()

public:
	/** The amount of damage actually applied */
	UPROPERTY()
	float ActualDamage;

	/** The damage type we were hit with. */
	UPROPERTY()
	UClass* DamageTypeClass;

	/** Who hit us */
	UPROPERTY()
	TWeakObjectPtr<class APawn> PawnInstigator;

	/** Who actually caused the damage */
	UPROPERTY()
	TWeakObjectPtr<class AActor> DamageCauser;

	/** Specifies which DamageEvent below describes the damage received. */
	UPROPERTY()
	int32 DamageEventClassID;

	/** Rather this was a kill */
	UPROPERTY()
	uint32 bKilled : 1;

private:

	/** A rolling counter used to ensure the struct is dirty and will replicate. */
	UPROPERTY()
	uint8 EnsureReplicationByte;

	///** Describes general damage. */
	//UPROPERTY()
	//FDamageEvent GeneralDamageEvent;
	///** Describes point damage, if that is what was received. */
	//UPROPERTY()
	//FPointDamageEvent PointDamageEvent;
	///** Describes radial damage, if that is what was received. */
	//UPROPERTY()
	//FRadialDamageEvent RadialDamageEvent;

public:
	FTakeHitReplicationInfo()
		: ActualDamage(0)
		, DamageTypeClass(NULL)
		, PawnInstigator(NULL)
		, DamageCauser(NULL)
		, DamageEventClassID(0)
		, bKilled(false)
		, EnsureReplicationByte(0)
	{}

	//FDamageEvent& GetDamageEvent()
	//{
	//	switch (DamageEventClassID)
	//	{
	//	case FPointDamageEvent::ClassID:
	//		if (PointDamageEvent.DamageTypeClass == NULL)
	//		{
	//			PointDamageEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass();
	//		}
	//		return PointDamageEvent;
	//	case FRadialDamageEvent::ClassID:
	//		if (RadialDamageEvent.DamageTypeClass == NULL)
	//		{
	//			RadialDamageEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass();
	//		}
	//		return RadialDamageEvent;
	//	default:
	//		if (GeneralDamageEvent.DamageTypeClass == NULL)
	//		{
	//			GeneralDamageEvent.DamageTypeClass = DamageTypeClass ? DamageTypeClass : UDamageType::StaticClass();
	//		}
	//		return GeneralDamageEvent;
	//	}
	//}

	//void SetDamageEvent(const FDamageEvent& DamageEvent)
	//{
	//	DamageEventClassID = DamageEvent.GetTypeID();
	//	switch (DamageEventClassID)
	//	{
	//	case FPointDamageEvent::ClassID:
	//		PointDamageEvent = *((FPointDamageEvent const*)(&DamageEvent));
	//		break;
	//	case FRadialDamageEvent::ClassID:
	//		RadialDamageEvent = *((FRadialDamageEvent const*)(&DamageEvent));
	//		break;
	//	default:
	//		GeneralDamageEvent = DamageEvent;
	//	}
	//	DamageTypeClass = DamageEvent.DamageTypeClass;
	//}

	void EnsureReplication()
	{
		EnsureReplicationByte++;
	}
};



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TACTICALFPS_API UBRSActorHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBRSActorHealthComponent();

	virtual void InitializeComponent() override;

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	virtual bool CanDie(float KillingDamage, const class UDamageType* DamageType, AController* Killer, AActor* DamageCauser) const;




	UPROPERTY(BlueprintAssignable, Category = "Health Component")
	FOnHealthDamageSignature OnDamage;

	UPROPERTY(BlueprintAssignable, Category = "Health Component")
	FOnHealthDamageSignature OnDeath;

	UFUNCTION(Category = "Health Component", BlueprintCallable)
	float GetCurrentHealth() const;
	UFUNCTION(Category = "Health Component", BlueprintCallable)
	float GetMaxHealth() const;

	UFUNCTION(Category = "Health Component", BlueprintCallable)
	float GetRelativeHealth() const;

	UFUNCTION(Category = "Health Component", BlueprintCallable)
	bool IsAlive() const;

protected:
	UFUNCTION()
	virtual void OnTakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	virtual bool Die(float KillingDamage, const class UDamageType* DamageType, AController* Killer, AActor* DamageCauser);

	void ReplicateHit(float Damage, const class UDamageType* DamageType, class APawn* PawnInstigator, class AActor* DamageCauser, bool bKilled);
	
	UFUNCTION()
	void OnRep_LastHit();

protected:
	UPROPERTY(Category = "Health Component", BlueprintReadOnly, Replicated)
	float Health;
	UPROPERTY(Category = "Health Component", EditAnywhere, BlueprintReadOnly, Replicated)
	float MaxHealth;

	UPROPERTY(Replicated)
	uint32 bDied : 1;

	/** Replicate where this pawn was last hit and damaged */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_LastHit)
	FTakeHitReplicationInfo LastHitInfo;

	/** Time at which point the last take hit info for the actor times out and won't be replicated; Used to stop join-in-progress effects all over the screen */
	float LastHitTimeoutTime;
};
