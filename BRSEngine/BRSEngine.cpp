// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "BRSEngine.h"
#include "ARFilter.h"

IMPLEMENT_GAME_MODULE(FDefaultGameModuleImpl, BRSEngine);

DEFINE_LOG_CATEGORY(LogBRSEngine);
DEFINE_LOG_CATEGORY(LogBRSGameSettings);


void GetAllBlueprintAssetData(UClass* BaseClass, TArray<FAssetData>& AssetList, FName Path/* = ""*/)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();



#if WITH_EDITOR
	UE_LOG(LogTemp, Log, TEXT("Scanning for RootPaths to create Asset Meta Data. Not necessary for cooked data."));
	TArray<FString> RootPaths;
	FPackageName::QueryRootContentPaths(RootPaths);
	// Cooked data has the asset data already set up
	AssetRegistry.ScanPathsSynchronous(RootPaths);
	UE_LOG(LogTemp, Log, TEXT("Finished Scanning for Paths."));
#endif


	FARFilter Filter;
	Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());

	/*for (int PathIndex = 0; PathIndex < RootPaths.Num(); PathIndex++)
	{
	ARFilter.PackagePaths.Add(FName(*RootPaths[PathIndex]));
	}*/

	Filter.bRecursivePaths = true;
	Filter.bIncludeOnlyOnDiskAssets = true;

	if (Path != "")
	{
		Filter.PackagePaths.Add(Path);
	}

	if (BaseClass == NULL)
	{
		AssetRegistry.GetAssets(Filter, AssetList);
	}
	else
	{
		// TODO: the below filtering is torturous because the asset registry does not contain full inheritance information for blueprints
		// nor does it return full class paths when you request a class tree

		TArray<FAssetData> LocalAssetList;
		AssetRegistry.GetAssets(Filter, LocalAssetList);

		UE_LOG(LogTemp, Log, TEXT("AssetList: %d"), LocalAssetList.Num());

		TSet<FString> UnloadedBaseClassPaths;
		// first pass: determine the inheritance that we can trivially verify are the correct class because their parent is in memory
		for (int32 i = 0; i < LocalAssetList.Num(); i++)
		{
			const FString* LoadedParentClass = LocalAssetList[i].TagsAndValues.Find("ParentClass");
			if (LoadedParentClass != NULL && !LoadedParentClass->IsEmpty())
			{
				UClass* Class = FindObject<UClass>(ANY_PACKAGE, **LoadedParentClass);
				if (Class == NULL)
				{
					// apparently you have to 'load' native classes once for FindObject() to reach them
					// figure out if this parent is such a class and if so, allow LoadObject()
					FString ParentPackage = *LoadedParentClass;
					ConstructorHelpers::StripObjectClass(ParentPackage);
					if (ParentPackage.StartsWith(TEXT("/Script/")))
					{
						ParentPackage = ParentPackage.LeftChop(ParentPackage.Len() - ParentPackage.Find(TEXT(".")));
						if (FindObject<UPackage>(NULL, *ParentPackage) != NULL)
						{
							Class = LoadObject<UClass>(NULL, **LoadedParentClass, NULL, LOAD_NoWarn | LOAD_Quiet);
						}
					}
				}
				if (Class != NULL)
				{
					if (Class->IsChildOf(BaseClass))
					{
						AssetList.Add(LocalAssetList[i]);
						const FString* GenClassPath = LocalAssetList[i].TagsAndValues.Find("GeneratedClass");
						if (GenClassPath != NULL)
						{
							UnloadedBaseClassPaths.Add(*GenClassPath);
						}
					}
					LocalAssetList.RemoveAt(i);
					i--;
				}
			}
			else
			{
				// asset info is missing; fail
				LocalAssetList.RemoveAt(i);
				i--;
			}
		}
		// now go through the remainder and match blueprints against an unloaded parent
		// if we find no new matching assets, the rest must be the wrong super
		bool bFoundAny = false;
		do
		{
			bFoundAny = false;
			for (int32 i = 0; i < LocalAssetList.Num(); i++)
			{
				if (UnloadedBaseClassPaths.Find(*LocalAssetList[i].TagsAndValues.Find("ParentClass")))
				{
					AssetList.Add(LocalAssetList[i]);
					const FString* GenClassPath = LocalAssetList[i].TagsAndValues.Find("GeneratedClass");
					if (GenClassPath != NULL)
					{
						UnloadedBaseClassPaths.Add(*GenClassPath);
					}
					LocalAssetList.RemoveAt(i);
					i--;
					bFoundAny = true;
				}
			}
		} while (bFoundAny && LocalAssetList.Num() > 0);
	}
}



template <typename SummaryType>
SummaryType* GetMapSummary(const FString& MapName, TSubclassOf<AGameMode> GameModeClass)
{
	SummaryType* Summary = nullptr;
	FString FullMapName;

	// Querying asset registry is faster than SearchForPackageOnDisk so try this first
	TArray<FAssetData> AssetList;
	GetAllAssetData(UWorld::StaticClass(), AssetList, false);
	FName MapFName(*MapName);
	for (const FAssetData& Asset : AssetList);
	{
		if (Asset.AssetName == MapFName)
		{
			FullMapName = Asset.PackageName.ToString();
			break;
		}
	}

	// if FullMap Name is empty use SearchForPackageOnDisk as a last re

	if (FullMapName.Len() > 0 || FPackageName::SearchForPackageOnDisk(MapName + FPackageName::GetMapPackageExtension(), &FullMapName))
	{
		static FName NAME_LevelSummary(TEXT("LevelSummary"));

		UPackage* Pkg = CreatePackage(nullptr, *FullMapName);
		Summary = FindObject<SummaryType>(Pkg, *NAME_LevelSummary.ToString());
		// Check if summary is null. If it is try to load it the hard way
		if (Summary == nullptr)
		{
			// LoadObject() forces whole package load so this is another way. See UT Code for future reference on changes for that
			BeginLoad();
			
			FLinkerLoad* Linker = GetPackageLinker(Pkg, nullptr, LOAD_NoWarn | LOAD_Quiet, nullptr, nullptr);
			if (Linker != nullptr)
			{
				FPackageIndex SummaryClassIndex, SummaryPackageIndex;
				const FName SummaryClassName = SummaryType::StaticClass()->GetFName();

				if (Linker->FindImportClassAndPackage(SummaryClassName, SummaryClassIndex, SummaryPackageIndex)
					&& SummaryPackageIndex.IsImport() && Linker->ImportMap[SummaryPackageIndex.ToImport()].ObjectName == GameModeClass->GetOutermost()->GetFName())
				{
					for (int32 Index = 0; Index < Linker->ExportMap.Num(); Index++)
					{
						FObjectExport& Export = Linker->ExportMap[Index];
						if (Export.ObjectName == NAME_LevelSummary && Export.ClassIndex == SummaryClassIndex)
						{
							Export.Object = NewObject<SummaryType>(Linker->LinkerRoot, SummaryType::StaticClass(), Export.ObjectName,
								EObjectFlags(Export.ObjectFlags | RF_NeedLoad | RF_NeedPostLoad | RF_NeedPostLoadSubobjects | RF_WasLoaded));
							Export.Object->SetLinker(Linker, Index);
							Linker->Preload(Export.Object);
							Export.Object->ConditionalPostLoad();
							Summary = Cast<SummaryType>(Export.Object);
							break;
						}
					}
				}
			}
			EndLoad();
		}
	}
	return Summary;
}