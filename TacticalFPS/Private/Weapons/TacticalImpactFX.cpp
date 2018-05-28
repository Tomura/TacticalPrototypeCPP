// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalImpactFX.h"
#include "TacticalImpactFXComponent.h"

// Sets default values
ATacticalImpactFX::ATacticalImpactFX()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bSpawnLocal = true;
	bCheckView = false;

	bAllowParticleOverride = true;
	bAllowDecalOverride = true;
	bAllowSoundOverride = true;

	DecalLifeTime = 5.f;
}



void ATacticalImpactFX::SpawnImpactEffects(UWorld* World, const FHitResult& Hit, const FVector& ShotDir, UTacticalImpactFXComponent* ImpactComp)
{

	if (World && World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (World)
	{
		EPhysicalSurface Surface = Hit.PhysMaterial.IsValid() ? Hit.PhysMaterial->SurfaceType : SurfaceType_Default;

		const FVector SpawnLoc = Hit.ImpactPoint;
		const FRotator SpawnRot = Hit.ImpactNormal.Rotation();

		TEnumAsByte<EAxis::Type> ParticleSpawnAxis = EAxis::None;

		if (bAllowParticleOverride && ImpactComp && ImpactComp->OverrideParticle())
		{
			ImpactComp->SpawnImpactParticle(Hit, ShotDir);
		}
		else if (UParticleSystem* ImpactParticle = GetImpactParticle(Hit, Surface, ParticleSpawnAxis))
		{
			FRotator ImpactRot;
			switch (ParticleSpawnAxis.GetValue())
			{
			case EAxis::Z:
				ImpactRot = FRotationMatrix::MakeFromZ(ShotDir - 2 * (Hit.ImpactNormal | ShotDir) * Hit.ImpactNormal).Rotator();
				break;
			case EAxis::Y:
				ImpactRot = FRotationMatrix::MakeFromY(ShotDir - 2 * (Hit.ImpactNormal | ShotDir) * Hit.ImpactNormal).Rotator();
				break;
			case EAxis::X:
			default:
				ImpactRot = FRotationMatrix::MakeFromX(ShotDir - 2 * (Hit.ImpactNormal | ShotDir) * Hit.ImpactNormal).Rotator();
				break;
			}

			UParticleSystemComponent* NewParticle = UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticle, SpawnLoc, ImpactRot, true);
			if (ImpactComp && NewParticle && ImpactComp->OnSpawnImpactParticle.IsBound())
			{
				ImpactComp->OnSpawnImpactParticle.Broadcast(NewParticle, Hit, ShotDir);
			}
		}

		if (bAllowSoundOverride && ImpactComp && ImpactComp->OverrideSound())
		{
			ImpactComp->PlayImpactSound(Hit);
		}
		else if (USoundCue* ImpactSound = GetImpactSound(Hit, Surface))
		{
			UGameplayStatics::PlaySoundAtLocation(World, ImpactSound, SpawnLoc, SpawnRot);
		}

		if (bAllowDecalOverride && ImpactComp && ImpactComp->OverrideDecal())
		{
			ImpactComp->SpawnImpactDecal(Hit);
		}
		else
		{
			if (UMaterialInterface* ImpactDecal = GetImpactDecal(Hit, Surface))
			{
				const FVector DecalSize = GetDecalSize(Surface);

				if (Hit.Component.IsValid())
				{
					if (Hit.Component->bReceivesDecals)
					{
						FName AttachLoc = Hit.Component->GetClass()->IsChildOf<USkeletalMeshComponent>() ? Hit.BoneName : NAME_None;

						UDecalComponent* SpawnedDecal = ATacticalImpactFX::SpawnImpactDecal(ImpactDecal, Hit, DecalSize, DecalLifeTime);

						UMaterialInstanceDynamic* DynMat = SpawnedDecal->CreateDynamicMaterialInstance();
						const int32 FrameIdx = FMath::RandRange(0, 3);
						DynMat->SetScalarParameterValue(FName("Frame"), (float)FrameIdx);
						SpawnedDecal->SetDecalMaterial(DynMat);
					}
				}
			}
		}

		if (UClass* ImpactActorClass = GetImpactActorClass(Hit, Surface))
		{
			FRotator ImpactRot = FRotationMatrix::MakeFromZ(ShotDir - 2 * (Hit.ImpactNormal | ShotDir) * Hit.ImpactNormal).Rotator();

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			World->SpawnActor<AActor>(ImpactActorClass, SpawnLoc, ImpactRot, SpawnParams);
		}
	}
}

UDecalComponent* ATacticalImpactFX::SpawnImpactDecal(UMaterialInterface* ImpactDecal, const FHitResult& Hit, const FVector& DecalSize, float DecalLifeTime)
{
	if (!ImpactDecal)
		return nullptr;

	UDecalComponent* SpawnedDecal = nullptr;
	// we need to change the way be spawn the decal if we have no component to attach to
	if (Hit.Component.IsValid())
	{
		SpawnedDecal = UGameplayStatics::SpawnDecalAttached(ImpactDecal, DecalSize, Hit.Component.Get(), NAME_None, Hit.ImpactPoint, (-Hit.ImpactNormal).Rotation(), EAttachLocation::KeepWorldPosition, DecalLifeTime);
	}
	else if(GWorld) // ensure that world exists
	{
		SpawnedDecal = UGameplayStatics::SpawnDecalAtLocation(GWorld, ImpactDecal, DecalSize, Hit.Location, (-Hit.ImpactNormal).Rotation(), DecalLifeTime);
	}
	
	if(SpawnedDecal)
	{
		SpawnedDecal->ComponentTags.Add(TAG_RemoveAfterRound);
		SpawnedDecal->FadeScreenSize = 0.001f;
	}

	return SpawnedDecal;
}
