// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponPreviewScene.h"
#include "TacticalWeaponAttachment.h"
#include "TacticalWeaponAttachmentPoint.h"
#include "TacticalWPNAttachmentSelection.h"
#include "TacticalWPNAmmoSelection.h"
#include "TacticalPlayerController.h"
#include "TacticalAmmoType.h"
#include "TacticalPauseMenu.h"


// Sets default values
ATacticalWeaponPreviewScene::ATacticalWeaponPreviewScene()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	PreviewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PreviewCamera"));
	PreviewCamera->SetupAttachment(Root);
	PreviewCamera->RelativeLocation = FVector::ZeroVector;
	PreviewCamera->RelativeRotation = FRotator::ZeroRotator;
	PreviewCamera->RelativeScale3D = FVector(1.f, 1.f, 1.f);

	WeaponScene_Pitch = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponScene_Pitch"));
	WeaponScene_Pitch->SetupAttachment(Root);
	WeaponScene_Pitch->RelativeLocation = FVector::ZeroVector;
	WeaponScene_Pitch->RelativeRotation = FRotator::ZeroRotator;
	WeaponScene_Pitch->RelativeScale3D = FVector(1.f, 1.f, 1.f);

	WeaponScene = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponScene"));
	WeaponScene->SetupAttachment(WeaponScene_Pitch);
	WeaponScene->RelativeLocation = FVector::ZeroVector;
	WeaponScene->RelativeRotation = FRotator::ZeroRotator;
	WeaponScene->RelativeScale3D = FVector(1.f, 1.f, 1.f);

	Weapon = nullptr; 
	Sidearm = nullptr;
	PreviewWeapon = nullptr;

	SetTickableWhenPaused(true);
}

// Called when the game starts or when spawned
void ATacticalWeaponPreviewScene::BeginPlay()
{
	Super::BeginPlay();
	
}

void ATacticalWeaponPreviewScene::Destroyed()
{
	if (Weapon)
	{
		Weapon->Destroy();
		Weapon = nullptr;
	}
	if (Sidearm)
	{
		Sidearm->Destroy();
		Sidearm = nullptr;
	}
	PreviewWeapon = nullptr;
}

// Called every frame
void ATacticalWeaponPreviewScene::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	return; // prevent this block
	if (!bRotatePreviewMesh)
	{
		{
			const FRotator CurrentRot = WeaponScene->RelativeRotation;
			const FRotator TargetRot = GetDefault<ATacticalWeaponPreviewScene>(GetClass())->WeaponScene->RelativeRotation;
			if (CurrentRot != TargetRot)
			{
				const FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 5.f);
				WeaponScene->SetRelativeRotation(NewRot);
			}
		}

		{
			const FRotator CurrentRot = WeaponScene_Pitch->RelativeRotation;
			const FRotator TargetRot = GetDefault<ATacticalWeaponPreviewScene>(GetClass())->WeaponScene_Pitch->RelativeRotation;
			if (CurrentRot != TargetRot)
			{
				const FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 5.f);
				WeaponScene_Pitch->SetRelativeRotation(NewRot);
			}
		}
	}
}

void ATacticalWeaponPreviewScene::EnablePreviewRotation(bool bEnabled)
{
	bRotatePreviewMesh = bEnabled;
}

void ATacticalWeaponPreviewScene::RotatePreviewX(float Delta)
{
	if (bRotatePreviewMesh)
	{
		FRotator DeltaRot = FRotator(0.f, -Delta * RotationRate, 0.f);
		WeaponScene->AddRelativeRotation(DeltaRot);
	}
}

void ATacticalWeaponPreviewScene::RotatePreviewY(float Delta)
{
	if (bRotatePreviewMesh)
	{
		FRotator DeltaRot = FRotator(-Delta * RotationRate, 0.f, 0.f);
		WeaponScene_Pitch->AddRelativeRotation(DeltaRot);
	}
}



void ATacticalWeaponPreviewScene::OnClickedOutside()
{
	if (AttachmentWidgets.Num() <= 0)
		return;

	for (UTacticalWPNAttachmentSelection* TestWidget : AttachmentWidgets)
	{
		TestWidget->OnOtherClicked();
	}
	if (AmmoWidget)
	{
		AmmoWidget->OnOtherClicked();
	}
}

class ATacticalWeapon* ATacticalWeaponPreviewScene::SpawnPreviewWeapon(TSubclassOf<ATacticalWeapon> WeaponClass)
{
	FTransform SpawnTramsform = FTransform(GetActorRotation(), GetActorLocation(), FVector(1.f));
	// we need deferred spawning here, to disable replication before it's too late
	ATacticalWeapon* NewWeapon = GetWorld()->SpawnActorDeferred<ATacticalWeapon>(WeaponClass, SpawnTramsform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (NewWeapon && NewWeapon->GetMesh())
	{
		NewWeapon->SetReplicates(false);
		NewWeapon->SetReplicateMovement(false);
		NewWeapon->SetTickableWhenPaused(true);
		USkeletalMeshComponent* MeshAsset = NewWeapon->GetMesh();
		MeshAsset->SetRelativeTransform(NewWeapon->PreviewTransform);
		MeshAsset->LightingChannels.bChannel0 = false;
		MeshAsset->LightingChannels.bChannel1 = false;
		MeshAsset->LightingChannels.bChannel2 = true;
		NewWeapon->FinishSpawning(SpawnTramsform);
		NewWeapon->AttachToComponent(this->WeaponScene, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, false));
		NewWeapon->SetActorRelativeLocation(NewWeapon->PreviewTransform.GetLocation(), false);
		NewWeapon->GetMesh()->ForcedLodModel = 1; // Force to LOD 0 (ForcedLodModel -1)
		NewWeapon->FiringMode = EWeaponFiringMode::FM_None;
	}

	return NewWeapon;
}

void ATacticalWeaponPreviewScene::SetupAmmoPreview(ATacticalWeapon* InWeapon, APlayerController* PC)
{
	if (InWeapon)
	{
		int32 AmmoIdx = 0;
		ATacticalPlayerController* TacPC = PC ? Cast<ATacticalPlayerController>(PC) : nullptr;
		if (TacPC)
		{
			bool bMainWPN = InWeapon->GetClass()->IsChildOf(ATacticalPrimaryWeapon::StaticClass());
			if (bMainWPN)
			{
				AmmoIdx = TacPC->GetLoadout().MainWPN_Ammo;
			}
			else
			{
				AmmoIdx = TacPC->GetLoadout().Sidearm_Ammo;
			}
		}
		SetAmmoPreview(AmmoIdx);
	}
}

void ATacticalWeaponPreviewScene::SetWeapon(TSubclassOf<class ATacticalWeapon> WeaponClass, APlayerController* PC, EWeaponClass WPNType)
{
	ATacticalWeapon*& WeaponRef = (WPNType == EWeaponClass::MainWeapon) ? Weapon : Sidearm;

	if (WeaponClass == nullptr)
		return;

	bool bIsPreview = (PreviewWeapon == WeaponRef);

	if (WeaponRef)
	{
		if (WeaponRef == PreviewWeapon)
		{
			PreviewWeapon = nullptr;
		}
		WeaponRef->Destroy();
		WeaponRef = nullptr;
	}

	WeaponRef = SpawnPreviewWeapon(WeaponClass);
	if (WeaponRef)
	{
		WeaponRef->GetRootComponent()->SetVisibility(bIsPreview, true);
		ATacticalPlayerController* TacPC = PC ? Cast<ATacticalPlayerController>(PC) : nullptr;
		const int32 AmmoIdx = (WPNType == EWeaponClass::MainWeapon) ? TacPC->GetLoadout().MainWPN_Ammo : TacPC->GetLoadout().Sidearm_Ammo;
		WeaponRef->SetAmmoType(AmmoIdx);
		//SetupAmmoPreview(Weapon, PC);
	}

	if (bIsPreview)
	{
		SetPreviewWeapon(WPNType, PC);
	}
}

void ATacticalWeaponPreviewScene::SetPreviewWeapon(EWeaponClass WPNType, APlayerController* PC)
{
	
	ATacticalWeapon* NewPreviewWeapon = (WPNType == EWeaponClass::MainWeapon) ?	Weapon : Sidearm;

	if (PreviewWeapon && PreviewWeapon != NewPreviewWeapon)
	{
		PreviewWeapon->GetRootComponent()->SetVisibility(false, true);
	}

	if (NewPreviewWeapon != nullptr)
	{
		NewPreviewWeapon->GetRootComponent()->SetVisibility(true, true);
		PreviewWeapon = NewPreviewWeapon;
	}
	SetupAmmoPreview(PreviewWeapon, PC);
	UpdateAttachmentWidgets(PC);
}

USkeletalMeshComponent* ATacticalWeaponPreviewScene::GetWeaponMesh_Implementation() const
{
	if (Weapon && !Weapon->IsPendingKill())
	{
		return Weapon->GetMesh();
	}
	return nullptr;
}


void ATacticalWeaponPreviewScene::AddWeaponAttachment(const FString& AttachmentPoint, int32 AttachmentIdx, EWeaponClass WPNClass)
{
	ATacticalWeapon* WeaponToAdd = (WPNClass == EWeaponClass::MainWeapon) ? Weapon : Sidearm;

	if (WeaponToAdd)
	{
		ATacticalWeaponAttachment* SpawnedAttachment = WeaponToAdd->CreateAttachment(AttachmentPoint, AttachmentIdx);

		if (SpawnedAttachment != nullptr)
		{
			SpawnedAttachment->SetupPreviewLighting();
			if (WeaponToAdd != PreviewWeapon)
			{
				WeaponToAdd->GetRootComponent()->SetVisibility(false, true);
			}
		}
	}
}


void ATacticalWeaponPreviewScene::SetAmmoPreview(TSubclassOf<class UTacticalAmmoType> AmmoClass)
{
	if (AmmoClass == nullptr)
	{
		SetAmmoPreviewMeshes(nullptr, nullptr);
		return;
	}

	const UTacticalAmmoType* AmmoCDO = AmmoClass->GetDefaultObject<UTacticalAmmoType>();
	if (AmmoCDO)
	{
		SetAmmoPreviewMeshes(AmmoCDO->PreviewBoxMesh, AmmoCDO->PreviewBulletMesh);
	}
}

void ATacticalWeaponPreviewScene::SetAmmoPreview(int32 AmmoIdx)
{
	const TArray<TSubclassOf<UTacticalAmmoType>> AllowedAmmos = PreviewWeapon->GetAllowedAmmo();
	TSubclassOf<UTacticalAmmoType> AmmoType = nullptr;
	if (AllowedAmmos.IsValidIndex(AmmoIdx))
	{
		AmmoType = AllowedAmmos[AmmoIdx];
	}
	else if (AllowedAmmos.IsValidIndex(0))
	{
		AmmoType = AllowedAmmos[0];
	}
	SetAmmoPreview(AmmoType);
}


void ATacticalWeaponPreviewScene::SetFromPlayer(class ATacticalPlayerController* Controller)
{
	if (Controller && Controller->IsValidLowLevel())
	{
		SetWeapon(Controller->GetLoadout().MainWeaponType, Controller, EWeaponClass::MainWeapon);
		SetWeapon(Controller->GetLoadout().SidearmType, Controller, EWeaponClass::Sidearm);

		if (Weapon)
		{
			TArray<UTacticalWeaponAttachmentPoint*> AttachPoints;
			Weapon->GetAttachmentPoints(AttachPoints);
			for (UTacticalWeaponAttachmentPoint* P : AttachPoints)
			{
				const int32 idx = Controller->GetLoadout().GetAttachmentIdxFromName(P->GetName(), EWeaponClass::MainWeapon);
				AddWeaponAttachment(P->GetName(), idx, EWeaponClass::MainWeapon);
			}
		}
		if (Sidearm)
		{
			TArray<UTacticalWeaponAttachmentPoint*> AttachPoints;
			Sidearm->GetAttachmentPoints(AttachPoints);
			for (UTacticalWeaponAttachmentPoint* P : AttachPoints)
			{
				const int32 idx = Controller->GetLoadout().GetAttachmentIdxFromName(P->GetName(), EWeaponClass::Sidearm);
				AddWeaponAttachment(P->GetName(), idx, EWeaponClass::Sidearm);
			}
		}
	}
	UpdateAttachmentWidgets(Controller);
}

void ATacticalWeaponPreviewScene::OnFocusPreview(class ATacticalPlayerController* PC)
{
	SetPreviewWeapon(EWeaponClass::MainWeapon, PC);
}

void ATacticalWeaponPreviewScene::OnUnfocusPreview(class ATacticalPlayerController* PC)
{
	RemoveAllAttachmentWidgets();
	bRotatePreviewMesh = false;
}

void ATacticalWeaponPreviewScene::UpdateAttachmentWidgets(class APlayerController* PC)
{
	RemoveAllAttachmentWidgets();

	if (!AttachmentWidgetClass)
		return;

	if (!PreviewWeapon)
		return;

	ATacticalPlayerController* TPC = Cast<ATacticalPlayerController>(PC);
	if (!TPC)
	{
		return;
	}


	if(AttachmentWidgetClass)
	{
		TArray<UTacticalWeaponAttachmentPoint*> FoundComps;
		PreviewWeapon->GetComponents<UTacticalWeaponAttachmentPoint>(FoundComps);
		for (UTacticalWeaponAttachmentPoint* TestComp : FoundComps)
		{
			if (TestComp)
			{
				UTacticalWPNAttachmentSelection* NewWidget = CreateWidget<UTacticalWPNAttachmentSelection>(PC, AttachmentWidgetClass);
				if (NewWidget)
				{
					NewWidget->SetAttachmentPoint(TestComp);
					NewWidget->SetPreviewActor(this);
					UTacticalPauseMenu* MenuWidget = TPC->GetPauseMenuWidget();
					if (MenuWidget->GetMainOverlay())
					{
						MenuWidget->GetMainOverlay()->AddChildToOverlay(NewWidget);
					}
					//NewWidget->AddToPlayerScreen(-500);
					AttachmentWidgets.Add(NewWidget);
					//UE_LOG(LogTemp, Log, TEXT("Added Widget: %s for Attachpoint %s"), *NewWidget->GetName(), *TestComp->GetName());
					NewWidget->InitAttachmentList();
				}
			}
		}
	}

	// Also add ammo widget
	if(AmmoWidgetClass)
	{
		UTacticalWPNAmmoSelection* NewWidget = CreateWidget<UTacticalWPNAmmoSelection>(PC, AmmoWidgetClass);
		if(NewWidget)
		{
			NewWidget->SetPreviewActor(this);
			UTacticalPauseMenu* MenuWidget = TPC->GetPauseMenuWidget();
			if (MenuWidget->GetMainOverlay())
			{
				MenuWidget->GetMainOverlay()->AddChildToOverlay(NewWidget);
			}
			AmmoWidget = NewWidget;
			AmmoWidget->WeaponType = PreviewWeapon ? PreviewWeapon->GetClass() : nullptr;
			AmmoWidget->InitAmmoList();
		}
	}
}

void ATacticalWeaponPreviewScene::RemoveAllAttachmentWidgets()
{
	for (UTacticalWPNAttachmentSelection* TestWidget : AttachmentWidgets)
	{
		if (TestWidget != nullptr && TestWidget->IsValidLowLevel())
		{
			TestWidget->RemoveFromParent();
		}
		TestWidget = nullptr;
	}

	if (AmmoWidget)
	{
		AmmoWidget->RemoveFromParent();
		AmmoWidget = nullptr;
	}
}