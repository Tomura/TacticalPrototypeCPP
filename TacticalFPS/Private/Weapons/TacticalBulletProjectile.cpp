// Copyright (c) 2015-2016, Tammo Beil - All Rights Reserved.

#include "TacticalFPS.h"
#include "TacticalBulletProjectile.h"
#include "TacticalAmmoType.h"
#include "TacticalImpactFXComponent.h"
#include "TacticalImpactFXActor.h"
#include "TacticalImpactFX.h"




ATacticalBulletProjectile::ATacticalBulletProjectile()
	: Super()
{
	SetReplicates(true);
}

void ATacticalBulletProjectile::BeginPlay()
{
	Super::BeginPlay();
	GetProjectileMovement()->OnProjectileStop.AddDynamic(this, &ATacticalBulletProjectile::OnImpact);
}

void ATacticalBulletProjectile::OnImpact(const FHitResult& Hit)
{
	if (Role == ROLE_Authority)
	{
		if(Hit.Actor != nullptr)
		{
			if (const UTacticalAmmoType* AmmoCDO = (AmmoType ? AmmoType->GetDefaultObject<UTacticalAmmoType>() : nullptr))
			{
				const FVector ShotDir = (Hit.TraceEnd - Hit.TraceStart).GetSafeNormal();
				UGameplayStatics::ApplyPointDamage(Hit.Actor.Get(), AmmoCDO->GetDamage()*0.3f, -ShotDir, Hit, GetInstigator() ? GetInstigator()->GetController() : nullptr,
					GetInstigator(), AmmoCDO->GetDamageType() ? AmmoCDO->GetDamageType() : UDamageType::StaticClass());
			}
		}

		OnSimulateHit(Hit);
		Destroy();
	}
}



void ATacticalBulletProjectile::OnSimulateHit_Implementation(const FHitResult& Hit)
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
		else if (ImpactFXComp)
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
