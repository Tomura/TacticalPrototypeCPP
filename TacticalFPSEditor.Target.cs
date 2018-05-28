// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TacticalFPSEditorTarget : TargetRules
{
	public TacticalFPSEditorTarget(TargetInfo Target) : base(Target)
    {
		Type = TargetType.Editor;

        ExtraModuleNames.Add("BRSEngine");
        ExtraModuleNames.Add("TacticalFPS");
        ExtraModuleNames.Add("BRSEditor");
    }
}
