// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

using UnrealBuildTool;

public class BRSEditor : ModuleRules
{
	public BRSEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivatePCHHeaderFile = "Public/BRSEditor.h";
        PrivateIncludePaths.AddRange(new string[] { 
			"BRSEngine/Private/"
            });

		PublicDependencyModuleNames.AddRange(
            new string[] { 
                "Core", 
                "CoreUObject", 
                "Engine", 
                "InputCore", 
                "NavigationSystem",
                "AIModule",
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "RHI", 
                "Slate", 
                "SlateCore", 
                "UMG",
                "EditorStyle",
                "PropertyEditor",
                "DetailCustomizations",
                "BlueprintGraph",
                "AnimGraph",
                "BRSEngine" 
            }
       );
	}
}
