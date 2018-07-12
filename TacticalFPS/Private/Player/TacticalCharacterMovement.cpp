// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalCharacterMovement.h"
#include "TacticalCharacter.h"
#include "TacticalAIController.h"
#include "TacticalWeapon.h"
#include "UnrealNetwork.h"
#include "TacticalCoverMarker.h"




UTacticalCharacterMovement::UTacticalCharacterMovement()
	: Super()
{
	SetIsReplicated(true);
	bReplicates = true;
	NavAgentProps.bCanJump = false;
	NavAgentProps.bCanCrouch = true;

	bWantsToSprint = false;


	MaxWalkSpeedProne = 80.f;

	MaxWalkSpeed = 320.f;
	MaxWalkSpeedCrouched = 160.f;
	MaxSprintSpeed = 600.f;

	bCrouchMaintainsBaseLocation = true;

	ProneTransitionTime = 2.f;
	ProneTransitionTime = 1.5f;
	AimSpeedModifier = 0.8f;

	PostJumpStopTime = 0.4f;

	bUseAccelerationForPaths = true;
	
	MovementMode = MOVE_Walking;

	DesiredStance = ETacticalStance::STANCE_Default;
	Stance = ETacticalStance::STANCE_Default;
}

void UTacticalCharacterMovement::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UTacticalCharacterMovement, DesiredStance, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(UTacticalCharacterMovement, bSprinting, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(UTacticalCharacterMovement, bInCover, COND_SimulatedOnly);
}

float UTacticalCharacterMovement::GetMaxSpeed() const
{
	if (GetWorld()->GetTimerManager().IsTimerActive(PostLandedCooldown))
	{
		return 0.f;
	}

	ATacticalCharacter* myChar = CharacterOwner ? Cast<ATacticalCharacter>(CharacterOwner) : nullptr;
	bool bAiming = false;
	if (myChar != nullptr)
	{
		bAiming = myChar->IsAiming();
		if (MovementMode == MOVE_Walking)
		{
			
			const bool bIsAI = myChar->IsAIControlled();
			switch (Stance)
			{
			case ETacticalStance::STANCE_Default:
				if (bWantsToSprint && CanSprint())
				{
					// Get Forward part of Speed
					const float SpeedRatio = MaxWalkSpeed / MaxSprintSpeed;
					const FVector AxisFwd = myChar->GetActorForwardVector();

					const float Fwd = bIsAI ? 
						FMath::Max((AxisFwd | Velocity.GetSafeNormal2D()), 0.f):	// AI does not have acceleration in MoveComp
						FMath::Max((AxisFwd | Acceleration.GetSafeNormal2D()), 0.f); // for player acceleration is equivalent to input

					//float SpeedMod = SpeedRatio + (1.f - SpeedRatio) * Fwd;
					return (Fwd > 0.3f) ? MaxSprintSpeed : MaxWalkSpeed;
				}
				if (!bIsAI)
				{
					return MaxWalkSpeed * (bAiming ? AimSpeedModifier : 1.f);
				}
				else
				{
					// limit AI speed by WalkSpeedFactor. (Players already limit it with analog stick (simulation))
					const float SpeedMod = myChar ? myChar->GetWalkSpeedFactor() : 1.f;
					return MaxWalkSpeed * SpeedMod;
				}
				break;
			case ETacticalStance::STANCE_Crouched:
				return MaxWalkSpeedCrouched * (bAiming ? AimSpeedModifier : 1.f);
				break;
			case ETacticalStance::STANCE_Prone:
				return MaxWalkSpeedProne;
				break;
			default:
				break;
			}
		}
	}
	return Super::GetMaxSpeed();
}

void UTacticalCharacterMovement::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	// check if stance needs to be changed
	if (CanCrouchInCurrentState() || (CharacterOwner->Role == ROLE_SimulatedProxy))
	{
		if (Stance != GetDesiredStance())
		{
			switch (GetDesiredStance())
			{
			case ETacticalStance::STANCE_Crouched:
				TryCrouch();
				break;
			case ETacticalStance::STANCE_Prone:
				TryProne();
				break;
			case ETacticalStance::STANCE_Default:
			default:
				TryStanding();
				break;
			}
		}
	}
	else
	{
		if (Stance != ETacticalStance::STANCE_Default)
		{
			TryStanding();
		}
	}

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UTacticalCharacterMovement::PerformMovement(float DeltaTime)
{
	Super::PerformMovement(DeltaTime);
	if (ATacticalCharacter* TacticalChar = Cast<ATacticalCharacter>(CharacterOwner))
	{
		TacticalChar->PerformedMovement();
	}
}

void UTacticalCharacterMovement::ProcessLanded(const FHitResult& Hit, float remainingTime, int32 Iterations)
{
	Super::ProcessLanded(Hit, remainingTime, Iterations);

	GetWorld()->GetTimerManager().SetTimer(PostLandedCooldown, PostJumpStopTime, false);
}



bool UTacticalCharacterMovement::CanSwitchStance()
{
	return (MovementMode == MOVE_Walking);
}

ETacticalStance UTacticalCharacterMovement::GetCurrentStance() const
{
	return Stance;
}

ETacticalStance UTacticalCharacterMovement::GetDesiredStance() const
{
	//ATacticalCharacter* TacC = Cast<ATacticalCharacter>(CharacterOwner);
	//if (TacC)
	//{
	//	return TacC->PendingStance;
	//}

	return DesiredStance;
	//return ETacticalStance::STANCE_Default;
}

bool UTacticalCharacterMovement::IsSprinting() const
{
	if (CharacterOwner && (CharacterOwner->Role == ROLE_SimulatedProxy))
	{
		return bSprinting;
	}
	return (IsMovingOnGround() && Velocity.Size2D() > (MaxWalkSpeed * 1.05));
}

bool UTacticalCharacterMovement::CanSprint() const
{
	return (IsMovingOnGround() && Stance == ETacticalStance::STANCE_Default);
}

bool UTacticalCharacterMovement::IsChangingStance() const
{
	return (GetWorld()->GetTimerManager().IsTimerActive(CrouchTransitionTimer) || GetWorld()->GetTimerManager().IsTimerActive(ProneTransitionTimer));
}

void UTacticalCharacterMovement::TryCrouch()
{
	const ETacticalStance OldStance = Stance;
	if (!HasValidData())
		return;
	if (!CanCrouchInCurrentState())
		return;

	if (ChangeCollisionToSize(CrouchedHalfHeight))
	{
		// success
		Stance = ETacticalStance::STANCE_Crouched;		
		if (OldStance == ETacticalStance::STANCE_Prone)
		{
			GetWorld()->GetTimerManager().SetTimer(ProneTransitionTimer, ProneTransitionTime, false);
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(CrouchTransitionTimer, CrouchTransitionTime, false);
		}
	}
}

void UTacticalCharacterMovement::TryProne()
{
	const ETacticalStance OldStance = Stance;
	if (!HasValidData())
		return;
	if (!CanCrouchInCurrentState())
		return;

	if (ChangeCollisionToSize(CrouchedHalfHeight))
	{
		// success
		Stance = ETacticalStance::STANCE_Prone;
		GetWorld()->GetTimerManager().SetTimer(ProneTransitionTimer, ProneTransitionTime, false);
	}
}

void UTacticalCharacterMovement::TryStanding()
{
	const ETacticalStance OldStance = Stance;
	if (!HasValidData())
		return;

	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	const float DefaultHalfHeight = DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	if (ChangeCollisionToSize(DefaultHalfHeight))
	{
		// success
		Stance = ETacticalStance::STANCE_Default;
		if (OldStance == ETacticalStance::STANCE_Prone)
		{
			GetWorld()->GetTimerManager().SetTimer(ProneTransitionTimer, ProneTransitionTime, false);
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(CrouchTransitionTimer, CrouchTransitionTime, false);
		}
	}
}

bool UTacticalCharacterMovement::ChangeCollisionToSize(float DesiredHalfHeight)
{
	const bool bClientSimulation = GetOwnerRole() == ROLE_SimulatedProxy;
	ATacticalCharacter* TacticalChar = Cast<ATacticalCharacter>(CharacterOwner);
	if (!TacticalChar)
		return false;

	if (CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() == DesiredHalfHeight)
	{
		// everything is already done. Return true
		//TacticalChar->AdjustMeshHeight(0.f, 0.f);
		return true;
	}

	// todo: maybe encroach test before size change

	const float CompScale = CharacterOwner->GetCapsuleComponent()->GetShapeScale();
	const float OldHalfHeight = CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), DesiredHalfHeight);
	float HalfHeightAdjust = (OldHalfHeight - DesiredHalfHeight);
	float ScaledHalfHeightAdjust = HalfHeightAdjust * CompScale;
	
	// change collision size
	if (!bClientSimulation)
	{
		// Crouching to a larger height? (this is rare)
		if (DesiredHalfHeight > OldHalfHeight)
		{
			static const FName NAME_CrouchTrace = FName(TEXT("CrouchTrace"));
			FCollisionQueryParams CapsuleParams(NAME_CrouchTrace, false, CharacterOwner);
			FCollisionResponseParams ResponseParam;
			InitCollisionParams(CapsuleParams, ResponseParam);
			const bool bEncroached = GetWorld()->OverlapBlockingTestByChannel(UpdatedComponent->GetComponentLocation() - FVector(0.f, 0.f, ScaledHalfHeightAdjust), FQuat::Identity,
				UpdatedComponent->GetCollisionObjectType(), GetPawnCapsuleCollisionShape(SHRINK_None), CapsuleParams, ResponseParam);

			// If encroached, cancel
			if (bEncroached)
			{
				CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(CharacterOwner->GetCapsuleComponent()->GetUnscaledCapsuleRadius(), OldHalfHeight);
				return false;
			}
		}
		if (bCrouchMaintainsBaseLocation)
		{
			//UE_LOG(LogTemp, Log, TEXT("MoveComp: %f,   %f"), HalfHeightAdjust, ScaledHalfHeightAdjust);
			// Intentionally not using MoveUpdatedComponent, where a horizontal plane constraint would prevent the base of the capsule from staying at the same spot.
			UpdatedComponent->MoveComponent(FVector(0.f, 0.f, -ScaledHalfHeightAdjust), UpdatedComponent->GetComponentQuat(), true);
		}
	}
	bForceNextFloorCheck = true;
	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();
	HalfHeightAdjust = (DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - DesiredHalfHeight);

	AdjustProxyCapsuleSize();
	TacticalChar->AdjustMeshHeight(HalfHeightAdjust, ScaledHalfHeightAdjust);
	return true;
}

void UTacticalCharacterMovement::PhysWalking(float deltaTime, int32 Iterations)
{
	const float SavedAnalogModifier = AnalogInputModifier;
	const FVector SavedAccel = Acceleration;
	ATacticalCharacter* myChar = Cast<ATacticalCharacter>(CharacterOwner);

	if (myChar)
	{
		if ((CharacterOwner->Role == ROLE_Authority) && !IsSprinting() && bSprinting)
		{
			bSprinting = false;
		}

		if (GetWorld()->GetTimerManager().IsTimerActive(ProneTransitionTimer))
		{
			Acceleration = FVector::ZeroVector;
		}
		else if (Stance == ETacticalStance::STANCE_Prone && myChar->GetWeapon() && myChar->GetWeapon()->IsReloading())
		{
			Acceleration = FVector::ZeroVector;
		}
		else if (IsSprinting())
		{
			if ((CharacterOwner->Role == ROLE_Authority) && !bSprinting)
			{
				bSprinting = true;
			}
			if (Acceleration.SizeSquared2D() > 0.f)
			{
				const FVector AxisFwd = CharacterOwner->GetActorForwardVector();
				const FVector AxisRight = CharacterOwner->GetActorRightVector();

				float AccelRate = Acceleration.Size2D();
				Acceleration = AxisFwd * (AxisFwd | Acceleration) * ((AxisFwd | Acceleration) > 0.f ? 2.f : 1.f) + AxisRight * (AxisRight | Acceleration);
				Acceleration = Acceleration.GetSafeNormal2D() * AccelRate;
			}
		}
		// Move towards cover
		if (ATacticalCoverMarker* Cover = myChar->GetBestCover())
		{
			if (Cover->GetDistanceSquared2D(myChar->GetActorLocation()) > 1.f)
			{
				const FVector CoverRot = Cover->GetDesiredRotation().Vector();
				const float AccelSize = Acceleration.Size2D();
				if (AccelSize <= KINDA_SMALL_NUMBER)
				{
					Acceleration = CoverRot * this->GetMaxAcceleration();
				}
				else
				{
					Acceleration = (CoverRot*GetMaxAcceleration()*0.5f + Acceleration).GetClampedToMaxSize2D(AccelSize);
				}
			}
		}
	}

	Super::PhysWalking(deltaTime, Iterations);

	Acceleration = SavedAccel;
	AnalogInputModifier = SavedAnalogModifier;
}





class FNetworkPredictionData_Client_Character* UTacticalCharacterMovement::GetPredictionData_Client() const
{
	// Should only be called on client or listen server (for remote clients) in network games
	check(CharacterOwner != NULL);
	checkSlow(CharacterOwner->Role < ROLE_Authority || (CharacterOwner->GetRemoteRole() == ROLE_AutonomousProxy && GetNetMode() == NM_ListenServer));
	checkSlow(GetNetMode() == NM_Client || GetNetMode() == NM_ListenServer);


	// once the NM_Client bug is fixed during map transition, should re-enable this
	//check(GetNetMode() == NM_Client);

	if (!ClientPredictionData)
	{
		UTacticalCharacterMovement* MutableThis = const_cast<UTacticalCharacterMovement*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_TacticalCharacter(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f; // 2X character capsule radius // todo
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UTacticalCharacterMovement::UpdateFromCompressedFlags(uint8 Flags)
{
	if (!CharacterOwner)
	{
		return;
	}

	Super::UpdateFromCompressedFlags(Flags);

	bWantsToSprint = ((Flags & FSavedMove_Character::FLAG_Custom_0) != 0);
	bSprinting = ((Flags & FSavedMove_Character::FLAG_Custom_1) != 0);

	const bool bFlagCrouch = ((Flags & FSavedMove_Character::FLAG_Custom_2) != 0);
	const bool bFlagProne = ((Flags & FSavedMove_Character::FLAG_Custom_3) != 0);
	
	ATacticalCharacter* TacC = Cast<ATacticalCharacter>(CharacterOwner);

	//UE_LOG(LogTemp, Log, TEXT("%s: CompressedFlags: %d"), ((CharacterOwner->GetRemoteRole() == ROLE_Authority) ? TEXT("Server") : TEXT("Client")), Flags);

	if (bFlagProne == bFlagCrouch)
	{
		SetPendingStance(ETacticalStance::STANCE_Default);
	}
	else if(bFlagProne)
	{
		SetPendingStance(ETacticalStance::STANCE_Prone);
	}
	else if (bFlagCrouch)
	{
		SetPendingStance(ETacticalStance::STANCE_Crouched);
	}
}

bool UTacticalCharacterMovement::SetPendingStance(ETacticalStance NewStance)
{
	DesiredStance = NewStance;
	return true;
}

FSavedMovePtr FNetworkPredictionData_Client_TacticalCharacter::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_TacticalCharacter());
}

void FSavedMove_TacticalCharacter::Clear()
{
	Super::Clear();
	
	bCrouch = false;
	bProne = false;
	bSprinting = false;
	bWantsToSprint = false;
}

void FSavedMove_TacticalCharacter::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character & ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);
	UTacticalCharacterMovement* TacCharMove = Cast<UTacticalCharacterMovement>(Character->GetMovementComponent());
	if (TacCharMove)
	{
		bCrouch = (TacCharMove->GetDesiredStance() == ETacticalStance::STANCE_Crouched);
		bProne = (TacCharMove->GetDesiredStance() == ETacticalStance::STANCE_Prone);
		bSprinting = TacCharMove->IsSprinting();
		bWantsToSprint = TacCharMove->bWantsToSprint;
	}

	//UE_LOG(LogTemp, Log, TEXT("%s: Set Flags %d"), (Character->GetRemoteRole() == ROLE_Authority ? TEXT("Server") : TEXT("Client")), GetCompressedFlags());
	// todo maybe round acceleration
}

uint8 FSavedMove_TacticalCharacter::GetCompressedFlags() const
{
	uint8 Result = 0;
	if (bPressedJump)
	{
		Result |= FLAG_JumpPressed;
	}
	if (bWantsToCrouch)
	{
		Result |= FLAG_WantsToCrouch;
	}
	if (bWantsToSprint)
	{
		Result |= FLAG_Custom_0;
	}
	if (bSprinting)
	{
		Result |= FLAG_Custom_1;
	}
	if (bCrouch)
	{
		Result |= FLAG_Custom_2;
	}
	if (bProne)
	{
		Result |= FLAG_Custom_3;
	}

	return Result;
}

bool FSavedMove_TacticalCharacter::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	FSavedMove_TacticalCharacter* NewTacMove = (FSavedMove_TacticalCharacter*)&NewMove;
	if (bWantsToSprint != NewTacMove->bWantsToSprint)
	{
		return false;
	}
	if (bSprinting != NewTacMove->bSprinting)
	{
		return false;
	}
	if (bProne != NewTacMove->bProne)
	{
		return false;
	}
	if (bCrouch != NewTacMove->bCrouch)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

bool FSavedMove_TacticalCharacter::IsImportantMove(const FSavedMovePtr& LastAckedMove) const
{
	// unsafe casting to TacticalCharacter. This should be ok because we are always using this class for Movement Replication
	FSavedMove_TacticalCharacter* LastTacMove = (FSavedMove_TacticalCharacter*)&LastAckedMove;
	if ((bWantsToSprint != LastTacMove->bWantsToSprint) || (bSprinting != LastTacMove->bSprinting)
		|| (bProne != LastTacMove->bProne) || (bCrouch != LastTacMove->bCrouch))
	{
		return true;
	}

	if (!LastAckedMove.IsValid())
	{
		return bPressedJump;
	}

	// Check if any important movement flags have changed status.
	if ((bPressedJump && (bPressedJump != LastAckedMove->bPressedJump)) || (bWantsToCrouch != LastAckedMove->bWantsToCrouch))
	{
		return true;
	}

	if (MovementMode != LastAckedMove->MovementMode)
	{
		return true;
	}

	// check if acceleration has changed significantly
	if (Acceleration != LastAckedMove->Acceleration)
	{
		// Compare magnitude and orientation
		if ((FMath::Abs(AccelMag - LastAckedMove->AccelMag) > AccelMagThreshold) || ((AccelNormal | LastAckedMove->AccelNormal) < AccelDotThreshold))
		{
			return true;
		}
	}
	return false;
}

void FSavedMove_TacticalCharacter::PrepMoveFor(class ACharacter* C)
{
	Super::PrepMoveFor(C);

	ATacticalCharacter* TacC = Cast<ATacticalCharacter>(C);
	UTacticalCharacterMovement* TacMove = TacC->GetTacticalMovement();
	if (TacMove)
	{
		if (bCrouch == bProne)
		{
			// this is invalid and falls back to default stance
			// could also be when both crouch and prone are false which also should result in default stance
			TacMove->SetPendingStance(ETacticalStance::STANCE_Default);
		}
		else if (bCrouch)
		{
			TacMove->SetPendingStance(ETacticalStance::STANCE_Crouched);
		}
		else if (bProne)
		{
			TacMove->SetPendingStance(ETacticalStance::STANCE_Prone);
		}

		TacMove->bWantsToSprint = bWantsToSprint;
		TacMove->bSprinting = bSprinting;
	}
}
