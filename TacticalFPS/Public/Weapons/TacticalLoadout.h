// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "TacticalLoadout.generated.h"

USTRUCT(BlueprintType)
struct FTacticalLoadout
{
	GENERATED_USTRUCT_BODY()
public:
	FTacticalLoadout() :
		Name(FString(TEXT("New Loadout"))),
		MainWeaponType(nullptr),
		MainWPN_Ammo(0),
		SidearmType(nullptr),
		Sidearm_Ammo(0),
		GrenadeType(nullptr),
		Gadget1(nullptr),
		Gadget2(nullptr)
		
	{}

	FORCEINLINE bool operator==(const FTacticalLoadout& Other)
	{
		return(Other.Name == Name);
	}

	int32 GetAttachmentIdxFromName(const FString& ident, EWeaponClass WPNType) const;
	void SetAttachment(const FString& AttachPointName, int32 AttachmentIdx, EWeaponClass WPNType);

	TSubclassOf<class ATacticalWeaponAttachment> GetAttachmentForPoint(const FString& Key, bool bMainWeapon) const;

	

	UPROPERTY(Category = Loadout, BlueprintReadWrite, EditAnywhere)
	FString Name;


	UPROPERTY(Category = Loadout, BlueprintReadWrite, EditAnywhere)
	TSubclassOf<class ATacticalPrimaryWeapon> MainWeaponType;

	UPROPERTY(Category = Loadout, BlueprintReadWrite, EditAnywhere)
	int32 MainWPN_Ammo;

	//UPROPERTY(Category = Loadout, BlueprintReadWrite, EditAnywhere)
	//TArray<FString> MainWeaponAttachmentNames;
	//UPROPERTY(Category = Loadout, BlueprintReadWrite, EditAnywhere)
	//TArray<int32> MainWeaponAttachmentIdx;
	UPROPERTY(Category = Loadout, BlueprintReadWrite, EditAnywhere)
	FString MainWPNAttachmentString;
	UPROPERTY(Category = Loadout, BlueprintReadWrite, EditAnywhere)
	FString SidearmAttachmentString;


	UPROPERTY(Category = Loadout, BlueprintReadWrite, EditAnywhere)
	TSubclassOf<class ATacticalSidearm> SidearmType;

	UPROPERTY(Category = Loadout, BlueprintReadWrite, EditAnywhere)
	int32 Sidearm_Ammo;

	//UPROPERTY(Category = Loadout, BlueprintReadWrite, EditAnywhere)
	//TArray<FString> SidearmAttachmentNames;
	//UPROPERTY(Category = Loadout, BlueprintReadWrite, EditAnywhere)
	//TArray<int32> SidearmAttachmentIdx;

	UPROPERTY(Category = Loadout, BlueprintReadWrite, EditAnywhere)
	TSubclassOf<class ATacticalGrenade> GrenadeType;

	UPROPERTY(Category = Loadout, BlueprintReadWrite, EditAnywhere)
	TSubclassOf<class ATacticalInventory> Gadget1;
	UPROPERTY(Category = Loadout, BlueprintReadWrite, EditAnywhere)
	FString Gadget1_Params;
	UPROPERTY(Category = Loadout, BlueprintReadWrite, EditAnywhere)
	TSubclassOf<class ATacticalInventory> Gadget2;
	UPROPERTY(Category = Loadout, BlueprintReadWrite, EditAnywhere)
	FString Gadget2_Params;

	//FString GenerateURLParams() const;
	//static FTacticalLoadout CreateFromURLParams(const FString& Options);


protected:
	TMap<FString, int32> MainWPNAttachments;

	TMap<FString, int32> SidearmAttachments;
};