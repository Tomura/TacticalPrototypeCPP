// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalLoadoutManager.h"
#include "Misc/Paths.h"
#include "TacticalWeapon.h"
#include "FileManager.h"

const FString UTacticalLoadoutManager::TAG_Loadout(TEXT("loadout"));
const FString UTacticalLoadoutManager::TAG_MainWeapon(TEXT("mainwpn"));
const FString UTacticalLoadoutManager::TAG_Sidearm(TEXT("sidearm"));
const FString UTacticalLoadoutManager::TAG_Gadget1(TEXT("gadget1"));
const FString UTacticalLoadoutManager::TAG_Gadget2(TEXT("gadget2"));
const FString UTacticalLoadoutManager::TAG_Attachments(TEXT("attachments"));
const FString UTacticalLoadoutManager::TAG_Class(TEXT("class"));
const FString UTacticalLoadoutManager::TAG_Ammo(TEXT("ammo"));
const FString UTacticalLoadoutManager::TAG_Params(TEXT("param"));
const FString UTacticalLoadoutManager::DIR_Loadout(TEXT("Loadouts"));
const FString UTacticalLoadoutManager::EXT_Loadout(TEXT("xml"));



UTacticalLoadoutManager::UTacticalLoadoutManager(const FObjectInitializer& OI)
	:Super(OI)
{

}

bool UTacticalLoadoutManager::AddLoadout(const FTacticalLoadout& NewLoadout)
{
	// don't save unnamed load outs
	if (NewLoadout.Name.IsEmpty())
		return false;

	// don't allow the same names as the default load outs
	for (FTacticalLoadout& TestDefaultLoadout : DefaultLoadoutProfiles)
	{
		if (TestDefaultLoadout.Name.Compare(NewLoadout.Name, ESearchCase::IgnoreCase) == 0)
		{
			return false;
		}
	}

	// overwrite existing if name is present
	for (FTacticalLoadout& TestLoadout : UserLoadoutProfiles)
	{
		if (TestLoadout.Name == NewLoadout.Name)
		{
			TestLoadout = NewLoadout;
			return true;
		}
	}
	
	UserLoadoutProfiles.Add(NewLoadout);

	return true;
}

bool UTacticalLoadoutManager::RemoveLoadout(const FString& FileName)
{
	IFileManager& FM = IFileManager::Get();
	return FM.Delete(*GetLoadoutSavedPath(FileName), false);
}

FTacticalLoadout UTacticalLoadoutManager::LoadFromFile(const FString& InFile)
{
	FTacticalLoadout ResultLoadout;

	UE_LOG(LogTemp, Log, TEXT("loading loadout from: %s"), *(GetLoadoutSavedPath(InFile)))
	FXmlFile loadoutFile(GetLoadoutSavedPath(InFile));
	FXmlNode* loadoutNode = loadoutFile.GetRootNode();
	if (loadoutNode)
	{
		LoadWeaponFromXML(ResultLoadout, loadoutNode, EWeaponClass::MainWeapon);
		LoadWeaponFromXML(ResultLoadout, loadoutNode, EWeaponClass::Sidearm);
		LoadGadgetFromXML(ResultLoadout, loadoutNode, 1);
		LoadGadgetFromXML(ResultLoadout, loadoutNode, 2);

		return ResultLoadout;
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("invalid file"));
	}
	
	return FTacticalLoadout();
}

void UTacticalLoadoutManager::LoadWeaponFromXML(FTacticalLoadout& InLoadout, class FXmlNode* Node, EWeaponClass WeaponType)
{
	FXmlNode* WpnNode = Node->FindChildNode((WeaponType == EWeaponClass::MainWeapon) ? TAG_MainWeapon : TAG_Sidearm);
	if (WpnNode)
	{
		FXmlNode* ClassNode = WpnNode->FindChildNode(TAG_Class);
		if (ClassNode)
		{
			const FString WeaponClassString = ClassNode->GetContent();
			FStringAssetReference WeaponClassRef(WeaponClassString);

			UObject* WeaponClassObj = WeaponClassRef.TryLoad();
			if(WeaponClassObj)
			{
				UClass* WeaponUClass = Cast<UClass>(WeaponClassObj);
				if (WeaponUClass && WeaponUClass->IsChildOf(ATacticalWeapon::StaticClass()))
				{
					if (WeaponUClass)
					{
						if (WeaponType == EWeaponClass::MainWeapon)
						{
							InLoadout.MainWeaponType = WeaponUClass;
						}
						else
						{
							InLoadout.SidearmType = WeaponUClass;
						}

						{
							const FXmlNode* AttachmentsNode = WpnNode->FindChildNode(TAG_Attachments);
							if (AttachmentsNode)
							{
								TArray<FXmlNode*> AttachmentNodes = AttachmentsNode->GetChildrenNodes();
								for (FXmlNode* AttachNode : AttachmentNodes)
								{
									if (AttachNode)
									{
										const FString AddString = FString::Printf(TEXT("?%s=%s"), *(AttachNode->GetTag()), *(AttachNode->GetContent()));
										if (WeaponType == EWeaponClass::MainWeapon)
											InLoadout.MainWPNAttachmentString = InLoadout.MainWPNAttachmentString.Append(AddString);
										else
											InLoadout.SidearmAttachmentString = InLoadout.SidearmAttachmentString.Append(AddString);
									}
								}
							}
						}

						{
							const FXmlNode* AmmoNode = WpnNode->FindChildNode(TAG_Ammo);
							if (AmmoNode)
							{
								if (WeaponType == EWeaponClass::MainWeapon)
									InLoadout.MainWPN_Ammo = FCString::Atoi(*(AmmoNode->GetContent()));
								else
									InLoadout.Sidearm_Ammo = FCString::Atoi(*(AmmoNode->GetContent()));
							}
						}

					}
				}
			}
		}
	}
}

void UTacticalLoadoutManager::LoadGadgetFromXML(FTacticalLoadout& InLoadout, class FXmlNode* Node, int32 GadgetNum)
{
	const bool bGadget1 = (GadgetNum <= 1);
	FXmlNode* GadgetNode = Node->FindChildNode( bGadget1 ? TAG_Gadget1 : TAG_Gadget2);
	if (GadgetNode)
	{
		FXmlNode* ClassNode = GadgetNode->FindChildNode(TAG_Class);
		if (ClassNode)
		{
			const FString GadgetClassString = ClassNode->GetContent();
			FStringAssetReference GadgetClassRef(GadgetClassString);

			UObject* GadgetClassObj = GadgetClassRef.TryLoad();
			if (GadgetClassObj)
			{
				UClass* GadgetUClass = Cast<UClass>(GadgetClassObj);
				if (GadgetUClass && GadgetUClass->IsChildOf<ATacticalInventory>())
				{
					if (bGadget1)
					{
						InLoadout.Gadget1 = GadgetUClass;
					}
					else
					{
						InLoadout.Gadget2 = GadgetUClass;
					}

					FXmlNode* ParamNode = GadgetNode->FindChildNode(TAG_Params);
					if (ParamNode)
					{
						if (bGadget1)
						{
							InLoadout.Gadget1_Params = ParamNode->GetContent();
						}
						else
						{
							InLoadout.Gadget2_Params = ParamNode->GetContent();
						}
					}
				}
			}
		}
	}
}

void UTacticalLoadoutManager::WriteToXmlFile(const FTacticalLoadout& InLoadout, const FString& InFile)
{
	const FString XML_Root = FString::Printf(TEXT("<%s></%s>"), *TAG_Loadout, *TAG_Loadout);
	FXmlFile LoadoutFile(XML_Root, EConstructMethod::ConstructFromBuffer);

	TArray<FString> Nodes;

	FXmlNode* RootNode = LoadoutFile.GetRootNode();
	if (RootNode)
	{
		RootNode->AppendChildNode(TAG_MainWeapon, TEXT(""));
		RootNode->AppendChildNode(TAG_Sidearm, TEXT(""));
		RootNode->AppendChildNode(TAG_Gadget1, TEXT(""));
		RootNode->AppendChildNode(TAG_Gadget2, TEXT(""));
		FXmlNode* ChildNode = RootNode->FindChildNode(TAG_MainWeapon);
		if (ChildNode)
		{
			ChildNode->AppendChildNode(TEXT("class"), InLoadout.MainWeaponType->GetPathName());
			ChildNode->AppendChildNode(TEXT("attachments"), TEXT(""));
			ChildNode->AppendChildNode(TEXT("ammo"), FString::Printf(TEXT("%d"), InLoadout.MainWPN_Ammo));
			FXmlNode* attachmentnode = ChildNode->FindChildNode(TEXT("attachments"));
			if (attachmentnode)
			{
				FString Attachments = InLoadout.MainWPNAttachmentString;
				FString GrabResult;
				while (UGameplayStatics::GrabOption(Attachments, GrabResult))
				{
					FString PairKey, PairValue;
					UGameplayStatics::GetKeyValue(GrabResult, PairKey, PairValue);
					if (!PairKey.IsEmpty())
					{
						attachmentnode->AppendChildNode(PairKey, PairValue);
					}
				}
			}
		}
		ChildNode = RootNode->FindChildNode(TAG_Sidearm);
		if (ChildNode)
		{
			ChildNode->AppendChildNode(TEXT("class"), InLoadout.SidearmType->GetPathName());
			ChildNode->AppendChildNode(TEXT("attachments"), TEXT(""));
			ChildNode->AppendChildNode(TEXT("ammo"), FString::Printf(TEXT("%d"), InLoadout.Sidearm_Ammo));
			FXmlNode* AttachmentNode = ChildNode->FindChildNode(TEXT("attachments"));
			if (AttachmentNode)
			{
				FString Attachments = InLoadout.SidearmAttachmentString;
				FString GrabResult;
				while (UGameplayStatics::GrabOption(Attachments, GrabResult))
				{
					FString PairKey, PairValue;
					UGameplayStatics::GetKeyValue(GrabResult, PairKey, PairValue);
					if (!PairKey.IsEmpty())
					{
						AttachmentNode->AppendChildNode(PairKey, PairValue);
					}
				}
			}
		}
		ChildNode = RootNode->FindChildNode(TAG_Gadget1);
		if (ChildNode)
		{
			ChildNode->AppendChildNode(TAG_Class, InLoadout.Gadget1->GetPathName());
			ChildNode->AppendChildNode(TAG_Params, InLoadout.Gadget1_Params);
		}
		ChildNode = RootNode->FindChildNode(TAG_Gadget2);
		if (ChildNode)
		{
			ChildNode->AppendChildNode(TAG_Class, InLoadout.Gadget2->GetPathName());
			ChildNode->AppendChildNode(TAG_Params, InLoadout.Gadget2_Params);
		}

	}
	

	UE_LOG(LogTemp, Log, TEXT("Saved Loadout to %s"), *(GetLoadoutSavedPath(InFile)));

	LoadoutFile.Save(GetLoadoutSavedPath(InFile));
}

FString UTacticalLoadoutManager::GetLoadoutSavedPath(const FString& FileName) const
{
	return (FPaths::ProjectSavedDir() + "/" + DIR_Loadout + "/" + GetLoadoutFile(FileName));
}

void UTacticalLoadoutManager::GetLoadoutUserFiles(TArray<FString>& FoundFiles) const
{
	IFileManager& FileManager = IFileManager::Get();

	FString FinalPath = GetLoadoutSavedPath("*." + EXT_Loadout);
	FileManager.FindFiles(FoundFiles, *FinalPath, true, false);
}

FString UTacticalLoadoutManager::GetLoadoutFile(const FString& FileName) const
{
	const int32 LenExt = EXT_Loadout.Len() + 1;
	FString R = FileName.Right(LenExt);
	if (R.Compare("." + EXT_Loadout, ESearchCase::IgnoreCase) == 0)
	{
		return FileName;
	}
	return (FileName + "." + EXT_Loadout);
}
