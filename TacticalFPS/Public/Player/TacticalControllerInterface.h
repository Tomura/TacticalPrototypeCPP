// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "TacticalControllerInterface.generated.h"

UINTERFACE(MinimalAPI)
class UTacticalControllerInterface : public UInterface
{
	GENERATED_BODY()
};

class ITacticalControllerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(Category = "Events", BlueprintNativeEvent)
		void OnPawnTakeDamage(float Damage, const class UDamageType* DamageType, class APawn* InstigatedBy, AActor* DamageCauser);
	virtual void OnPawnTakeDamage_Implementation(float Damage, const class UDamageType* DamageType, class APawn* InstigatedBy, AActor* DamageCauser){}

	UFUNCTION(Category = "Events", BlueprintNativeEvent)
	void OnPawnDied(float Damage, const class UDamageType* DamageType, class APawn* InstigatedBy, AActor* DamageCauser);
	virtual void OnPawnDied_Implementation(float Damage, const class UDamageType* DamageType, class APawn* InstigatedBy, AActor* DamageCauser) {}


};