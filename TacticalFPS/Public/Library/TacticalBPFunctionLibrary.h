// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GenericTeamAgentInterface.h"
#include "TacticalLoadout.h"
#include "TacticalBPFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class TACTICALFPS_API UTacticalBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(Category = "Loadout", BlueprintCallable, BlueprintPure)
	static class UTacticalLoadoutManager* GetLoadoutManager();

	UFUNCTION(Category = "HUD|Settings", BlueprintCallable, BlueprintPure)
		static bool ShowCrosshair();
	UFUNCTION(Category = "HUD|Settings", BlueprintCallable, BlueprintPure)
		static bool Use3dWeaponWidget();
	UFUNCTION(Category = "HUD|Settings", BlueprintCallable, BlueprintPure)
	static FLinearColor GetHUD_AccentColor();
	UFUNCTION(Category = "HUD|Settings", BlueprintCallable)
	static void SetHUD_AccentColor(const FLinearColor& NewColor);

	UFUNCTION(Category = "Loadout", BlueprintCallable)
	static void GetWeaponClassesOfType(TArray<TSubclassOf<class ATacticalWeapon> >& Weapons, TSubclassOf<ATacticalWeapon>ParentClass);

	UFUNCTION(Category = "Loadout", BlueprintCallable)
	static void GetGadgetsClasses(TArray<TSubclassOf<class ATacticalInventory> >& InventoryItems);

	UFUNCTION(category = "Loadout", BlueprintPure)
	static void GetWeaponAttachmentPointNames(TSubclassOf<ATacticalWeapon> WeaponClass, TArray<FString>& VarName, TArray<FText>& LocalizedName);

	UFUNCTION(category = "Loadout", BlueprintPure)
	static void GetAttachmentPointAttachmentList(TSubclassOf<ATacticalWeapon> WeaponClass, const FString& AttachPointKey, TArray<TSubclassOf<ATacticalWeaponAttachment>>& OutList);




	UFUNCTION(Category = "Level", BlueprintCallable)
	static void GetMapSummary(const FString& MapName, class UTacticalLevelSummary* &Summary);

	UFUNCTION(Category = "Level", BlueprintPure)
	static FText GetMapTitle(const FString& MapName);

	UFUNCTION(Category = "Team Interface", BlueprintPure)
	static ETeamAttitude::Type GetAttitudeTowards(AActor* Origin, AActor* Other);


	UFUNCTION(Category = "Loadout", BlueprintPure)
	static int32 GetAttachmentIdxForPoint(class UTacticalWeaponAttachmentPoint* AttachPoint, const FTacticalLoadout& Loadout, bool bMainWeapon);
	UFUNCTION(category = "Loadout", BlueprintPure)
	static TSubclassOf<class ATacticalWeaponAttachment> GetAttachmentForPoint(const FTacticalLoadout& Loadout, const FString& Key, bool bMainWeapon);

	static bool IsCoverFree(AActor* Actor, const FVector& Location);


	UFUNCTION(Category = "Weapon|FX", BlueprintCallable, meta=(DisplayName="Spawn Impact Decal"))
	static UDecalComponent* BP_SpawnImpactDecal(UMaterialInterface* ImpactDecal, const FHitResult& Hit, const FVector& DecalSize, float DecalLifeTime = 0.f);

};
