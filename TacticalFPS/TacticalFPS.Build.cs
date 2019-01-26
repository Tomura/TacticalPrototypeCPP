// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

using UnrealBuildTool;

public class TacticalFPS : ModuleRules
{
	public TacticalFPS(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivatePCHHeaderFile = "TacticalFPS.h";
		PublicDependencyModuleNames.AddRange(
            new string[] 
            {
                "Core", 
                "CoreUObject", 
                "Engine", 
                "BRSEngine",
                "NavigationSystem",
                "AIModule",
                "InputCore",
                "RHI", 
                "Slate", 
                "SlateCore", 
                "UMG",
                "XmlParser"
            }
        );
	}
}
