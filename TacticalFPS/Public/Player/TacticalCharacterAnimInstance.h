// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "TacticalTypes.h"
#include "BRSPerlinNoiseStream.h"
#include "TacticalCharacterAnimInstance.generated.h"




USTRUCT(BlueprintType)
struct TACTICALFPS_API FTwoBoneJointData
{
	GENERATED_BODY()
public:
	FTwoBoneJointData() :
		TargetLocation(FVector::ZeroVector),
		TargetRotation(FRotator::ZeroRotator),
		JointLocation(FVector::ZeroVector),
		bIsActive(false)
	{}

	FTwoBoneJointData(const FVector& inTargetLocation, const FRotator& inTargetRotation, const FVector& inJointLocation) :
		TargetLocation(inTargetLocation),
		TargetRotation(inTargetRotation),
		JointLocation(inJointLocation),
		bIsActive(false)
	{}

	UPROPERTY(Category = Default, BlueprintReadWrite, EditAnywhere)
	bool bIsActive;

	UPROPERTY(Category = Default, BlueprintReadWrite, EditAnywhere)
	FVector TargetLocation;

	UPROPERTY(Category = Default, BlueprintReadWrite, EditAnywhere)
	FRotator TargetRotation;

	UPROPERTY(Category = Default, BlueprintReadWrite, EditAnywhere)
	FVector JointLocation;
};


USTRUCT(BlueprintInternalUseOnly)
struct FTacticalAnimInstanceProxy :public FAnimInstanceProxy
{
	GENERATED_BODY()

public:
	FTacticalAnimInstanceProxy() : FAnimInstanceProxy() {}
	FTacticalAnimInstanceProxy(UAnimInstance* Instance) : FAnimInstanceProxy(Instance) {}

	/** Update Internal variables*/
	virtual void Update(float DeltaSeconds) override;


	UPROPERTY(BlueprintReadWrite, Category = Test, EditAnywhere)
		float ASDASDA;

	// variables
};





/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Animation | Proxy", meta = (AllowPrivateAccess = "true"))
	FTacticalAnimInstanceProxy Proxy;


	friend struct FTacticalAnimInstanceProxy;


public:
	UTacticalCharacterAnimInstance();
	
	virtual void NativeUpdateAnimation(const float DeltaTime) override;

	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override
	{
		// override just to return the proxy
		return &Proxy;
	}

	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override
	{
		// proxy is a struct variable of this class and will be destroyed with it.
	}



	UFUNCTION(BlueprintNativeEvent, Category="Animation | Weapon")
	FVector GetGunShake();
	virtual FVector GetGunShake_Implementation();

	virtual void GetRecoilAnim(FVector& OutOffset, FRotator& OutPitch);

	UFUNCTION(BlueprintNativeEvent, Category = "Animation | Weapon")
	void CalculateGunLag(const float DeltaTime, FVector& Translation, FVector& Rotation) ;
	virtual void CalculateGunLag_Implementation(const float DeltaTime, FVector& Translation, FVector& Rotation) ;

	virtual FRotator CalculateFootRotfromNormal(const FVector& Normal) const;

public:
	UPROPERTY(Category = Character, BlueprintReadOnly, EditAnywhere)
	bool bIsAlive;

	UPROPERTY(Category = Character, BlueprintReadOnly, EditAnywhere)
		bool bFirstPerson;

	UPROPERTY(Category = Movement, BlueprintReadOnly, EditAnywhere)
	ETacticalRotationType CurrentRotation;

	UPROPERTY(Category = Movement, BlueprintReadOnly, EditAnywhere)
	FRotator RelativeAim;


	UPROPERTY(Category = Movement, BlueprintReadOnly, EditAnywhere)
	FVector2D VelocityDir;
	UPROPERTY(Category = Movement, EditAnywhere)
	float VelocityInterpSpeed;


	UPROPERTY(Category = Movement, BlueprintReadOnly, EditAnywhere)
	float Direction;
	UPROPERTY(Category = Movement, BlueprintReadOnly, EditAnywhere)
	float Speed;

	UPROPERTY(Category = Movement, BlueprintReadOnly, EditAnywhere)
	float Accel;

	UPROPERTY(Category = Movement, BlueprintReadOnly, EditAnywhere)
	FRotator LeanRotation;

	UPROPERTY(Category = Movement, BlueprintReadOnly, EditAnywhere)
	bool bIsSprinting;

	UPROPERTY(Category = Movement, BlueprintReadOnly, EditAnywhere)
	bool bIsFalling;


	UPROPERTY(Category = Movement, BlueprintReadOnly, EditAnywhere)
	bool bIsRelaxed;

	UPROPERTY(Category = Movement, BlueprintReadOnly, EditAnywhere)
	ETacticalStance Stance;



	UPROPERTY(Category = Hands, BlueprintReadOnly, EditAnywhere)
	FRotator DefaultRightHandRotation;
	UPROPERTY(Category = Hands, BlueprintReadOnly, EditAnywhere)
	FTwoBoneJointData RightHandData;
	UPROPERTY(Category = Hands, BlueprintReadOnly, EditAnywhere)
	FTwoBoneJointData LeftHandData;

	UPROPERTY(Category = Hands, BlueprintReadOnly)
	float FPHandStrength;


	UPROPERTY(Category = Feet, BlueprintReadOnly, EditAnywhere)
	FTwoBoneJointData RightFootData;
	UPROPERTY(Category = Feet, BlueprintReadOnly, EditAnywhere)
	FTwoBoneJointData LeftFootData;
	
	/** Cached translation view/movement lag */
	FVector ViewRotationLag;
	/** Cached rotational view/movement lag*/
	FVector ViewRotationLag_Rot;

	// Weapon Lag

	UPROPERTY(Category = "Weapon|Lag", EditAnywhere)
	FVector2D ViewRotationLagStrength;

	UPROPERTY(Category = "Weapon|Lag", EditAnywhere)
	FVector2D ViewRotationLagStrength_Rot;

	UPROPERTY(Category = "Weapon|Lag", EditAnywhere)
	FVector MovementLagStrength;

	UPROPERTY(Category = "Weapon|Lag", EditAnywhere)
	FVector MovementLagStrength_Rot;

	UPROPERTY(Category = "Weapon|Lag", EditAnywhere)
	FVector ViewRotationLagLimit;
	UPROPERTY(Category = "Weapon|Lag", EditAnywhere)
	FRotator ViewRotationLagRotLimit;

	UPROPERTY(Category = "Weapon|Gun Shake", EditAnywhere)
	FVector2D GunShakeStrength;
	UPROPERTY(Category = "Weapon|Gun Shake", EditAnywhere)
	FVector2D GunShakeStrength_Rot;

	UPROPERTY(Category = "Weapon", BlueprintReadOnly, EditAnywhere)
	EWeaponType WeaponType;

	UPROPERTY(Category = "Weapon", BlueprintReadOnly, EditAnywhere)
	bool bWeaponObstructed;

	UPROPERTY(Category = "Weapon", BlueprintReadOnly, EditAnywhere)
	FRotator HeadRotOffset;
	UPROPERTY(Category = "Weapon", BlueprintReadOnly, EditAnywhere)
	FVector HeadRotFactor;
	//UPOPERTY(Category = Movement, BlueprintReadOnly, EditAnywhere)
	//	float HeadRotStrength;

	UPROPERTY(Category = "Weapon", BlueprintReadOnly, EditAnywhere)
		bool bWeaponLeftHanded;

	UPROPERTY(Category = Cover, BlueprintReadOnly, EditAnywhere)
	ECoverType CoverType;	
	UPROPERTY(Category = Cover, BlueprintReadOnly, EditAnywhere)
	ECoverAimType CoverAimType;
	UPROPERTY(Category = Cover, BlueprintReadOnly, EditAnywhere)
		bool bCoverRight;

	UFUNCTION(BlueprintImplementableEvent, Category = Animation, BlueprintCallable)
	void PlayVaultAnimation();
	UFUNCTION(BlueprintImplementableEvent, Category = Animation, BlueprintCallable)
	void PlayClimbAnimation();

protected:
	//UFUNCTION(Category = Anims, BlueprintCallable)
	//ETacticalStance GetStance() const;

	float WeaponMoveBackDist;

	UPROPERTY(Category = "Weapon", BlueprintReadOnly, EditAnywhere)
	FVector2D GunShakeTranslationFactor;

	UPROPERTY(Category = "Weapon", EditAnywhere)
	bool bAdvancedReloadAnim;

	UPROPERTY(Category = "Weapon", BlueprintReadOnly)
	float ReloadStrength;

	UPROPERTY(Category = "Weapon", BlueprintReadOnly)
	USkeletalMeshComponent* FPMesh;

	UFUNCTION(Category = "Movement", BlueprintPure)
		bool IsMoving() const;
};
