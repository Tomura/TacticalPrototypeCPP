// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#include "TacticalFPS.h"
#include "TacticalPhysicalMaterial.h"


#include "TacticalImpactFX.h"
#include "TacticalImpactFXActor.h"
#include "TacticalImpactFXComponent.h"



UTacticalPhysicalMaterial::UTacticalPhysicalMaterial()
	: Super()
{
	ImpactPSC = nullptr;
	ImpactDecal = nullptr;
	ImpactSound = nullptr;
	ImpactActorClass = nullptr;

	ImpactPSCAxis = EAxis::X;
	DecalSizeRange = FFloatRange(5.f*0.7f, 5.f*1.5f);
}

void UTacticalPhysicalMaterial::SpawnImpactFX(UWorld* World, const FHitResult& Hit, const FVector& ShotDir, class UTacticalImpactFXComponent* ImpactComp)
{
	if (World && World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	if (World)
	{
		const FVector SpawnLoc = Hit.ImpactPoint;
		const FRotator SpawnRot = Hit.ImpactNormal.Rotation();
		
		bool bAllowParticleOverride = true;
		if (bAllowParticleOverride && ImpactComp && ImpactComp->OverrideParticle())
		{
			ImpactComp->SpawnImpactParticle(Hit, ShotDir);
		}
		else if (ImpactPSC != nullptr)
		{
			FRotator ImpactRot;
			switch (ImpactPSCAxis.GetValue())
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

			UParticleSystemComponent* NewParticle = UGameplayStatics::SpawnEmitterAtLocation(World, ImpactPSC, SpawnLoc, ImpactRot, true);
			if (ImpactComp && NewParticle && ImpactComp->OnSpawnImpactParticle.IsBound())
			{
				ImpactComp->OnSpawnImpactParticle.Broadcast(NewParticle, Hit, ShotDir);
			}
		}

		bool bAllowSoundOverride = true;
		if (bAllowSoundOverride && ImpactComp && ImpactComp->OverrideSound())
		{
			ImpactComp->PlayImpactSound(Hit);
		}
		else if (ImpactSound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(World, ImpactSound, SpawnLoc, SpawnRot);
		}


		bool bAllowDecalOverride = true;
		if (bAllowDecalOverride && ImpactComp && ImpactComp->OverrideDecal())
		{
			ImpactComp->SpawnImpactDecal(Hit);
		}
		else
		{
			if (ImpactDecal != nullptr)
			{
				const FVector DecalSize = GetDecalSize();

				if (Hit.Component.IsValid())
				{
					if (Hit.Component->bReceivesDecals)
					{
						FName AttachLoc = Hit.Component->GetClass()->IsChildOf<USkeletalMeshComponent>() ? Hit.BoneName : NAME_None;

						UDecalComponent* SpawnedDecal = ATacticalImpactFX::SpawnImpactDecal(ImpactDecal, Hit, DecalSize, -1);

						UMaterialInstanceDynamic* DynMat = SpawnedDecal->CreateDynamicMaterialInstance();
						const int32 FrameIdx = FMath::RandRange(0, 3);
						DynMat->SetScalarParameterValue(FName("Frame"), (float)FrameIdx);
						SpawnedDecal->SetDecalMaterial(DynMat);
					}
				}
			}
		}

		if (ImpactActorClass != nullptr)
		{
			FRotator ImpactRot = FRotationMatrix::MakeFromZ(ShotDir - 2 * (Hit.ImpactNormal | ShotDir) * Hit.ImpactNormal).Rotator();

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			World->SpawnActor<AActor>(ImpactActorClass, SpawnLoc, ImpactRot, SpawnParams);
		}
	}
}


FVector UTacticalPhysicalMaterial::GetDecalSize() const
{
	const float Width = FMath::FRandRange(DecalSizeRange.GetLowerBoundValue(), DecalSizeRange.GetUpperBoundValue());
	return FVector(5.f, Width, Width);
}
