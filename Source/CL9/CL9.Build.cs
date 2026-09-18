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
			"GameplayTags",
			"NavigationSystem",
			"PhysicsCore",
			"UMG",
			"Slate",
			"SlateCore",
			"Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"AssetRegistry"
		});

		PublicIncludePaths.AddRange(new string[] {
			"CL9"
		});
	}
}
