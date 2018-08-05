// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

#pragma once
#include "NavMesh/RecastNavMesh.h"
#include "TacticalNavigationHelpers.generated.h"

// can hold up to 16 Flags (Default = 0)
UENUM(BlueprintType)
namespace ENavAreaFlag
{
	enum Type
	{
		Default,
		Crouch,
		Jump,
		Door,
		Cover
	};
}

namespace FNavAreaHelper
{
	FORCEINLINE bool IsSet(uint16 Flags, ENavAreaFlag::Type Bit) { return (Flags & (1 << Bit)) != 0; }
	FORCEINLINE void Set(uint16& Flags, ENavAreaFlag::Type Bit) { Flags |= (1 << Bit); }

	FORCEINLINE bool IsNavLink(const FNavPathPoint& PathVert) { return (FNavMeshNodeFlags(PathVert.Flags).PathFlags & RECAST_STRAIGHTPATH_OFFMESH_CONNECTION) != 0; }
	FORCEINLINE bool HasJumpFlag(const FNavPathPoint& PathVert) { return     IsSet(FNavMeshNodeFlags(PathVert.Flags).AreaFlags, ENavAreaFlag::Jump); }
	FORCEINLINE bool HasCrouchFlag(const FNavPathPoint& PathVert) { return IsSet(FNavMeshNodeFlags(PathVert.Flags).AreaFlags, ENavAreaFlag::Crouch); }

	FORCEINLINE bool HasFlag(const FNavPathPoint& PathVert, ENavAreaFlag::Type Bit) { return IsSet(FNavMeshNodeFlags(PathVert.Flags).AreaFlags, Bit); }
}