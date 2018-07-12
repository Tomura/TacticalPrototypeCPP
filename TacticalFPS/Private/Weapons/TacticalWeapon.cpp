// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "UnrealNetwork.h"

#include "Animation/AnimMontage.h"

#include "TacticalWeapon.h"

#include "TacticalCharacter.h"
#include "TacticalCharacterMovement.h"
#include "TacticalInventoryComponent.h"

#include "TacticalWeaponState.h"
#include "TacticalWeaponState_WPN.h"
#include "TacticalWeaponStateActive.h"
#include "TacticalWeaponStateInactive.h"
#include "TacticalWeaponStateEquipping.h"
#include "TacticalWeaponStateUnequipping.h"
#include "TacticalWeaponStateFiring.h"
#include "TacticalWeaponStateReloading.h"
#include "TacticalWeaponStateObstructed.h"
#include "TacticalWeaponStateGadget.h"
#include "TacticalWeaponStateGadgetReload.h"
#include "TacticalWeaponStateGadgetFiring.h"

#include "TacticalAmmoType.h"
#include "TacticalWeaponAttachment.h"
#include "TacticalWeaponAttachment_Weapon.h"
#include "TacticalWeaponAttachmentPoint.h"

#include "TacticalImpactFXComponent.h"
#include "TacticalImpactFX.h"

#include "TacticalArmsAnimInstance.h"
#include "TacticalWeaponAnimInstance.h"

#include "TacticalRecoilCurve.h"
#include "Perception/AISense_Hearing.h"

#define STATE_Inactive	 0
#define STATE_Active	 1
#define STATE_Firing	 2
#define STATE_Equip		 3
#define STATE_Unequip	 4
#define STATE_Reload	 5
#define STATE_Obstructed 6
#define STATE_GadgetFiring 7
#define STATE_GadgetReloading 8


// Sets default values
ATacticalWeapon::ATacticalWeapon(const FObjectInitializer& OI)
	: Super(OI)
{
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UTexture2D> WidgetPointSprite;
		FConstructorStatics()
			: WidgetPointSprite(TEXT("/Engine/EditorMaterials/TargetIcon"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	StateActive = OI.CreateDefaultSubobject<UTacticalWeaponStateActive>(this, TEXT("StateActive"));
	StateEquipping = OI.CreateDefaultSubobject<UTacticalWeaponStateEquipping>(this, TEXT("StateEquipping"));
	StateUnequipping = OI.CreateDefaultSubobject<UTacticalWeaponStateUnequipping>(this, TEXT("StateUnequipping"));
	StateFiring = OI.CreateDefaultSubobject<UTacticalWeaponStateFiring>(this, TEXT("StateFiring"));
	StateInactive = OI.CreateDefaultSubobject<UTacticalWeaponStateInactive>(this, TEXT("StateInactive"));
	StateReloading = OI.CreateDefaultSubobject<UTacticalWeaponStateReloading>(this, TEXT("StateReloading"));
	StateObstructed = OI.CreateDefaultSubobject<UTacticalWeaponStateObstructed>(this, TEXT("StateObstructed"));
	CurrentState = nullptr;

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create Dummy Root to enable Tranformation of Mesh
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("DummyRoot"));
	RootComponent = Root;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->LightingChannels.bChannel0 = false;
	Mesh->LightingChannels.bChannel1 = true;
	Mesh->LightingChannels.bChannel2 = false;

	BulletEjectPSC = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BulletEject"));
	BulletEjectPSC->SetupAttachment(Mesh);
	BulletEjectPSC->bAutoActivate = false;

	WidgetPoint = CreateDefaultSubobject<UBillboardComponent>(TEXT("WidgetLocation"));
	WidgetPoint->SetupAttachment(Mesh);
	WidgetPoint->SetSprite(ConstructorStatics.WidgetPointSprite.Get());
	WidgetPoint->SetRelativeScale3D(FVector(0.1f));


	BaseAimPoint = CreateDefaultSubobject<USceneComponent>(TEXT("BaseAimPoint"));
	BaseAimPoint->SetupAttachment(Mesh);
	SightAimPoint = CreateDefaultSubobject<USceneComponent>(TEXT("SightAimPoint"));
	SightAimPoint->SetupAttachment(BaseAimPoint);


	EquipTime = 0.7f;
	UnequipTime = 0.7f;

	bShowCrosshairWhenAiming = false;

	// Stats
	Spread = FFloatInterval(0.f, 10.f);		// Spread
	SpreadRecovery = 1.f;					// Spread Recovery
	SpreadIncrement = 1.f;
	MovementSpreadModifier = 0.05f;
	RotationSpreadModifier = 0.035f;
	MaxMovementSpread = 60.f;
	AimSpreadModifer = 0.5;

	// Recoil
	BaseRecoil = FVector2D::ZeroVector;
	bRandomizeYRecoil = true;
	bRandomizeYawRecoil = true;
	CrouchRecoilModifier = 0.8f;
	ProneRecoilModifier = 0.35f;
	AimSpreadModifer = 0.7f;

	PenetrationModifier = 1.f;		// Penetration through Surfaces
	BaseRange = 50000.f;		// Range
	BaseFireInterval = 0.1f;
	TriggerDelay = 0.f;

	NumMagazines = 6;
	MagazineCapacity = 30;
	bCanBulletStayInChamber = true;

	bCanFMAuto = true;
	bCanFMBurst = true;
	bCanFMSingle = true;

	DamageModifier = 1.f;
	PenetrationModifier = 1.f;
	BurstCount = 3;

	ReloadTime = 2.f;

	DefaultAmmoType = 0;
	AmmoType = nullptr;
	BulletEjectDelay = 0.1f;

	bPendingUnequip = false;
	bPendingReload = false;

	FiringMode = EWeaponFiringMode::FM_None;

	CurrentMagazine = 0;
	Magazine.Empty();


	EquippedSocketName = FName(TEXT("socket_weapon"));
	EquippedSocketNameLeft = FName(TEXT("socket_weapon_l"));
	AttachSocketName = FName(TEXT("rifle_holster"));
	GunIKSocketName = FName(TEXT("ik_hand_gun"));

	WeaponOffset = FVector::ZeroVector;
	WeaponAimOffset = FVector::ZeroVector;

	WeaponType = EWeaponType::Rifle;

	bIsLightOn = false;
	ObstructionTraceDistance = 40.f;
	MaxMoveBackDistance = 0.f;

	AimZoomModifier = 2.f;

	RecoilCameraAnim = nullptr;
	AimRecoilCameraAnimScale = 0.5f;

	AddSpread_Standing = 0.7f;
	AddSpread_Crouched = 0.3f;
	AddSpread_Prone = 0.0f;

	AIBurstInterval = FFloatInterval(0.1f, 0.5f);
	AIBurstCount = FInt32Interval(3, 5);

	FPHandsIdleAnim = nullptr;

	FiringNoiseLoudness = 1.f;
	FiringNoiseMaxRange = 2000.f;

	ADSSpeed = 10.f;

	FPHandsIdleAnimLeft = nullptr;
	ReloadAnimOffsetLeft = FVector(0.f, 0.f, 0.f);
	ReloadAnimOffsetRight = FVector(0.f, 0.f, 0.f);

	LeftHandSocketName = FName("LeftHand");
}

void ATacticalWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (UTacticalWeaponStateFiring* StateFireC = Cast<UTacticalWeaponStateFiring>(StateFiring))
	{
		StateFireC->GetSimulateFireEvent().AddDynamic(this, &ATacticalWeapon::NativeLocalSimulateFire);
	}
}

void ATacticalWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATacticalWeapon, CurrentMagazine);
	DOREPLIFETIME(ATacticalWeapon, Magazine);
	DOREPLIFETIME(ATacticalWeapon, FiringMode);
	DOREPLIFETIME_CONDITION(ATacticalWeapon, RepWeaponState, COND_SkipOwner);
}

// Called when the game starts or when spawned
void ATacticalWeapon::BeginPlay()
{
	Super::BeginPlay();
	if (Role == ROLE_Authority)
	{
		Magazine.Empty();
		// set up magazines
		if (ReloadRoundsIndividually())
		{
			Magazine.SetNumZeroed(2);
			Magazine[0] = GetMagazineCapacity();
			Magazine[1] = NumMagazines;
			CurrentMagazine = 0;
		}
		else
		{
			for (int32 i = 0; i < NumMagazines; i++)
			{
				Magazine.Add(GetMagazineCapacity());
			}
		}
	}

	FiringMode = GetDefaultFiringMode();
	if (AllowedAmmoTypes.IsValidIndex(DefaultAmmoType))
	{
		AmmoType = AllowedAmmoTypes[DefaultAmmoType];
	}

	if (AmmoType && BulletEjectPSC)
	{
		BulletEjectPSC->SetTemplate(GetDefault<UTacticalAmmoType>(AmmoType)->BulletEjectParticle);
	}
	if (BulletEjectPSC)
	{
		BulletEjectPSC->OnParticleCollide.AddDynamic(this, &ATacticalWeapon::OnBulletDropCollision);
	}


	if (!CurrentState)
	{
		GotoState(StateInactive);
	}
}

// Called every frame
void ATacticalWeapon::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	// Play layered procedural Recoil Animations
	if (PendingAnimRecoils.Num() > 0)
	{
		for (int32 i = 0; i < PendingAnimRecoils.Num(); i++)
		{
			if (PendingAnimRecoils[i].bPendingKill)
			{
				PendingAnimRecoils.RemoveAt(i);
				i--;
			}
			else
			{
				PendingAnimRecoils[i].Tick(DeltaTime);
			}
		}
	}

	// Tick Spread
	TickSpread(DeltaTime);
}

void ATacticalWeapon::Destroyed()
{
	GotoState(StateInactive);

	if (GetInventoryOwner() && GetInventoryOwner()->GetInventory())
	{
		GetInventoryOwner()->GetInventory()->OnWeaponDestroyed(this);
	}

	Super::Destroyed();
}

void ATacticalWeapon::ItemRemovedFromOwner(class ATacticalCharacter* OldOwner)
{
	GotoState(StateInactive);
	if ( GetInventoryComponent() && GetInventoryComponent()->GetWeapon() == this)
	{
		GetInventoryComponent()->Weapon = nullptr;
		GetInventoryComponent()->WeaponChanged();
	}

	TArray<UActorComponent*> Comps = GetComponentsByClass(UTacticalWeaponAttachmentPoint::StaticClass());
	for (UActorComponent* TestComp : Comps)
	{
		UTacticalWeaponAttachmentPoint* AttachPoint = TestComp ? Cast<UTacticalWeaponAttachmentPoint>(TestComp) : nullptr;
		if (AttachPoint && AttachPoint->GetCurrentAttachment())
		{
			AttachPoint->GetCurrentAttachment()->OnWeaponRemovedFromOwner();
		}
	}

	OnStopAiming();
}


void ATacticalWeapon::OnOwnerDied()
{
	TArray<UActorComponent*> Comps = GetComponentsByClass(UTacticalWeaponAttachmentPoint::StaticClass());
	for (UActorComponent* TestComp : Comps)
	{
		UTacticalWeaponAttachmentPoint* AttachPoint = TestComp ? Cast<UTacticalWeaponAttachmentPoint>(TestComp) : nullptr;
		if (AttachPoint && AttachPoint->GetCurrentAttachment())
		{
			AttachPoint->GetCurrentAttachment()->OnCharacterOwnerDied();
		}
	}
}

void ATacticalWeapon::OnFirstPerson()
{
	BP_OnFirstPerson();
	TArray<UActorComponent*> Comps = GetComponentsByClass(UTacticalWeaponAttachmentPoint::StaticClass());
	for (UActorComponent* TestComp : Comps)
	{
		UTacticalWeaponAttachmentPoint* AttachPoint = TestComp ? Cast<UTacticalWeaponAttachmentPoint>(TestComp) : nullptr;
		if (AttachPoint && AttachPoint->GetCurrentAttachment())
		{
			AttachPoint->GetCurrentAttachment()->OnFirstPerson();
		}
	}

	// force highest LOD for mesh
	TArray<UActorComponent*> Meshes = GetComponentsByClass(UMeshComponent::StaticClass());
	for (UActorComponent* TestComp : Meshes)
	{
		if (UStaticMeshComponent* TestMesh = Cast<UStaticMeshComponent>(TestComp))
		{
			TestMesh->ForcedLodModel = 1;
			continue;
		}

		if (USkeletalMeshComponent* TestMesh = Cast<USkeletalMeshComponent>(TestComp))
		{
			TestMesh->ForcedLodModel = 1;
			continue;
		}
	}
}

void ATacticalWeapon::OnThirdPerson()
{
	BP_OnThirdPerson();
	TArray<UActorComponent*> Comps = GetComponentsByClass(UTacticalWeaponAttachmentPoint::StaticClass());
	for (UActorComponent* TestComp : Comps)
	{
		UTacticalWeaponAttachmentPoint* AttachPoint = TestComp ? Cast<UTacticalWeaponAttachmentPoint>(TestComp) : nullptr;
		if (AttachPoint && AttachPoint->GetCurrentAttachment())
		{
			AttachPoint->GetCurrentAttachment()->OnThirdPerson();
		}
	}

	// un-force LOD for mesh
	TArray<UActorComponent*> Meshes = GetComponentsByClass(UMeshComponent::StaticClass());
	for (UActorComponent* TestComp : Meshes)
	{
		if (UStaticMeshComponent* TestMesh = Cast<UStaticMeshComponent>(TestComp))
		{
			TestMesh->ForcedLodModel = 0;
			continue;
		}

		if (USkeletalMeshComponent* TestMesh = Cast<USkeletalMeshComponent>(TestComp))
		{
			TestMesh->ForcedLodModel = 0;
			continue;
		}
	}
}

void ATacticalWeapon::OnStartAiming()
{
	BP_OnStartAiming();

	TArray<UActorComponent*> Comps = GetComponentsByClass(UTacticalWeaponAttachmentPoint::StaticClass());
	for (UActorComponent* TestComp : Comps)
	{
		UTacticalWeaponAttachmentPoint* AttachPoint = TestComp ? Cast<UTacticalWeaponAttachmentPoint>(TestComp) : nullptr;
		if (AttachPoint && AttachPoint->GetCurrentAttachment())
		{
			AttachPoint->GetCurrentAttachment()->OnStartAiming();
		}
	}
}

void ATacticalWeapon::OnStopAiming()
{
	BP_OnStopAiming();

	TArray<UActorComponent*> Comps = GetComponentsByClass(UTacticalWeaponAttachmentPoint::StaticClass());
	for (UActorComponent* TestComp : Comps)
	{
		UTacticalWeaponAttachmentPoint* AttachPoint = TestComp ? Cast<UTacticalWeaponAttachmentPoint>(TestComp) : nullptr;
		if (AttachPoint && AttachPoint->GetCurrentAttachment())
		{
			AttachPoint->GetCurrentAttachment()->OnStopAiming();
		}
	}
}

UParticleSystemComponent* ATacticalWeapon::GetMuzzleFlashParticle() const
{
	ATacticalWeaponAttachment_Muzzle* MuzzleAttachment = GetMuzzleAttachment();
	if (MuzzleAttachment)
	{
		if(UParticleSystemComponent* MuzzlePSC = MuzzleAttachment->GetMuzzleFlashPSC())
		{
			return MuzzlePSC;
		}

	}

	return BP_GetMuzzleFlashParticle();

}


void ATacticalWeapon::GetCurrentAnimRecoil(FRotator& outRot, FVector& outOffset) const
{
	if (PendingAnimRecoils.Num() > 0)
	{
		for (const FPendingAnimRecoil& TestRecoil : PendingAnimRecoils)
		{
			FVector TempLoc;
			FRotator TempRot;

			if (TestRecoil.Anim == nullptr)
				continue;

			float CurveTime = TestRecoil.Time; // multiply with anim speed
			TestRecoil.Anim->GetVectorAndRotationValue(CurveTime, TempLoc, TempRot);
			TempRot.Yaw *= TestRecoil.YawFactor;
			float tempYaw = TempRot.Yaw;
			TempLoc.Y *= TestRecoil.YFactor;
			float tempY = TempLoc.Y;
			TempRot *= TestRecoil.Scale;
			TempLoc *= TestRecoil.Scale;

			TempRot.Yaw = tempYaw;
			TempLoc.Y = tempY;

			outOffset += TempLoc;
			outRot += TempRot;
		}
	}
}


FTransform ATacticalWeapon::GetLeftHandTransform() const
{
	FTransform AttachmentLeftHandTransform; // LeftHand Socket 2 Attachment root

	TArray<UTacticalWeaponAttachmentPoint*> Attachments;
	GetAttachmentPoints(Attachments);
	for (UTacticalWeaponAttachmentPoint* TestAttachPoint : Attachments)
	{
		if (TestAttachPoint && TestAttachPoint->GetCurrentAttachment())
		{
			if (ATacticalWeaponAttachment_Special* Gadget = Cast<ATacticalWeaponAttachment_Special>(TestAttachPoint->GetCurrentAttachment()))
			{
				if (Gadget->GetLeftHandTransform(AttachmentLeftHandTransform))
				{
					const FTransform AttachmentTransform = Gadget->GetTransform(); // Attachment root to World
					const FTransform WeaponTransform = GetMesh()->GetComponentTransform();	// Weapon root to world
					const FTransform CompTransform = AttachmentLeftHandTransform*AttachmentTransform*WeaponTransform.Inverse();
					return CompTransform;
				}
			}
		}
	}
	return GetOwnLeftHandTransform();
}

FTransform ATacticalWeapon::GetOwnLeftHandTransform_Implementation() const
{
	const FTransform LeftHandSocketTransform = GetMesh()->GetSocketTransform(LeftHandSocketName, RTS_Component); // Transform from Weapon Root -> Socket
	return LeftHandSocketTransform;
}

void ATacticalWeapon::SwitchLight(const bool bTurnOn)
{
	if (bTurnOn && (CurrentState != StateInactive || CurrentState != StateEquipping || CurrentState != StateUnequipping))
	{
		bIsLightOn = bTurnOn;
		BPSwitchLight(bIsLightOn);
	}
	else
	{
		bIsLightOn = false;
		BPSwitchLight(bIsLightOn);
	}
}

void ATacticalWeapon::NativeToggleAttachment(int32 AttachmentIdx /*= 0*/)
{
	if (Role != ROLE_Authority)
	{
		ServerToggleAttachment(AttachmentIdx);
	}


	if(!CurrentState->CanUseGadget())
		return;

	ToggleAttachment(AttachmentIdx);
}

void ATacticalWeapon::ToggleAttachment_Implementation(int32 AttachmentIdx /*= 0*/)
{



	TArray<UTacticalWeaponAttachmentPoint*> GadgetPoints;
	{
		TArray<UTacticalWeaponAttachmentPoint*> Attachments;
		GetAttachmentPoints(Attachments);
		for (UTacticalWeaponAttachmentPoint* TestAttachPoint : Attachments)
		{
			if (TestAttachPoint && TestAttachPoint->GetCurrentAttachment())
			{
				if (Cast<ATacticalWeaponAttachment_Special>(TestAttachPoint->GetCurrentAttachment()))
				{
					GadgetPoints.Add(TestAttachPoint);
				}
			}
		}
	}

	// return if we haven't found a suitable gadget
	if (GadgetPoints.Num() <= 0)
		return;

	GadgetPoints.Sort([&](const UTacticalWeaponAttachmentPoint& One, const UTacticalWeaponAttachmentPoint& Two)
	{
		return One.GetPriority() >= Two.GetPriority();
	});

	
	if (!GadgetPoints.IsValidIndex(AttachmentIdx))
	{
		if (AttachmentIdx < 0)
		{
			AttachmentIdx = 0;
		}
		else
		{
			AttachmentIdx = GadgetPoints.Num() - 1;
		}
	}

	GadgetPoints[AttachmentIdx]->ToggleGadget();
}

void ATacticalWeapon::ServerToggleAttachment_Implementation(int32 AttachmenIdx /*= 0*/)
{
	NativeToggleAttachment(AttachmenIdx);
}

void ATacticalWeapon::ToggleSight()
{
	TArray<UActorComponent*> Comps = GetComponentsByClass(UTacticalWeaponAttachmentPoint_Sight::StaticClass());
	for (UActorComponent* TestComp : Comps)
	{
		UTacticalWeaponAttachmentPoint_Sight* SightAttachmentPoint = TestComp ? Cast<UTacticalWeaponAttachmentPoint_Sight>(TestComp) : nullptr;
		if (SightAttachmentPoint)
		{
			SightAttachmentPoint->ToggleGadget();
		}
	}
}

bool ATacticalWeapon::ShowCrosshair() const
{
	if (GetInventoryOwner())
	{
		if (GetInventoryOwner()->IsAiming())
		{
			return bShowCrosshairWhenAiming;
		}
	}
	return true;
}

float ATacticalWeapon::GetAimZoomModifier() const
{
	ATacticalWeaponAttachment_Sight* Sight = GetSightAttachment();
	if (Sight)
	{
		return Sight->GetZoomModifier();
	}
	return AimZoomModifier;
}


void ATacticalWeapon::GotoState(UTacticalWeaponState* NextState)
{
	if (NextState == NULL || 
		!(NextState->IsIn(this) || NextState->IsA(UTacticalWeaponStateGadget::StaticClass()) && Cast<UTacticalWeaponStateGadget>(NextState)->GetAttachment()->GetOwner() == this))
	{

	}
	else if (GetInventoryOwner())
	{
		if (CurrentState != NextState)
		{
			UTacticalWeaponState* PrevState = CurrentState;
			// End Current State
			if (CurrentState)
			{
				CurrentState->EndState(NextState);
			}
			// Change Current State to New State if nothing has changed
			if (CurrentState == PrevState)
			{
				CurrentState = NextState;
				CurrentState->BeginState(PrevState);
				// Call State Changed Event
				StateChanged();
			}
		}
	}
}

void ATacticalWeapon::StateChanged(){}


void ATacticalWeapon::Activate(bool bSkipEquip /*= false*/)
{
	const float SavedEquipTime = EquipTime;
	if (bSkipEquip)
	{
		EquipTime = 0.f;
	}

	if (CurrentState)
	{
		CurrentState->Equip();
	}
	else
	{
		StateInactive->Equip();
	}

	EquipTime = SavedEquipTime;

	if (GetInventoryOwner()->IsThirdPerson())
	{
		OnThirdPerson();
	}
	else
	{
		OnFirstPerson();
	}
}

bool ATacticalWeapon::IsChangingWeapon() const
{
	return CurrentState->IsChangingWeapon();
}

bool ATacticalWeapon::TryUnequip()
{
	if (CurrentState)
		CurrentState->Unequip();

	if (CurrentState == StateUnequipping) // check if state was switched properly
	{
		OnThirdPerson();
		// Turn all Gadgets Off
		TArray<UTacticalWeaponAttachmentPoint*> AttachPoints;
		GetComponents<UTacticalWeaponAttachmentPoint>(AttachPoints);
		for (UTacticalWeaponAttachmentPoint* TestAttach : AttachPoints)
		{
			if (TestAttach)
			{
				// todo: maybe add an property that tells us if we even need to do this
				TestAttach->TurnGadgetOnOff(false);
			}
		}

		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////
// Equipping / Unequipping

void ATacticalWeapon::LocalSimulateEquip()
{
	if (GetInventoryOwner())
	{
		//GetInventoryOwner()->OnWeaponEquip(EquipTime); //todo
		if (GetInventoryComponent()->OnWeaponBeginEquip.IsBound())
		{
			GetInventoryComponent()->OnWeaponBeginEquip.Broadcast(this, EquipTime);
		}

		USkeletalMeshComponent* FPMesh = GetInventoryOwner()->GetFirstPersonMesh();
		if (FPMesh && FPMesh->GetAnimInstance())
		{
			UTacticalArmsAnimInstance* ArmsAnimInstance = Cast<UTacticalArmsAnimInstance>(FPMesh->GetAnimInstance());
			if (ArmsAnimInstance)
			{
				ArmsAnimInstance->IdleAnimation = FPHandsIdleAnim;
				if (FPHandsIdleAnimLeft != nullptr)
				{
					ArmsAnimInstance->IdleAnimation_Left = FPHandsIdleAnimLeft;
				}
				else
				{
					ArmsAnimInstance->IdleAnimation_Left = FPHandsIdleAnim;
				}
				ArmsAnimInstance->ReloadOffsetLeft = ReloadAnimOffsetLeft;
				ArmsAnimInstance->ReloadOffsetRight = ReloadAnimOffsetRight;
			}

			TArray<UTacticalWeaponAttachmentPoint*> AttachPoints;
			GetAttachmentPoints(AttachPoints);
			for (UTacticalWeaponAttachmentPoint* TestPoint : AttachPoints)
			{
				if (TestPoint && TestPoint->GetClass()->IsChildOf(UTacticalWeaponAttachmentPoint_Special::StaticClass()))
				{
					if (TestPoint->GetCurrentAttachment() != nullptr)
					{
						if (ATacticalWeaponAttachment_Special* TestAttachment = Cast<ATacticalWeaponAttachment_Special>(TestPoint->GetCurrentAttachment()))
						{
							if (UAnimSequence* const FPLeftAnim = TestAttachment->GetFPHandAnimation_Left())
							{
								ArmsAnimInstance->IdleAnimation_Left = FPLeftAnim;
								if (UAnimSequence* const FPRightAnim = TestAttachment->GetFPHandAnimation_Right())
								{
									ArmsAnimInstance->IdleAnimation = FPRightAnim;
								}
								break;
							}
						}
					}
				}
			}
		}
	}
	//UE_LOG(LogTemp, Log, TEXT("Simulating Equip"));
	//AttachRootComponentTo(GetInventoryOwner()->GetMesh(), GunIKSocketName, EAttachLocation::SnapToTarget);
}

void ATacticalWeapon::NetMulti_EquipWeapon_Implementation()
{
	if (Role == ROLE_Authority || !IsOwnerLocallyControlled())
	{
		LocalSimulateEquip();
	}
}

void ATacticalWeapon::WeaponUnequipped()
{
	if (GetInventoryOwner())
	{
		GetInventoryOwner()->WeaponUnequipped();
	}

	UE_LOG(LogTemp, Log, TEXT("Simulating Unequip"));
	//AttachRootComponentTo(GetInventoryOwner()->GetMesh(), AttachSocketName, EAttachLocation::SnapToTarget);

}

void ATacticalWeapon::LocalSimulateUnequip()
{
	if (GetInventoryOwner())
	{
		//GetInventoryOwner()->OnWeaponUnequip(UnequipTime); //todo
		if (GetInventoryComponent()->OnWeaponBeginUnequip.IsBound())
		{
			GetInventoryComponent()->OnWeaponBeginUnequip.Broadcast(this, UnequipTime);
		}
	}
}

void ATacticalWeapon::NetMulti_UnequipWeapon_Implementation()
{
	if (Role == ROLE_Authority || (!GetInventoryOwner() || !GetInventoryOwner()->IsLocallyControlled()))
	{
		LocalSimulateUnequip();
	}
	//AttachRootComponentTo(GetInventoryOwner()->GetMesh(), GunIKSocketName, EAttachLocation::SnapToTarget);
}

///////////////////////////////////////////////////////////
// Ammo

bool ATacticalWeapon::HasAmmoLoaded() const
{
	if (ReloadRoundsIndividually())
	{
		if (Magazine.IsValidIndex(0))
		{
			return Magazine[0] > 0;
		}
	}
	else
	{
		if (Magazine.IsValidIndex(CurrentMagazine))
		{
			return Magazine[CurrentMagazine] > 0;
		}
	}
	return false;
}

bool ATacticalWeapon::HasAnyAmmo() const
{
	if (ReloadRoundsIndividually())
	{
		if (Magazine.IsValidIndex(0) && Magazine[0] > 0)
		{
			return true;
		}
		if (Magazine.IsValidIndex(1) && Magazine[1] > 0)
		{
			return true;
		}
	}
	else
	{
		for (int32 i = 0; i < Magazine.Num(); i++)
		{
			if (Magazine.IsValidIndex(i))
			{
				int32 AmmoInMag = Magazine[i];
				if (AmmoInMag > 0)
				{
					return true;
				}
			}
		}
	}
	return false;
}

int32 ATacticalWeapon::GetNumMagazines() const
{
	if (ReloadRoundsIndividually())
	{
		if (Magazine.IsValidIndex(1))
		{
			return Magazine[1];
		}
		return 0;
	}
	
	int32 Mags = 0;
	for (int32 i = 0; i < Magazine.Num(); i++)
	{
		if (Magazine.IsValidIndex(i))
		{
			int32 AmmoInMag = Magazine[i];
			if (AmmoInMag > 0)
			{
				Mags++;
			}
		}
	}
	return Mags;
}

int32 ATacticalWeapon::GetAmmoLoaded() const
{
	if (ReloadRoundsIndividually())
	{
		if (Magazine.IsValidIndex(0))
		{
			return Magazine[0];
		}
		return 0;
	}

	if (Magazine.IsValidIndex(CurrentMagazine))
	{
		return Magazine[CurrentMagazine];
	}
	return 0;
}

int32 ATacticalWeapon::GetMagazineCapacity() const
{
	return MagazineCapacity;
}

int32 ATacticalWeapon::GetBestMag() const
{
	int32 BestMagId = 0;
	int32 BestMagAmmo = 0;
	for (int32 i = 0; i < Magazine.Num(); i++)
	{
		if (Magazine.IsValidIndex(i))
		{
			int32 AmmoInMag = Magazine[i];
			if (AmmoInMag > BestMagAmmo)
			{
				BestMagId = i;
				BestMagAmmo = AmmoInMag;
			}
		}
	}
	return BestMagId;
}

FVector ATacticalWeapon::GetWeaponOffset(bool bAimed /*= false*/) const
{
	//FVector AimedOffset = WeaponAimOffset;
	const FVector RelLoc = GetInventoryOwner()->IsWeaponLeftHanded() ? GetMesh()->RelativeLocation * FVector(-1.f, 1.f, 1.f) : GetMesh()->RelativeLocation;
	FVector AimedOffset = - BaseAimPoint->RelativeLocation - RelLoc;
	{
		const float tempX = AimedOffset.X;
		AimedOffset.X = AimedOffset.Y;
		AimedOffset.Y = -tempX;
	}

	ATacticalWeaponAttachment_Sight* Sight = GetSightAttachment();

	if (bAimed && Sight)
	{
		AimedOffset = Sight->ModifyAimOffset(AimedOffset);
	}
	else
	{
		FVector SightOffset = SightAimPoint->RelativeLocation;
		SightOffset.X = SightAimPoint->RelativeLocation.Y;
		SightOffset.Y = -SightAimPoint->RelativeLocation.X;

		AimedOffset -= SightOffset;
	}
	return (!bAimed ? WeaponOffset : AimedOffset);
}

void ATacticalWeapon::GetAttachmentPoints(TArray<UTacticalWeaponAttachmentPoint*>& AttachPoints) const
{
	TArray<UActorComponent*> Comps = GetComponentsByClass(UTacticalWeaponAttachmentPoint::StaticClass());
	if (Comps.Num() > 0)
	{
		for (UActorComponent* TestComp : Comps)
		{
			if (TestComp)
			{
				if (UTacticalWeaponAttachmentPoint* TestAttach = Cast<UTacticalWeaponAttachmentPoint>(TestComp))
				{
					AttachPoints.Add(TestAttach);
				}
			}
		}
	}

	if (AttachPoints.Num() > 1)
	{
		AttachPoints.Sort([](const UTacticalWeaponAttachmentPoint& One, const UTacticalWeaponAttachmentPoint& Two){
			return (One.GetPriority() > Two.GetPriority());
		});
	}
}



ATacticalWeaponAttachment_Sight* ATacticalWeapon::GetSightAttachment() const
{
	TArray<UTacticalWeaponAttachmentPoint*> AttachPoints;
	GetAttachmentPoints(AttachPoints);
	for (UTacticalWeaponAttachmentPoint* TestAttach : AttachPoints)
	{
		if (TestAttach && TestAttach->GetCurrentAttachment())
		{
			ATacticalWeaponAttachment_Sight* TestSight = Cast<ATacticalWeaponAttachment_Sight>(TestAttach->GetCurrentAttachment());
			if (TestSight)
			{
				return TestSight;
			}
		}
	}
	return nullptr;
}

ATacticalWeaponAttachment_Muzzle* ATacticalWeapon::GetMuzzleAttachment() const
{
	TArray<UTacticalWeaponAttachmentPoint*> AttachPoints;
	GetAttachmentPoints(AttachPoints);
	for (UTacticalWeaponAttachmentPoint* TestAttach : AttachPoints)
	{
		if (TestAttach && TestAttach->GetCurrentAttachment())
		{
			ATacticalWeaponAttachment_Muzzle* TestMuzzle = Cast<ATacticalWeaponAttachment_Muzzle>(TestAttach->GetCurrentAttachment());
			if (TestMuzzle)
			{
				return TestMuzzle;
			}
		}
	}
	return nullptr;
}

void ATacticalWeapon::OnBulletDropCollision(FName EventName, float EmitterTime, int32 ParticleTime, FVector Location, FVector Velocity, FVector Direction, FVector Normal, FName BoneName, UPhysicalMaterial* PhysMat)
{
	if (AmmoType)
	{
		USoundBase* BulletDropSound = GetDefault<UTacticalAmmoType>(AmmoType)->GetBulletDropSound(PhysMat ? PhysMat->SurfaceType : EPhysicalSurface::SurfaceType_Default);
		if (BulletDropSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, BulletDropSound, Location);
		}
	}
}

void ATacticalWeapon::ConsumeAmmo()
{
	if (ReloadRoundsIndividually())
	{
		if (Magazine.IsValidIndex(0))
		{
			Magazine[0] -= 1;
		}
		return;
	}

	if (Magazine.IsValidIndex(CurrentMagazine))
	{
		Magazine[CurrentMagazine] -= 1;
	}
}

void ATacticalWeapon::TickSpread(float DeltaTime)
{
	if (FMath::Abs(CurrentSpread - TargetSpread) < KINDA_SMALL_NUMBER)
	{
		// Spread is almost Target -> Regenerate
		CurrentSpread = FMath::FInterpTo(CurrentSpread, GetMinSpread(), DeltaTime, GetSpreadRegeneration());
		TargetSpread = CurrentSpread;
	}
	else
	{
		// Spread needs to be increased
		CurrentSpread = FMath::FInterpTo(CurrentSpread, TargetSpread, DeltaTime, 20.f);
	}
}

float ATacticalWeapon::GetSpread() const
{
	// todo: use stats + attachment to calculate this value 
	float MovementSpread = 0.f;
	float StanceSpread = 0.f;
	bool bIsAiming = false;
	if (GetInventoryOwner())
	{
		const FRotator ViewRotationVelocity= GetInventoryOwner()->GetViewRotationSpeed();
		const float ViewRotationSpeed = FMath::Sqrt(FMath::Square(ViewRotationVelocity.Yaw) + FMath::Square(ViewRotationVelocity.Pitch));
		const float Speed = GetInventoryOwner()->GetVelocity().Size2D();

		MovementSpread += Speed * MovementSpreadModifier + ViewRotationSpeed * RotationSpreadModifier;
		MovementSpread = FMath::Clamp(MovementSpread, 0.f, MaxMovementSpread);
		switch (GetInventoryOwner()->GetStance())
		{
		case ETacticalStance::STANCE_Default:
			StanceSpread = AddSpread_Standing;
			break;
		case ETacticalStance::STANCE_Crouched:
			StanceSpread = AddSpread_Crouched;
			break;
		case ETacticalStance::STANCE_Prone:
			StanceSpread = AddSpread_Prone;
		default:
			break;
		}
		bIsAiming = GetInventoryOwner()->IsAiming();
	}

	float ReturnSpread = (CurrentSpread + StanceSpread) * (bIsAiming ? AimSpreadModifer : 1.f) + MovementSpread;


	TArray<UTacticalWeaponAttachmentPoint*> AttachmentPoints;
	GetAttachmentPoints(AttachmentPoints);
	for (UTacticalWeaponAttachmentPoint* TestPoint : AttachmentPoints)
	{
		if (TestPoint && TestPoint->GetCurrentAttachment())
		{
			ReturnSpread = TestPoint->GetCurrentAttachment()->ModifySpread(ReturnSpread);
		}
	}


	return ReturnSpread;
}

float ATacticalWeapon::GetCombinedSpread() const
{
	const float AmmoSpread = AmmoType->IsValidLowLevelFast() ? AmmoType->GetDefaultObject<UTacticalAmmoType>()->GetBuckshotAngle() : 0.f;
	return GetSpread() + AmmoSpread;
}

float ATacticalWeapon::GetMaxSpread() const
{
	// todo: use stats + attachment to calculate this value
	return Spread.Max;
}

float ATacticalWeapon::GetMinSpread() const
{
	// todo: use stats + attachment to calculate this value
	return Spread.Min;
}

float ATacticalWeapon::GetMaxMovementSpread() const
{
	return MaxMovementSpread;
}

void ATacticalWeapon::IncrementSpread()
{
	TargetSpread = FMath::Min(TargetSpread + GetSpreadIncrement(), GetMaxSpread());
}

float ATacticalWeapon::GetSpreadRegeneration() const
{
	// todo: use stats + attachment to calculate this value
	const ATacticalWeaponAttachment_Muzzle* Muzzle = GetMuzzleAttachment();
	float SpreadRegenMod = Muzzle ? Muzzle->GetSpreadRegenerationModifier() : 1.f;
	SpreadRegenMod *= AttachmentSpecial ? AttachmentSpecial->GetSpreadRegenerationModifier() : 1.f;
	return SpreadRecovery;
}

float ATacticalWeapon::GetSpreadIncrement() const
{
	// todo: use stats + attachment to calculate this value
	return SpreadIncrement;
}

FVector2D ATacticalWeapon::GetRecoil() const
{
	//const ATacticalWeaponAttachment_Muzzle* Muzzle = GetMuzzleAttachment();
	//float RecoilMod = Muzzle ? Muzzle->GetRecoilModifier() : 1.f;
	//RecoilMod *= AttachmentSpecial ? AttachmentSpecial->GetRecoilModifier() : 1.f;
	return BaseRecoil;
}

float ATacticalWeapon::GetFireInterval() const
{
	return BaseFireInterval;
}
float ATacticalWeapon::GetRange() const
{
	return BaseRange;
}

////////////////////////////////
// Firing

bool ATacticalWeapon::CanFire() const
{
	return (GetInventoryOwner() && GetInventoryOwner()->CanFire() && (!GetInventoryOwner()->IsWeaponObstructed() || GetInventoryOwner()->bMoveWeaponBack));
}

void ATacticalWeapon::GetWeaponTraceOriginAndDirection_Implementation(FVector& Origin, FVector& Direction) const
{
	Origin = GetInventoryOwner() ? GetInventoryOwner()->GetWeaponTraceOrigin() : GetActorLocation();
	Direction = GetCameraAim();
}

FVector ATacticalWeapon::GetCameraAim() const
{
	return GetInventoryOwner()->GetBaseAimRotation().Vector();
}

EWeaponFiringMode ATacticalWeapon::GetFiringMode() const
{
	return FiringMode;
}
EWeaponFiringMode ATacticalWeapon::GetDefaultFiringMode_Implementation() const
{
	if (bCanFMAuto)
	{
		return EWeaponFiringMode::FM_Auto;
	}
	else if (bCanFMBurst)
	{
		return EWeaponFiringMode::FM_Burst;
	}
	else if (bCanFMSingle)
	{
		return EWeaponFiringMode::FM_Single;
	}
	return EWeaponFiringMode::FM_None;
}

bool ATacticalWeapon::IsPendingFire() const
{
	return (GetInventoryComponent() ? GetInventoryComponent()->IsPendingFire() : false);
}

void ATacticalWeapon::SetPendingFire()
{
	if (GetInventoryComponent())
	{
		GetInventoryComponent()->SetPendingFire();
	}
}

void ATacticalWeapon::ClearPendingFire()
{
	if (GetInventoryComponent())
	{
		GetInventoryComponent()->ClearPendingFire();
	}
}


void ATacticalWeapon::NativeLocalSimulateFire()
{
	bool bSkipLocalSimulation = false;	

	FTimerHandle FireSoundTimer;
	if (TriggerDelay > 0.01f)
	{
		GetWorldTimerManager().SetTimer(FireSoundTimer, this, &ATacticalWeapon::PlayFiringSound, TriggerDelay, false);
	}
	else
	{
		PlayFiringSound();
	}

	if (GetMesh() && GetMesh()->GetAnimInstance())
	{
		UTacticalWeaponAnimInstance* AnimInstance = Cast<UTacticalWeaponAnimInstance>(GetMesh()->GetAnimInstance());
		if (AnimInstance)
		{
			AnimInstance->OnEventFire(0.f);
		}
		if(FiringAnimation)
		{
			GetMesh()->GetAnimInstance()->Montage_Play(FiringAnimation);
		}
	}
	if (BulletEjectPSC)
	{
		if (AmmoType && BulletEjectDelay >= 0.01f)
		{
			FTimerHandle TempTimer;
			GetWorldTimerManager().SetTimer(TempTimer, this, &ATacticalWeapon::SimulateBulletEject, BulletEjectDelay, false);
		}
		else
		{
			SimulateBulletEject();
		}
	}

	if (UParticleSystemComponent* MuzzlePSC = GetMuzzleFlashParticle())
	{
		MuzzlePSC->Activate(true);
	}


	AddRecoilAnimCurve(GetRecoilCurve(), bRandomizeYRecoil, bRandomizeYawRecoil, ScaleRecoil(1.f));


	if (!bSkipLocalSimulation)
	{
		LocalSimulateFire();
	}
}




void ATacticalWeapon::AddRecoilAnimCurve(UTacticalRecoilCurve* Curve, bool bRandomY, bool bRandomYaw, float Scale /*= 1.f*/)
{
	PendingAnimRecoils.Add(FPendingAnimRecoil(Curve, bRandomY, bRandomYaw, Scale));
}


void ATacticalWeapon::PlayFiringSound_Implementation()
{
	ATacticalWeaponAttachment_Muzzle* Muzzle = GetMuzzleAttachment();
	const bool bSilenced = (Muzzle != nullptr) && Muzzle->IsSilencer();

	USoundCue* FiringSoundToUse = bSilenced ? FiringSound_Silenced : FiringSound_Default;
	if(FiringSoundToUse)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FiringSoundToUse, GetActorLocation(), GetCameraAim().Rotation());
	}
}



void ATacticalWeapon::SimulateBulletEject()
{
	BulletEjectPSC->Activate(true);
}

void ATacticalWeapon::SimulateHit_Implementation(int32 RandomSeed, float InSpread)
{
	if (Role == ROLE_Authority || (!GetInventoryOwner() || !GetInventoryOwner()->IsLocallyControlled()))
	{
		SimulateHit_Internal(RandomSeed, InSpread);
	}
}

void ATacticalWeapon::SimulateHit_Internal(int32 RandomSeed, float InSpread)
{
	const UTacticalAmmoType* AmmoCDO = AmmoType ? AmmoType->GetDefaultObject<UTacticalAmmoType>() : nullptr;
	if (!AmmoCDO)
	{
		return;
	}
	FRandomStream WeaponRandomStream(RandomSeed);
	const float currentSpread = InSpread;
	const float ConeHalfAngle = FMath::DegreesToRadians(currentSpread * 0.5f);

	const float BuckshotConeHalfAngle = FMath::DegreesToRadians(AmmoCDO->GetBuckshotAngle() * 0.5f);

	FVector TraceStart, ShotDir;
	GetWeaponTraceOriginAndDirection(TraceStart, ShotDir);

	const int32 NumShots = AmmoCDO->GetBuckshots();

	//Simulate Weapon Trace
	for (int32 i = 0; i < NumShots; i++)
	{
		FVector TraceEnd = TraceStart + WeaponRandomStream.VRandCone(ShotDir, BuckshotConeHalfAngle) * FMath::Max(GetRange(), WEAPON_MIN_TRACE_RANGE);

		TArray<FHitResult> Hits;
		TraceShot(Hits, TraceStart, TraceEnd);

		const FVector TraceDir = (TraceEnd - TraceStart).GetSafeNormal();


		for (int32 NumPenetrations = 0; NumPenetrations < (AmmoCDO->AllowsSurfacePenetration() ? 2 : 1); NumPenetrations++)
		{
			if (Hits.IsValidIndex(NumPenetrations))
			{
				LocalSimulateHit(Hits[NumPenetrations], TraceDir);
			}
		}

	}
}

void ATacticalWeapon::LocalSimulateHit_Implementation(FHitResult Hit, const FVector& TraceDir)
{
	if (const UTacticalAmmoType* AmmoCDO = (AmmoType ? AmmoType->GetDefaultObject<UTacticalAmmoType>() : nullptr))
	{

		UTacticalImpactFXComponent* ImpactFXComp = nullptr;
		if (Hit.GetActor())
		{
			ImpactFXComp = Hit.GetActor()->FindComponentByClass<UTacticalImpactFXComponent>();

		}

		const FVector ShotDir = (Hit.TraceEnd - Hit.TraceStart).GetSafeNormal();

		if (AmmoCDO->GetImpactEffect())
		{
			ATacticalImpactFX* ImpactEffectCDO = AmmoCDO->GetImpactEffect()->GetDefaultObject<ATacticalImpactFX>();
			if (ImpactEffectCDO)
			{
				ImpactEffectCDO->SpawnImpactEffects(GetWorld(), Hit, ShotDir, ImpactFXComp);
			}
		}
		else if(ImpactFXComp)
		{
			// We don't have a ImpactEffect so we fall back to the Impact Override Comp
			ImpactFXComp->PlayImpactSound(Hit);
			ImpactFXComp->SpawnImpactParticle(Hit, ShotDir);
			ImpactFXComp->SpawnImpactDecal(Hit);
		}

		if (ImpactFXComp && ImpactFXComp->OnSimulateHit.IsBound())
		{
			ImpactFXComp->OnSimulateHit.Broadcast(Hit, ShotDir);
		}
	}

}

void ATacticalWeapon::OnFire()
{
	// Add Noise for AI Perception
	if (Role == ROLE_Authority && GetInventoryOwner())
	{
		float NoiseVolume = 1.f;
		// Modify Noise with Muzzle Attachment	
		const ATacticalWeaponAttachment_Muzzle* Muzzle = GetMuzzleAttachment();
		if (Muzzle)
		{
			NoiseVolume *= Muzzle->GetNoiseModifier();
		}
		// Clamp between 0 and 1 (1 means full hearing radius for Perceiver (AI)
		NoiseVolume = FMath::Clamp(NoiseVolume, 0.f, 1.f);
		MakeNoise(NoiseVolume, GetInventoryOwner(), GetInventoryOwner()->GetActorLocation(), 0.f, TEXT("Noise_GunFire"));
	}

	// Recoil for Gun
	ApplyRecoil();
	// static Recoil Camera Anim (for more detail/polish)
	if (RecoilCameraAnim && IsOwnerLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(GetInventoryOwner()->GetController()))
		{
			const float AnimScale = GetInventoryOwner()->IsAiming() ? AimRecoilCameraAnimScale : 1.f;
			PC->ClientPlayCameraAnim(RecoilCameraAnim, ScaleRecoil(AnimScale), 1.f, 0.1f, 0.1f, false, false, ECameraAnimPlaySpace::CameraLocal);
		}
	}

	// todo: Play Recoil for spectators in first person mode
}

void ATacticalWeapon::MakeFiringNoise()
{
	float NoiseMultiplier = 1.f;

	ATacticalWeaponAttachment_Muzzle* Muzzle = GetMuzzleAttachment();
	if (Muzzle)
	{
		NoiseMultiplier = Muzzle->GetNoiseModifier();
	}

	UAISense_Hearing::ReportNoiseEvent(this, GetInventoryOwner()->GetActorLocation(), FiringNoiseLoudness*NoiseMultiplier, GetInventoryOwner(), FiringNoiseMaxRange, FName("WeaponFire"));

}

void ATacticalWeapon::ApplyDamage(const FHitResult& Hit, const FVector& ShotDir, int32 NumPenetrations)
{
	if (Role == ROLE_Authority)
	{
		if (AmmoType && Hit.GetActor())
		{
			const UTacticalAmmoType* AmmoCDO = AmmoType->GetDefaultObject<UTacticalAmmoType>();
			const float Damage = AmmoCDO->GetDamage() * DamageModifier * FMath::Pow(AmmoCDO->GetSurfacePenetrationDamageModifier(), NumPenetrations);

			const TSubclassOf<UDamageType> DamageType = AmmoCDO->GetDamageType();

			UGameplayStatics::ApplyPointDamage(Hit.GetActor(), Damage, ShotDir, Hit, GetInventoryOwner()->Controller, GetInventoryOwner(), DamageType);
		}
	}
}

float ATacticalWeapon::ScaleRecoil(float BaseRecoilScale) const
{
	float ReturnRecoilScale = BaseRecoilScale;

	if (GetInventoryOwner()->IsAiming())
	{
		ReturnRecoilScale *= AimRecoilModifier;
	}


	TArray<UTacticalWeaponAttachmentPoint*> AttachPoints;
	GetAttachmentPoints(AttachPoints);
	if (AttachPoints.Num() > 0)
	{
		for (UTacticalWeaponAttachmentPoint* TestPoint : AttachPoints)
		{
			if (TestPoint && TestPoint->GetCurrentAttachment())
			{
				ReturnRecoilScale = TestPoint->GetCurrentAttachment()->ModifyRecoil(ReturnRecoilScale);
			}
		}
	}
	return ReturnRecoilScale;
}

void ATacticalWeapon::OnRep_WeaponState(uint8 PrevState)
{
	GotoState(GetWeaponStateForByte(RepWeaponState)); //todo maybe use gotostate()
}

UTacticalWeaponState* ATacticalWeapon::GetWeaponStateForByte(uint8 Val) const
{
	switch (Val)
	{
	case STATE_Active: 
		return StateActive;
	case STATE_Firing: 
		return StateFiring;
	case STATE_Equip: 
		return StateEquipping;
	case STATE_Unequip: 
		return StateUnequipping;
	case STATE_Reload: 
		return StateReloading;
	case STATE_Obstructed: 
		return StateObstructed;
	case STATE_GadgetFiring:
	{
		TArray<UTacticalWeaponAttachmentPoint*> AttachPoints;
		GetAttachmentPoints(AttachPoints);
		for (UTacticalWeaponAttachmentPoint* TestPoint : AttachPoints)
		{
			if (TestPoint->GetCurrentAttachment() == nullptr)
				continue;

			if (ATacticalWeaponAttachment_Weapon* TestAttach = Cast<ATacticalWeaponAttachment_Weapon>(TestPoint->GetCurrentAttachment()))
			{
				return TestAttach->StateFiring;
			}
		}
	}
	case STATE_GadgetReloading:
	{
		TArray<UTacticalWeaponAttachmentPoint*> AttachPoints;
		GetAttachmentPoints(AttachPoints);
		for (UTacticalWeaponAttachmentPoint* TestPoint : AttachPoints)
		{
			if (TestPoint->GetCurrentAttachment() == nullptr)
				continue;

			if (ATacticalWeaponAttachment_Weapon* TestAttach = Cast<ATacticalWeaponAttachment_Weapon>(TestPoint->GetCurrentAttachment()))
			{
				return TestAttach->StateReloading;
			}
		}
	}
	case STATE_Inactive:
	default:
		return StateInactive;
	}
	return StateInactive;
}

class ATacticalWeaponAttachment* ATacticalWeapon::CreateAttachment(const FString& AttachmentPointName, int32 AttachmentIdx, const FString& Options)
{
	TArray<UTacticalWeaponAttachmentPoint*> AttachPoints;
	GetAttachmentPoints(AttachPoints);
	if (AttachPoints.Num() <= 0)
		return nullptr;

	for (UTacticalWeaponAttachmentPoint* TestAttach : AttachPoints)
	{
		if (TestAttach->GetName().Compare(AttachmentPointName, ESearchCase::CaseSensitive) == 0)
		{
			return CreateAttachment(TestAttach, AttachmentIdx);
			break;
		}
	}

	return nullptr;
}

class ATacticalWeaponAttachment* ATacticalWeapon::CreateAttachment(UTacticalWeaponAttachmentPoint* AttachmentPoint, int32 AttachmentIdx, const FString& Options)
{
	TArray<TSubclassOf<ATacticalWeaponAttachment>> Attachments;
	AttachmentPoint->GetAttachmentList(Attachments);
	// Set AttachmentIdx to 0 (default attachment) when using invalid index
	if (!Attachments.IsValidIndex(AttachmentIdx))
	{
		AttachmentIdx = 0;
	}
	// Spawn actor (check index to make sure 0 exists)
	if (Attachments.IsValidIndex(AttachmentIdx))
	{
		const TSubclassOf<ATacticalWeaponAttachment> AttachmentClass = Attachments[AttachmentIdx];
		if (GetWorld() && AttachmentClass != nullptr)
		{
			FActorSpawnParameters Params;
			Params.Owner = this;
			Params.Instigator = GetInventoryOwner();
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			//ATacticalWeaponAttachment* SpawnedAttachment = GetWorld()->SpawnActor<ATacticalWeaponAttachment>(AttachmentClass, Params);
			ATacticalWeaponAttachment* SpawnedAttachment = GetWorld()->SpawnActorDeferred<ATacticalWeaponAttachment>(AttachmentClass, FTransform(), Params.Owner, Params.Instigator, Params.SpawnCollisionHandlingOverride);
			if (SpawnedAttachment != nullptr)
			{
				SpawnedAttachment->SetTickableWhenPaused(true);
				SpawnedAttachment->FinishSpawning(FTransform());
			}

			if (SpawnedAttachment != nullptr)
			{
				FAttachmentTransformRules AttachRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false);
				SpawnedAttachment->AttachToComponent(AttachmentPoint, AttachRules);
				AttachmentPoint->SetCurrentAttachment(SpawnedAttachment);
				AttachmentPoint->SetAttachmentIdx(AttachmentIdx);

				SpawnedAttachment->OnApplyOptions(Options);

				return SpawnedAttachment;
			}

		}
	}
	// we should not get here, but can happen
	AttachmentPoint->SetAttachmentIdx(-1);
	AttachmentPoint->SetCurrentAttachment(nullptr);

	return nullptr;
}

bool ATacticalWeapon::IsAttachmentAllowed(TSubclassOf<class ATacticalWeaponAttachment> AttachmentClass) const
{
	return false;
}

bool ATacticalWeapon::IsAttachmentAllowed(class ATacticalWeaponAttachment* Attachment) const
{
	return IsAttachmentAllowed(Attachment->GetClass());
}

float ATacticalWeapon::MontagePlay(UAnimMontage* Montage, float Rate, FName Section)
{
	float ReturnTime = -1.f;
	if (GetMesh() && Montage)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			ReturnTime = AnimInstance->Montage_Play(Montage, Rate);
			
			if (Montage->IsValidSectionName(Section))
			{
				AnimInstance->Montage_JumpToSection(Section);
			}
		}
	}
	return ReturnTime;
}

class UTacticalRecoilCurve* ATacticalWeapon::GetRecoilCurve_Implementation() const
{
	return RecoilAnimation;
}

void ATacticalWeapon::SetAmmoType(int32 AmmoTypeIdx)
{
	check(AllowedAmmoTypes.IsValidIndex(DefaultAmmoType));

	AmmoType = AllowedAmmoTypes.IsValidIndex(AmmoTypeIdx) ? AllowedAmmoTypes[AmmoTypeIdx] : AllowedAmmoTypes[DefaultAmmoType];
}

float ATacticalWeapon::GetADSSpeed() const
{
	float Result_ADSSpeed = ADSSpeed;
	TArray<UTacticalWeaponAttachmentPoint*> AttachPoints;
	GetAttachmentPoints(AttachPoints);
	for (UTacticalWeaponAttachmentPoint* TestPoint : AttachPoints)
	{
		if (TestPoint && TestPoint->GetCurrentAttachment())
		{
			Result_ADSSpeed = TestPoint->GetCurrentAttachment()->ModifyAimSpeed(Result_ADSSpeed);
		}
	}
	return Result_ADSSpeed;
}

bool ATacticalWeapon::IsFiring() const
{
	return CurrentState->IsFiring();
}

void ATacticalWeapon::TraceShot(TArray<FHitResult>& OutHits, const FVector& TraceStart, const FVector& TraceEnd) const
{
	FHitResult Hit = FHitResult(ForceInit);

	OutHits.Empty();

	FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("TraceShot")), true, GetInventoryOwner());
	TraceParams.AddIgnoredActor(this);
	TraceParams.bReturnFaceIndex = true;
	TraceParams.bReturnPhysicalMaterial = true;
	TArray<UTacticalWeaponAttachmentPoint*> AttachPoints;
	GetAttachmentPoints(AttachPoints);
	if (AttachPoints.Num() > 0)
	{
		for (UTacticalWeaponAttachmentPoint* TestAttach : AttachPoints)
		{
			if (TestAttach->GetCurrentAttachment())
			{
				TraceParams.AddIgnoredActor(TestAttach->GetCurrentAttachment());
			}
		}
	}

	TraceParams.bReturnPhysicalMaterial = true;

	//FCollisionResponseParams()
	//FCollisionObjectQueryParams TraceObjects = FCollisionObjectQueryParams(ECC_TO_BITFIELD(ECC_Weapon));

	//GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, TRACE_Weapon, TraceParams);

	GetWorld()->LineTraceMultiByChannel(OutHits, TraceStart, TraceEnd, TRACE_Weapon, TraceParams);

	//return Hit;
}

FHitResult ATacticalWeapon::TraceShotCompensated(FVector TraceStart, FVector TraceEnd, float RewindTime)
{
	bool bLagCompoensation = ((RewindTime > 0.f) && (Role == ROLE_Authority));

	ECollisionChannel TraceChannel = bLagCompoensation ? TRACE_WeaponNoCharacter : TRACE_Weapon;
	FHitResult Hit(ForceInit);
	FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("TraceShot")), true, GetInventoryOwner());
	TraceParams.AddIgnoredActor(this);
	TraceParams.bReturnPhysicalMaterial = true;

	if (!GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, TraceChannel, TraceParams))
	{
		Hit.Location = TraceEnd;
	}

	//DrawDebugLine(GetWorld(), TraceStart, Hit.Location, FColor::Red, true, 1.f);
	if (bLagCompoensation && !(Hit.Location - TraceStart).IsNearlyZero())
	{
		//GetWorld()->LineTraceSingle()
		ATacticalCharacter* BestTarget = nullptr;
		FVector BestClosestPoint = FVector::ZeroVector;
		FVector BestTargetPoint = FVector::ZeroVector;
		float BestRadius = 0.f;
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
		{
			ATacticalCharacter* TestChar = Cast<ATacticalCharacter>(*It);
			if (TestChar && TestChar != GetInventoryOwner() && TestChar->IsAlive())
			{
				FVector TestLoc = TestChar->GetRewindPosition(RewindTime);
				float TestRadius = TestChar->GetCapsuleComponent()->GetScaledCapsuleRadius();
				float TestHeight = TestChar->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
				FVector ClosestPoint = FMath::ClosestPointOnSegment(TestLoc, TraceStart, Hit.Location);

				if (!(ClosestPoint - Hit.Location).IsNearlyZero() // if closest point is at trace end shot most likely missed
					&& (FMath::Abs(TestLoc.Z - ClosestPoint.Z) < TestHeight) // check if Height is ok
					&& ((TestLoc - ClosestPoint).Size2D() < TestRadius)) // check if closets point is in radius
				{
					// Character can be hit. See if it is closer to the origin of the shot than the previous best
					if (!BestTarget || ((ClosestPoint - TraceStart).SizeSquared() < (BestClosestPoint - TraceStart).SizeSquared()))
					{
						BestTarget = TestChar;
						BestClosestPoint = ClosestPoint;
						BestTargetPoint = TestLoc;
						BestRadius = TestRadius;
					}
				}
			}
		}
		if (BestTarget)
		{
			// Found a player that was hit with PingCompensation
			// update HitResult

			Hit.Location = BestClosestPoint;
			Hit.Normal = (TraceEnd - TraceStart).GetSafeNormal2D();
			Hit.Actor = BestTarget;
			Hit.bBlockingHit = true;
			Hit.Component = BestTarget->GetCapsuleComponent();

			// todo: optimize with look a numeric math

			float DistanceFromClosest = FMath::Sqrt(FMath::Square(BestRadius) - (BestClosestPoint - BestTargetPoint).SizeSquared2D());
			float HeightOffset = (BestClosestPoint.Z - TraceStart.Z) * (DistanceFromClosest / (BestClosestPoint - TraceStart).Size2D());
			FVector BestImpactPoint = BestClosestPoint + DistanceFromClosest * (TraceStart - Hit.Location).GetSafeNormal2D();
			BestImpactPoint.Z -= HeightOffset;

			Hit.ImpactPoint = BestImpactPoint;
			Hit.Time = (BestClosestPoint - TraceStart).Size() / (TraceEnd - TraceStart).Size();
			// maybe generate Hit.Normal
		}
	}

	return Hit;
}

void ATacticalWeapon::OnEmpty()
{
	// Gun becomes Empty
	if (IsOwnerLocallyControlled() && IsOwnerPlayerControlled())
	{
		LocalSimulateEmpty();
	}
}

void ATacticalWeapon::StartFiring()
{
	int32 RandomSeed = FMath::Rand();
	StartFiring_Internal(RandomSeed);
	if (Role < ROLE_Authority)
	{
		ServerStartFiring(RandomSeed);
	}
}
void ATacticalWeapon::ServerStartFiring_Implementation(int32 RandomSeed)
{
	StartFiring_Internal(RandomSeed);
}
void ATacticalWeapon::StartFiring_Internal(int32 RandomSeed)
{
	if (!GetInventoryOwner() || IsPendingKill())
		return;

	RandomSpreadStream.Initialize(RandomSeed);
	// Set Pending Fire
	SetPendingFire();

	// If Weapon is active Fire Weapon, if not current fire needs to be handled first
	CurrentState->BeginFiring();
}

void ATacticalWeapon::StopFiring()
{
	StopFiring_Internal();
	if (Role < ROLE_Authority)
	{
		ServerStopFiring();
	}
}
void ATacticalWeapon::ServerStopFiring_Implementation()
{
	StopFiring_Internal();
}
void ATacticalWeapon::StopFiring_Internal()
{
	ClearPendingFire();
	CurrentState->EndFiring();
}


void ATacticalWeapon::ApplyRecoil()
{
	if (GetInventoryOwner())
	{
		FVector2D RecoilToAdd = GetRecoil();
		if (bRandomizeYRecoil || bRandomizeYawRecoil)
		{
			RecoilToAdd.Y = RandomSpreadStream.FRandRange(-BaseRecoil.Y, BaseRecoil.Y);
		}
		switch (GetInventoryOwner()->GetStance())
		{
		case ETacticalStance::STANCE_Crouched:
			RecoilToAdd *= CrouchRecoilModifier;
			break;
		case ETacticalStance::STANCE_Prone:
			RecoilToAdd *= ProneRecoilModifier;
			break;
		default:
			break;
		}
		RecoilToAdd *= ScaleRecoil(1.f);
		GetInventoryOwner()->AddRecoil(RecoilToAdd);
	}
}

////////////////////////////////
// Reload

void ATacticalWeapon::StartReload()
{
	StartReload_Internal();
	if (Role < ROLE_Authority)
	{
		ServerStartReload();
	}
}
void ATacticalWeapon::ServerStartReload_Implementation()
{
	StartReload_Internal();
}
void ATacticalWeapon::StartReload_Internal()
{
	// if Weapon is active reload right away
	UE_LOG(LogTemp, Log, TEXT("Wants to Reload in State: %s"), CurrentState ? *CurrentState->GetName() : *FString(TEXT("None")));
	if (CurrentState)
	{
		CurrentState->Reload();
	}
}

void ATacticalWeapon::StopReload()
{
	StopReload_Internal();
	if (Role < ROLE_Authority)
	{
		ServerStopReload();
	}
}
void ATacticalWeapon::ServerStopReload_Implementation()
{
	StopReload_Internal();
}
void ATacticalWeapon::StopReload_Internal()
{
	UE_LOG(LogTemp, Log, TEXT("Wants to stop Reload in State: %s"), CurrentState ? *CurrentState->GetName() : *FString(TEXT("None")));
	if (CurrentState)
	{
		CurrentState->CancelReload();
	}
}

bool ATacticalWeapon::NeedsReload() const
{
	bool bHasMags = true;
	if (GetAmmoLoaded() > 0)
	{
		bHasMags = (GetNumMagazines() > 1); // if current mag has ammo and num mags is 1, then current Mag is the only one that is counted.
	}
	else
	{
		bHasMags = (GetNumMagazines() > 0);
	}

	return (CanReload() && (GetAmmoLoaded() < (MagazineCapacity + (bCanBulletStayInChamber ? 1 : 0))) && HasAnyAmmo() && bHasMags);
}

bool ATacticalWeapon::CanReload() const
{
	return (GetInventoryOwner()->CanReload());
}

float ATacticalWeapon::GetReloadTime() const
{
	return ReloadTime;
}


UAnimMontage* ATacticalWeapon::GetFPReloadAnim() const
{
	return FPReloadAnim;
}

UAnimMontage* ATacticalWeapon::GetReloadAnim() const
{
	return WeaponReloadAnim;
}

float ATacticalWeapon::GetReloadProgress() const
{
	if (CurrentState != nullptr && IsReloading())
	{
		UTacticalWeaponStateReloading* ReloadState = Cast<UTacticalWeaponStateReloading>(CurrentState);
		if (ReloadState != nullptr)
		{
			const float ActualReloadTime = GetReloadTime();
			const float ReloadTimeElapsed = ReloadState->GetReloadTimeElapsed();
			if (ActualReloadTime >= 0.f)
			{
				return (ReloadTimeElapsed / ActualReloadTime);
			}
		}
	}
	return -1.f;
}

bool ATacticalWeapon::IsReloading() const
{
	return CurrentState->IsReloading();
}

void ATacticalWeapon::LocalSimulateReload()
{
	if (GetInventoryOwner())
	{
		const float AnimTime = ReloadRoundsIndividually() ? ReloadStartTime : ReloadTime;
		//GetInventoryOwner()->OnWeaponReload(AnimTime); //todo
		if (GetInventoryComponent()->OnWeaponReload.IsBound())
		{
			GetInventoryComponent()->OnWeaponReload.Broadcast(this, AnimTime);
		}

		if (!GetInventoryOwner()->IsThirdPerson())
		{
			UAnimInstance* WeaponAnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
			if ((WeaponReloadAnim != nullptr) && (WeaponAnimInstance != nullptr))
			{
				const float MontageTime = WeaponReloadAnim->GetSectionLength(0);
				WeaponAnimInstance->Montage_Play(WeaponReloadAnim, MontageTime / AnimTime);

				UTacticalWeaponAnimInstance* TacticalAnimInstance = Cast<UTacticalWeaponAnimInstance>(WeaponAnimInstance);
				if (TacticalAnimInstance)
				{
					TacticalAnimInstance->OnEventReload(AnimTime);
				}
			}
		}

		if (ReloadRoundsIndividually())
		{
			GetWorldTimerManager().SetTimer(ReloadStartTimer, this, &ATacticalWeapon::LocalSimulateContinueReload, FMath::Max(ReloadStartTime, 0.01f), false);
		}
		OnSimulateReloadStart(AnimTime);
	}
}

void ATacticalWeapon::LocalSimulateContinueReload()
{
	if (GetInventoryOwner())
	{
		if (GetInventoryOwner()->GetFirstPersonMesh())
		{
			UAnimInstance* FPAnimInstance = GetInventoryOwner()->GetFirstPersonMesh()->GetAnimInstance();
			if (FPAnimInstance)
			{
				const float MontageLoopTime = FPReloadAnim->GetSectionLength(1);
				FPAnimInstance->Montage_SetPlayRate(FPReloadAnim, MontageLoopTime/ReloadTime);
			}
		}
		UAnimInstance* WeaponAnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
		//GetMesh()->GetAnimInstance()
		if (WeaponReloadAnim && WeaponAnimInstance)
		{
			const float MontageLoopTime = WeaponReloadAnim->GetSectionLength(1);
			WeaponAnimInstance->Montage_SetPlayRate(WeaponReloadAnim, MontageLoopTime/ReloadTime);
		}
		OnSimulateReloadContinue(ReloadTime);
	}
}

void ATacticalWeapon::NetMulti_Reload_Implementation()
{
	if (IsOwnerLocallyControlled())
	{
		LocalSimulateReload();
	}
}


void ATacticalWeapon::LocalSimulateStopReload()
{
	if (GetInventoryOwner())
	{
		GetInventoryOwner()->OnWeaponStopReload(GetReloadTime());

		const float AnimTime = ReloadFinishTime;
		UAnimInstance* WeaponAnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
		if (WeaponReloadAnim && WeaponAnimInstance)
		{
			const float MontageTime = WeaponReloadAnim->GetSectionLength(2);
			WeaponAnimInstance->Montage_SetPlayRate(WeaponReloadAnim, MontageTime/ AnimTime);
			WeaponAnimInstance->Montage_JumpToSection(FName("End"), WeaponReloadAnim);
		}
		OnSimulateReloadFinish(AnimTime);
	}
}

void ATacticalWeapon::NetMulti_StopReload_Implementation()
{
	if (IsOwnerLocallyControlled())
	{
		LocalSimulateStopReload();
	}
}

void ATacticalWeapon::ToggleFiringMode()
{
	LocalToggleFiringMode();
	if (Role < ROLE_Authority)
	{
		ServerToggleFiringMode();
	}
}

void ATacticalWeapon::ServerToggleFiringMode_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("Needs Reload?: %d  - Can Reload?: %d"), NeedsReload(), CanReload());

	LocalToggleFiringMode();
}

void ATacticalWeapon::LocalToggleFiringMode()
{
	if (IsOwnerLocallyControlled() && IsOwnerPlayerControlled())
	{
		// todo: play sound cue for audio feedback
		OnToggleFiringMode();
	}
	switch (GetFiringMode())
	{
		case EWeaponFiringMode::FM_Auto:
			if (bCanFMSingle)
			{
				FiringMode = EWeaponFiringMode::FM_Single;
				break;
			}
		case EWeaponFiringMode::FM_Single:
			if (bCanFMBurst)
			{
				FiringMode = EWeaponFiringMode::FM_Burst;
				break;
			}
		case EWeaponFiringMode::FM_Burst:
			if (bCanFMAuto)
			{
				FiringMode = EWeaponFiringMode::FM_Auto;
				break;
			}
		default:
			break;
	}
}

FName ATacticalWeapon::GetEquippedSocketName()
{
	if (GetInventoryOwner() && GetInventoryOwner()->IsWeaponLeftHanded())
	{
		return EquippedSocketNameLeft;
	}
	return EquippedSocketName;
}

EWeaponType ATacticalWeapon::GetWeaponType() const
{
	return WeaponType;
}

void ATacticalWeapon::AttachGunToHolster()
{
	FAttachmentTransformRules AttachRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false);
	AttachToComponent(GetInventoryOwner()->GetMesh(), AttachRules, AttachSocketName);
	CurrentAttachmentPoint = AttachSocketName;
}

void ATacticalWeapon::AttachGunToHand()
{
	FAttachmentTransformRules AttachRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false);
	AttachToComponent(GetInventoryOwner()->GetMesh(), AttachRules, GetEquippedSocketName());
	if (GetEquippedSocketName() == EquippedSocketNameLeft)
	{
		GetMesh()->RelativeLocation = GetDefault<ATacticalWeapon>(GetClass())->GetMesh()->RelativeLocation * FVector(-1.f,1.f,1.f);
	}
	else
	{
		GetMesh()->RelativeLocation = GetDefault<ATacticalWeapon>(GetClass())->GetMesh()->RelativeLocation;
	}
	CurrentAttachmentPoint = GetEquippedSocketName();
}

void ATacticalWeapon::AttachGunToIK()
{
	FAttachmentTransformRules AttachRules = FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false);
	AttachToComponent(GetInventoryOwner()->GetMesh(), AttachRules, GunIKSocketName);
	CurrentAttachmentPoint = GunIKSocketName;
}

ATacticalPrimaryWeapon::ATacticalPrimaryWeapon(const FObjectInitializer& OI)
	: Super(OI)
{
		WeaponType = EWeaponType::Rifle;
		if (GetWidgetPointComponent() != nullptr)
		{
			GetWidgetPointComponent()->RelativeLocation = FVector(0.f, 18.f, 11.f);
		}
}

ATacticalSidearm::ATacticalSidearm(const FObjectInitializer& OI)
	: Super(OI)
{
		WeaponType = EWeaponType::Pistol;
		AIBurstInterval = FFloatInterval(0.2f, 0.5f);
		AIBurstCount = FInt32Interval(1, 2);

		if (GetWidgetPointComponent() != nullptr)
		{
			GetWidgetPointComponent()->RelativeLocation = FVector(0.f, 10.f, 6.f);
		}
}

FPendingAnimRecoil::FPendingAnimRecoil(class UTacticalRecoilCurve* newAnim, bool bRandomizeY, bool bRandomizeYaw, float newScale)
{
	Anim = newAnim;
	Time = 0.f;
	bPendingKill = false;
	Scale = newScale;

	const float RandFactor = FMath::FRandRange(-1.f, 1.f);
	YawFactor = bRandomizeYaw ? RandFactor*newScale : 1.f;
	YFactor = bRandomizeY ? RandFactor*newScale : 1.f;

	Duration = 0.f;
	if (Anim != nullptr)
	{
		Duration = Anim->GetDuration();
	}
	else
	{
		bPendingKill = true;
	}
}

void FPendingAnimRecoil::Tick(float DeltaTime)
{
	Time += DeltaTime;
	if (Time >= Duration)
	{
		bPendingKill = true;
	}
}
