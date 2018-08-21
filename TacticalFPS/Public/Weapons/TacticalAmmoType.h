// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Object.h"
#include "TacticalAmmoType.generated.h"

/**
 * 
 */
UCLASS(abstract, Blueprintable)
class TACTICALFPS_API UTacticalAmmoType : public UObject
{
	GENERATED_BODY()

public:
	UTacticalAmmoType();

	float GetDamage() const { return Damage; }
	TSubclassOf<UDamageType> GetDamageType() const { return DamageType; }
	float GetArmorPenetration() const { return ArmorPenetration; }
	float GetRangeModifier() const { return RangeModifier; }
	bool AllowsSurfacePenetration() const { return bAllowSurfacePenetrations; }
	float GetSurfacePenetrationDamageModifier() const { return SurfacePenetrationDamageModifier; }

	int32 GetBuckshots() const { return (bIsBuckshot ? NumBuckshots : 1); }
	float GetBuckshotAngle() const { return (bIsBuckshot ? ScatterAngle : 0.f); }

	// These are both public so you can access them with the class-defaults node. Needs a better solution (probably more native code)
	UPROPERTY(Category = "Ammo", EditDefaultsOnly, BlueprintReadOnly)
		class UStaticMesh* PreviewBoxMesh;
	UPROPERTY(Category = "Ammo", EditDefaultsOnly, BlueprintReadOnly)
		class UStaticMesh* PreviewBulletMesh;

	UPROPERTY(Category = "Ammo", EditDefaultsOnly, BlueprintReadOnly)
		class UParticleSystem* BulletEjectParticle;

	UPROPERTY(Category = "Ammo", EditDefaultsOnly, BlueprintReadOnly)
		class USoundBase* BulletDropSound;

	UFUNCTION(Category = Ammo, BlueprintNativeEvent, BlueprintCallable, BlueprintPure)
		class USoundBase* GetBulletDropSound(EPhysicalSurface SurfaceType) const;
	class USoundBase* GetBulletDropSound_Implementation(EPhysicalSurface SurfaceType) const { return nullptr; }

	UPROPERTY(Category = "Ammo", EditDefaultsOnly, BlueprintReadOnly)
		FText AmmoDisplayName;

	TSubclassOf<class ATacticalImpactFX> GetImpactEffect() const { return ImpactEffect; }

	UFUNCTION(Category = Ammo, BlueprintNativeEvent)
	bool DoesRicochet(const FHitResult& Hit) const;
	virtual bool DoesRicochet_Implementation(const FHitResult& Hit) const { return bRicochet; }

	float GetRicochetAngleThreshold() const { return RicochetAngleThreshold; }
	TSubclassOf<class ATacticalProjectile> GetRicochetProjectile() const { return RicochetProjectile; }


protected:
	UPROPERTY(Category = Ammo, EditDefaultsOnly)
	float ArmorPenetration;
	UPROPERTY(Category = Ammo, EditDefaultsOnly)
	float Damage;
	UPROPERTY(Category = Ammo, EditDefaultsOnly)
	TSubclassOf<UDamageType> DamageType;
	UPROPERTY(Category = Ammo, EditDefaultsOnly)
	float RangeModifier;

	UPROPERTY(Category = Ammo, EditDefaultsOnly)
	uint32 bAllowSurfacePenetrations : 1;
	UPROPERTY(Category = Ammo, EditDefaultsOnly, meta = (EditCondition = "bAllowSurfacePenetrations"))
	float SurfacePenetrationDamageModifier;

	UPROPERTY(Category = Ammo, EditDefaultsOnly)
		uint32 bIsBuckshot : 1;
	UPROPERTY(Category = Ammo, EditDefaultsOnly, meta = (EditCondition = "bIsBuckshot"))
		int32 NumBuckshots;
	UPROPERTY(Category = Ammo, EditDefaultsOnly, meta = (EditCondition = "bIsBuckshot"))
		float ScatterAngle;

	UPROPERTY(Category = Ammo, EditDefaultsOnly)
	uint32 bRicochet : 1;
	UPROPERTY(Category = Ammo, EditDefaultsOnly, meta = (EditCondition = "bRicochet"))
	float RicochetAngleThreshold;
	UPROPERTY(Category = Ammo, EditDefaultsOnly, meta = (EditCondition = "bRicochet"))
	TSubclassOf<class ATacticalProjectile> RicochetProjectile;



	UPROPERTY(Category = Ammo, EditDefaultsOnly)
		TSubclassOf<class ATacticalImpactFX> ImpactEffect;


};
