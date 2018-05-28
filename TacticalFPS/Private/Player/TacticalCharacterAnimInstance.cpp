// Copyright (c) 2015-2018, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalGameplaySettings.h"
#include "BRSStatics.h"
#include "TacticalCharacter.h"
#include "TacticalCharacterMovement.h"
#include "TacticalPlayerController.h"
#include "TacticalWeapon.h"
#include "TacticalCharacterAnimInstance.h"


UTacticalCharacterAnimInstance::UTacticalCharacterAnimInstance()
	: Super()
{
	bIsAlive = false;

	RelativeAim = FRotator::ZeroRotator;
	Direction = 0.f;
	Speed = 0.f;
	LeanRotation = FRotator::ZeroRotator;
	bIsFalling = false;
	bIsSprinting = false;
	Accel = 0.f;

	Stance = ETacticalStance::STANCE_Default;

	DefaultRightHandRotation = FRotator(0.f, -90.f, 90.f);
	RightHandData = FTwoBoneJointData(
		FVector(15.f, 13.f, -25.f),
		FRotator(0.f, -90.f, 90.f),
		FVector(-100.f, -100.f, 0.f));

	LeftHandData = FTwoBoneJointData(
		FVector(-34.f, 6.f, -12.f),
		FRotator(0.f, -120.f, 82.f),
		FVector(0.f, 0.f, 0.f));

	RightFootData = FTwoBoneJointData(
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		FVector(-40.f, 0.f, 0.f));
	LeftFootData = FTwoBoneJointData(
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		FVector(40.f, 0.f, 0.f));

	bWeaponObstructed = false;

	GunShakeTranslationFactor = FVector2D(1.f, 1.f);

	VelocityInterpSpeed = 5.f;

	bAdvancedReloadAnim = true;

	ReloadStrength = 0.f;
	FPMesh = nullptr;

	bFirstPerson = false;

	bCoverRight = false;
	CoverType = ECoverType::None;
	CoverAimType = ECoverAimType::None;

	ViewRotationLagStrength = FVector2D(0.007f, 0.007f);
	ViewRotationLagStrength_Rot = FVector2D(0.007f, 0.007f); 
	MovementLagStrength = FVector(-0.005f);
	ViewRotationLagLimit = FVector(5.f);
	ViewRotationLagRotLimit = FRotator(10.f, 10.f, 10.f);


	GunShakeStrength = FVector2D(1.f, 1.f);
	GunShakeStrength_Rot = FVector2D(1.f, 1.f);

	FPHandStrength = 0.f;

	bWeaponLeftHanded = false;
}

void UTacticalCharacterAnimInstance::NativeUpdateAnimation(const float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (TryGetPawnOwner())
	{
		if (ATacticalCharacter* MyChar = Cast<ATacticalCharacter>(TryGetPawnOwner()))
		{
			bIsAlive = MyChar->IsAlive();
			bFirstPerson = MyChar->GetTacticalPlayerController() && MyChar->IsLocallyControlled()
				&& !MyChar->IsThirdPerson() && !MyChar->GetTacticalPlayerController()->IsUsingFreecam()
				&& !MyChar->IsInCover();

			FPMesh = MyChar->GetFirstPersonMesh();

			const FRotator& PawnRot = MyChar->GetActorRotation();
			const FRotator& AimRot = MyChar->GetBaseAimRotation();
			const FTransform& PawnTransform = MyChar->GetTransform();
			const UCharacterMovementComponent* const MoveComp = MyChar->GetTacticalMovement();

			CurrentRotation = MyChar->RotationType;
			RelativeAim = (AimRot - PawnRot).GetNormalized();
			if(bWeaponLeftHanded)
			{
				RelativeAim = FRotator(RelativeAim.Pitch, -RelativeAim.Yaw, RelativeAim.Roll);
			}

			Accel = MoveComp->GetCurrentAcceleration().Size2D();
			FVector PawnVelocity = PawnTransform.InverseTransformVectorNoScale(MyChar->GetVelocity()); 
			PawnVelocity.Z = 0;
			const FVector2D TargetVelocity(PawnVelocity.GetSafeNormal2D());
			VelocityDir = FMath::Vector2DInterpConstantTo(VelocityDir, TargetVelocity, DeltaTime, VelocityInterpSpeed);
			Speed = PawnVelocity.Size2D();
			if (Speed >= 10.f)
			{
				const float TargetDirection = PawnVelocity.Rotation().Yaw;
				Direction = TargetDirection;
			}
			bIsSprinting = MyChar->GetTacticalMovement() ? MyChar->GetTacticalMovement()->IsSprinting() : false;

			LeanRotation = FRotator(MyChar->GetLeanAngle(), 0.f, 0.f);

			bIsFalling = MoveComp ? MoveComp->IsFalling() : false;
			Stance = MyChar->GetStance();
			

			if (MyChar->IsInCover())
			{
				CoverType = MyChar->IsInLowCover() ? ECoverType::LowCover :ECoverType::HighCover;
				if (bCoverRight)
				{
					if (VelocityDir.Y < 0.f) bCoverRight = false;
				}
				else
				{
					if (VelocityDir.Y > 0.f) bCoverRight = true;
				}
			}
			else
			{
				CoverType = ECoverType::None;
			}
			CoverAimType = MyChar->CoverAimType;
			// Process Arms Location
			if (ATacticalWeapon* WPN = MyChar->GetWeapon())
			{
				bWeaponLeftHanded = MyChar->IsWeaponLeftHanded();
				const float timesec = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

				bWeaponObstructed = MyChar->IsWeaponDown() || (MyChar->IsWeaponObstructed() && !MyChar->bMoveWeaponBack);
				WeaponType = WPN->GetWeaponType();
				RightHandData.bIsActive = MyChar->UseWeaponAimIK();


				CalculateGunLag(DeltaTime, ViewRotationLag, ViewRotationLag_Rot);

				FVector RecoilOffset = FVector::ZeroVector;
				FRotator RecoilRot =FRotator::ZeroRotator;
				GetRecoilAnim(RecoilOffset, RecoilRot);

				WeaponMoveBackDist = FMath::FInterpTo(WeaponMoveBackDist, MyChar->WeaponMoveBackDistance, DeltaTime, 10.f);


				const USkeletalMeshSocket* WeaponSocket = MyChar->GetMesh()->GetSocketByName(WPN->EquippedSocketName);
				const FRotator WPNSocketRot = WeaponSocket->RelativeRotation;
				const FVector WPNSocketLoc = WeaponSocket->RelativeLocation;

				const float PitchError	= WPNSocketRot.Pitch;
				const float YawError	= WPNSocketRot.Yaw - 90.f;
				const float RollError	= -WPNSocketRot.Roll;

				const FVector GunShake = GetGunShake();


				const FVector RHTargetLoc = MyChar->GetWeaponHandOffset() + ViewRotationLag
					+ FVector(0.f, GunShakeStrength.X, GunShakeStrength.Y) * GunShake
					+ RecoilOffset - FVector(WeaponMoveBackDist, 0.f, 0.f);

				const FRotator RHTargetRot =  FRotator(
					DefaultRightHandRotation.Pitch	+ RecoilRot.Yaw + YawError + ViewRotationLag_Rot.X + GunShakeStrength_Rot.X * GunShake.Y,				// actually Yaw
					DefaultRightHandRotation.Yaw	- RecoilRot.Pitch + RollError - ViewRotationLag_Rot.Y + GunShakeStrength_Rot.Y * GunShake.Z,	// actually -Pitch
					DefaultRightHandRotation.Roll	+ RecoilRot.Roll + ViewRotationLag_Rot.Z).GetNormalized();			// actually Roll

				const FVector RHJointLoc = GetClass()->GetDefaultObject<UTacticalCharacterAnimInstance>()->RightHandData.JointLocation;

				if (!RHTargetLoc.Equals(RightHandData.TargetLocation))
				{
					RightHandData.TargetLocation = RHTargetLoc;
				}
				if (!RHTargetRot.Equals(RightHandData.TargetRotation))
				{
					RightHandData.TargetRotation = RHTargetRot;
				}
				if (!RHJointLoc.Equals(RightHandData.JointLocation))
				{
					RightHandData.JointLocation = RHJointLoc;
				}

				// process left arm location
				if (WPN->GetMesh())	// Left hand location is determined by left hand socket on weapon. Skip this if we don't have a weapon mesh.
				{
					LeftHandData.bIsActive = true;

					//const USkeletalMeshSocket* WeaponSocket = MyChar->GetMesh()->GetSocketByName(WPN->EquippedSocketName);
					// Transform from Right Hand -> Weapon Attach Point
					const FTransform AttachTransform = WeaponSocket ? FTransform(WPNSocketRot, WPNSocketLoc, WeaponSocket->RelativeScale) : FTransform(); // WeaponSocket -> Right Hand
					FTransform WPNMeshTransform = WPN->GetMesh()->GetRelativeTransform(); //GetDefault<ATacticalWeapon>(WPN->GetClass())->GetMesh()->GetRelativeTransform();
					if (bWeaponLeftHanded)
					{
						WPNMeshTransform.SetLocation(WPNMeshTransform.GetLocation()*FVector(-1.f, 1.f, 1.f));
					}

					FTransform LeftHandSocketTransform = WPN->GetLeftHandTransform();			// Transform from Socket -> Mesh

					LeftHandSocketTransform = LeftHandSocketTransform * WPNMeshTransform;
					//if (bWeaponLeftHanded)
					//{
					//	LeftHandSocketTransform.SetLocation(LeftHandSocketTransform.GetLocation()*FVector(1.f, 1.f, 1.f));
					//}

					const FTransform ComposedTransform = LeftHandSocketTransform * AttachTransform; // Transform LeftHand -> Mesh -> Root (weaponSocket) -> RightHand

					const FVector LHTargetLoc = ComposedTransform.GetLocation();
					const FRotator LHTargetRot = ComposedTransform.GetRotation().Rotator().GetNormalized();
					if (!LHTargetLoc.Equals(LeftHandData.TargetLocation))
					{
						LeftHandData.TargetLocation = LHTargetLoc;
					}
					if (!LHTargetRot.Equals(LeftHandData.TargetRotation))
					{
						LeftHandData.TargetRotation = LHTargetRot;
					}
				}
				else
				{
					LeftHandData.bIsActive = false;
				}
				// left hand joint to default location. todo: maybe implement a smarter system.
				LeftHandData.JointLocation = GetClass()->GetDefaultObject<UTacticalCharacterAnimInstance>()->LeftHandData.JointLocation;

				if (FPMesh)
				{
					FPHandStrength = FMath::Clamp(FPMesh->GetAnimInstance()->GetCurveValue(FName("FPHands")), 0.f, 1.f);
				}

				ReloadStrength = 0.f;
				if (bAdvancedReloadAnim && FPMesh && WPN->IsReloading())
				{
					ReloadStrength = FPMesh->GetAnimInstance()->GetCurveValue("Reload");
					FPHandStrength = 1.f;

					// bunch of transformation to transform bone locations from FPP Skeleton to TPP Skeleton's skeletal control space 
					// (right hand = head space, left hand = right hand space)
					const FTransform FPHeadTransform = FPMesh->GetSocketTransform(FName(TEXT("head")), RTS_Component);			// head -> comp
					const FTransform FPRightHandTransform = FPMesh->GetSocketTransform(FName(TEXT("hand_r")), RTS_Component);	// right hand -> comp
					const FTransform FPLeftHandTransform = FPMesh->GetSocketTransform(FName(TEXT("hand_l")), RTS_Component);	// left hand -> comp
	

					const FTransform FP_RHFinal = FPRightHandTransform * FPHeadTransform.Inverse();	// right hand -> comp -> head
					const FTransform FP_LHFinal = FPLeftHandTransform * FPRightHandTransform.Inverse();// *Correction; // left hand  -> comp -> right hand -> adjusted right hand
					const FVector TempHandLoc = FP_RHFinal.GetTranslation();

					// extract location and rotation from resulting transforms and lerp.
					RightHandData.TargetLocation = FMath::Lerp(RightHandData.TargetLocation, FVector(TempHandLoc.Y, TempHandLoc.Z, TempHandLoc.X), ReloadStrength);
					RightHandData.TargetRotation = UBRSStatics::LerpRotators(
						RightHandData.TargetRotation,
						FP_RHFinal.GetRotation().Rotator(),
						ReloadStrength);

					LeftHandData.TargetLocation = FMath::Lerp(LeftHandData.TargetLocation, FP_LHFinal.GetTranslation(), ReloadStrength);				
					LeftHandData.TargetRotation = UBRSStatics::LerpRotators(
						LeftHandData.TargetRotation,
						FP_LHFinal.GetRotation().Rotator(),
						ReloadStrength);
				}
			}
			else
			{
				// turn skeletal controls off, when carrying no weapon
				WeaponType = EWeaponType::None;
				RightHandData.bIsActive = false;
				LeftHandData.bIsActive = false;
				bWeaponLeftHanded = false;
			}

			// Process Foot Location
			if (MyChar->GetMesh())
			{
				RightFootData.bIsActive = true;
				LeftFootData.bIsActive = true;

				FVector RightFootLoc;
				FVector LeftFootLoc;
				FVector RightFootNormal;
				FVector LeftFootNormal;
				MyChar->GetDesiredFootLocations(RightFootLoc, LeftFootLoc, RightFootNormal, LeftFootNormal);

				const FTransform RFootTransform = MyChar->GetMesh()->GetSocketTransform(FName(TEXT("foot_r")), RTS_World);
				RightFootData.TargetLocation = RFootTransform.InverseTransformVector(RightFootLoc);
				RightFootData.TargetRotation = CalculateFootRotfromNormal(RightFootNormal);

				const FTransform LFootTransform = MyChar->GetMesh()->GetSocketTransform(FName(TEXT("foot_l")), RTS_World);
				LeftFootData.TargetLocation = LFootTransform.InverseTransformVector(LeftFootLoc);
				LeftFootData.TargetRotation = CalculateFootRotfromNormal(LeftFootNormal);
			}
		}
	}
}

FVector UTacticalCharacterAnimInstance::GetGunShake_Implementation()
{
	bool bAiming = false;
	if (TryGetPawnOwner())
	{
		if (ATacticalCharacter* MyChar = Cast<ATacticalCharacter>(TryGetPawnOwner()))
		{
			bAiming = MyChar->IsAiming();
		}
	}
	const float ShakeRight = GetCurveValue(FName(TEXT("GunShakeRight")));
	const float ShakeUp = GetCurveValue(FName(TEXT("GunShakeUp")));

	const UTacticalGameplaySettings* TacticalSettings = UTacticalGameplaySettings::StaticClass()->GetDefaultObject<UTacticalGameplaySettings>();
	const float ShakeStrength = TacticalSettings->WeaponBobMultiplier;

	return (FVector(0.f, ShakeRight * GunShakeTranslationFactor.X, ShakeUp * GunShakeTranslationFactor.Y) * (bAiming ? 0.3f : 1.f) * ShakeStrength);
}

void UTacticalCharacterAnimInstance::GetRecoilAnim(FVector& outOffset, FRotator& outRot)
{
	if (TryGetPawnOwner())
	{
		if (ATacticalCharacter* MyChar = Cast<ATacticalCharacter>(TryGetPawnOwner()))
		{
			if (MyChar->GetWeapon())
			{
				MyChar->GetWeapon()->GetCurrentAnimRecoil(outRot, outOffset);
			}
		}
	}
}

void UTacticalCharacterAnimInstance::CalculateGunLag_Implementation(const float DeltaTime, FVector& Translation, FVector& Rotation)
{
	if (TryGetPawnOwner())
	{
		if (ATacticalCharacter* MyChar = Cast<ATacticalCharacter>(TryGetPawnOwner()))
		{
			const FTransform& PawnTransform = MyChar->GetTransform();
			FVector PawnVelocity = PawnTransform.InverseTransformVector(MyChar->GetVelocity());

			const FRotator& ViewRotSpeed = MyChar->GetViewRotationSpeed();

			const FVector TargetOffset = FVector(
				0.f, 
				FMath::Clamp(ViewRotSpeed.Yaw * ViewRotationLagStrength.X, -ViewRotationLagLimit.Y, ViewRotationLagLimit.Y),
				FMath::Clamp(ViewRotSpeed.Pitch * ViewRotationLagStrength.Y, -ViewRotationLagLimit.Z, ViewRotationLagLimit.Z))
				+ (PawnVelocity * MovementLagStrength);
			const FVector OutOffset = FMath::VInterpTo(ViewRotationLag, TargetOffset, DeltaTime, 6.f);

			const FVector TargetOffset_Rot = FVector(
				FMath::Clamp(ViewRotSpeed.Yaw * ViewRotationLagStrength_Rot.X, -ViewRotationLagRotLimit.Yaw, ViewRotationLagRotLimit.Yaw),
				FMath::Clamp(ViewRotSpeed.Pitch * ViewRotationLagStrength_Rot.Y, -ViewRotationLagRotLimit.Pitch, ViewRotationLagRotLimit.Pitch),
				FMath::Clamp(PawnVelocity.Y * 0.01f, -ViewRotationLagRotLimit.Roll, ViewRotationLagRotLimit.Roll));
			const FVector OutOffset_Rot = FMath::VInterpTo(ViewRotationLag_Rot, TargetOffset_Rot, DeltaTime, 10.f);

			const UTacticalGameplaySettings* TacticalSettings = UTacticalGameplaySettings::StaticClass()->GetDefaultObject<UTacticalGameplaySettings>();
			const float LagStrength = TacticalSettings->WeaponLagMultiplier;

			Translation = (MyChar->IsAiming() ? 0.5f : 1.f) * OutOffset * LagStrength;
			Rotation = (MyChar->IsAiming() ? 0.5f : 1.f) * OutOffset_Rot * LagStrength;
		}
	}
}

FRotator UTacticalCharacterAnimInstance::CalculateFootRotfromNormal(const FVector& Normal) const
{
	return FRotator(-FMath::RadiansToDegrees(FMath::Atan2(Normal.X, Normal.Z)), 0.f, FMath::RadiansToDegrees(FMath::Atan2(Normal.Y, Normal.Z)));
}
eKTdcJn0
bool UTacticalCharacterAnimInstance::IsMoving() const
{
	return (Speed > 10.f);
}

void FTacticalAnimInstanceProxy::Update(float DeltaSeconds)
{
	FAnimInstanceProxy::Update(DeltaSeconds);
}
