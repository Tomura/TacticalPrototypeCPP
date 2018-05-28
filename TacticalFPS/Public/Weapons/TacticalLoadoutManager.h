// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Object.h"
#include "TacticalLoadout.h"
#include "TacticalLoadoutManager.generated.h"

/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta = (DisplayName="[Tactical] Loadout"))
class TACTICALFPS_API UTacticalLoadoutManager : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	// Constants
	static const FString TAG_Loadout;
	static const FString TAG_MainWeapon;
	static const FString TAG_Sidearm;
	static const FString TAG_Gadget1;
	static const FString TAG_Gadget2;
	static const FString TAG_Attachments;
	static const FString TAG_Class;
	static const FString TAG_Ammo;
	static const FString TAG_Params;

	static const FString DIR_Loadout;
	static const FString EXT_Loadout;

public:
	UTacticalLoadoutManager(const FObjectInitializer& OI);

	UPROPERTY(Config, Category = "Loadout Profiles", EditAnywhere, BlueprintReadOnly)
	TArray<FTacticalLoadout>DefaultLoadoutProfiles;

	UPROPERTY(Config, Category = "Loadout Profiles", BlueprintReadOnly)
	TArray<FTacticalLoadout>UserLoadoutProfiles;

	UPROPERTY(Config, Category = "Test", EditAnywhere)
		int32 test[5];
	
	bool AddLoadout(const FTacticalLoadout& NewLoadout);
	bool RemoveLoadout(const FString& FileName);

	FTacticalLoadout LoadFromFile(const FString& InFile);
	void LoadWeaponFromXML(FTacticalLoadout& InLoadout, class FXmlNode* Node, EWeaponClass WeaponType);
	void LoadGadgetFromXML(FTacticalLoadout& InLoadout, class FXmlNode* Node, int32 GadgetNum);

	void WriteToXmlFile(const FTacticalLoadout& InLoadout, const FString& InFile);

	FString GetLoadoutSavedPath(const FString& FileName = TEXT("")) const;

	void GetLoadoutUserFiles(TArray<FString>& FoundFiles) const;
	FString GetLoadoutFile(const FString& FileName) const;
};