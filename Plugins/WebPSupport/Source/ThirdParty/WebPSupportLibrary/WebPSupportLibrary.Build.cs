// Copyright 2025 guigui17f. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class WebPSupportLibrary : ModuleRules
{
	public WebPSupportLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		PublicSystemIncludePaths.Add("$(ModuleDir)/Public");

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64", "libwebp.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64", "libwebpdemux.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64", "libwebpmux.lib"));
		}
	}
}
