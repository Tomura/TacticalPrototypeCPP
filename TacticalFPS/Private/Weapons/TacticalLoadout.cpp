// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#include "TacticalFPS.h"
#include "TacticalWeapon.h"
#include "TacticalGrenade.h"
#include "TacticalLoadout.h"
#include "XmlFile.h"
#include "TacticalBPFunctionLibrary.h"


//FString FTacticalLoadout::GenerateURLParams() const
//{
//	FString TravelParams;
//
//	if (MainWeaponType)
//	{
//		TravelParams = TravelParams.Append(FString::Printf(TEXT("?mainweapon=%s"), *MainWeaponType->GetPathName()));
//		TravelParams = TravelParams.Append(FString::Printf(TEXT("?main_attach_num=%d"), MainWeaponAttachmentNames.Num()));
//		if (MainWeaponAttachmentNames.Num() > 0)
//		{
//			for (int32 i = 0; i < MainWeaponAttachmentNames.Num(); i++)
//			{
//				if (MainWeaponAttachmentIdx.IsValidIndex(i) && MainWeaponAttachmentNames.IsValidIndex(i))
//				{
//					TravelParams = TravelParams.Append(FString::Printf(TEXT("?main_attach_name%d=%s"), i, *MainWeaponAttachmentNames[i]));
//					TravelParams = TravelParams.Append(FString::Printf(TEXT("?main_attach_idx%d=%d"), i, MainWeaponAttachmentIdx[i]));
//				}
//			}
//		}
//		//TravelParams = TravelParams.Append(FString::Printf(TEXT("?main_sight=%d"),	 MainWPN_Sight));
//		//TravelParams = TravelParams.Append(FString::Printf(TEXT("?main_muzzle=%d"),	 MainWPN_Muzzle));
//		//TravelParams = TravelParams.Append(FString::Printf(TEXT("?main_special=%d"), MainWPN_Special));
//		TravelParams = TravelParams.Append(FString::Printf(TEXT("?main_ammo=%d"),	 MainWPN_Ammo));
//	}
//	if (SidearmType)
//	{
//		TravelParams = TravelParams.Append(FString::Printf(TEXT("?sidearm=%s"), *SidearmType->GetPathName()));
//		TravelParams = TravelParams.Append(FString::Printf(TEXT("?side_attach_num=%d"), SidearmAttachmentNames.Num()));
//		if (SidearmAttachmentNames.Num() > 0)
//		{
//			for (int32 i = 0; i < SidearmAttachmentNames.Num(); i++)
//			{
//				if (SidearmAttachmentIdx.IsValidIndex(i) && SidearmAttachmentNames.IsValidIndex(i))
//				{
//					TravelParams = TravelParams.Append(FString::Printf(TEXT("?side_attach_name%d=%s"), i, *SidearmAttachmentNames[i]));
//					TravelParams = TravelParams.Append(FString::Printf(TEXT("?side_attach_idx%d=%d"), i, SidearmAttachmentIdx[i]));
//				}
//			}
//		}
//		//TravelParams = TravelParams.Append(FString::Printf(TEXT("?side_sight=%d"),	  Sidearm_Sight));
//		//TravelParams = TravelParams.Append(FString::Printf(TEXT("?side_muzzle=%d"),	  Sidearm_Muzzle));
//		//TravelParams = TravelParams.Append(FString::Printf(TEXT("?side_special=%d"),  Sidearm_Special));
//		TravelParams = TravelParams.Append(FString::Printf(TEXT("?side_ammo=%d"),	  Sidearm_Ammo));
//	}	
//	if (GrenadeType)
//	{
//		TravelParams = TravelParams.Append(FString::Printf(TEXT("?grenade=%s"), *GrenadeType->GetPathName()));
//	}
//
//	return TravelParams;
//}
//
//FTacticalLoadout FTacticalLoadout::CreateFromURLParams(const FString& Options)
//{
//	FTacticalLoadout NewLoadout;
//
//	// Main Weapon
//	FString TestPath = UGameplayStatics::ParseOption(Options, TEXT("mainweapon"));
//	if (TestPath.Len() > 0)
//	{
//		TSubclassOf<ATacticalPrimaryWeapon> WPNClass = LoadClass<ATacticalPrimaryWeapon>(nullptr, *TestPath, nullptr, LOAD_NoWarn, nullptr);
//		NewLoadout.MainWeaponType = WPNClass;
//
//		int32 NumAttach = FCString::Atoi(*UGameplayStatics::ParseOption(Options, TEXT("main_attach_num")));
//
//		NewLoadout.MainWeaponAttachmentNames.Empty();
//		NewLoadout.MainWeaponAttachmentIdx.Empty();
//		for (int32 i = 0; i < NumAttach; i++)
//		{
//			FString KeyName = FString::Printf(TEXT("main_attach_name%d"), i);
//			FString KeyIdx = FString::Printf(TEXT("main_attach_idx%d"), i);
//			NewLoadout.MainWeaponAttachmentNames.Add(UGameplayStatics::ParseOption(Options, KeyName));
//			NewLoadout.MainWeaponAttachmentIdx.Add(FCString::Atoi(*UGameplayStatics::ParseOption(Options, KeyIdx)));
//		}
//
//		//NewLoadout.MainWPN_Sight = FCString::Atoi(*UGameplayStatics::ParseOption(Options, TEXT("main_sight")));
//		//NewLoadout.MainWPN_Muzzle = FCString::Atoi(*UGameplayStatics::ParseOption(Options, TEXT("main_muzzle")));
//		//NewLoadout.MainWPN_Special = FCString::Atoi(*UGameplayStatics::ParseOption(Options, TEXT("side_special")));
//		NewLoadout.MainWPN_Ammo = FCString::Atoi(*UGameplayStatics::ParseOption(Options, TEXT("side_ammo")));
//	}
//	
//	// Sidearm
//	TestPath = UGameplayStatics::ParseOption(Options, TEXT("sidearm"));
//	if (TestPath.Len() > 0)
//	{
//		TSubclassOf<ATacticalSidearm> WPNClass = LoadClass<ATacticalSidearm>(nullptr, *TestPath, nullptr, LOAD_NoWarn, nullptr);
//		NewLoadout.SidearmType = WPNClass;
//
//		int32 NumAttach = FCString::Atoi(*UGameplayStatics::ParseOption(Options, TEXT("side_attach_num")));
//
//		NewLoadout.SidearmAttachmentNames.Empty();
//		NewLoadout.SidearmAttachmentIdx.Empty();
//		for (int32 i = 0; i < NumAttach; i++)
//		{
//			FString KeyName = FString::Printf(TEXT("side_attach_name%d"), i);
//			FString KeyIdx = FString::Printf(TEXT("side_attach_idx%d"), i);
//			NewLoadout.SidearmAttachmentNames.Add(UGameplayStatics::ParseOption(Options, KeyName));
//			NewLoadout.SidearmAttachmentIdx.Add(FCString::Atoi(*UGameplayStatics::ParseOption(Options, KeyIdx)));
//		}
//
//		//NewLoadout.Sidearm_Sight = FCString::Atoi(*UGameplayStatics::ParseOption(Options, TEXT("side_sight")));
//		//NewLoadout.Sidearm_Muzzle = FCString::Atoi(*UGameplayStatics::ParseOption(Options, TEXT("side_muzzle")));
//		//NewLoadout.Sidearm_Special = FCString::Atoi(*UGameplayStatics::ParseOption(Options, TEXT("side_special")));
//		NewLoadout.Sidearm_Ammo = FCString::Atoi(*UGameplayStatics::ParseOption(Options, TEXT("side_ammo")));
//	}
//
//	// Equipment 1
//	TestPath = UGameplayStatics::ParseOption(Options, TEXT("grenade"));
//	if (TestPath.Len() > 0)
//	{
//		TSubclassOf<ATacticalGrenade> WPNClass = LoadClass<ATacticalGrenade>(nullptr, *TestPath, nullptr, LOAD_NoWarn, nullptr);
//		NewLoadout.GrenadeType = WPNClass;
//	}
//
//	// Equipment 2
//	//	todo: Stuff
//
//	return NewLoadout;
//}

int32 FTacticalLoadout::GetAttachmentIdxFromName(const FString& ident, EWeaponClass WPNType) const
{
	const FString& AttachString = (WPNType == EWeaponClass::MainWeapon) ? MainWPNAttachmentString : SidearmAttachmentString;
	return UGameplayStatics::GetIntOption(AttachString, ident, 0);
}

void FTacticalLoadout::SetAttachment(const FString& AttachPointName, int32 AttachmentIdx, EWeaponClass WPNType)
{
	FString AddString = FString::Printf(TEXT("?%s=%d"), *AttachPointName, AttachmentIdx);
	FString& AttachString = (WPNType == EWeaponClass::MainWeapon) ? MainWPNAttachmentString : SidearmAttachmentString;

	if (UGameplayStatics::HasOption(AttachString, AttachPointName))
	{
		// Remove existing Option
		int32 value = UGameplayStatics::GetIntOption(AttachString, AttachPointName, 0);
		const FString OldPair = FString::Printf(TEXT("?%s=%d"), *AttachPointName, value);
		AttachString = AttachString.Replace(*OldPair, TEXT(""));
	}
	AttachString = AttachString.Append(AddString);
}

TSubclassOf<class ATacticalWeaponAttachment> FTacticalLoadout::GetAttachmentForPoint(const FString& Key, bool bMainWeapon) const
{
	const int32 idx = GetAttachmentIdxFromName(Key, bMainWeapon ? EWeaponClass::MainWeapon : EWeaponClass::Sidearm);
	const TSubclassOf<ATacticalWeapon> WPNClass = bMainWeapon ? *MainWeaponType : *SidearmType;
	if (WPNClass)
	{
		TArray<TSubclassOf<ATacticalWeaponAttachment>> AttachList;
		UTacticalBPFunctionLibrary::GetAttachmentPointAttachmentList(WPNClass, Key, AttachList);
		if (AttachList.IsValidIndex(idx))
		{
			return AttachList[idx];
		}
	}
	return nullptr;
}
