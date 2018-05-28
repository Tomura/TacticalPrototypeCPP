// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalThrowable.h"
#include "TacticalCharacter.h"
#include "TacticalProjectile.h"

ATacticalThrowable::ATacticalThrowable(const FObjectInitializer& OI)
	: Super(OI)
{
	NumCharges = 3;
}

void ATacticalThrowable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATacticalThrowable, NumCharges);
}




void ATacticalThrowable::OwnerPressedKey_Implementation()
{
	if (GetInventoryOwner())
	{
		GetInventoryOwner()->StartThrow(this);
	}
}

void ATacticalThrowable::OwnerReleasedKey_Implementation()
{
	if (GetInventoryOwner())
	{
		GetInventoryOwner()->FinishThrow(this);
	}
}

class ATacticalProjectile* ATacticalThrowable::Throw(const FVector& Location, const FRotator& Rotation)
{
	if (!GetInventoryOwner())
		return nullptr;

	if (NumCharges <= 0)
		return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.Owner = GetInventoryOwner();

	ATacticalProjectile* NewProjectile = GetWorld()->SpawnActor<ATacticalProjectile>(ProjectileClass, Location, Rotation, SpawnParams);
	if (NewProjectile)
	{
		//Setup move ignore actors
		NewProjectile->GetCollisionComponent()->IgnoreActorWhenMoving(GetInventoryOwner(), true);
	}

	NumCharges--;

	return nullptr;
}

void ATacticalThrowable::OnRep_NumCharges()
{

}