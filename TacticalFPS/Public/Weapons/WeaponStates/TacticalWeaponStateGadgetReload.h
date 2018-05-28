#pragma once

#include "TacticalWeaponStateGadget.h"
#include "TacticalWeaponStateGadgetReload.generated.h"


UCLASS(Within = TacticalWeaponAttachment_Weapon)
class TACTICALFPS_API UTacticalWeaponStateGadgetReload : public UTacticalWeaponStateGadget
{
	GENERATED_BODY()

public:
	UTacticalWeaponStateGadgetReload();

	virtual void BeginState(const UTacticalWeaponState* PrevState) override;
	virtual void EndState(const UTacticalWeaponState* NextState) override;

	virtual bool IsReloading() const override { return true; }

	virtual void CancelReload() override;
	void ReloadFinished();

	void SwapMagazine();
	void ContinueReloading();

	virtual void Unequip() override;

	float GetReloadTimeElapsed() const;

protected:
	FTimerHandle TimerReload;
	bool bPendingStop;
};

