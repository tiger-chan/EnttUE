// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using System.Security.Permissions;
using UnrealBuildTool;

public class EnttUELibrary : ModuleRules
{
	public EnttUELibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		var plugin_dir = Path.Combine(PluginDirectory, "ThirdParty", "EnttUELibrary");

		PublicIncludePaths.AddRange(new string[] {
				Path.Combine(plugin_dir, "entt", "src")
			});
	}
}
