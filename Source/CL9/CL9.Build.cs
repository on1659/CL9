// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CL9 : ModuleRules
{
	public CL9(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"NavigationSystem",
			"PhysicsCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"AssetRegistry"
		});

		PublicIncludePaths.AddRange(new string[] {
			"CL9"
		});
	}
}
