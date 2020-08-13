// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class EnttUE : ModuleRules
{
	public EnttUE(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivatePCHHeaderFile = Path.Combine(ModuleDirectory, "ue-src", "EnttUE.h");
		CppStandard = CppStandardVersion.Cpp17;
		bEnableExceptions = true;

		PublicIncludePaths.AddRange(new string[] {
				Path.Combine(ModuleDirectory, "src"),
				Path.Combine(ModuleDirectory, "ue-src")
			});


		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"EnttUELibrary",
				"Projects",
				// ... add other public dependencies that you statically link with here ...
			}
			);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// ... add private dependencies that you statically link with here ...	
				"Engine",
				"CoreUObject",
			}
			);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
