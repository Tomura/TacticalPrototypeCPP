// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "TacticalTypes.h"
#include "TacticalCharacterMovement.generated.h"



/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalCharacterMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	friend class ATacticalCharacter;
	
	UTacticalCharacterMovement();
	
	virtual class FNetworkPredictionData_Client_Character* GetPredictionData_Client() const override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;


	virtual float GetMaxSpeed() const override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void PerformMovement(float DeltaTime) override;

	virtual void ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations) override;


	bool CanSwitchStance();

	virtual bool SetPendingStance(ETacticalStance NewStance);

	UPROPERTY(Replicated)
	ETacticalStance DesiredStance;

	UFUNCTION(Category = Movement, BlueprintCallable)
	ETacticalStance GetCurrentStance() const;

	UFUNCTION(Category = Movement, BlueprintCallable)
	ETacticalStance GetDesiredStance() const;
	UFUNCTION(Category = Movement, BlueprintCallable, BlueprintPure)
	bool IsSprinting() const;
	UFUNCTION(Category = Movement, BlueprintCallable, BlueprintPure)
	bool WantsToSprint() const { return bWantsToSprint; }

	UFUNCTION(Category = Movement, BlueprintCallable, BlueprintPure)
	bool CanSprint() const;

	UFUNCTION(Category = Movement, BlueprintCallable, BlueprintPure)
	bool IsChangingStance() const;

	UFUNCTION(Category = Movement, BlueprintCallable)
	bool IsInCover() const { return bInCover; }
	UFUNCTION(Category = Movement, BlueprintCallable)
	bool WantsToCover() const { return bWantsToCover; }


	UPROPERTY(Replicated)
	uint32 bWantsToSprint : 1;
	UPROPERTY(Replicated)
	uint32 bSprinting : 1;

	UPROPERTY(Replicated)
	uint32 bWalking : 1;

	UPROPERTY(Replicated)
	uint32 bInCover : 1;


	//UPROPERTY(Replicated)
	uint32 bWantsToCover : 1;

protected:
	virtual void TryCrouch();
	virtual void TryProne();
	virtual void TryStanding();

	bool ChangeCollisionToSize(float DesiredHeight);

	UPROPERTY(Category = "Movement | Walking", EditAnywhere)
	float MaxWalkSpeedProne;
	UPROPERTY(Category = "Movement | Walking", EditAnywhere)
	float MaxSprintSpeed;

	ETacticalStance Stance;

	FTimerHandle ProneTransitionTimer;
	FTimerHandle CrouchTransitionTimer;

	UPROPERTY(Category = "Movement | Stance", EditAnywhere)
	float CrouchTransitionTime;
	UPROPERTY(Category = "Movement | Stance", EditAnywhere)
	float ProneTransitionTime;

	UPROPERTY(Category = "Movement | Stance", EditAnywhere)
	float AimSpeedModifier;

	virtual void PhysWalking(float deltaTime, int32 Iterations) override;

	FTimerHandle PostLandedCooldown;
	UPROPERTY(Category = "Movement | Jumping", EditAnywhere)
	float PostJumpStopTime;

};


class TACTICALFPS_API FSavedMove_TacticalCharacter : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;

	FSavedMove_TacticalCharacter()
	{

	}

	// flags saved for simulation/sync
	bool bWantsToSprint;
	bool bSprinting;
	bool bCrouch;
	bool bProne;


	virtual void Clear() override;
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData) override;
	virtual uint8 GetCompressedFlags() const override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
	virtual bool IsImportantMove(const FSavedMovePtr& LastAckedMove) const override;
	//virtual void PostUpdate(class ACharacter* C, EPostUpdateMode PostUpdateMode) override;
	virtual void PrepMoveFor(class ACharacter* C) override;
};

class TACTICALFPS_API FNetworkPredictionData_Client_TacticalCharacter : public FNetworkPredictionData_Client_Character
{
public:
	typedef FNetworkPredictionData_Client_Character Super;

	FNetworkPredictionData_Client_TacticalCharacter(const UTacticalCharacterMovement& ClientMovement) : FNetworkPredictionData_Client_Character(ClientMovement) {}

	/** Allocate a new saved move. Subclasses should override this if they want to use a custom move class. */
	virtual FSavedMovePtr AllocateNewMove() override;

};