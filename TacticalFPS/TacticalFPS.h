// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#ifndef __TACTICALFPS_H__
#define __TACTICALFPS_H__


#include "BRSEngine.h"

#include "AIModule.h"
#include "Perception/AIPerceptionListenerInterface.h"

#include "TacticalTypes.h"
#include "XmlParser.h"

#include "BRSBPFunctionLibrary.h"
#include "BRSStatics.h"
#include "BRSOnlineLibrary.h"

#define TRACE_Weapon ECC_GameTraceChannel1
#define TRACE_WeaponNoCharacter ECC_GameTraceChannel2
#define ECC_Projectile ECC_GameTraceChannel3
#define TRACE_WeaponObstruction ECC_GameTraceChannel4
#define TRACE_AIVision ECC_GameTraceChannel5
#define TRACE_FootPlacement ECC_Visibility //needs to be changed to a dedicated foot trace channel?
#define TRACE_ImpactDecal ECC_GameTraceChannel6
#define TRACE_Interaction ECC_GameTraceChannel7
#define ECC_WeaponMesh ECC_GameTraceChannel8
#define ECC_Helper ECC_GameTraceChannel9


#define TEAM_Players	0
#define TEAM_Enemies	1
#define TEAM_Civilians	2

#define TAG_RemoveAfterRound FName("RemoveAfterRound")


#endif
