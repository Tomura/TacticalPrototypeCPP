// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalBPFunctionLibrary.h"
#include "TacticalLoadoutManager.h"
#include "TacticalHUDSettings.h"
#include "TacticalGameInstance.h"
#include "TacticalWeapon.h"
#include "TacticalLevelSummary.h"
#include "TacticalGameModeBase.h"
#include "TacticalGadgetInterface.h"
#include "TacticalWeaponAttachmentPoint.h"
#include "TacticalCoverMarker.h"
#include "TacticalImpactFX.h"


class UTacticalLoadoutManager* UTacticalBPFunctionLibrary::GetLoadoutManager()
{
	return GetMutableDefault<UTacticalLoadoutManager>();
}

bool UTacticalBPFunctionLibrary::ShowCrosshair()
{
	const UTacticalHUDSettings* HUDSettings = GetDefault<UTacticalHUDSettings>();
	if (HUDSettings)
	{
		return HUDSettings->ShowCrosshair();
	}
	return true;
}

bool UTacticalBPFunctionLibrary::Use3dWeaponWidget()
{
	const UTacticalHUDSettings* HUDSettings = GetDefault<UTacticalHUDSettings>();
	if (HUDSettings)
	{
		return HUDSettings->Use3dWeaponWidget();
	}
	return true;
}

FLinearColor UTacticalBPFunctionLibrary::GetHUD_AccentColor()
{
	const UTacticalHUDSettings* HUDSettings = GetDefault<UTacticalHUDSettings>();
	if (HUDSettings)
	{
		return HUDSettings->GetAccentColor();
	}
	return FLinearColor::Red;
}

void UTacticalBPFunctionLibrary::SetHUD_AccentColor(const FLinearColor& NewColor)
{
	UTacticalHUDSettings::SetAccentColor(NewColor);
}

void UTacticalBPFunctionLibrary::GetWeaponClassesOfType(TArray<TSubclassOf<ATacticalWeapon> >& Weapons, TSubclassOf<ATacticalWeapon> ParentClass)
{
	Weapons.Empty();
	if (ParentClass == nullptr)
	{
		ParentClass = ATacticalWeapon::StaticClass();
	}

	// Load all weapons, since we need them
	TArray<FAssetData> AssetList;
	GetAllBlueprintAssetData(ATacticalWeapon::StaticClass(), AssetList, "/Game/Blueprints/Weapons");
	for (FAssetData Asset : AssetList)
	{
		static FName NAME_GeneratedClass(TEXT("GeneratedClass"));
		const FString* ClassPath = Asset.TagsAndValues.Find(NAME_GeneratedClass);
		if (ClassPath != nullptr)
		{
			UClass* TestClass = LoadObject<UClass>(nullptr, **ClassPath);
			if (TestClass != nullptr && !TestClass->HasAnyClassFlags(CLASS_Abstract) && TestClass->IsChildOf(ParentClass))
			{
				//UE_LOG(LogTemp, Log, TEXT("%s"), *TestClass->GetName())
				const ATacticalWeapon* WeaponCDO = TestClass->GetDefaultObject<ATacticalWeapon>();
				const bool bIsGadget = TestClass->ImplementsInterface(UTacticalGadgetInterface::StaticClass());
				if(WeaponCDO && WeaponCDO->IsValidLowLevelFast() && !WeaponCDO->HideFromLoadoutList() && !bIsGadget)
					Weapons.Add(TestClass);
			}
		}
	}
}

void UTacticalBPFunctionLibrary::GetGadgetsClasses(TArray<TSubclassOf<class ATacticalInventory> >& InventoryItems)
{
	TSubclassOf<class ATacticalInventory> ParentClass = ATacticalInventory::StaticClass();
	TArray<FAssetData> AssetList;
	GetAllBlueprintAssetData(ATacticalInventory::StaticClass(), AssetList, "/Game/Blueprints/");
	for (FAssetData Asset : AssetList)
	{
		static FName NAME_GeneratedClass(TEXT("GeneratedClass"));
		const FString* ClassPath = Asset.TagsAndValues.Find(NAME_GeneratedClass);

		if (ClassPath != nullptr)
		{
			UClass* TestClass = LoadObject<UClass>(nullptr, **ClassPath);
			if (TestClass != nullptr && !TestClass->HasAnyClassFlags(CLASS_Abstract) && TestClass->IsChildOf(ParentClass))
			{

				//UE_LOG(LogTemp, Log, TEXT("%s"), *TestClass->GetName())
				const ATacticalInventory* InventoryCDO = TestClass->GetDefaultObject<ATacticalInventory>();
				const bool bIsGadget = TestClass->ImplementsInterface(UTacticalGadgetInterface::StaticClass());
				if (InventoryCDO && InventoryCDO->IsValidLowLevelFast() && !InventoryCDO->HideFromLoadoutList() && bIsGadget)
					InventoryItems.Add(TestClass);
			}
		}
	}
}

void UTacticalBPFunctionLibrary::GetWeaponAttachmentPointNames(TSubclassOf<ATacticalWeapon> WeaponClass, TArray<FString>& VarName, TArray<FText>& LocalizedName)
{
	TArray<UTacticalWeaponAttachmentPoint*> Templates;

	TArray<USCS_Node*> BPNodes;
	TArray<const UBlueprintGeneratedClass*> ParentBPClassStack;
	UBlueprintGeneratedClass::GetGeneratedClassesHierarchy(WeaponClass, ParentBPClassStack);

	for (int32 i = ParentBPClassStack.Num() - 1; i >= 0; i--)
	{
		const UBlueprintGeneratedClass* CurrentBPGClass = ParentBPClassStack[i];
		if (CurrentBPGClass->SimpleConstructionScript)
		{
			BPNodes += CurrentBPGClass->SimpleConstructionScript->GetAllNodes();
		}
	}

	for (int32 i = 0; i < BPNodes.Num(); i++)
	{
		USceneComponent* ComponentTemplate = Cast<USceneComponent>(BPNodes[i]->GetActualComponentTemplate(Cast<UBlueprintGeneratedClass>(WeaponClass)));
		if (ComponentTemplate != NULL)
		{
			if (UTacticalWeaponAttachmentPoint* AttachPointTemplate = Cast<UTacticalWeaponAttachmentPoint>(ComponentTemplate))
			{
				//BPNodes[i]->GetVariableName().ToString();

				Templates.AddUnique(AttachPointTemplate);
			}
		}
	}

	if (Templates.Num() > 0)
	{
		// Sort attachment point templates by priority
		Templates.Sort([](UTacticalWeaponAttachmentPoint& A, UTacticalWeaponAttachmentPoint& B)
		{
			return (A.GetPriority() > B.GetPriority());
		});

		
		for (UTacticalWeaponAttachmentPoint* AttachPoint : Templates)
		{
			const FString Name = AttachPoint->GetName();
			VarName.Add(Name.Replace(TEXT("_GEN_VARIABLE"), TEXT(""), ESearchCase::IgnoreCase));
			LocalizedName.Add(AttachPoint->GetAttachPointName());
		}
	}
}

void UTacticalBPFunctionLibrary::GetAttachmentPointAttachmentList(TSubclassOf<ATacticalWeapon> WeaponClass, const FString& AttachPointKey, TArray<TSubclassOf<ATacticalWeaponAttachment>>& OutList)
{
	//TArray<USCS_Node*> BPNodes;
	TArray<const UBlueprintGeneratedClass*> ParentBPClassStack;
	UBlueprintGeneratedClass::GetGeneratedClassesHierarchy(WeaponClass, ParentBPClassStack);

	// Empty our list so it only contains the list we want to get
	OutList.Empty();

	bool bSuccess = false;
	// find BP Nodes in Class Stack
	for (int32 i = ParentBPClassStack.Num() - 1; i >= 0; i--)
	{
		const UBlueprintGeneratedClass* CurrentBPGClass = ParentBPClassStack[i];
		if (CurrentBPGClass->SimpleConstructionScript)
		{
			for (USCS_Node* BPNode : CurrentBPGClass->SimpleConstructionScript->GetAllNodes())
			{
				USceneComponent* ComponentTemplate = Cast<USceneComponent>(BPNode->GetActualComponentTemplate(Cast<UBlueprintGeneratedClass>(WeaponClass)));
				if (ComponentTemplate != nullptr)
				{
					if (UTacticalWeaponAttachmentPoint* AttachPointTemplate = Cast<UTacticalWeaponAttachmentPoint>(ComponentTemplate))
					{
						if (AttachPointKey.Compare(BPNode->GetVariableName().ToString(), ESearchCase::IgnoreCase))
						{
							AttachPointTemplate->GetAttachmentList(OutList);
							bSuccess = true;
							break;
						}
					}
				}
			}
		}

		if (bSuccess)
		{
			break;
		}
	}
}

int32 UTacticalBPFunctionLibrary::GetAttachmentIdxForPoint(UTacticalWeaponAttachmentPoint* AttachPoint, const FTacticalLoadout& Loadout, bool bMainWeapon)
{
	if (AttachPoint)
	{
		return Loadout.GetAttachmentIdxFromName(AttachPoint->GetName(), bMainWeapon ? EWeaponClass::MainWeapon : EWeaponClass::Sidearm);
	}
	return 0;
}

TSubclassOf<class ATacticalWeaponAttachment> UTacticalBPFunctionLibrary::GetAttachmentForPoint(const FTacticalLoadout& Loadout, const FString& Key, bool bMainWeapon)
{
	return Loadout.GetAttachmentForPoint(Key, bMainWeapon);
}



void UTacticalBPFunctionLibrary::GetMapSummary(const FString& MapName, UTacticalLevelSummary* &Summary)
{
	//UTacticalLevelSummary* Summary = nullptr;
	FString FullMapName;
	if (FPackageName::SearchForPackageOnDisk(MapName + FPackageName::GetMapPackageExtension(), &FullMapName))
	{
		static FName NAME_LevelSummary(TEXT("LevelSummary"));
		UPackage* Pkg = CreatePackage(nullptr, *FullMapName);
		Summary = FindObject<UTacticalLevelSummary>(Pkg, *NAME_LevelSummary.ToString());
		// Check if summary is null. If it is try to load it the hard way
		if (Summary == nullptr)
		{
			// LoadObject() forces whole package load so this is another way. See UT Code for future reference on changes for that
			BeginLoad();

			FLinkerLoad* Linker = GetPackageLinker(Pkg, nullptr, LOAD_NoWarn | LOAD_Quiet, nullptr, nullptr);
			if (Linker != nullptr)
			{
				FPackageIndex SummaryClassIndex, SummaryPackageIndex;
				if (Linker->FindImportClassAndPackage(FName(TEXT("TacticalLevelSummary")), SummaryClassIndex, SummaryPackageIndex)
					&& SummaryPackageIndex.IsImport() && Linker->ImportMap[SummaryPackageIndex.ToImport()].ObjectName == ATacticalGameModeBase::StaticClass()->GetOutermost()->GetFName())
				{
					for (int32 Index = 0; Index < Linker->ExportMap.Num(); Index++)
					{
						FObjectExport& Export = Linker->ExportMap[Index];
						if (Export.ObjectName == NAME_LevelSummary && Export.ClassIndex == SummaryClassIndex)
						{
							Export.Object = NewObject<UTacticalLevelSummary>(Linker->LinkerRoot, UTacticalLevelSummary::StaticClass(), Export.ObjectName,
								EObjectFlags(Export.ObjectFlags | RF_NeedLoad | RF_NeedPostLoad | RF_NeedPostLoadSubobjects | RF_WasLoaded));
							Export.Object->SetLinker(Linker, Index);
							Linker->Preload(Export.Object);
							Export.Object->ConditionalPostLoad();
							Summary = Cast<UTacticalLevelSummary>(Export.Object);
							break;
						}
					}
				}
			}
			EndLoad();
		}
	}
	if (Summary)
	{
		UE_LOG(LogTemp, Log, TEXT("Found Level Summary"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Error Finding Summary"));
	}
}

FText UTacticalBPFunctionLibrary::GetMapTitle(const FString& MapName)
{
	UTacticalLevelSummary* Summary = nullptr;
	UTacticalBPFunctionLibrary::GetMapSummary(MapName, Summary);
	if (Summary)
	{
		return Summary->MapTitle;
	}
	return FText::GetEmpty();
}


ETeamAttitude::Type UTacticalBPFunctionLibrary::GetAttitudeTowards(AActor* Origin, AActor* Other)
{
	if (Origin)
	{
		IGenericTeamAgentInterface* TeamActor = Cast<IGenericTeamAgentInterface>(Origin);
		if (TeamActor)
		{
			return TeamActor->GetTeamAttitudeTowards(*Other);
		}
	}
	return ETeamAttitude::Neutral;
}


bool UTacticalBPFunctionLibrary::IsCoverFree(AActor* Actor, const FVector& Location)
{
	if (Actor == nullptr)
	{
		return false;
	}

	if (Actor->GetWorld() == nullptr)
	{
		return false;
	}

	bool bSuccess = false;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Helper);
	FCollisionQueryParams QueryParams(FName("OVERLAP_Cover"), false);
	FCollisionShape Shape = FCollisionShape::MakeSphere(50.f);

	TArray<FOverlapResult> Overlaps;

	Actor->GetWorld()->OverlapMultiByObjectType(Overlaps, Location, FQuat(), ObjectParams, Shape, QueryParams);
	if (Overlaps.Num() > 0)
	{
		for (FOverlapResult& TestOverlap : Overlaps)
		{
			if (TestOverlap.Actor.IsValid())
			{
				ATacticalCoverMarker* TestCover = Cast<ATacticalCoverMarker>(TestOverlap.GetActor());
				if (TestCover && TestCover->IsCoverLocationFreeConst(Actor, Location))
				{
					bSuccess = true;
				}
			}
		}
	}

	return bSuccess;
}

UDecalComponent* UTacticalBPFunctionLibrary::BP_SpawnImpactDecal(UMaterialInterface* ImpactDecal, const FHitResult& Hit, const FVector& DecalSize, float DecalLifeTime)
{
	return ATacticalImpactFX::SpawnImpactDecal(ImpactDecal, Hit, DecalSize, DecalLifeTime);
}

//void UBRSCreateGameUMG::OnMapSelected(FString SelectedMap)
//{
//	UBRSLevelSummary* Summary = nullptr;
//	FString FullMapName;
//	if (FPackageName::SearchForPackageOnDisk(SelectedMap + FPackageName::GetMapPackageExtension(), &FullMapName))
//	{
//		static FName NAME_LevelSummary(TEXT("LevelSummary"));
//		UPackage* Pkg = CreatePackage(nullptr, *FullMapName);
//		Summary = FindObject<UBRSLevelSummary>(Pkg, *NAME_LevelSummary.ToString());
//		// Check if summary is null. If it is try to load it the hard way
//		if (Summary == nullptr)
//		{
//			// LoadObject() forces whole package load so this is another way. See UT Code for future reference on changes for that
//			BeginLoad();
//
//			FLinkerLoad* Linker = GetPackageLinker(Pkg, nullptr, LOAD_NoWarn | LOAD_Quiet, nullptr, nullptr);
//			if (Linker != nullptr)
//			{
//				FPackageIndex SummaryClassIndex, SummaryPackageIndex;
//				if (Linker->FindImportClassAndPackage(FName(TEXT("BRSLevelSummary")), SummaryClassIndex, SummaryPackageIndex)
//					&& SummaryPackageIndex.IsImport() && Linker->ImportMap[SummaryPackageIndex.ToImport()].ObjectName == ABRSGameMode::StaticClass()->GetOutermost()->GetFName())
//				{
//					for (int32 Index = 0; Index < Linker->ExportMap.Num(); Index++)
//					{
//						FObjectExport& Export = Linker->ExportMap[Index];
//						if (Export.ObjectName == NAME_LevelSummary && Export.ClassIndex == SummaryClassIndex)
//						{
//							Export.Object = NewObject<UBRSLevelSummary>(Linker->LinkerRoot, UBRSLevelSummary::StaticClass(), Export.ObjectName,
//								EObjectFlags(Export.ObjectFlags | RF_NeedLoad | RF_NeedPostLoad | RF_NeedPostLoadSubobjects | RF_WasLoaded));
//							Export.Object->SetLinker(Linker, Index);
//							Linker->Preload(Export.Object);
//							Export.Object->ConditionalPostLoad();
//							Summary = Cast<UBRSLevelSummary>(Export.Object);
//							break;
//						}
//					}
//				}
//			}
//			EndLoad();
//		}
//	}
//	if (Summary != nullptr)
//	{
//		MapAuthor = FText::FromString(Summary->Author);
//		RecommendedPlayers = FText::FromString(FString::Printf(TEXT("%d - %d"), Summary->RecommendedPlayerCount.X, Summary->RecommendedPlayerCount.Y));
//		MapDescription = Summary->Description;
//		PreviewImage = Summary->PreviewImage;
//		LoadingScreenImage = Summary->LoadingScreenImage;
//	}
//	else
//	{
//		MapAuthor = NSLOCTEXT("BRSCreateGameUMG", "UnknownAuthor", "Unknown");
//		RecommendedPlayers = FText::FromString(FString::Printf(TEXT("%d - %d"), 8, 12));
//		MapDescription = FText();
//		PreviewImage = nullptr;
//		LoadingScreenImage = nullptr;
//	}
//}
