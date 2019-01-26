// Copyright (c) 2015-2016, Tammo Beil - All rights reserved

using UnrealBuildTool;

public class BRSEngine : ModuleRules
{
    public BRSEngine(ReadOnlyTargetRules Target) : base(Target)
	{
        PrivatePCHHeaderFile = "BRSEngine.h";
		PublicDependencyModuleNames.AddRange(
            new string[] 
            {
                "Core", 
                "CoreUObject", 
                "Engine",
                "EngineSettings",
                "InputCore",             
                "OnlineSubsystem",
                "OnlineSubsystemUtils",
                "RHI", 
                "Slate", 
                "SlateCore", 
                "UMG",
                "AnimGraphRuntime",
                "NavigationSystem",
                "AIModule",
                "AssetRegistry",
                "GameplayTasks"
            }
        );

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32) || (Target.Platform == UnrealTargetPlatform.Linux) || (Target.Platform == UnrealTargetPlatform.Mac))
        {
            PublicDependencyModuleNames.AddRange(new string[] { "Steamworks"/*"Voice", "OnlineSubsystemSteam"*/ });
        }

        PrivateDependencyModuleNames.AddRange(
        new string[] {
                "InputCore",
                "Slate",
                "SlateCore",
                "BRSLoadingScreen",
            }
        );

        DynamicallyLoadedModuleNames.Add("OnlineSubsystemNull");
        if ((Target.Platform == UnrealTargetPlatform.Win32) || (Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Linux) || (Target.Platform == UnrealTargetPlatform.Mac))
        {
            DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
        }
	}
}
