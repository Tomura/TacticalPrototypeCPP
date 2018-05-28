// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TacticalFPSTarget : TargetRules
{
	public TacticalFPSTarget(TargetInfo Target) : base(Target)
    {
		Type = TargetType.Game;
        bUsesSteam = true;

        ExtraModuleNames.Add("BRSEngine");
        ExtraModuleNames.Add("TacticalFPS");

        //if (this.bBuildEditor)
        //{
        //    ExtraModuleNames.Add("BRSEditor");
        //}
    }
}
