// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "TacticalTypes.generated.h"

UENUM(BlueprintType)
enum class ETacticalStance : uint8
{
	STANCE_Default		UMETA(DisplayName = "Default"),
	STANCE_Crouched		UMETA(DisplayName = "Crouched"),
	STANCE_Prone		UMETA(DisplayName = "Prone")
};

UENUM(Blueprintable)
enum class ETacticalRotationType : uint8
{
	None,
	Left,
	Right
};

UENUM(Blueprintable)
enum class EDirection : uint8
{
	None,
	Left,
	Right
};

UENUM(Blueprintable)
enum class EWeaponClass : uint8
{
	MainWeapon,
	Sidearm
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None,
	Rifle,
	Pistol
};

UENUM(BlueprintType)
enum class ECoverType : uint8
{
	None,
	HighCover,
	LowCover
};
UENUM(BlueprintType)
enum class ECoverAimType : uint8
{
	None,
	AimUp,
	AimLeft,
	AimRight
};


UENUM(BlueprintType)
enum class EWeaponAttachmentType : uint8
{
	ATTACHMENT_None,
	ATTACHMENT_Sight,
	ATTACHMENT_Muzzle,
	ATTACHMENT_Special
};