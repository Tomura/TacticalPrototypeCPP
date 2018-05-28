// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponAttachment.h"
#include "TacticalCharacter.h"
#include "TacticalPlayerController.h"
#include "GameFramework/GameUserSettings.h"


ATacticalWeaponAttachment::ATacticalWeaponAttachment(const FObjectInitializer& OI)
	: Super(OI)
{
	WeaponOwner = nullptr;

	Root = OI.CreateDefaultSubobject<USceneComponent>(this, FName("AttachRoot"));
	RootComponent = Root;

	AttachmentName = FText::GetEmpty();
}


void ATacticalWeaponAttachment::BeginPlay()
{
	Super::BeginPlay();
	TArray<UActorComponent*> Comps = GetComponentsByClass(UPrimitiveComponent::StaticClass());
	for (UActorComponent* TestComp : Comps)
	{
		if(TestComp == nullptr)
			continue;

		//UE_LOG(LogTemp, Log, TEXT("Setting up attachment: %s : %s"), *GetName(), *TestComp->GetName());
		if (UMeshComponent* Mesh = Cast<UMeshComponent>(TestComp))
		{
			//UE_LOG(LogTemp, Log, TEXT("Is Primitive!"));
			Mesh->LightingChannels.bChannel0 = false;
			Mesh->LightingChannels.bChannel1 = true;
			Mesh->LightingChannels.bChannel2 = false;
		}
	}
}



void ATacticalWeaponAttachment::OnCharacterOwnerDied()
{

}

void ATacticalWeaponAttachment::OnWeaponRemovedFromOwner()
{

}

void ATacticalWeaponAttachment::OnFirstPerson()
{
	BP_OnFirstPerson();

	auto Meshes = GetComponentsByClass(UMeshComponent::StaticClass());
	for (auto TestComp : Meshes)
	{
		if (auto TestMesh = Cast<UStaticMeshComponent>(TestComp))
		{
			TestMesh->ForcedLodModel = 1;
			continue;
		}

		if (auto TestMesh = Cast<USkeletalMeshComponent>(TestComp))
		{
			TestMesh->ForcedLodModel = 1;
			continue;
		}
	}
}


void ATacticalWeaponAttachment::OnThirdPerson()
{
	BP_OnThirdPerson();

	auto Meshes = GetComponentsByClass(UMeshComponent::StaticClass());
	for (auto TestComp : Meshes)
	{
		if (auto TestMesh = Cast<UStaticMeshComponent>(TestComp))
		{
			TestMesh->ForcedLodModel = 0;
			continue;
		}

		if (auto TestMesh = Cast<USkeletalMeshComponent>(TestComp))
		{
			TestMesh->ForcedLodModel = 0;
			continue;
		}
	}
}

void ATacticalWeaponAttachment::SetupPreviewLighting()
{
	TArray<UActorComponent*> Comps = GetComponentsByClass(UPrimitiveComponent::StaticClass());
	for (UActorComponent* TestComp : Comps)
	{
		if (TestComp == nullptr)
			continue;

		if (UMeshComponent* Mesh = Cast<UMeshComponent>(TestComp))
		{
			Mesh->LightingChannels.bChannel0 = false;
			Mesh->LightingChannels.bChannel1 = false;
			Mesh->LightingChannels.bChannel2 = true;
		}
	}
}




class ATacticalCharacter* ATacticalWeaponAttachment::GetCharacterOwner() const
{
	const ATacticalWeapon* WPNOwner = GetWeaponOwner();
	
	if (WPNOwner)
	{
		return WPNOwner->GetInventoryOwner();
	}
	return nullptr;
}

class ATacticalWeapon* ATacticalWeaponAttachment::GetWeaponOwner() const
{
	if (GetOwner())
	{
		return	Cast<ATacticalWeapon>(GetOwner());
	}
	
	return nullptr;
}

void ATacticalWeaponAttachment::OnStartAiming()
{
	BP_OnStartAiming();
}

void ATacticalWeaponAttachment::OnStopAiming()
{
	BP_OnStopAiming();
}

//void ATacticalWeaponAttachment::OnRepWeaponOwner(ATacticalWeapon* PreviousOwner)
//{
//
//}

ATacticalWeaponAttachment_Muzzle::ATacticalWeaponAttachment_Muzzle(const FObjectInitializer& OI)
	: Super(OI)
{

	SpreadModifier = 1.f;
	RecoilModifier = 1.f;
	SpreadRegenModifier = 1.f;

	bSkipLocalSimulateFire = false;

	NoiseModifier = 1.f;
}


ATacticalWeaponAttachment_Special::ATacticalWeaponAttachment_Special(const FObjectInitializer& OI)
	: Super(OI)
{
	SpreadModifier = 1.f;
	RecoilModifier = 1.f;
	SpreadRegenModifier = 1.f;
	FPPLeftHandAnimation = nullptr;
}

bool ATacticalWeaponAttachment_Special::GetLeftHandTransform_Implementation(FTransform& OutTransform)
{
	return false;
}




ATacticalWeaponAttachment_Sight::ATacticalWeaponAttachment_Sight(const FObjectInitializer& OI)
	: Super(OI)
{
	AimOffset = FVector::ZeroVector;

	AimZoomModifier = 1.f;
	PrimaryActorTick.bCanEverTick = true;
	
	bModifyDepthOfField		= false;
	AimedNearBlurSizeMod	= 0.f;
	AimedFarBlurSizeMod		= 0.f;
	AimedFocusDistanceMod	= 0.f;

	SightAimPoint = OI.CreateDefaultSubobject<USceneComponent>(this, TEXT("AimPoint"));
	SightAimPoint->SetupAttachment(RootComponent);
}



void ATacticalWeaponAttachment_Sight::OnCharacterOwnerDied()
{
	OnStopAiming();
	OnThirdPerson();
}

void ATacticalWeaponAttachment_Sight::OnWeaponRemovedFromOwner()
{
	OnStopAiming();
	OnThirdPerson();
}

void ATacticalWeaponAttachment_Sight::OnFirstPerson()
{
	if (GetCharacterOwner()->IsAiming())
	{
		USceneCaptureComponent2D* Capture = GetSceneCapture();
		if (Capture)
		{
			Capture->SetVisibility(true);
		}
	}
}

void ATacticalWeaponAttachment_Sight::OnThirdPerson()
{
	USceneCaptureComponent2D* Capture = GetSceneCapture();
	if (Capture)
	{
		Capture->SetVisibility(false);
	}
}

FORCEINLINE bool ATacticalWeaponAttachment_Sight::IsFirstPerson() const
{
	const ATacticalCharacter* myChar = GetCharacterOwner();
	if (myChar)
	{
		return !(myChar->IsThirdPerson());
	}
	return false;
}

FVector ATacticalWeaponAttachment_Sight::ModifyAimOffset_Implementation(const FVector& DefaultOffset) const
{
	if (bOverrideAimOffset)
	{
		return SightAimPoint->RelativeLocation;
		//return AimOffset;
	}
	else
	{
		return DefaultOffset - SightAimPoint->RelativeLocation;
	}
}

float ATacticalWeaponAttachment_Sight::GetRecommendedRenderTargetResolution() const
{
	const FIntPoint Resolution = GEngine->GetGameUserSettings()->GetScreenResolution();
	
	return (SightScreenPercent * Resolution.Y);
}

float ATacticalWeaponAttachment_Sight::GetRecommendedFieldOfView() const
{
	ATacticalCharacter* MyChar = GetCharacterOwner();
	if (MyChar)
	{
		ATacticalPlayerController* Controller = MyChar->GetTacticalPlayerController();
		if (Controller)
		{
			const float FOV = Controller->GetDefaultFOV();

		}
	}
	
	return 10.f;
}

void ATacticalWeaponAttachment_Sight::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ATacticalCharacter* myChar  = GetCharacterOwner();
	if (bModifyDepthOfField && myChar != nullptr && IsFirstPerson())
	{
		const float ADS_alpha = (GetWeaponOwner() == myChar->GetWeapon()) ? myChar->GetADSState() : 0.f;
		const FPostProcessSettings& DefaultPlayerPostProcess = GetDefault<ATacticalCharacter>(myChar->GetClass())->GetFirstPersonCamera()->PostProcessSettings;
		FPostProcessSettings& PlayerPostProcess = myChar->GetFirstPersonCamera()->PostProcessSettings;

		const float ppalpha		= FMath::InterpEaseInOut(0.f, 1.f, ADS_alpha, 3);
		const float FarBlur		= DefaultPlayerPostProcess.DepthOfFieldFarBlurSize	 + FMath::FloorToFloat(FMath::Lerp(0.f, AimedFarBlurSizeMod,  ppalpha));
		const float NearBlur	= DefaultPlayerPostProcess.DepthOfFieldNearBlurSize  + FMath::FloorToFloat(FMath::Lerp(0.f, AimedNearBlurSizeMod, ppalpha));
		const float Focus		= DefaultPlayerPostProcess.DepthOfFieldFocalDistance + FMath::FloorToFloat(FMath::Lerp(0.f, AimedFocusDistanceMod,  ppalpha));

		PlayerPostProcess.DepthOfFieldFarBlurSize = FarBlur;
		PlayerPostProcess.DepthOfFieldNearBlurSize = NearBlur;
		PlayerPostProcess.DepthOfFieldFocalDistance = Focus;
	}
}
