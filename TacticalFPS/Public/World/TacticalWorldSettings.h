// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "GameFramework/WorldSettings.h"
#include "TacticalWorldSettings.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API ATacticalWorldSettings : public AWorldSettings
{
	GENERATED_BODY()
	
public:
	ATacticalWorldSettings(const FObjectInitializer& OI);

	virtual void PostLoad() override;
	virtual void PostInitProperties() override;

#if WITH_EDITOR
	virtual void CheckForErrors() override;
#endif
	
protected:
	/** level summary for UI details */
	UPROPERTY(VisibleAnywhere, Instanced, Category = LevelSummary)
	class UTacticalLevelSummary* LevelSummary;

protected:
	virtual class UTacticalLevelSummary* CreateLevelSummary();
};
