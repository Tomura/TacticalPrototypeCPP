
#include "TacticalFPS.h"
#include "TacticalCharacter.h"
#include "TacticalInventoryComponent.h"
#include "TacticalWeapon.h"
#include "TacticalWeaponState.h"
#include "Net/UnrealNetwork.h"

UTacticalWeaponState::UTacticalWeaponState()
	: Super()
{
	bDoObstructionCheck = false;

	SetNetAddressable();
	SetIsReplicated(true);
}

FORCEINLINE class ATacticalWeapon* UTacticalWeaponState::GetWeapon() const
{
	return nullptr;
}


FORCEINLINE class ATacticalCharacter* UTacticalWeaponState::GetInventoryOwner() const
{
	return nullptr;
}

class FTimerManager& UTacticalWeaponState::GetWorldTimerManager() const
{
	return GetWorld()->GetTimerManager();
}
