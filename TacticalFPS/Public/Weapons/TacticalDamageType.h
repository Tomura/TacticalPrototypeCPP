// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "GameFramework/DamageType.h"
#include "TacticalDamageType.generated.h"

UCLASS()
class UTacticalDamageType : public UDamageType
{
	GENERATED_BODY()
public:
	UTacticalDamageType(const FObjectInitializer& OI) : Super(OI)
	{
		bHasKnockback = false;
		KnockbackStunTime = 0.f;
		HeadshotDamageModifier = 4.f;
	}

	UPROPERTY(Category = TacticalDamage, EditDefaultsOnly, BlueprintReadOnly)
	uint32 bHasKnockback : 1;
	UPROPERTY(Category = TacticalDamage, EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition = "bHasKnockback"))
	float KnockbackStunTime;
	UPROPERTY(Category = TacticalDamage, EditDefaultsOnly, BlueprintReadOnly)
	float HeadshotDamageModifier;
};