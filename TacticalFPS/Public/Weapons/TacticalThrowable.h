// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Weapons/TacticalInventory.h"
#include "TacticalGadgetInterface.h"
#include "TacticalThrowable.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API ATacticalThrowable : public ATacticalInventory, public ITacticalGadgetInterface
{
	GENERATED_BODY()
	
public:
	ATacticalThrowable(const FObjectInitializer& OI);

	// overrides from ITacticalGadgetInterface
	/** Called when owner presses the key for this gadget */
	virtual void OwnerPressedKey_Implementation() override;
	/** Called when owner releases the key for this gadget */
	virtual void OwnerReleasedKey_Implementation() override;
	/** return type of this gadget */
	virtual ETacticalGadgetType GetGadgetType_Implementation()  override  { return ETacticalGadgetType::Throwable; }
	


	class ATacticalProjectile* Throw(const FVector& Location, const FRotator& Rotation);
	
	UFUNCTION(Category = "Grenade", BlueprintCallable)
	bool HasAnyChargesLeft() const { return (NumCharges > 0); }

protected:

	UPROPERTY(Category = "Grenade", EditAnywhere, ReplicatedUsing = OnRep_NumCharges)
	int32 NumCharges;

	UPROPERTY(Category = "Grenade", EditAnywhere)
	TSubclassOf<class ATacticalProjectile> ProjectileClass;

	UFUNCTION()
	virtual void OnRep_NumCharges();

};
