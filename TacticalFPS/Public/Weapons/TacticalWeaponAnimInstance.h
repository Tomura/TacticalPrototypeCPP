// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Animation/AnimInstance.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalWeaponAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UTacticalWeaponAnimInstance(const FObjectInitializer& OI);
	
	friend class ATacticalWeapon;
protected:

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(Category = "Weapon", BlueprintPure)
	bool IsMagEmpty() { return (AmmoLoaded <= 0); }

	UFUNCTION(Category = "Weapon", BlueprintCallable)
	class ATacticalWeapon* GetOwningWeapon() const;


	UFUNCTION(Category = "Weapon", BlueprintImplementableEvent)
		void OnEventFire(float Time);
	UFUNCTION(Category = "Weapon", BlueprintImplementableEvent)
		void OnEventReload(float Time);



protected:
	UPROPERTY(Category = "Weapon", BlueprintReadOnly, EditAnywhere)
	int32 AmmoLoaded;

	UPROPERTY(Category = "Weapon", BlueprintReadOnly, EditAnywhere)
	EWeaponFiringMode FiringMode;

	

};
