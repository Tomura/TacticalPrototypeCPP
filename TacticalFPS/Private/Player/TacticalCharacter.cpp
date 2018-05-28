// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "UnrealNetwork.h"
#include "DisplayDebugHelpers.h"

#include "Animation/AnimMontage.h"

#include "Perception/AISense_Team.h"
#include "Perception/AISense_Damage.h"

#include "TacticalCharacter.h"
#include "TacticalCharacterMovement.h"
#include "TacticalAIController.h"

#include "TacticalInventoryComponent.h"
#include "TacticalInventory.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponAttachment.h"
#include "TacticalWeaponAttachmentPoint.h"
#include "TacticalWeaponStateReloading.h"
#include "TacticalWeaponStateGadgetReload.h"
#include "TacticalWeaponAttachment_Weapon.h"
#include "TacticalGrenade.h"

#include "BRSActorHealthComponent.h"
#include "TacticalPlayerController.h"
#include "BRSCharacterInteractionInterface.h"
#include "TacticalVaultMarker.h"
#include "TacticalDamageType.h"

#include "TacticalGameplaySettings.h"

#include "TacticalGameModeBase.h"

#include "TacticalCoverMarker.h"

#include "TacticalWeaponAttachmentPoint.h"
#include "TacticalGadgetInterface.h"

#include "TacticalImpactFX.h"
#include "TacticalImpactFXComponent.h"

#include "Perception/AIPerceptionSystem.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Perception/AISense_Touch.h"
#include "Perception/AISense_Team.h"

#include "Components/WidgetInteractionComponent.h"

//////////////////////////////////////////////////////////////////////////
// ATacticalCharacter

// Constructor
ATacticalCharacter::ATacticalCharacter(const FObjectInitializer& OI)
	: Super(OI.SetDefaultSubobjectClass<UTacticalCharacterMovement>(ACharacter::CharacterMovementComponentName))
{
	//SetReplicates(true);
	bReplicates = true;
	bReplicateMovement = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	bForceRotation = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->SetIsReplicated(true);

	GetMesh()->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::AlwaysTickPoseAndRefreshBones;
	GetMesh()->bEnablePhysicsOnDedicatedServer = true; // for rag doll

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FPCamera"));
	FollowCamera->SetupAttachment(GetMesh());
	//FollowCamera->AttachTo(GetMesh(), TEXT("head")); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	//FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	NoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("NoiseEmitter"));

	HealthComponent = CreateDefaultSubobject<UBRSActorHealthComponent>(TEXT("Health"));
	InventoryComponent = CreateDefaultSubobject<UTacticalInventoryComponent>(TEXT("Inventory"));

	ProjectileSpawn = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSpawn"));
	ProjectileSpawn->SetupAttachment(FollowCamera);
	ProjectileSpawn->SetRelativeLocation(FVector(30.f, 0.f, 0.f));

	FPMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPArms"));
	FPMesh->SetupAttachment(FollowCamera);
	FPMesh->RelativeLocation = FVector(-15.f, 0.f, -155.f);
	FPMesh->RelativeRotation = FRotator(0.f, -90.f, 0.f);
	FPMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FPMesh->bOnlyOwnerSee = true;
	FPMesh->SetVisibility(false);
	FPMesh->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::AlwaysTickPoseAndRefreshBones;


	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
	LeanAngle = 0.f;
	TargetLeanAngle = 0.f;
	MaxLeanAngle = 50.f;
	LeanSpeed = 100.f;

	LastViewRotation = FRotator::ZeroRotator;
	ViewRotationSpeed = FRotator::ZeroRotator;


	BaseWeaponHandOffset_Default	= FVector(14.f, 13.f, -23.5f);
	BaseWeaponHandOffset_Aim		= FVector(14.f, 6.f, -19.f);
	WeaponHandOffset = BaseWeaponHandOffset_Default;

	bWeaponDown = false;
	bFullyRagdoll = false;
	RecoilSpeed = 15.f;

	GrenadeCooldown = 0.85f;

	RootAdjustSpeed = 20.f;
	MaxRootAdjustment = 30.f;
	FootPlacementSpeed = 30.f;

	InteractionTraceDistance = 70.f;

	bIsWeaponObstructed = false;

	ObstructionTesterExtent = FVector(40.f, 3.f, 5.f);

	MinSafeFallSpeedZ = -300.f;
	LethalFallSpeedZ = -680.f;

	InteractActor = nullptr;

	RootTraceLocs.Add(FVector(30.f, 0.f, 0.f));
	RootTraceLocs.Add(FVector(-30.f, 0.f, 0.f));

	CrouchTimeForToggle = 0.5f;
	CrouchTime = 0.f;

	bThirdPersonView = false;

	BonesToHide.Empty();

	bUseFPArmsModel = false;

	TeamNotifyDamageRadius = 700.f;

	CapsuleGrowthSpeed = 10.f;

	SpeedIncrement = 0.05f;
	WalkSpeedFactor = 0.7f;

	WeaponSway = FVector::ZeroVector;
	BehaviorTreeAssetOverride = nullptr;



	InteractImmidiateTimeFrame = 0.25f;

	MeleeDamage = 200.f;
	MeleeRange = 200.f;
	MeleeDamageDelay = 0.25f;

	FootStepNoiseMaxRange = 1000.f;
	FootStepNoiseSpeedFactor = 0.0015f;

	WidgetInteractionComp = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("Widget Interaction"));
	WidgetInteractionComp->InteractionDistance = 100.f;
	WidgetInteractionComp->bEnableHitTesting = true;
	WidgetInteractionComp->SetupAttachment(FollowCamera);
	WidgetInteractionComp->InteractionSource = EWidgetInteractionSource::World;
	//WidgetInteractionComp->

	bWeaponLeftHanded = false;
}

void ATacticalCharacter::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	if (Role == ROLE_Authority && GetController())
	{
		SetRemoteViewYaw(GetController()->GetControlRotation().Yaw);
	}
}

void ATacticalCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	GetHealthComponent()->OnDeath.AddDynamic(this, &ATacticalCharacter::OnDeath);
	GetHealthComponent()->OnDamage.AddDynamic(this, &ATacticalCharacter::OnDamage);

	if (GetInventory())
	{
		GetInventory()->OnWeaponBeginEquip.AddDynamic(this, &ATacticalCharacter::OnWeaponEquip);
		GetInventory()->OnWeaponBeginUnequip.AddDynamic(this, &ATacticalCharacter::OnWeaponUnequip);
		GetInventory()->OnWeaponReload.AddDynamic(this, &ATacticalCharacter::OnWeaponReload);
	}
}

// Network
void ATacticalCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATacticalCharacter, bIsVaulting);
	DOREPLIFETIME(ATacticalCharacter, bVaultForward);
	DOREPLIFETIME(ATacticalCharacter, CurrentVaultMarker);
	DOREPLIFETIME(ATacticalCharacter, RotationType);
	DOREPLIFETIME(ATacticalCharacter, bFullyRagdoll);
	DOREPLIFETIME(ATacticalCharacter, bIsStunned);
	DOREPLIFETIME(ATacticalCharacter, TargetCapsuleSize);
	DOREPLIFETIME(ATacticalCharacter, bIsAIControlled);
	DOREPLIFETIME(ATacticalCharacter, bAIWantsToCover);
	DOREPLIFETIME(ATacticalCharacter, bAIWantsToFire);
	DOREPLIFETIME(ATacticalCharacter, CoverAimType);
	DOREPLIFETIME_CONDITION(ATacticalCharacter, bIsWeaponObstructed, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ATacticalCharacter, bWantsToAim, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ATacticalCharacter, bIsAiming, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ATacticalCharacter, LeanAngle, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ATacticalCharacter, bThrowingGrenade, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ATacticalCharacter, RemoteViewYaw, COND_SkipOwner);
}

// Overrides from AActor
void ATacticalCharacter::BeginPlay()
{
	Super::BeginPlay();

	TargetCapsuleSize = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();

	if (GetMesh() && GetMesh()->DoesSocketExist(TEXT("head")))
	{
		FollowCamera->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, false), TEXT("head"));
	}
	//if (Role == ROLE_Authority || Role == ROLE_AutonomousProxy)
	//{
	//	GetWorldTimerManager().SetTimer(WeaponObstructionTimer, this, &ATacticalCharacter::TimeCheckWeaponObstruction, 0.1f, true, 0.1f);
	//}

	if (bUseFPArmsModel)
	{
		if (GetMesh())
		{
			GetMesh()->bOwnerNoSee = true;
		}
		if (FPMesh)
		{
			FPMesh->bOnlyOwnerSee = true;
			FPMesh->SetVisibility(true);
		}
	}
	else
	{
		if(FPMesh)
			FPMesh->SetVisibility(false, true);
	}

	if (GetMesh() && FPMesh && FPMesh->IsVisible() && BonesToHide.Num() > 0)
	{
		for (const FName& Bone : BonesToHide)
		{
			GetMesh()->HideBoneByName(Bone, EPhysBodyOp::PBO_None);
		}
	}

	// Register as stimuli source
	UAIPerceptionSystem::RegisterPerceptionStimuliSource(this, UAISense_Hearing::StaticClass(), this);
}

void ATacticalCharacter::Restart()
{
	Super::Restart();

	if (GetInventory() && GetInventory()->IsValidLowLevel())
	{
		GetInventory()->OnOwnerRestart();
	}
}

void ATacticalCharacter::Destroyed()
{
	if (GetInventory())
	{
		GetInventory()->DiscardInventory();
	}
	Super::Destroyed();
}

void ATacticalCharacter::DisplayDebug(class UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);
	float Indent = 0.f;
	UFont* RenderFont = GEngine->GetSmallFont();
	FFontRenderInfo FontRenderInfo;
	FontRenderInfo.bEnableShadow = true;

	FColor DefaultDrawColor = Canvas->DrawColor;
	Canvas->SetDrawColor(FColor(220, 220, 220, 255));

	YPos += YL;

	YL = Canvas->DrawText(RenderFont, FString::Printf(TEXT("Speed: %f (%d, %d)"), GetVelocity().Size2D(), bMoveFwd, bMoveRight), Indent, YPos, 1.f, 1.f, FontRenderInfo);
	YPos += YL;
	YPos += YL;

	if (HealthComponent)
	{
		YL = Canvas->DrawText(RenderFont, FString::Printf(TEXT("Health: %f/%f"), HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth()), Indent, YPos, 1.f, 1.f, FontRenderInfo);
		YPos += YL;
		YPos += YL;
	}

	static FName NAME_Weapon = FName(TEXT("Weapon"));
	if (DebugDisplay.IsDisplayOn(NAME_Weapon))
	{
		FIndenter WeaponIndent(Indent);

		// Print current weapon information
		FString WeaponString = FString::Printf(TEXT("Current Weapon: %s"), GetWeapon() ? *GetWeapon()->GetClass()->GetName() : TEXT("NULL"));
		YL = Canvas->DrawText(RenderFont, WeaponString, Indent, YPos, 1.f, 1.f, FontRenderInfo);
		YPos += YL;


		if (GetWeapon())
		{
			FColor OldColor = Canvas->DrawColor;
			Canvas->SetDrawColor(FColor::White);

			YL = Canvas->DrawText(RenderFont, FString::Printf(TEXT("Ammo: %d/%d, Magazines: %d"), GetWeapon()->GetAmmoLoaded(), GetWeapon()->GetMagazineCapacity(), GetWeapon()->GetNumMagazines()), Indent + 8.f, YPos, 1.f, 1.f, FontRenderInfo);
			YPos += YL;
			FString FiringModeString;
			switch (GetWeapon()->GetFiringMode())
			{
			case EWeaponFiringMode::FM_Auto:
				FiringModeString = "Automatic";
				break;
			case EWeaponFiringMode::FM_Burst:
				FiringModeString = FString::Printf(TEXT("Burst (x%d)"), GetWeapon()->GetBurstCount());
				break;
			case EWeaponFiringMode::FM_Single:
				FiringModeString = "Single";
				break;
			default:
				FiringModeString = "INVALID";
				break;
			}
			
			YL = Canvas->DrawText(RenderFont, FString::Printf(TEXT("FiringMode: %s"), *FiringModeString), Indent + 8.f, YPos, 1.f, 1.f, FontRenderInfo);
			YPos += YL; 
			YL = Canvas->DrawText(RenderFont, FString::Printf(TEXT("Spread: %f°"), GetWeapon()->GetCombinedSpread()), Indent + 8.f, YPos, 1.f, 1.f, FontRenderInfo);
			YPos += YL;

			// Draw Spread as circle
			if (Cast<APlayerController>(Controller))
			{
				const FVector2D ScreenSize(Canvas->SizeX, Canvas->SizeY);
				const FVector2D ScreenCenter = ScreenSize*0.5f;
				const EAspectRatioAxisConstraint AspectRatioAxisConstraint = GetDefault<ULocalPlayer>()->AspectRatioAxisConstraint;
				
				FVector2D Center = ScreenCenter + 10 * FVector2D(GetWeaponSway());

				float FOVScreenSize = 0.f;
				switch (AspectRatioAxisConstraint)
				{
				case AspectRatio_MaintainYFOV:
					FOVScreenSize = ScreenSize.Y;
					break;
				case AspectRatio_MajorAxisFOV:
					FOVScreenSize = FMath::Max(ScreenSize.X, ScreenSize.Y);
					break;
				default:
					FOVScreenSize = ScreenSize.X;
					break;
				}

				const float FOV = Cast<APlayerController>(Controller)->PlayerCameraManager->GetFOVAngle();

				//const float Radius = FMath::Sin(FMath::DegreesToRadians(GetWeapon()->GetCombinedSpread()*0.5f)) / FMath::Sin(FMath::DegreesToRadians(FOV)) * FOVScreenSize * 0.5f;
				const float Radius = 2;
				const int32 NumPoints = 32;

				for (int32 i = 0; i < NumPoints; i++)
				{
					const float Angle1 = (2.f * PI / (float)NumPoints)*i;
					const float Angle2 = (2.f * PI / (float)NumPoints)*(i + 1);
					const FVector2D PointA(Radius*FMath::Cos(Angle1), Radius*FMath::Sin(Angle1));
					const FVector2D PointB(Radius*FMath::Cos(Angle2), Radius*FMath::Sin(Angle2));

					FCanvasLineItem Line(Center + PointA, Center + PointB);
					Line.LineThickness = 1.f;
					Line.SetColor(FLinearColor::Red);
					Canvas->DrawItem(Line);
				}
				FCanvasLineItem Line1(ScreenCenter + FVector2D(7.f, 0.f), ScreenCenter - FVector2D(7.f, 0.f));
				Line1.LineThickness = 1.f;
				Line1.SetColor(FLinearColor::Red);
				Canvas->DrawItem(Line1);
				FCanvasLineItem Line2(ScreenCenter + FVector2D(0.f, 7.f), ScreenCenter - FVector2D(0.f, 7.f));
				Line2.LineThickness = 1.f;
				Line2.SetColor(FLinearColor::Red);
				Canvas->DrawItem(Line2);
				//FCanvas
			}

			Canvas->SetDrawColor(OldColor);
		}

		// Print Inventory
		YL = Canvas->DrawText(RenderFont, FString(TEXT("Inventory:")), Indent, YPos, 1.f, 1.f, FontRenderInfo);
		YPos += YL;

		YL = Canvas->DrawText(RenderFont, FString::Printf(TEXT("1. %s"), GetInventory()->PrimaryWeapon ? *GetInventory()->PrimaryWeapon->GetClass()->GetName() : TEXT("NULL")), Indent + 8.f, YPos, 1.f, 1.f, FontRenderInfo);
		YPos += YL;		
		YL = Canvas->DrawText(RenderFont, FString::Printf(TEXT("2. %s"), GetInventory()->Sidearm ? *GetInventory()->Sidearm->GetClass()->GetName() : TEXT("NULL")), Indent + 8.f, YPos, 1.f, 1.f, FontRenderInfo);
		YPos += YL;
		//YL = Canvas->DrawText(RenderFont, FString::Printf(TEXT("3. %s"), GetInventory()->Grenade ? *GetInventory()->Grenade->GetClass()->GetName() : TEXT("NULL")), Indent + 8.f, YPos, 1.f, 1.f, FontRenderInfo);
		//YPos += YL;
		YL = Canvas->DrawText(RenderFont, FString::Printf(TEXT("3. %s"), GetInventory()->Gadget1 ? *GetInventory()->Gadget1->GetClass()->GetName() : TEXT("NULL")), Indent + 8.f, YPos, 1.f, 1.f, FontRenderInfo);
		YPos += YL;
		YL = Canvas->DrawText(RenderFont, FString::Printf(TEXT("4. %s"), GetInventory()->Gadget2 ? *GetInventory()->Gadget2->GetClass()->GetName() : TEXT("NULL")), Indent + 8.f, YPos, 1.f, 1.f, FontRenderInfo);
		YPos += YL;
		//YL = Canvas->DrawText(RenderFont, FString::Printf(TEXT("4. %s"), Equipment2 ? *Equipment2->GetClass()->GetName() : TEXT("NULL")), Indent, YPos);
		//YPos += YL;

	}


	static FName NAME_Hearing = FName(TEXT("Hearing"));
	if (DebugDisplay.IsDisplayOn(NAME_Hearing))
	{
		const float Speed = GetVelocity().Size2D();
		const float Loudness = FMath::Clamp(Speed * FootStepNoiseSpeedFactor, 0.f, 1.f);
		YPos += YL;
		YL = Canvas->DrawText(RenderFont, FString::Printf(TEXT("Loudness: %f"), Loudness), Indent, YPos, 1.f, 1.f, FontRenderInfo);
		YPos += YL;
		DrawDebugSphere(GetWorld(), GetActorLocation(), Loudness * FootStepNoiseMaxRange, 64, FColor::Yellow, false, -1.f, 100, Loudness * 3.f);
	}

	Canvas->SetDrawColor(DefaultDrawColor);
}

void ATacticalCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const bool bPlayerControlled = GetController() && Cast<APlayerController>(GetController());

	if (IsAlive())
	{
		if (Role == ROLE_Authority || Role == ROLE_AutonomousProxy)
			TimeCheckWeaponObstruction();

		float DeltaYaw = FRotator::NormalizeAxis(LastViewRotation.Yaw - GetControlRotation().Yaw) / DeltaSeconds;
		float DeltaPitch = FRotator::NormalizeAxis(LastViewRotation.Pitch - GetControlRotation().Pitch) / DeltaSeconds;
		float DeltaRoll = FRotator::NormalizeAxis(LastViewRotation.Roll - GetControlRotation().Roll) / DeltaSeconds;
		ViewRotationSpeed = FRotator(DeltaPitch, DeltaYaw, DeltaRoll);

		LastViewRotation = GetViewRotation();


		//if (GetTacticalMovment() && GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() != TargetCapsuleSize)
		//{
		//	GetTacticalMovment()->ChangeCollisionToSize(TargetCapsuleSize);
		//}
		
		const float ADSSpeed = GetWeapon() ? GetWeapon()->GetADSSpeed() : 10.f;
		const float TargetADSRatio = IsAiming() ? 1.f : 0.f;
		ADSState = FMath::FInterpTo(ADSState, TargetADSRatio, DeltaSeconds, ADSSpeed);
		if(GetWeapon())
		{
			WeaponHandOffset = FMath::Lerp<FVector>(BaseWeaponHandOffset_Default + GetWeapon()->GetWeaponOffset(false), BaseWeaponHandOffset_Aim + GetWeapon()->GetWeaponOffset(true), ADSState);
		}



		//if (IsAiming() && !CanAim())
		//{
		//	UE_LOG(LogTemp, Log, TEXT("Abort Aim"));
		//	bIsAiming = false;
		//}

		if (Role == ROLE_Authority || IsLocallyControlled())
		{
			if (bPlayerControlled && IsLocallyControlled())
			{
				const float Speed = GetVelocity().Size2D();
				const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
				WeaponSway.X = 2 * WeaponSwayX.Sample(TimeSeconds) - 1.f;
				WeaponSway.Y = 2 * WeaponSwayY.Sample(TimeSeconds) - 1.f;
				

				const float ShakeFreq = WeaponShakeFrequencyCurve.GetRichCurveConst()->Eval(Speed);
				const float ShakeAmplitude = WeaponShakeAmplitudeCurve.GetRichCurveConst()->Eval(Speed);

				WeaponSway.X += ShakeAmplitude * FMath::Sin(ShakeFreq*TimeSeconds);
				WeaponSway.Y += ShakeAmplitude * FMath::Sin(2*ShakeFreq*TimeSeconds);
			}

			// Scan for interact actors (only local player!)
			/*if(CanInteract())*/
			if(bPlayerControlled && IsLocallyControlled())
			{
				AActor* NewInteractActor = nullptr;
				if (ATacticalVaultMarker* BestMarker = GetBestVaultMarker())
				{
					NewInteractActor = BestMarker;
				}
				else
				{
					NewInteractActor = InteractionTrace();
				}
				
				if (NewInteractActor != InteractActor)
				{
					if (InteractActor && InteractActor->GetClass()->ImplementsInterface(UBRSCharacterInteractionInterface::StaticClass()))
					{
						IBRSCharacterInteractionInterface::Execute_OnStopHighlight(InteractActor, this);
					}
					if (NewInteractActor && NewInteractActor->GetClass()->ImplementsInterface(UBRSCharacterInteractionInterface::StaticClass()))
					{
						IBRSCharacterInteractionInterface::Execute_OnStartHighlight(NewInteractActor, this);
					}
					InteractActor = NewInteractActor;
				}
			}

			if (RecoilToDo != FVector2D::ZeroVector)
			{
				FRotator NewRot = GetControlRotation();
				FVector2D RecoilToAdd = FMath::Vector2DInterpTo(FVector2D::ZeroVector, RecoilToDo, DeltaSeconds, RecoilSpeed);
				NewRot.Pitch += RecoilToAdd.X;
				NewRot.Yaw += RecoilToAdd.Y;
				Controller->SetControlRotation(NewRot);
				RecoilToDo -= RecoilToAdd;
				if (RecoilToDo.SizeSquared() == KINDA_SMALL_NUMBER)
				{
					RecoilToDo = FVector2D::ZeroVector;
				}
			}
		}

		if (GetRemoteRole() == ROLE_AutonomousProxy)
		{
			ServerSendLeanAngle(LeanAngle);
		}

		TickFootIK(DeltaSeconds);

		if (Role == ROLE_Authority)
		{
			// make noise
			const float Speed = GetVelocity().Size2D();
			if (Speed > 10.f)
			{
				const float Loudness = FMath::Clamp(Speed * FootStepNoiseSpeedFactor, 0.f, 1.f);
				//NoiseEmitter->MakeNoise(this, Loudness, GetActorLocation());
				UAISense_Hearing::ReportNoiseEvent(this, GetActorLocation(), Loudness, this, FootStepNoiseMaxRange, FName("Footstep"));
			}
		}

	}
	else if (!bFullyRagdoll)
	{
		USkeletalMeshComponent* UseMesh = GetMesh();
		if (UseMesh)
		{
			const float PhysicsBlend = UseMesh->GetAnimInstance()->GetCurveValue(FName(TEXT("PhysicsWeight"))) > 0.5f ? 1.f : 0.f;
			GetMesh()->SetPhysicsBlendWeight(PhysicsBlend);
			if (GetWeapon() && GetWeapon()->GetMesh())
			{
				GetWeapon()->GetMesh()->SetSimulatePhysics(true);
				GetWeapon()->GetMesh()->SetAllBodiesSimulatePhysics(true);
				GetWeapon()->GetMesh()->WakeAllRigidBodies();
				GetWeapon()->GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
				GetWeapon()->GetMesh()->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
			}
			if (PhysicsBlend >= 1.f)
			{
				bFullyRagdoll = true;
			}
		}
	}
}


void ATacticalCharacter::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	IGenericTeamAgentInterface::SetGenericTeamId(TeamID);
}

FGenericTeamId ATacticalCharacter::GetGenericTeamId() const
{
	if (PlayerState)
	{
		IGenericTeamAgentInterface* TeamPlayerState = Cast<IGenericTeamAgentInterface>(PlayerState);
		if (TeamPlayerState)
		{
			return TeamPlayerState->GetGenericTeamId();
		}
	}
	else if (GetController())
	{
		IGenericTeamAgentInterface* TeamController = Cast<IGenericTeamAgentInterface>(GetController());
		if (TeamController)
		{
			return TeamController->GetGenericTeamId();
		}
	}
	return IGenericTeamAgentInterface::GetGenericTeamId();
}

ETeamAttitude::Type ATacticalCharacter::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (PlayerState)
	{
		IGenericTeamAgentInterface* TeamPlayerState = Cast<IGenericTeamAgentInterface>(PlayerState); 
		if (TeamPlayerState)
		{
			return TeamPlayerState->GetTeamAttitudeTowards(Other);
		}
	} 
	else if (GetController())
	{
		IGenericTeamAgentInterface* TeamController = Cast<IGenericTeamAgentInterface>(GetController());
		if (TeamController)
		{
			return TeamController->GetTeamAttitudeTowards(Other);
		}
	}
	return IGenericTeamAgentInterface::GetTeamAttitudeTowards(Other);
}



float ATacticalCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Damage;

	if (GetWorld() && GetWorld()->GetAuthGameMode())
	{
		ATacticalGameModeBase* GM = Cast<ATacticalGameModeBase>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			ActualDamage = GM->GetModifiedDamage(Damage, EventInstigator, DamageCauser, this);
		}
	}

	if (DamageCauser)
	{
		// Report Damage Event to AI System
		UAISense_Damage::ReportDamageEvent(GetWorld(), this, EventInstigator ? EventInstigator->GetPawn() : nullptr, ActualDamage, DamageCauser->GetActorLocation(), GetActorLocation());
		// Report Team Event to AI System
		if (UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(GetWorld()))
		{
			FAITeamStimulusEvent TeamNotifyDamageEvent(this, DamageCauser, DamageCauser->GetActorLocation(), TeamNotifyDamageRadius, 0.f, 1.f);
			TeamNotifyDamageEvent.TeamIdentifier = FGenericTeamId(GetGenericTeamId());
			PerceptionSystem->OnEvent(TeamNotifyDamageEvent);
		}
	}

	if ((DamageEvent.DamageTypeClass != nullptr) && (DamageEvent.DamageTypeClass->IsChildOf(UTacticalDamageType::StaticClass())))
	{
		const UTacticalDamageType* DMGCDO = DamageEvent.DamageTypeClass->GetDefaultObject<UTacticalDamageType>();
		// Head shot
		if (DamageEvent.IsOfType(FPointDamageEvent::ClassID) && DMGCDO != nullptr)
		{
			const FPointDamageEvent* PointDamage = (const FPointDamageEvent*)(&DamageEvent);
			const FName HeadBoneName(TEXT("head"));
			//UE_LOG(LogTemp, Log, TEXT("Hit: %s"), *PointDamage->HitInfo.BoneName.ToString());
			if (PointDamage->HitInfo.BoneName.Compare(HeadBoneName) == 0)
			{
				ActualDamage *= DMGCDO->HeadshotDamageModifier;
			}
		}
		PerformHitReaction(DMGCDO);
	}


	ActualDamage = Super::TakeDamage(ActualDamage, DamageEvent, EventInstigator, DamageCauser);

	return ActualDamage;
}

// Overrides from APawn/ACharacter
void ATacticalCharacter::FaceRotation(FRotator NewControlRotation, float DeltaTime)
{
	// Only if we actually are going to use any component of rotation.
	if (bUseControllerRotationPitch || bUseControllerRotationYaw || bUseControllerRotationRoll)
	{
		const FRotator CurrentRotation = GetActorRotation();
		if (bIsVaulting && CurrentVaultMarker)
		{
			const float TargetYaw = FRotator::NormalizeAxis(CurrentVaultMarker->GetActorRotation().Yaw + (bVaultForward ? 180.f : 0.f));
			const float NewYaw = FMath::FInterpTo(FRotator::NormalizeAxis(CurrentRotation.Yaw), TargetYaw, DeltaTime, 8.f);
			const FRotator TargetRot = FRotator(NewControlRotation.Pitch, CurrentVaultMarker->GetActorRotation().Yaw + (bVaultForward ? 180.f : 0.f), NewControlRotation.Roll);

			NewControlRotation = FMath::RInterpTo(NewControlRotation.GetNormalized(), TargetRot.GetNormalized(), DeltaTime, 8.f);

			FRotator NewActorRotation = FMath::RInterpTo(CurrentRotation.GetNormalized(), TargetRot.GetNormalized(), DeltaTime, 8.f);
			NewActorRotation.Roll = CurrentRotation.Roll;
			NewActorRotation.Pitch = CurrentRotation.Pitch;

			Controller->SetControlRotation(NewControlRotation);

			SetActorRotation(NewActorRotation);
			return;
		}
		else if (ATacticalCoverMarker* UseCover = GetBestCover())
		{
			// Keep Pawn rotated to cover direction
			const float TargetYaw = FRotator::NormalizeAxis(UseCover->GetDesiredRotation().Yaw);
			const float NewActorYaw = FMath::FInterpTo(FRotator::NormalizeAxis(CurrentRotation.Yaw), TargetYaw, DeltaTime, 5.f);
			SetActorRotation(FRotator(0.f, NewActorYaw, 0.f));
			return;

			//const float TargetYaw = 
		}
		else if (GetTacticalMovement())
		{
			// Limit Rotation for prone characters
			if (GetStance() == ETacticalStance::STANCE_Prone)
			{
				float MaxYawDelta = ((GetVelocity().SizeSquared2D() > 10.f) ? 20.f : 0.f);
				FRotator ModifiedControlRotation = NewControlRotation;
				if (Controller)
				{
					if (FRotator::NormalizeAxis(NewControlRotation.Yaw - CurrentRotation.Yaw) >= (45.f + MaxYawDelta * DeltaTime))
					{
						ModifiedControlRotation.Yaw = CurrentRotation.Yaw + (45.f + MaxYawDelta * DeltaTime);
					}
					else if (FRotator::NormalizeAxis(NewControlRotation.Yaw - CurrentRotation.Yaw) <= -(45.f + MaxYawDelta * DeltaTime))
					{
						ModifiedControlRotation.Yaw = CurrentRotation.Yaw - (45.f + MaxYawDelta * DeltaTime);
					}

					if (FRotator::NormalizeAxis(NewControlRotation.Pitch - CurrentRotation.Pitch) >= 45.f)
					{
						ModifiedControlRotation.Pitch = CurrentRotation.Pitch + 45.f;
					}
					else if (FRotator::NormalizeAxis(NewControlRotation.Pitch - CurrentRotation.Pitch) <= -45.f)
					{
						ModifiedControlRotation.Pitch = CurrentRotation.Pitch - 45.f;
					}
					Controller->SetControlRotation(ModifiedControlRotation);
				}
				NewControlRotation = MaxYawDelta > 0.f ? FMath::RInterpConstantTo(CurrentRotation, ModifiedControlRotation, DeltaTime, MaxYawDelta) : CurrentRotation;
			}
			else
			{
				// Standing or Crouching. Have a dead zone. Rotate if Delta is outside of it. If Delta is way to high rotate to the maximum allowed Delta
				const FRotator DeltaRot = (NewControlRotation - CurrentRotation).GetNormalized();
				const float DeltaYaw = FMath::Abs(DeltaRot.Yaw);
				const bool bRHS = DeltaRot.Yaw > 0.f;

				if (DeltaYaw < 90.f)
				{
					// Check if Rotation should be started
					if (bForceRotation || ((RotationType == ETacticalRotationType::None) && DeltaYaw > 45.f))
					{
						bForceRotation = false;
						RotationType = bRHS ? ETacticalRotationType::Right : ETacticalRotationType::Left;
					}
					else if ((RotationType != ETacticalRotationType::None) && DeltaYaw < 1.f)
					{
						RotationType = ETacticalRotationType::None;
					}

					// Perform the Rotation
					if ((RotationType != ETacticalRotationType::None) || (GetTacticalMovement()->Velocity.SizeSquared2D() > 100))
					{
						// Rotate at speed
						NewControlRotation = FMath::RInterpConstantTo(CurrentRotation, FRotator(0.f, NewControlRotation.Yaw, 0.f), DeltaTime, 140.f);
					}
					else
					{
						// Should not Rotate
						NewControlRotation = CurrentRotation;
					}
				}
				else
				{
					// Delta is so High that we need to Rotate to max offset
					RotationType = bRHS ? ETacticalRotationType::Right : ETacticalRotationType::Left;
					NewControlRotation.Yaw = NewControlRotation.Yaw + (bRHS ? -89.f : 89.f);
				}
			}
		}


		if (!bUseControllerRotationPitch)
		{
			NewControlRotation.Pitch = CurrentRotation.Pitch;
		}

		if (!bUseControllerRotationYaw)
		{
			NewControlRotation.Yaw = CurrentRotation.Yaw;
		}

		if (!bUseControllerRotationRoll)
		{
			NewControlRotation.Roll = CurrentRotation.Roll;
		}

		SetActorRotation(NewControlRotation);
	}
}

FRotator ATacticalCharacter::GetBaseAimRotation() const
{
	// If we have a controller, by default we aim at the player's 'eyes' direction
	// that is by default Controller.Rotation for AI, and camera (crosshair) rotation for human players.
	FVector POVLoc;
	FRotator POVRot;
	if (Controller != NULL /*&& !InFreeCam()*/)
	{
		//Controller->GetPlayerViewPoint(POVLoc, POVRot);
		return GetControlRotation();
	}

	// If we have no controller, we simply use our rotation
	POVRot = GetActorRotation();

	// If our Pitch is 0, then use RemoteViewPitch
	if (FMath::IsNearlyZero(POVRot.Pitch))
	{
		POVRot.Pitch = RemoteViewPitch;
		POVRot.Pitch = POVRot.Pitch * 360.f / 255.f;
	}
	// todo: we could make this even better if Remote View Yaw is just the deviation from the actor rotation
	POVRot.Yaw = RemoteViewYaw;
	POVRot.Yaw = POVRot.Yaw * 360.f / 255.f;

	return POVRot;
}

void ATacticalCharacter::Landed(const FHitResult& Hit)
{
	const float FallingVelocityZ = GetVelocity().Z;

	if (FallingVelocityZ < MinSafeFallSpeedZ)
	{
		const float FallDamage = (MinSafeFallSpeedZ - FallingVelocityZ) / (MinSafeFallSpeedZ - LethalFallSpeedZ) * 100.f;
		// maybe delay it for a tick (todo)
		TakeDamage(FallDamage, FDamageEvent(UDamageType::StaticClass()), GetController(), this);
	}
}



void ATacticalCharacter::CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult)
{
	bool bUseFirstPersonCamera = !bThirdPersonView;

	if (bUseFirstPersonCamera && GetFirstPersonCamera() && GetFirstPersonCamera()->IsActive())
	{
		GetFirstPersonCamera()->GetCameraView(DeltaTime, OutResult);
		return;
	}
	else if (GetThirdPersonCamera() && GetThirdPersonCamera()->IsActive())
	{
		GetThirdPersonCamera()->GetCameraView(DeltaTime, OutResult);
		return;
	}

	Super::CalcCamera(DeltaTime, OutResult);
}

void ATacticalCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (NewController)
	{
		if (Cast<AAIController>(NewController))
		{
			UE_LOG(LogTemp, Log, TEXT("AI CONTROLLED"));
			bIsAIControlled = true;
		}
	}
	bIsAIControlled = false;
}

void ATacticalCharacter::GetActorEyesViewPoint(FVector& Location, FRotator& Rotation) const
{
	Rotation = GetControlRotation();
	Location = GetFirstPersonCamera()->GetComponentLocation();

	// in low cover allow more
	if (IsInLowCover())
	{
		Location.Z += 30.f;
	}
}

void ATacticalCharacter::OnFirstPerson()
{
	if (GetWeapon())
	{
		GetWeapon()->OnFirstPerson();
	}
}

void ATacticalCharacter::OnThirdPerson()
{
	if (GetWeapon())
	{
		GetWeapon()->OnThirdPerson();
	}
}

void ATacticalCharacter::SetRemoteViewYaw(float inYaw)
{
	// Compress pitch to 1 byte
	inYaw = FRotator::ClampAxis(inYaw);
	RemoteViewYaw = (uint8)(inYaw * 255.f / 360.f);
}


////////////////////
// Health

bool ATacticalCharacter::IsAlive() const
{
	return (GetHealthComponent() ? GetHealthComponent()->IsAlive() : true);
}

////////////////////
// Movement

class UTacticalCharacterMovement* ATacticalCharacter::GetTacticalMovement() const
{
	return (GetCharacterMovement() ? Cast<UTacticalCharacterMovement>(GetCharacterMovement()) : nullptr);
}

float ATacticalCharacter::GetLeanAngle() const
{
	return LeanAngle;
}

FRotator ATacticalCharacter::GetViewRotationSpeed()
{
	return ViewRotationSpeed;
}

class ATacticalWeapon* ATacticalCharacter::GetWeapon() const
{
	return (GetInventory() ? GetInventory()->GetWeapon() : nullptr);
}

ETacticalStance ATacticalCharacter::GetStance() const
{
	if (GetTacticalMovement())
	{
		return GetTacticalMovement()->GetCurrentStance();
	}
	return ETacticalStance::STANCE_Default;
}

void ATacticalCharacter::PerformedMovement()
{
	const float WorldTime = GetWorld()->GetTimeSeconds();

	// Save Movement
	SavedPositions.Add(FSavedPosition(WorldTime, GetActorLocation(), GetCharacterMovement()->Velocity, GetViewRotation(), GetCharacterMovement()->bJustTeleported, false));

	// Adjust size to limit of how many moves need to be saved
	if ((SavedPositions.Num() > 1) && (SavedPositions[1].Time < (WorldTime - MaxPositionSaveTime)))
	{
		SavedPositions.RemoveAt(0);
	}

	// Log to check how many are saved // todo: probably better to move to DebugPawn
	// UE_LOG(LogTemp, Log, TEXT("Saved Position (Size: %d)"), SavedPositions.Num());
}

FVector ATacticalCharacter::GetRewindPosition(float inTime /*= -1.f*/) const
{
	float TargetWorldTime = GetWorld()->GetTimeSeconds() - inTime;
	FVector RewindLoc = GetActorLocation();
	// iterate though all saved times beginning with newest
	if (SavedPositions.Num() > 0)
	{
		for (int32 i = SavedPositions.Num() - 1; i >= 0; i--)
		{
			RewindLoc = SavedPositions[i].Position;
			// if Saved Time is smaller than target time we have the newest sample that is older than TargetTime
			if (SavedPositions[i].Time < TargetWorldTime)
			{
				// check if interpolation is necessary/possible
				// don't interpolate on teleport or is sample is missing
				// todo: review code regarding teleportation
				if (SavedPositions.IsValidIndex(i + 1) && !SavedPositions[i + 1].bJustTeleported)
				{
					// Linear Interpolation
					float InterpPercent = (SavedPositions[i + 1].Time == SavedPositions[i].Time) ? 1.f
						: (TargetWorldTime - SavedPositions[i].Time) / (SavedPositions[i + 1].Time - SavedPositions[i].Time);
					RewindLoc += InterpPercent * (SavedPositions[i + 1].Position - SavedPositions[i].Position);
				}
				break;
			}
		}
	}
	return RewindLoc;
}


////////////////////
// Animation

bool ATacticalCharacter::UseWeaponAimIK() const
{
	FRotator DeltaRot = (GetControlRotation() - GetActorRotation()).GetNormalized();

	if (!CanFire())
		return false;

	if (GetTacticalMovement())
	{
		if (GetTacticalMovement()->GetCurrentStance() == ETacticalStance::STANCE_Prone)
		{
			if (DeltaRot.Pitch < -20.f || DeltaRot.Pitch > 46.f)
				return false;
		}
	}

	return true;
}

void ATacticalCharacter::PlayEquipAnimation(float Time)
{
	UAnimMontage* Anim = EquipAnimations.GetMontage(GetWeapon() ? GetWeapon()->GetWeaponType() : EWeaponType::None);
	if (Anim != nullptr)
	{
		const float MontageDuration = Anim->GetPlayLength();
		const float Rate = (Time > 0.f) ? MontageDuration / Time : 1.f;
		PlayAnimMontage(Anim, Rate);
	}
}

void ATacticalCharacter::PlayUnequipAnimation(float Time)
{
	UAnimMontage* Anim = UneqipAnimations.GetMontage(GetWeapon() ? GetWeapon()->GetWeaponType() : EWeaponType::None);
	if (Anim != nullptr)
	{
		// Request longer time to have a better transition
		Time += 0.1f;
		const float MontageDuration = Anim->GetPlayLength();
		const float Rate = (Time > 0.f) ? MontageDuration / Time : 1.f;
		PlayAnimMontage(Anim, Rate);
	}
}

void ATacticalCharacter::PlayReloadAnimation(float Time)
{
	if (GetWeapon())
	{
		//UAnimMontage* Anim = (GetStance() == ETacticalStance::STANCE_Prone) ? GetWeapon()->ReloadAnimationProne :
		//	(GetStance() == ETacticalStance::STANCE_Crouched) ? GetWeapon()->ReloadAnimationCrouched : GetWeapon()->ReloadAnimationDefault;
		//if (Anim)
		//{
		//	const float MontageDuration = Anim->GetSectionLength(0);
		//	const float Rate = (Time > 0.f) ? MontageDuration / Time : 1.f;
		//	PlayAnimMontage(Anim, Rate);

		if(!IsThirdPerson())
		{
			if (GetWeapon()->GetFPReloadAnim() && GetFirstPersonMesh() && GetFirstPersonMesh()->GetAnimInstance())
			{
				if (GetWeapon()->GetCurrentState()->IsA(UTacticalWeaponStateGadget::StaticClass()))
				{
					ATacticalWeaponAttachment_Weapon* AttachWPN = Cast<UTacticalWeaponStateGadget>(GetWeapon()->GetCurrentState())->GetAttachment();
					const float FPTime = AttachWPN->WeaponReloadAnim->GetSectionLength(0);
					const float FPRate = (Time > 0.f) ? FPTime / Time : 1.f;
					GetFirstPersonMesh()->GetAnimInstance()->Montage_Play(AttachWPN->FPReloadAnim, FPRate);
				}
				else
				{
					const float FPTime = GetWeapon()->GetReloadAnim()->GetSectionLength(0);
					const float FPRate = (Time > 0.f) ? FPTime / Time : 1.f;
					GetFirstPersonMesh()->GetAnimInstance()->Montage_Play(GetWeapon()->GetFPReloadAnim(), FPRate);
				}
			}
		}
		else
		{
			UAnimMontage* DefaultMontage = GetWeapon()->ReloadAnimationDefault;
			switch (GetStance())
			{
			case ETacticalStance::STANCE_Prone:
				DefaultMontage = GetWeapon()->ReloadAnimationProne;
			case ETacticalStance::STANCE_Crouched:
				DefaultMontage = GetWeapon()->ReloadAnimationCrouched;
			case ETacticalStance::STANCE_Default:
			default:
				break;
			}


			if(DefaultMontage)
			{
				const float FPTime = DefaultMontage->GetSectionLength(0);
				const float FPRate = (Time > 0.f) ? FPTime / Time : 1.f;
				PlayAnimMontage(DefaultMontage, FPRate);
			}
		}

	}
}

void ATacticalCharacter::PlayStopReloadAnimation(float Time)
{
	if (GetWeapon())
	{
		//UAnimMontage* Anim = (GetStance() == ETacticalStance::STANCE_Prone) ? GetWeapon()->ReloadAnimationProne :
		//	(GetStance() == ETacticalStance::STANCE_Crouched) ? GetWeapon()->ReloadAnimationCrouched : GetWeapon()->ReloadAnimationDefault;
		//if (Anim)
		//{
			//const float MontageDuration = Anim->GetSectionLength(0);
			//const float Rate = (Time > 0.f) ? MontageDuration / Time : 1.f;
			//PlayAnimMontage(Anim, Rate);

		if (GetWeapon()->GetFPReloadAnim() && GetFirstPersonMesh() && GetFirstPersonMesh()->GetAnimInstance())
		{
			const float FPTime = GetWeapon()->GetReloadAnim()->GetSectionLength(2);
			const float FPRate = (Time > 0.f) ? FPTime / Time : 1.f;
			GetFirstPersonMesh()->GetAnimInstance()->Montage_JumpToSection(FName("End"), GetWeapon()->FPReloadAnim);
		}
		
	}
}

void ATacticalCharacter::GetDesiredFootLocations(FVector& RightFoot, FVector& LeftFoot, FVector& RightFootNormal, FVector& LeftFootNormal) const
{
	RightFoot = RightFootLoc;
	LeftFoot = LeftFootLoc;
	RightFootNormal = RightFootNorm;
	LeftFootNormal = LeftFootNorm;
}


////////////////////
// Weapon/Combat

bool ATacticalCharacter::CanFire() const
{
	if (!IsAlive()
		|| (!GetTacticalMovement()) 
		|| (!GetTacticalMovement()->IsMovingOnGround())
		|| (GetTacticalMovement()->IsSprinting())
		|| (GetTacticalMovement()->IsSprinting())
		|| ((GetTacticalMovement()->GetCurrentStance() == ETacticalStance::STANCE_Prone) && (GetTacticalMovement()->Velocity.SizeSquared() > 100.f))
		|| (GetWorldTimerManager().IsTimerActive(GetTacticalMovement()->ProneTransitionTimer))
		|| IsVaulting()
		|| (IsInCover() && CoverAimType==ECoverAimType::None)
		|| bIsStunned
		)
	{
		return false;
	}
	return true;
}

bool ATacticalCharacter::IsAiming() const
{
	return bIsAiming && CanAim();
}

bool ATacticalCharacter::CanAim() const
{
	return CanFire() && !(GetTacticalMovement() && GetTacticalMovement()->IsSprinting()) && GetWeapon() && !GetWeapon()->IsReloading() && !GetWeapon()->IsChangingWeapon() && !IsWeaponObstructed();
}

bool ATacticalCharacter::CanReload() const
{
	if (
		(!GetTacticalMovement())
		|| (!GetTacticalMovement()->IsMovingOnGround())
		|| (GetTacticalMovement()->IsSprinting())
		|| (GetTacticalMovement()->IsSprinting())
		|| (GetWorldTimerManager().IsTimerActive(GetTacticalMovement()->ProneTransitionTimer))
		|| IsVaulting()
		|| IsWeaponObstructed()
		)
	{
		return false;
	}
	return true;
}

FVector ATacticalCharacter::GetWeaponHandOffset() const
{
	return WeaponHandOffset;
}

FVector ATacticalCharacter::GetWeaponTraceOrigin() const
{
	return GetFirstPersonCamera()->GetComponentLocation();
}

void ATacticalCharacter::AddRecoil(FVector2D Recoil)
{
	RecoilToDo += Recoil;
}

bool ATacticalCharacter::IsFiring() const
{
	return (GetWeapon() && GetWeapon()->IsFiring());
}

bool ATacticalCharacter::IsChangingWeapon() const
{
	if (GetWeapon())
	{
		return GetWeapon()->IsChangingWeapon();
	}
	return false;
}

bool ATacticalCharacter::IsReloading() const
{
	if (GetWeapon())
	{
		return GetWeapon()->IsReloading();
	}
	return false;
}

bool ATacticalCharacter::IsThrowingGrenade() const
{
	return bThrowingGrenade || GetWorldTimerManager().IsTimerActive(TimerGrenadeCooldown);
}
bool ATacticalCharacter::ShowCrosshair() const
{
	return (GetWeapon() && CanFire() && GetWeapon()->ShowCrosshair());
}

bool ATacticalCharacter::WeaponObstructionCheck() const
{
	if (!GetWeapon() || !GetFirstPersonCamera())
		return false;

	const float SweepDistance = GetWeapon()->GetObstructionSweepDistance();
	const FVector SweepExtent = FVector(SweepDistance, ObstructionTesterExtent.Y, ObstructionTesterExtent.Z);

	const FVector Start = GetFirstPersonCamera()->GetComponentTransform().TransformPosition(FVector(SweepDistance, 7.f, -17.f));
	const FVector End = Start + GetFirstPersonCamera()->GetForwardVector() * 0.1f;
	FCollisionQueryParams TraceParams = FCollisionQueryParams(TEXT("WeaponObstructionSweep"), false, this);
	if (GetWeapon())
		TraceParams.AddIgnoredActor(GetWeapon());
	FCollisionShape Shape = FCollisionShape::MakeBox(SweepExtent);

	const bool bHit = GetWorld()->SweepTestByChannel(Start, End, GetFirstPersonCamera()->GetComponentRotation().Quaternion(), TRACE_WeaponObstruction, Shape, TraceParams);



	// todo: Save Distance -> IK control that pulls gun back

	return bHit;
}

bool ATacticalCharacter::IsWeaponObstructed() const
{
	return bIsWeaponObstructed;
}

void ATacticalCharacter::TimeCheckWeaponObstruction()
{
	bool Result = WeaponObstructionCheck();
	if (Result)
	{
		const float SweepDistance = GetWeapon()->GetObstructionSweepDistance();
		const FVector Start = GetFirstPersonCamera()->GetComponentTransform().TransformPosition(FVector(SweepDistance, 7.f, -17.f));
		FCollisionQueryParams TraceParams = FCollisionQueryParams(TEXT("WeaponMoveBackTrace"), false, this);
		if (GetWeapon())
			TraceParams.AddIgnoredActor(GetWeapon());
		FHitResult Hit(ForceInit);
		const FVector TraceStart = Start - GetFirstPersonCamera()->GetForwardVector() * SweepDistance;
		const FVector TraceEnd = Start + GetFirstPersonCamera()->GetForwardVector() * SweepDistance;

		bool bDidHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, TRACE_WeaponObstruction, TraceParams);

		if (bDidHit)
		{
			WeaponMoveBackDistance = (TraceEnd - Hit.ImpactPoint).Size();
			bMoveWeaponBack = GetWeapon()->CanMoveBack(WeaponMoveBackDistance);
		}
		else
		{
			bMoveWeaponBack = GetWeapon()->CanMoveBack(0.f);
			WeaponMoveBackDistance = 0.f;
		}

		//DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 10.f, FColor::Red, true, .1f);
	}
	else
	{
		bMoveWeaponBack = false;
		WeaponMoveBackDistance = 0.f;
	}

	if (Result != bIsWeaponObstructed)
	{
		bIsWeaponObstructed = Result;
	}
}




bool ATacticalCharacter::WantsToCover() const
{
	return bAIWantsToCover;
}


bool ATacticalCharacter::IsInCover(ECoverType CoverFilter) const
{
	if (IsPlayerControlled())
	{
		return false;
	}

	if (!WantsToCover())
		return false;

	//if (CoverAimType != ECoverAimType::None)
	//	return true;

	TArray<ATacticalCoverMarker*> OverlappingCovers;
	GetOverlappingCover(OverlappingCovers, CoverFilter);
	if (OverlappingCovers.Num() > 0)
	{
		return true;
	}
	return false;
}

bool ATacticalCharacter::IsInHighCover() const
{
	return IsInCover(ECoverType::HighCover);
}

bool ATacticalCharacter::IsInLowCover() const
{
	return IsInCover(ECoverType::LowCover);
}

bool ATacticalCharacter::GetOverlappingCover(TArray<class ATacticalCoverMarker*>& OutCovers, ECoverType CoverFilter) const
{
	bool bSuccess = false;
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, ATacticalCoverMarker::StaticClass());
	if (OverlappingActors.Num() > 0)
	{
		for (AActor* TestActor : OverlappingActors)
		{
			if (TestActor)
			{
				ATacticalCoverMarker* TestCover = Cast<ATacticalCoverMarker>(TestActor);
				if (TestCover)
				{
					if(CoverFilter == ECoverType::None || TestCover->GetCoverType() == CoverFilter)
					{
						OutCovers.AddUnique(TestCover);
						bSuccess = true;
					}
				}
			}
		}
	}
	return bSuccess;
}

//void ATacticalCharacter::DumpWeaponInfo()
//{
//	if (GetWeapon())
//	{
//		UE_LOG(LogTemp, Log, TEXT("Weapon: %s"), *GetWeapon()->GetName());
//		TArray<UTacticalWeaponAttachmentPoint*> AttachPoints;
//		GetWeapon()->GetAttachmentPoints(AttachPoints);
//		for (UTacticalWeaponAttachmentPoint* TestAttach : AttachPoints)
//		{
//			if (TestAttach)
//			{
//				UE_LOG(LogTemp, Log, TEXT("AttachPoint: %s"), *TestAttach->GetName());
//
//			}
//		}
//	}
//}

//////////////////////////////////////////////////////////////////////////
// Input

void ATacticalCharacter::SetupPlayerInputComponent(class UInputComponent* inInputComponent)
{
	// Set up gameplay key bindings
	check(inInputComponent);
	//JumpInputBind = InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	//InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	inInputComponent->BindAction("Sprint", IE_Pressed, this, &ATacticalCharacter::StartSprint);
	inInputComponent->BindAction("Sprint", IE_Released, this, &ATacticalCharacter::StopSprint);

	inInputComponent->BindAction("LowerStance", IE_Pressed, this, &ATacticalCharacter::TryLowerStance);
	inInputComponent->BindAction("RaiseStance", IE_Released, this, &ATacticalCharacter::TryRaiseStance);

	//InputComponent->BindAction("Jump", IE_Pressed, this, &ATacticalCharacter::Jump);

	// We have 2 versions of movement bindings to handle different devices.
	// "keymoveforward" and "keymoveright" also takes a set speed modifier into account to enable varying speeds with keys
	// "moveforward" and "moveright" is used to move at a speed determined by the axis value.
	inInputComponent->BindAxis("MoveForward", this, &ATacticalCharacter::KeyMoveForward);
	inInputComponent->BindAxis("MoveRight", this, &ATacticalCharacter::KeyMoveRight);
	inInputComponent->BindAxis("Gamepad_MoveForward", this, &ATacticalCharacter::MoveForward);
	inInputComponent->BindAxis("Gamepad_MoveRight", this, &ATacticalCharacter::MoveRight);


	inInputComponent->BindAction("SpeedUp", IE_Pressed, this, &ATacticalCharacter::SpeedUp);
	inInputComponent->BindAction("SpeedDown", IE_Pressed, this, &ATacticalCharacter::SpeedDown);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	inInputComponent->BindAxis("Turn", this, &ATacticalCharacter::AddControllerYawInput);
	inInputComponent->BindAxis("LookUp", this, &ATacticalCharacter::AddControllerPitchInput);
	inInputComponent->BindAxis("TurnRate", this, &ATacticalCharacter::TurnAtRate);
	inInputComponent->BindAxis("LookUpRate", this, &ATacticalCharacter::LookUpAtRate);

	inInputComponent->BindAxis("LeanRight", this, &ATacticalCharacter::LeanRight);
	inInputComponent->BindAction("Toggle_LeanRight", IE_Pressed, this, &ATacticalCharacter::ToggleLeanRight);
	inInputComponent->BindAction("Toggle_Leanleft", IE_Pressed, this, &ATacticalCharacter::ToggleLeanLeft);

	inInputComponent->BindAction("Melee", IE_Pressed, this, &ATacticalCharacter::Melee);

	inInputComponent->BindAction("Fire", IE_Pressed, this, &ATacticalCharacter::StartFire);
	inInputComponent->BindAction("Fire", IE_Released, this, &ATacticalCharacter::StopFire);
	inInputComponent->BindAction("Aim", IE_Pressed, this, &ATacticalCharacter::StartAim);
	inInputComponent->BindAction("Aim", IE_Released, this, &ATacticalCharacter::ReleaseAim);

	inInputComponent->BindAction("Reload", IE_Pressed, this, &ATacticalCharacter::StartReload);
	inInputComponent->BindAction("Reload", IE_Released, this, &ATacticalCharacter::StopReload);

	inInputComponent->BindAction("ChangeFiringMode", IE_Pressed, this, &ATacticalCharacter::ToggleFireMode);

	inInputComponent->BindAction("Weapon1", IE_Pressed, this, &ATacticalCharacter::SwitchToPrimaryWeapon);
	inInputComponent->BindAction("Weapon2", IE_Pressed, this, &ATacticalCharacter::SwitchToSidearm);
	inInputComponent->BindAction("Holster", IE_Pressed, this, &ATacticalCharacter::HolsterWeapon);
	inInputComponent->BindAction("Gadget1", IE_Pressed, this, &ATacticalCharacter::Gadget1_Pressed);
	inInputComponent->BindAction("Gadget1", IE_Released, this, &ATacticalCharacter::Gadget1_Released);
	inInputComponent->BindAction("Gadget2", IE_Pressed, this, &ATacticalCharacter::Gadget2_Pressed);
	inInputComponent->BindAction("Gadget2", IE_Released, this, &ATacticalCharacter::Gadget2_Released);

	inInputComponent->BindAction("WeaponGadget1", IE_Pressed, this, &ATacticalCharacter::ToggleWeaponGadget1); // Toggles Weapon Gadget/Attachment functionality like a flashlight
	inInputComponent->BindAction("WeaponGadget2", IE_Pressed, this, &ATacticalCharacter::ToggleWeaponGadget2); // Toggles Weapon Gadget/Attachment functionality like a flashlight
	inInputComponent->BindAction("ToggleSight", IE_Pressed, this, &ATacticalCharacter::ToggleWeaponSight); // Toggles between two sights or flips a magnifier

	//inInputComponent->BindAction("Grenade", IE_Pressed, this, &ATacticalCharacter::StartGrenadeThrow);
	//inInputComponent->BindAction("Grenade", IE_Released, this, &ATacticalCharacter::FinishThrow);

	inInputComponent->BindAction("Interact", IE_Pressed, this, &ATacticalCharacter::StartInteract);
	inInputComponent->BindAction("Interact", IE_Released, this, &ATacticalCharacter::StopInteract);

	//InputComponent->BindAction("OpenDoor", IE_Pressed, this, &ATacticalCharacter::SlowOpen);
	//InputComponent->BindAction("CloseDoor", IE_Pressed, this, &ATacticalCharacter::SlowClose);

	inInputComponent->BindAction("Crouch", IE_Pressed, this, &ATacticalCharacter::CrouchPressed);
	inInputComponent->BindAction("Crouch", IE_Released, this, &ATacticalCharacter::CrouchReleased);
	inInputComponent->BindAction("Prone", IE_Pressed, this, &ATacticalCharacter::PronePressed);

	// Gamepad specific actions
	inInputComponent->BindAction("Gamepad_Crouch", IE_Pressed, this, &ATacticalCharacter::GamepadCrouch);
	inInputComponent->BindAction("Gamepad_Crouch", IE_Released, this, &ATacticalCharacter::GamepadCrouchReleased);

	inInputComponent->BindAction("SwitchWeapon", IE_Pressed, this, &ATacticalCharacter::GamepadSwitchWeapon);

	inInputComponent->BindAction("SwitchWeaponHand", IE_Pressed, this, &ATacticalCharacter::SwitchWeaponHand);
}

// Turning

void ATacticalCharacter::AddControllerYawInput(float Value)
{
	if (IsAiming())
	{
		const float SensMod = GetDefault<UTacticalGameplaySettings>(UTacticalGameplaySettings::StaticClass())->ADSMouseSensitivityMultiplier.X;
		Value *= SensMod;
	}

	Super::AddControllerYawInput(Value);
}

void ATacticalCharacter::AddControllerPitchInput(float Value)
{
	if (IsAiming())
	{
		const float SensMod = GetDefault<UTacticalGameplaySettings>(UTacticalGameplaySettings::StaticClass())->ADSMouseSensitivityMultiplier.Y;
		Value *= SensMod;
	}

	Super::AddControllerPitchInput(Value);
}

void ATacticalCharacter::TurnAtRate(float Rate)
{
	if (IsAiming())
	{
		const float SensMod = GetDefault<UTacticalGameplaySettings>(UTacticalGameplaySettings::StaticClass())->ADSStickSensitivityMultiplier.X;
		Rate *= SensMod;
	}
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ATacticalCharacter::LookUpAtRate(float Rate)
{
	if (IsAiming())
	{
		const float SensMod = GetDefault<UTacticalGameplaySettings>(UTacticalGameplaySettings::StaticClass())->ADSStickSensitivityMultiplier.Y;
		Rate *= SensMod;
	}
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

// Jump (DEPRECATED?)
void ATacticalCharacter::Jump()
{
	ServerJump();
}

void ATacticalCharacter::ServerJump_Implementation()
{
	if (ATacticalVaultMarker* BestMarker = GetBestVaultMarker())
	{
		Vault(BestMarker);
	}
}


void ATacticalCharacter::SwitchWeaponHand()
{
	ServerSwitchWeaponHand(!bWeaponLeftHanded);
	if(Role != ROLE_Authority)
	{	
		SwitchWeaponHand_Internal(!bWeaponLeftHanded);
	}
}

void ATacticalCharacter::ServerSwitchWeaponHand_Implementation(bool bUseLeftHand)
{
	SwitchWeaponHand_Internal(bUseLeftHand);
}


void ATacticalCharacter::SwitchWeaponHand_Internal(bool bUseLeftHand)
{
	bWeaponDown = true;
	UE_LOG(LogTemp, Log, TEXT("Switch weapon hand"));
	bForceRotation = true;
	bPendingWeaponHandLeft = bUseLeftHand;
	GetWorldTimerManager().SetTimer(WeaponHandSwitchTimer,this, &ATacticalCharacter::PerformSwitchWeaponHand, 0.4f, false);
}

void ATacticalCharacter::PerformSwitchWeaponHand()
{
	UE_LOG(LogTemp, Log, TEXT("Finish Switch Hand: Current: %d, Pending%d"), bWeaponLeftHanded, bPendingWeaponHandLeft);

	bWeaponLeftHanded = bPendingWeaponHandLeft;

	if (GetWeapon())
	{
		GetWeapon()->AttachGunToHand();
	}

	GetWorldTimerManager().SetTimer(WeaponHandSwitchTimer, this, &ATacticalCharacter::FinishSwitchWeaponHand, 0.3f, false);
}

void ATacticalCharacter::FinishSwitchWeaponHand()
{
	bWeaponDown = false;
}

void ATacticalCharacter::NetMultiVault_Implementation(bool bClimb)
{
	if (Role == ROLE_Authority)
		return;

	//if (GetTacticalMovment() && !bClimb)
	//	GetTacticalMovment()->ChangeCollisionToSize(GetTacticalMovment()->CrouchedHalfHeight);

	const float VaultTime = bClimb ? PlayClimbAnimations() : PlayVaultAnimations();

	GetWorldTimerManager().SetTimer(VaultTimer, this, &ATacticalCharacter::VaultEnd, VaultTime* 0.93f, false);
}

// Vault
void ATacticalCharacter::Vault(class ATacticalVaultMarker* Marker)
{
	if (!Marker)
		return;

	if (Role == ROLE_Authority)
	{
		const bool bClimb = Marker->ShouldClimb();

		// todo: switch to smaller collision
		//if(GetTacticalMovment() && !bClimb)
		//	GetTacticalMovment()->ChangeCollisionToSize(GetTacticalMovment()->CrouchedHalfHeight);


		const float VaultTime = bClimb ? PlayClimbAnimations() : PlayVaultAnimations();
		if(VaultTime > 0.f)
		{
			const FVector RelativeLocation = Marker->GetTransform().InverseTransformPositionNoScale(GetActorLocation());
			bVaultForward = RelativeLocation.X >= 0.f;
			CurrentVaultMarker = Marker;
			bIsVaulting = true;


			GetWorldTimerManager().SetTimer(VaultTimer,this, &ATacticalCharacter::VaultEnd, VaultTime* 1.f, false);
			GetCharacterMovement()->SetMovementMode(MOVE_Flying); 
		}
		NetMultiVault(bClimb);
	}
}

float ATacticalCharacter::PlayVaultAnimations_Implementation()
{
	UAnimMontage* Anim = VaultAnimations.GetMontage(GetWeapon() ? GetWeapon()->GetWeaponType() : EWeaponType::None);
	if (Anim)
	{
		return PlayAnimMontage(Anim);
	}
	return 0.f;
}

float ATacticalCharacter::PlayClimbAnimations_Implementation()
{
	UAnimMontage* Anim = ClimbAnimations.GetMontage(GetWeapon() ? GetWeapon()->GetWeaponType() : EWeaponType::None);
	if (Anim)
	{
		return PlayAnimMontage(Anim);
	}
	return 0.f;
}

void ATacticalCharacter::VaultEnd()
{
	bIsVaulting = false;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking); 
	//if(GetTacticalMovment())
	//	GetTacticalMovment()->ChangeCollisionToSize(GetDefault<ATacticalCharacter>(GetClass())->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());

	CurrentVaultMarker = nullptr;
}

bool ATacticalCharacter::CanVault() const
{
	if (GetTacticalMovement())
	{
		if (!GetTacticalMovement()->IsMovingOnGround())
			return false;

		if (GetStance() != ETacticalStance::STANCE_Default)
			return false;

		if (bIsVaulting)
			return false;

		if (IsReloading() || IsChangingWeapon() || IsThrowingGrenade() || !IsAlive())
			return false;

		return true;
	}
	return false;
}

// Movement
void ATacticalCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ATacticalCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}


void ATacticalCharacter::KeyMoveForward(float Value)
{
	bMoveFwd = FMath::Abs(Value) > 0.01f;
	const float SpeedFactor =  GetTacticalMovement()->bWantsToSprint ? 1.f : ((bMoveRight ? 0.7071f : 1.f) * FMath::Clamp(WalkSpeedFactor, 0.25f, 1.f));
	MoveForward(SpeedFactor*Value);
}

void ATacticalCharacter::KeyMoveRight(float Value)
{
	bMoveRight = FMath::Abs(Value) > 0.01f;
	const float SpeedFactor = GetTacticalMovement()->bWantsToSprint ? 1.f : ((bMoveFwd ? 0.7071f : 1.f) * FMath::Clamp(WalkSpeedFactor, 0.25f, 1.f));
	MoveRight(SpeedFactor*Value);
}

// Stance/Crouch/Vault
void ATacticalCharacter::TryStand()
{
	if (!IsReloading() && GetStance() != ETacticalStance::STANCE_Default)
	{
		if (GetStance() == ETacticalStance::STANCE_Prone)
			StopAim();

		StopSprint();
		ChangeStance(ETacticalStance::STANCE_Default);
	}
}

void ATacticalCharacter::TryCrouch()
{
	if (!IsReloading() && GetStance() != ETacticalStance::STANCE_Crouched)
	{
		if (GetStance() == ETacticalStance::STANCE_Prone)
			StopAim();

		StopSprint();
		ChangeStance(ETacticalStance::STANCE_Crouched);
	}
}

void ATacticalCharacter::TryProne()
{
	if (!IsReloading() && GetStance() != ETacticalStance::STANCE_Prone)
	{
		StopAim();
		StopSprint();
		ChangeStance(ETacticalStance::STANCE_Prone);
	}
}

void ATacticalCharacter::TryLowerStance()
{
	if (!IsReloading())
	{
		StopSprint();
		switch (GetTacticalMovement()->GetCurrentStance())
		{
		case ETacticalStance::STANCE_Default:
			ChangeStance(ETacticalStance::STANCE_Crouched);
			break;
		case ETacticalStance::STANCE_Crouched:
			StopAim();
			ChangeStance(ETacticalStance::STANCE_Prone);
			break;
		default:
			break;
		}
	}
}

void ATacticalCharacter::TryRaiseStance()
{
	StopSprint();
	switch (GetTacticalMovement()->GetCurrentStance())
	{
	case ETacticalStance::STANCE_Prone:
		StopAim();
		ChangeStance(ETacticalStance::STANCE_Crouched);
		break;
	case ETacticalStance::STANCE_Crouched:
		ChangeStance(ETacticalStance::STANCE_Default);
		break;
	default:
		break;
	}
}

void ATacticalCharacter::GamepadCrouch()
{
	GetWorldTimerManager().SetTimer(GamepadCrouchTimer, this, &ATacticalCharacter::GamepadProne, 0.3f, false);
}

void ATacticalCharacter::GamepadCrouchReleased()
{
	if (GetWorldTimerManager().IsTimerActive(GamepadCrouchTimer))
	{
		GetWorldTimerManager().ClearTimer(GamepadCrouchTimer);
		if (GetStance() == ETacticalStance::STANCE_Crouched)
		{
			TryStand();
		}
		else
		{
			TryCrouch();
		}
	}
}

void ATacticalCharacter::GamepadProne()
{
	if (GetStance() == ETacticalStance::STANCE_Prone)
	{
		TryStand();
	}
	else
	{
		TryProne();
	}
}

void ATacticalCharacter::CrouchPressed()
{
	ATacticalPlayerController* TPC = Cast<ATacticalPlayerController>(GetController());
	if (TPC && TPC->ToggleCrouch() && (GetStance() == ETacticalStance::STANCE_Crouched))
	{
		TryStand();
	}
	else
	{
		TryCrouch();
		CrouchTime = GetWorld()->GetRealTimeSeconds();
	}
}

void ATacticalCharacter::CrouchReleased()
{
	ATacticalPlayerController* TPC = Cast<ATacticalPlayerController>(GetController());
	if ((!TPC || !TPC->ToggleCrouch()) /*|| (GetWorld()->GetRealTimeSeconds() - CrouchTime > CrouchTimeForToggle)*/)
	{
		TryStand();
	}
}

void ATacticalCharacter::PronePressed()
{
	if (GetStance() == ETacticalStance::STANCE_Prone)
	{
		TryStand();
	}
	else
	{
		TryProne();
	}
}

void ATacticalCharacter::ChangeStance(ETacticalStance NewStance)
{
	GetTacticalMovement()->SetPendingStance(NewStance);
}

// Sprint
void ATacticalCharacter::StartSprint()
{
	if (GetStance() != ETacticalStance::STANCE_Prone && !IsReloading() && !IsThrowingGrenade())
	{
		StopAim();
		ChangeStance(ETacticalStance::STANCE_Default);
		GetTacticalMovement()->bWantsToSprint = true;
		if (Role != ROLE_Authority)
		{
			ServerStartSprint();
		}
	}	
}

void ATacticalCharacter::ServerStartSprint_Implementation()
{
	if (!(GetWeapon() && GetWeapon()->IsReloading()))
	{
		StopAim();
		GetTacticalMovement()->bWantsToSprint = true;
	}
}

void ATacticalCharacter::StopSprint()
{
	GetTacticalMovement()->bWantsToSprint = false;
	if (Role != ROLE_Authority)
	{
		ServerStopSprint();
	}
}

void ATacticalCharacter::Gadget1_Pressed()
{
	ATacticalInventory* Inv = GetInventory()->Gadget1;
	if (Inv == nullptr)
	{
		return;
	}

	if (Inv->GetClass()->ImplementsInterface(UTacticalGadgetInterface::StaticClass()))
	{
		ITacticalGadgetInterface::Execute_OwnerPressedKey(Inv);
	}
}

void ATacticalCharacter::Gadget1_Released()
{
	ATacticalInventory* Inv = GetInventory()->Gadget1;
	if (Inv == nullptr)
	{
		return;
	}

	if (Inv->GetClass()->ImplementsInterface(UTacticalGadgetInterface::StaticClass()))
	{
		ITacticalGadgetInterface::Execute_OwnerReleasedKey(Inv);
	}
}

void ATacticalCharacter::Gadget2_Pressed()
{
	ATacticalInventory* Inv = GetInventory()->Gadget2;
	if (Inv == nullptr)
	{
		return;
	}

	if (Inv->GetClass()->ImplementsInterface(UTacticalGadgetInterface::StaticClass()))
	{
		ITacticalGadgetInterface::Execute_OwnerPressedKey(Inv);
	}
}

void ATacticalCharacter::Gadget2_Released()
{
	ATacticalInventory* Inv = GetInventory()->Gadget2;
	if (Inv == nullptr)
	{
		return;
	}

	if (Inv->GetClass()->ImplementsInterface(UTacticalGadgetInterface::StaticClass()))
	{
		ITacticalGadgetInterface::Execute_OwnerReleasedKey(Inv);
	}
}

void ATacticalCharacter::ServerStopSprint_Implementation()
{
	GetTacticalMovement()->bWantsToSprint = false;
}

// Aim
void ATacticalCharacter::StartAim()
{
	ATacticalPlayerController* TPC = Cast<ATacticalPlayerController>(GetController());
	if (TPC && TPC->ToggleAim())
	{
		if (!bIsAiming && CanAim())
		{
			StopSprint();
			bIsAiming = true;
			if (GetWeapon())
			{
				GetWeapon()->OnStartAiming();
			}
			if (Role != ROLE_Authority)
			{
				ServerStartAim();
			}
		}
		else
		{
			bIsAiming = false;
			if (GetWeapon())
			{
				GetWeapon()->OnStopAiming();
			}
			if (Role != ROLE_Authority)
			{
				ServerStopAim();
			}
		}
	}
	else
	{
		if (CanAim())
		{
			StopSprint();
			bIsAiming = true;
			if (GetWeapon())
			{
				GetWeapon()->OnStartAiming();
			}
			if (Role != ROLE_Authority)
			{
				ServerStartAim();
			}
		}
	}
	/*
	if (bThrowingGrenade)
	{
		CancelGrenadeThrow();
	}
	else*/ 

}

void ATacticalCharacter::ServerStartAim_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("ServerStartAim"));
	StopSprint();
	bIsAiming = true;

	if (GetWeapon())
	{
		GetWeapon()->OnStartAiming();
	}
}

void ATacticalCharacter::ReleaseAim()
{
	ATacticalPlayerController* TPC = Cast<ATacticalPlayerController>(GetController());
	if (!TPC || !TPC->ToggleAim())
	{
		StopAim();
	}
}

void ATacticalCharacter::StopAim()
{
	bIsAiming = false;

	if (GetWeapon())
	{
		GetWeapon()->OnStopAiming();
	}
	if (Role != ROLE_Authority)
	{
		ServerStopAim();
	}
}

void ATacticalCharacter::ServerStopAim_Implementation()
{
	bIsAiming = false;
	if (GetWeapon())
	{
		GetWeapon()->OnStopAiming();
	}
}

// Interaction
AActor* ATacticalCharacter::InteractionTrace()
{
	FHitResult Hit(ForceInit);
	const FVector TraceStart = GetFirstPersonCamera()->GetComponentLocation();
	const FVector TraceEnd = TraceStart + GetBaseAimRotation().Vector() * InteractionTraceDistance;
	FCollisionQueryParams TraceParams(TEXT("InteractionTrace"), false, this);
	if (GetWeapon())
		TraceParams.AddIgnoredActor(GetWeapon());
	
	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, TRACE_Interaction, TraceParams))
	{
		if (Hit.Actor.IsValid() && Hit.Actor->GetClass()->ImplementsInterface(UBRSCharacterInteractionInterface::StaticClass()))
		{
			return Hit.Actor.Get();
		}
	}
	return nullptr;

}

class ATacticalVaultMarker* ATacticalCharacter::GetBestVaultMarker() const
{		
	ATacticalVaultMarker* BestMarker = nullptr;
	if (CanVault())
	{
		TArray<AActor*> OverlappingActors;
		GetOverlappingActors(OverlappingActors, ATacticalVaultMarker::StaticClass());
		for (AActor* TestActor : OverlappingActors)
		{
			if (ATacticalVaultMarker* TestMarker = Cast<ATacticalVaultMarker>(TestActor))
			{
				const FVector RelativeLocation = TestMarker->GetTransform().InverseTransformPositionNoScale(GetActorLocation());
				//UE_LOG(LogTemp, Log, TEXT("Loc: %s"), *RelativeLocation.ToString());
				if (FMath::Abs(RelativeLocation.Z + 15.f) < 20.f)
				{

					const bool bIsBehind = RelativeLocation.X < 0.f;
					const float DeltaYaw = FMath::Abs(FRotator::NormalizeAxis(TestMarker->GetActorRotation().Yaw - GetActorRotation().Yaw));
					// is facing vault marker or is AIControlled (AI doesn't need to face marker)
					if (IsAIControlled() || ((bIsBehind && (DeltaYaw < 45.f)) || (!bIsBehind && (DeltaYaw > (180.f - 45.f)))))
					{
						// Initiate Vault
						BestMarker = TestMarker;
						break;
					}
				}
			}
		}
		if (BestMarker)
		{
			return BestMarker;
		}
	}
	return nullptr;
}

void ATacticalCharacter::StartInteract()
{
	if (IsAlive())
	{
		ServerStartInteract();
		bIsInteracting = true;
		GetWorldTimerManager().SetTimer(InteractPressedTimer, InteractImmidiateTimeFrame, false);
		if (InteractActor && InteractActor->GetClass()->ImplementsInterface(UBRSCharacterInteractionInterface::StaticClass()))
		{
			IBRSCharacterInteractionInterface::Execute_OnLocalStartInteract(InteractActor, this);
		}
	}
}


void ATacticalCharacter::ServerStartInteract_Implementation()
{
	if (IsAlive())
	{
		bIsInteracting = true;
		// Try to vault if we are overlapping a vault marker
		if (ATacticalVaultMarker* BestMarker = GetBestVaultMarker())
		{
			Vault(BestMarker);
			return;
		}

		// Try to interact otherwise
		InteractActor = InteractionTrace();
		if (InteractActor && InteractActor->GetClass()->ImplementsInterface(UBRSCharacterInteractionInterface::StaticClass()))
		{
			IBRSCharacterInteractionInterface::Execute_OnServerStartInteract(InteractActor, this);
		}
	}
}

void ATacticalCharacter::StopInteract()
{
	if (IsAlive())
	{
		bIsInteracting = false;
		if (GetWorldTimerManager().IsTimerActive(InteractPressedTimer))
		{
			GetWorldTimerManager().ClearTimer(InteractPressedTimer);
			InteractImmidiate();
		}
		ServerStopInteract();

		if (InteractActor && InteractActor->GetClass()->ImplementsInterface(UBRSCharacterInteractionInterface::StaticClass()))
		{
			IBRSCharacterInteractionInterface::Execute_OnLocalStopInteract(InteractActor, this);
		}
	}
}

void ATacticalCharacter::ServerStopInteract_Implementation()
{
	if (IsAlive())
	{
		bIsInteracting = false;
		InteractActor = InteractionTrace();
		if (InteractActor && InteractActor->GetClass()->ImplementsInterface(UBRSCharacterInteractionInterface::StaticClass()))
		{
			IBRSCharacterInteractionInterface::Execute_OnServerStopInteract(InteractActor, this);
		}
	}
}

void ATacticalCharacter::InteractImmidiate()
{
	if (IsAlive())
	{
		bIsInteracting = false;
		ServerInteractImmidiate();

		if (InteractActor && InteractActor->GetClass()->ImplementsInterface(UBRSCharacterInteractionInterface::StaticClass()))
		{
			IBRSCharacterInteractionInterface::Execute_OnLocalInteractImmidiate(InteractActor, this);
		}
	}
}

void ATacticalCharacter::ServerInteractImmidiate_Implementation()
{
	if (IsAlive())
	{
		bIsInteracting = false;
		InteractActor = InteractionTrace();
		if (InteractActor && InteractActor->GetClass()->ImplementsInterface(UBRSCharacterInteractionInterface::StaticClass()))
		{
			IBRSCharacterInteractionInterface::Execute_OnServerInteractImmidiate(InteractActor, this);
		}
	}
}

void ATacticalCharacter::SlowOpen()
{
	if (IsAlive())
	{
		ServerSlowOpen();
	}
}

void ATacticalCharacter::ServerSlowOpen_Implementation()
{
	if (IsAlive())
	{
		InteractActor = InteractionTrace();
		if (InteractActor && InteractActor->GetClass()->ImplementsInterface(UBRSCharacterInteractionInterface::StaticClass()))
		{
			IBRSCharacterInteractionInterface::Execute_OnServerSlowOpen(InteractActor, this);
		}
	}
}

void ATacticalCharacter::SlowClose()
{
	if (IsAlive())
	{
		ServerSlowClose();
	}
}

void ATacticalCharacter::ServerSlowClose_Implementation()
{
	if (IsAlive())
	{
		InteractActor = InteractionTrace();
		if (InteractActor && InteractActor->GetClass()->ImplementsInterface(UBRSCharacterInteractionInterface::StaticClass()))
		{
			IBRSCharacterInteractionInterface::Execute_OnServerSlowClose(InteractActor, this);
		}
	}
}


// Firing
void ATacticalCharacter::StartFire()
{
	if (WidgetInteractionComp->IsOverHitTestVisibleWidget())
	{
		WidgetInteractionComp->PressPointerKey(EKeys::LeftMouseButton);
	}
	else
	{
		if (GetWeapon() && !IsThrowingGrenade())
		{
			GetWeapon()->StartFiring();
		}
	}
}

void ATacticalCharacter::StopFire()
{
	WidgetInteractionComp->ReleasePointerKey(EKeys::LeftMouseButton);
	if (GetWeapon())
	{
		GetWeapon()->StopFiring();
	}
}

void ATacticalCharacter::ToggleWeaponSight()
{
	if (GetWeapon())
	{
		GetWeapon()->ToggleSight();
	}
}

// Weapon Attachments/Features
void ATacticalCharacter::ToggleWeaponGadget1()
{
	if (GetWeapon())
	{
		GetWeapon()->NativeToggleAttachment(0);
	}
}

void ATacticalCharacter::ToggleWeaponGadget2()
{
	if (GetWeapon())
	{
		GetWeapon()->NativeToggleAttachment(1);
	}
}

void ATacticalCharacter::ToggleFireMode()
{
	if (GetWeapon() && !IsThrowingGrenade())
	{
		GetWeapon()->ToggleFiringMode();
	}
}

// Weapon Switching
void ATacticalCharacter::SwitchToPrimaryWeapon()
{
	if (GetInventory()->PrimaryWeapon && GetWeapon() != GetInventory()->PrimaryWeapon)
	{
		TrySwitchWeapon(GetInventory()->PrimaryWeapon);
	}
}

void ATacticalCharacter::SwitchToSidearm()
{
	if (GetInventory()->Sidearm && GetWeapon() != GetInventory()->Sidearm)
	{
		TrySwitchWeapon(GetInventory()->Sidearm);
	}
}

void ATacticalCharacter::HolsterWeapon()
{
	TrySwitchWeapon(nullptr);
}

void ATacticalCharacter::GamepadSwitchWeapon()
{
	if (GetWeapon() == GetInventory()->PrimaryWeapon)
	{
		SwitchToSidearm();
	}
	else
	{
		SwitchToPrimaryWeapon();
	}
}


// Grenades
void ATacticalCharacter::OnRep_ThrowingGrenade(bool bPrevValue)
{
	if (bPrevValue != bThrowingGrenade)
	{
		if (bThrowingGrenade)
		{
			OnStartGrenadeThrow();
		}
		else
		{
			OnCancelGrenadeThrow();
		}
	}
}

bool ATacticalCharacter::CanThrowGrenade(class ATacticalThrowable* GadgetToThrow) const
{
	if (!IsAlive())
	{
		return false;
	}

	if (!GadgetToThrow || !GadgetToThrow->HasAnyChargesLeft())
	{
		return false;
	}

	if (IsVaulting() || (GetTacticalMovement() && GetTacticalMovement()->IsChangingStance()) || (GetStance() == ETacticalStance::STANCE_Prone)
		|| IsChangingWeapon() || IsReloading())
	{
		return false;
	}
	return true;
}



void ATacticalCharacter::OnGrenadePressed()
{
	/*if (!IsThrowingGrenade() && CanThrowGrenade())
	{
		StopSprint();
		StopAim();
		StopFire();

		StartGrenadeThrow();
	}*/
	//else if (bThrowingGrenade)
	//{
	//	FinishGrenadeThrow();
	//}
}

void ATacticalCharacter::EnableThrowGrenade()
{
	GetWorldTimerManager().ClearTimer(TimerGrenadePreThrowDelay);
	if (bWantsToFinishGrenadeThrow)
	{
		bWantsToFinishGrenadeThrow = false;
		FinishThrow(PendingThrowable);
	}
}

void ATacticalCharacter::StartGrenadeThrow()
{
	StartThrow(InventoryComponent->Grenade);
}

void ATacticalCharacter::ServerStartGrenadeThrow_Implementation(class ATacticalThrowable* GadgetToThrow)
{
	if (!CanThrowGrenade(GadgetToThrow))
		return;

	PendingThrowable = GadgetToThrow;
	bWantsToFinishGrenadeThrow = false;
	bThrowingGrenade = true;
	OnStartGrenadeThrow();
}

void ATacticalCharacter::StartThrow(class ATacticalThrowable* GadgetToThrow)
{
	if (!CanThrowGrenade(GadgetToThrow) || IsThrowingGrenade(/*GadgetToThrow*/))
		return;

	UE_LOG(LogTemp, Log, TEXT("asda"))
	StopSprint();
	StopAim();
	StopFire();

	PendingThrowable = GadgetToThrow;
	bWantsToFinishGrenadeThrow = false;
	bThrowingGrenade = true;
	GetWorldTimerManager().SetTimer(TimerGrenadePreThrowDelay, this, &ATacticalCharacter::EnableThrowGrenade, GrenadePreThrowDelay, false);
	OnStartGrenadeThrow();

	if (Role < ROLE_Authority)
	{
		//UE_LOG(LogTemp, Log, TEXT("Calling server"));
		ServerStartGrenadeThrow(GadgetToThrow);
	}
}

void ATacticalCharacter::FinishThrow(class ATacticalThrowable* GadgetToThrow)
{
	if ((!GadgetToThrow) || (PendingThrowable != GadgetToThrow))
	{
		return;
	}

	if (bThrowingGrenade && PendingThrowable)
	{
		if (!GetWorldTimerManager().IsTimerActive(TimerGrenadePreThrowDelay))
		{
			bThrowingGrenade = false;
			//OnFinishGrenadeThrow();
			GetWorldTimerManager().SetTimer(TimerGrenadeCooldown, GrenadeCooldown, false);
			if (Role < ROLE_Authority)
			{
				ServerFinishGrenadeThrow();
			}
			else
			{
				NetMulti_FinishGrenadeThrow();
				GetWorldTimerManager().SetTimer(TimerGrenadePostThrowDelay, this, &ATacticalCharacter::ThrowGrenade, GrenadePostThrowDelay, false);
			}
		}
		else
		{
			//UE_LOG(LogTemp, Log, TEXT("Wants To throw but needs to wait"));
			bWantsToFinishGrenadeThrow = true;
		}
	}
}

void ATacticalCharacter::ServerFinishGrenadeThrow_Implementation()
{
	if (bThrowingGrenade)
	{
		bThrowingGrenade = false;
		NetMulti_FinishGrenadeThrow();
		GetWorldTimerManager().SetTimer(TimerGrenadeCooldown, GrenadeCooldown, false);
		GetWorldTimerManager().SetTimer(TimerGrenadePostThrowDelay, this, &ATacticalCharacter::ThrowGrenade, GrenadePostThrowDelay, false);
	}
}

void ATacticalCharacter::CancelGrenadeThrow()
{
	GetWorldTimerManager().ClearTimer(TimerGrenadePreThrowDelay);
	bThrowingGrenade = false;
	bWantsToFinishGrenadeThrow = false;
	OnCancelGrenadeThrow();
	if (Role < ROLE_Authority)
	{
		ServerCancelGrenadeThrow();
	}
	PendingThrowable = nullptr;
}

void ATacticalCharacter::ServerCancelGrenadeThrow_Implementation()
{
	GetWorldTimerManager().ClearTimer(TimerGrenadePreThrowDelay);
	bThrowingGrenade = false;
	PendingThrowable = nullptr;
}

void ATacticalCharacter::ThrowGrenade()
{
	if (Role == ROLE_Authority)
	{
		// spawn Grenade projectile
		if (PendingThrowable)
		{
			PendingThrowable->Throw(ProjectileSpawn->GetComponentLocation(), ProjectileSpawn->GetComponentRotation());
		}
		PendingThrowable = nullptr;
	}
}

// Reload
void ATacticalCharacter::StartReload()
{
	if (GetWeapon() && !(GetTacticalMovement() && GetTacticalMovement()->IsChangingStance()))
	{
		GetWeapon()->StartReload();
		if (GetWeapon()->IsReloading())
		{
			StopAim();
			StopSprint();
		}
	}
}

void ATacticalCharacter::StopReload()
{
	// Stop Reload e.g. for Shotguns or Revolvers
	if (GetWeapon())
	{
		GetWeapon()->StopReload();
	}
}




void ATacticalCharacter::BehindView(bool bUseBehindView)
{
	bThirdPersonView = bUseBehindView;
	if (bThirdPersonView)
	{
		OnThirdPerson();
	}
	else
	{
		OnFirstPerson();
	}
}

void ATacticalCharacter::LeanRight(float Val)
{
	Val = FMath::Clamp(Val+ToggleLeanVal, -1.f, 1.f);
	TargetLeanAngle = (GetTacticalMovement()->GetCurrentStance() != ETacticalStance::STANCE_Prone && GetTacticalMovement()->IsMovingOnGround()) ?
		Val * MaxLeanAngle : 0.f;

	float NewLeanAngle = FMath::FInterpConstantTo(LeanAngle, TargetLeanAngle, GetWorld()->GetDeltaSeconds(), LeanSpeed);
	
	if (Role < ROLE_Authority && LeanAngle != NewLeanAngle)
	{
		ServerLeanRight(Val);
	}

	LeanAngle = NewLeanAngle;
}

void ATacticalCharacter::ToggleLeanRight()
{
	ToggleLean(EDirection::Right);
}

void ATacticalCharacter::ToggleLeanLeft()
{
	ToggleLean(EDirection::Left);
}

void ATacticalCharacter::ToggleLean(EDirection Dir)
{
	// If character is not leaning in any direction lean to specified direction
	// If character is leaning left/right reset to default
	if (ToggleLeanVal == 0.f)
	{
		ToggleLeanVal = (Dir == EDirection::Right) ? 1.f : -1.f;
	}
	else
	{
		ToggleLeanVal = 0.f;
	}
}

void ATacticalCharacter::ServerLeanRight_Implementation(float Val)
{
	TargetLeanAngle = (GetTacticalMovement()->GetCurrentStance() != ETacticalStance::STANCE_Prone && GetTacticalMovement()->IsMovingOnGround()) ?
		Val * MaxLeanAngle : 0.f;

	LeanAngle = FMath::FInterpConstantTo(LeanAngle, TargetLeanAngle, GetWorld()->GetDeltaSeconds(), LeanSpeed);
}

void ATacticalCharacter::SpeedUp()
{
	if (InteractActor && bIsInteracting)
	{
		SlowOpen();
	}
	else
	{
		SetWalkSpeedFactor(WalkSpeedFactor + SpeedIncrement);
	}
}

void ATacticalCharacter::SpeedDown()
{
	if (InteractActor && bIsInteracting)
	{
		SlowClose();
	}
	else
	{
		SetWalkSpeedFactor(WalkSpeedFactor - SpeedIncrement);
	}
}

void ATacticalCharacter::SetWalkSpeedFactor(float newSpeedFactor)
{
	WalkSpeedFactor = FMath::Clamp(newSpeedFactor, 0.25f, 1.f);
}

bool ATacticalCharacter::IsThirdPerson() const
{
	// todo: When spectating from first person, also return false


	if (GetTacticalPlayerController() && GetTacticalPlayerController()->IsLocalController())
	{
		return bThirdPersonView;
	}
	
	return true;
}

void ATacticalCharacter::Melee()
{
	if (GetWorldTimerManager().IsTimerActive(TimerMeleeCooldown))
		return;

	ServerMelee();

	// todo: maybe do some forward prediction
	if(MeleeCooldown >= 0.01)
	{
		GetWorldTimerManager().SetTimer(TimerMeleeCooldown, MeleeCooldown, false);
	}

}

void ATacticalCharacter::ServerMelee_Implementation()
{
	NetMultiMelee();

	if (MeleeDamageDelay > 0.01f)
	{
		GetWorldTimerManager().SetTimer(TimerMeleeDamageDelay, this,  &ATacticalCharacter::DoMeleeDamage, MeleeDamageDelay, false);
	}
	else
	{
		DoMeleeDamage();
	}


	if (MeleeCooldown >= 0.01)
	{
		GetWorldTimerManager().SetTimer(TimerMeleeCooldown, MeleeCooldown, false);
	}

}

void ATacticalCharacter::NetMultiMelee_Implementation()
{
	SimulateMelee();
}

void ATacticalCharacter::SimulateMelee_Implementation()
{

}

void ATacticalCharacter::DoMeleeDamage()
{
	FCollisionQueryParams QueryParams(FName("MeleeTrace"), false, this);
	QueryParams.AddIgnoredActor(GetWeapon());
	QueryParams.bReturnPhysicalMaterial = true;


	const float TraceDist = MeleeRange;
	const FVector TraceStart = GetWeaponTraceOrigin();
	const FVector TraceDir = GetBaseAimRotation().Vector();
	const FVector TraceEnd = TraceStart + TraceDir * TraceDist;

	TArray<FHitResult> Hits;
	//GetWorld()->LineTraceMultiByChannel(Hits, TraceStart, TraceEnd, TRACE_Weapon, QueryParams);
	GetWorld()->SweepMultiByChannel(Hits, TraceStart, TraceEnd, FQuat(ForceInit), TRACE_Weapon, FCollisionShape::MakeSphere(30.f), QueryParams);

	if (Hits.Num() > 0)
	{
		const FHitResult& Hit = Hits[0];
		if (Hit.Actor.IsValid())
		{
			AActor* HitActor = Hit.Actor.Get();
			UE_LOG(LogTemp, Log, TEXT("Melee Hit: %s"), *HitActor->GetName());
			UGameplayStatics::ApplyPointDamage(HitActor, MeleeDamage, -TraceDir, Hit, GetController(), this, MeleeDamageType ? MeleeDamageType : UTacticalDamageType::StaticClass());
		}
		// Spawn Impact FX
		UTacticalImpactFXComponent* ImpactFXComp = nullptr;
		if (Hit.GetActor())
		{
			ImpactFXComp = Hit.GetActor()->FindComponentByClass<UTacticalImpactFXComponent>();

		}

		if (MeleeImpactFX)
		{
			ATacticalImpactFX* ImpactEffectCDO = MeleeImpactFX->GetDefaultObject<ATacticalImpactFX>();
			if (ImpactEffectCDO)
			{
				ImpactEffectCDO->SpawnImpactEffects(GetWorld(), Hit, -TraceDir, ImpactFXComp);
			}
		}
		else if (ImpactFXComp)
		{
			// We don't have a ImpactEffect so we fall back to the Impact Override Comp
			ImpactFXComp->PlayImpactSound(Hit);
			ImpactFXComp->SpawnImpactParticle(Hit, -TraceDir);
			ImpactFXComp->SpawnImpactDecal(Hit);
		}

		if (ImpactFXComp && ImpactFXComp->OnSimulateHit.IsBound())
		{
			ImpactFXComp->OnSimulateHit.Broadcast(Hit, -TraceDir);
		}
	}
}

void ATacticalCharacter::OnRep_FullyRagdoll()
{
	if (GetMesh() && bFullyRagdoll)
	{
		GetMesh()->SetPhysicsBlendWeight(1.f);
		if (GetWeapon() && GetWeapon()->GetMesh())
		{
			GetWeapon()->GetMesh()->SetSimulatePhysics(true);
			GetWeapon()->GetMesh()->SetAllBodiesSimulatePhysics(true);
			GetWeapon()->GetMesh()->WakeAllRigidBodies();
			GetWeapon()->GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
			GetWeapon()->GetMesh()->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
		}
	}
	else if (GetMesh())
	{
		GetMesh()->SetPhysicsBlendWeight(0.f);
	}
}

void ATacticalCharacter::ServerSendLeanAngle_Implementation(float Angle)
{
	LeanAngle = Angle;
}

void ATacticalCharacter::Suicide()
{
	TakeDamage(1000.f, FDamageEvent(UDamageType::StaticClass()), Controller, this);
}

void ATacticalCharacter::Stun(float Duration)
{
	if (Role == ROLE_Authority && Duration > 0.f)
	{
		bIsStunned = true;
		GetWorldTimerManager().SetTimer(TimerStunned, this, &ATacticalCharacter::EndStun, Duration, false);
	}
}

void ATacticalCharacter::EndStun()
{
	bIsStunned = false;
	GetWorldTimerManager().ClearTimer(TimerStunned);
}


void ATacticalCharacter::PerformHitReaction(const UTacticalDamageType* DamageType)
{
	if (Role == ROLE_Authority && IsAIControlled() 
		&& DamageType && DamageType->bHasKnockback && DamageType->KnockbackStunTime > 0.01f)
	{
		if (!GetTacticalMovement() || !GetTacticalMovement()->IsMovingOnGround())
			return;
		if (IsVaulting())
			return;

		const float StunTime = DamageType->KnockbackStunTime;
		Stun(StunTime+0.2f);

		// If character is prone it doesn't lose balance when hit
		if (GetStance() == ETacticalStance::STANCE_Prone)
		{
			return;
		}

		UAnimMontage* HitMontage = GetHitMontage();
		int32 SectionNum = GetHitMontage()->CompositeSections.Num();
		uint8 MontageIdx = (uint8)FMath::Clamp(FMath::RandHelper(SectionNum), 0, 255);

		const float HitReactionAnimTime = FMath::Max(StunTime - 0.25f, 0.f);
		if (HitReactionAnimTime > KINDA_SMALL_NUMBER)
		{
			NetMulti_PerformHitReaction(MontageIdx, HitReactionAnimTime);
		}
	}
}

void ATacticalCharacter::NetMulti_PerformHitReaction_Implementation(uint8 MontageIdx, float StunnedTime)
{
	if(GetHitMontage())
	{
		// If Timer isn't active it means that we are probably not playing the hit animation. Play one
		if (!GetWorldTimerManager().IsTimerActive(HitReactionTimer))
		{
			//GetWorldTimerManager().ClearTimer(HitReactionTimer);
			UAnimMontage* Montage = GetHitMontage();
			PlayAnimMontage(Montage, 1.f, Montage->GetSectionName(MontageIdx));
		}
		// Reset Stun timer
		GetWorldTimerManager().SetTimer(HitReactionTimer, this, &ATacticalCharacter::FinishHitReaction, StunnedTime, false);
	}
}

UAnimMontage* ATacticalCharacter::GetHitMontage() const
{
	if (GetWeapon())
	{
		switch (GetWeapon()->GetWeaponType())
		{
		case EWeaponType::Rifle:
			return HitMontageRifle;
		case EWeaponType::Pistol:
			return HitMontagePistol;
		default:
			break;
		}
	}
	return HitMontageDefault;
}


void ATacticalCharacter::FinishHitReaction()
{
	if (GetHitMontage())
	{
		StopAnimMontage(GetHitMontage());
	}
}

void ATacticalCharacter::TickFootIK(float DeltaTime)
{
	if (!GetMesh())
		return;

	const FVector DefaultTranslation = GetDefault<ATacticalCharacter>()->GetMesh()->RelativeLocation;
	const FVector CurrentTranslation = GetMesh()->RelativeLocation;
	float DefaultZ = CurrentTranslation.Z - RootZAdjust;

	const FVector RootBoneLoc = GetMesh()->GetSocketLocation(FName(TEXT("root")));

	// Trace at foot bones to determine how much the bones need to be moved
	FHitResult Hit_R(ForceInit);
	FHitResult Hit_L(ForceInit);

	const FName SocketR(TEXT("foottrace_r"));
	const FName SocketL(TEXT("foottrace_l"));

	if (GetCharacterMovement()->IsMovingOnGround())
	{
		TraceFoot(Hit_R, SocketR);
		TraceFoot(Hit_L, SocketL);
	}

	// Trace a couple of points (front and back of character) to determine how much the root bone needs to be lowered
	float RootMoveZ = 0.f;
	float MinZ = RootBoneLoc.Z;
	bool bFirstTrace = true;
	for (const FVector& TraceLoc : RootTraceLocs)
	{
		FHitResult Hit(ForceInit);
		FVector TraceBase = GetTransform().TransformPositionNoScale(TraceLoc);
		TraceBase.Z = RootBoneLoc.Z;
		const FVector TraceStart = TraceBase + FVector::UpVector * 50.f;
		const FVector TraceEnd = TraceBase - FVector::UpVector * 50.f;
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(this);
		GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, TRACE_FootPlacement, TraceParams);

		/*DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Red);
		DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 10.f, FColor::Yellow);*/

		// Take the minimum Z out of all successful hits
		if (Hit.bBlockingHit)
		{
			if (bFirstTrace)
			{
				MinZ = Hit.ImpactPoint.Z;
				bFirstTrace = false;
			}
			else if (Hit.ImpactPoint.Z < MinZ)
			{
				MinZ = Hit.ImpactPoint.Z;
			}
		}

	}

	// Determine how much Root needs to be moved and smooth it.
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		RootMoveZ = FMath::Clamp((MinZ - RootBoneLoc.Z), -RootAdjustSpeed*DeltaTime, RootAdjustSpeed*DeltaTime);
	}
	else
	{
		// Undo adjustment
		RootMoveZ = FMath::Clamp(-RootZAdjust, -RootAdjustSpeed*DeltaTime, RootAdjustSpeed*DeltaTime);
	}

	// Clamp Root Position to bounds
	if ((RootZAdjust + RootMoveZ) < -MaxRootAdjustment)
	{
		RootMoveZ = (-MaxRootAdjustment) - RootZAdjust;
	}
	else if ((RootZAdjust + RootMoveZ) > 0.f)
	{
		RootMoveZ = -RootZAdjust;
	}
	RootZAdjust += RootMoveZ;
	RootZAdjust *= 0.5f;
	RootMoveZ = CurrentTranslation.Z - DefaultZ - RootZAdjust;

	// Move Mesh
	const FVector NewMeshRelLoc = FVector(CurrentTranslation.X, CurrentTranslation.Y, DefaultZ + RootZAdjust);
	GetMesh()->SetRelativeLocation(NewMeshRelLoc);
	//GetMesh()->RelativeLocation.Z = CurrentTranslation.Z + RootMoveZ;

	// Add Root Movement to the final Foot offsets.
	float RTarget = 0.f;
	float LTarget = 0.f;

	RTarget = Hit_R.bBlockingHit ? Hit_R.ImpactPoint.Z - RootBoneLoc.Z - RootMoveZ : 0.f;
	LTarget = Hit_L.bBlockingHit ? Hit_L.ImpactPoint.Z - RootBoneLoc.Z - RootMoveZ : 0.f;

	// Define Foot Normals for slanted surfaces
	RightFootNorm = Hit_R.bBlockingHit ? Hit_R.ImpactNormal : FVector::UpVector;
	LeftFootNorm = Hit_L.bBlockingHit ? Hit_L.ImpactNormal : FVector::UpVector;

	// Smooth foot offsets
	RightFootLoc.Z = FMath::FInterpConstantTo(RightFootLoc.Z, RTarget, DeltaTime, FootPlacementSpeed);
	LeftFootLoc.Z = FMath::FInterpConstantTo(LeftFootLoc.Z, LTarget, DeltaTime, FootPlacementSpeed);
}

bool ATacticalCharacter::TraceFoot(FHitResult& Hit, const FName& FootSocketName)
{
	if (GetMesh() && GetMesh()->DoesSocketExist(FootSocketName))
	{
		const FVector FootLoc = GetMesh()->GetSocketLocation(FootSocketName);
		const FVector TraceStart = FootLoc + FVector::UpVector * 50.f;
		const FVector TraceEnd = FootLoc - FVector::UpVector * 50.f;
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(this);
		bool bResult = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, TRACE_FootPlacement, TraceParams);
		//DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 10.f, FColor::Red, false);
		return bResult;
	}
	return false;
}



void ATacticalCharacter::AdjustMeshHeight(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	//RecalculateBaseEyeHeight();
	if (GetMesh())
	{
		FVector NewLoc = GetDefault<ACharacter>(GetClass())->GetMesh()->RelativeLocation + FVector::UpVector * HalfHeightAdjust;
		GetMesh()->SetRelativeLocation(NewLoc);
		BaseTranslationOffset.Z = GetMesh()->RelativeLocation.Z;
	}
	else
	{
		BaseTranslationOffset.Z = GetDefault<ACharacter>(GetClass())->GetBaseTranslationOffset().Z + HalfHeightAdjust;
	}
}



void ATacticalCharacter::OnDeath(float Damage, const class UDamageType* DamageType, class APawn* InstigatedBy, AActor* DamageCauser)
{
	bReplicateMovement = false;
	//bTearOff = true;
	if(WidgetInteractionComp)
	{
		WidgetInteractionComp->ReleasePointerKey(EKeys::LeftMouseButton);
	}

	bool bImplementsInteractInterface = (InteractActor && InteractActor->IsValidLowLevel()) ? InteractActor->GetClass()->ImplementsInterface(UBRSCharacterInteractionInterface::StaticClass()) : false;

	if (bImplementsInteractInterface && GetController()->IsLocalPlayerController())
	{
		IBRSCharacterInteractionInterface::Execute_OnStopHighlight(InteractActor, this);
	}

	if(Role == ROLE_Authority)
	{
		// Remove Inventory // currently commented to keep items on death

		// If the character is interacting with something stop it now.
		if (bImplementsInteractInterface)
		{
			IBRSCharacterInteractionInterface::Execute_OnServerStopInteract(InteractActor, this);
		}
	}


	if (GetController() && GetController()->GetClass()->ImplementsInterface(UTacticalControllerInterface::StaticClass()))
	{
		ITacticalControllerInterface::Execute_OnPawnDied(GetController(), Damage, DamageType, InstigatedBy, DamageCauser);
	}
	if (ATacticalPlayerController* PC = Cast<ATacticalPlayerController>(GetController()))
	{
		if(PC->OnPawnDiedDelegate.IsBound())
			PC->OnPawnDiedDelegate.Broadcast();
	}


	// Detach from controller
	GetWorldTimerManager().SetTimer(TimerDetachFromController, this, &ATacticalCharacter::DetachCharacterFromController, 2.f, false);

	const bool bMovingOnGround = GetCharacterMovement()->IsMovingOnGround();
	// Deactivate Movement
	GetCharacterMovement()->Deactivate();

	// todo: Handle View
	// todo: Play Death sounds

	// Stop all animmontages
	// StopAllAnimMontages();

	// Disable collision capsule
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);

	if (GetMesh())
	{
		static FName CollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetCollisionProfileName(CollisionProfileName);
	}
	SetActorEnableCollision(true);


	if (GetWeapon())
	{
		GetWeapon()->OnOwnerDied();
	}

	//Rag doll(if playing death anim use timer enable rag doll after death anim)
	//SetRagdollPhysics();
	const bool bInStunAnim = GetWorldTimerManager().IsTimerActive(TimerStunned) && (GetWorldTimerManager().GetTimerElapsed(TimerStunned) >= 0.075f);
	if (bMovingOnGround && !bInStunAnim)
	{
		PlayDeathAnimation();
	}
	else
	{
		bFullyRagdoll = true;
		GetMesh()->SetPhysicsBlendWeight(1.f);		
		if (GetWeapon() && GetWeapon()->GetMesh())
		{
			GetWeapon()->GetMesh()->SetSimulatePhysics(true);
			GetWeapon()->GetMesh()->SetAllBodiesSimulatePhysics(true);
			GetWeapon()->GetMesh()->WakeAllRigidBodies();
			GetWeapon()->GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
			GetWeapon()->GetMesh()->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
		}
	}
}

void ATacticalCharacter::OnDamage(float Damage, const class UDamageType* DamageType, class APawn* InstigatedBy, AActor* DamageCauser)
{
	if (GetController() && GetController()->GetClass()->ImplementsInterface(UTacticalControllerInterface::StaticClass()))
	{
		ITacticalControllerInterface::Execute_OnPawnTakeDamage(GetController(), Damage, DamageType, InstigatedBy, DamageCauser);
	}
}

void ATacticalCharacter::DetachCharacterFromController()
{
	//Request Respawn from game mode
	APlayerController* PC = Cast<APlayerController>(Controller);

	if (GetWorldTimerManager().IsTimerActive(TimerDetachFromController))
		GetWorldTimerManager().ClearTimer(TimerDetachFromController);

	if (Controller != nullptr && Controller->GetPawn() == this)
	{
		Controller->UnPossess();
		Controller = nullptr;
	}
}

void ATacticalCharacter::SetRagdollPhysics()
{
	bool bInRagdoll = false;

	if (IsPendingKill())
	{
		bInRagdoll = false;
	}
	else if (!GetMesh() || !GetMesh()->GetPhysicsAsset())
	{
		bInRagdoll = false;
	}
	else
	{
		// initialize physics/etc
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();
		GetMesh()->bBlendPhysics = true;
		GetMesh()->SetPhysicsBlendWeight(0.f);

		bInRagdoll = true;
	}

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetComponentTickEnabled(false);

	if (!bInRagdoll)
	{
		// hide and set short lifespan
		TurnOff();
		SetActorHiddenInGame(true);
		//SetLifeSpan(1.0f);
	}
	else
	{
		//SetLifeSpan(10.0f);
	}
}

void ATacticalCharacter::StopAllAnimMontages()
{
	USkeletalMeshComponent* UseMesh = GetMesh();
	if (UseMesh && UseMesh->AnimScriptInstance)
	{
		UseMesh->AnimScriptInstance->Montage_Stop(0.0f);
	}
}

void ATacticalCharacter::PlayDeathAnimation_Implementation()
{
	if (GetMesh())
	{
		const TArray<UAnimMontage*>& Anims = DeathAnimations.GetMontageArray(GetWeapon() ? GetWeapon()->GetWeaponType() : EWeaponType::None);
		int32 idx = FMath::RandRange(0, Anims.Num() - 1);
		PlayAnimMontage(Anims[idx]);
	}
}



bool ATacticalCharacter::CanBeSeenFrom(const FVector& ObserverLocation, FVector& OutSeenLocation, int32& NumberOfLoSChecksPerformed, float& OutSightStrength, const AActor* IgnoreActor /*= NULL*/) const
{
	// todo: make better check. i.e. a specific AI sight channel
	FCollisionQueryParams TraceParams(TEXT("AILoSTrace"), false, IgnoreActor);
	// add weapon and Attachment to ignored actors
	if(GetWeapon())
	{
		TraceParams.AddIgnoredActor(GetWeapon());
		TArray<UTacticalWeaponAttachmentPoint*> WPNAttach;
		GetWeapon()->GetAttachmentPoints(WPNAttach);
		for (UTacticalWeaponAttachmentPoint* TestAttach : WPNAttach)
		{
			if (TestAttach && TestAttach->GetCurrentAttachment())
			{
				TraceParams.AddIgnoredActor(TestAttach->GetCurrentAttachment());
			}
		}
	}

	FHitResult HitResult;
	float StanceAdjust = 0.0f;
	switch (GetStance())
	{
	case ETacticalStance::STANCE_Crouched:
		StanceAdjust = -20.f;
	case ETacticalStance::STANCE_Prone:
		StanceAdjust = -50.f;
	default:
		break;
	}
	// bHit is true if something blocks vision
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, ObserverLocation, 
		GetActorLocation() + FVector::UpVector * StanceAdjust, 
		TRACE_AIVision, TraceParams);
	OutSightStrength = 1.f;
	// maybe head can be seen
	if (bHit)
	{
		TraceParams.bTraceComplex = true;
		const FVector HeadLocation = GetMesh()->GetSocketLocation(FName("head"));
		bHit = GetWorld()->LineTraceSingleByChannel(HitResult, ObserverLocation, HeadLocation, TRACE_AIVision, TraceParams);
		//DrawDebugLine(GetWorld(), ObserverLocation, HeadLocation, bHit ? FColor::Green : FColor::Red ,false, 0.5f);
		OutSightStrength = 0.2f;
	}
	//const bool bHit = GetWorld()->LineTraceSingleByObjectType(HitResult, ObserverLocation, GetActorLocation(), FCollisionObjectQueryParams(ECC_WorldStatic), TraceParams);

	++NumberOfLoSChecksPerformed;
	OutSeenLocation = GetActorLocation();

	return (bHit == false || (HitResult.Actor.IsValid() && HitResult.GetActor()->IsOwnedBy(this)));
}

bool ATacticalCharacter::IsAIControlled() const
{
	if (PlayerState)
	{
		return PlayerState->bIsABot;
	}
	// Character doesn't seem to have a player state
	// we test the controller if we are authority
	if (Role == ROLE_Authority && GetController() && Cast<AAIController>(GetController()))
	{
		return true;
	}
	// we don't have a controller so we are probably just a proxy. Look up the variable that is replicated for this case.
	return bIsAIControlled;
}

class ATacticalPlayerController* ATacticalCharacter::GetTacticalPlayerController() const
{
	if (GetController())
	{
		return Cast<ATacticalPlayerController>(GetController());
	}
	return nullptr;
}

class ATacticalAIController* ATacticalCharacter::GetTacticalAIController() const
{
	if (GetController())
	{
		return Cast<ATacticalAIController>(GetController());
	}
	return nullptr;
}

void ATacticalCharacter::SetAIWantsToCoverFire(bool bVal)
{
	if (bVal)
	{
		ATacticalCoverMarker* BestCover = GetBestCover();
		if (BestCover)
		{
			const ECoverAimType AimType = BestCover->GetAimType(GetActorLocation());
			bAIWantsToFire = (AimType != ECoverAimType::None);
			CoverAimType = AimType;
		}
		if (CoverAimType != ECoverAimType::None)
		{
			return;
		}
	}

	bAIWantsToFire = false;
	CoverAimType = ECoverAimType::None;
}

class ATacticalCoverMarker* ATacticalCharacter::GetBestCover() const
{
	if ((!bIsAIControlled) && (GetTacticalAIController()) == nullptr)
	{
		return nullptr;
	}

	TArray<ATacticalCoverMarker*> Covers;
	if (GetOverlappingCover(Covers))
	{
		ATacticalCoverMarker* UseCover = Covers[0];
		for (ATacticalCoverMarker* TestCover : Covers)
		{
			// prefer low cover
			if (TestCover->GetCoverType() == ECoverType::LowCover)
			{
				return TestCover;
			}
		}
	}
	return nullptr;
}

void ATacticalCharacter::RemoveWeapon()
{
	if (GetWeapon())
	{
		GetWeapon()->Destroy();
	}
}
