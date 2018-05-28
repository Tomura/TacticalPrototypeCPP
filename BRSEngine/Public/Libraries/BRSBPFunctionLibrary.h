// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once

#include "FindSessionsCallbackProxy.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BRSBPFunctionLibrary.generated.h"

//above name must match the name on your hard disk for this .h file

//note about UBlueprintFunctionLibrary
// This class is a base class for any function libraries exposed to blueprints.
// Methods in subclasses are expected to be static


UCLASS()
class BRSENGINE_API UBRSBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UBRSBPFunctionLibrary(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}

	///** Saves text to filename of your choosing, make sure include whichever file extension you want in the filename, ex: SelfNotes.txt . Make sure to include the entire file path in the save directory, ex: C:\MyGameDir\BPSavedTextFiles */
	//UFUNCTION(BlueprintCallable, Category = "BRS BP Library")
	//static bool SaveStringTextToFile(FString SaveDirectory, FString FileName, FString SaveText, bool AllowOverWriting = false);

	UFUNCTION(BlueprintPure, Category = "BRS BP Library | Input")
	static void GetKeyForAction(const FName& Action, bool bIsConsoleKey, FKey& Key);

	UFUNCTION(BlueprintPure, Category = "BRS BP Library | Input")
	static void GetCharacterForKey(const FKey& Key, FString& Character);


	UFUNCTION(BlueprintPure, Category = "BRS BP Library", meta=(DisplayName="GetDefaultObject"))
	static UObject* BP_GetDefaultObject(UClass* inClass);

	UFUNCTION(BlueprintPure, Category = "BRS BP Library")
	static float GetAnimSequenceLength(class UAnimSequence* Anim);

	UFUNCTION(BlueprintPure, Category = "BRS BP Library")
	static float GetAnimMontageLength(class UAnimMontage* Montage);
	UFUNCTION(BlueprintPure, Category = "BRS BP Library")
	static float GetAnimMontageSectionLength(class UAnimMontage* Montage, int32 Section);

	UFUNCTION()
	static void SetupNavLinkProxy(class ANavLinkProxy* NavLink, const FVector& V1, const FVector& V2);


	//UFUNCTION(BlueprintPure, Category = "BRS BP Library")
	//static bool HasGamepad();

	UFUNCTION(BlueprintPure, Category = "BRS BP Library")
	static bool IsMainMenu();

	UFUNCTION(BlueprintPure, Category = "BRS BP Library")
	static bool IsUsingGamepad(APlayerController* PC);


	/** Hurt locally authoritative actors within the radius. Will only hit components that block the Visibility channel.
	* @param BaseDamage - The base damage to apply, i.e. the damage at the origin.
	* @param Origin - Epicenter of the damage area.
	* @param DamageInnerRadius - Radius of the full damage area, from Origin
	* @param DamageOuterRadius - Radius of the minimum damage area, from Origin
	* @param DamageFalloff - Falloff exponent of damage from DamageInnerRadius to DamageOuterRadius
	* @param DamageTypeClass - Class that describes the damage that was done.
	* @param DamageCauser - Actor that actually caused the damage (e.g. the grenade that exploded)
	* @param InstigatedByController - Controller that was responsible for causing this damage (e.g. player who threw the grenade)
	* @param bFullDamage - if true, damage not scaled based on distance from Origin
	* @param DamagePreventionChannel - Damage will not be applied to victim if there is something between the origin and the victim which blocks traces on this channel
	* @return true if damage was applied to at least one actor.
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game|Damage", meta = (WorldContext = "WorldContextObject", AutoCreateRefTerm = "IgnoreActors", DisplayName="Apply Radial Damage with Falloff (hurt self)"))
	static bool ApplyRadialDamageWithFalloff(UObject* WorldContextObject, float BaseDamage, float MinimumDamage, const FVector& Origin, float DamageInnerRadius, float DamageOuterRadius, float DamageFalloff, TSubclassOf<class UDamageType> DamageTypeClass, const TArray<AActor*>& IgnoreActors, AActor* DamageCauser = NULL, AController* InstigatedByController = NULL, ECollisionChannel DamagePreventionChannel = ECC_Visibility);


	UFUNCTION(BlueprintPure, Category = "Viewport")
	static float HorizontalToVerticalFOV(float HorizontalFOV);

	UFUNCTION(BlueprintPure, Category = "Viewport")
	static float VerticalToHorizontalFOV(float VerticalFOV);

	UFUNCTION(BlueprintPure, Category = "BRS BP Library")
	static const FText GetKeyboardKeyShortName(const FKey& Key);

	UFUNCTION(BlueprintPure, Category = "BRS BP Library")
	static FString GetClassPath(UClass* InClass);

	UFUNCTION(BlueprintPure, Category = "BRS BP Library")
	static TSubclassOf<AGameMode> GetGameModeFromShortName(const FString& ShortName);

	UFUNCTION(BlueprintPure, Category = "BRS BP Library")
		static FString GetNetMode(APlayerController* PC);

	UFUNCTION(BlueprintPure, Category = "AI|BehaviorTree", Meta = (HidePin = "NodeOwner", DefaultToSelf = "NodeOwner"))
	static bool BlackboardKeyValid(class UBTNode* NodeOwner, const FBlackboardKeySelector& Key);
};